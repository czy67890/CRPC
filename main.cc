//
// Created by czy on 2023/12/24.
//

#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include <assert.h>
#include "crpc/any_invocable.h"
#include "core/lib/event_engine/thread_pool/thread_pool.h"

using namespace std;
std::atomic<int> val;
void threadFunc(){
    val++;
    std::this_thread::sleep_for(3s);
    std::cout<<"thread"<<std::this_thread::get_id()<<endl;
}

int main(){
    std::unique_ptr<int> ptr(new int(30));
    std::array<int,40> arr{};
    crpc_function::AnyInvocable<void()> func = [arr,ptr = std::move(ptr)](){
        cout<<*ptr<<endl;
        for(auto i : arr){
            cout<<i<<endl;
        }
    };
    func();
    return 0;
}
