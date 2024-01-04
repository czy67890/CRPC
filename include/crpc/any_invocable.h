//
// Created by czy on 2024/1/4.
//

#ifndef CZYSERVER_ANY_INVOCABLE_H
#define CZYSERVER_ANY_INVOCABLE_H
#include "crpc/impl/any_invocable_internal.h"
namespace crpc_function {

    template<class Sig>
    class AnyInvocable
    /// here we will get the special extension
    : private internal_any_invocable::Impl<Sig>{

    private:
        static_assert(std::is_function_v<Sig>," the template must be a func");

        using Impl = internal_any_invocable::Impl<Sig>;

    public:

        using result_type = typename Impl::result_type;

        AnyInvocable() noexcept  = default;

        AnyInvocable(std::nullptr_t)  noexcept{
        }

        AnyInvocable(AnyInvocable &&rhs) noexcept = default;

        template <class F,typename = std::enable_if_t<internal_any_invocable::CanConvert<Sig,F>::value>>
        AnyInvocable(F && f)
            :Impl(internal_any_invocable::ConversionConstruct(),std::forward<F>(f))
        {

        }

        template <class T ,class ... Args,typename = std::enable_if_t<internal_any_invocable::CanEmplace<Sig,T,Args...>::value>>
        explicit  AnyInvocable(std::in_place_type_t<T>,Args&& ... args)
            :Impl(std::in_place_type<std::decay_t<T>>,std::forward<Args>(args)...)
        {

                ///std::decay_t will remove const and volatile and &
                static_assert(std::is_same_v<T,std::decay_t<T>> ,"must have no const and volatile of in_place_type ");
        }

        template <class T ,class U,class ...Args,typename = std::enable_if_t<
                                internal_any_invocable::CanEmplace<Sig,T,std::initializer_list<U>&,Args...>::value>>
        explicit AnyInvocable(std::in_place_type_t<T>,std::initializer_list<U>& ilist,Args&& ...args )
            :Impl(std::in_place_type<std::decay_t<T>>,ilist,std::forward<Args>(args)...)
        {
            static_assert(std::is_same_v<T,std::decay_t<T>> ,"must have no const and volatile of in_place_type ");
        }

        AnyInvocable& operator = (AnyInvocable &&rhs ) noexcept = default;

        AnyInvocable& operator = (std::nullptr_t) noexcept{
            this->Clear();
        }

        template<class F,typename = std::enable_if_t<internal_any_invocable::CanAssign<Sig,F>::value>>
        AnyInvocable& operator=(F&& f){
            *this = AnyInvocable(std::forward<F>(f));
            return *this;
        }

        template<class F , typename = std::enable_if_t<internal_any_invocable::CanAssignReferenceWrapper<Sig,F>::value>>
        AnyInvocable& operator=(std::reference_wrapper<F> f) noexcept{
            *this = AnyInvocable(f);
            return *this;
        }

        ~AnyInvocable() = default;

        void swap(AnyInvocable &other) noexcept{
            /// because of the noexcept move operator exist
            std::swap(*this,other);
        }

        explicit  operator bool() const noexcept{
            return this->HasValue();
        }

        using Impl::operator();

         friend bool operator == (const AnyInvocable& f,std::nullptr_t){
             return !f.HasValue();
         }

         friend bool operator == (std::nullptr_t,const AnyInvocable &f){
             return f == nullptr;
         }

         friend bool operator != (const AnyInvocable &f,std::nullptr_t ){
             return ! (f == nullptr);
         }

         friend bool operator != (std::nullptr_t ,const AnyInvocable &f){
             return f != nullptr;
         }

         friend void swap(AnyInvocable &lhs,AnyInvocable &rhs){
             lhs.swap(rhs);
         }

    private:
        template <bool,class,class ...>
        friend class internal_any_invocable::CoreImpl;
    };



}


#endif //CZYSERVER_ANY_INVOCABLE_H
