#pragma once
#include<atomic>
#include <functional>
#include <inttypes.h>
#include <string>
#include <stddef.h>
namespace crpc_util{
    struct CrpcSliceRefcount{
    public:
        static CrpcSliceRefcount *NoopRefcount(){
            
        }
        
        using DesFunc = std::function<void(CrpcSliceRefcount *)>;

        CrpcSliceRefcount() = default;
        
        explicit CrpcSliceRefcount(DesFunc desFunc)
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


    /// @brief  a class with the string stroed and manager it
    class MovedCppStringSliceCount
        :public CrpcSliceRefcount
    {   
    public:
        explicit MovedCppStringSliceCount(std::string && s)
            :desFunc_(destory),str_(std::move(s))
        {
        }

       uint8_t *data() {
            return reinterpret_cast<uint8_t*>(str_.data());
       }

        size_t size() const{
            return str_.length();
        }

    private:
         static void destory(CrpcSliceRefcount *p){
            delete static_cast<MovedCppStringSliceCount*>(p);
        }

        std::string str_; 
    };


}

