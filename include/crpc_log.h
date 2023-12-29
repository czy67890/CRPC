//
// Created by czy on 2023/12/28.
//

#ifndef CZYSERVER_CRPC_LOG_H
#define CZYSERVER_CRPC_LOG_H
#include <string>
#include <iostream>

#include <string_view>
#include "singletion.h"


namespace crpc_log{
    class Log{
    public:
         Log &operator<<(const std::string &str){
            std::cout<<str<<std::endl;
        }
    private:

    };

    Log& LogIns = crpc_core::Singletion<Log>::GetIns();

}


#endif //CZYSERVER_CRPC_LOG_H
