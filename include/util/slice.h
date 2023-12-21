#pragma once 
#include "util/slice_refcount.h"

namespace crpc_util{

static constexpr int kCrpcExtraSize = sizeof(void*);
static constexpr int kSliceInlinedSize = sizeof(size_t) + sizeof(uint8_t*) - 1 + kCrpcExtraSize;


struct crpc_slice {
    struct crpc_slice_refcount *refcount;
    /// @brief indicate that the inlined 
    
    union grpc_slice_data{
        struct crpc_slice_refcounted{
            size_t len;
            uint8_t* bytes;
        } refcounted;
        struct crpc_slice_inlined{
            int8_t len;
            uint8_t bytes[kSliceInlinedSize];
        } inlined;
    } data;

    uint8_t* StartPtr() const{
        if(refcount){
            return data.refcounted.bytes;
        }
        return data.inlined.bytes;
    }

    const size_t Len() const{
        if(refcount){
            return data.refcounted.len;
        }
        return data.inlined.len;
    }

    void setLen(size_t len){
        if(refcount){
            data.refcounted.length = len;
        }
    }
}



};
static constexpr int kCrpcSliceBufferCount = 7;


typedef struct crpc_slice_buffer{
    
    crpc_slice *base;
    
    crpc_slice *slices;
    
    size_t count;
    
    size_t capacity;

    size_t length;

    crpc_slice inlined[kCrpcSliceBufferCount];




} crpc_slice_buffer;

