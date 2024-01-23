//
// Created by czy on 2024/1/22.
//

#ifndef CZYSERVER_PROMISE_FACTORY_H
#define CZYSERVER_PROMISE_FACTORY_H
#include <type_traits>
#include <utility>
#include "core/lib/promise/detail/promise_like.h"
namespace crpc_core{


namespace promise_detail{
    template<typename T,typename Ignored = void>
    struct IsVoidCallable{
        static constexpr bool value = false;
    };

    template<typename F>
    struct IsVoidCallable<F,std::void_t<decltype(std::declval<F>()())>>{
        static constexpr bool value = true;
    };

    template<typename T,typename Ignored = void>
    struct ResultOfT;

    template <typename F,typename ...Args>
    struct ResultOfT<F(Args...),std::void_t<decltype( std::declval<std::remove_cvref_t<F>>()(std::declval<Args>()...))>>{
        using T = decltype( std::declval<std::remove_cvref_t<F>>()(std::declval<Args>()...));
    };

    template <typename F,typename ...Args>
    struct ResultOfT<F(Args...)&,std::void_t<decltype( std::declval<std::remove_cvref_t<F>>()(std::declval<Args>()...))>>{
        using T = decltype( std::declval<std::remove_cvref_t<F>>()(std::declval<Args>()...));
    };

    template <typename F,typename ...Args>
    struct ResultOfT<const F(Args...)&,std::void_t<decltype( std::declval<std::remove_cvref_t<F>>()(std::declval<Args>()...))>>{
        using T = decltype( std::declval<std::remove_cvref_t<F>>()(std::declval<Args>()...));
    };

    template<typename F>
    using ResultOf = ResultOfT<F>::T;

    /// remember when use it
    /// we need to remove_cv_ref
    template<typename F,typename Arg>
    class Curried{
    public:
        Curried(F && f,Arg && arg)
            :f_(std::forward<F>(f)),arg_(std::forward<Arg>(arg))
        {}

        using Result = decltype(std::declval<F>()(std::declval<Arg>()));

        Result operator()(){
            return f_(std::move(arg_));
        }

    private:
        [[no_unique_address]]F f_;
        [[no_unique_address]] Arg arg_;
    };

    template<typename A,typename F>
    std::enable_if_t<!IsVoidCallable<ResultOf<F(A)>>::value,PromiseLike<Curried<std::remove_cvref_t<F>,A>>>
    PromiseFactoryImpl(F && f,A &&arg){
        return Curried<std::remove_cvref_t<F>,A>(std::forward<F>(f),std::forward<A>(arg));
    }

    template<typename >

}
}

#endif //CZYSERVER_PROMISE_FACTORY_H
