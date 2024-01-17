//
// Created by chenzeyu on 2024/1/6.
//

#ifndef CZYSERVER_TIMERI_H
#define CZYSERVER_TIMERI_H
#include <chrono>

namespace crpc_core{
    using TimePoint = std::chrono::steady_clock::time_point;

    using Duration = std::chrono::steady_clock::duration;

    Duration MinTimeGap(){
        return Duration(1);
    }

    Duration FromSecondsAsDouble(const double seconds){
        return Duration(uint64_t (seconds * 1e9));
    }

    TimePoint FromNanoSecondsAfterEpoch(uint64_t nanoseconds){
        return TimePoint{} + Duration (nanoseconds);
    }

    constexpr TimePoint InfFuture(){
        return TimePoint {} + Duration (std::numeric_limits<uint64_t>::max());
    }

    TimePoint Now(){
        return std::chrono::steady_clock::now();
    }
}


#endif //CZYSERVER_TIMERI_H
