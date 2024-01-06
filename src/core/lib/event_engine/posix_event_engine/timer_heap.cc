//
// Created by chenzeyu on 2024/1/6.
//

#include "core/lib/event_engine/posix_event_engine/timer_heap.h"
#include <cstdint>
#include "core/lib/event_engine/posix_event_engine/timer.h"


namespace crpc_event_engine{

    bool TimerHeap::IsEmpty() const{
        return timers_.empty();
    }

    Timer* TimerHeap::Top() const{
        return  timers_[0];
    }

    void TimerHeap::Pop(){
        Remove(Top());
    }

    bool TimerHeap::Add(Timer* timer){
        timer->heap_index = timers_.size();
        timers_.push_back(timer);
        AdjustUpwards(timer->heap_index,timer);
        return timer->heap_index == 0;
    }

    void TimerHeap::AdjustUpwards(size_t i,Timer * t){
        auto remember_timer = t;
        auto remember_index = i;
        while(i > 0){
            size_t parent = (i - 1) / 2;
            if(timers_[parent]->deadline <= remember_timer->deadline){
                break;
            }
            timers_[i] = timers_[parent];
            timers_[i]->heap_index = i;
            i = parent;
        }
        timers_[i] = remember_timer;
        remember_timer->heap_index = i;
    }

    void TimerHeap::AdjustDownwards(size_t i,Timer * t){
        size_t limit = timers_.size();
        auto remember_timer = timers_[i];
        while(i < limit){
            size_t son = i * 2 + 1;
            if(son >= limit){
                break;
            }
            if(son  + 1 < limit && timers_[son]->deadline > timers_[son + 1]->deadline){
                son++;
            }
            if(timers_[son]->deadline >= remember_timer->deadline){
                break;
            }
            timers_[i] = timers_[son];
            timers_[i]->heap_index = i;
            i = son;
        }
        timers_[i] = remember_timer;
        timers_[i]->heap_index = i;
    }

    void TimerHeap::Remove(Timer * timer){
        uint32_t i = timer->heap_index;
        if(i == timers_.size() - 1){
            timers_.pop_back();
            return;
        }
        timers_[i] = timers_[timers_.size() - 1];
        timers_[i]->heap_index = i;
        timers_.pop_back();
        NoteChangedPriority(timers_[i]);
    }

    void TimerHeap::NoteChangedPriority(Timer* timer){
        uint32_t i = timer->heap_index;
        uint32_t parent = static_cast<uint32_t>(  ((static_cast<int>(i ) - 1 )/2));
        if(timers_[parent]->deadline > timer->deadline){
            AdjustUpwards(i,timer);
        }
        else{
            AdjustDownwards(i,timer);
        }
    }

}