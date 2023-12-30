//
// Created by czy on 2023/12/30.
//
#include "core/lib/event_engine/thread_pool/thread_pool.h"
#include "core/lib/event_engine/forkable.h"
#include "core/lib/cprpp/no_destruct.h"
#include "core/lib/event_engine/thread_pool/work_stealing_thread_pool.h"


namespace crpc_event_engine{
    namespace {
        crpc_core::NoDestruct<ObjectGroupForkHandler> g_thread_pool_fork_manager;

        class ThreadPoolForkCallBackMethods{
        public:
            static void Prefork(){
                g_thread_pool_fork_manager->Prefork();
            }

            static void PostforkParent(){
                g_thread_pool_fork_manager->PostforkParent();
            }

            static void PostforkChild(){
                g_thread_pool_fork_manager->PostforkChild();
            }
        };
    }

    std::shared_ptr<ThreadPool> MakeThreadPool(size_t reserve_threads){
        auto thread_pool = std::make_shared<WorkStealingThreadPool>(reserve_threads);
        g_thread_pool_fork_manager->RegisterForkable(thread_pool,ThreadPoolForkCallBackMethods::Prefork,ThreadPoolForkCallBackMethods::PostforkParent
                                                    ,ThreadPoolForkCallBackMethods::PostforkChild
        );
        return thread_pool;
    }
}

