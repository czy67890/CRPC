#pragma once
#include<atomic>
#include <functional>
#include <inttypes.h>
#include <stddef.h>
namespace crpc_util{
    struct crpc_slice_refcount{
    public:
        static crpc_slice_refcount *NoopRefcount(){
            
        }
        
        using DesFunc = std::function<void(crpc_slice_refcount *)>;

        crpc_slice_refcount() = default;
        
        explicit crpc_slice_refcount(DesFunc desFunc)
            :desFunc_(desFunc)
        {

        }

        void Ref(){
             ref_.fetch_add(1,std::memory_order_relaxed);
             //TODO :: here must some log to record this
        }

        void Unref(){
            auto prev_count = ref_.fetch_sub(1,std::memory_order_relaxed);
            if(prev_count == 1){
                desFunc_(this);
            }
        }

        bool IsUniq(){
            return ref_.load(std::memory_order_relaxed) == 1;
        }
        
    private:
        std::atomic<size_t > ref_{1};
        DesFunc desFunc_;
    };


}

