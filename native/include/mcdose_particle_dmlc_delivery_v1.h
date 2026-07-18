#ifndef MCDOSE_PARTICLE_DMLC_DELIVERY_V1_H
#define MCDOSE_PARTICLE_DMLC_DELIVERY_V1_H

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32) && defined(MCDOSE_PARTICLE_DMLC_BUILD)
#define MCDOSE_PARTICLE_DMLC_API __declspec(dllexport)
#elif defined(_WIN32)
#define MCDOSE_PARTICLE_DMLC_API __declspec(dllimport)
#else
#define MCDOSE_PARTICLE_DMLC_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION 1u

enum mcdose_particle_dmlc_device_kind_v1 {
    MCDOSE_PARTICLE_DMLC_DEVICE_JAW = 1,
    MCDOSE_PARTICLE_DMLC_DEVICE_MLC = 2
};

enum mcdose_particle_dmlc_device_axis_v1 {
    MCDOSE_PARTICLE_DMLC_AXIS_X = 1,
    MCDOSE_PARTICLE_DMLC_AXIS_Y = 2
};

enum mcdose_particle_dmlc_rotation_direction_v1 {
    MCDOSE_PARTICLE_DMLC_ROTATION_NONE = 0,
    MCDOSE_PARTICLE_DMLC_ROTATION_CW = 1,
    MCDOSE_PARTICLE_DMLC_ROTATION_CCW = 2
};

enum mcdose_particle_dmlc_status_v1 {
    MCDOSE_PARTICLE_DMLC_STATUS_OK = 0,
    MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT = 1,
    MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH = 2,
    MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED = 3,
    MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY = 4,
    MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED = 5
};

/*
 * All arrays are caller-owned and immutable for the duration of a context-
 * construction call. A successful native context must copy any data it keeps.
 * Lengths are centimetres, angles are degrees, energy is MeV, and isocentre
 * coordinates use DICOM patient LPS. Device offsets contain device_count + 1
 * entries. Bank arrays are control-point-major and then device-major.
 */
typedef struct mcdose_particle_dmlc_delivery_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    int32_t beam_number;
    uint32_t device_count;
    uint32_t control_point_count;
    uint32_t reserved;
    uint64_t total_pair_count;
    uint64_t boundary_count;
    double source_axis_distance_cm;
    const int32_t *device_kinds;
    const int32_t *device_axes;
    const uint32_t *device_pair_counts;
    const double *device_source_distances_cm;
    const uint8_t *device_source_distance_present;
    const uint64_t *device_opening_offsets;
    const uint64_t *device_boundary_offsets;
    const double *cumulative_meterset_fractions;
    const double *gantry_angles_deg;
    const int32_t *gantry_directions;
    const double *collimator_angles_deg;
    const int32_t *collimator_directions;
    const double *patient_support_angles_deg;
    const int32_t *patient_support_directions;
    const double *isocenter_patient_lps_cm;
    const double *nominal_beam_energies_mev;
    const double *dose_rates_mu_per_minute;
    const uint8_t *dose_rate_present;
    const double *tabletop_pitch_angles_deg;
    const double *tabletop_roll_angles_deg;
    const double *bank_1_positions_cm;
    const double *bank_2_positions_cm;
    const double *mlc_boundaries_at_isocenter_cm;
} mcdose_particle_dmlc_delivery_v1;

typedef struct mcdose_particle_dmlc_sampled_state_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t source_segment_index;
    uint32_t reserved;
    uint64_t total_pair_count;
    double fractional_meterset;
    double interpolation_fraction;
    double gantry_angle_deg;
    double collimator_angle_deg;
    double patient_support_angle_deg;
    double isocenter_patient_lps_cm[3];
    double nominal_beam_energy_mev;
    double dose_rate_mu_per_minute;
    uint32_t dose_rate_present;
    uint32_t reserved_2;
    double tabletop_pitch_angle_deg;
    double tabletop_roll_angle_deg;
} mcdose_particle_dmlc_sampled_state_v1;

typedef struct mcdose_particle_dmlc_delivery_context_v1
    mcdose_particle_dmlc_delivery_context_v1;

MCDOSE_PARTICLE_DMLC_API int32_t mcdose_particle_dmlc_validate_delivery_v1(
    const mcdose_particle_dmlc_delivery_v1 *delivery,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API int32_t mcdose_particle_dmlc_create_delivery_context_v1(
    const mcdose_particle_dmlc_delivery_v1 *delivery,
    mcdose_particle_dmlc_delivery_context_v1 **context,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API void mcdose_particle_dmlc_destroy_delivery_context_v1(
    mcdose_particle_dmlc_delivery_context_v1 *context);

MCDOSE_PARTICLE_DMLC_API int32_t mcdose_particle_dmlc_sample_delivery_v1(
    const mcdose_particle_dmlc_delivery_context_v1 *context,
    double fractional_meterset,
    mcdose_particle_dmlc_sampled_state_v1 *state,
    double *bank_1_positions_cm,
    double *bank_2_positions_cm,
    uint64_t position_capacity,
    char *diagnostic,
    size_t diagnostic_capacity);

#ifdef __cplusplus
}
#endif

#endif
