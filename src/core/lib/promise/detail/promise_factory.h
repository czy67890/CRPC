//
// Created by czy on 2024/1/22.
//

#ifndef CZYSERVER_PROMISE_FACTORY_H
#define CZYSERVER_PROMISE_FACTORY_H
#include <type_traits>


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
    struct ResultOfT{

    };

    template <typename F,typename ...Args>
    struct ResultOfT<F(Args...),std::void_t<decltype( std::declval<std::remove_cvref<F>>()(std::declval<Args>()...))>>{
        using T = decltype( std::declval<std::remove_cvref<F>>()(std::declval<Args>()...));
    };

    template <typename F,typename ...Args>
    struct ResultOfT<F(Args...)&,std::void_t<decltype( std::declval<std::remove_cvref<F>>()(std::declval<Args>()...))>>{
        using T = decltype( std::declval<std::remove_cvref<F>>()(std::declval<Args>()...));
    };

    template <typename F,typename ...Args>
    struct ResultOfT<const F(Args...)&,std::void_t<decltype( std::declval<std::remove_cvref<F>>()(std::declval<Args>()...))>>{
        using T = decltype( std::declval<std::remove_cvref<F>>()(std::declval<Args>()...));
    };

    template<typename F>
    using ResultOf = ResultOfT<F>::T;


}


}

#endif //CZYSERVER_PROMISE_FACTORY_H
