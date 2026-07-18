#ifndef MCDOSE_PARTICLE_DMLC_STARTUP_V1_H
#define MCDOSE_PARTICLE_DMLC_STARTUP_V1_H

#include "mcdose_particle_dmlc_producer_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MCDOSE_PARTICLE_DMLC_STARTUP_APPLICATION_ID 0x4D434450u
#define MCDOSE_PARTICLE_DMLC_STARTUP_USER_VERSION 1u

typedef struct mcdose_particle_dmlc_startup_info_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    int32_t beam_number;
    uint32_t mlc_device_index;
    uint32_t device_count;
    uint32_t control_point_count;
    uint32_t leaf_pair_count;
    uint32_t generated_electron_policy;
    char delivery_sha256[65];
    char machine_sha256[65];
    char payload_sha256[65];
    uint8_t reserved[5];
} mcdose_particle_dmlc_startup_info_v1;

/*
 * Strictly loads a Python-created SQLite startup artifact, verifies its
 * complete canonical payload hash, and constructs the existing deep-copying
 * producer context. SQLite handles and parsing buffers are released before
 * this function returns. Version 1 requires a little-endian host.
 *
 * If info is non-null, its ABI version and size must be initialized by the
 * caller. It is populated only on success. The returned producer context is
 * destroyed with mcdose_particle_dmlc_destroy_producer_context_v1.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_create_producer_from_startup_v1(
    const char *path,
    mcdose_particle_dmlc_producer_context_v1 **context,
    mcdose_particle_dmlc_startup_info_v1 *info,
    char *diagnostic,
    size_t diagnostic_capacity);

#ifdef __cplusplus
}
#endif

#endif
