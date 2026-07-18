#include "mcdose_particle_dmlc_machine_v1.h"

#include "context_v1.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>

namespace {
constexpr double section_plane_tolerance_cm = 1.0e-4;
constexpr double row_boundary_tolerance_cm = 1.0e-10;

int32_t fail(int32_t status, char *diagnostic, size_t capacity, const char *message) {
    if (diagnostic != nullptr && capacity > 0) {
        std::snprintf(diagnostic, capacity, "%s", message);
    }
    return status;
}

bool is_finite(double value) { return std::isfinite(value); }

bool count_fits_size(uint64_t count) {
    return count <= std::numeric_limits<size_t>::max();
}
}  // namespace

extern "C" int32_t mcdose_particle_dmlc_validate_machine_v1(
    const mcdose_particle_dmlc_machine_v1 *machine,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (diagnostic != nullptr && diagnostic_capacity > 0) {
        diagnostic[0] = '\0';
    }
    if (machine == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "machine pointer is null");
    }
    if (machine->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        machine->struct_size < sizeof(mcdose_particle_dmlc_machine_v1)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity, "machine ABI version or structure size differs");
    }
    if (machine->reserved != 0 || machine->leaf_pair_count == 0 ||
        machine->section_count != 2 || machine->section_row_count == 0 ||
        machine->attenuation_point_count < 2 ||
        !count_fits_size(machine->section_row_count) ||
        !count_fits_size(machine->attenuation_point_count) ||
        !count_fits_size(machine->calibration_point_count)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "machine counts or reserved fields are invalid");
    }
    const double positive_values[] = {
        machine->source_axis_distance_cm,
        machine->source_to_device_distance_cm,
        machine->position_specification_distance_cm,
        machine->material_density_g_cm3,
        machine->projection_ratio,
        machine->rounded_tip_maximum_thickness_cm,
        machine->rounded_tip_radius_cm,
        machine->rounded_tip_angle_deg,
        machine->rounded_tip_transition_distance_cm,
    };
    for (const double value : positive_values) {
        if (!is_finite(value) || value <= 0) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "machine scalar is not positive and finite");
        }
    }
    if (!is_finite(machine->physical_leaf_offset_cm) ||
        machine->physical_leaf_offset_cm < 0 ||
        (machine->motion_axis != MCDOSE_PARTICLE_DMLC_AXIS_X &&
         machine->motion_axis != MCDOSE_PARTICLE_DMLC_AXIS_Y) ||
        machine->rounded_tip_angle_deg >= 90 ||
        machine->rounded_tip_section_count < 3 ||
        machine->touching_pairs_bypass_curve > 1) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "machine calibration or rounded tip is invalid");
    }
    if (machine->section_z_enter_cm == nullptr ||
        machine->section_z_center_cm == nullptr ||
        machine->section_z_exit_cm == nullptr ||
        machine->section_row_offsets == nullptr ||
        machine->section_y_start_cm == nullptr ||
        machine->section_y_end_cm == nullptr ||
        machine->section_leaf_pair_indices == nullptr ||
        machine->section_thickness_cm == nullptr ||
        machine->section_groove_thickness_cm == nullptr ||
        machine->section_groove_distance_from_tip_cm == nullptr ||
        machine->attenuation_energy_mev == nullptr ||
        machine->total_linear_attenuation_per_cm == nullptr ||
        machine->incoherent_linear_attenuation_per_cm == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "machine contains a null required array pointer");
    }
    if (machine->section_row_offsets[0] != 0 ||
        machine->section_row_offsets[machine->section_count] !=
            machine->section_row_count) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "machine section offsets are inconsistent");
    }
    for (uint32_t section = 0; section < machine->section_count; ++section) {
        const double enter = machine->section_z_enter_cm[section];
        const double center = machine->section_z_center_cm[section];
        const double exit = machine->section_z_exit_cm[section];
        const uint64_t start = machine->section_row_offsets[section];
        const uint64_t stop = machine->section_row_offsets[section + 1];
        if (!is_finite(enter) || !is_finite(center) || !is_finite(exit) ||
            !(0 < enter && enter < center && center < exit) || stop <= start) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "machine section planes or offsets are invalid");
        }
        for (uint64_t row = start; row < stop; ++row) {
            const double y_start = machine->section_y_start_cm[row];
            const double y_end = machine->section_y_end_cm[row];
            const double thickness = machine->section_thickness_cm[row];
            const double groove_thickness = machine->section_groove_thickness_cm[row];
            const double groove_distance =
                machine->section_groove_distance_from_tip_cm[row];
            if (!is_finite(y_start) || !is_finite(y_end) || y_end <= y_start ||
                !is_finite(thickness) ||
                !is_finite(groove_thickness) || !is_finite(groove_distance) ||
                thickness < 0 || groove_thickness < 0 || groove_distance < 0 ||
                machine->section_leaf_pair_indices[row] >= machine->leaf_pair_count ||
                (row > start &&
                 std::fabs(y_start - machine->section_y_end_cm[row - 1]) >
                     row_boundary_tolerance_cm)) {
                return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                            diagnostic_capacity, "machine section row is invalid");
            }
        }
    }
    if (std::fabs(machine->section_z_exit_cm[0] - machine->section_z_enter_cm[1]) >
        section_plane_tolerance_cm) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "machine sections are not contiguous");
    }
    for (uint64_t point = 0; point < machine->attenuation_point_count; ++point) {
        const double energy = machine->attenuation_energy_mev[point];
        const double total = machine->total_linear_attenuation_per_cm[point];
        const double incoherent = machine->incoherent_linear_attenuation_per_cm[point];
        if (!is_finite(energy) || !is_finite(total) || !is_finite(incoherent) ||
            energy <= 0 || total < 0 || incoherent < 0 ||
            incoherent > total ||
            (point > 0 && energy <= machine->attenuation_energy_mev[point - 1])) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "machine attenuation row is invalid");
        }
    }
    const bool identity =
        machine->calibration_mode == MCDOSE_PARTICLE_DMLC_LEAF_CALIBRATION_IDENTITY;
    const bool piecewise = machine->calibration_mode ==
                           MCDOSE_PARTICLE_DMLC_LEAF_CALIBRATION_PIECEWISE_LINEAR;
    if ((!identity && !piecewise) ||
        (identity && (machine->calibration_point_count != 0 ||
                      machine->touching_pairs_bypass_curve != 0)) ||
        (piecewise && machine->calibration_point_count < 2)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "machine calibration mode or count is invalid");
    }
    if (piecewise &&
        (machine->calibration_specified_position_cm == nullptr ||
         machine->calibration_actual_projected_position_cm == nullptr)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "machine calibration contains a null array pointer");
    }
    for (uint64_t point = 0; point < machine->calibration_point_count; ++point) {
        const double specified = machine->calibration_specified_position_cm[point];
        const double actual = machine->calibration_actual_projected_position_cm[point];
        if (!is_finite(specified) || !is_finite(actual) ||
            (point > 0 &&
             (specified <= machine->calibration_specified_position_cm[point - 1] ||
              actual <= machine->calibration_actual_projected_position_cm[point - 1]))) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "machine calibration row is invalid");
        }
    }
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_create_machine_context_v1(
    const mcdose_particle_dmlc_machine_v1 *machine,
    mcdose_particle_dmlc_machine_context_v1 **context,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (context == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "machine context output pointer is null");
    }
    *context = nullptr;
    const int32_t validation = mcdose_particle_dmlc_validate_machine_v1(
        machine, diagnostic, diagnostic_capacity);
    if (validation != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return validation;
    }
    try {
        auto result = std::make_unique<mcdose_particle_dmlc_machine_context_v1>();
        const size_t sections = machine->section_count;
        const size_t rows = static_cast<size_t>(machine->section_row_count);
        const size_t attenuation =
            static_cast<size_t>(machine->attenuation_point_count);
        const size_t calibration =
            static_cast<size_t>(machine->calibration_point_count);
        result->leaf_pair_count = machine->leaf_pair_count;
        result->source_axis_distance_cm = machine->source_axis_distance_cm;
        result->source_to_device_distance_cm = machine->source_to_device_distance_cm;
        result->position_specification_distance_cm =
            machine->position_specification_distance_cm;
        result->material_density_g_cm3 = machine->material_density_g_cm3;
        result->calibration_mode = machine->calibration_mode;
        result->motion_axis = machine->motion_axis;
        result->touching_pairs_bypass_curve =
            machine->touching_pairs_bypass_curve != 0;
        result->projection_ratio = machine->projection_ratio;
        result->physical_leaf_offset_cm = machine->physical_leaf_offset_cm;
        result->rounded_tip_maximum_thickness_cm =
            machine->rounded_tip_maximum_thickness_cm;
        result->rounded_tip_radius_cm = machine->rounded_tip_radius_cm;
        result->rounded_tip_angle_deg = machine->rounded_tip_angle_deg;
        result->rounded_tip_transition_distance_cm =
            machine->rounded_tip_transition_distance_cm;
        result->rounded_tip_section_count = machine->rounded_tip_section_count;
        result->rounded_tip_x_cm.resize(machine->rounded_tip_section_count);
        result->rounded_tip_depth_cm.resize(machine->rounded_tip_section_count);
        const double tip_angle_rad =
            machine->rounded_tip_angle_deg * std::acos(-1.0) / 180.0;
        const double intersection_x =
            machine->rounded_tip_radius_cm * (1.0 - std::cos(tip_angle_rad));
        const double intersection_depth =
            machine->rounded_tip_radius_cm * std::sin(tip_angle_rad);
        const double half_maximum_thickness =
            machine->rounded_tip_maximum_thickness_cm * 0.5;
        result->rounded_tip_x_cm[0] =
            intersection_x -
            (intersection_depth - half_maximum_thickness) *
                std::tan(tip_angle_rad);
        result->rounded_tip_depth_cm[0] = half_maximum_thickness;
        result->rounded_tip_x_cm[1] = intersection_x;
        result->rounded_tip_depth_cm[1] = intersection_depth;
        const double depth_step =
            intersection_depth /
            static_cast<double>(machine->rounded_tip_section_count - 2);
        double depth = intersection_depth;
        const double radius_squared = machine->rounded_tip_radius_cm *
                                      machine->rounded_tip_radius_cm;
        for (uint32_t tip = 2; tip < machine->rounded_tip_section_count; ++tip) {
            depth -= depth_step;
            result->rounded_tip_x_cm[tip] =
                machine->rounded_tip_radius_cm -
                std::sqrt(std::max(0.0, radius_squared - depth * depth));
            result->rounded_tip_depth_cm[tip] = depth;
        }
        result->section_z_enter_cm.assign(machine->section_z_enter_cm,
                                          machine->section_z_enter_cm + sections);
        result->section_z_center_cm.assign(machine->section_z_center_cm,
                                           machine->section_z_center_cm + sections);
        result->section_z_exit_cm.assign(machine->section_z_exit_cm,
                                         machine->section_z_exit_cm + sections);
        result->section_row_offsets.assign(machine->section_row_offsets,
                                           machine->section_row_offsets + sections + 1);
        result->section_y_start_cm.assign(machine->section_y_start_cm,
                                          machine->section_y_start_cm + rows);
        result->section_y_end_cm.assign(machine->section_y_end_cm,
                                        machine->section_y_end_cm + rows);
        result->section_leaf_pair_indices.assign(machine->section_leaf_pair_indices,
                                                 machine->section_leaf_pair_indices + rows);
        result->section_thickness_cm.assign(machine->section_thickness_cm,
                                            machine->section_thickness_cm + rows);
        result->section_groove_thickness_cm.assign(
            machine->section_groove_thickness_cm,
            machine->section_groove_thickness_cm + rows);
        result->section_groove_distance_from_tip_cm.assign(
            machine->section_groove_distance_from_tip_cm,
            machine->section_groove_distance_from_tip_cm + rows);
        result->attenuation_energy_mev.assign(
            machine->attenuation_energy_mev,
            machine->attenuation_energy_mev + attenuation);
        result->total_linear_attenuation_per_cm.assign(
            machine->total_linear_attenuation_per_cm,
            machine->total_linear_attenuation_per_cm + attenuation);
        result->incoherent_linear_attenuation_per_cm.assign(
            machine->incoherent_linear_attenuation_per_cm,
            machine->incoherent_linear_attenuation_per_cm + attenuation);
        if (calibration > 0) {
            result->calibration_specified_position_cm.assign(
                machine->calibration_specified_position_cm,
                machine->calibration_specified_position_cm + calibration);
            result->calibration_actual_projected_position_cm.assign(
                machine->calibration_actual_projected_position_cm,
                machine->calibration_actual_projected_position_cm + calibration);
        }
        *context = result.release();
    } catch (const std::bad_alloc &) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity, "cannot allocate native machine context");
    } catch (const std::length_error &) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity, "native machine context array is too large");
    }
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" void mcdose_particle_dmlc_destroy_machine_context_v1(
    mcdose_particle_dmlc_machine_context_v1 *context) {
    delete context;
}
