//
// Created by chenzeyu on 2024/1/6.
//

#ifndef CZYSERVER_TIMER_H
#define CZYSERVER_TIMER_H
#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>
#include <optional>
#include <chrono>
#include <mutex>


#include "core/lib/event_engine/posix_event_engine/timer_heap.h"
#include "non_copyable.h"
#include "crpc/event_engine/event_engine.h"
#include "core/lib/cprpp/timer.h"


#include "core/lib/cprpp/time_averaged_stats.h"

namespace crpc_event_engine{

    struct Timer{
        uint64_t deadline;
        size_t heap_index;
        bool pending;
        struct Timer * next;
        struct Timer *prev;
        EventEngine::Closure* closure;
        crpc_event_engine::EventEngine::TaskHandle task_handle;
    };


    class TimerListHost{
    public:

        virtual crpc_core::TimePoint Now() = 0;

        virtual void Kick() = 0;

    protected:

        ~TimerListHost() = default;
    };

    class TimerList :public crpc_core::NonCopyable{

    public:

        explicit TimerList(TimerListHost *host);

        void TimerInit(Timer * timer,crpc_core::TimePoint deadline,EventEngine::Closure *closure);

        [[nodiscard]] bool TimerCancel(Timer *timer);

        std::optional<std::vector<EventEngine::Closure*>> TimerCheck(crpc_core::TimePoint *next);


    private:



        struct Shard{
            Shard();


            crpc_core::TimePoint ComputeDeadLine();

            bool RefillHeap(crpc_core::TimePoint now);

            Timer* PopOne(crpc_core::TimePoint  now);

            [[nodiscard]] std::vector<EventEngine::Closure*> PopTimers(crpc_core::TimePoint now,crpc_core::TimePoint &new_deadline);

            std::mutex mux;
            crpc_core::TimeAveragedStats stats;
            crpc_core::TimePoint queue_dead_line_cap;
            crpc_core::TimePoint min_deadline;
            uint32_t shard_queue_index ;
            TimerHeap heap;
            Timer list;
        };

        void NoteDeadLineChange(Shard* shard);

        void SwapAdjacentShardsInQueue(uint32_t first_shard_queue_index);


    private:
        std::vector<EventEngine::Closure*> FindExpiredTimers(crpc_core::TimePoint now,crpc_core::TimePoint * next);

    private:
        TimerListHost * const host_;
        const size_t num_shards_;
        std::mutex mux_;

        std::atomic<uint64_t> min_timer_;
        std::mutex checker_mu_;

        const std::unique_ptr<Shard[]> shards_;
        const std::unique_ptr<Shard*[]> shard_queue_;

    };

}



#endif //CZYSERVER_TIMER_H
