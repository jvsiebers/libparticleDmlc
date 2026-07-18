#ifndef MCDOSE_PARTICLE_DMLC_PRODUCER_V1_H
#define MCDOSE_PARTICLE_DMLC_PRODUCER_V1_H

#include "mcdose_particle_dmlc_host_adapter_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mcdose_particle_dmlc_producer_context_v1
    mcdose_particle_dmlc_producer_context_v1;

enum mcdose_particle_dmlc_producer_product_kind_v1 {
    MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY = 1,
    MCDOSE_PARTICLE_DMLC_PRODUCER_SCATTERED_PHOTON = 2,
    MCDOSE_PARTICLE_DMLC_PRODUCER_GENERATED_COMPTON_ELECTRON = 3
};

enum mcdose_particle_dmlc_generated_electron_policy_v1 {
    MCDOSE_PARTICLE_DMLC_GENERATED_ELECTRON_DISCARD = 1
};

typedef struct mcdose_particle_dmlc_producer_config_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t mlc_device_index;
    uint32_t generated_electron_policy;
    uint32_t reserved[4];
} mcdose_particle_dmlc_producer_config_v1;

typedef struct mcdose_particle_dmlc_producer_summary_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t retained_product_count;
    uint32_t random_draw_count;
    uint32_t source_segment_index;
    uint32_t primary_retained;
    uint32_t scattered_photon_retained;
    uint32_t generated_compton_electron_discarded;
    double fractional_meterset;
    double interpolation_fraction;
} mcdose_particle_dmlc_producer_summary_v1;

typedef struct mcdose_particle_dmlc_producer_product_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t has_product;
    uint32_t remaining_product_count;
    int32_t product_kind;
    uint32_t reserved;
    mcdose_particle_dmlc_particle_v1 particle;
} mcdose_particle_dmlc_producer_product_v1;

/*
 * Creates one source-owned producer. The delivery and commissioned machine are
 * deep-copied. mlc_device_index is zero-based and must select a DICOM MLC pair
 * device compatible with the machine motion axis and leaf-pair count.
 *
 * The production path uses libParticleDmlc's commissioned equivalent-thickness
 * model. It does not require caller-supplied physical intervals or individual
 * physical leaf-plane locations.
 *
 * Static jaws are assumed to have been transported by the upstream BEAM
 * model. If either DICOM jaw pair moves between control points, the producer
 * requires one single-pair X jaw and one single-pair Y jaw and applies their
 * interpolated openings as an ideal aperture at the incident particle's
 * positive-Z source plane. This intentionally reproduces the reviewed legacy
 * jaw-tracking approximation without double-filtering static jaws.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_create_producer_context_v1(
    const mcdose_particle_dmlc_delivery_v1 *delivery,
    const mcdose_particle_dmlc_machine_v1 *machine,
    const mcdose_particle_dmlc_producer_config_v1 *config,
    mcdose_particle_dmlc_producer_context_v1 **context,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API void
mcdose_particle_dmlc_destroy_producer_context_v1(
    mcdose_particle_dmlc_producer_context_v1 *context);

/*
 * Configures the host-owned random stream used by forced interaction and
 * Compton sampling. No random value is requested for open, outside, or charged
 * paths that do not generate a Compton branch.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_set_producer_random_source_v1(
    mcdose_particle_dmlc_producer_context_v1 *context,
    mcdose_particle_dmlc_host_random_callback_v1 callback,
    void *user_data,
    char *diagnostic,
    size_t diagnostic_capacity);

/*
 * Samples the delivery exactly once at fractional_meterset and produces the
 * retained products in legacy order: primary, scattered photon, generated
 * Compton electron. Version 1 explicitly discards the generated electron, so
 * at most the first two tagged products are queued. Photon calls require two
 * explicit, unique candidate product identifiers even when no interaction is
 * ultimately sampled. A new call is rejected until the prior queue is drained.
 *
 * On failure the queue remains empty. summary.random_draw_count still reports
 * values consumed before a random-source or sampling failure; other summary
 * result fields remain zero. A particle blocked by tracked jaws is a successful
 * call with no retained products, no random draws, and populated sampled-state
 * summary fields.
 */
MCDOSE_PARTICLE_DMLC_API int32_t mcdose_particle_dmlc_produce_v1(
    mcdose_particle_dmlc_producer_context_v1 *context,
    const mcdose_particle_dmlc_particle_v1 *incident_particle,
    double fractional_meterset,
    uint64_t scattered_photon_particle_id,
    uint64_t electron_particle_id,
    mcdose_particle_dmlc_producer_summary_v1 *summary,
    char *diagnostic,
    size_t diagnostic_capacity);

/* Returns the next tagged product, or explicit empty state after the drain. */
MCDOSE_PARTICLE_DMLC_API int32_t mcdose_particle_dmlc_next_producer_product_v1(
    mcdose_particle_dmlc_producer_context_v1 *context,
    mcdose_particle_dmlc_producer_product_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity);

#ifdef __cplusplus
}
#endif

#endif
