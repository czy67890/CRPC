//
// Created by czy on 2023/12/30.
//

#include <thread>
#include "core/lib/event_engine/thready_event_engine/thready_event_engine.h"

namespace crpc_event_engine {
    std::unique_ptr<ThreadyEventEngine::Listener> ThreadyEventEngine::CreateListener(AcceptCallback acc_cb,  crpc_function::AnyInvocable<void(Status)> on_shut_down,
                                             std::unique_ptr<MemoryAllocatorFactory> mem_factory) {

        /// impl method to seprate the functional with a
        /// and add the extension
        return impl_->CreateListener(
            [this,acc_cb = std::make_shared<AcceptCallback>(std::move(acc_cb))](std::unique_ptr<EndPoint> endpoint,MemoryAllocator mem_alloc)
            {
                Asynchronously([acc_cb,endpoint = std::move(endpoint),mem_alloc = std::move(mem_alloc)] () mutable {
                    (*acc_cb)(std::move(endpoint),std::move(mem_alloc));
                });
            },
            [this,on_shut_down = std::move(on_shut_down)](Status status)mutable{
                Asynchronously([on_shut_down = std::move(on_shut_down),status = std::move(status)]() mutable{
                    on_shut_down(std::move(status));
                });
            },std::move(mem_factory)
        );
    }

    ThreadyEventEngine::ConnectionHandle ThreadyEventEngine::Connect(ThreadyEventEngine::OnConectionCallBack on_connect,
                                                                     const ThreadyEventEngine::ResolvedAddr &addr,
                                                                     const EndPointConfig &config,
                                                                        MemoryAllocator alloc,
                                                                        ThreadyEventEngine:: Duration timeout){
        return impl_->Connect([this,on_connect = std::move(on_connect)](std::unique_ptr<EndPoint> c)  mutable{
            Asynchronously([on_connect = std::move(on_connect), c = std::move(c)]() mutable{
                on_connect(std::move(c));
            });
        },addr,config,std::move(alloc),timeout);
    }


    Status ThreadyEventEngine::CancelConnect(ThreadyEventEngine::ConnectionHandle handle) {
        return impl_->CancelConnect(handle);
    }


    bool ThreadyEventEngine::IsWorkThread() {
        assert(false);
    }

    std::unique_ptr<ThreadyEventEngine::DnsResolver> ThreadyEventEngine::GetDNSResolver(const DnsResolver::ResolverOption &options) {
        return std::make_unique<ThreadyDNSResolver>(impl_->GetDNSResolver(options));
    }

    void ThreadyEventEngine::Run(ThreadyEventEngine::Closure *closure) {
        Run([closure](){
            closure->Run();
        });
    }

    void ThreadyEventEngine::Run( crpc_function::AnyInvocable<void()> func) {
        Asynchronously(std::move(func));
    }

    ThreadyEventEngine::TaskHandle ThreadyEventEngine::RunAfter(Duration when, Closure *closure) {
        return RunAfter(when,[closure](){
            closure->Run();
        });
    }

    ThreadyEventEngine::TaskHandle ThreadyEventEngine::RunAfter(Duration when, crpc_function::AnyInvocable<void()> func) {
        return impl_->RunAfter(when,[this,func  = std::move(func)]() mutable{
            Asynchronously(std::move(func));
        });
    }

    bool ThreadyEventEngine::Cancel(TaskHandle handle) {
        return impl_->Cancel(handle);
    }

    void ThreadyEventEngine:: Asynchronously( crpc_function::AnyInvocable<void()> fn){
        std::thread t{std::move(fn)};
        t.detach();
    }

    
    void ThreadyEventEngine::ThreadyDNSResolver::LookupHostname(LookupHostnameCallback on_lookup,
                                                                std::string_view name,
                                                                std::string_view default_port) {
        return impl_->LookupHostname([this,on_lookup = std::move(on_lookup)](std::vector<ResolvedAddr> addr) mutable{
            engine_->Asynchronously([on_lookup = std::move(on_lookup),
                                        addr = std::move(addr)]()mutable{
                on_lookup(std::move(addr));
            });
        },name,default_port);
    }

    void ThreadyEventEngine::ThreadyDNSResolver::LookupSRV(LookupSRVRecordCallback on_lookup,std::string_view name) {
        return impl_->LookupSRV([this,on_lookup = std::move(on_lookup)](std::vector<SRVRecord> records)mutable{
            engine_->Asynchronously([on_lookup = std::move(on_lookup),records = std::move(records)]()mutable{
                on_lookup(std::move(records));
            });
        },name);
    }

    void ThreadyEventEngine::ThreadyDNSResolver::LookupTXT(LookupTXTCallback on_lookup,std::string_view name) {
        return impl_->LookupTXT([this,on_lookup = std::move(on_lookup)](std::vector<std::string> record)    mutable{
            return engine_->Asynchronously([on_lookup = std::move(on_lookup),record = std::move(record)]()
            mutable
            {
                on_lookup(std::move(record));
            });
        },name);
    }



}