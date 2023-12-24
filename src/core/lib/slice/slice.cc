//
// Created by czy on 2023/12/24.
//
#include "crpc/impl/slice_type.h"
#include "crpc/slice.h"
#include <string_view>
#include <functional>
#include <cassert>
#include "core/lib/slice/slice_refcount.h"



static constexpr crpc_util::crpc_slice EmptySlice(){
    return {nullptr,{0}};
}
namespace crpc_util {
    void crpc_slice::Unref() {
        if (reinterpret_cast<uintptr_t>(refcount) > 1) {
            refcount->Unref();
        }
    }

    void crpc_slice::Ref() {
        if (reinterpret_cast<uintptr_t>(refcount) > 1) {
            refcount->Ref();
        }
    }

    uint8_t *crpc_slice::StartPtr() const {
        if (refcount) {
            return data.refcounted.bytes;
        }
        return (uint8_t *) data.inlined.bytes;
    }

    const size_t crpc_slice::Len() const {
        if (refcount) {
            return data.refcounted.len;
        }
        return data.inlined.len;
    }

    bool crpc_slice::Equal(const crpc_slice &other) const {
        if ((!refcount) || (!other.refcount)) {
            return BiteWiseEq(other);
        }

        return data.refcounted.len == other.data.refcounted.len &&
               data.refcounted.bytes == other.data.refcounted.bytes;
    }
    size_t crpc_slice::Hash() const{
        auto stv = StringView();
        std::hash<std::string_view> hasher;
        return hasher(stv);
    }

    void crpc_slice::SetLen(size_t len){
        if(refcount){
            data.refcounted.len = len;
        }
        data.inlined.len = len;
    }

    uint8_t* crpc_slice::End() const{
        if(refcount){
            return data.refcounted.bytes + data.refcounted.len;
        }
        return reinterpret_cast<uint8_t *> ((uint8_t *)data.inlined.bytes + data.inlined.len);
    }

    std::string_view crpc_slice::StringView() const{
        return std::string_view{reinterpret_cast<char*>(StartPtr()),Len()};
    }

    static constexpr crpc_slice EmptySlice(){
        return {nullptr,{0}};
    }
}

using crpc_util::crpc_slice;
using crpc_util::CrpcSliceRefcount;


crpc_slice crpc_slice_malloc_large(size_t length){
    crpc_slice res;
    uint8_t *memory = new uint8_t[sizeof(CrpcSliceRefcount) + length];
    res.refcount = new (memory) CrpcSliceRefcount([](CrpcSliceRefcount *p){
        /// in the case of the memory
        /// the [] and cast operator is unmissingable
        delete [] reinterpret_cast<uint8_t*> (p);
    });
    res.data.refcounted.bytes = memory + sizeof(CrpcSliceRefcount);
    res.data.refcounted.len = length;
    return res;
}

crpc_slice crpc_slice_malloc(size_t length){
    if(length <= crpc_util::kSliceInlinedSize){
        crpc_slice slice;
        slice.refcount = nullptr;
        slice.data.inlined.len = length;
        return slice;
    }
    else{
        return crpc_slice_malloc_large(length);
    }

}

crpc_slice crpc_slice_from_copied_buffer(const char *source,size_t len){
    if(len == 0){
        return EmptySlice();
    }
    crpc_slice out = crpc_slice_malloc_large(len);
    memcpy(out.StartPtr(),source,len);
    return out;
}

crpc_slice crpc_slice_from_cpp_string(std::string s){
    crpc_slice res{nullptr,{0}};
    if(s.length()  <= crpc_util::kSliceInlinedSize){
        memcpy(res.data.inlined.bytes,s.data(),s.length());
        res.data.inlined.len = s.length();
    }
    else{
        auto refCounted  = new crpc_util::MovedCppStringSliceCount(std::move(s));
        res.data.refcounted.len = refCounted->size();
        res.data.refcounted.bytes = refCounted->data();
        res.refcount = refCounted;
    }
    return res;
}


/// the end and begin definition as cpp definition
/// [Begin,End)
crpc_util::crpc_slice sub_no_ref(crpc_util::crpc_slice source,size_t begin,size_t end){
    crpc_slice subset;
    /// some times u need some assert to ensure
    /// logic not allowed situation
    assert(end >= begin);
    /// always consider the situation of out bounder
    assert(end <= source.Len());
    if(source.refcount != nullptr){

        /// why here use recount to point to the source?
        /// TODO::figure out
        subset.refcount = source.refcount;
        subset.data.refcounted.bytes = source.data.refcounted.bytes + begin;
        subset.data.refcounted.len = end - begin;
    }
    else{
        /// this must be excute
        subset.refcount = nullptr;
        subset.data.inlined.len = end - begin;
        memcpy(subset.data.inlined.bytes,source.data.refcounted.bytes + begin,end - begin);
    }
}

crpc_util::crpc_slice crpc_slice_sub_no_ref(crpc_util::crpc_slice source,size_t begin,size_t end){
    return sub_no_ref(source,begin,end);
}
