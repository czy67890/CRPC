#pragma once
#include <memory>
#include <chrono>
#include <unistd.h>
#include <functional>
#include <sys/socket.h>
#include "non_copyable.h"
#include "crpc/status.h"
#include "crpc/slice.h"
#include "crpc/event_engine/slice_buffer.h"
using crpc_util::SliceBuffer;
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
        intptr_t keys[2];
        static const TaskHandle kInvalid;
        friend bool operator == (const TaskHandle &lsh ,const TaskHandle &rhs);
        friend bool operator != (const TaskHandle &lsh,const TaskHandle &rhs);
    };


    using ConnectionHandle = TaskHandle;

    class ResolvedAddr{
    public:
        static constexpr socklen_t kMaxSizeBytes = 128;

        ResolvedAddr(const sockaddr *addr,socklen_t size);

        ResolvedAddr() = default;

        ResolvedAddr(const ResolvedAddr &) = default;

        const struct sockaddr* address() const{
            return reinterpret_cast<const struct sockaddr*>(address_);
        }

        size_t size()const{
            return size_;
        }

    private:
        char address_[kMaxSizeBytes]{};
        socklen_t size_{0};
    };

    class EndPoint{
    public:
        
        virtual ~EndPoint() = default;

        struct ReadArgs{
            int64_t read_hint_bytes;
        };

        virtual void Read(std::function<void(Status)> on_read,SliceBuffer *buffer,const ReadArgs *args) = 0;

        struct WriteArgs{

            void *sepcial{nullptr};

            size_t max_frame_size;

        };

        virtual void Write(std::function<void(Status)> on_Write,SliceBuffer *buffer,const WriteArgs *write_arg) = 0;

        virtual const ResolvedAddr& GetPeerAddr() const = 0;

        virtual const ResolvedAddr& GetLocalAddr() const = 0;

        virtual void * QueryExtension(std::string_view id){return nullptr;}



    };


};




}
