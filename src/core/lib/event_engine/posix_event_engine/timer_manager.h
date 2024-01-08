//
// Created by czy on 2024/1/8.
//

#ifndef CZYSERVER_TIMER_MANAGER_H
#define CZYSERVER_TIMER_MANAGER_H
#include <mutex>
#include <condition_variable>
#include <optional>

#include "core/lib/event_engine/forkable.h"
#include "core/lib/event_engine/posix_event_engine/timer.h"
#include "core/lib/cprpp/notification.h"
#include "core/lib/event_engine/thread_pool/thread_pool.h"

namespace crpc_event_engine{
    class TimerManager final :public Forkable{

    public:
        explicit TimerManager(std::shared_ptr<ThreadPool> thread_pool);

        ~TimerManager() override;

        void TimerInit(Timer * timer,crpc_core::TimePoint dealline,EventEngine::Closure * closure);

        bool TimerCancel(Timer *timer);

        static bool IsTimerManagerThread();

        void ShutDown();

        void Prefork() override;

        void PostforkParent() override;

        void PostforkChild() override;

    private:
        class Host final : public TimerListHost{
        public:
            explicit Host(TimerManager *timer_manager)
                :timer_manager_(timer_manager)
            {}

            void Kick() override;

            crpc_core::TimePoint  Now() override;

        private:
            TimerManager * timer_manager_;
        };

        void RestartPostFork();

        void MainLoop();

        void RunSomeTimers(std::vector<EventEngine::Closure*> funcs);

        bool WaitUntil(crpc_core::TimePoint next);

        void Kick();

    private:


        std::mutex mu_;
        std::condition_variable cond_;
        Host host_;

        bool shut_down_{false};
        bool kicked_ {false};

        uint64_t  wake_ups_{0};
        std::unique_ptr<TimerList> timer_list_;
        std::shared_ptr<ThreadPool> thread_pool_;
        std::optional<crpc_core::Notification> main_loop_exit_signal_;

    };

}


#endif //CZYSERVER_TIMER_MANAGER_H
