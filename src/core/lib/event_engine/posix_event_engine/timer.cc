//
// Created by chenzeyu on 2024/1/6.
//

#include "core/lib/event_engine/posix_event_engine/timer.h"
#include "core/lib/cprpp/timer.h"
#include "core/lib/event_engine/posix_event_engine/timer_heap.h"
#include "core/lib/cprpp/useful.h"
#include "hardware_value.h"

#include <functional>
#include <algorithm>
#include <atomic>
#include <limits>
#include <utility>



namespace crpc_event_engine{


    namespace {

        void ListJoin(Timer *head,Timer * timer){
            timer->next = head;
            timer->prev = head->prev;
            timer->next->prev = timer->prev->next = timer;
        }

        void ListRemove(Timer *timer){
            timer->next->prev = timer->prev;
            timer->prev->next = timer->next;
        }

    }


    static const size_t kInvalidHeapIndex = std::numeric_limits<size_t>::max();
    static const double kAddDeadlineScale = 0.33;
    static const double kMinQueueWindowDuration = 0.01;
    static const double kMaxQueueWindowDuration = 1.0;


    crpc_core::TimePoint TimerList::Shard::ComputeDeadLine() {
        return heap.IsEmpty() ? queue_dead_line_cap + crpc_core::MinTimeGap() :
                                crpc_core::FromNanoSecondsAfterEpoch(heap.Top()->deadline);
    }


    TimerList::Shard::Shard()
        : stats(1.0/kAddDeadlineScale,0.1,0.5)
    {

    }


    bool TimerList::Shard::RefillHeap(crpc_core::TimePoint now) {
        const double computed_deadline_gap  = stats.UpdateAverage() *kAddDeadlineScale;
        double deadline_in_range = crpc_core::Clamp(computed_deadline_gap,kMinQueueWindowDuration,kMaxQueueWindowDuration);

        Timer * timer,* next;

        queue_dead_line_cap = std::max(now,queue_dead_line_cap) + crpc_core::FromSecondsAsDouble(deadline_in_range);

        for(timer = list.next;timer != &list;timer = next){
            next = timer->next;
            auto timer_deadline = crpc_core::FromNanoSecondsAfterEpoch(timer->deadline);

            if(timer_deadline < queue_dead_line_cap){
                ListRemove(timer);
                heap.Add(timer);
            }
        }

        return !heap.IsEmpty();
    }


    Timer *TimerList::Shard::PopOne(crpc_core::TimePoint now) {
        Timer *timer;
        while(true){
            if(heap.IsEmpty()){
                /// we dont need the Refill .Because of the timer will never be filled util queue_dead_line
                if(now < queue_dead_line_cap){
                    return nullptr;
                }
                if(!RefillHeap(now)) return nullptr;
            }
            timer = heap.Top();
            auto timer_deadline = crpc_core::FromNanoSecondsAfterEpoch(timer->deadline);
            if(timer_deadline > now){
                return nullptr;
            }
            timer->pending = false;
            heap.Pop();
            return timer;
        }
    }


    void TimerList::Shard::PopTimers(crpc_core::TimePoint now, crpc_core::TimePoint& new_deadline,std::vector<EventEngine::Closure*> &out) {
        std::lock_guard<std::mutex> lk{mux};
        while(Timer *timer = PopOne(now)){
            out.push_back(timer->closure);
        }
        new_deadline = ComputeDeadLine();

    }

    TimerList::TimerList(TimerListHost *host)
        :host_(host),num_shards_(crpc_core::Clamp(crpc_hard_ware::SysConf::CpuNum(),(size_t)1,(size_t)32))
            , min_timer_(host_->Now().time_since_epoch().count()),
            shards_(new Shard[num_shards_]),
           shard_queue_(new Shard* [num_shards_])
    {

        //// Init the shard queue
        for(size_t i = 0;i < num_shards_;++i){
            Shard & shard = shards_[i];
            shard.queue_dead_line_cap = crpc_core::FromNanoSecondsAfterEpoch(min_timer_.load(std::memory_order_relaxed));
            shard.shard_queue_index = i;
            shard.list.next = shard.list.prev = &shard.list;
            shard.min_deadline = shard.ComputeDeadLine();
            shard_queue_[i] = &shard;
        }
    }

