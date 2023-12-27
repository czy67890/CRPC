//
// Created by czy on 2023/12/26.
//

#ifndef CZYSERVER_ENDPOINT_CONDIG_H
#define CZYSERVER_ENDPOINT_CONDIG_H
#include <string_view>
#include <string>

namespace crpc_event_engine{
    class EndPointConfig{
    public:
        virtual ~EndPointConfig() = default;

        virtual int GetInt(std::string_view config_name) = 0;

        virtual std::string GetString(std::string_view config_name) = 0;

    };


}


#endif //CZYSERVER_ENDPOINT_CONDIG_H
