//
// Created by czy on 2023/12/25.
//

#ifndef CZYSERVER_MEM_ALLOCATOR_H
#define CZYSERVER_MEM_ALLOCATOR_H
#include "crpc/event_engine/mem_request.h"
#include <memory>
#include "non_copyable.h"
#include "crpc/event_engine/internal/mem_allocatoe_impl.h"

namespace crpc_event_engine{

    class MemoryAllocator :public crpc_core::NonCopyable{

    public:
        explicit MemoryAllocator(std::shared_ptr<MemoryAllocatorImpl> impl)
            :allocator_(std::move(impl))
        {

        }

        ~MemoryAllocator()
        {
            if(allocator_ != nullptr){
                /// smater ptr no need to call reset
                /// but we need shutDown
                allocator_->ShutDown();
            }
        }

        /// we need define it explict
        MemoryAllocator(MemoryAllocator && rhs) = default;

        MemoryAllocator& operator=(MemoryAllocator&& rhs) = default;

        void Reset(){
            auto ptr = std::move(allocator_);
            if(ptr) {
                ptr->ShutDown();
            }
        }

        void Release(size_t n){
            allocator_->Release(n);
        }



    class Reservation
        :public crpc_core::NonCopyable
    {
    public:
        Reservation() = default;

        Reservation(Reservation &&rhs)  = default;

        Reservation& operator=( Reservation &&rhs) = default;

        ~Reservation() {
            if (allcator_) {
                allcator_->Release(mem_allocated_);
            }
        }

        size_t MemAllocated() const{
            return mem_allocated_;
        }

    private:
        friend class MemoryAllocator;

        Reservation(std::shared_ptr<MemoryAllocatorImpl > allo,size_t mem)
            : allcator_(std::move(allo)),mem_allocated_(mem)
        {

        }

        std::shared_ptr<MemoryAllocatorImpl> allcator_{nullptr};
        size_t mem_allocated_{0};
    };



        size_t Reserve(MemoryRequest req){
                return allocator_->Reserve(req);
        }


    Reservation MakeReservation(MemoryRequest req){
        return Reservation(allocator_, Reserve(req));
    }




    /// such a beautiful implement
    template <typename T,typename ...Args>

    std::enable_if<std::has_virtual_destructor_v<T>,T*>::type New(Args&& ... arg)
    {
        class Wrapper final:public  T{
        public:
            explicit Wrapper(std::shared_ptr<MemoryAllocatorImpl> allc,Args&& ...arg)
                :T(std::forward<Args>(arg)...),ptr(allc)
            {
            }

            ~Wrapper(){
                ptr->Release(sizeof(Wrapper));
            }

        private:
            std::shared_ptr<MemoryAllocatorImpl> ptr;
        };

        Release(sizeof(Wrapper));
        return new Wrapper(allocator_,std::forward(arg)...);
    }

    private:
        std::shared_ptr<MemoryAllocatorImpl> allocator_{nullptr};

    };


    template <typename T,typename ...Args>
    std::unique_ptr<T> make_unique(Args&&... args){
        return std::unique_ptr<T>(New<T,std::forward<Args>>(args)...);
    }



}

#endif //CZYSERVER_MEM_ALLOCATOR_H
