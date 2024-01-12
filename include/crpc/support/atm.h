//
// Created by chenzeyu on 2024/1/12.
//

#ifndef CZYSERVER_ATM_H
#define CZYSERVER_ATM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Adds \a delta to \a *value, clamping the result to the range specified
    by \a min and \a max.  Returns the new value. */
gpr_atm gpr_atm_no_barrier_clamped_add(gpr_atm* value, gpr_atm delta,
                                       gpr_atm min, gpr_atm max);

#ifdef __cplusplus
}
#endif
#endif //CZYSERVER_ATM_H
