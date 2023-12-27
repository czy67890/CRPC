//
// Created by czy on 2023/12/27.
//

#ifndef CZYSERVER_COMM_CLOSURES_H
#define CZYSERVER_COMM_CLOSURES_H
#include <functional>
#include "crpc/event_engine/event_engine.h"
namespace crpc_event_engine {
    class StdFuncClosure :
        public EventEngine::Closure{
    public:
        explicit StdFuncClosure(std::function<void()> cb)
            : innerFunc_(std::move(cb))
        {

        }

        void Run() override{
            innerFunc_();
        }



    private:
        std::function<void()> innerFunc_;
    };

    class SlefDeleteingClosure
        :public EventEngine::Closure
    {
    public:
        static EventEngine::Closure* CreateSelfDeleteClosure(std::function<void()> func,std::function<void()> dest = std::function<void()>{})
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

        explicit SlefDeleteingClosure(std::function<void()> func,std::function<void()> dest = std::function<void()>{})
            :func_(std::move(func)),dest_(std::move(dest))
        {

        }




        std::function<void()> func_;
        /// this func can be set ,when u need some action after run
        std::function<void()> dest_;
    };

}
#endif //CZYSERVER_COMM_CLOSURES_H
