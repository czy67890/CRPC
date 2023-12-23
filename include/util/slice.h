#pragma once
#include "util/cslice.h"
#include "non_copymoveable.h"
#include <string_view>
#include <stdint.h>
#include <string>
#include <stdlib.h>
namespace crpc_util{

class Slice {

};

}


namespace slice_detail{
    using crpc_util::crpc_slice;
    
   
    static constexpr size_t kMinIntToStringSize = 4 * sizeof(uint64_t);

    /// @brief  kind like only moveable class
    /// own a slice and can take it out
    class BaseSlice
        :public crpc_core::NonCopyMoveable
    {
    public:

        /// @brief not use the const 
        /// bacause it will change the immutability of the base slice
        /// @return start ptr of slice 
        uint8_t *begin(){
            return slice_.StartPtr();
        } 
        
        const uint8_t *cbegin() const{
            return slice_.StartPtr();
        }   

        uint8_t* end() {
            return slice_.End();
        }

        const uint8_t * cend() const {
            return slice_.End();
        }
        
        const crpc_slice& c_slice() const{
            return slice_;
        }

        /// @brief  take the slice and put the inner slice to null
        /// @return 
        crpc_slice TakeCSlice(){
            crpc_slice out = slice_;
            slice_ = EmptySlice();
            return out;
        }

        std::string_view as_string_view() const{
            return std::string_view{reinterpret_cast<char*>(slice_.StartPtr()),slice_.Len()};
        } 

        uint8_t operator[](size_t i) const{
            return slice_.StartPtr()[i];
        }

        const uint8_t* data() const{
            return slice_.StartPtr();
        }

        size_t size() const{
            return slice_.Len();
        }


        size_t length() const{
            return slice_.Len();
        }        

        bool empty() const{
            return size() == 0;
        }

        bool is_equivalent(const BaseSlice &other) const{
            return slice_.Equal(other.slice_); 
        }

        uint32_t Hash() const {
            return slice_.Hash();
        }   



    protected:
        BaseSlice()
            :slice_(EmptySlice())
        {
        }

        explicit BaseSlice(const crpc_slice &slice)
            :slice_(slice){} 
        
        ~BaseSlice() = default;

        void Swap(BaseSlice& other) {
            std::swap(other.slice_,slice_);
        }

        void SetCSlice(const crpc_slice &slice){
            slice_ = slice;
        }

        uint8_t* mutable_data(){
            return slice_.StartPtr();
        }

        crpc_slice *c_slice_ptr(){
            return &slice_;
        }


    private:
        crpc_slice slice_;
    };

    inline bool operator==(const BaseSlice &a,const BaseSlice &b){
        return a.c_slice().Equal(b.c_slice());
    }

    inline bool operator != (const BaseSlice &a,const BaseSlice &b){
        return !(operator==(a,b));
    }
    
    inline bool operator == (const BaseSlice &a,std::string_view b){
        return a.as_string_view() == b;
    }

    inline bool operator != (const BaseSlice &a,std::string_view b){
        return !(operator==(a,b));
    }

    inline bool operator==(std::string_view a, const BaseSlice& b) {
        return a == b.as_string_view();
    }

    inline bool operator!=(std::string_view a, const BaseSlice& b) {
        return a != b.as_string_view();
    }

    inline bool operator==(const BaseSlice& a, const crpc_slice& b) {
        return a.c_slice().Equal(b);
    }

    inline bool operator!=(const BaseSlice& a, const crpc_slice& b) {
        return !(operator==(a,b));
    }

    inline bool operator==(const crpc_slice& a, const BaseSlice& b) {
        return a.Equal(b.c_slice());
    }

    inline bool operator!=(const crpc_slice& a, const BaseSlice& b) {
        return !(operator==(a,b));
    }

template<typename Out>
struct CopyConstructors{
    
    static Out FromCopiedString(const char * s){
        /// To be caution of the s ,must end of '\0'
        return  FromCopiedBuffer(s,stelen(s));
    }

    static Out FromCopiedString(std::string_view stv){
        return FromCopiedBuffer(stv.data(),stv.size());
    }   

    static Out FromCopiedString(std::string s) {
        return crpc_slice_from_cpp_string(std::move(s));
    }

    static Out FromCopiedBuffer(const char * p,size_t len){
        return crpc_slice_from_copied_buffer(p,len);
    }

    
    static Out FromCopiedBuffer(const uint8_t* p,size_t len){
        return FromCopiedBuffer(reinterpret_cast<const char*>(p),len);
    }

    /// @brief actually we only need this
    /// the strig version is generate autoly
    /// @tparam Buffer 
    /// @param buffer 
    /// @return 
    template<typename Buffer>
    static Out FromCopiedBuffer(const Buffer & buffer){
        return FromCopiedBuffer(reinterpret_cast<const char*> (buffer.data()),buffer.size());
    }

    static Out FromInt64(int64_t i){
        /// new is not allowed here
        char tmpBuffer[kMinIntToStringSize];
        ltoa(i,tmpBuffer,10);
        return FromCopiedBuffer(tmpBuffer);
    }



};

}


