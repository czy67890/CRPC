//
// Created by czy on 2023/12/26.//

#ifndef CZYSERVER_WORK_STEALING_THREAD_POOL_H
#define CZYSERVER_WORK_STEALING_THREAD_POOL_H
#include <memory>
#include <mutex>
#include <unordered_set>


#include "core/lib/backoff/backoff.h"
#include "core/lib/event_engine/thread_pool/thread_pool.h"
#include "crpc/event_engine/event_engine.h"
#include "core/lib/cprpp/notification.h"
#include "core/lib/event_engine/thread_pool/thread_count.h"
#include "core/lib/event_engine/work_queue/work_queue.h"
#include "core/lib/event_engine/work_queue/basic_work_queue.h"


namespace crpc_event_engine{


    class WorkStealingThreadPool
        final :public ThreadPool
    {
    public:

        explicit WorkStealingThreadPool(size_t reserve_thread);
        ~WorkStealingThreadPool() override;

        void Quit()override{
            threadpool_impl_->Quit();
        }

        void Run(std::function<void()> func) override;

        void Run(EventEngine::Closure* closure) override{
            threadpool_impl_->Run(closure);
        }

        void Prefork() override;

        void PostforkParent() override{
            threadpool_impl_->PostFork();
        }

        void PostforkChild() override{
            threadpool_impl_->PostFork();
        }


    private:

        class TheftReistry{
        public:
            void Enroll(WorkQueue *queue);


            ////in order to support the method effectively
            //// we use the unordered_set as our lower data struct
            void UnEnroll(WorkQueue* queue);

            EventEngine::Closure *StealOne();


        private:
            std::mutex mu_;
            std::unordered_set<WorkQueue*> queues_;
        };

        class WorkSignal{
        public:
            void Signal();

            void SignalAll();

            bool WaitWithTimeOut(uint64_t nanoseconds);

        private:
            std::mutex mux_;
            std::condition_variable cond_var_;
        };

        class WorkStealingThreadPoolImpl
            :public std::enable_shared_from_this<WorkStealingThreadPoolImpl>
        {
        public:

            explicit  WorkStealingThreadPoolImpl(size_t reserve_thread);

            void Quit();


            void Run(EventEngine::Closure* closure);

            void Start();

            void StartThread();

            bool SetThrottled(bool throttle);

            void SetShutDown(bool is_shutdown);

            void SetForking(bool is_forking);

            void PrePareFork();

            void PostFork();

            bool IsShutDown();

            bool IsForking();

            bool IsQuit();

            size_t ReserveThreads() const{
                return reserve_threads_;
            }

            BusyThreadCount* GetBusyThreadCount();

            LivingThreadCounter *GetLivingThreadCount();

            TheftReistry * GetTheftRegistry();

            WorkQueue* GetWorkQueue(){
                return &queue_;
            }

            WorkSignal* GetWorkSignal(){
                return  &work_signal_;
            }


        private:

            /// this class provide the ability to scale the
            /// thread pool's thread num to match the workload

            class LifeGuard{
            public:
                explicit LifeGuard(WorkStealingThreadPoolImpl *threadpool);

                void Start();

                void BlockUtilShutDownAndReset();

            private:
                void LifeGuardMain();


                /// this func will create a new thread
                /// when the workload up to a
                /// unacceptable rate that we think
                /// is not acceptable
                void MaybeStartNewThread();

                std::unique_ptr<crpc_core::Notification> lifeguard_should_shut_down_;
                std::unique_ptr<crpc_core::Notification> lifeguard_is_shut_down_;
                std::atomic<bool> lifeguard_running_{false};
                WorkStealingThreadPoolImpl * threadpool_impl_;
                crpc_core::BackOff back_off_;
            };


        private:
            const size_t reserve_threads_;
            BusyThreadCount  busy_thread_count_;
            LivingThreadCounter living_thread_count_;
            TheftReistry theft_reistry_;
            BasicWorkQueue queue_;
            std::atomic<bool> shutdown_{false};
            std::atomic<bool> forking_{false};
            std::atomic<bool> quit_{false};
            std::atomic<uint64_t> last_start_thread_{0};
            std::atomic<bool> throttled_{false};
            WorkSignal work_signal_;
            LifeGuard life_guard_;
        };

        class ThreadState{
        public:
            explicit  ThreadState(std::shared_ptr<WorkStealingThreadPoolImpl> pool);

            void ThreadBody();

            void SleepIfRunning();

            bool Step();

            void FinishDraining();

        private:
            std::shared_ptr<WorkStealingThreadPoolImpl> threadpool_impl_;
            LivingThreadCounter::AutoThreadCounter auto_thread_counter_;
            size_t busy_count_index_;
            crpc_core::BackOff backoff_;
        };

        std::shared_ptr<WorkStealingThreadPoolImpl> threadpool_impl_;
    };
}


#endif //CZYSERVER_WORK_STEALING_THREAD_POOL_H
