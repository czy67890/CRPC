//
// Created by czy on 2024/1/8.
//

#ifndef CZYSERVER_EVENT_POLLER_H
#define CZYSERVER_EVENT_POLLER_H
#include "crpc/event_engine/event_engine.h"
#include "crpc/any_invocable.h"
#include "core/lib/event_engine/posix_event_engine/posix_engine_closure.h"
#include "core/lib/event_engine/poller.h"
#include "core/lib/event_engine/forkable.h"


#include <string_view>
#include <string>

namespace crpc_event_engine{


    class Scheduler{
    public:
        virtual ~Scheduler() = default;

        virtual void Run(EventEngine::Closure * closure) = 0;

        virtual void Run(crpc_function::AnyInvocable<void()> ) = 0;
    };

    class PosixEventPoller;

    class EventHandle{
    public:

        virtual int WrappedFd() = 0;

        virtual void OrphanHandle() = 0;

        virtual void ShutdownHandle(std::string_view why) = 0;

        virtual void NotifyOnRead(PosixEventEngineClosure * on_read) = 0;

        virtual void NotifyOnWrite(PosixEventEngineClosure * on_write) = 0;

        virtual void NotifyOnError(PosixEventPoller * on_error) = 0;

        virtual void SetReadable() = 0;

        virtual void SetWritable() = 0;

        virtual void SetError() = 0;

        virtual bool IsHandleShutDown() = 0;

        virtual PosixEventPoller* Poller() = 0;

        virtual ~EventHandle() = default;
    };

    class PosixEventPoller
        :public Poller,public Forkable
    {
    public:
        virtual EventHandle * CreateHandle(int fd,std::string_view name,bool track_err) = 0;

        virtual bool CanTrackErrors() const = 0;

        virtual std::string Name() = 0;

        virtual void ShutDown() = 0;

        ~PosixEventPoller() = default;
    };

}
#endif //CZYSERVER_EVENT_POLLER_H
