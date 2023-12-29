//
// Created by czy on 2023/12/26.
//

#include "work_stealing_thread_pool.h"
#include "core/lib/event_engine/thread_local.h"
namespace crpc_event_engine{
    namespace {
        thread_local WorkQueue* g_local_queue{nullptr};
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

    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::Run(std::function<void()> func) {

    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::Run(EventEngine::Closure *closure) {

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
        return false;
    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::SetShutDown(bool is_shutdown) {

    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::PrePareFork() {

    }

    void WorkStealingThreadPool::WorkStealingThreadPoolImpl::PostFork() {

    }

    bool WorkStealingThreadPool::WorkStealingThreadPoolImpl::IsShutDown() {
        return shutdown_.load(std::memory_order_relaxed);
    }

    bool WorkStealingThreadPool::WorkStealingThreadPoolImpl::IsForking() {
        return forking_.load(std::memory_order_relaxed);
    }

    bool WorkStealingThreadPool::WorkStealingThreadPoolImpl::IsQuit() {
        return false;
    }

    void WorkStealingThreadPool::ThreadState::ThreadBody() {
        g_local_queue = new BasicWorkQueue(threadpool_impl_.get());
        threadpool_impl_->GetTheftRegistry()->Enroll(g_local_queue);
        ThreadLocal::SetIsEventEngineThreadLocal(true);
        while(Step()){

        }
    }

    bool WorkStealingThreadPool::ThreadState::Step() {
       if(threadpool_impl_->IsForking()){
           return false;
       }
       auto *closure = g_local_queue->PopMostRecent();
       if(closure){
           /// such a beauty RAII design
            auto busy = threadpool_impl_->GetBusyThreadCount()->MakeAutoThreadCounter();
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

           bool time_out = threadpool_impl_->GetWorkSignal()->WaitWithTimeOut();

       }

    }
}