//
// Created by czy on 2023/12/26.
//

#ifndef CZYSERVER_BASIC_WORK_QUEUE_H
#define CZYSERVER_BASIC_WORK_QUEUE_H
#include "core/lib/event_engine/work_queue/work_queue.h"
#include "crpc/event_engine/event_engine.h"
#include <deque>
#include <condition_variable>


/// this is  a dqueue that when empty only return nullptr,and never block

namespace crpc_event_engine{
    class BasicWorkQueue :
        public WorkQueue
    {
    public:
        BasicWorkQueue()
            :owner_(nullptr)
        {

        }

        explicit BasicWorkQueue(void *owner)
            :owner_(owner)
        {

        }

        bool Empty() const override;

        size_t Size() const override;

        EventEngine::Closure * PopMostRecent() override;

        EventEngine::Closure * PopMostOld() override;

        void Add(EventEngine::Closure *task) override;

        void Add(crpc_function::AnyInvocable<void()> func) override;

        void* owner() override{
            return owner_;
        }
    private:
        void *owner_;
        mutable std::mutex mux_;
        std::deque<EventEngine::Closure *> que_;
    };
}

#endif //CZYSERVER_BASIC_WORK_QUEUE_H
