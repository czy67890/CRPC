#pragma once 
#include <cstring>
#include <string_view>
#include <functional>
#include <string>
#include "util/slice_refcount.h"
#include <vector>


namespace std{
    template <>
    struct hash<std::string_view>
    {  
        static constexpr uint64_t kHashBase = 131;
        static constexpr uint64_t kHashMod = 1e9 + 9;
        size_t operator()(const std::string_view &stv) const{
            //double hash to ensure the probability of hash collision
            // to be the minium
            /// this is the simpest implement
            size_t len = stv.length();
            uint64_t res = 0;
            for(int i = 0; i < stv.length() ;++ i){
              res = (res * kHashBase + (uint8_t)stv[i] ) %kHashMod;
            }
            return res;
        }
    };
}

namespace crpc_util{



static constexpr int kCrpcExtraSize = sizeof(void*);
/// @brief the crpc_slice inlined to avoid malloc when small string
static constexpr int kSliceInlinedSize = sizeof(size_t) + sizeof(uint8_t*) - 1 + kCrpcExtraSize;


/// @brief  just the normal mean of operator= is good
struct crpc_slice {
    struct CrpcSliceRefcount *refcount;
    /// @brief indicate that the inlined 
    
    union crpc_slice_data{
        struct crpc_slice_refcounted{
            size_t len;
            uint8_t* bytes;
        } refcounted;
        struct crpc_slice_inlined{
            int8_t len;
            uint8_t bytes[kSliceInlinedSize];
        } inlined;
    } data;

    void Unref(){
        if( reinterpret_cast<uintptr_t>(refcount) > 1){
            refcount->Unref();
        }
    }

    void Ref(){
        if(reinterpret_cast<uintptr_t>(refcount) > 1){
            refcount->Ref();
        }
    }

    uint8_t* StartPtr() const{
        if(refcount){
            return data.refcounted.bytes;
        }
        return (uint8_t*)data.inlined.bytes;
    }

    const size_t Len() const{
        if(refcount){
            return data.refcounted.len;
        }
        return data.inlined.len;
    }

    bool Equal(const crpc_slice& other) const{
        if((!refcount) || (!other.refcount)){
            return BiteWiseEq(other);
        } 

        return data.refcounted.len == other.data.refcounted.len &&
         data.refcounted.bytes == other.data.refcounted.bytes;
    }


    size_t Hash() const{
        return std::hash<std::string_view> (StringView());
    }

    void SetLen(size_t len){
        if(refcount){
            data.refcounted.len = len;
        }
        data.inlined.len = len;
    }

    uint8_t* End() const{
        if(refcount){
            return data.refcounted.bytes + data.refcounted.len;
        }
        return reinterpret_cast<uint8_t *> ((uint8_t *)data.inlined.bytes + data.inlined.len);
    }

    std::string_view StringView() const{
        return std::string_view{reinterpret_cast<char*>(StartPtr()),Len()};
    }

private:
    
    bool BiteWiseEq(const crpc_slice &rhs) const {
        if(Len() != rhs.Len()){ 
            return false;
        }
        if(Len() == 0){
            return true;
        }
        /// tips in order to prevent the case u writte wrong 
        /// we put number to the left
        return 0 == memcmp(StartPtr(),rhs.StartPtr(),Len());
    }

};



static constexpr int kCrpcSliceBufferCount = 7;


struct crpc_slicebuffer{
    void Swap(crpc_slicebuffer &other);

    void ResetAndUnref(){
        for(size_t i =0 ;i  < count;++i){
            slices->Unref();
        }
        count = 0;
        length = 0;
        slices = base;
    }

    void Destory(){
        ResetAndUnref();
        if(base != inlined){
            free(base);
            base = slices = inlined;
        }
    }

    void Init();

    /// @brief only for the internal use
    crpc_slice *base;
    

    /// @brief slices array start
    crpc_slice *slices;
    
    /// @brief  number of slice in the array
    size_t count;
    
    /// @brief number of slice allocated
    size_t capacity;

    /// @brief total len of the slice buffer
    size_t length;

    /// @brief when the num smaller than kcrpc_sliceBufferCount
    /// we use this deirectly 
    /// avoid allocator
    crpc_slice inlined[kCrpcSliceBufferCount];
};

}//end of namespace

#ifdef __cplusplus
extern "C"{ 
#endif
    using crpc_util::crpc_slice;
    using crpc_util::CrpcSliceRefcount;
    static constexpr crpc_slice EmptySlice(){
        return {nullptr,{0}};
    }

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
            auto refCounted  = new MovedCppStringSliceCount(std::move(s));
            res.data.refcounted.len = refCounted->size();
            res.data.refcounted.bytes = refCounted->data();
            res.refcount = refCounted;
        }
        return res;
    }
    



#ifdef __cplusplus
}
#endif

