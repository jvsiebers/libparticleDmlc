#ifndef MCDOSE_PARTICLE_DMLC_EGSNRC_SOURCE_V1_H
#define MCDOSE_PARTICLE_DMLC_EGSNRC_SOURCE_V1_H

#include "mcdose_particle_dmlc_egsnrc_adapter_v1.h"
#include "mcdose_particle_dmlc_source_output_correction_v1.h"
#include "mcdose_particle_dmlc_startup_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mcdose_particle_dmlc_egsnrc_source_context_v1
    mcdose_particle_dmlc_egsnrc_source_context_v1;

typedef struct mcdose_particle_dmlc_egsnrc_source_result_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t has_entry;
    uint32_t remaining_entry_count;
    int32_t product_kind;
    uint32_t reserved;
    mcdose_particle_dmlc_egsnrc_stack_entry_v1 entry;
} mcdose_particle_dmlc_egsnrc_source_result_v1;

/*
 * Creates one source-owned context from the strict startup artifact and binds
 * it to the host's exact-consumption random stream. The context owns the
 * producer; startup parsing storage is already released on return.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_create_egsnrc_source_from_startup_v1(
    const char *path,
    double electron_rest_mass_mev,
    mcdose_particle_dmlc_host_random_callback_v1 random_callback,
    void *random_user_data,
    mcdose_particle_dmlc_egsnrc_source_context_v1 **context,
    mcdose_particle_dmlc_startup_info_v1 *startup_info,
    char *diagnostic,
    size_t diagnostic_capacity);

/*
 * Creates the same source with one separately versioned, startup-bound output
 * correction. The correction is evaluated at the producer's exact sampled jaw
 * state and applied once before retained products enter the EGSnrc stack.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_create_egsnrc_source_from_startup_with_output_correction_v1(
    const char *startup_path,
    const char *output_correction_path,
    double electron_rest_mass_mev,
    mcdose_particle_dmlc_host_random_callback_v1 random_callback,
    void *random_user_data,
    mcdose_particle_dmlc_egsnrc_source_context_v1 **context,
    mcdose_particle_dmlc_startup_info_v1 *startup_info,
    mcdose_particle_dmlc_source_output_correction_info_v1 *correction_info,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API void
mcdose_particle_dmlc_destroy_egsnrc_source_context_v1(
    mcdose_particle_dmlc_egsnrc_source_context_v1 *context);

/*
 * Translates one EGSnrc total-energy source entry, samples the exact supplied
 * fractional meterset once, and atomically prepares retained products. A new
 * incident entry is rejected until the previous product queue is drained.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_begin_egsnrc_source_particle_v1(
    mcdose_particle_dmlc_egsnrc_source_context_v1 *context,
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 *incident_entry,
    double fractional_meterset,
    uint64_t scattered_photon_particle_id,
    uint64_t electron_particle_id,
    mcdose_particle_dmlc_producer_summary_v1 *summary,
    char *diagnostic,
    size_t diagnostic_capacity);

/*
 * Returns one tagged EGSnrc total-energy entry. Region and latch are inherited
 * from the incident entry. The caller must drain until remaining_entry_count
 * is zero before requesting another BEAM particle.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_next_egsnrc_source_entry_v1(
    mcdose_particle_dmlc_egsnrc_source_context_v1 *context,
    mcdose_particle_dmlc_egsnrc_source_result_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity);

#ifdef __cplusplus
}
#endif

#endif
