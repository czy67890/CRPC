//
// Created by chenzeyu on 2024/1/15.
//
#include "core/lib/cprpp/fork.h"
#include "core/lib/event_engine/thread_local.h"
#include <mutex>
#include <condition_variable>
namespace crpc_core{
namespace {

#define UNBLOCKED(n) ((n) +2)
#define BLOCKED(n) (n)

class ExecCtxState{
public:
    ExecCtxState()
        :fork_complete_(true)
    {
        blocked.store(UNBLOCKED(0),std::memory_order_release);
    }

    void IncExecCtxCount(){
        if(crpc_event_engine::ThreadLocal::GetIsEventEngineThreadLocal()){
            return;
        }
    }

private:
    bool fork_complete_;
    std::mutex mux_;
    std::condition_variable cond_;
    std::atomic<size_t> blocked;
};


}
}