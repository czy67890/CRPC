//
// Created by czy on 2024/1/17.
//

#include "core/lib/iomgr/combiner.h"
static void combiner_exec(crpc_core::Combiner *lock,crpc_closure * cl,Status error){

}

crpc_core::Combiner * crpc_combiner_create(std::shared_ptr<crpc_event_engine::EventEngine> event_engine){

}


namespace crpc_core {

    Combiner::Combiner(std::shared_ptr<crpc_event_engine::EventEngine> event_engine)
        :event_engine(std::move(event_engine))
    {
        LOG_INFO<<"Combiner :"<<(void *)this<<"created";
    }

    Combiner::~Combiner(){
        LOG_INFO<<"Combiner :"<<(void *)this<<"destroyed";
    }

    void Combiner::Run(crpc_closure *closure, Status error){
        /// a read-modify_write operate
        auto last = state.fetch_add(kStateElemCountLowBit);
        LOG_INFO<<"Combiner :"<<(void *)this<<"crpc_combiner_execute closure = "<<(void*)(closure)<<"last = "<<(void *)(last);
        if(last == 1){
            /// we store this to
            init_exec_ctx_or_null.store(reinterpret_cast<uintptr_t>(ExecCtx::Get()));

        }
    }



    void Combiner::FinallyRun(crpc_closure *closure, Status){

    }

    void Combiner::ForceOffload(){

    }

}