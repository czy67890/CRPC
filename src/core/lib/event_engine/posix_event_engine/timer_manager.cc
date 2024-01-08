//
// Created by czy on 2024/1/8.
//


#include "core/lib/event_engine/posix_event_engine/timer_manager.h"
#include "core/lib/cprpp/timer.h"
#include <memory>
#include <utility>
#include <cassert>

namespace crpc_event_engine{

    void TimerManager::MainLoop(){
        crpc_core::TimePoint next = crpc_core::InfFuture();
        auto check_result = timer_list_->TimerCheck(&next);
        /// when here raise a assert ,that present more than one loop is running
        assert(check_result.has_value());
        bool found_timers = !check_result->empty();

        if(found_timers){
            RunSomeTimers(*check_result);
        }
        thread_pool_->Run([this,next,found_timers](){
            if(!found_timers && !WaitUntil(next)){
                main_loop_exit_signal_->Notify();
                return;
            }
            MainLoop();
        });
    }

}