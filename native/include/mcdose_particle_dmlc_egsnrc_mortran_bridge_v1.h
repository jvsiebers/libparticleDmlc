#ifndef MCDOSE_PARTICLE_DMLC_EGSNRC_MORTRAN_BRIDGE_V1_H
#define MCDOSE_PARTICLE_DMLC_EGSNRC_MORTRAN_BRIDGE_V1_H

#include "mcdose_particle_dmlc_egsnrc_adapter_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Host callback implemented by the optional EGSnrc-configured bridge target.
 * The target must be linked with a user code containing
 * mcdose_particle_dmlc_shower_v1 from the supplied Mortran source.
 * It also exports the configured Fortran symbol
 * mcdose_particle_dmlc_dispatch_shower_v1. That scalar entry point accepts
 * explicit lineage, product kind, and electron rest mass, then routes the
 * particle through the validated tagged dispatcher and this callback.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_egsnrc_mortran_shower_callback_v1(
    void *user_data,
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 *entry);

#ifdef __cplusplus
}
#endif

#endif
