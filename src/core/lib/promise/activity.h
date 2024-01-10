//
// Created by czy on 2024/1/9.
//

#ifndef CZYSERVER_ACTIVITY_H
#define CZYSERVER_ACTIVITY_H

#include <stdint.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <functional>


#include "core/lib/cprpp/no_destruct.h"

namespace crpc_core{

    class Activity;

    using WakeupMask = uint64_t;

    class Wakeable{
    public:
        virtual void Wakeup(WakeupMask wakeup_mask) = 0;

        virtual void WakeupAsync(WakeupMask wakeup_mask) = 0;

        virtual void Drop(WakeupMask wakeup_mask) = 0;

        virtual std::string ActivityDebugTag(WakeupMask wakeup_mask) const{
            return "no implement now ,please check the source code";
        };
    protected:
        virtual ~Wakeable(){};
    };

    namespace promise_detail{
        struct UnWakeable final :public Wakeable{
            void Wakeup(WakeupMask wakeup_mask){};

            void WakeupAsync(WakeupMask wakeup_mask){};

            void Drop(WakeupMask wakeup_mask) {};

            std::string ActivityDebugTag(WakeupMask wakeup_mask) const override{return std::string{};}
        };

        static UnWakeable *  unwakeable() {
            return NoDestructSingletion<UnWakeable>::get();
        }
    }


    class Waker{

    public:
        Waker()
            :wakeable_and_arg_(promise_detail::unwakeable(),0)
        {}

        Waker(Wakeable *wakeable,WakeupMask mask)
        : wakeable_and_arg_(wakeable,mask){}

        ~Waker(){
            wakeable_and_arg_.Drop();
        }

        Waker(const Waker &rhs) = delete;

        Waker& operator=(const Waker & rhs ) = delete;

        Waker(Waker&& rhs) noexcept
            : wakeable_and_arg_(rhs.Take())
        {}

        Waker& operator=(Waker &&rhs) noexcept{
            Waker tmp(std::move(rhs));
            wakeable_and_arg_ = {tmp.Take()};
            return *this;
        }

        void Wakeup(){
            Take().Wakeup();
        }

        void WakeupAsync(){
            Take().WakeupAsync();
        }

        size_t GetHashRes() const {
            return (std::hash<WakeupMask>()(wakeable_and_arg_.wakeup_mask)
            <<16) || (std::hash<Wakeable*>()(wakeable_and_arg_.wakeable) >> 16);
        }


        bool operator == (const Waker & rhs) const{
            return   wakeable_and_arg_ == rhs.wakeable_and_arg_;
        }

        bool operator != (const Waker &rhs) const {
            return !(operator==(rhs));
        }

        bool IsUnWakeable() const{
            /// this is a not null value
            /// and unwakeable have a permanent addr
            return  wakeable_and_arg_.wakeable == promise_detail::unwakeable();
        }

    private:
        struct WakeableAndArg{
            Wakeable *wakeable;
            WakeupMask wakeup_mask;

            void Wakeup(){
                if(wakeable) {
                    wakeable->Wakeup(wakeup_mask);
                }
            }

            void WakeupAsync(){
                if(!wakeable){
                    return;
                }
                wakeable->Wakeup(wakeup_mask);
            }


            bool operator==(const WakeableAndArg &rhs) const{
                return wakeable == rhs.wakeable && wakeup_mask == rhs.wakeup_mask;
            }

            void Drop() {
                if(!wakeable) return;
                wakeable->Drop(wakeup_mask);
            }
        };

        WakeableAndArg Take(){
            return std::exchange(wakeable_and_arg_,{nullptr,0});
        }

        WakeableAndArg wakeable_and_arg_;
    };


    class IntraActivityWaiter{
    public:


    };



}



namespace std{
    template<>
    struct hash<crpc_core::Waker>{
        size_t operator()(const crpc_core::Waker &waker) const{
            return waker.GetHashRes();
        }
    };
}

#endif //CZYSERVER_ACTIVITY_H
