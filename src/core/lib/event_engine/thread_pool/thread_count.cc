//
// Created by czy on 2023/12/27.
//

#include "thread_count.h"
#include "crpc_log.h"
namespace crpc_event_engine{
    constexpr uint64_t kNanoSeocndsPerSecond = 1e9;

    size_t LivingThreadCounter::WaitForCountChange(size_t desire_thread_count,
                                                   Duration timeout){

        auto deadline = std::chrono::system_clock::now() + timeout;
        std::unique_lock<std::mutex> lk{mux_};
        cond_var_.wait_until(lk,deadline,[this,desire_thread_count](){
            return living_thread_count_ == desire_thread_count;
        });
        return living_thread_count_;
    }

    void LivingThreadCounter::BlockUtilThreadCount(size_t thread_count){
        /// this func will block util
        constexpr Duration kLogTimeGap = Duration{3 * kNanoSeocndsPerSecond};
        while(true){
            auto curr_threads = WaitForCountChange(thread_count,kLogTimeGap);
            if(curr_threads == thread_count){
                break;
            }
            std::string thread_count_str = std::to_string(curr_threads);
            crpc_log::LogIns<<thread_count_str;
        }
    }

}