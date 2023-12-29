//
// Created by czy on 2023/12/28.
//

#ifndef CZYSERVER_NOTIFICATION_H
#define CZYSERVER_NOTIFICATION_H
#include <mutex>
#include <condition_variable>
#include <chrono>
namespace crpc_core{
    class Notification{
    public:

        void Notify(){
            std::unique_lock<std::mutex> lk{mux_};
            has_notified = true;
            cond_var_.notify_all();
        }

        bool WaitForNotificationWithTimeout(uint64_t nano_seconds_time_out){
            auto currTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::nanoseconds(nano_seconds_time_out);
            auto deadLine = currTime + duration;

            std::unique_lock<std::mutex> lk{mux_};
            cond_var_.wait_until(lk,deadLine,[this](){
                return has_notified;
            });

            return has_notified;
        }

        bool HasBeenNotified(){
            std::lock_guard<std::mutex> lk{mux_};
            return has_notified;
        }

        void WaitForNotification(){
            std::unique_lock<std::mutex> lk{mux_};
            cond_var_.wait(lk,[this](){
                return has_notified;
            });
        }

    private:
        std::mutex mux_;
        std::condition_variable cond_var_;
        bool has_notified{false};
    };

}
#endif //CZYSERVER_NOTIFICATION_H
