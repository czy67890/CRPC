//
// Created by czy on 2024/1/5.
//

#ifndef CZYSERVER_REF_COUNTED_H
#define CZYSERVER_REF_COUNTED_H

#include <atomic>
#include <cassert>
#include "core/lib/cprpp/ref_count_ptr.h"
#include "core/lib/cprpp/atomic_utils.h"

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
            //return value indicate need destruct or not
            return value_.fetch_sub(1,std::memory_order_acq_rel) == 1;
        }

        bool RefIfNoneZero(){
            return IncrementIfNonZero(&value_);
        }



    private:


        Value Get() const{
            return value_.load(std::memory_order_relaxed);
        }

        std::atomic<Value> value_{0};
    };

    class PolymorphicRefCount{
    public:
        ~PolymorphicRefCount() = default;
    };


    class NonPolymorphicRefCount{
    public:
        ~NonPolymorphicRefCount() = default;
    };


    struct UnrefDeleter{
        template <typename T>
        void operator()(T * p) const{
            delete p;
        }
    };


    struct UnrefNonDelete{
        template <typename T>
        void operator()(T */**/) const{
            ///dont do any thing
        }
    };

    struct UnrefCallDtor{
        template <typename T>
        void operator()(T *p) const {
            p->~T();
        }
    };

    template <typename Child,typename Impl = PolymorphicRefCount,typename UnrefBehavior = UnrefDeleter>
    class RefCounted
        :public Impl
    {
    public:
        using RefCountedChildType = Child;

        ~RefCounted() = default;

        [[nodiscard]] RefCountedPtr<Child> Ref() {
            /// down_cast is safe
            return RefCountedPtr<Child> (static_cast<Child*>(this));
        }

        [[nodiscard]] RefCountedPtr<const Child> Ref() const{
            return RefCountedPtr<const Child>(static_cast<const Child*>(this));
        }

        void UnRef() const{
            if(refs_.UnRef())[[unlikely]]{
                unref_behaviour_(static_cast<Child *>(this));
            }
        }

        [[nodiscard]] RefCountedPtr<Child> RefIfNonZero(){
            return RefCountedPtr<Child> (refs_.RefIfNoneZero() ? static_cast<Child *>(this) : nullptr);
        }

        [[nodiscard]] RefCountedPtr<const Child> RefIfNonZero() const{
            return RefCountedPtr<const Child> (refs_.RefIfNoneZero() ? static_cast<const Child *>(this) : nullptr);
        }


        RefCounted(const RefCounted &rhs) = delete;

        RefCounted& operator = (RefCount &)  = delete;


    protected:
        explicit  RefCounted(intptr_t init_refcount = 1)
            :refs_(init_refcount)
        {

        }

        explicit RefCounted(UnrefBehavior b,intptr_t init_count = 1)
            :unref_behaviour_(b),refs_(init_count)
        {

        }

    private:
        template<typename T>
        friend class RefCountedPtr;

        void IncrementRefCount() const{
            refs_.Ref();
        }


        mutable RefCount refs_;

#if __cplusplus >= 202002L
    /// this will work on GNU tool
    /// when msvc not work
        [[no_unique_address]]
#endif
        UnrefBehavior unref_behaviour_;
    };


}

#endif //CZYSERVER_REF_COUNTED_H
