//
// Created by czy on 2023/12/26.
//

#ifndef CZYSERVER_WORK_STEALING_THREAD_POOL_H
#define CZYSERVER_WORK_STEALING_THREAD_POOL_H
#include <memory>
#include <mutex>
#include <unordered_set>

#include "core/lib/event_engine/thread_pool/thread_pool.h"
#include "crpc/event_engine/event_engine.h"


#include "core/lib/event_engine/work_queue/work_queue.h"
namespace crpc_event_engine{


    class WorkStealingThreadPool
        final :public ThreadPool
    {
    public:
        ~WorkStealingThreadPool() override;

        void Quit() = 0;

        void Run(std::function<void()> func) override;

        void Run(EventEngine::Closure* closure) override;

        void PrepFork() override;

        void PostforkParent() override;

        void PostforkChild() override;


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


        class WorkStealingThreadPoolImpl{
        public:
            void Quit();

            void Run(std::function<void()> func);

            void Run(EventEngine::Closure* closure);

            void Start();

            void StartThread();

            bool SetThrottled(bool throttle);

            void SetShutDown(bool is_shutdown);

            void PrePareFork();

            void PostFork();

            bool IsShutDown();

            bool IsForking();

            bool IsQuit();

            size_t ReserveThreads(){

            }

        private:
            const size_t reserve_threads_;

        };

        std::shared_ptr<WorkStealingThreadPoolImpl> threadpool_impl_;
    };
}


#endif //CZYSERVER_WORK_STEALING_THREAD_POOL_H
