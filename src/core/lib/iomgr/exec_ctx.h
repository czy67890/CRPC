//
// Created by chenzeyu on 2024/1/11.
//

#ifndef CZYSERVER_EXEC_CTX_H
#define CZYSERVER_EXEC_CTX_H

#include <optional>
#include "core/lib/iomgr/closure.h"
#include "core/lib/cprpp/timer.h"
#include "core/lib/cprpp/debug_location.h"
#include "core/lib/cprpp/fork.h"
#include "non_copyable.h"
#include "crpc/impl/crpc_types.h"

typedef struct crpc_combiner crpc_combiner;

namespace crpc_core{

    enum  ExecFlag{
        kIsFinished = 1,
        kResourceLoop = 2,
        kInternalThread = 4
    };

    static constexpr size_t kAppInternalThread = 1;

    class Combiner;

    class ExecCtx : public NonCopyable{
    public:
        ExecCtx()
            :flags_(ExecFlag::kIsFinished)
        {
            Fork::IncExecCtxCount();
            Set(this);
        }

        explicit ExecCtx(ExecFlag flag)
            :flags_(flag)
        {
            if( !(flags_ & kInternalThread)){
                Fork::IncExecCtxCount();
            }
            Set(this);
        }

        virtual ~ExecCtx(){
            flags_ |= kIsFinished;
            /// when have closure
            /// we run it at des constructor
            Flush();
            Set(last_exec_ctx_);
            if( ! (flags_ & kInternalThread)){
                Fork::DecExecCtxCount();
            }
        }

        struct CombinerData{
            Combiner *active_combiner;
            Combiner *last_combiner;
        };

        CombinerData& GetCombinerData(){
            return combiner_data_;
        }

        crpc_closure_list& ClosureList(){
            return closure_list_;
        }

        uintptr_t  Flags(){
            return flags_;
        }

        bool HasWork(){
            return combiner_data_.active_combiner != nullptr || !crpc_closure_list_empty(closure_list_);
        }

        bool Flush();

        bool IsReadyToFinish(){
            if((flags_ & kIsFinished) == 0){
                if(CheckReadyToFinish()){
                    flags_ |= kIsFinished;
                    return true;
                }
                return false;
            }
            else{
                return true;
            }
        }

        void SetReadyToFinishFlag(){
            flags_ |= kIsFinished;
        }

        TimePoint Now(){
            return crpc_core::Now();
        }

        void InvalidateNow(){
            time_cache_.reset();
        }

        void SetNowIomgrShutdown(){
            time_cache_.emplace(InfFuture());
        }

        void TestOnlySetNow(TimePoint now){
            time_cache_.emplace(now);
        }

        static ExecCtx* Get() {
            return exec_ctx_;
        }

        static void Run(const DebugLocation & debug_location,crpc_closure *closure,Status error);

        static void RunList(const DebugLocation & debug_location,crpc_closure_list* list);


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
        uintptr_t flags_;
        std::optional<TimePoint> time_cache_;
        static thread_local ExecCtx * exec_ctx_;
        /// the reason this can be safe is each thread run-stack
        /// is single and certain
        ExecCtx *last_exec_ctx_ {Get()};
    };

    class ApplicationCallbackExecCtx{
    public:
        ApplicationCallbackExecCtx(){
            Set(this,flags_);
        }

        explicit ApplicationCallbackExecCtx(uintptr_t f1)
            :flags_(f1)
        {
            Set(this,flags_);
        }

        ~ApplicationCallbackExecCtx(){
            if(Get() == this){
                while(head_ != nullptr){
                    auto f = head_;
                    head_ = f->internal_next;
                    if(f->internal_next == nullptr){
                        tail_ = nullptr;
                    }
                    (*f->functor_to_run)(f,f->internal_success);
                }
                callback_exec_ctx_ = nullptr;
                if(!(kInternalThread & flags_)){
                    Fork::DecExecCtxCount();
                }
            }
            else{
                assert(head_ == nullptr);
                assert(tail_ == nullptr);
            }
        }

        static ApplicationCallbackExecCtx* Get(){
            return callback_exec_ctx_;
        }

        static void Set(ApplicationCallbackExecCtx *exec_ctx,uintptr_t flags){
            if(Get() == nullptr){
                if(!(flags & kInternalThread)){
                    Fork::IncExecCtxCount();
                }
                callback_exec_ctx_ = exec_ctx;
            }
        }


        uintptr_t Flags(){
            return flags_;
        }

        static void Enqueue(crpc_completion_queue_functor* functor,bool is_success){
            functor->internal_success = is_success;
            functor->internal_next = nullptr;
            auto ctx = Get();
            if(ctx->head_ == nullptr){
                ctx->head_ = functor;
            }
            if(ctx->tail_ != nullptr){
                ctx->tail_->internal_next = functor;
            }
            ctx->tail_ = functor;
        }

        static bool Available(){
            return Get() != nullptr;
        }

    private:
        uintptr_t  flags_{0u};
        crpc_completion_queue_functor* head_{nullptr};
        crpc_completion_queue_functor* tail_{nullptr};
        static thread_local ApplicationCallbackExecCtx *callback_exec_ctx_;
    };

    template<typename F>
    void EnsureRunInExecCtx(F f){
        if(ExecCtx::Get() == nullptr){
            ApplicationCallbackExecCtx app_ctx;
            ExecCtx exec_ctx;
            f();
        }
        else{
            f();
        }
    }
}
#endif //CZYSERVER_EXEC_CTX_H