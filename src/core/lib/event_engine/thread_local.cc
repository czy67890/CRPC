//
// Created by czy on 2023/12/28.
//
#include "core/lib/event_engine/thread_local.h"

namespace {
    thread_local bool g_is_event_engine{false};
}
namespace crpc_event_engine{
    void ThreadLocal::SetIsEventEngineThreadLocal(bool is){
        g_is_event_engine = is;
    }

    bool ThreadLocal::GetIsEventEngineThreadLocal(){
        return g_is_event_engine;
    }

}
