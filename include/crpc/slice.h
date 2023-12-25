//
// Created by czy on 2023/12/24.
//

#ifndef CZYSERVER_SLICE_H
#define CZYSERVER_SLICE_H

#include <string>
#include "non_copymoveable.h"
#include "crpc/impl/slice_type.h"


static constexpr crpc_util::crpc_slice EmptySlice();

crpc_util::crpc_slice crpc_slice_malloc_large(size_t length);

crpc_util::crpc_slice crpc_slice_malloc(size_t length);

crpc_util::crpc_slice crpc_slice_from_copied_buffer(const char *source,size_t len);

crpc_util::crpc_slice crpc_slice_from_cpp_string(std::string s);


crpc_util::crpc_slice crpc_slice_sub_no_ref(crpc_util::crpc_slice source,size_t begin,size_t end);

crpc_util::crpc_slice crpc_slice_copy(crpc_util::crpc_slice source);

namespace crpc_util{
    struct CrpcSliceRefcount;
}

using crpc_util::crpc_slice;
using crpc_util::CrpcSliceRefcount;
namespace slice_detail{

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
        /// a bit like move operator
        /// @return
        crpc_slice TakeCSlice();


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
        BaseSlice();

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

        static Out FromCopiedString(const char * s);

        static Out FromCopiedString(std::string_view stv){
            return Out{FromCopiedBuffer(stv.data(),stv.size())};
        }

        static Out FromCopiedString(std::string s);

        static Out FromCopiedBuffer(const char * p,size_t len);


        static Out FromCopiedBuffer(const uint8_t* p,size_t len){
            return  Out{FromCopiedBuffer(reinterpret_cast<const char*>(p),len)};
        }

        /// @brief actually we only need this
        /// the strig version is generate autoly
        /// @tparam Buffer
        /// @param buffer
        /// @return
        template<typename Buffer>
        static Out FromCopiedBuffer(const Buffer & buffer){
            return Out{FromCopiedBuffer(reinterpret_cast<const char*> (buffer.data()),buffer.size())};
        }

        static Out FromInt64(int64_t i){
            /// new is not allowed here
            char tmpBuffer[kMinIntToStringSize];
            snprintf(tmpBuffer, kMinIntToStringSize, "%d", i);
            return Out{FromCopiedBuffer(tmpBuffer)};
        }

    };


    template<typename Out>
    struct StaticConstruct{
        static Out FromStaticString(const char * s);

        static Out FromStaticString(std::string_view s){
            return Out{FromStaticBuffer(s.data(),s.size())};
        }

        static Out FromStaticBuffer(const void * s,size_t len);

    };

}//end of namespace slice_detail

/// a static slice is slice never own refcount
/// and the life time must be ruled by user
/// not delete the raw mem before u use the static slice over
class StaticSlice
        :public slice_detail::BaseSlice,
         public slice_detail::StaticConstruct<StaticSlice>{
public:
    StaticSlice() = default;

    explicit  StaticSlice(const crpc_slice &slice);

    StaticSlice(const StaticSlice &rhs);

    StaticSlice& operator=(const StaticSlice &rhs);

    StaticSlice(StaticSlice &&rhs) noexcept;

    StaticSlice& operator=(StaticSlice &&rhs) noexcept;


};

class MutableSlice
        :public slice_detail::BaseSlice,
         public slice_detail::CopyConstructors<MutableSlice>
{

public:

    MutableSlice() = default;

    explicit  MutableSlice(const crpc_slice &slice);

    ~MutableSlice();

    MutableSlice(MutableSlice &&rhs) noexcept;

    MutableSlice& operator=(MutableSlice &&rhs) noexcept;

    static MutableSlice CreateUninitialized(size_t len) ;

    MutableSlice TakeSubSlice(size_t pos,size_t len);

    uint8_t* begin();

    uint8_t * end();

    uint8_t* data();

    //// no forget the &
    uint8_t& operator[](size_t i);

};

class Slice
        :public slice_detail::BaseSlice,
         public slice_detail::CopyConstructors<Slice>{

public:
    Slice() = default;

    ~Slice();

    /// use const reference to recive the r value
    explicit Slice(const crpc_slice &slice)
            :slice_detail::BaseSlice(slice)
    {
    }

    explicit Slice(Slice &&rhs) noexcept
            :slice_detail::BaseSlice(rhs.TakeCSlice())
    {

    }

    Slice& operator=(Slice &&rhs) noexcept;

    Slice TakeOwned();

    ///
    Slice AsOwned() const;

    MutableSlice TakeMutable();

    void Ref();

    Slice FromRefCountAndBytes(CrpcSliceRefcount &refCount, const uint8_t* begin,const uint8_t *end);
};




#endif //CZYSERVER_SLICE_H
