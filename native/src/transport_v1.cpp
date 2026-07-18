#include "mcdose_particle_dmlc_transport_v1.h"

#include "context_v1.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

namespace {
constexpr double direction_tolerance = 1.0e-10;
constexpr double gap_tolerance_cm = 1.0e-5;
constexpr double tip_intersection_tolerance_cm = 5.0e-6;

int32_t fail(int32_t status, char *diagnostic, size_t capacity, const char *message) {
    if (diagnostic != nullptr && capacity > 0) {
        std::snprintf(diagnostic, capacity, "%s", message);
    }
    return status;
}

bool is_finite(double value) { return std::isfinite(value); }

bool interpolate_calibration(const mcdose_particle_dmlc_machine_context_v1 *machine,
                             double specified,
                             double *actual) {
    if (machine->calibration_mode ==
        MCDOSE_PARTICLE_DMLC_LEAF_CALIBRATION_IDENTITY) {
        *actual = specified;
        return true;
    }
    const auto &x = machine->calibration_specified_position_cm;
    const auto &y = machine->calibration_actual_projected_position_cm;
    if (specified < x.front() || specified > x.back()) {
        return false;
    }
    const auto upper = std::lower_bound(x.begin(), x.end(), specified);
    if (upper == x.begin()) {
        *actual = y.front();
        return true;
    }
    if (upper == x.end()) {
        *actual = y.back();
        return true;
    }
    const size_t stop = static_cast<size_t>(upper - x.begin());
    if (*upper == specified) {
        *actual = y[stop];
        return true;
    }
    const size_t start = stop - 1;
    const double fraction = (specified - x[start]) / (x[stop] - x[start]);
    *actual = y[start] + fraction * (y[stop] - y[start]);
    return true;
}

bool physical_leaf_positions(const mcdose_particle_dmlc_machine_context_v1 *machine,
                             double bank_1,
                             double bank_2,
                             double *following,
                             double *leading) {
    const bool touching = std::fabs(bank_2 - bank_1) <= gap_tolerance_cm;
    const bool bypass = touching && machine->touching_pairs_bypass_curve;
    double projected_following = bank_1;
    double projected_leading = bank_2;
    if (!bypass &&
        machine->calibration_mode ==
            MCDOSE_PARTICLE_DMLC_LEAF_CALIBRATION_PIECEWISE_LINEAR) {
        double calibrated_following = 0.0;
        double calibrated_reversed_leading = 0.0;
        if (!interpolate_calibration(machine, bank_1, &calibrated_following) ||
            !interpolate_calibration(machine, -bank_2,
                                     &calibrated_reversed_leading)) {
            return false;
        }
        projected_following = calibrated_following;
        projected_leading = -calibrated_reversed_leading;
    }
    *following = projected_following / machine->projection_ratio;
    *leading = projected_leading / machine->projection_ratio;
    if (*leading - *following > gap_tolerance_cm) {
        *following -= machine->physical_leaf_offset_cm;
        *leading += machine->physical_leaf_offset_cm;
    }
    if (*leading < *following) {
        if (*following - *leading > gap_tolerance_cm) {
            return false;
        }
        *leading = *following;
    }
    return true;
}

double coordinate_at_z(double position,
                       double direction,
                       double position_z,
                       double direction_z,
                       double target_z) {
    return position + direction / direction_z * (target_z - position_z);
}

size_t find_row(const mcdose_particle_dmlc_machine_context_v1 *machine,
                size_t section,
                double transverse) {
    const size_t start = static_cast<size_t>(machine->section_row_offsets[section]);
    const size_t stop = static_cast<size_t>(machine->section_row_offsets[section + 1]);
    for (size_t row = start; row < stop; ++row) {
        if (machine->section_y_start_cm[row] <= transverse &&
            transverse <= machine->section_y_end_cm[row]) {
            return row;
        }
    }
    return machine->section_y_end_cm.size();
}

double tip_thickness(
    const mcdose_particle_dmlc_machine_context_v1 *machine,
    size_t section,
    bool leading_tip,
    bool begins_inside_leaf,
    double entry,
    double exit,
    double tip_position) {
    const double entry_z = section == 0 ? machine->section_z_enter_cm[section]
                                        : machine->section_z_exit_cm[section];
    const double exit_z = section == 0 ? machine->section_z_exit_cm[section]
                                       : machine->section_z_enter_cm[section];
    const double tip_anchor_z =
        section == 0 ? machine->section_z_exit_cm[section]
                     : machine->section_z_enter_cm[section];
    const double depth_sign = section == 0 ? -1.0 : 1.0;
    const double x_sign = leading_tip ? 1.0 : -1.0;
    const double exit_relative = exit - tip_position;
    const bool vertical = exit == entry;
    const double particle_slope =
        vertical ? 0.0 : (exit_z - entry_z) / (exit - entry);
    const double particle_intercept = exit_z - particle_slope * exit_relative;
    double x_intersection = exit_relative;
    for (size_t point = 1; point < machine->rounded_tip_x_cm.size(); ++point) {
        const double tip_x_start =
            x_sign * machine->rounded_tip_x_cm[point - 1];
        const double tip_x_stop = x_sign * machine->rounded_tip_x_cm[point];
        const double tip_z_start =
            tip_anchor_z +
            depth_sign * machine->rounded_tip_depth_cm[point - 1];
        const double tip_z_stop =
            tip_anchor_z + depth_sign * machine->rounded_tip_depth_cm[point];
        const double tip_slope =
            (tip_z_stop - tip_z_start) / (tip_x_stop - tip_x_start);
        if (particle_slope == tip_slope) {
            continue;
        }
        const double tip_intercept = tip_z_stop - tip_slope * tip_x_stop;
        if (!vertical) {
            x_intersection =
                (tip_intercept - particle_intercept) /
                (particle_slope - tip_slope);
        }
        const double z_intersection =
            tip_slope * x_intersection + tip_intercept;
        const bool between = (tip_z_start < z_intersection) ==
                             (z_intersection < tip_z_stop);
        if (between ||
            std::fabs(tip_z_start - z_intersection) <
                tip_intersection_tolerance_cm ||
            std::fabs(tip_z_stop - z_intersection) <
                tip_intersection_tolerance_cm) {
            return begins_inside_leaf ? std::fabs(z_intersection - entry_z)
                                      : std::fabs(exit_z - z_intersection);
        }
    }
    return 0.0;
}

int32_t classify_section(const mcdose_particle_dmlc_machine_context_v1 *machine,
                         size_t section,
                         double entry,
                         double center,
                         double exit,
                         double following,
                         double leading,
                         double tip_distance,
                         double full_thickness,
                         double groove_thickness,
                         double groove_distance,
                         double *resolved_thickness,
                         uint32_t *resolved) {
    *resolved_thickness = 0.0;
    *resolved = 0;
    const bool empty_row = full_thickness == 0.0;
    const auto tip = [&](bool leading_tip, bool begins_inside_leaf,
                         double position) {
        return empty_row ? 0.0
                         : tip_thickness(machine, section, leading_tip,
                                         begins_inside_leaf, entry, exit,
                                         position);
    };
    if (entry > leading) {
        if (entry < leading + tip_distance) {
            *resolved_thickness = tip(true, false, leading);
            if (exit < following) {
                *resolved_thickness += tip(false, false, following);
            }
            *resolved = 1;
            return MCDOSE_PARTICLE_DMLC_SECTION_TIP_OR_CROSSING;
        }
        if (exit > leading) {
            *resolved_thickness = full_thickness;
            if (!empty_row && center > leading + groove_distance) {
                *resolved_thickness -= groove_thickness;
            }
            *resolved = 1;
            return MCDOSE_PARTICLE_DMLC_SECTION_LEADING_FULL;
        }
        *resolved_thickness = tip(true, true, leading);
        if (exit < following) {
            *resolved_thickness += tip(false, false, following);
        }
        *resolved = 1;
        return MCDOSE_PARTICLE_DMLC_SECTION_TIP_OR_CROSSING;
    }
    if (entry < following) {
        if (entry > following - tip_distance) {
            *resolved_thickness = tip(false, false, following);
            if (exit > leading) {
                *resolved_thickness += tip(true, false, leading);
            }
            *resolved = 1;
            return MCDOSE_PARTICLE_DMLC_SECTION_TIP_OR_CROSSING;
        }
        if (exit < following) {
            *resolved_thickness = full_thickness;
            if (!empty_row && center < following - groove_distance) {
                *resolved_thickness -= groove_thickness;
            }
            *resolved = 1;
            return MCDOSE_PARTICLE_DMLC_SECTION_FOLLOWING_FULL;
        }
        *resolved_thickness = tip(false, true, following);
        if (exit > leading) {
            *resolved_thickness += tip(true, false, leading);
        }
        *resolved = 1;
        return MCDOSE_PARTICLE_DMLC_SECTION_TIP_OR_CROSSING;
    }
    if (following <= exit && exit <= leading) {
        *resolved = 1;
        return MCDOSE_PARTICLE_DMLC_SECTION_OPEN;
    }
    if (exit > leading) {
        *resolved_thickness = tip(true, false, leading);
    } else if (exit < following) {
        *resolved_thickness = tip(false, false, following);
    }
    *resolved = 1;
    return MCDOSE_PARTICLE_DMLC_SECTION_TIP_OR_CROSSING;
}
}  // namespace

