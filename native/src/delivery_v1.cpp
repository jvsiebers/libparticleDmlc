#include "mcdose_particle_dmlc_delivery_v1.h"

#include <cmath>
#include <cstdio>
#include <limits>
#include <memory>
#include <new>
#include <vector>

namespace {
constexpr double tolerance = 1.0e-10;

int32_t fail(int32_t status, char *diagnostic, size_t capacity, const char *message) {
    if (diagnostic != nullptr && capacity > 0) {
        std::snprintf(diagnostic, capacity, "%s", message);
    }
    return status;
}

bool is_finite(double value) { return std::isfinite(value); }

bool valid_direction(int32_t value) {
    return value == MCDOSE_PARTICLE_DMLC_ROTATION_NONE ||
           value == MCDOSE_PARTICLE_DMLC_ROTATION_CW ||
           value == MCDOSE_PARTICLE_DMLC_ROTATION_CCW;
}

bool same_angle(double first, double second) {
    double difference = std::fmod(std::fabs(second - first), 360.0);
    difference = std::fmin(difference, 360.0 - difference);
    return difference <= tolerance;
}

bool multiplication_fits_size(uint64_t first, uint64_t second) {
    if (first == 0 || second == 0) {
        return true;
    }
    return first <= std::numeric_limits<size_t>::max() / second;
}

double interpolate(double start, double stop, double fraction) {
    return start + fraction * (stop - start);
}

double interpolate_angle(double start, double stop, int32_t direction, double fraction) {
    if (direction == MCDOSE_PARTICLE_DMLC_ROTATION_NONE) {
        return start == 360.0 ? 0.0 : start;
    }
    const double raw_delta = stop - start;
    double delta = 0.0;
    if (direction == MCDOSE_PARTICLE_DMLC_ROTATION_CW) {
        delta = std::fmod(raw_delta, 360.0);
        if (delta < 0) {
            delta += 360.0;
        }
        if (std::fabs(delta) <= tolerance) {
            delta = 360.0;
        }
    } else {
        delta = -std::fmod(-raw_delta, 360.0);
        if (delta > 0) {
            delta -= 360.0;
        }
        if (std::fabs(delta) <= tolerance) {
            delta = -360.0;
        }
    }
    double result = std::fmod(start + fraction * delta, 360.0);
    if (result < 0) {
        result += 360.0;
    }
    return std::fabs(result - 360.0) <= tolerance ? 0.0 : result;
}
}  // namespace

struct mcdose_particle_dmlc_delivery_context_v1 {
    uint32_t control_point_count;
    uint64_t total_pair_count;
    std::vector<double> cumulative_meterset_fractions;
    std::vector<double> gantry_angles_deg;
    std::vector<int32_t> gantry_directions;
    std::vector<double> collimator_angles_deg;
    std::vector<int32_t> collimator_directions;
    std::vector<double> patient_support_angles_deg;
    std::vector<int32_t> patient_support_directions;
    std::vector<double> isocenter_patient_lps_cm;
    std::vector<double> nominal_beam_energies_mev;
    std::vector<double> dose_rates_mu_per_minute;
    std::vector<uint8_t> dose_rate_present;
    std::vector<double> tabletop_pitch_angles_deg;
    std::vector<double> tabletop_roll_angles_deg;
    std::vector<double> bank_1_positions_cm;
    std::vector<double> bank_2_positions_cm;
};

