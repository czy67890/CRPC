//
// Created by chenzeyu on 2024/1/10.
//

#ifndef CZYSERVER_ORPHANABLE_H
#define CZYSERVER_ORPHANABLE_H
#include "non_copyable.h"
#include "core/lib/cprpp/ref_counted.h"

#include <memory>

namespace crpc_core{
    class Orphanable{
    public:

        virtual void Orphan() = 0;

    protected:
        Orphanable() = default;

        virtual ~Orphanable(){}
    };

    class OrphanableDelete{
    public:
        template<typename T>
        void operator()(T * p) const{
            p->Orphan();
        }
    };

    template<typename T,typename Deleter = OrphanableDelete>
    using OrphanablePtr = std::unique_ptr<T,Deleter>;

    template<typename T,typename ... Args>
    inline OrphanablePtr<T> MakeOrphanable(Args&& ... args){
        return std::make_unique<OrphanablePtr<T>>(std::forward<Args>(args)...);
    }


    template<typename Child,typename UnrefBehavior = crpc_core::UnrefDeleter>
    class InternallyRefCounted :public crpc_core::Orphanable{
    public:
        InternallyRefCounted(const InternallyRefCounted &rhs) = delete;

        InternallyRefCounted& operator=(const InternallyRefCounted &rhs) = delete;

    protected:
        template<typename T>
        friend class crpc_core::RefCountedPtr;

        explicit  InternallyRefCounted(intptr_t initial_refcount = 1)
                :refs_(initial_refcount)
        {
        }

        ~InternallyRefCounted() override = default;

        RefCountedPtr<Child> Ref(){
            return RefCountedPtr<Child> (static_cast<Child*>(this));
        }

        void Unref(){
            if(refs_.UnRef())[[unlikely]]{
                unref_behavior_(static_cast<Child*>(this));
            }
        }

        [[nodiscard]] RefCountedPtr<Child> RefIfNonzero(){
            return RefCountedPtr<Child>(refs_.RefIfNoneZero() ? static_cast<Child*>(this) : nullptr);
        }

    protected:

        void IncrementRefCount(){
            refs_.Ref();
        }


    private:

        crpc_core::RefCount refs_;
        [[no_unique_address]] UnrefBehavior unref_behavior_;
    };
}




#endif //CZY SERVER_ORPHANABLE_H
