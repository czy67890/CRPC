//
// Created by czy on 2023/12/28.
//

#ifndef CZYSERVER_SINGLETION_H
#define CZYSERVER_SINGLETION_H
#include <mutex>
namespace crpc_core{
    template<typename T>
    class Singletion {
    public:

        Singletion() = delete;

        Singletion(const Singletion&rhs) = delete;

        Singletion(Singletion &&rhs ) = delete;

        Singletion& operator=(const Singletion &rhs) = delete;

        Singletion& operator=( Singletion &&rhs) = delete;

        static T &GetIns(){
            if(!ins_ptr_) {
                Create();
            }
            return *ins_ptr_;
        }

        static T* GetPtr(){
            if(!ins_ptr_){
                Create();
            }
            return ins_ptr_;
        }




    private:

        static void Create(){

            std::call_once(init_flag_,[](){
                Singletion::ins_ptr_ = new T();
            });
            return;
        }

        static std::once_flag init_flag_;
        static T * ins_ptr_;
    };


    template<typename T>
    std::once_flag Singletion<T>::init_flag_{};

    template <typename T>
    T* Singletion<T>::ins_ptr_{nullptr};

}

#endif //CZYSERVER_SINGLETION_H
