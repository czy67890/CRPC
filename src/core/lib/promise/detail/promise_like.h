//
// Created by czy on 2024/1/22.
//

#ifndef CZYSERVER_PROMISE_LIKE_H
#define CZYSERVER_PROMISE_LIKE_H
#include <type_traits>
#include <utility>
#include "core/lib/promise/poll.h"

namespace crpc_core{
    namespace promise_detail{
        template<typename T>
        struct PollWrapper{
            static Poll<T> Wrap(T && x){
                return Poll<T>(std::forward<T>(x));
            }
        };

        ///because of has no Poll's Poll
        ///we partial specialization it
        template<typename T>
        struct PollWrapper<Poll<T>>{
            static Poll<T> Wrap(Poll<T>&& x){
                return Poll<T> (std::forward<Poll<T>>(x));
            }
        };

        template <typename T>
        auto WrapInPoll(T && x){
            return PollWrapper<T>::Wrap(x);
        }

        template <typename F,typename SfinaeVoid = void>
        class PromiseLike;

        template<>
        class PromiseLike<void>;

        /// remember this is a partial specialization
        template<typename F>
        class PromiseLike<F,std::enable_if_t<!std::is_void_v<std::result_of_t<F()> > >>{
            PromiseLike(F && f)
                :f_(std::forward<F>(f)){}

            auto operator()(){
                return WrapInPoll(f_());
            }

            using Result = typename PollTraits<decltype(WrapInPoll(F()))>::Type;

        private:
            [[no_unique_address]] F f_;
        };

    }

}


#endif //CZYSERVER_PROMISE_LIKE_H