extern "C" int32_t mcdose_particle_dmlc_classify_ray_v1(
    const mcdose_particle_dmlc_machine_context_v1 *machine,
    const mcdose_particle_dmlc_aperture_v1 *aperture,
    const mcdose_particle_dmlc_ray_v1 *ray,
    mcdose_particle_dmlc_classification_v1 *classification,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (diagnostic != nullptr && diagnostic_capacity > 0) {
        diagnostic[0] = '\0';
    }
    if (machine == nullptr || aperture == nullptr || ray == nullptr ||
        classification == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "classification call contains a null pointer");
    }
    if (aperture->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        aperture->struct_size < sizeof(mcdose_particle_dmlc_aperture_v1) ||
        ray->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        ray->struct_size < sizeof(mcdose_particle_dmlc_ray_v1) ||
        classification->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        classification->struct_size <
            sizeof(mcdose_particle_dmlc_classification_v1)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity, "classification ABI version or size differs");
    }
    if (aperture->reserved != 0 || aperture->leaf_pair_count != machine->leaf_pair_count ||
        aperture->bank_1_positions_at_isocenter_cm == nullptr ||
        aperture->bank_2_positions_at_isocenter_cm == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "classification aperture is invalid");
    }
    for (uint32_t pair = 0; pair < aperture->leaf_pair_count; ++pair) {
        const double bank_1 = aperture->bank_1_positions_at_isocenter_cm[pair];
        const double bank_2 = aperture->bank_2_positions_at_isocenter_cm[pair];
        if (!is_finite(bank_1) || !is_finite(bank_2) ||
            bank_1 > bank_2 + gap_tolerance_cm) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "classification aperture bank pair is invalid");
        }
    }
    double direction_norm_squared = 0.0;
    for (size_t coordinate = 0; coordinate < 3; ++coordinate) {
        if (!is_finite(ray->position_cm[coordinate]) ||
            !is_finite(ray->direction[coordinate])) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "classification ray is not finite");
        }
        direction_norm_squared += ray->direction[coordinate] * ray->direction[coordinate];
    }
    if (ray->direction[2] <= 0 ||
        std::fabs(direction_norm_squared - 1.0) > direction_tolerance) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "classification ray direction is invalid");
    }

    classification->section_count = 2;
    classification->reserved = 0;
    for (size_t section = 0; section < 2; ++section) {
        classification->region[section] = MCDOSE_PARTICLE_DMLC_SECTION_OUTSIDE;
        classification->leaf_pair_index[section] =
            std::numeric_limits<uint32_t>::max();
        classification->thickness_resolved[section] = 1;
        classification->reserved_2[section] = 0;
        classification->transverse_center_cm[section] = 0.0;
        classification->motion_entry_cm[section] = 0.0;
        classification->motion_center_cm[section] = 0.0;
        classification->motion_exit_cm[section] = 0.0;
        classification->following_position_cm[section] = 0.0;
        classification->leading_position_cm[section] = 0.0;
        classification->resolved_longitudinal_thickness_cm[section] = 0.0;

        const bool motion_x = machine->motion_axis == MCDOSE_PARTICLE_DMLC_AXIS_X;
        const size_t motion_coordinate = motion_x ? 0 : 1;
        const size_t transverse_coordinate = motion_x ? 1 : 0;
        const double transverse = coordinate_at_z(
            ray->position_cm[transverse_coordinate],
            ray->direction[transverse_coordinate], ray->position_cm[2],
            ray->direction[2], machine->section_z_center_cm[section]);
        classification->transverse_center_cm[section] = transverse;
        const size_t row = find_row(machine, section, transverse);
        if (row == machine->section_y_end_cm.size()) {
            continue;
        }
        const uint32_t pair = machine->section_leaf_pair_indices[row];
        classification->leaf_pair_index[section] = pair;
        double following = 0.0;
        double leading = 0.0;
        if (!physical_leaf_positions(
                machine, aperture->bank_1_positions_at_isocenter_cm[pair],
                aperture->bank_2_positions_at_isocenter_cm[pair], &following,
                &leading)) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity,
                        "classification leaf calibration failed or crossed leaves");
        }
        classification->following_position_cm[section] = following;
        classification->leading_position_cm[section] = leading;
        const double entry = coordinate_at_z(
            ray->position_cm[motion_coordinate], ray->direction[motion_coordinate],
            ray->position_cm[2], ray->direction[2],
            machine->section_z_enter_cm[section]);
        const double center = coordinate_at_z(
            ray->position_cm[motion_coordinate], ray->direction[motion_coordinate],
            ray->position_cm[2], ray->direction[2],
            machine->section_z_center_cm[section]);
        const double exit = coordinate_at_z(
            ray->position_cm[motion_coordinate], ray->direction[motion_coordinate],
            ray->position_cm[2], ray->direction[2],
            machine->section_z_exit_cm[section]);
        classification->motion_entry_cm[section] = entry;
        classification->motion_center_cm[section] = center;
        classification->motion_exit_cm[section] = exit;
        const double classifier_entry = section == 0 ? entry : exit;
        const double classifier_exit = section == 0 ? exit : entry;
        classification->region[section] = classify_section(
            machine, section, classifier_entry, center, classifier_exit,
            following, leading,
            machine->rounded_tip_transition_distance_cm,
            machine->section_thickness_cm[row],
            machine->section_groove_thickness_cm[row],
            machine->section_groove_distance_from_tip_cm[row],
            &classification->resolved_longitudinal_thickness_cm[section],
            &classification->thickness_resolved[section]);
        if (classification->thickness_resolved[section] != 0 &&
            classification->resolved_longitudinal_thickness_cm[section] < 0) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity,
                        "classification resolved a negative section thickness");
        }
    }
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}
