//
// Created by chenzeyu on 2024/1/6.
//

#ifndef CZYSERVER_POSIX_ENGINE_H
#define CZYSERVER_POSIX_ENGINE_H
#include <mutex>
#include "core/lib/event_engine/posix_event_engine/posix_engine_closure.h"
#include "core/lib/event_engine/thread_pool/thread_pool.h"
#include "core/lib/event_engine/posix_event_engine/event_poller.h"

namespace crpc_event_engine{
    class AsyncConnect{

    private:
        std::mutex mux_;
        PosixEventEngineClosure * on_writable_{nullptr};
        EventEngine::OnConectionCallBack on_connect_;
        ThreadPool * executor_;
        EventEngine::TaskHandle alarm_handle_;
        int refs_{2};
        EventHandle *fd_;
        MemoryAllocator mem_alloc_;

    };



}

#endif //CZYSERVER_POSIX_ENGINE_H