extern "C" int32_t mcdose_particle_dmlc_validate_delivery_v1(
    const mcdose_particle_dmlc_delivery_v1 *delivery,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (diagnostic != nullptr && diagnostic_capacity > 0) {
        diagnostic[0] = '\0';
    }
    if (delivery == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "delivery pointer is null");
    }
    if (delivery->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        delivery->struct_size < sizeof(mcdose_particle_dmlc_delivery_v1)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity, "delivery ABI version or structure size differs");
    }
    if (delivery->reserved != 0 || delivery->beam_number < 1 ||
        delivery->device_count == 0 || delivery->control_point_count < 2 ||
        delivery->total_pair_count == 0 || !is_finite(delivery->source_axis_distance_cm) ||
        delivery->source_axis_distance_cm <= 0) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "delivery scalar fields are invalid");
    }
    if (!multiplication_fits_size(delivery->control_point_count, 3) ||
        !multiplication_fits_size(delivery->control_point_count,
                                  delivery->total_pair_count) ||
        delivery->boundary_count > std::numeric_limits<size_t>::max()) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "delivery array dimensions overflow size_t");
    }
    if (delivery->device_kinds == nullptr || delivery->device_axes == nullptr ||
        delivery->device_pair_counts == nullptr ||
        delivery->device_source_distances_cm == nullptr ||
        delivery->device_source_distance_present == nullptr ||
        delivery->device_opening_offsets == nullptr ||
        delivery->device_boundary_offsets == nullptr ||
        delivery->cumulative_meterset_fractions == nullptr ||
        delivery->gantry_angles_deg == nullptr || delivery->gantry_directions == nullptr ||
        delivery->collimator_angles_deg == nullptr ||
        delivery->collimator_directions == nullptr ||
        delivery->patient_support_angles_deg == nullptr ||
        delivery->patient_support_directions == nullptr ||
        delivery->isocenter_patient_lps_cm == nullptr ||
        delivery->nominal_beam_energies_mev == nullptr ||
        delivery->dose_rates_mu_per_minute == nullptr ||
        delivery->dose_rate_present == nullptr ||
        delivery->tabletop_pitch_angles_deg == nullptr ||
        delivery->tabletop_roll_angles_deg == nullptr ||
        delivery->bank_1_positions_cm == nullptr ||
        delivery->bank_2_positions_cm == nullptr ||
        delivery->mlc_boundaries_at_isocenter_cm == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "delivery contains a null array pointer");
    }

    if (delivery->device_opening_offsets[0] != 0 ||
        delivery->device_boundary_offsets[0] != 0) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "device offsets must start at zero");
    }
    uint32_t mlc_count = 0;
    for (uint32_t device = 0; device < delivery->device_count; ++device) {
        const int32_t kind = delivery->device_kinds[device];
        const int32_t axis = delivery->device_axes[device];
        const uint32_t pair_count = delivery->device_pair_counts[device];
        const uint64_t opening_start = delivery->device_opening_offsets[device];
        const uint64_t opening_stop = delivery->device_opening_offsets[device + 1];
        const uint64_t boundary_start = delivery->device_boundary_offsets[device];
        const uint64_t boundary_stop = delivery->device_boundary_offsets[device + 1];
        if ((kind != MCDOSE_PARTICLE_DMLC_DEVICE_JAW &&
             kind != MCDOSE_PARTICLE_DMLC_DEVICE_MLC) ||
            (axis != MCDOSE_PARTICLE_DMLC_AXIS_X &&
             axis != MCDOSE_PARTICLE_DMLC_AXIS_Y) ||
            pair_count == 0 || opening_stop < opening_start ||
            opening_stop - opening_start != pair_count || boundary_stop < boundary_start) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "device code, count, or offset is invalid");
        }
        if (kind == MCDOSE_PARTICLE_DMLC_DEVICE_MLC) {
            ++mlc_count;
            if (boundary_stop - boundary_start != static_cast<uint64_t>(pair_count) + 1) {
                return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                            diagnostic_capacity, "MLC boundary count is invalid");
            }
            for (uint64_t index = boundary_start; index < boundary_stop; ++index) {
                const double boundary = delivery->mlc_boundaries_at_isocenter_cm[index];
                if (!is_finite(boundary) ||
                    (index > boundary_start &&
                     boundary <= delivery->mlc_boundaries_at_isocenter_cm[index - 1])) {
                    return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                                diagnostic_capacity, "MLC boundaries are not finite and increasing");
                }
            }
        } else if (boundary_stop != boundary_start) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "jaw device unexpectedly declares boundaries");
        }
        const uint8_t present = delivery->device_source_distance_present[device];
        const double distance = delivery->device_source_distances_cm[device];
        if (present > 1 || !is_finite(distance) ||
            (present == 1 && distance <= 0) || (present == 0 && distance != 0)) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "device source distance is invalid");
        }
    }
    if (mlc_count != 1 ||
        delivery->device_opening_offsets[delivery->device_count] !=
            delivery->total_pair_count ||
        delivery->device_boundary_offsets[delivery->device_count] !=
            delivery->boundary_count) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "delivery requires one MLC and consistent final offsets");
    }

    bool has_positive_segment = false;
    for (uint32_t point = 0; point < delivery->control_point_count; ++point) {
        const double fraction = delivery->cumulative_meterset_fractions[point];
        const double gantry = delivery->gantry_angles_deg[point];
        const double collimator = delivery->collimator_angles_deg[point];
        const double support = delivery->patient_support_angles_deg[point];
        const double energy = delivery->nominal_beam_energies_mev[point];
        const double dose_rate = delivery->dose_rates_mu_per_minute[point];
        const uint8_t dose_present = delivery->dose_rate_present[point];
        if (!is_finite(fraction) || fraction < 0 || fraction > 1 ||
            (point > 0 &&
             fraction < delivery->cumulative_meterset_fractions[point - 1]) ||
            !is_finite(gantry) || gantry < 0 || gantry > 360 ||
            !is_finite(collimator) || collimator < 0 || collimator > 360 ||
            !is_finite(support) || support < 0 || support > 360 ||
            !valid_direction(delivery->gantry_directions[point]) ||
            !valid_direction(delivery->collimator_directions[point]) ||
            !valid_direction(delivery->patient_support_directions[point]) ||
            !is_finite(energy) || energy <= 0 || dose_present > 1 ||
            !is_finite(dose_rate) ||
            (dose_present == 1 && dose_rate < 0) ||
            (dose_present == 0 && dose_rate != 0) ||
            !is_finite(delivery->tabletop_pitch_angles_deg[point]) ||
            !is_finite(delivery->tabletop_roll_angles_deg[point]) ||
            std::fabs(delivery->tabletop_pitch_angles_deg[point]) > tolerance ||
            std::fabs(delivery->tabletop_roll_angles_deg[point]) > tolerance) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "control-point scalar value is invalid");
        }
        for (size_t coordinate = 0; coordinate < 3; ++coordinate) {
            if (!is_finite(delivery->isocenter_patient_lps_cm[point * 3 + coordinate])) {
                return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                            diagnostic_capacity, "isocentre coordinate is not finite");
            }
        }
    }
    if (delivery->cumulative_meterset_fractions[0] != 0 ||
        delivery->cumulative_meterset_fractions[delivery->control_point_count - 1] != 1) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "meterset fractions must span zero to one");
    }
    for (uint32_t point = 0; point + 1 < delivery->control_point_count; ++point) {
        const double start = delivery->cumulative_meterset_fractions[point];
        const double stop = delivery->cumulative_meterset_fractions[point + 1];
        if (stop == start) {
            continue;
        }
        has_positive_segment = true;
        if (std::fabs(delivery->nominal_beam_energies_mev[point + 1] -
                      delivery->nominal_beam_energies_mev[point]) > tolerance ||
            delivery->dose_rate_present[point + 1] != delivery->dose_rate_present[point]) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "positive-MU segment changes a v1 constant");
        }
        for (size_t coordinate = 0; coordinate < 3; ++coordinate) {
            if (std::fabs(delivery->isocenter_patient_lps_cm[(point + 1) * 3 + coordinate] -
                          delivery->isocenter_patient_lps_cm[point * 3 + coordinate]) >
                tolerance) {
                return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                            diagnostic_capacity, "positive-MU segment changes isocentre");
            }
        }
        const double *angles[] = {delivery->gantry_angles_deg,
                                  delivery->collimator_angles_deg,
                                  delivery->patient_support_angles_deg};
        const int32_t *directions[] = {delivery->gantry_directions,
                                       delivery->collimator_directions,
                                       delivery->patient_support_directions};
        for (size_t rotation = 0; rotation < 3; ++rotation) {
            if (directions[rotation][point] == MCDOSE_PARTICLE_DMLC_ROTATION_NONE &&
                !same_angle(angles[rotation][point], angles[rotation][point + 1])) {
                return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                            diagnostic_capacity, "angle changes while direction is NONE");
            }
        }
    }
    if (!has_positive_segment) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "delivery has no positive-MU segment");
    }

    const uint64_t position_count =
        static_cast<uint64_t>(delivery->control_point_count) * delivery->total_pair_count;
    for (uint64_t index = 0; index < position_count; ++index) {
        if (!is_finite(delivery->bank_1_positions_cm[index]) ||
            !is_finite(delivery->bank_2_positions_cm[index])) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "device bank position is not finite");
        }
    }
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_create_delivery_context_v1(
    const mcdose_particle_dmlc_delivery_v1 *delivery,
    mcdose_particle_dmlc_delivery_context_v1 **context,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (context == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "context output pointer is null");
    }
    *context = nullptr;
    const int32_t validation = mcdose_particle_dmlc_validate_delivery_v1(
        delivery, diagnostic, diagnostic_capacity);
    if (validation != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return validation;
    }
    try {
        auto result = std::make_unique<mcdose_particle_dmlc_delivery_context_v1>();
        const size_t points = delivery->control_point_count;
        const size_t positions =
            points * static_cast<size_t>(delivery->total_pair_count);
        result->control_point_count = delivery->control_point_count;
        result->total_pair_count = delivery->total_pair_count;
        result->cumulative_meterset_fractions.assign(
            delivery->cumulative_meterset_fractions,
            delivery->cumulative_meterset_fractions + points);
        result->gantry_angles_deg.assign(delivery->gantry_angles_deg,
                                          delivery->gantry_angles_deg + points);
        result->gantry_directions.assign(delivery->gantry_directions,
                                          delivery->gantry_directions + points);
        result->collimator_angles_deg.assign(delivery->collimator_angles_deg,
                                              delivery->collimator_angles_deg + points);
        result->collimator_directions.assign(delivery->collimator_directions,
                                              delivery->collimator_directions + points);
        result->patient_support_angles_deg.assign(
            delivery->patient_support_angles_deg,
            delivery->patient_support_angles_deg + points);
        result->patient_support_directions.assign(
            delivery->patient_support_directions,
            delivery->patient_support_directions + points);
        result->isocenter_patient_lps_cm.assign(delivery->isocenter_patient_lps_cm,
                                                delivery->isocenter_patient_lps_cm + points * 3);
        result->nominal_beam_energies_mev.assign(
            delivery->nominal_beam_energies_mev,
            delivery->nominal_beam_energies_mev + points);
        result->dose_rates_mu_per_minute.assign(delivery->dose_rates_mu_per_minute,
                                                delivery->dose_rates_mu_per_minute + points);
        result->dose_rate_present.assign(delivery->dose_rate_present,
                                          delivery->dose_rate_present + points);
        result->tabletop_pitch_angles_deg.assign(
            delivery->tabletop_pitch_angles_deg,
            delivery->tabletop_pitch_angles_deg + points);
        result->tabletop_roll_angles_deg.assign(delivery->tabletop_roll_angles_deg,
                                                 delivery->tabletop_roll_angles_deg + points);
        result->bank_1_positions_cm.assign(delivery->bank_1_positions_cm,
                                            delivery->bank_1_positions_cm + positions);
        result->bank_2_positions_cm.assign(delivery->bank_2_positions_cm,
                                            delivery->bank_2_positions_cm + positions);
        *context = result.release();
    } catch (const std::bad_alloc &) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity, "cannot allocate native delivery context");
    }
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" void mcdose_particle_dmlc_destroy_delivery_context_v1(
    mcdose_particle_dmlc_delivery_context_v1 *context) {
    delete context;
}

