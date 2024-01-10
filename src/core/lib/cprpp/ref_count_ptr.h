//
// Created by czy on 2024/1/5.
//

#ifndef CZYSERVER_REF_COUNT_PTR_H
#define CZYSERVER_REF_COUNT_PTR_H
#include <atomic>
#include <algorithm>
#include <functional>

/// RefCountedPtr is a RAII class
/// while it auto manage the T *value_
/// and called the value_->IncrementRefCount()
/// ,value_->UnRef() properly


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
            :value_(static_cast<T *>(value)){
            value_->IncrementRefCount();
        }

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

        RefCountedPtr& operator= (const RefCountedPtr &rhs){
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
    public:
        WeakRefCountedPtr(){}

        WeakRefCountedPtr(std::nullptr_t ){}

        template <typename Y>
        explicit WeakRefCountedPtr(Y *value)
            :value_ ( value)
        {
            value_->IncrementWeakRefCount();
        }

        void swap(WeakRefCountedPtr &rhs) noexcept{
            std::swap(value_ ,rhs.value_);
        }

        WeakRefCountedPtr(WeakRefCountedPtr&& rhs) noexcept{
            value_ = std::exchange(rhs.value_,nullptr);
        }

        WeakRefCountedPtr& operator=(WeakRefCountedPtr && rhs) noexcept{
            WeakRefCountedPtr tmp{std::move(rhs)};
            swap(rhs);
        }

        template<typename Y>
        WeakRefCountedPtr (WeakRefCountedPtr<Y> && rhs) noexcept{
            value_ = static_cast<T*>(rhs.value_);
            rhs.value_ = nullptr;
        }

        template <typename Y>
        WeakRefCountedPtr& operator=(WeakRefCountedPtr<T> &&rhs) noexcept{
            WeakRefCountedPtr tmp{std::move(rhs)};
            swap(tmp);
        }

        WeakRefCountedPtr(const WeakRefCountedPtr &rhs){
            if(rhs.value_ != nullptr){
                rhs.value_->IncrementWeakRefCount();
            }
            value_ = rhs.value_;
        }

        WeakRefCountedPtr& operator=(const WeakRefCountedPtr & rhs){
            WeakRefCountedPtr tmp{rhs};
            swap(tmp);
        }

        template<typename Y>
        WeakRefCountedPtr( const WeakRefCountedPtr &rhs){
            if(rhs.value_){
                rhs.value_->IncrementWeakRefCount();
            }
            value_ = static_cast<Y*>(rhs.value_);
        }

        template<typename Y>
        WeakRefCountedPtr& operator=(const WeakRefCountedPtr &rhs){
            WeakRefCountedPtr tmp{rhs};
            swap(tmp);
        }


        ~WeakRefCountedPtr(){
            if(value_ != nullptr){
                value_->WeakUnref();
            }
        }

        T* release(){
            return std::exchange(value_,nullptr);
        }


        T *get() const{
            return value_;
        }

        T& operator*() const{
            return *value_;
        }

        T* operator->() const{
            return value_;
        }

        template <typename Y>
        bool operator==(const WeakRefCountedPtr<Y> &rhs) const{
            return value_ == rhs.value_;
        }

        template <typename Y>
        bool operator==(const Y * rhs) const{
            return value_ == rhs;
        }

        bool operator==(std::nullptr_t) const{
            return value_ == nullptr;
        }

        bool operator!=(std::nullptr_t) const{
            return value_ != nullptr;
        }

        template<typename Y>
        bool operator !=(const WeakRefCountedPtr<Y> &rhs) const{
            return !(*this == rhs);
        }

        template<typename Y>
        bool operator != (const Y * rhs) const{
            return !(*this == rhs);
        }



    private:
        template <typename Y>
        friend class WeakRefCountedPtr;

        T * value_{nullptr};
    };

    template<typename T,typename ...Args>
    RefCountedPtr<T> MakeRefCounted(Args&& ...args){
        return RefCountedPtr<T>(new T(std::forward<Args>(args)...));
    }

    template<typename T>
    bool operator<(const RefCountedPtr<T> &lhs,const RefCountedPtr<T> &rhs){
        return lhs.get() < rhs.get();
    }

    template<typename T>
    bool operator<(const WeakRefCountedPtr<T> &lhs,const WeakRefCountedPtr<T> &rhs){
        return lhs.get() < rhs.get();
    }


    template <typename T>
    struct EqualOperatorObject
    {
        bool operator()(const RefCountedPtr<T> &lhs,const RefCountedPtr<T> &rhs) const{
            return lhs == rhs;
        }

        bool operator()(const WeakRefCountedPtr<T> &lhs,const WeakRefCountedPtr<T> &rhs)const{
            return lhs == rhs;
        }

        bool operator()(const RefCountedPtr<T> &lhs,const WeakRefCountedPtr<T> &rhs) const{
            return lhs == rhs.get() ;
        }

        bool operator()(const WeakRefCountedPtr<T> &lhs,const RefCountedPtr<T> &rhs) const{
            return rhs == lhs.get();
        }

        bool operator()(const RefCountedPtr<T> &lhs , const T * rhs) const{
            return lhs == rhs;
        }

        bool operator()(const T * rhs,const RefCountedPtr<T> &lhs ) const{
            return lhs == rhs;
        }

        bool operator()(const WeakRefCountedPtr<T> &lhs , const T * rhs) const{
            return lhs == rhs;
        }

        bool operator()(const T * rhs,const WeakRefCountedPtr<T> &lhs ) const{
            return lhs == rhs;
        }

    };


}

namespace std {
    template<typename T>
    struct hash<crpc_core::RefCountedPtr<T>>{

        size_t operator()(const crpc_core::RefCountedPtr<T> & rp) const{
            return std::hash<T*> ()( (rp.get()));
        }

        size_t operator()(const crpc_core::WeakRefCountedPtr<T> &wrp) const{
            return std::hash<T*>()( wrp.get());
        }

    };
}



#endif //CZYSERVER_REF_COUNT_PTR_H
