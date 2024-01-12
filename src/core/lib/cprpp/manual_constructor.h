//
// Created by chenzeyu on 2024/1/12.
//

#ifndef CZYSERVER_MANUAL_CONSTRUCTOR_H
#define CZYSERVER_MANUAL_CONSTRUCTOR_H

#include <stddef.h>
#include <utility>
#include <algorithm>
#include "core/lib/cprpp/construct_and_destruct.h"

namespace crpc_core{
    namespace manual_ctor_impl{
        template<class Member,class ... List>
        class is_one_of;

        template<class Member,class ...List>
        class is_one_of<Member,Member,List...>{
        public:
            static constexpr const bool value = true;
        };

        template<class Member,class A,class ... List>
        class is_one_of<Member,A,List...>{
        public:
            static constexpr const bool value = is_one_of<Member,List...>::value;
        };

        template<class Member>
        class is_one_of<Member>{
        public:
            static constexpr const bool value = false;
        };

        template<class ...Types>
        class max_size_of;

        template<class A>
        class max_size_of<A>{
        public:
            static constexpr const size_t value = sizeof(A);
        };

        template<class A,class ...B>
        class max_size_of<A,B...>{
        public:
            static constexpr const size_t value = sizeof(A) >max_size_of<B...>::value ?
                                                  sizeof(A) : max_size_of<B...>::value;
        };

        template <class... Types>
        class max_align_of;

        template <class A>
        class max_align_of<A> {
        public:
            static constexpr const size_t value = alignof(A);
        };

        template <class A, class... B>
        class max_align_of<A, B...> {
        public:
            static constexpr const size_t value = alignof(A) > max_align_of<B...>::value
                                                  ? alignof(A)
                                                  : max_align_of<B...>::value;
        };
    }

    template<typename Type>
    class ManualConstructor{
    public:
        Type *Get() {
            return reinterpret_cast<Type*>(&space_);
        }

        const Type *Get() const{
            return reinterpret_cast<const Type*>(&space_);
        }

        Type *operator()(){
            return Get();
        }

        Type& operator*() {
            return *Get();
        }

        void Init(){
            Construct(Get());
        }

        template<typename ...Ts>
        void Init(Ts&& ... args){
            Construct(Get(),std::forward<Ts>(args)...);
        }

        void Init(const Type &x){
            Construct(Get(),x);
        }

        void Init(Type && x){
            Construct(Get(),std::move(x));
        }

        void Destory(){
            Destruct(Get());
        };
    private:
        alignas(Type) char space_[sizeof(Type)];
    };


}

#endif //CZYSERVER_MANUAL_CONSTRUCTOR_H
