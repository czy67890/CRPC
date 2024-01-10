//
// Created by chenzeyu on 2024/1/10.
//

#ifndef CZYSERVER_CONTEXT_H
#define CZYSERVER_CONTEXT_H
#include <utility>
#include <cassert>

namespace crpc_core{

    template<typename T>
    struct ContextType;

    namespace promise_detail{

        template<typename T>
        class Context :public ContextType<T>{
        public:

            explicit Context(T * p )
                :old_(current_)
            {
                current_ = p;
            }

            ~Context() {current_ = old_;}

            Context(const Context & rhs) = delete;

            Context& operator=(const Context &) = delete;


            static T* Get(){
                return current_;
            }
        private:
            T * const old_;
            static thread_local  T * current_;
        };


        template<typename T>
        thread_local  T * Context<T>::current_;


        template<typename T,typename F>
        class WithContext{
        public:
            WithContext(F f,T *context)
                :context_(context),f_(std::move(f))
            {}

            decltype(std::decay<F>()()) operator()(){
                Context<T> ctx(context_);
                return f_();
            }

        private:
            T * context_;
            F f_;
        };
    }// namespace promise_detail

    template<typename T>
    bool HasContext(){
        return promise_detail::Context<T>::Get() != nullptr;
    }

    template<typename T>
    T * GetContext(){
        auto * p = promise_detail::Context<T>::Get();
        assert(p != nullptr);
        return p;
    }

    template<typename T,typename F>
    promise_detail::WithContext<T,F> WithContext(F f,T * context)
    {
        return promise_detail::WithContext<T,F>(std::move(f),context);
    }
}

#endif //CZYSERVER_CONTEXT_H
