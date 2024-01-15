//
// Created by chenzeyu on 2024/1/12.
//
#include "crpc/support/log.h"
#include <cstdio>
#include <cstring>
#ifndef CRPC_DEFAULT_LOG_VERBOSITY_STRING
#define CRPC_DEFAULT_LOG_VERBOSITY_STRING "ERROR"
#endif  // !GPR_DEFAULT_LOG_VERBOSITY_STRING


void crpc_default_log(crpc_log_func_args *args);

static void (*g_log_func)(crpc_log_func_args *) = crpc_default_log;
static crpc_log_severity g_min_severity_to_print = crpc_log_severity::CRPC_LOG_SEVERITY_UNSET;
static crpc_log_severity g_min_severity_to_print_stack =  crpc_log_severity::CRPC_LOG_SEVERITY_UNSET;

void crpc_set_log_function(crpc_log_func func){
    g_log_func = func;
}

void crpc_assertion_failed(const char *filename,int line,const char *message){
    abort();
}



/// before c99 bool is not built-in the crpc_should_log
bool crpc_should_log(crpc_log_severity level){
    return level >= g_min_severity_to_print;
}


void crpc_log_message(const char *file,int line,crpc_log_severity level,const char *message){
    if(!crpc_should_log(level)) {
        return;
    }
    crpc_log_func_args args;
    memset(&args,0,sizeof(args));
    args.file = file;
    args.line = line;
    args.severity = level;
    args.message = message;
    (*g_log_func)(&args);
}

void crpc_set_log_verbosity(crpc_log_severity min_level){
    g_min_severity_to_print = min_level;
    g_min_severity_to_print_stack = min_level;
}

void crpc_log_verbosity_init(){
    if(g_min_severity_to_print == crpc_log_severity::CRPC_LOG_SEVERITY_UNSET){
        ///TODO :: now this is set to dead
        /// consider to change it by complete config define
        g_min_severity_to_print = crpc_log_severity::CRPC_LOG_SEVERITY_DEBUG;
    }
    if(g_min_severity_to_print_stack == crpc_log_severity::CRPC_LOG_SEVERITY_UNSET){
        g_min_severity_to_print_stack = crpc_log_severity::CRPC_LOG_SEVERITY_DEBUG;
    }

}

const char * crpc_log_serverity_string(crpc_log_severity ser_level){
    switch(ser_level){
        case crpc_log_severity::CRPC_LOG_SEVERITY_DEBUG:
            return "D";
        case crpc_log_severity::CRPC_LOG_SEVERITY_INFO:
            return "I";
        case crpc_log_severity::CRPC_LOG_SEVERITY_ERROR:
            return "E";
        case crpc_log_severity::CRPC_LOG_SEVERITY_UNSET:
            return "UNSET";
    }
}