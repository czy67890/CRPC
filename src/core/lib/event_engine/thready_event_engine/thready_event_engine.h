//
// Created by czy on 2023/12/30.
//

#ifndef CZYSERVER_THREADY_EVENT_ENGINE_H
#define CZYSERVER_THREADY_EVENT_ENGINE_H


#include <memory>
#include <utility>
#include <string_view>
#include "crpc/status.h"
#include "crpc/event_engine/endpoint_condig.h"
#include "crpc/event_engine/event_engine.h"
#include "crpc/event_engine/mem_allocator.h"

namespace crpc_event_engine{
    class ThreadyEventEngine final :
        public EventEngine
    {
    public:
        explicit  ThreadyEventEngine(std::shared_ptr<EventEngine> event_engine)
            :impl_(std::move(event_engine))
        {}

        std::unique_ptr<Listener>  CreateListener(AcceptCallback acc_cb, crpc_function::AnyInvocable<void(Status)> on_shut_down
        ,std::unique_ptr<MemoryAllocatorFactory> mem_factory) override;

        ConnectionHandle Connect(OnConectionCallBack on_accept,const ResolvedAddr &addr,const EndPointConfig &config,MemoryAllocator alloc,Duration timeout) override;


        Status CancelConnect(ConnectionHandle handle) override;

        ~ThreadyEventEngine() = default;

        bool IsWorkThread() override;

        std::unique_ptr<DnsResolver> GetDNSResolver(const DnsResolver::ResolverOption &options) override;

        void Run(Closure *closure) override;

        void  Run( crpc_function::AnyInvocable<void()> func) override;

        TaskHandle RunAfter(Duration when,Closure *closure) override;

        TaskHandle RunAfter(Duration when, crpc_function::AnyInvocable<void()> func) override;

        bool Cancel(TaskHandle handle) override;


    private:
        class ThreadyDNSResolver final :public DnsResolver{
        public:
            explicit ThreadyDNSResolver(std::unique_ptr<DnsResolver> impl)
                :impl_(std::move(impl))
            {
            }

            void LookupHostname(LookupHostnameCallback on_lookup,std::string_view name,std::string_view default_port) override;

            void LookupSRV(LookupSRVRecordCallback on_lookup,std::string_view name) override;

            void LookupTXT(LookupTXTCallback on_lookup,std::string_view name) override;

        private:
            std::unique_ptr<DnsResolver> impl_;
            ThreadyEventEngine *engine_;
        };

        void Asynchronously( crpc_function::AnyInvocable<void()> fn);

        std::shared_ptr<EventEngine> impl_;
    };
}








#endif //CZYSERVER_THREADY_EVENT_ENGINE_H
