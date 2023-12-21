#pragma once

#include <stddef.h>
#include <memory>
#include <functional>

#include "src/core/lib/event_engine/forkable.h"
#include "crpc/event_engine/event_engine.h"
namespace crpc_event_engine{


/// @brief  only a interface of ThreadPool 
/// not suply any implement
class ThreadPool :public Forkable{
 public:
    
    ~ThreadPool() override = 0;

    virtual void Quit() = 0;

    virtual void Run(std::function<void()>) = 0;

    virtual void Run();

};


}

