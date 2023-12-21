#pragma once
#include <memory>
#include <chrono>
#include <unistd.h>
#include <socket.h>
#include "crpc/non_copyable.h"
namespace crpc_event_engine{


class EventEngine :public std::enable_shared_from_this<EventEngine>
{
public:

    using Duration = std::chrono::nanoseconds;

    class Closure :public crpc_core::NonCopyable{
    public:
    Closure() = default;

    virtual  ~Closure() = default; 

    virtual void Run() = 0;

    };


    struct TaskHandle{
        /// @brief usually a point to the task
        /// 
        intptr_t key[2];
        static const TaskHandle kInvalid;
        friend operator == (const TaskHandle &lsh ,const TaskHandle &rhs);
        friend operator != (const TaskHandle &lsh,const TaskHandle &rhs);
    };


    using ConnectionHandle = TaskHandle;

    class ResolveAddr{
    public:
        static constexpr socklen_t kMaxSizeBytes = 128;

        ResolveAddr(const sockaddr *addr,socklen_t size);

        ResolveAddr() = default;

        ResolveAddr(const ResolveAddr &) = default;

        const struct sockaddr* address() const{
            return reinterpret_cast<const struct sockaddr*>(address_)
        }

    private:
        char address_[kMaxSizeBytes]{};
        socklen_t size_{0};
    };


};




}
