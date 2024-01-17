//
// Created by czy on 2024/1/17.
//

#ifndef CZYSERVER_COMBINER_H
#define CZYSERVER_COMBINER_H
#include <memory>
#include "crpc/event_engine/event_engine.h"
#include "core/lib/cprpp/mpscq.h"
#include "core/lib/iomgr/exec_ctx.h"

namespace crpc_core{
    class Combiner{

    private:

    };

}

bool crpc_combiner_continue_exec_ctx();

#endif
// uf