//
// Created by czy on 2023/12/27.
//

#ifndef CZYSERVER_THREAD_COUNT_H
#define CZYSERVER_THREAD_COUNT_H
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>
#include <atomic>
#include <vector>
#include <thread>
#include <condition_variable>
#include <chrono>


#include "hardware_value.h"

#include "non_copyable.h"





namespace crpc_event_engine{
    using Duration = std::chrono::nanoseconds;
    class BusyThreadCount{
    public:

    class AutoThreadCounter :public crpc_core::NonCopyable{
    public:
        AutoThreadCounter(BusyThreadCount *counter,size_t idx)
        :counter_(counter),idx_(idx){
            counter_->Increment(idx_);
        }

        ~AutoThreadCounter(){
            if(counter_){
                counter_->Decrement(idx_);
            }
        }

        AutoThreadCounter(AutoThreadCounter&& rhs) noexcept{
            counter_ = std::exchange(rhs.counter_,nullptr);
            idx_ = rhs.idx_;
        }

        AutoThreadCounter& operator=(AutoThreadCounter &&rhs)noexcept{
            counter_ = std::exchange(rhs.counter_,nullptr);
            idx_ = rhs.idx_;
            return *this;
        }

    private:
        BusyThreadCount* counter_;
        size_t idx_;
    };

        BusyThreadCount()
            :busy_counts_(std::thread::hardware_concurrency()){}




        AutoThreadCounter MakeAutoThreadCounter(size_t idx){
            return AutoThreadCounter{this,idx};
        }


        /// fill this to avoid false sharing
        struct alignas(GetCacheLineSize()) CacheFilled{
            std::atomic<int> busy_count{0};
        };

        void Increment(size_t idx){
            busy_counts_[idx].busy_count.fetch_add(1,std::memory_order_relaxed);
        }

        void Decrement(size_t idx){
            busy_counts_[idx].busy_count.fetch_sub(1,std::memory_order_relaxed);
        }

        size_t Count() const{
            return (size_t) std::accumulate(busy_counts_.begin(),busy_counts_.end(),(size_t)0,[](size_t total,const CacheFilled &cacheFilled){
                return total + cacheFilled.busy_count.load(std::memory_order_relaxed);
            });
        }

        size_t NextIndex(){
            return (next_idx_.fetch_add(1) )% busy_counts_.size();
        }

    private:
        std::vector<CacheFilled> busy_counts_;
        std::atomic<size_t> next_idx_{0};
    };

    class LivingThreadCounter{
    public:

        class AutoThreadCounter :public crpc_core::NonCopyable{
        public:
            explicit AutoThreadCounter(LivingThreadCounter *counter)
            :counter_(counter){
                counter_->Increment();
            }

            ~AutoThreadCounter(){
                if(counter_){
                    counter_->Decrement();
                }
            }

            AutoThreadCounter(AutoThreadCounter&& rhs) noexcept{
                counter_ = std::exchange(rhs.counter_,nullptr);
            }

            AutoThreadCounter& operator=(AutoThreadCounter &&rhs)noexcept{
                counter_ = std::exchange(rhs.counter_,nullptr);
                return *this;
            }

        private:
            LivingThreadCounter* counter_;
        };

        AutoThreadCounter MakeAutoThreadCounter() {
            return AutoThreadCounter(this);
        }

        void BlockUtilThreadCount(size_t thread_count);

        size_t Count() const{
            std::lock_guard<std::mutex> lk{mux_};
            return CountUnLocked();
        }

        void Increment(){
            std::lock_guard<std::mutex> lk{mux_};
            ++living_thread_count_;
            cond_var_.notify_all();
        }

        void Decrement(){
            std::lock_guard<std::mutex> lk{mux_};
            --living_thread_count_;
            cond_var_.notify_all();
        }



    private:
        /// this is a way to implement
        /// lock when we need the lock version and the unlock version
        size_t CountUnLocked() const{
            return living_thread_count_;
        }

        size_t WaitForCountChange(size_t desire_thread_count,Duration timeout);

        size_t living_thread_count_;
        mutable std::mutex mux_;
        std::condition_variable cond_var_;
    };

}
#endif //CZYSERVER_THREAD_COUNT_H
