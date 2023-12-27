//
// Created by czy on 2023/12/26.
//

#include "work_stealing_thread_pool.h"
namespace crpc_event_engine{
    void WorkStealingThreadPool::TheftReistry::Enroll(WorkQueue *queue) {
        std::lock_guard<std::mutex> lk{mu_};
        queues_.insert(queue);
    }

    void WorkStealingThreadPool::TheftReistry::UnEnroll(WorkQueue *queue) {
        std::lock_guard<std::mutex> lk{mu_};
        queues_.erase(queue);
    }

    EventEngine::Closure *WorkStealingThreadPool::TheftReistry::StealOne() {
        EventEngine::Closure *closure;
        std::lock_guard<std::mutex> lk{mu_};
        for(auto *queue:queues_){
            closure = queue->PopMostRecent();
            if(closure){
                return closure;
            }
        }
        return nullptr;
    }


}