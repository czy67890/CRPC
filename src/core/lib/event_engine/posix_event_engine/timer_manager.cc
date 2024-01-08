//
// Created by czy on 2024/1/8.
//


#include "core/lib/event_engine/posix_event_engine/timer_manager.h"
#include "core/lib/cprpp/timer.h"
#include <memory>
#include <utility>
#include <cassert>

static thread_local bool g_timer_thread;

namespace crpc_event_engine{

    void TimerManager::Prefork(){
        ShutDown();
    }

    void TimerManager::PostforkParent() {
        RestartPostFork();
    }

    void TimerManager::PostforkChild() {
        RestartPostFork();
    }


    void TimerManager::RunSomeTimers(std::vector<EventEngine::Closure*> funcs){
        for(auto * timer_func: funcs){
            thread_pool_->Run(timer_func);
        }
    }

    bool TimerManager::WaitUntil(crpc_core::TimePoint next){
        std::unique_lock<std::mutex> lk{mu_};
        if(shut_down_){
            return false;
        }

        if(!kicked_){
            cond_.wait_for(lk,next - host_.Now());
            ++wake_ups_;
        }

        kicked_ = false;
        return true;
    }

    TimerManager::TimerManager(std::shared_ptr<ThreadPool> thread_pool)
        : host_(this), thread_pool_(std::move(thread_pool))
    {
        timer_list_ = std::make_unique<TimerList>(&host_);
        //// this will create the mainloop_signal here
        main_loop_exit_signal_.emplace();
        thread_pool_->Run([this](){
            MainLoop();
        });
    }

    void TimerManager::TimerInit(Timer * timer,crpc_core::TimePoint dealline,EventEngine::Closure * closure){
        timer_list_->TimerInit(timer,dealline,closure);
    }

    bool TimerManager::TimerCancel(Timer *timer){
        return timer_list_->TimerCancel(timer);
    }

    bool TimerManager::IsTimerManagerThread(){
        return g_timer_thread;
    }


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

    TimerManager::~TimerManager(){
        ShutDown();
    }


    void TimerManager::Kick(){
        std::unique_lock<std::mutex> lk{mu_};
        kicked_ = true;
        cond_.notify_one();
    }

    void TimerManager::ShutDown(){
        {
            std::unique_lock<std::mutex> lk{mu_};
            if(shut_down_){
                return;
            }
            shut_down_ = true;
            cond_.notify_one();

        }
        main_loop_exit_signal_->WaitForNotification();

    }

}