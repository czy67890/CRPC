//
// Created by czy on 2023/12/24.
//

#ifndef CZYSERVER_SLICE_TYPE_H
#define CZYSERVER_SLICE_TYPE_H
#include <cstring>
#include <string_view>
#include <functional>
#include <string>
#include <vector>


//namespace std{
//    template <>
//    struct hash<std::string_view>
//    {
//        static constexpr uint64_t kHashBase = 131;
//        static constexpr uint64_t kHashMod = 1e9 + 9;
//        size_t operator()(const std::string_view &stv) const{
//            //double hash to ensure the probability of hash collision
//            // to be the minium
//            /// this is the simpest implement
//            size_t len = stv.length();
//            uint64_t res = 0;
//            for(int i = 0; i < stv.length() ;++ i){
//                res = (res * kHashBase + (uint8_t)stv[i] ) %kHashMod;
//            }
//            return res;
//        }
//    };
//}

namespace crpc_util{

    struct CrpcSliceRefcount;

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




        void Unref();

        void Ref();

        uint8_t *StartPtr() const;

        const size_t Len() const;


        bool Equal(const crpc_slice &other) const;

        size_t Hash() const;

        void SetLen(size_t len);

        uint8_t* End() const;

        std::string_view StringView() const;



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


#endif //CZYSERVER_SLICE_TYPE_H
