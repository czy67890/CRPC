//
// Created by czy on 2024/1/8.
//

#ifndef CZYSERVER_POLLER_H
#define CZYSERVER_POLLER_H

#include "crpc/event_engine/event_engine.h"

namespace crpc_event_engine{
    class Poller{
    public:
        enum class WorkResult{kOk,kDeadlineExceeded,kKicked};

        virtual ~Poller() = default;

        virtual WorkResult Work(EventEngine::Duration  timeout,
                                crpc_function::AnyInvocable<void(void)> schedule_poll_again) = 0;

        virtual void Kick() = 0;
    };

}
#endif //CZYSERVER_POLLER_H
