//
// Created by czy on 2024/1/5.
//

#ifndef CZYSERVER_REF_COUNT_PTR_H
#define CZYSERVER_REF_COUNT_PTR_H
#include <atomic>
#include <algorithm>
namespace crpc_core{
    template <typename T>
    class RefCountedPtr{
    public:

        RefCountedPtr(){}

        RefCountedPtr(std::nullptr_t ){}

        ~RefCountedPtr(){
            if(value_){
                value_->UnRef();
            }
        }

        template<typename Y>
        explicit RefCountedPtr(Y * value)
            :value_(static_cast<T *>(value)){}

        RefCountedPtr(RefCountedPtr && rhs) noexcept{
            value_ = (std::exchange(rhs.value_,nullptr));
        }

        template<typename Y>
        RefCountedPtr(RefCountedPtr<Y> &&rhs) noexcept{
            value_ = static_cast<T*>(std::exchange(rhs.value_,nullptr));
        }

        void swap(RefCountedPtr &rhs) noexcept{
            std::swap(rhs.value_,value_);
        }

        RefCountedPtr& operator=(RefCountedPtr&& rhs) noexcept{
            ///copy and swap item
            RefCountedPtr tmp{std::move(rhs)};
            swap(tmp);
        }

        template <typename Y>
        RefCountedPtr& operator=(RefCountedPtr<Y> &&rhs) noexcept{
            RefCountedPtr tmp{std::move(rhs)};
            swap(tmp);
        }

        RefCountedPtr (const RefCountedPtr &rhs){
            if(rhs.value_){
                rhs.value_->IncrementRefCount();
            }
            value_ = rhs.value_;
        }

        template <typename Y>
        RefCountedPtr(const RefCountedPtr<Y> &rhs){
            if(rhs.value_){
                rhs.value_->IncrementRefCount();
            }
            value_ = static_cast<T*>(rhs.value_);
        }

        RefCountedPtr& operator=(const RefCountedPtr &rhs){
            RefCountedPtr tmp{rhs};
            swap(tmp);
            return *this;
        }

        template<typename Y>
        RefCountedPtr &operator=(const RefCountedPtr<Y>&rhs){
            RefCountedPtr tmp{rhs};
            swap(tmp);
            return *this;
        }


        /// not use out of the range
        /// of crpc core
        T *release(){
            return std::exchange(value_,nullptr);
        }

        T*  get() const{
            return value_;
        }

        T & operator* () const {
            return *value_;
        }

        T * operator->() const{
            return value_;
        }

        template <typename Y>
        bool operator== (const Y * rhs)const{
            return value_ == rhs;
        }

        template <typename Y>
        bool operator == (const RefCountedPtr<Y> &rhs) const{
            return value_ == rhs.value_;
        }

        bool operator==(std::nullptr_t ) const{
            return value_ == nullptr;
        }

        template <typename Y>
        bool operator != (const Y *rhs) const{
            return !(this == rhs);
        }

        template<typename Y>
        bool operator  != (const RefCountedPtr<Y> &rhs) const{
            return !(this == rhs);
        }

        bool operator!=(std::nullptr_t) const { return value_ != nullptr; }




    private:
        template <typename Y>
        friend class RefCountedPtr;

        T *value_ {nullptr} ;
    };


    template <typename T>
    class WeakRefCountedPtr{


    };

}
#endif //CZYSERVER_REF_COUNT_PTR_H
