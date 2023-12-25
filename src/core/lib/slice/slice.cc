//
// Created by czy on 2023/12/24.
//
#include "crpc/impl/slice_type.h"
#include "crpc/slice.h"
#include <string_view>
#include <functional>
#include <cassert>
#include "core/lib/slice/slice_refcount.h"
#include "slice.h"
#include "crpc/slice.h"

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

crpc_util::crpc_slice crpc_slice_copy(crpc_util::crpc_slice source) {
    /// use the code
    /// inorder to reduce duplicated code
    crpc_slice out = crpc_slice_malloc(source.Len());
    memcpy(out.StartPtr(),source.StartPtr(),source.Len());
    return out;
}

/// this func will move the internal slice out
Slice Slice::TakeOwned() {
//    crpc_slice out;
    crpc_slice tmp{c_slice()};
    if(tmp.refcount == nullptr ){
        return Slice(tmp);
    }
    if(tmp.refcount == CrpcSliceRefcount::NoopRefcount()){
        return Slice{crpc_slice_copy(tmp)};
    }
    return Slice(TakeCSlice());
}

Slice Slice::AsOwned() const {
    auto tmpSlice {crpc_slice()};
    if(tmpSlice.refcount == nullptr){
        return Slice(tmpSlice);
    }
    if(tmpSlice.refcount == CrpcSliceRefcount::NoopRefcount()){
        return Slice(crpc_slice_copy(tmpSlice));
    }
    tmpSlice.Ref();
    return Slice(tmpSlice);
}

MutableSlice Slice::TakeMutable() {
    crpc_slice tmpSlice{c_slice()};
    if(tmpSlice.refcount == nullptr){
        return MutableSlice(tmpSlice);
    }
    if(tmpSlice.refcount != CrpcSliceRefcount::NoopRefcount() && tmpSlice.refcount->IsUniq()){
        return MutableSlice(TakeCSlice());
    }
    return MutableSlice(crpc_slice_copy(tmpSlice));
}

Slice::~Slice() {
    c_slice_ptr()->Unref();
}

void Slice::Ref() {
    c_slice_ptr()->Ref();
}

Slice Slice::FromRefCountAndBytes(CrpcSliceRefcount &refCount, const uint8_t* begin,const uint8_t *end) {
    crpc_slice out;
    out.refcount = &refCount;
    if(&refCount != CrpcSliceRefcount::NoopRefcount()){
        refCount.Ref();
    }
    out.data.refcounted.bytes = const_cast<uint8_t*>(begin);
    out.data.refcounted.len = end - begin;
    return Slice{out};
}



using crpc_util::crpc_slice;
using crpc_util::CrpcSliceRefcount;
namespace slice_detail{

    crpc_slice BaseSlice::TakeCSlice(){
        crpc_slice out = slice_;
        slice_ = ::EmptySlice();
        return out;
    }

}//end of namespace slice_detail

StaticSlice::StaticSlice(const crpc_slice &slice)
        :slice_detail::BaseSlice(slice)
{
    /// static slice must not be refed
    assert(slice.refcount == CrpcSliceRefcount::NoopRefcount());
}

StaticSlice::StaticSlice(const StaticSlice &rhs)
        :slice_detail::BaseSlice(rhs.c_slice()){

}

StaticSlice& StaticSlice::operator=(const StaticSlice &rhs){
    SetCSlice(rhs.c_slice());
    return *this;
}

StaticSlice::StaticSlice(StaticSlice &&rhs) noexcept
:slice_detail::BaseSlice(rhs.c_slice())
{
    rhs.SetCSlice({nullptr,{}});
}

StaticSlice& StaticSlice::operator=(StaticSlice &&rhs) noexcept{
    ///copy and swap item ,very safe
    StaticSlice tmpSlice(std::move(rhs));
    Swap(tmpSlice);
    return *this;
}

MutableSlice::MutableSlice(const crpc_slice &slice)
        :slice_detail::BaseSlice(slice)
{
    assert(slice.refcount == nullptr || slice.refcount->IsUniq());
}

MutableSlice::~MutableSlice(){
    c_slice_ptr()->Unref();
}

MutableSlice::MutableSlice(MutableSlice &&rhs) noexcept{
    SetCSlice(rhs.TakeCSlice());
}

MutableSlice& MutableSlice::operator=(MutableSlice &&rhs) noexcept{
    MutableSlice tmpSlice(std::move(rhs));
    Swap(tmpSlice);
    return *this;
}

MutableSlice MutableSlice::CreateUninitialized(size_t len) {
    return MutableSlice(crpc_slice_malloc(len));
}

MutableSlice MutableSlice::TakeSubSlice(size_t pos,size_t len){
    return MutableSlice(crpc_slice_sub_no_ref(TakeCSlice(),pos,pos + len));
}

uint8_t* MutableSlice::begin(){
    return mutable_data();
}

uint8_t * MutableSlice::end(){
    return mutable_data() + size();
}

uint8_t* MutableSlice::data(){
    return mutable_data();
}

//// no forget the &
uint8_t& MutableSlice::operator[](size_t i){
    return mutable_data()[i];
}





Slice& Slice::operator=(Slice &&rhs) noexcept{
    Slice tmpSlice{std::move(rhs)};
    Swap(tmpSlice);
    return *this;
}









