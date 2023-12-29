//
// Created by czy on 2023/12/28.
//

#ifndef CZYSERVER_THREAD_LOCAL_H
#define CZYSERVER_THREAD_LOCAL_H
namespace crpc_event_engine{

    class ThreadLocal{
    public:
        static void SetIsEventEngineThreadLocal(bool is);

        static bool GetIsEventEngineThreadLocal();

    };

}
#endif //CZYSERVER_THREAD_LOCAL_H
