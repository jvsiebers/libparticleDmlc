#ifndef MCDOSE_PARTICLE_DMLC_MACHINE_V1_H
#define MCDOSE_PARTICLE_DMLC_MACHINE_V1_H

#include "mcdose_particle_dmlc_delivery_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

enum mcdose_particle_dmlc_leaf_calibration_mode_v1 {
    MCDOSE_PARTICLE_DMLC_LEAF_CALIBRATION_IDENTITY = 0,
    MCDOSE_PARTICLE_DMLC_LEAF_CALIBRATION_PIECEWISE_LINEAR = 1
};

/*
 * All arrays are caller-owned and immutable during context construction.
 * A successful native context deep-copies every retained value. Lengths are
 * centimetres, density is g/cm3, energy is MeV, and linear attenuation is
 * cm^-1. Section rows are section-major and indexed by section_row_offsets.
 */
typedef struct mcdose_particle_dmlc_machine_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t leaf_pair_count;
    uint32_t section_count;
    uint64_t section_row_count;
    uint64_t attenuation_point_count;
    uint64_t calibration_point_count;
    double source_axis_distance_cm;
    double source_to_device_distance_cm;
    double position_specification_distance_cm;
    double material_density_g_cm3;
    int32_t calibration_mode;
    int32_t motion_axis;
    uint32_t touching_pairs_bypass_curve;
    double projection_ratio;
    double physical_leaf_offset_cm;
    double rounded_tip_maximum_thickness_cm;
    double rounded_tip_radius_cm;
    double rounded_tip_angle_deg;
    double rounded_tip_transition_distance_cm;
    uint32_t rounded_tip_section_count;
    uint32_t reserved;
    const double *section_z_enter_cm;
    const double *section_z_center_cm;
    const double *section_z_exit_cm;
    const uint64_t *section_row_offsets;
    const double *section_y_start_cm;
    const double *section_y_end_cm;
    const uint32_t *section_leaf_pair_indices;
    const double *section_thickness_cm;
    const double *section_groove_thickness_cm;
    const double *section_groove_distance_from_tip_cm;
    const double *attenuation_energy_mev;
    const double *total_linear_attenuation_per_cm;
    const double *incoherent_linear_attenuation_per_cm;
    const double *calibration_specified_position_cm;
    const double *calibration_actual_projected_position_cm;
} mcdose_particle_dmlc_machine_v1;

typedef struct mcdose_particle_dmlc_machine_context_v1
    mcdose_particle_dmlc_machine_context_v1;

MCDOSE_PARTICLE_DMLC_API int32_t mcdose_particle_dmlc_validate_machine_v1(
    const mcdose_particle_dmlc_machine_v1 *machine,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API int32_t mcdose_particle_dmlc_create_machine_context_v1(
    const mcdose_particle_dmlc_machine_v1 *machine,
    mcdose_particle_dmlc_machine_context_v1 **context,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API void mcdose_particle_dmlc_destroy_machine_context_v1(
    mcdose_particle_dmlc_machine_context_v1 *context);

#ifdef __cplusplus
}
#endif

#endif
