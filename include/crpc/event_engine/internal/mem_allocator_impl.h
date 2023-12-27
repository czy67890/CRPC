//
// Created by czy on 2023/12/25.
//

#ifndef CZYSERVER_MEM_ALLOCATOE_IMPL_H
#define CZYSERVER_MEM_ALLOCATOE_IMPL_H


#include <algorithm>
#include <memory>
#include <type_traits>
#include <vector>
#include "crpc/event_engine/mem_request.h"
#include "crpc/slice.h"
#include "non_copyable.h"
namespace crpc_event_engine{

    class MemoryAllocatorImpl
        :public crpc_core::NonCopyable,
        public std::enable_shared_from_this<MemoryAllocatorImpl>{
    public:

        MemoryAllocatorImpl(){}

        virtual ~MemoryAllocatorImpl() {};

        virtual size_t Reserve(MemoryRequest mem_req) = 0;

        virtual crpc_slice MakeSlice(MemoryRequest memreq) = 0;

        virtual void Release(size_t n) = 0;

        virtual void ShutDown() = 0;

    };

}



#endif //CZYSERVER_MEM_ALLOCATOE_IMPL_H
