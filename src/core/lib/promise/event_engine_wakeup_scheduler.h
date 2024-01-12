//
// Created by chenzeyu on 2024/1/11.
//

#ifndef CZYSERVER_EVENT_ENGINE_WAKEUP_SCHEDULER_H
#define CZYSERVER_EVENT_ENGINE_WAKEUP_SCHEDULER_H

#include <memory>
#include <utility>
#include "crpc/event_engine/event_engine.h"

namespace crpc_core{
    class EventEngineWakeupScheduler{
    public:

        template<typename ActivityType>
        class BoundScheduler
            :public crpc_event_engine::EventEngine::Closure{
        protected:
            explicit BoundScheduler(){

            }

            BoundScheduler(const BoundScheduler &) = delete;
            BoundScheduler& operator=(const BoundScheduler &) = delete;

            void ScheduleWakeup(){
                event_engine_->Run(this);
            }

            void Run() final{

            }

        private:
            std::shared_ptr<crpc_event_engine::EventEngine> event_engine_;
        };

    private:
        std::shared_ptr<crpc_event_engine::EventEngine> event_engine_;
    };
}

#endif //CZYSERVER_EVENT_ENGINE_WAKEUP_SCHEDULER_H
//uf