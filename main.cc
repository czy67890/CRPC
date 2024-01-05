//
// Created by czy on 2023/12/24.
//

#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include <assert.h>
#include <unordered_set>
#include "crpc/any_invocable.h"
#include "core/lib/event_engine/thread_pool/thread_pool.h"
#include "core/lib/cprpp/ref_count_ptr.h"
using namespace std;
std::atomic<int> val;
void threadFunc(){
    val++;
    std::this_thread::sleep_for(3s);
    std::cout<<"thread"<<std::this_thread::get_id()<<endl;
}

class SomeClass{
public:

    void IncrementRefCount(){

    }

    void UnRef(){

    }

};

int main(){
    std::unordered_set<crpc_core::RefCountedPtr<SomeClass>> s;
    auto res = crpc_core::MakeRefCounted<SomeClass>();
    s.insert( res);
    return 0;
}
