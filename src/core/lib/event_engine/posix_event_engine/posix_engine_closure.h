//
// Created by czy on 2024/1/8.
//

#ifndef CZYSERVER_POSIX_ENGINE_CLOSURE_H
#define CZYSERVER_POSIX_ENGINE_CLOSURE_H
#include "crpc/any_invocable.h"
#include "crpc/status.h"
#include "crpc/event_engine/event_engine.h"

#include <utility>


namespace crpc_event_engine{

    class PosixEventEngineClosure
                final :public crpc_event_engine::EventEngine::Closure
    {
    public:
        PosixEventEngineClosure() = default;

        explicit PosixEventEngineClosure(crpc_function::AnyInvocable<void(Status)> cb,
                                         bool is_permanent):is_permanent_(is_permanent),cb_(std::move(cb))
                                         ,status_(Status::kOk){
        }

        void Run() override{
            cb_(std::exchange(status_,Status::kOk));
            if(!is_permanent_){
                delete this;
            }
        }

        void SetStatus(Status status){
            status_ = status;
        }

        ~PosixEventEngineClosure() final = default;

        static PosixEventEngineClosure* ClosureToPermanent(crpc_function::AnyInvocable<void(Status)> func){
            return new PosixEventEngineClosure(std::move(func),true);
        }

    private:
        bool is_permanent_{false};

        crpc_function::AnyInvocable<void(Status)> cb_;

        Status status_;
    };

}


#endif //CZYSERVER_POSIX_ENGINE_CLOSURE_H
