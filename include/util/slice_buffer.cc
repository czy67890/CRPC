#include "util/slice_buffer.h"
#include <stdlib.h>
crpc_util::SliceBuffer::~SliceBuffer()
{
   slice_buffer_.Destory();
}
