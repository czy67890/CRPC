//
// Created by chenzeyu on 2024/1/12.
//

#ifndef CZYSERVER_ATM_H
#define CZYSERVER_ATM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <cstdint>
    typedef intptr_t crpc_atm;


/** Adds \a delta to \a *value, clamping the result to the range specified
    by \a min and \a max.  Returns the new value. */
    crpc_atm crpc_atm_no_barrier_clamped_add(crpc_atm* value, crpc_atm delta,
                                         crpc_atm min, crpc_atm max);

#ifdef __cplusplus
}
#endif
#endif //CZYSERVER_ATM_H
