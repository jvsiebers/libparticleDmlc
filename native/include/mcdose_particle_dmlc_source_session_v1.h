#ifndef MCDOSE_PARTICLE_DMLC_SOURCE_SESSION_V1_H
#define MCDOSE_PARTICLE_DMLC_SOURCE_SESSION_V1_H

#include "mcdose_particle_dmlc_producer_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mcdose_particle_dmlc_source_session_context_v1
    mcdose_particle_dmlc_source_session_context_v1;

/*
 * An incident-source callback supplies one source particle at a time. It may
 * return an explicit empty state to mark end of source. The callback owns its
 * state and may use any upstream RNG; the session never calls an RNG itself.
 */
typedef struct mcdose_particle_dmlc_source_incident_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t has_incident;
    uint32_t starts_new_history;
    double fractional_meterset;
    double z_last_cm;
    int32_t latch;
    int32_t photon_history;
    uint64_t scattered_photon_particle_id;
    uint64_t electron_particle_id;
    mcdose_particle_dmlc_particle_v1 particle;
} mcdose_particle_dmlc_source_incident_v1;

typedef int32_t (*mcdose_particle_dmlc_next_incident_callback_v1)(
    void *user_data,
    mcdose_particle_dmlc_source_incident_v1 *incident,
    char *diagnostic,
    size_t diagnostic_capacity);

typedef struct mcdose_particle_dmlc_source_session_config_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t reserved[4];
    /*
     * A null producer emits supplied particles unchanged. A non-null producer
     * is borrowed, must remain alive for the session lifetime, and applies the
     * commissioned MLC transport before results are emitted.
     */
    mcdose_particle_dmlc_producer_context_v1 *producer;
} mcdose_particle_dmlc_source_session_config_v1;

typedef struct mcdose_particle_dmlc_source_session_result_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t has_product;
    uint32_t remaining_product_count;
    int32_t product_kind;
    uint32_t starts_new_history;
    double fractional_meterset;
    double z_last_cm;
    int32_t latch;
    int32_t photon_history;
    uint64_t source_history_id;
    mcdose_particle_dmlc_particle_v1 particle;
} mcdose_particle_dmlc_source_session_result_v1;

typedef struct mcdose_particle_dmlc_source_session_summary_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t source_exhausted;
    uint32_t reserved;
    uint64_t incident_callback_count;
    uint64_t incident_particle_count;
    uint64_t observed_source_history_count;
    uint64_t emitted_product_count;
    uint64_t pass_through_product_count;
    uint64_t producer_incident_count;
    uint64_t producer_blocked_incident_count;
    uint64_t producer_retained_product_count;
    uint64_t producer_primary_retained_count;
    uint64_t producer_scattered_photon_retained_count;
    uint64_t producer_generated_electron_discarded_count;
    uint64_t producer_random_draw_count;
    uint64_t source_callback_failure_count;
    uint64_t incident_rejection_count;
    uint64_t producer_failure_count;
    uint64_t last_source_history_id;
    uint64_t reserved_counts[4];
} mcdose_particle_dmlc_source_session_summary_v1;

/*
 * Creates a scalar source session. It has no DICOM, EGSnrc, or BEAM ABI
 * dependency. One context and its callback state are single-consumer only.
 */
MCDOSE_PARTICLE_DMLC_API int32_t mcdose_particle_dmlc_create_source_session_v1(
    const mcdose_particle_dmlc_source_session_config_v1 *config,
    mcdose_particle_dmlc_next_incident_callback_v1 next_incident,
    void *incident_user_data,
    mcdose_particle_dmlc_source_session_context_v1 **context,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API void
mcdose_particle_dmlc_destroy_source_session_context_v1(
    mcdose_particle_dmlc_source_session_context_v1 *context);

/*
 * Drains pending MLC products before calling the incident source again. Every
 * product derived from one incident particle retains its source history ID and
 * starts_new_history flag, so correlated re-use is scored as one history.
 * Empty output means that the incident source is exhausted.
 */
MCDOSE_PARTICLE_DMLC_API int32_t mcdose_particle_dmlc_next_source_product_v1(
    mcdose_particle_dmlc_source_session_context_v1 *context,
    mcdose_particle_dmlc_source_session_result_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity);

/*
 * Drains up to result_capacity products in exactly the same order as repeated
 * scalar calls. Every result entry must be initialized with the current ABI
 * version and its struct size. product_count excludes the explicit empty
 * entry written when source exhaustion is reached. A context selects scalar or
 * batch access on first successful use and rejects later mixing.
 */
MCDOSE_PARTICLE_DMLC_API int32_t mcdose_particle_dmlc_next_source_products_v1(
    mcdose_particle_dmlc_source_session_context_v1 *context,
    mcdose_particle_dmlc_source_session_result_v1 *results,
    uint32_t result_capacity,
    uint32_t *product_count,
    char *diagnostic,
    size_t diagnostic_capacity);

/* Copies a side-effect-free snapshot of the source and producer counters. */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_get_source_session_summary_v1(
    const mcdose_particle_dmlc_source_session_context_v1 *context,
    mcdose_particle_dmlc_source_session_summary_v1 *summary,
    char *diagnostic,
    size_t diagnostic_capacity);

#ifdef __cplusplus
}
#endif

#endif
