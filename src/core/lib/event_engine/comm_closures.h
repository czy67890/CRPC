//
// Created by czy on 2023/12/27.
//

#ifndef CZYSERVER_COMM_CLOSURES_H
#define CZYSERVER_COMM_CLOSURES_H
#include <functional>
#include "crpc/event_engine/event_engine.h"
#include "crpc/any_invocable.h"
namespace crpc_event_engine {
    class StdFuncClosure :
        public EventEngine::Closure{
    public:
        explicit StdFuncClosure( crpc_function::AnyInvocable<void()> cb)
            : innerFunc_(std::move(cb))
        {

        }

        void Run() override{
            innerFunc_();
        }



    private:
         crpc_function::AnyInvocable<void()> innerFunc_;
    };

    class SlefDeleteingClosure
        :public EventEngine::Closure
    {
    public:
        static EventEngine::Closure* CreateSelfDeleteClosure( crpc_function::AnyInvocable<void()> func, crpc_function::AnyInvocable<void()> dest =  crpc_function::AnyInvocable<void()>{})
        {
            return static_cast<EventEngine::Closure*> (new SlefDeleteingClosure(std::move(func),std::move(dest)));
        }

        ~SlefDeleteingClosure() override{

            if(dest_){
                dest_();
            }
        }

        void Run() override{
            if(func_) {
                func_();
            }
            delete this;
        }

    private:

        explicit SlefDeleteingClosure( crpc_function::AnyInvocable<void()> func, crpc_function::AnyInvocable<void()> dest =  crpc_function::AnyInvocable<void()>{})
            :func_(std::move(func)),dest_(std::move(dest))
        {

        }




         crpc_function::AnyInvocable<void()> func_;
        /// this func can be set ,when u need some action after run
         crpc_function::AnyInvocable<void()> dest_;
    };

}
#endif //CZYSERVER_COMM_CLOSURES_H
