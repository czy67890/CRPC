//
// Created by chenzeyu on 2024/1/14.
//

#ifndef CZYSERVER_CPPLOG_H
#define CZYSERVER_CPPLOG_H
#include "crpc/any_invocable.h"

#include "non_copyable.h"
#include <cstdio>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>


namespace crpc_core{
    void DefaultOutputFunc(const char *message){
        printf("%s",message);
    }
    using crpc_function::AnyInvocable;

    class LogCore :public NonCopyable{
    public:

        enum class crpc_log_severity{
            CRPC_LOG_SEVERITY_DEBUG,
            CRPC_LOG_SEVERITY_INFO,
            CRPC_LOG_SEVERITY_ERROR,
            CRPC_LOG_SEVERITY_UNSET
        };

        explicit LogCore(crpc_log_severity server_level)
            :server_level_(server_level)
        {
            WriteTimeAndLevel();
        }


        ~LogCore(){
            stream_<<"\n";
            output_func_(std::move(*stream_.rdbuf()).str().c_str());
        }


        std::stringstream &stream(){
            return  stream_;
        }

        static void SetLogFunc(AnyInvocable<void(const char * message)> func){
            output_func_ = std::move(func);
        }

    private:
        void FillWithLevel(char * buffer){
            switch (server_level_)
            {
                case crpc_log_severity::CRPC_LOG_SEVERITY_INFO:
                    memcpy(buffer,"[INFO]  ",8);
                    return;
                case crpc_log_severity::CRPC_LOG_SEVERITY_DEBUG:
                    memcpy(buffer,"[DEBUG] ",8);
                    return;
                case crpc_log_severity::CRPC_LOG_SEVERITY_ERROR:
                    memcpy(buffer,"[ERROR] ",8);
                    return;
                case crpc_log_severity::CRPC_LOG_SEVERITY_UNSET:
                    memcpy(buffer,"[UNSET] ",8);
                    return;
                    
            }
        }

        void WriteTimeAndLevel(){
            auto nowtp = std::chrono::system_clock::now();
            std::time_t time = std::chrono::system_clock::to_time_t(nowtp);
            auto dismiss = std::chrono::system_clock::from_time_t(time);
            auto gap = std::chrono::duration_cast<std::chrono::milliseconds>(nowtp - dismiss).count();
            std::tm* time_info = std::localtime(&time);

            char stack_buff[kTimeLevelLen];
            FillWithLevel(stack_buff);
            std::strftime(stack_buff + 8, kTimeLevelLen, "%Y-%m-%d %H:%M:%S", time_info);
            snprintf(stack_buff + 27, kTimeLevelLen - 27, ":%03ld", gap);

            stream_.write(stack_buff, static_cast<size_t>(strlen(stack_buff)));
            stream_.write("  ", 2);
        }

    private:
        crpc_log_severity server_level_ {crpc_log_severity::CRPC_LOG_SEVERITY_UNSET};
        static crpc_function::AnyInvocable<void(const char * message)> output_func_;
        std::stringstream stream_;
        static constexpr size_t kTimeLevelLen = 48;
    };


#define LOG_DEBUG crpc_core::LogCore(crpc_core::LogCore::crpc_log_severity::CRPC_LOG_SEVERITY_DEBUG).stream()
#define LOG_INFO crpc_core::LogCore(crpc_core::LogCore::crpc_log_severity::CRPC_LOG_SEVERITY_INFO).stream()
#define LOG_ERROR crpc_core::LogCore(crpc_core::LogCore::crpc_log_severity::CRPC_LOG_SEVERITY_ERROR).stream()


    crpc_function::AnyInvocable<void(const char *)> LogCore::output_func_ = DefaultOutputFunc;

}
#endif //CZYSERVER_CPPLOG_H
