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
#include "hardware_value.h"

#include "non_copyable.h"

template<typename T>
class AutoThreadCounter :public crpc_core::NonCopyable{
public:
    AutoThreadCounter(T *counter,size_t idx)
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
    T*counter_;
    size_t idx_;
};




namespace crpc_event_engine{

    class BusyThreadCount{
    public:

        BusyThreadCount()
            :busy_counts_(std::thread::hardware_concurrency()){}


        using AutoThreadCounter = AutoThreadCounter<BusyThreadCount>;


        AutoThreadCounter MakeAutoThreadCounter(){
            size_t idx  = NextIndex();
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
            return std::accumulate(busy_counts_.begin(),busy_counts_.end(),0,[](size_t total,const CacheFilled &cacheFilled){
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
        using AutoThreadCounter = AutoThreadCounter<LivingThreadCounter>;

        

    };

}
#endif //CZYSERVER_THREAD_COUNT_H
