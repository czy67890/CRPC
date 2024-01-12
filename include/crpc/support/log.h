//
// Created by chenzeyu on 2024/1/12.
//

#ifndef CZYSERVER_LOG_H
#define CZYSERVER_LOG_H
#include <stdarg.h>
#include <stdlib.h>

#ifdef __cplusplus
    extern  "C" {
#endif

    enum class crpc_log_severity{
        CRPC_LOG_SEVERITY_DEBUG,
        CRPC_LOG_SEVERITY_INFO,
        CRPC_LOG_SEVERITY_ERROR
    };

    const char * crpc_log_serverity_string(crpc_log_severity ser_level);

#define CRPC_DEBUG __FILE__,__LINE__,crpc_log_severity::CRPC_LOG_SEVERITY_DEBUG
#define CRPC_INFO __FILE__,__LINE__,crpc_log_severity::CRPC_LOG_SEVERITY_INFO
#define CRPC_ERROR __FILE__,__LINE__,crpc_log_severity::CRPC_LOG_SEVERITY_ERROR

    void crpc_log(const char * file,int line,crpc_log_severity level,const char *format,...);


    /// before c99 bool is not built-in the crpc_should_log
    bool crpc_should_log(crpc_log_severity level);

    void crpc_log_message(const char *file,int line,crpc_log_severity level,const char *message);

    void crpc_set_log_verbosity(crpc_log_severity min_level);

    void crpc_log_verbosity_init(void );

    struct crpc_log_func_args{
        const char *file;
        int line;
        crpc_log_severity severity;
        const char *message;
    };

    typedef void (*crpc_log_func)(crpc_log_func_args *arg);

    void crpc_set_log_function(crpc_log_func func);

    void crpc_assertion_failed(const char *filename,int line,const char *message);

#define CRPC_ASSERT(x)  \
    do{                 \
        if(!x) [[unlikely]]{           \
            crpc_assertion_failed(__FILE__,__LINE__,#x);            \
        }               \
    }while(0)

#ifndef NDEBUG
#define CRPC_DEBUG_ASSERT(x) CRPC_ASSERT(x)
#else
#define CRPC_DEBUG_ASSERT(x)
#endif



#ifdef __cplusplus
    }
#endif


#endif //CZYSERVER_LOG_H
