//
// Created by czy on 2024/1/5.
//

#ifndef CZYSERVER_ATOMIC_UTILS_H
#define CZYSERVER_ATOMIC_UTILS_H
#include <atomic>
namespace crpc_core{

    template <typename T>
    inline bool IncrementIfNonZero(std::atomic<T> *p){
        auto count = p->load(std::memory_order_acquire);

        do{
            if(count == 0){
                return false;
            }
            ///first para is when success our mem_order ,second para is when fail our mem_order
        }while(!p->compare_exchange_weak(count,count + 1,std::memory_order_acq_rel,std::memory_order_acquire));

        return true;
    }
}
#endif //CZYSERVER_ATOMIC_UTILS_H
