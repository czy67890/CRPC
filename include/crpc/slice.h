//
// Created by czy on 2023/12/24.
//

#ifndef CZYSERVER_SLICE_H
#define CZYSERVER_SLICE_H

#include <string>

namespace crpc_util{
    struct crpc_slice;
}

static constexpr crpc_util::crpc_slice EmptySlice();

crpc_util::crpc_slice crpc_slice_malloc_large(size_t length);

crpc_util::crpc_slice crpc_slice_malloc(size_t length);

crpc_util::crpc_slice crpc_slice_from_copied_buffer(const char *source,size_t len);

crpc_util::crpc_slice crpc_slice_from_cpp_string(std::string s);


crpc_util::crpc_slice crpc_slice_sub_no_ref(crpc_util::crpc_slice source,size_t begin,size_t end);


#endif //CZYSERVER_SLICE_H
