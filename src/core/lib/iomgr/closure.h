//
// Created by chenzeyu on 2024/1/11.
//

#ifndef CZYSERVER_CLOSUREIO_H
#define CZYSERVER_CLOSUREIO_H
#include "crpc/support/cpplog.h"
#include "crpc/status.h"
#include "core/lib/cprpp/mpscq.h"
#include "core/lib/cprpp/manual_constructor.h"

struct crpc_closure;
typedef crpc_closure crpc_closure;

struct crpc_closure_list{
    crpc_closure *head;
    crpc_closure *tail;
};

typedef void (*crpc_iomgr_cb_func)(void *arg,Status error);

struct crpc_closure {
    union{
        crpc_closure *next;
        crpc_core::ManualConstructor<crpc_core::MPSCQueue::Node> mpscq_node;
        uintptr_t scratch;
    }next_data;
    crpc_iomgr_cb_func  cb;

    void *cb_arg;

    union {
        uintptr_t  error;
        uintptr_t  scratch;
    }error_data;

    bool scheduled;
    bool run;
    const char * file_created;
    int line_created;
    const char *file_initiated;
    int line_initiated;

    std::string DebugString() const;

};

inline crpc_closure * crpc_closure_init(const char *file,int line,crpc_closure *closure,crpc_iomgr_cb_func cb,void *cb_arg){
    closure->cb = cb;
    closure->cb_arg = cb_arg;
    closure->error_data.error = 0;
    closure->scheduled = false;
    closure->file_initiated = nullptr;
    closure->line_initiated = 0;
    closure->run = false;
    closure->file_created = file;
    closure->line_created = line;
}
#define CRPC_CLOSURE_INIT(closure, cb, cb_arg, scheduler) \
  crpc_closure_init(__FILE__, __LINE__, closure, cb, cb_arg)
namespace crpc_core{
    template<typename T,void (T::*cb)(Status)>
    crpc_closure MakeMemberClosure(T * p){
        crpc_closure  out;
        CRPC_CLOSURE_INIT(&out,[](void *p,Status e){
            (static_cast<T*>(p)->*cb)(e);
        },p,nullptr);

        return out;
    }

    template<typename F>
    crpc_closure * NewClosure(F f){
        struct Closure :public crpc_closure {
            explicit Closure(F f)
                :f(std::move(f))
            {
            }
            F f;
            static void Run(void *arg,Status error){
                auto self = static_cast<Closure*>(arg);
                self->f(error);
                delete self;
            }
        };

        Closure * c = new Closure(std::move(f));
        CRPC_CLOSURE_INIT(c,Closure::Run,c,nullptr);
        return c;
    }
}

namespace closure_impl{
    struct wrapped_closure{
        crpc_iomgr_cb_func cb;
        void *cb_arg;
        crpc_closure wrapper;
    };

    inline void closure_wrapper(void *arg,Status error){
        wrapped_closure *wc = static_cast<wrapped_closure*>(arg);
        crpc_iomgr_cb_func cb = wc->cb;
        void *wc_arg = wc->cb_arg;
        free(wc);
        cb(wc_arg,error);
    }
}

inline crpc_closure * crpc_closure_create(const char * file,int line,crpc_iomgr_cb_func cb,void *cb_arg){
    closure_impl::wrapped_closure * wc = static_cast<closure_impl::wrapped_closure*>(malloc(sizeof(*wc)));
    wc->cb = cb;
    wc->cb_arg = cb_arg;
    crpc_closure_init(file,line,&wc->wrapper,closure_impl::closure_wrapper,wc);
    return &wc->wrapper;
}


#define CRPC_CLOSURE_CREATE(cb, cb_arg, scheduler) \
  crpc_closure_create(__FILE__, __LINE__, cb, cb_arg)

#define CRPC_CLOSURE_LIST_INIT {nullptr,nullptr}

inline void crpc_closure_list_init(crpc_closure_list *closure_list){
    closure_list->head = closure_list->tail = nullptr;
}


inline bool crpc_closure_list_append(crpc_closure_list * closure_list,crpc_closure *closure){
    if(closure == nullptr){
        return false;
    }
    closure->next_data.next = nullptr;
    bool was_empty = (!closure_list->head);
    if(was_empty){
        closure_list->head = closure;
    }
    else{
        closure_list->tail->next_data.next = closure;
    }
    closure_list->tail = closure;
    return was_empty;
}

inline bool crpc_closure_list_append(crpc_closure_list * closure_list,crpc_closure *closure,Status error){
    if(!closure){
        return false;
    }
    closure->error_data.error = (uintptr_t)error;
    return crpc_closure_list_append(closure_list,closure);
}

inline void crpc_closure_list_fail_all(crpc_closure_list * list,Status status){
    for(crpc_closure * c = list->head;c != nullptr;c = c->next_data.next){
        if(c->error_data.error == 0){
            c->error_data.error = (uintptr_t)status;
        }
    }
}

inline void crpc_closure_list_move(crpc_closure_list *src,crpc_closure_list *dst){
    if(src->head == nullptr){
        return ;
    }
    if(dst->head == nullptr){
        *dst = *src;
    }
    else{
        dst->tail->next_data.next = src->head;
        dst->tail = src->tail;
    }
    src->head = src->tail = nullptr;
}

inline bool crpc_closure_list_empty(crpc_closure_list closure_list){
    return closure_list.head == nullptr;
}

namespace crpc_core{
    class Closure{
    public:
        static void Run(crpc_closure * closure,Status error){
            if(!closure){
                return;
            }
            closure->cb(closure->cb_arg,error);
            LOG_DEBUG<<"closure finished";
        }
    };
}

#endif //CZYSERVER_CLOSUREIO_H