    void TimerList::TimerInit(Timer *timer, crpc_core::TimePoint deadline, EventEngine::Closure *closure) {
        bool is_first_timer = false;
        /// to make the load average as possible
        Shard* shard = shard_queue_[std::hash<Timer*>()(timer) % num_shards_];;
        timer->closure = closure;
        timer->deadline = deadline.time_since_epoch().count();

        {
            std::lock_guard<std::mutex> lk{shard->mux};
            timer->pending = true;
            crpc_core::TimePoint  now = host_->Now();

            if(deadline <= now){
                deadline = now;
            }

            shard->stats.AddSample(std::chrono::duration_cast<std::chrono::seconds>(deadline - now).count());
            if(deadline < shard->queue_dead_line_cap){
                is_first_timer = shard->heap.Add(timer);
            }
            else{
                timer->heap_index = kInvalidHeapIndex;
                ListJoin(&shard->list,timer);
            }
        }

        if(is_first_timer){
            std::lock_guard<std::mutex> lk{mux_};
            if(deadline < shard->min_deadline){
                crpc_core::TimePoint  old_min_deadline = shard_queue_[0]->min_deadline;
                shard->min_deadline = deadline;
                NoteDeadLineChange(shard);
                if(shard->shard_queue_index == 0 && deadline < old_min_deadline){
                    min_timer_.store(deadline.time_since_epoch().count(),std::memory_order_relaxed);
                    host_->Kick();
                }
            }
        }
    }

    void TimerList::NoteDeadLineChange(TimerList::Shard *shard) {
        ///we swap the shard to the proper place
        while(shard->shard_queue_index > 0 && shard->min_deadline < shard_queue_[shard->shard_queue_index - 1]->min_deadline){
            SwapAdjacentShardsInQueue(shard->shard_queue_index - 1);
        }

        while(shard->shard_queue_index < num_shards_ - 1 && shard->min_deadline > shard_queue_[shard->shard_queue_index + 1]->min_deadline){
            SwapAdjacentShardsInQueue(shard->shard_queue_index);
        }

    }

    void TimerList::SwapAdjacentShardsInQueue(uint32_t first_shard_queue_index) {

        Shard *temp = shard_queue_[first_shard_queue_index];
        shard_queue_[first_shard_queue_index] = shard_queue_[first_shard_queue_index + 1];
        shard_queue_[first_shard_queue_index + 1] = temp;
        shard_queue_[first_shard_queue_index]->shard_queue_index = first_shard_queue_index;
        shard_queue_[first_shard_queue_index + 1]->shard_queue_index = first_shard_queue_index + 1;

    }

    bool TimerList::TimerCancel(Timer *timer) {
        Shard *shard = shard_queue_[std::hash<Timer*>()(timer) % num_shards_];
        std::lock_guard<std::mutex> lk{shard->mux};

        if(timer->pending){
            timer->pending = true;
            if(timer->heap_index == kInvalidHeapIndex){
                ListRemove(timer);
            }
            else{
                shard->heap.Remove(timer);
            }

            return true;
        }
        /// this timer is already be cancel
        return false;
    }

    std::optional<std::vector<EventEngine::Closure *>> TimerList::TimerCheck(crpc_core::TimePoint *next) {
        crpc_core::TimePoint now = host_->Now();

         crpc_core::TimePoint min_timer = crpc_core::FromNanoSecondsAfterEpoch(min_timer_.load(std::memory_order_relaxed));

         if(now < min_timer){
             if(next != nullptr){
                *next = std::min(*next,min_timer);
             }
             return std::vector<EventEngine::Closure*>();
         }

         std::unique_lock<std::mutex> lk{checker_mu_,std::defer_lock};

         if(!lk.try_lock()){
             return std::nullopt;
         }
         std::vector<EventEngine::Closure*> res = FindExpiredTimers(now,next);

         return res;
    }

    std::vector<EventEngine::Closure *>
    TimerList::FindExpiredTimers(crpc_core::TimePoint now, crpc_core::TimePoint *next) {
        crpc_core::TimePoint  min_timer = crpc_core::FromNanoSecondsAfterEpoch(min_timer_.load(std::memory_order_relaxed));

        std::vector<EventEngine::Closure*> done;
        if(now < min_timer){
            if(next != nullptr){
                *next = std::min(*next,min_timer);

            }
            return done;
        }

        std::lock_guard<std::mutex> lk{mux_};

        while(shard_queue_[0]->min_deadline < now || (now != crpc_core::InfFuture() && shard_queue_[0]->min_deadline == now)){
            ///when pop we refresh the min timer
            crpc_core::TimePoint new_min_deadline;
            shard_queue_[0]->PopTimers(now,new_min_deadline,done);
            shard_queue_[0]->min_deadline = new_min_deadline;
            NoteDeadLineChange(shard_queue_[0]);
        }

        if(next){
            *next = min(*next,shard_queue_[0]->min_deadline);
        }

        min_timer_.store(shard_queue_[0]->min_deadline.time_since_epoch().count(),std::memory_order_relaxed);
        return done;
    }
}



