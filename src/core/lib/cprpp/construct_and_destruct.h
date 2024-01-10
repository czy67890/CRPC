//
// Created by czy on 2024/1/10.
//

#ifndef CZYSERVER_CONSTRUCT_AND_DESTRUCT_H
#define CZYSERVER_CONSTRUCT_AND_DESTRUCT_H
#include <new>
#include <functional>
namespace crpc_core{

template <typename T,typename... Args>
void Construct(T *p,Args&&... args){
    new(p) T(std::forward<Args>(args)...);
}


template <typename T>
void Destruct(T * p){
    p->~T();
};

}


#endif //CZYSERVER_CONSTRUCT_AND_DESTRUCT_H
