//
// Created by czy on 2024/1/10.
//

#ifndef CZYSERVER_POLL_H
#define CZYSERVER_POLL_H
#include <string>
#include <utility>
#include <cassert>
#include "core/lib/cprpp/construct_and_destruct.h"

namespace crpc_core{

    struct Pending{};

    inline bool operator == (const Pending &,const Pending &){
        return true;
    }

    struct Empty{};
    inline bool operator == (const Empty & lhs,const Empty &rhs) {
        return true;
    }


    template <typename T>
    class Poll{

    public:
        Poll(Pending ):ready_(false){}

        Poll() :ready_(false){}

        Poll(const Poll &rhs )
            :ready_(rhs.ready_)
        {
            if(ready_){
                Construct(&value_,rhs.value_);
            }
        }

        Poll(Poll && rhs) noexcept
            :ready_(rhs.ready_)
        {
            if(ready_){
                Construct(&value_,std::move(rhs.value_));
            }
        }



        Poll& operator=(const Poll & rhs ) {
            if(ready_){
                if(rhs.ready_){
                    value_ = rhs.value_;
                }
                else{
                    Destruct(&value_);
                    ready_ = false;
                }
            }
            else if(rhs.ready_){
                /// in here we only need Construct
                Construct(&value_,rhs.value_);
                ready_ = true;
            }
            return *this;
        }



        Poll& operator=(Poll &&rhs) noexcept{
            if(ready_){
                if(rhs.ready_){
                    value_ = std::move(rhs.value_);
                }
                else{
                    Destruct(&value_);
                    ready_ = false;
                }
            }
            else if(rhs.ready_){
                /// in here we only need Construct
                Construct(&value_,std::move(rhs.value_));
                ready_ = true;
            }
            return *this;
        };

        template<typename U>
        Poll(U value)
            :ready_(true)
        {
            Construct(&value_,std::move(value));
        }


        /// this is a r value version
        Poll(T && value)
            :ready_(true),value_(std::move(value))
        {
        }

        Poll(const T &value)
            :ready_(true),value_(value)
        {

        }

        ~Poll() {
            if(ready_){
                Destruct(value_);
            }
        }

        bool Pending() const{
            return !ready_;
        }

        bool Ready() const{
            return ready_;
        }

        T &Value(){
            assert(Ready());
            return value_;
        }

        const T& Value() const{
            assert(Ready());
            return value_;
        }

        T * ValueIfReady(){
            if(ready_){
                return &ready_;
            }
            return nullptr;
        }

        const T* ValueIfReady() const{
            if(ready_){
                return &value_;
            }
            return nullptr;
        }



    private:

        bool ready_;

        //// a very good skill
        union {
            T value_;
        };
    };

    template <>
    class Poll<Empty>{
    public:
        Poll(Pending ):ready_(false){}

        Poll() :ready_(false){}

        Poll(const Poll &rhs ) = default;

        Poll& operator=(const Poll & rhs ) = default;

        Poll& operator=(Poll &&rhs) noexcept = default;

        Poll(Poll &&rhs ) noexcept = default;


        Poll(Empty)
            :ready_(true)
        {}

        ~Poll() = default;

        bool Ready() const{
            return ready_;
        }

        Empty Value() const{
            assert(Ready());
            return Empty{};
        }

        Empty * ValueIfReady(){
            static Empty value;
            if(ready_){
                return &value;
            }
            return nullptr;
        }

        const Empty* ValueIfReady() const{
            static Empty value;
            if(ready_){
                return &value;
            }
            return nullptr;
        }

    private:

        bool ready_;
    };


    ///skill

    //// void Pending Poll
    template <>
    class Poll<Pending>;
    ///void poll<poll>
    template <typename T>
    class Poll<Poll<T>>;

    template <typename T>
    struct PollTraits{
        using Type = T;
        static constexpr bool is_poll() {return false;}
    };


    template <typename T>
    struct PollTraits<Poll<T>>{
        using Type = T;
        static constexpr bool is_poll() {return true;}
    };

    template <typename T>
    bool operator==(const Poll<T> &lhs,const Poll<T> &rhs){
        if(lhs.Pending() && rhs.Pending()){
            return true;
        }
        if(lhs.Ready() && rhs.Ready()){
            return lhs.Value() == rhs.Value();
        }
        return false;
    }

    template <typename T,typename U,typename SfinaeVoid = void>
    struct PollCastImpl;

    template<typename T ,typename U>
    struct PollCastImpl<T,Poll<U>>{
        static Poll<T> Cast(Poll<U> && poll){
            if(poll.Pending()){
                return Pending{};
            }
            return static_cast<T>(std::move(poll.value()));
        }
    };

    template <typename T,typename U>
    struct PollCastImpl<T,U,std::enable_if<!PollTraits<U>::is_poll()>>
    {
        static Poll<T> Cast(U &&poll){
            return Poll<T> (T(std::move(poll)));
        }
    };

    template <typename T>
    struct PollCastImpl<T,T>{
        static Poll<T> Cast(T && poll){
            return Poll<T>(T(std::move(poll)));
        }
    };

    template <typename T>
    struct PollCastImpl<T,Poll<T>>{
        static Poll<T> Cast(Poll<T> && poll){
            return (std::move(poll));
        }
    };

    template <typename T,typename U>
    Poll<T> poll_cast(U poll){
        return PollCastImpl<T,U>::Cast(std::move(poll));
    }

    template <typename T,typename F>
    std::string PollToString(
            const Poll<T> &poll, F t_to_string = [](const T & t){
                return t.ToString();
            }
    )
    {
        if(poll.Pending()){
            return "<pending>";
        }
        else{
            return t_to_string(poll.Value());
        }
    }

}

#endif //CZYSERVER_POLL_H
