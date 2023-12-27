#pragma once
#include <memory>
#include <chrono>
#include <unistd.h>
#include <functional>
#include <vector>
#include <sys/socket.h>

#include "non_copyable.h"
#include "crpc/status.h"
#include "crpc/slice.h"
#include "crpc/event_engine/slice_buffer.h"
#include "crpc/event_engine/mem_allocator.h"
#include "crpc/event_engine/endpoint_condig.h"
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

            void *special{nullptr};

            size_t max_frame_size;

        };

        virtual void Write(std::function<void(Status)> on_Write,SliceBuffer *buffer,const WriteArgs *write_arg) = 0;

        virtual const ResolvedAddr& GetPeerAddr() const = 0;

        virtual const ResolvedAddr& GetLocalAddr() const = 0;

        virtual void * QueryExtension(std::string_view id){return nullptr;}
    };

    using OnConectionCallBack = std::function<void(std::unique_ptr<EndPoint>)>;
    using AcceptCallback = std::function<void(std::unique_ptr<EndPoint>,MemoryAllocator)>;

    class Listener{
    public:


        virtual ~Listener() = default;

        virtual int Bind(const ResolvedAddr& addr) = 0;

        //// this is the interface with the event engine
        virtual void Start() = 0;

    };

    /// what a listener need ,we guess that
    /// first , accept_callback
    /// second, what to do in shutdown
    /// last for the high performance network, we need a memory factory
    /// to manager the mem in use
    virtual std::unique_ptr<Listener>  CreateListener(AcceptCallback acc_cb,std::function<void(Status)> on_shut_down
                                                    ,std::unique_ptr<MemoryAllocatorFactory> mem_factory) = 0;

    virtual ConnectionHandle Connect(OnConectionCallBack on_accept,const ResolvedAddr &addr,const EndPointConfig &config,MemoryAllocator alloc,Duration timeout) = 0;


    virtual Status CancelConnect(ConnectionHandle handle) = 0;

    class DnsResolver{
    public:
        DnsResolver() = default;

        virtual ~DnsResolver() = default;

        struct ResolverOption{
            /// use sys dns if string empty
            /// otherwise must meet the ip:port format

            std::string dns_server;
        };

        struct SRVRecord{
            std::string host;
            size_t port;
            size_t priority;
            size_t weight;
        };

        using LookupHostnameCallback = std::function<void(std::vector<ResolvedAddr>)>;

        using LookupSRVRecordCallback = std::function<void(std::vector<SRVRecord>)>;

        using LookupTXTCallback = std::function<void(std::vector<std::string>)>;

        virtual void LookupHostname(LookupHostnameCallback on_lookup,std::string_view name,std::string_view default_port) = 0;

        virtual void LookupSRV(LookupSRVRecordCallback on_lookup,std::string_view name);

        virtual void LookupTXT(LookupTXTCallback on_lookup,std::string_view name);

    };

    virtual ~EventEngine() = default;

    virtual bool IsWorkThread() = 0;

    virtual std::unique_ptr<DnsResolver> GetDNSResolver(const DnsResolver::ResolverOption &options) = 0;

    virtual void Run(Closure *closure) = 0;

    virtual void  Run(std::function<void()> func) = 0;

    virtual TaskHandle RunAfter(Duration when,Closure *closure) = 0;

    virtual TaskHandle RunAfter(Duration when,std::function<void()> func) = 0;

    virtual bool Cancel(TaskHandle handle) = 0;


};





}
