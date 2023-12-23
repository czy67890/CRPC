#pragma once
#include <string>
#include "util/cslice.h"
#include "non_copyable.h"
#include "util/slice.h"
namespace crpc_util{
 
 
class SliceBuffer :public crpc_core::NonCopyable{
public:
    explicit SliceBuffer()
    {
        slice_buffer_.Init();
    }

    ~SliceBuffer();
    
    SliceBuffer(SliceBuffer &&rhs) noexcept{
        slice_buffer_.Init();
        slice_buffer_.Swap(rhs.slice_buffer_);
    }

    SliceBuffer& operator=(SliceBuffer&& rhs) noexcept{
        slice_buffer_.Swap(rhs.slice_buffer_);
        return *this;
    }

    void Append(Slice slice);



private:

    crpc_slicebuffer slice_buffer_;
};



}