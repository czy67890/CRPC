//
// Created by czy on 2024/1/17.
//
#include "core/lib/iomgr/exec_ctx.h"
#include "crpc/support/cpplog.h"
#include "core/lib/iomgr/combiner.h"
#include <cassert>

static void exec_ctx_run(crpc_closure *closure){
#ifndef NDEBUG
    closure->scheduled = false;
    /// TODO ::complete the DEBUG
    LOG_DEBUG<<"running closure "<<closure;
#endif
    Status error = Status::kOk;
    closure->cb(closure->cb_arg,error);
#ifndef NDEBUG
    LOG_DEBUG<<"closure finished"<<'\n';
#endif
}

static void exec_ctx_sched(crpc_closure *closure){
    crpc_closure_list_append(&crpc_core::ExecCtx::Get()->ClosureList(),closure);
}




namespace crpc_core{
    void ExecCtx::Run(const DebugLocation & debug_location,crpc_closure *closure,Status error){
        (void)debug_location;
        if(!closure){
            return;
        }
        assert(!closure->scheduled);
        closure->scheduled = true;
        closure->file_initiated = debug_location.File();
        closure->line_initiated = debug_location.Line();
        closure->run = false;
        assert(closure->cb != nullptr);
        //TODO::error now be assigned as the integer
        // change it to a real obj and pointer to it
        closure->error_data.error = (intptr_t)(error);
        exec_ctx_sched(closure);
    }

    bool ExecCtx::Flush(){
        bool did_something = false;
        for(;;){
            if(!crpc_closure_list_empty(closure_list_)){
                crpc_closure  * c = closure_list_.head;
                closure_list_.head = closure_list_.tail = nullptr;
                while(c != nullptr){
                    auto next = c->next_data.next;
                    did_something = true;
                    exec_ctx_run(c);
                    c = next;
                }
            }
            else if(!crpc_combiner_continue_exec_ctx()){
                break;
            }
        }
        assert(combiner_data_.active_combiner == nullptr);
        return did_something;
    }

    void ExecCtx::RunList(const DebugLocation & debug_location,crpc_closure_list* list){
        (void)debug_location;
        crpc_closure *c = list->head;
        while(c){
            auto next = c->next_data.next;
            assert(!c->scheduled);
            c->scheduled = true;
            c->file_initiated = debug_location.File();
            c->line_initiated = debug_location.Line();
            c->run = false;
            assert(c->cb != nullptr);
            exec_ctx_sched(c);
            c = next;
        }
        list->head = list->tail = nullptr;
    }

    thread_local ExecCtx * ExecCtx::exec_ctx_ = nullptr;
    thread_local ApplicationCallbackExecCtx* ApplicationCallbackExecCtx::callback_exec_ctx_{nullptr};
}
