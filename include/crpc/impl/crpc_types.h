//
// Created by czy on 2024/1/17.
//

#ifndef CZYSERVER_CRPC_TYPES_H
#define CZYSERVER_CRPC_TYPES_H

struct crpc_completion_queue_functor{
    void (*functor_to_run)(struct crpc_completion_queue_functor * ,int);
    bool inlineable;
    int internal_success;
    struct crpc_completion_queue_functor* internal_next;
};


#endif //CZYSERVER_CRPC_TYPES_H
