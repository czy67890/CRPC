//
// Created by czy on 2023/12/26.
//

#include <cassert>

#include "work_stealing_thread_pool.h"
#include "core/lib/event_engine/thread_local.h"
#include "core/lib/backoff/backoff.h"
#include "core/lib/event_engine/comm_closures.h"
namespace crpc_event_engine{
    namespace {
        thread_local WorkQueue* g_local_queue{nullptr};

        constexpr auto kIdleThreadLimit  = std::chrono::seconds(20);
        ///the time gap when we can start a new thread
        constexpr uint64_t kPerNanoSecondEachSecond = 1e9;
        constexpr auto kTimeBetweenThrottledThreadStart = std::chrono::nanoseconds (kPerNanoSecondEachSecond);

    }

    void WorkStealingThreadPool::TheftReistry::Enroll(WorkQueue *queue) {
        std::lock_guard<std::mutex> lk{mu_};
        queues_.insert(queue);
    }

    void WorkStealingThreadPool::TheftReistry::UnEnroll(WorkQueue *queue) {
        std::lock_guard<std::mutex> lk{mu_};
        queues_.erase(queue);
    }

    EventEngine::Closure *WorkStealingThreadPool::TheftReistry::StealOne() {
        EventEngine::Closure *closure;
        std::lock_guard<std::mutex> lk{mu_};
        for(auto *queue:queues_){
            closure = queue->PopMostRecent();
            if(closure){
                return closure;
            }
        }
        return nullptr;
    }


    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::Quit() {
        SetShutDown(true);
        bool is_threadpool_thread = (g_local_queue != nullptr);
        GetWorkSignal()->SignalAll();
        ///wait util all thread be closed
        GetLivingThreadCount()->BlockUtilThreadCount(is_threadpool_thread ? 1 : 0);
        quit_.store(true,std::memory_order_relaxed);
        life_guard_.BlockUtilShutDownAndReset();
    }



    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::Run(EventEngine::Closure *closure) {
        assert(!quit_.load(std::memory_order_relaxed));

        if(g_local_queue != nullptr &&  g_local_queue->owner() == this){
            g_local_queue->Add(closure);
        }
        else{
            queue_.Add(closure);
        }
        work_signal_.Signal();
    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::Start() {
        for(size_t i = 0; i < reserve_threads_;++i){
            StartThread();
        }
        life_guard_.Start();
    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::StartThread() {
        std::chrono::nanoseconds nanosecond = std::chrono::steady_clock::now().time_since_epoch();
        last_start_thread_.store(nanosecond.count());
        std::thread t([](ThreadState *arg){
            arg->ThreadBody();
            delete arg;
        },new ThreadState(shared_from_this()));
        t.detach();
    }

    bool WorkStealingThreadPool::WorkStealingThreadPoolImpl::SetThrottled(bool throttle) {
        return throttled_.exchange(throttle,std::memory_order_relaxed);
    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::SetShutDown(bool is_shutdown) {
        auto was_shutdown = shutdown_.exchange(is_shutdown,std::memory_order_relaxed);
        assert(!was_shutdown);
        work_signal_.SignalAll();
    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::PrePareFork() {
        SetForking(true);
        work_signal_.SignalAll();
        living_thread_count_.BlockUtilThreadCount(0);
        life_guard_.BlockUtilShutDownAndReset();
    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::PostFork() {
        SetForking(false);
        Start();
    }

    bool WorkStealingThreadPool::WorkStealingThreadPoolImpl::IsShutDown() {
        return shutdown_.load(std::memory_order_relaxed);
    }

    bool WorkStealingThreadPool::WorkStealingThreadPoolImpl::IsForking() {
        return forking_.load(std::memory_order_relaxed);
    }

    bool WorkStealingThreadPool::WorkStealingThreadPoolImpl::IsQuit() {
        return quit_.load(std::memory_order_relaxed);
    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::SetForking(bool is_forking) {
        auto was_forking =  forking_.exchange(is_forking,std::memory_order_relaxed);
        assert(was_forking != is_forking);
    }

    LivingThreadCounter *WorkStealingThreadPool::WorkStealingThreadPoolImpl::GetLivingThreadCount() {
        return &living_thread_count_;
    }

    WorkStealingThreadPool::TheftReistry *WorkStealingThreadPool::WorkStealingThreadPoolImpl::GetTheftRegistry() {
        return &theft_reistry_;
    }

    BusyThreadCount *WorkStealingThreadPool::WorkStealingThreadPoolImpl::GetBusyThreadCount() {
        return &busy_thread_count_;
    }

    WorkStealingThreadPool::WorkStealingThreadPoolImpl::WorkStealingThreadPoolImpl(size_t reserve_thread)
        :reserve_threads_(reserve_thread), life_guard_(this),queue_(this)
    {

    }


    void WorkStealingThreadPool::ThreadState::ThreadBody() {
        g_local_queue = new BasicWorkQueue(threadpool_impl_.get());
        threadpool_impl_->GetTheftRegistry()->Enroll(g_local_queue);
        ThreadLocal::SetIsEventEngineThreadLocal(true);

        while(Step()){

        }

        if(threadpool_impl_->IsForking()){
            EventEngine::Closure* closure{nullptr};

            while(!g_local_queue->Empty()){
                closure = g_local_queue->PopMostRecent();
                if(closure) {
                    threadpool_impl_->GetWorkQueue()->Add(closure);
                }
            }
        }
        else if(threadpool_impl_->IsShutDown()){
            /// run the task added to the thread pool over
            FinishDraining();
        }

        assert(g_local_queue->Empty());

        threadpool_impl_->GetTheftRegistry()->UnEnroll(g_local_queue);

        /// this source need to be recyle
        delete g_local_queue;

    }

    bool WorkStealingThreadPool::ThreadState::Step() {
       if(threadpool_impl_->IsForking()){
           return false;
       }
       auto *closure = g_local_queue->PopMostRecent();
       if(closure){
           /// such a beauty RAII design
            auto busy = threadpool_impl_->GetBusyThreadCount()->MakeAutoThreadCounter(busy_count_index_);
            closure->Run();
            return true;
       }

       bool should_run_again {false};

       auto start_time = std::chrono::steady_clock::now();

       while(!threadpool_impl_->IsForking()){
           closure = threadpool_impl_->GetWorkQueue()->PopMostRecent();
           if(closure){
                should_run_again = true;
                break;
           }
           closure = threadpool_impl_->GetTheftRegistry()->StealOne();
           if(closure){
               should_run_again = true;
               break;
           }

           if(threadpool_impl_->IsShutDown()){
                break;
           }

           auto time_out = backoff_.NextAttemptTimeNanoCount();
           bool is_time_out = threadpool_impl_->GetWorkSignal()->WaitWithTimeOut(time_out);

           if(threadpool_impl_->IsForking() || threadpool_impl_->IsShutDown()){
               break;
           }

           /// in the case of the thread count greater than the thread-pool start
           /// and the wait time is too long , we close this thread
           /// the idea is that the thread not  longer be needed

           if(is_time_out && (threadpool_impl_->GetLivingThreadCount()->Count() > threadpool_impl_->ReserveThreads())){
               if(std::chrono::steady_clock::now() - start_time > kIdleThreadLimit){
                   /// the thread created by the Living Thread Guard
                   /// is closed here
                   return false;
               }
           }
       }

        if(threadpool_impl_->IsForking()){
            if(closure){
                /// store to the g_local_queue
                /// when we in a fork state
                g_local_queue->Add(closure);
                return false;
            }
        }

        if(closure){
            auto busy = threadpool_impl_->GetBusyThreadCount()->MakeAutoThreadCounter(busy_count_index_);
            closure->Run();

        }
        backoff_.Reset();
        return should_run_again;
    }


    /// when we shutdown a thread pool
    /// we need to run all the task remained
    void WorkStealingThreadPool::ThreadState::FinishDraining() {
        auto busy = threadpool_impl_->GetBusyThreadCount()->MakeAutoThreadCounter(busy_count_index_);

        while(!threadpool_impl_ ->IsForking()){
            if(!g_local_queue->Empty()){
                auto *closure = g_local_queue->PopMostRecent();
                if(closure){
                    closure->Run();
                }
                continue;
            }
            if(!threadpool_impl_->GetWorkQueue()->Empty()){
                auto *closure = threadpool_impl_->GetWorkQueue()->PopMostRecent();
                if(closure){
                    closure->Run();
                }
                continue;
            }
            break;
        }


    }

    WorkStealingThreadPool::ThreadState::ThreadState(std::shared_ptr<WorkStealingThreadPoolImpl> pool)
        :threadpool_impl_(std::move(pool)),
        auto_thread_counter_(threadpool_impl_->GetLivingThreadCount()),
        backoff_(crpc_core::BackOff::Options{}),busy_count_index_(threadpool_impl_->GetBusyThreadCount()->NextIndex())
    {

    }

    void WorkStealingThreadPool::ThreadState::SleepIfRunning() {
        if(threadpool_impl_->IsForking()){
            return;
        }
        std::this_thread::sleep_for(kTimeBetweenThrottledThreadStart);
    }

    void WorkStealingThreadPool::WorkSignal::Signal() {
        std::lock_guard<std::mutex> lk{mux_};
        cond_var_.notify_one();
    }

    void WorkStealingThreadPool::WorkSignal::SignalAll() {
        std::lock_guard<std::mutex> lk{mux_};
        cond_var_.notify_all();
    }

    bool WorkStealingThreadPool::WorkSignal::WaitWithTimeOut(uint64_t nanoseconds) {
        std::unique_lock<std::mutex> lk{mux_};
        cond_var_.wait_for(lk,std::chrono::nanoseconds(nanoseconds));
    }

    WorkStealingThreadPool::~WorkStealingThreadPool() {

    }

    void WorkStealingThreadPool::Run(std::function<void()> func) {
        Run(SlefDeleteingClosure::CreateSelfDeleteClosure(func));
    }

    void WorkStealingThreadPool::Prefork() {
        threadpool_impl_->PostFork();
    }

    WorkStealingThreadPool::WorkStealingThreadPool(size_t reserve_thread)
        :threadpool_impl_(std::make_shared<WorkStealingThreadPoolImpl>(reserve_thread))
    {
        threadpool_impl_->Start();
    }

    WorkStealingThreadPool::WorkStealingThreadPoolImpl::LifeGuard::LifeGuard(
            WorkStealingThreadPool::WorkStealingThreadPoolImpl *threadpool)
            :threadpool_impl_(threadpool), back_off_(crpc_core::BackOff::Options{})
            ,lifeguard_is_shut_down_(std::make_unique<crpc_core::Notification>()),
            lifeguard_should_shut_down_(std::make_unique<crpc_core::Notification>())
            {}

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::LifeGuard::Start() {
        lifeguard_running_.store(true);
        std::thread t{[](LifeGuard * ptr){
            ptr->LifeGuardMain();
        },this};
        t.detach();
    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::LifeGuard::LifeGuardMain() {
        while(true){

            if(threadpool_impl_->IsForking()){
                break;
            }

            if(threadpool_impl_->IsShutDown()){
                if(threadpool_impl_->IsQuit()){
                    break;
                }
                else{
                    lifeguard_should_shut_down_->WaitForNotificationWithTimeout(back_off_.NextAttemptTimeNanoCount());
                }
            }
            MaybeStartNewThread();
        }
        lifeguard_running_.store(false,std::memory_order_relaxed);
        lifeguard_is_shut_down_->Notify();
    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::LifeGuard::MaybeStartNewThread() {
        /// be attention to the forking state
        if(threadpool_impl_->IsForking()){
            return;
        }
        const auto living_thread_count = threadpool_impl_->GetLivingThreadCount()->Count();

        /// add this judgement to ensure we really need to start a new thread
        if(threadpool_impl_->GetBusyThreadCount()->Count() < living_thread_count){

            if(!threadpool_impl_->queue_.Empty()){
                threadpool_impl_->GetWorkSignal()->Signal();
                back_off_.Reset();
            }
            return;
        }

        uint64_t curr_nano_seconds = std::chrono::nanoseconds (std::chrono::steady_clock::now().time_since_epoch()).count();

        /// last start a thread not far
        /// we do not start a thread now
        if((curr_nano_seconds - threadpool_impl_->last_start_thread_.load()
                <= kTimeBetweenThrottledThreadStart.count())){
            back_off_.Reset();
            return;
        }

        threadpool_impl_->StartThread();
        back_off_.Reset();
    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::LifeGuard::BlockUtilShutDownAndReset() {
        lifeguard_should_shut_down_->Notify();

        while(lifeguard_running_.load(std::memory_order_relaxed)){
            lifeguard_is_shut_down_->WaitForNotification();
        }
        /// in the case of race condition
        lifeguard_is_shut_down_->WaitForNotification();
        back_off_.Reset();

        lifeguard_should_shut_down_ = std::make_unique<crpc_core::Notification>();
        lifeguard_is_shut_down_ = std::make_unique<crpc_core::Notification>();

    }


}