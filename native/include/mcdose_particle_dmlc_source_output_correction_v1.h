#ifndef MCDOSE_PARTICLE_DMLC_SOURCE_OUTPUT_CORRECTION_V1_H
#define MCDOSE_PARTICLE_DMLC_SOURCE_OUTPUT_CORRECTION_V1_H

#include "mcdose_particle_dmlc_producer_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

enum mcdose_particle_dmlc_source_output_correction_model_v1 {
    MCDOSE_PARTICLE_DMLC_SOURCE_OUTPUT_CORRECTION_LEGACY_MCV_FIELD_SIZE = 1
};

enum mcdose_particle_dmlc_source_output_correction_x2_mapping_v1 {
    MCDOSE_PARTICLE_DMLC_SOURCE_OUTPUT_CORRECTION_DICOM_X2 = 1,
    MCDOSE_PARTICLE_DMLC_SOURCE_OUTPUT_CORRECTION_LEGACY_MCV_Y2 = 2
};

typedef struct mcdose_particle_dmlc_source_output_correction_config_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t model;
    uint32_t x2_mapping;
    uint64_t x_jaw_opening_offset;
    uint64_t y_jaw_opening_offset;
    uint64_t total_pair_count;
    double universal_baseline;
    double universal_amplitude;
    double universal_exponential_rate_per_cm;
    double r_y_intercept_percent;
    double r_y_linear_percent_per_cm;
    double r_y_cubic_percent_per_cm3;
    double r_x_intercept_percent;
    double r_x_linear_percent_per_cm;
    double p_y_linear_per_cm;
    double p_y_cubic_per_cm3;
    double reference_jaws_cm_y1_y2_x1_x2[4];
    uint32_t reserved[4];
} mcdose_particle_dmlc_source_output_correction_config_v1;

typedef struct mcdose_particle_dmlc_source_output_correction_result_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    double universal_field_factor;
    double ion_chamber_backscatter_factor;
    double product;
} mcdose_particle_dmlc_source_output_correction_result_v1;

typedef struct mcdose_particle_dmlc_source_output_correction_info_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    char startup_payload_sha256[65];
    char correction_payload_sha256[65];
    uint8_t reserved[6];
} mcdose_particle_dmlc_source_output_correction_info_v1;

MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_load_source_output_correction_v1(
    const char *path,
    mcdose_particle_dmlc_source_output_correction_config_v1 *config,
    mcdose_particle_dmlc_source_output_correction_info_v1 *info,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_evaluate_source_output_correction_v1(
    const mcdose_particle_dmlc_source_output_correction_config_v1 *config,
    const double *bank_1_positions_cm,
    const double *bank_2_positions_cm,
    uint64_t position_count,
    mcdose_particle_dmlc_source_output_correction_result_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity);

/* Callback-compatible adapter for the producer's sampled-state boundary. */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_apply_source_output_correction_v1(
    void *user_data,
    const mcdose_particle_dmlc_sampled_state_v1 *sampled_state,
    const double *bank_1_positions_cm,
    const double *bank_2_positions_cm,
    uint64_t position_count,
    double *weight_factor,
    char *diagnostic,
    size_t diagnostic_capacity);

#ifdef __cplusplus
}
#endif

#endif
