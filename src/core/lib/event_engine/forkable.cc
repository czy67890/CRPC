#include "src/core/lib/event_engine/forkable.h"

#include <algorithm>
#include <utility>
#include <vector>
#include "forkable.h"

namespace crpc_event_engine{

namespace {
    bool IsForkEnable(){
        // TODO : this write to dead ,change it
        static bool enabled = true;
        return enabled;
    }
}

}

void crpc_event_engine::
     ObjectGroupForkHandler::
     RegisterForkable(std::shared_ptr<Forkable> forkable, 
                      std::function<void(void)> prepare,
                      std::function<void(void)> parent, 
                      std::function<void(void)> child)
{
    forkables_.emplace_back(forkable);
    auto parentTrans = parent.target<void(void)>();
    auto prepareTrans = prepare.target<void(void)>();
    auto childTrans = child.target<void(void)>();
    ///when not reg ,we will enable trans
    if(!std::exchange(registered_,true)){
        pthread_atfork(prepareTrans,parentTrans,childTrans);
    } 

}

void crpc_event_engine::ObjectGroupForkHandler::PrepFork()
{
    if(IsForkEnable()){

        for(auto iter = forkables_.begin();
                iter != forkables_.end();){
            auto shared = iter->lock();
            if(shared){
                shared->PrepFork();
                ++iter;
            }
            else{
                // prev iter invalid
                iter = forkables_.erase(iter);
            }
        }
    }
}

void crpc_event_engine::ObjectGroupForkHandler::PostforkParent()
{
    if(IsForkEnable()){

        for(auto iter = forkables_.begin();
                iter != forkables_.end();){
            auto shared = iter->lock();
            if(shared){
                shared->PostforkParent();
                ++iter;
            }
            else{
                // prev iter invalid
                iter = forkables_.erase(iter);
            }
        }
    }
}

void crpc_event_engine::ObjectGroupForkHandler::PostforkChild()
{
    if(IsForkEnable()){

        for(auto iter = forkables_.begin();
                iter != forkables_.end();){
            auto shared = iter->lock();
            if(shared){
                shared->PostforkChild();
                ++iter;
            }
            else{
                // prev iter invalid
                iter = forkables_.erase(iter);
            }
        }
    }
}
