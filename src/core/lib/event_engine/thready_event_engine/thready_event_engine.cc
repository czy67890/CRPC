//
// Created by czy on 2023/12/30.
//

#include <thread>
#include "core/lib/event_engine/thready_event_engine/thready_event_engine.h"

namespace crpc_event_engine {
    std::unique_ptr<ThreadyEventEngine::Listener> ThreadyEventEngine::CreateListener(AcceptCallback acc_cb, std::function<void(Status)> on_shut_down,
                                             std::unique_ptr<MemoryAllocatorFactory> mem_factory) {
        return impl_->CreateListener(
        [this,on_accept = std::make_shared<AcceptCallback>(std::move(acc_cb))](std::unique_ptr<EndPoint> endpoint,MemoryAllocator mem_alloc){
            std::function<void()> cb = [std::move(endpoint)](){
            };
        },
        );
    }

    ThreadyEventEngine::ConnectionHandle Connect(ThreadyEventEngine::OnConectionCallBack on_accept, const ThreadyEventEngine::ResolvedAddr &addr, const EndPointConfig &config,
                             MemoryAllocator alloc,ThreadyEventEngine:: Duration timeout) {}


    Status ThreadyEventEngine::CancelConnect(ThreadyEventEngine::ConnectionHandle handle) {}


    bool ThreadyEventEngine::IsWorkThread() {}

    std::unique_ptr<ThreadyEventEngine::DnsResolver> ThreadyEventEngine::GetDNSResolver(const DnsResolver::ResolverOption &options) {

    }

    void ThreadyEventEngine::Run(ThreadyEventEngine::Closure *closure) {

    }

    void ThreadyEventEngine::Run(std::function<void()> func) {

    }

    ThreadyEventEngine::TaskHandle ThreadyEventEngine::RunAfter(Duration when, Closure *closure) {

    }

    ThreadyEventEngine::TaskHandle ThreadyEventEngine::RunAfter(Duration when, std::function<void()> func) {

    }

    bool ThreadyEventEngine::Cancel(TaskHandle handle) {

    }

    void ThreadyEventEngine:: Asynchronously(std::function<void()> fn){
        std::thread t{std::move(fn)};
        t.detach();
    }

    
    void ThreadyEventEngine::ThreadyDNSResolver::LookupHostname(LookupHostnameCallback on_lookup,std::string_view name,std::string_view default_port) {}

    void ThreadyEventEngine::ThreadyDNSResolver::LookupSRV(LookupSRVRecordCallback on_lookup,std::string_view name) {}

    void ThreadyEventEngine::ThreadyDNSResolver::LookupTXT(LookupTXTCallback on_lookup,std::string_view name) {}

}