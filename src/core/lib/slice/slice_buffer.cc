//
// Created by czy on 2023/12/24.
//

#include "crpc/event_engine/slice_buffer.h"
crpc_util::SliceBuffer::~SliceBuffer()
{
    slice_buffer_.Destory();
}

/// @brief we will swap the base to the proper place
/// @param other
void crpc_util::crpc_slicebuffer::Swap(crpc_slicebuffer &other)
{

    size_t thisOffset = static_cast<size_t>(slices - base);
    size_t otherOffset = static_cast<size_t>(other.slices - base);

    size_t thisCount = count + thisOffset;
    size_t otherCount = other.count + otherOffset;

    if(base == inlined) {
        if(other.base == other.inlined){
            crpc_slice tmp[kCrpcSliceBufferCount];
            // dst src format
            memcpy(tmp,base,thisCount * sizeof(crpc_slice));
            memcpy(base,other.base,otherCount*sizeof(crpc_slice));
            memcpy(other.base,tmp,thisCount * sizeof(crpc_slice));
        }
        else{
            base = other.base;
            other.base = other.inlined;
            memcpy(other.inlined,inlined,sizeof(crpc_slice) * thisCount);
        }
    }
    else if(other.inlined == other.base){
        /// in the case of other is inlined when self are not
        other.base = base;
        base = inlined;
        memcpy(inlined,other.inlined,sizeof(crpc_slice) * otherCount);
    }
    else{
        /// just std::swap it
        std::swap(other.base,base);
    }

    other.slices = other.base + thisOffset;
    slices = base + otherOffset;

    std::swap(count,other.count);
    std::swap(capacity,other.capacity);
    std::swap(length,other.length);
}

void crpc_util::crpc_slicebuffer::Init()
{
    base = slices = inlined;
    count = length = 0;
    capacity = kCrpcSliceBufferCount;
}

