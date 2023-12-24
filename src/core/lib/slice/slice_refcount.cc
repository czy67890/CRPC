//
// Created by czy on 2023/12/24.
//

#include "slice_refcount.h"

crpc_util::CrpcSliceRefcount *crpc_util::CrpcSliceRefcount::NoopRefcount() {
    /// why use 1
    /// i think to distinguish the case of default
    /// and the case of non-ref
    return reinterpret_cast<CrpcSliceRefcount*>(1);
}
