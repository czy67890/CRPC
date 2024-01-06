//
// Created by chenzeyu on 2024/1/6.
//

#ifndef CZYSERVER_TIMER_HEAP_H
#define CZYSERVER_TIMER_HEAP_H
#include <vector>
#include <cstddef>

namespace crpc_event_engine{

    struct Timer;

    class TimerHeap{
    public:
        bool Add(Timer* timer);

        void Remove(Timer * timer);

        Timer* Top() const;

        void Pop();

        inline bool IsEmpty() const;

        const std::vector<Timer*> & TestOnlyGetTimers() const{
            return timers_;
        }

    private:
        void AdjustUpwards(size_t i,Timer * t);

        void AdjustDownwards(size_t i,Timer * t);

        void NoteChangedPriority(Timer* timer);

        std::vector<Timer*> timers_;
    };


}
#endif //CZYSERVER_TIMER_HEAP_H
