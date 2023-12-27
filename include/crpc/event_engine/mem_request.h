//
// Created by czy on 2023/12/25.
//

#ifndef CZYSERVER_MEM_REQUEST_H
#define CZYSERVER_MEM_REQUEST_H
#include <stdint.h>
#include <algorithm>

#include <stdlib.h>
namespace crpc_event_engine{
    class MemoryRequest{
    public:

        MemoryRequest(size_t min,size_t max)
            :min_(min),max_(max)
        {
            min_ = std::min(MaxAllocated(),min_);
            max_ = std::min(MaxAllocated(),max_);
        }


        MemoryRequest(size_t n)
                :min_(n),max_(n)
            {
                min_ = std::min(MaxAllocated(),min_);
                max_ = std::min(MaxAllocated(),max_);
            }

        static constexpr size_t MaxAllocated(){
            /// the most biggest can reach 1 GB
            return 1024 * 1024 * 1024;
        }

        MemoryRequest Increas(size_t amount){
            return MemoryRequest(min_ + amount,max_ + amount);
        }


        size_t min() const {
            return min_;
        }

        size_t max() const {
            return max_;
        }

    private:
        size_t min_;
        size_t max_;
    };

}

#endif //CZYSERVER_MEM_REQUEST_H
