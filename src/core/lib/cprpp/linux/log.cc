//
// Created by chenzeyu on 2024/1/13.
//

#include "crpc/support/log.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include <string>

void crpc_log(const char * file,int line,crpc_log_severity level,const char *format,...){
    if(!crpc_should_log(level)){
        return;
    }
    char *message = nullptr;
    va_list args;
    va_start(args,format);

    if(vasprintf(&message,format,args) == -1){
        va_end(args);
        return;
    }

    va_end(args);
    crpc_log_message(file,line,level,message);
    free(message);
}

void crpc_default_log(crpc_log_func_args *args){
    const char *final_slash;
    const char *display_file;
    char time_buf[64];
    time_t timer;
    
}

