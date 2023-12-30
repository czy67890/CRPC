//
// Created by czy on 2023/12/24.
//

#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include <assert.h>
#include "core/lib/event_engine/thread_pool/thread_pool.h"

using namespace std;
std::atomic<int> val;
void threadFunc(){
    val++;
    std::this_thread::sleep_for(3s);
    std::cout<<"thread"<<std::this_thread::get_id()<<endl;
}

int main(){
    cout<<"pc has "<<std::thread::hardware_concurrency()<<" core"<<endl;
    auto  thread_pool = crpc_event_engine::MakeThreadPool(std::thread::hardware_concurrency());
    for(int i = 0;i < 120;++i) {
        thread_pool->Run(threadFunc);
    }
    thread_pool->Quit();
    cout<<"all thing done "<<val.load()<<endl;
    return 0;
}