extern "C" int32_t mcdose_particle_dmlc_sample_delivery_v1(
    const mcdose_particle_dmlc_delivery_context_v1 *context,
    double fractional_meterset,
    mcdose_particle_dmlc_sampled_state_v1 *state,
    double *bank_1_positions_cm,
    double *bank_2_positions_cm,
    uint64_t position_capacity,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (diagnostic != nullptr && diagnostic_capacity > 0) {
        diagnostic[0] = '\0';
    }
    if (context == nullptr || state == nullptr || bank_1_positions_cm == nullptr ||
        bank_2_positions_cm == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "sample call contains a null pointer");
    }
    if (state->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        state->struct_size < sizeof(mcdose_particle_dmlc_sampled_state_v1)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity, "sampled-state ABI version or size differs");
    }
    if (position_capacity < context->total_pair_count) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY, diagnostic,
                    diagnostic_capacity, "sample bank output capacity is too small");
    }
    if (!is_finite(fractional_meterset) || fractional_meterset < 0 ||
        fractional_meterset > 1) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "sampled meterset must be finite and in [0, 1]");
    }

    size_t segment = context->control_point_count;
    if (fractional_meterset == 1.0) {
        for (size_t index = context->control_point_count - 1; index > 0; --index) {
            if (context->cumulative_meterset_fractions[index] >
                context->cumulative_meterset_fractions[index - 1]) {
                segment = index - 1;
                break;
            }
        }
    } else {
        for (size_t index = 0; index + 1 < context->control_point_count; ++index) {
            const double start = context->cumulative_meterset_fractions[index];
            const double stop = context->cumulative_meterset_fractions[index + 1];
            if (stop > start && start <= fractional_meterset && fractional_meterset < stop) {
                segment = index;
                break;
            }
        }
    }
    if (segment + 1 >= context->control_point_count) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "sampled meterset is not covered by a positive segment");
    }
    const double start_fraction = context->cumulative_meterset_fractions[segment];
    const double stop_fraction = context->cumulative_meterset_fractions[segment + 1];
    const double fraction =
        (fractional_meterset - start_fraction) / (stop_fraction - start_fraction);
    state->source_segment_index = static_cast<uint32_t>(segment);
    state->reserved = 0;
    state->total_pair_count = context->total_pair_count;
    state->fractional_meterset = fractional_meterset;
    state->interpolation_fraction = fraction;
    state->gantry_angle_deg = interpolate_angle(
        context->gantry_angles_deg[segment], context->gantry_angles_deg[segment + 1],
        context->gantry_directions[segment], fraction);
    state->collimator_angle_deg = interpolate_angle(
        context->collimator_angles_deg[segment],
        context->collimator_angles_deg[segment + 1],
        context->collimator_directions[segment], fraction);
    state->patient_support_angle_deg = interpolate_angle(
        context->patient_support_angles_deg[segment],
        context->patient_support_angles_deg[segment + 1],
        context->patient_support_directions[segment], fraction);
    for (size_t coordinate = 0; coordinate < 3; ++coordinate) {
        state->isocenter_patient_lps_cm[coordinate] =
            context->isocenter_patient_lps_cm[segment * 3 + coordinate];
    }
    state->nominal_beam_energy_mev = context->nominal_beam_energies_mev[segment];
    state->dose_rate_present = context->dose_rate_present[segment];
    state->reserved_2 = 0;
    state->dose_rate_mu_per_minute =
        state->dose_rate_present
            ? interpolate(context->dose_rates_mu_per_minute[segment],
                          context->dose_rates_mu_per_minute[segment + 1], fraction)
            : 0.0;
    state->tabletop_pitch_angle_deg =
        interpolate(context->tabletop_pitch_angles_deg[segment],
                    context->tabletop_pitch_angles_deg[segment + 1], fraction);
    state->tabletop_roll_angle_deg =
        interpolate(context->tabletop_roll_angles_deg[segment],
                    context->tabletop_roll_angles_deg[segment + 1], fraction);
    const size_t first_offset = segment * context->total_pair_count;
    const size_t second_offset = (segment + 1) * context->total_pair_count;
    for (size_t position = 0; position < context->total_pair_count; ++position) {
        bank_1_positions_cm[position] =
            interpolate(context->bank_1_positions_cm[first_offset + position],
                        context->bank_1_positions_cm[second_offset + position], fraction);
        bank_2_positions_cm[position] =
            interpolate(context->bank_2_positions_cm[first_offset + position],
                        context->bank_2_positions_cm[second_offset + position], fraction);
    }
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}
