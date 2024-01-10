//
// Created by czy on 2023/12/30.
//

#ifndef CZYSERVER_NO_DESTRUCT_H
#define CZYSERVER_NO_DESTRUCT_H
#include <type_traits>
#include <utility>
#include "non_copyable.h"
namespace crpc_core{
    template <typename T>
    class NoDestruct :public NonCopyable{
    public:
        template <typename ... Args>
        explicit  NoDestruct(Args&& ... args){
            /// NoDestruct need the tri
            static_assert(std::is_trivially_destructible_v<NoDestruct<T>>,"must be trivailly destructible");
            new (space_)T(std::forward<Args>(args)...);
        }

        ~NoDestruct() = default;

        T *operator->() {
            return get();
        }

        const T * operator->() const{
            return get();
        }

        T* get(){
            return reinterpret_cast<T*> (space_);
        }

        const T * get() const{
            return reinterpret_cast<const T * >(space_);
        }

        T& operator*(){
            return *get();
        }

        const T & operator*() const{
            return  *get();
        }



    private:
        alignas(sizeof(T)) char space_[sizeof(T)] ;
    };

    template <typename T>
    class NoDestructSingletion{
    public:

        static T *get(){
            return value.get();
        }

        NoDestructSingletion() = delete;
        ~NoDestructSingletion() = delete;

    private:
        static NoDestruct<T> value;
    };


    template <typename T>
    NoDestruct<T>  NoDestructSingletion<T> ::value{};

}





#endif //CZYSERVER_NO_DESTRUCT_H
