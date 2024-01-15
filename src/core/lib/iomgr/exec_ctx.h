//
// Created by chenzeyu on 2024/1/11.
//

#ifndef CZYSERVER_EXEC_CTX_H
#define CZYSERVER_EXEC_CTX_H

#include <optional>
#include "core/lib/iomgr/closure.h"
#include "core/lib/cprpp/timer.h"


typedef struct crpc_combiner crpc_combiner;

namespace crpc_core{

    enum class ExecFlag{
        kIsFinished = 1,
        kResourceLoop = 2,
        kInternalThread = 4
    };

    static constexpr size_t kInternalThread = 1;

    class Combiner;

    class ExecCtx {
    public:
        ExecCtx(){

        }

        struct CombinerData{
            Combiner *active_combiner;
            Combiner *last_combiner;
        };

        void SetNowIomgrShutdown(){
            time_cache_.emplace(InfFuture());
        }

        void TestOnlySetNow(TimePoint now){
            time_cache_.emplace(now);
        }

        static ExecCtx* Get() {
            return exec_ctx_;
        }

        static void Run(crpc_closure *closure,Status error);

        static void RunList(crpc_closure_list* list);


    protected:


        virtual bool CheckReadyToFinish(){
            return false;
        }


        static void operator delete(void *){
            abort();
        }
    private:
        static void Set(ExecCtx *ctx){
            exec_ctx_ = ctx;
        }

    private:
        crpc_closure_list closure_list_ = CRPC_CLOSURE_LIST_INIT;
        CombinerData combiner_data_ {nullptr,nullptr};
        ExecFlag flags_;
        std::optional<TimePoint> time_cache_;
        static thread_local ExecCtx * exec_ctx_;
        ExecCtx *last_exec_ctx_ = Get();
    };

}
#endif //CZYSERVER_EXEC_CTX_H
//uf