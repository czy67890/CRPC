//
// Created by czy on 2023/12/28.
//

#ifndef CZYSERVER_BACKOFF_H
#define CZYSERVER_BACKOFF_H
#include <chrono>

namespace crpc_core{


/// this class is a class that
/// implement the mechanism of
/// random and algorithm that tell how long to wait
/// now the value is set to dead because
/// of we not need the complex mechanism
/// TODO::We Need To Complete this class to a useful state
class BackOff{
public:



    /// we can adjust this to change the power of BackOff
    /// supply
    class Options;


    using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

    explicit BackOff(const Options& options){

    }

    BackOff(const BackOff &rhs) = default;

    class Options{
    public:
        explicit  Options() = default;


    };

    TimePoint NextAttemptTime(){
        return TimePoint{std::chrono::steady_clock::now() + std::chrono::seconds(3)};
    }

    uint64_t NextAttemptTimeNanoCount(){
        return  3 * kNanoSecondsPerSecond;
    }
    /// this function is to reset the backoff to the init state
    void Reset(){

    }


private:

    static constexpr uint32_t kNanoSecondsPerSecond = 1e9;

    Options op;

};



}
#endif //CZYSERVER_BACKOFF_H
