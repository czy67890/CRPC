//
// Created by czy on 2023/12/26.
//

#ifndef CZYSERVER_WORK_QUEUE_H
#define CZYSERVER_WORK_QUEUE_H
#include <cstdlib>
#include "crpc/event_engine/event_engine.h"

namespace crpc_event_engine{
        class WorkQueue{
        public:
            virtual ~WorkQueue() = default;

            virtual bool Empty() const = 0;

            virtual size_t Size() const = 0;

            virtual EventEngine::Closure * PopMostRecent() = 0;

            virtual EventEngine::Closure * PopMostOld() = 0;

            virtual void Add(EventEngine::Closure *task) = 0;

            virtual void Add( crpc_function::AnyInvocable<void()> func) = 0;


            /// this is just a optional
            virtual const void *owner() = 0;

        };

}


#endif //CZYSERVER_WORK_QUEUE_H
