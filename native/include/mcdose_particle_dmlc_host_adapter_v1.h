#ifndef MCDOSE_PARTICLE_DMLC_HOST_ADAPTER_V1_H
#define MCDOSE_PARTICLE_DMLC_HOST_ADAPTER_V1_H

#include "mcdose_particle_dmlc_interaction_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mcdose_particle_dmlc_host_adapter_context_v1
    mcdose_particle_dmlc_host_adapter_context_v1;

/*
 * Returns one finite uniform value in [0,1) through uniform_random. The host
 * owns user_data and must keep it alive until the callback is replaced or the
 * adapter context is destroyed. Return a version-1 status code.
 */
typedef int32_t (*mcdose_particle_dmlc_host_random_callback_v1)(
    void *user_data,
    double *uniform_random);

typedef struct mcdose_particle_dmlc_host_product_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t has_product;
    uint32_t remaining_product_count;
    mcdose_particle_dmlc_particle_v1 particle;
} mcdose_particle_dmlc_host_product_v1;

/*
 * Creates one host-owned bounded queue. The context has no process-global state
 * and is intended to belong to one EGSnrc source instance.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_create_host_adapter_context_v1(
    uint32_t product_capacity,
    mcdose_particle_dmlc_host_adapter_context_v1 **context,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API void
mcdose_particle_dmlc_destroy_host_adapter_context_v1(
    mcdose_particle_dmlc_host_adapter_context_v1 *context);

MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_set_host_random_source_v1(
    mcdose_particle_dmlc_host_adapter_context_v1 *context,
    mcdose_particle_dmlc_host_random_callback_v1 callback,
    void *user_data,
    char *diagnostic,
    size_t diagnostic_capacity);

/*
 * Samples Compton products using exactly as many callback values as the core
 * consumes, including rejection loops. No callback value is requested until
 * all non-random inputs pass core validation.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_sample_host_compton_products_v1(
    mcdose_particle_dmlc_host_adapter_context_v1 *context,
    const mcdose_particle_dmlc_particle_v1 *incident_photon,
    const mcdose_particle_dmlc_photon_attenuation_v1 *attenuation,
    const mcdose_particle_dmlc_forced_interaction_v1 *interaction,
    uint64_t scattered_photon_particle_id,
    uint64_t electron_particle_id,
    mcdose_particle_dmlc_compton_products_v1 *products,
    char *diagnostic,
    size_t diagnostic_capacity);

/*
 * Deep-copies a complete product set. Loading is rejected while an earlier set
 * remains undrained and when count exceeds the context capacity.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_load_host_products_v1(
    mcdose_particle_dmlc_host_adapter_context_v1 *context,
    const mcdose_particle_dmlc_particle_v1 *products,
    uint32_t product_count,
    char *diagnostic,
    size_t diagnostic_capacity);

/*
 * Returns the next product and explicit remaining count. Empty queues succeed
 * with has_product=0 and do not require a sentinel particle weight.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_next_host_product_v1(
    mcdose_particle_dmlc_host_adapter_context_v1 *context,
    mcdose_particle_dmlc_host_product_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity);

#ifdef __cplusplus
}
#endif

#endif
