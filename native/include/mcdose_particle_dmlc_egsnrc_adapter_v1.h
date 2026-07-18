#ifndef MCDOSE_PARTICLE_DMLC_EGSNRC_ADAPTER_V1_H
#define MCDOSE_PARTICLE_DMLC_EGSNRC_ADAPTER_V1_H

#include "mcdose_particle_dmlc_interaction_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV 0.5109989461

typedef struct mcdose_particle_dmlc_egsnrc_stack_context_v1
    mcdose_particle_dmlc_egsnrc_stack_context_v1;

typedef enum mcdose_particle_dmlc_egsnrc_product_kind_v1 {
    MCDOSE_PARTICLE_DMLC_EGSNRC_PRIMARY = 1,
    MCDOSE_PARTICLE_DMLC_EGSNRC_SCATTERED_PHOTON = 2,
    MCDOSE_PARTICLE_DMLC_EGSNRC_GENERATED_COMPTON_ELECTRON = 3
} mcdose_particle_dmlc_egsnrc_product_kind_v1;

/*
 * Host-neutral representation of one EGSnrc /STACK/ entry. EGSnrc stores
 * total energy for charged particles, whereas particleDmlc particle records
 * store kinetic energy for charged particles. Latch and lineage are explicit
 * sidecars because SHOWER receives neither as a formal argument.
 */
typedef struct mcdose_particle_dmlc_egsnrc_stack_entry_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    int32_t iq;
    int32_t region;
    int32_t latch;
    uint32_t reserved;
    uint64_t history_id;
    uint64_t particle_id;
    uint64_t parent_particle_id;
    uint64_t reserved_2;
    double total_energy_mev;
    double position_cm[3];
    double direction[3];
    double weight;
} mcdose_particle_dmlc_egsnrc_stack_entry_v1;

typedef struct mcdose_particle_dmlc_egsnrc_stack_result_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t has_entry;
    uint32_t remaining_entry_count;
    mcdose_particle_dmlc_egsnrc_stack_entry_v1 entry;
} mcdose_particle_dmlc_egsnrc_stack_result_v1;

typedef struct mcdose_particle_dmlc_egsnrc_dispatch_result_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t dispatched;
    uint32_t discarded_by_policy;
    int32_t product_kind;
    uint32_t reserved;
} mcdose_particle_dmlc_egsnrc_dispatch_result_v1;

/*
 * The host callback is the only operation permitted to cross into EGSnrc.
 * A Mortran shim sets /STACK/ LATCHI from entry->latch immediately before it
 * calls SHOWER with the remaining scalar values. The callback is synchronous
 * and must not retain the entry pointer.
 */
typedef int32_t (*mcdose_particle_dmlc_egsnrc_shower_callback_v1)(
    void *user_data,
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 *entry);

MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_particle_to_egsnrc_stack_entry_v1(
    const mcdose_particle_dmlc_particle_v1 *particle,
    int32_t region,
    int32_t latch,
    double electron_rest_mass_mev,
    mcdose_particle_dmlc_egsnrc_stack_entry_v1 *entry,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_egsnrc_stack_entry_to_particle_v1(
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 *entry,
    double electron_rest_mass_mev,
    mcdose_particle_dmlc_particle_v1 *particle,
    char *diagnostic,
    size_t diagnostic_capacity);

/*
 * Dispatches a tagged product through the host callback. The first-release
 * policy dispatches primary particles and scattered photons, but explicitly
 * discards generated Compton electrons without invoking the callback.
 * electron_rest_mass_mev must match the host EGSnrc stack convention.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_dispatch_egsnrc_product_v1(
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 *entry,
    int32_t product_kind,
    double electron_rest_mass_mev,
    mcdose_particle_dmlc_egsnrc_shower_callback_v1 callback,
    void *user_data,
    mcdose_particle_dmlc_egsnrc_dispatch_result_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity);

/*
 * The bounded LIFO context models EGSnrc stack ordering without accessing a
 * Fortran common block. A later Mortran/C bridge must copy these fields to the
 * real stack and set the common-block LATCHI value before calling SHOWER.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_create_egsnrc_stack_context_v1(
    uint32_t entry_capacity,
    double electron_rest_mass_mev,
    mcdose_particle_dmlc_egsnrc_stack_context_v1 **context,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API void
mcdose_particle_dmlc_destroy_egsnrc_stack_context_v1(
    mcdose_particle_dmlc_egsnrc_stack_context_v1 *context);

MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_push_egsnrc_stack_entry_v1(
    mcdose_particle_dmlc_egsnrc_stack_context_v1 *context,
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 *entry,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_pop_egsnrc_stack_entry_v1(
    mcdose_particle_dmlc_egsnrc_stack_context_v1 *context,
    mcdose_particle_dmlc_egsnrc_stack_result_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity);

#ifdef __cplusplus
}
#endif

#endif
