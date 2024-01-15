//
// Created by chenzeyu on 2024/1/15.
//

#ifndef CZYSERVER_FORK_H
#define CZYSERVER_FORK_H
#include <atomic>
#include <set>

namespace crpc_core{
    class Fork{
    public:
        typedef void (*child_postfork_func)(void);

        static void GlobalInit();

        static bool Enabled();

        static void IncExecCtxCount(){
            if(support_enabled_.load(std::memory_order_relaxed)) [[unlikely]]{
                DoIncExecCtxCount();
            }
        }

        static void DecExecCtxCount(){
            if(support_enabled_.load(std::memory_order_relaxed))[[unlikely]]{
                DoDecExecCtxCount();
            }
        }

        static const std::set<child_postfork_func> & GetResetChildPollingEngineFunc();
        static bool RegisterResetChildPollingEngineFunc(child_postfork_func reset_child_polling_engine);

        static bool BlockExecCtx();

        static void AllowExecCtx();

        static void IncThreadCount();

        static void DecThreadCount();

        static void AwaitThreads();

        static void Enable(bool enable);

    private:
        static void DoIncExecCtxCount();
        static void DoDecExecCtxCount();

        static std::atomic<bool> support_enabled_;
        static bool override_enabled_;
        static std::set<child_postfork_func>*  reset_child_polling_engine_;
    };
}


#endif //CZYSERVER_FORK_H
