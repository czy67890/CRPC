//
// Created by czy on 2024/1/17.
//

#ifndef CZYSERVER_COMBINER_H
#define CZYSERVER_COMBINER_H
#include <memory>
#include <atomic>
#include "crpc/event_engine/event_engine.h"
#include "core/lib/cprpp/mpscq.h"
#include "core/lib/iomgr/exec_ctx.h"



namespace crpc_core{


    class Combiner{
    public:
        enum {
            kStateUnOrphaned = 1,
            kStateElemCountLowBit = 2
        };

        Combiner(std::shared_ptr<crpc_event_engine::EventEngine> event_engine);

        ~Combiner();

        void Run(crpc_closure * closure,Status error);

        void FinallyRun(crpc_closure *closure,Status);

        void ForceOffload();


        void PushLastOnExecCtx();

        Combiner * next_combiner_on_this_exec_ctx {nullptr};

        MPSCQueue queue;
        std::atomic<uintptr_t> init_exec_ctx_or_null;
        std::atomic<uintptr_t> state{kStateUnOrphaned};
        bool time_to_exec_final {false};
        crpc_closure_list final_list {nullptr,nullptr};
        crpc_closure  offload;

        std::atomic<size_t> refs{1};
        std::shared_ptr<crpc_event_engine::EventEngine> event_engine;

    private:

    };

}


#ifndef NDEBUG
#define CRPC_COMBINER_DEBUG_ARGS \
  , const char *file, int line, const char *reason
#define CRPC_COMBINER_REF(combiner, reason) \
  crpc_combiner_ref((combiner), __FILE__, __LINE__, (reason))
#define GRPC_COMBINER_UNREF(combiner, reason) \
  crpc_combiner_unref((combiner), __FILE__, __LINE__, (reason))
#else
#define CRPC_COMBINER_DEBUG_ARGS
#define CRPC_COMBINER_REF(combiner, reason) crpc_combiner_ref((combiner))
#define CRPC_COMBINER_UNREF(combiner, reason) crpc_combiner_unref((combiner))
#endif



crpc_core::Combiner* crpc_combiner_ref(crpc_core::Combiner *lock CRPC_COMBINER_DEBUG_ARGS);

bool crpc_combiner_continue_exec_ctx();

#endif
// uf