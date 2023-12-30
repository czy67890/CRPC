#pragma once

#include <stddef.h>
#include <memory>
#include <functional>

#include "src/core/lib/event_engine/forkable.h"
#include "crpc/event_engine/event_engine.h"
namespace crpc_event_engine {


/// @brief  only a interface of ThreadPool 
/// not supply any implement
    class ThreadPool : public Forkable {
    public:

    ~ThreadPool() override = default;

    virtual void Quit() = 0;

    virtual void Run(std::function<void()> func) = 0;

    virtual void Run(EventEngine::Closure *closure) = 0;
    };



    std::shared_ptr<ThreadPool> MakeThreadPool(size_t reserve_thread);


}
