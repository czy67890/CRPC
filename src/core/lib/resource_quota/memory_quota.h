//
// Created by czy on 2024/1/9.
//

#ifndef CZYSERVER_MEMORY_QUOTA_H
#define CZYSERVER_MEMORY_QUOTA_H
#include <stdint.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "crpc/event_engine/mem_request.h"
#include "crpc/event_engine/internal/mem_allocator_impl.h"
#include "crpc/event_engine/mem_allocator.h"
namespace crpc_core{
    class BasicMemoryQuota;

    class MemoryQuota;

    class CrpcMemoryAllocatorImpl;

    using crpc_event_engine::MemoryRequest;

    using EventMemoryAllocatorImpl = crpc_event_engine::MemoryAllocatorImpl;

    using crpc_event_engine::MemoryAllocator;

    template<typename T>
    using Vec = std::vector<T>;


    /// three type of mem reclaim
    /// Benign means have no bad effect
    enum class ReclamationPss{
        /// like shrink the buff to their size
        kBenign = 0,

        /// like reclaim the channel be disposed
        kIdle = 1,


        /// cancel work and cancel request
        kDestructive= 2,
    };

    static constexpr size_t kNumReclamationPass = 3;


}


#endif //CZYSERVER_MEMORY_QUOTA_H
