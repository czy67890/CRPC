//
// Created by czy on 2023/12/24.
//
#include "crpc/slice.h"
#include "crpc/impl/slice_type.h"
#include "core/lib/slice/slice.h"
#include <iostream>
#include <string>
using namespace std;

int main(){

    crpc_util::crpc_slice slice = crpc_util::crpc_slice{};
    std::string s{"hello my slice"};
    return 0;
}
