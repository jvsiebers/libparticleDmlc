#ifndef MCDOSE_PARTICLE_DMLC_CONTEXT_V1_HPP
#define MCDOSE_PARTICLE_DMLC_CONTEXT_V1_HPP

#include "mcdose_particle_dmlc_machine_v1.h"

#include <vector>

struct mcdose_particle_dmlc_machine_context_v1 {
    uint32_t leaf_pair_count;
    double source_axis_distance_cm;
    double source_to_device_distance_cm;
    double position_specification_distance_cm;
    double material_density_g_cm3;
    int32_t calibration_mode;
    int32_t motion_axis;
    bool touching_pairs_bypass_curve;
    double projection_ratio;
    double physical_leaf_offset_cm;
    double rounded_tip_maximum_thickness_cm;
    double rounded_tip_radius_cm;
    double rounded_tip_angle_deg;
    double rounded_tip_transition_distance_cm;
    uint32_t rounded_tip_section_count;
    std::vector<double> rounded_tip_x_cm;
    std::vector<double> rounded_tip_depth_cm;
    std::vector<double> section_z_enter_cm;
    std::vector<double> section_z_center_cm;
    std::vector<double> section_z_exit_cm;
    std::vector<uint64_t> section_row_offsets;
    std::vector<double> section_y_start_cm;
    std::vector<double> section_y_end_cm;
    std::vector<uint32_t> section_leaf_pair_indices;
    std::vector<double> section_thickness_cm;
    std::vector<double> section_groove_thickness_cm;
    std::vector<double> section_groove_distance_from_tip_cm;
    std::vector<double> attenuation_energy_mev;
    std::vector<double> total_linear_attenuation_per_cm;
    std::vector<double> incoherent_linear_attenuation_per_cm;
    std::vector<double> calibration_specified_position_cm;
    std::vector<double> calibration_actual_projected_position_cm;
};

#endif
