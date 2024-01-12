//
// Created by chenzeyu on 2024/1/11.
//

#ifndef CZYSERVER_EXEC_CTX_H
#define CZYSERVER_EXEC_CTX_H


typedef struct crpc_combiner crpc_combiner;




namespace crpc_core{

    enum class ExecFlag{
        kIsFinished = 1,
        kResourceLoop = 2,
        kInternalThread = 4
    };

    class Combiner;

    class ExecCtx {
    public:
        ExecCtx(){

        }

    private:
        static thread_local ExecCtx * exec_ctx_;


    };

}
#endif //CZYSERVER_EXEC_CTX_H
//uf