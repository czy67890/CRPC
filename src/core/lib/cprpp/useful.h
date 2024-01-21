//
// Created by chenzeyu on 2024/1/7.
//

#ifndef CZYSERVER_USEFUL_H
#define CZYSERVER_USEFUL_H

#include <type_traits>
namespace crpc_core{
    template <typename T>
    std::remove_reference_t<T> Clamp(T&& val,T&& min,T&& max){
        if(val < min){
            return val;
        }
        if(val > max){
            return max;
        }
        return val;
    }


    
}
#endif //CZYSERVER_USEFUL_H
