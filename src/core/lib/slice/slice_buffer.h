//
// Created by czy on 2023/12/24.
//

#ifndef CZYSERVER_SLICE_BUFFER_H
#define CZYSERVER_SLICE_BUFFER_H
#include <string>
#include "crpc/impl/slice_type.h"
#include "non_copyable.h"
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

//        void Append(Slice slice);



    private:

        crpc_slicebuffer slice_buffer_;
    };



}
#endif //CZYSERVER_SLICE_BUFFER_H
