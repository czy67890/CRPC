//
// Created by czy on 2024/1/17.
//

#include "core/lib/iomgr/combiner.h"
static void combiner_exec(crpc_core::Combiner *lock,crpc_closure * cl,Status error){

}

static void enqueue_finally(void *closure,Status error){
    auto c = static_cast<crpc_closure*>(closure);
    auto Combiner = reinterpret_cast<crpc_core::Combiner*>(c->error_data.scratch);
    c->error_data.scratch = 0;
    Combiner->FinallyRun(c,error);
}

static void move_next(){
    auto combiner =  crpc_core::ExecCtx::Get()->GetCombinerData().active_combiner;
    if(combiner){
        crpc_core::ExecCtx::Get()->GetCombinerData().active_combiner = combiner->next_combiner_on_this_exec_ctx;
    }
    else{
        crpc_core::ExecCtx::Get()->GetCombinerData().last_combiner = nullptr;
    }
}

static void queue_offload(crpc_core::Combiner *combiner){
    move_next();
    combiner->state.store(crpc_core::Combiner::kStateUnOrphaned);
    LOG_INFO<<"Combiner : "<<(void *)(combiner)<< "offload";
    combiner->event_engine->Run([combiner](){
        crpc_core::ApplicationCallbackExecCtx callback_exec_ctx;
        crpc_core::ExecCtx ctx{crpc_core::kInitState};
        combiner->PushLastOnExecCtx();
        ctx.Flush();
    });
}

static void really_destroy(crpc_core::Combiner *lock){
    LOG_INFO<<"Combiner : "<<(void *)lock<<" really destroy";
    delete lock;
}

bool crpc_combiner_continue_exec_ctx(){
    using namespace crpc_core;
    auto lock = ExecCtx::Get()->GetCombinerData().active_combiner;
    if(lock == nullptr){
        return false;
    }
    bool contented = lock->init_exec_ctx_or_null.load() == 0;
    ///TODO:: here can write more information
    LOG_INFO<<"Combiner :"<<(lock)<<"crpc_combiner_continue_ctx "
                                    "contended = "<<contented;

    if(contented && ExecCtx::Get()->IsReadyToFinish()){
        queue_offload(lock);
        return true;
    }
    if(!lock->time_to_exec_final || (lock->state.load() >> 1) > 1){
        MPSCQueue::Node * n = lock->queue.Pop();
        LOG_INFO<<"Combiner :"<<(void *)lock<<"n = "<<n;
        if(n == nullptr){
            queue_offload(lock);
            return true;
        }
        auto c = reinterpret_cast<crpc_closure*>(n);
        c->scheduled = false;
        Status error = static_cast<Status>(c->error_data.error);
        c->error_data.error = 0;
        c->cb(c->cb_arg,std::move(error));
    }
    else{
        auto c = lock->final_list.head;
        assert(c != nullptr);
        crpc_closure_list_init(&lock->final_list);
        int loops = 0;
        while(c != nullptr){
            LOG_INFO<<"Combiner : "<<(void *)lock<<"execute_final";
            auto next = c->next_data.next;
            c->scheduled = false;
            Status error = static_cast<Status>(c->error_data.error);
            c->error_data.error = 0;
            c->cb(c->cb_arg,std::move(error));
            c = next;
        }
    }

    move_next();
    lock->time_to_exec_final = false;
    auto old_state = lock->state.fetch_add(-crpc_core::Combiner::kStateElemCountLowBit);
    /// TODO ::here can log more information
    LOG_INFO<<"Combiner :"<<(void *)lock<<"finish state ";

#define OLD_STATE_WAS(orphaned,elem_count) \
(((orphaned ) ? 0 : crpc_core::Combiner::kStateUnOrphaned) | \
((elem_count)*crpc_core::Combiner::kStateElemCountLowBit))

    switch(old_state){
        default:
            break;
        case OLD_STATE_WAS(false,2):
        case OLD_STATE_WAS(true,2):
            if(!crpc_closure_list_empty(lock->final_list)){
                lock->time_to_exec_final = true;
            }
            break;
        case OLD_STATE_WAS(false,1):
            return true;
        case OLD_STATE_WAS(true,1):
            really_destroy(lock);

    }
    lock->PushLastOnExecCtx();
    return true;
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

    void Combiner::PushLastOnExecCtx(){
        next_combiner_on_this_exec_ctx = nullptr;
        auto &combiner_data = ExecCtx::Get()->GetCombinerData() ;
        /// link self to the ExecCtx's List
        if(!combiner_data.active_combiner){
            combiner_data.active_combiner = combiner_data.last_combiner = this;
        }
        else{
            combiner_data.last_combiner->next_combiner_on_this_exec_ctx = this;
            combiner_data.last_combiner = this;
        }
    }

    void Combiner::Run(crpc_closure *closure, Status error){
        /// a read-modify_write operate
        auto last = state.fetch_add(kStateElemCountLowBit);
        LOG_INFO<<"Combiner :"<<(void *)this<<"crpc_combiner_execute closure = "<<(void*)(closure)<<"last = "<<(void *)(last);
        if(last == 1){
            /// we store this to init_exec
            init_exec_ctx_or_null.store(reinterpret_cast<uintptr_t>(ExecCtx::Get()));
            PushLastOnExecCtx();
        }
        else{
            uintptr_t  initiator = init_exec_ctx_or_null.load();
            if(initiator != 0 && initiator != reinterpret_cast<uintptr_t> (ExecCtx::Get())){
                init_exec_ctx_or_null.store(0);
            }
        }
        closure->error_data.error = static_cast<uintptr_t>(error);
        /// this next_data is a union
        /// in this situation
        /// it  represent self node
        queue.Push(closure->next_data.mpscq_node.Get());
    }



    void Combiner::FinallyRun(crpc_closure *closure, Status error){

        LOG_INFO<<"Combiner :" <<(void*) (this) <<"closure :" <<(void *)(closure) << "ac:" <<(void *)(ExecCtx::Get()->GetCombinerData().active_combiner);
        if(ExecCtx::Get()->GetCombinerData().active_combiner != this){
            /// when current active combiner not self we queue again
            closure->next_data.scratch = reinterpret_cast<uintptr_t>(this);
            Run(CRPC_CLOSURE_CREATE(enqueue_finally,closure,nullptr),error);
            return;
        }
        if(crpc_closure_list_empty(final_list)){
            state.fetch_add(kStateElemCountLowBit);
        }
        crpc_closure_list_append(&final_list,closure,error);
    }

    void Combiner::ForceOffload(){
        init_exec_ctx_or_null.store(0);
        ExecCtx::Get()->SetReadyToFinishFlag();
    }

}