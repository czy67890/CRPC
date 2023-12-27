//
// Created by czy on 2023/12/26.
//

#include "basic_work_queue.h"
#include "core/lib/event_engine/comm_closures.h"
namespace crpc_event_engine{
    bool BasicWorkQueue::Empty() const{
        std::lock_guard<std::mutex> lk(mux_);
        return que_.empty();
    }


    size_t BasicWorkQueue::Size() const{
        std::lock_guard<std::mutex> lk{mux_};
        return que_.size();
    }

    EventEngine::Closure *BasicWorkQueue::PopMostRecent(){
        std::lock_guard<std::mutex> lk{mux_};
        if(que_.empty()){
            return nullptr;
        }
        auto res = que_.back();
        que_.pop_back();
        return res;
    }


    EventEngine::Closure *BasicWorkQueue::PopMostOld(){
        std::lock_guard<std::mutex> lk{ mux_};
        if(que_.empty()){
            return nullptr;
        }
        auto res = que_.front();
        que_.pop_front();
        return res;
    }

    void BasicWorkQueue::Add(EventEngine::Closure *task){
        std::lock_guard<std::mutex> lk{mux_};
        que_.push_back(task);
    }


    void BasicWorkQueue::Add(std::function<void()> func){
        auto closure = SlefDeleteingClosure::CreateSelfDeleteClosure(std::move(func));
        std::lock_guard<std::mutex> lk{mux_};
        que_.push_back(closure);
    }


}