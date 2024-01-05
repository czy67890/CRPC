//
// Created by czy on 2024/1/5.
//

#ifndef CZYSERVER_REF_COUNTED_H
#define CZYSERVER_REF_COUNTED_H

#include <atomic>
#include <cassert>

namespace crpc_core{
    class RefCount{
    public:
        using Value = intptr_t;

        explicit RefCount():
            RefCount(1)
        {

        }

        explicit RefCount(Value init)
            :value_(init){}


        void Ref(Value n = 1){
            value_.fetch_add(n,std::memory_order_relaxed);
        }

        void RefNonZero(){
            assert(value_.fetch_add(1,std::memory_order_relaxed) > 0);
        }

        bool UnRef(){
            return value_.fetch_sub(1,std::memory_order_acq_rel) == 1;
        }



    private:


        Value Get() const{
            return value_.load(std::memory_order_relaxed);
        }

        std::atomic<Value> value_{0};
    };

    class NonPolymorphicRefCount{
    public:
        ~NonPolymorphicRefCount() = default;
    };

    template <typename T>
    struct UnrefDeleter{
        void operator()(T * p) const{
            delete p;
        }
    };

    template <typename T>
    struct UnrefNonDelete{
        void operator()(T */**/) const{
            ///dont do any thing
        }
    };

    template <typename T>
    struct UnrefCallDtor{
        void operator()(T *p) const {
            p->~T();
        }
    };

    template <typename Child,typename Impl = >



}

#endif //CZYSERVER_REF_COUNTED_H
