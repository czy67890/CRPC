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


#include "core/lib/cprpp/no_destruct.h"

namespace crpc_core{

    class Activity;

    using WakeupMask = uint64_t;

    class Wakeable{
    public:
        virtual void Wakeup(WakeupMask wakeup_mask) = 0;

        virtual void WakeupAsync(WakeupMask wakeup_mask) = 0;

        virtual void Drop(WakeupMask wakeup_mask) = 0;

        virtual std::string ActivityDebugTag(WakeupMask wakeup_mask) const = 0;
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


            bool operator==(const WakeableAndArg &rhs){
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




}

#endif //CZYSERVER_ACTIVITY_H
