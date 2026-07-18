#include "mcdose_particle_dmlc_interaction_v1.h"

#include "context_v1.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iterator>

namespace {
constexpr double thickness_tolerance_cm = 1.0e-12;
constexpr double consistency_tolerance = 1.0e-12;
constexpr double direction_tolerance = 1.0e-10;
constexpr double electron_rest_mass_mev = 0.51099906;
constexpr double two_pi = 6.283185307179586476925286766559;
constexpr uint64_t max_material_interval_count = 1024;

int32_t fail(int32_t status, char *diagnostic, size_t capacity, const char *message) {
    if (diagnostic != nullptr && capacity > 0) {
        std::snprintf(diagnostic, capacity, "%s", message);
    }
    return status;
}

bool nearly_equal(double left, double right) {
    const double scale = std::max({1.0, std::fabs(left), std::fabs(right)});
    return std::fabs(left - right) <= consistency_tolerance * scale;
}

bool valid_attenuation(
    const mcdose_particle_dmlc_photon_attenuation_v1 *attenuation) {
    const double thickness = attenuation->total_longitudinal_thickness_cm;
    const double path_length = attenuation->total_path_length_cm;
    const double total_coefficient = attenuation->total_linear_attenuation_per_cm;
    const double incoherent_coefficient =
        attenuation->incoherent_linear_attenuation_per_cm;
    if (attenuation->section_count != 2 || attenuation->reserved != 0 ||
        !std::isfinite(attenuation->energy_mev) || attenuation->energy_mev <= 0.0 ||
        !std::isfinite(thickness) || thickness <= 0.0 ||
        !std::isfinite(attenuation->direction_z) || attenuation->direction_z <= 0.0 ||
        attenuation->direction_z > 1.0 || !std::isfinite(path_length) ||
        path_length <= 0.0 || !std::isfinite(total_coefficient) ||
        total_coefficient <= 0.0 || !std::isfinite(incoherent_coefficient) ||
        incoherent_coefficient < 0.0 || incoherent_coefficient > total_coefficient ||
        !std::isfinite(attenuation->optical_depth) ||
        attenuation->optical_depth <= 0.0 ||
        !std::isfinite(attenuation->primary_transmission) ||
        attenuation->primary_transmission < 0.0 ||
        attenuation->primary_transmission >= 1.0 ||
        !std::isfinite(attenuation->interaction_probability) ||
        attenuation->interaction_probability <= 0.0 ||
        attenuation->interaction_probability > 1.0 ||
        !std::isfinite(attenuation->incoherent_interaction_fraction) ||
        attenuation->incoherent_interaction_fraction < 0.0 ||
        attenuation->incoherent_interaction_fraction > 1.0) {
        return false;
    }
    const double expected_path_length = thickness / attenuation->direction_z;
    const double expected_optical_depth = total_coefficient * expected_path_length;
    return nearly_equal(path_length, expected_path_length) &&
           nearly_equal(attenuation->optical_depth, expected_optical_depth) &&
           nearly_equal(attenuation->primary_transmission,
                        std::exp(-expected_optical_depth)) &&
           nearly_equal(attenuation->interaction_probability,
                        -std::expm1(-expected_optical_depth)) &&
           nearly_equal(attenuation->incoherent_interaction_fraction,
                        incoherent_coefficient / total_coefficient);
}

bool valid_interaction(
    const mcdose_particle_dmlc_forced_interaction_v1 *interaction,
    const mcdose_particle_dmlc_photon_attenuation_v1 *attenuation) {
    const double values[] = {
        interaction->uniform_random,
        interaction->traversed_longitudinal_thickness_cm,
        interaction->remaining_longitudinal_thickness_cm,
        interaction->traversed_path_length_cm,
        interaction->remaining_path_length_cm,
        interaction->interaction_optical_depth,
    };
    if (interaction->random_draw_count != 1 || interaction->reserved != 0 ||
        interaction->uniform_random < 0.0 || interaction->uniform_random > 1.0) {
        return false;
    }
    for (double value : values) {
        if (!std::isfinite(value) || value < 0.0) {
            return false;
        }
    }
    return nearly_equal(interaction->traversed_longitudinal_thickness_cm +
                            interaction->remaining_longitudinal_thickness_cm,
                        attenuation->total_longitudinal_thickness_cm) &&
           nearly_equal(interaction->traversed_path_length_cm +
                            interaction->remaining_path_length_cm,
                        attenuation->total_path_length_cm) &&
           nearly_equal(interaction->interaction_optical_depth,
                        attenuation->total_linear_attenuation_per_cm *
                            interaction->traversed_path_length_cm);
}

bool valid_forced_interaction_record(
    const mcdose_particle_dmlc_forced_interaction_v1 *interaction) {
    const double values[] = {
        interaction->uniform_random,
        interaction->traversed_longitudinal_thickness_cm,
        interaction->remaining_longitudinal_thickness_cm,
        interaction->traversed_path_length_cm,
        interaction->remaining_path_length_cm,
        interaction->interaction_optical_depth,
    };
    if (interaction->random_draw_count != 1 || interaction->reserved != 0 ||
        interaction->uniform_random < 0.0 || interaction->uniform_random > 1.0) {
        return false;
    }
    for (double value : values) {
        if (!std::isfinite(value) || value < 0.0) {
            return false;
        }
    }
    return true;
}

bool valid_equivalent_classification(
    const mcdose_particle_dmlc_classification_v1 *classification,
    double *total_longitudinal_thickness_cm,
    bool *outside) {
    if (classification->section_count != 2 || classification->reserved != 0) {
        return false;
    }
    double total = 0.0;
    bool any_outside = false;
    for (size_t section = 0; section < 2; ++section) {
        const int32_t region = classification->region[section];
        const double thickness =
            classification->resolved_longitudinal_thickness_cm[section];
        if (classification->reserved_2[section] != 0 ||
            classification->thickness_resolved[section] != 1 ||
            region < MCDOSE_PARTICLE_DMLC_SECTION_OPEN ||
            region > MCDOSE_PARTICLE_DMLC_SECTION_OUTSIDE ||
            !std::isfinite(thickness) || thickness < 0.0) {
            return false;
        }
        total += thickness;
        any_outside = any_outside ||
                      region == MCDOSE_PARTICLE_DMLC_SECTION_OUTSIDE;
    }
    if (!std::isfinite(total)) {
        return false;
    }
    *total_longitudinal_thickness_cm = total;
    *outside = any_outside;
    return true;
}

bool valid_forward_particle(const mcdose_particle_dmlc_particle_v1 *particle) {
    if (particle->charge < MCDOSE_PARTICLE_DMLC_ELECTRON ||
        particle->charge > MCDOSE_PARTICLE_DMLC_POSITRON ||
        particle->reserved != 0 || particle->reserved_2 != 0 ||
        particle->history_id == 0 || particle->particle_id == 0 ||
        !std::isfinite(particle->energy_mev) || particle->energy_mev <= 0.0 ||
        !std::isfinite(particle->weight) || particle->weight <= 0.0) {
        return false;
    }
    double norm_squared = 0.0;
    for (size_t index = 0; index < 3; ++index) {
        if (!std::isfinite(particle->position_cm[index]) ||
            !std::isfinite(particle->direction[index])) {
            return false;
        }
        norm_squared += particle->direction[index] * particle->direction[index];
    }
    return particle->direction[2] > 0.0 &&
           std::fabs(norm_squared - 1.0) <= direction_tolerance;
}

bool valid_incident_photon(const mcdose_particle_dmlc_particle_v1 *particle) {
    return particle->charge == MCDOSE_PARTICLE_DMLC_PHOTON &&
           valid_forward_particle(particle);
}

bool valid_propagating_photon(
    const mcdose_particle_dmlc_particle_v1 *particle) {
    if (particle->charge != MCDOSE_PARTICLE_DMLC_PHOTON ||
        particle->reserved != 0 || particle->reserved_2 != 0 ||
        particle->history_id == 0 || particle->particle_id == 0 ||
        !std::isfinite(particle->energy_mev) || particle->energy_mev <= 0.0 ||
        !std::isfinite(particle->weight) || particle->weight <= 0.0) {
        return false;
    }
    double norm_squared = 0.0;
    for (size_t index = 0; index < 3; ++index) {
        if (!std::isfinite(particle->position_cm[index]) ||
            !std::isfinite(particle->direction[index])) {
            return false;
        }
        norm_squared += particle->direction[index] * particle->direction[index];
    }
    return std::fabs(norm_squared - 1.0) <= direction_tolerance;
}

bool valid_material_attenuation(
    const mcdose_particle_dmlc_photon_material_attenuation_v1 *attenuation) {
    const double total = attenuation->total_linear_attenuation_per_cm;
    const double incoherent = attenuation->incoherent_linear_attenuation_per_cm;
    const double path = attenuation->material_path_length_cm;
    if (attenuation->reserved[0] != 0 || attenuation->reserved[1] != 0 ||
        !std::isfinite(attenuation->energy_mev) || attenuation->energy_mev <= 0.0 ||
        !std::isfinite(path) || path < 0.0 || !std::isfinite(total) || total < 0.0 ||
        !std::isfinite(incoherent) || incoherent < 0.0 || incoherent > total ||
        !std::isfinite(attenuation->optical_depth) ||
        attenuation->optical_depth < 0.0 ||
        !std::isfinite(attenuation->transmission) ||
        attenuation->transmission < 0.0 || attenuation->transmission > 1.0 ||
        !std::isfinite(attenuation->interaction_probability) ||
        attenuation->interaction_probability < 0.0 ||
        attenuation->interaction_probability > 1.0 ||
        !std::isfinite(attenuation->incoherent_interaction_fraction) ||
        attenuation->incoherent_interaction_fraction < 0.0 ||
        attenuation->incoherent_interaction_fraction > 1.0) {
        return false;
    }
    const double optical_depth = total * path;
    const double expected_fraction = total == 0.0 ? 0.0 : incoherent / total;
    return nearly_equal(attenuation->optical_depth, optical_depth) &&
           nearly_equal(attenuation->transmission, std::exp(-optical_depth)) &&
           nearly_equal(attenuation->interaction_probability,
                        -std::expm1(-optical_depth)) &&
           nearly_equal(attenuation->incoherent_interaction_fraction,
                        expected_fraction);
}

bool valid_ray(const mcdose_particle_dmlc_ray_v1 *ray) {
    double norm_squared = 0.0;
    for (size_t index = 0; index < 3; ++index) {
        if (!std::isfinite(ray->position_cm[index]) ||
            !std::isfinite(ray->direction[index])) {
            return false;
        }
        norm_squared += ray->direction[index] * ray->direction[index];
    }
    return ray->direction[2] > 0.0 &&
           std::fabs(norm_squared - 1.0) <= direction_tolerance;
}

void rotate_from_incident(const double incident[3],
                          double local_cosine,
                          double azimuth,
                          double transverse_sign,
                          double result[3]) {
    const double transverse =
        std::sqrt(std::max(0.0, 1.0 - local_cosine * local_cosine));
    const double local_u = transverse_sign * transverse * std::cos(azimuth);
    const double local_v = transverse_sign * transverse * std::sin(azimuth);
    const double rho = std::hypot(incident[0], incident[1]);
    const double cosine_alpha = rho > 0.0 ? incident[0] / rho : 1.0;
    const double sine_alpha = rho > 0.0 ? incident[1] / rho : 0.0;
    const double cosine_omega = incident[2];
    const double sine_omega = rho;
    result[0] = cosine_alpha * cosine_omega * local_u - sine_alpha * local_v +
                cosine_alpha * sine_omega * local_cosine;
    result[1] = cosine_omega * sine_alpha * local_u + cosine_alpha * local_v +
                sine_alpha * sine_omega * local_cosine;
    result[2] = -sine_omega * local_u + cosine_omega * local_cosine;
    const double norm = std::sqrt(result[0] * result[0] + result[1] * result[1] +
                                  result[2] * result[2]);
    for (size_t index = 0; index < 3; ++index) {
        result[index] /= norm;
    }
}

void set_product(mcdose_particle_dmlc_particle_v1 *product,
                 const mcdose_particle_dmlc_particle_v1 *incident,
                 uint64_t particle_id,
                 int32_t charge,
                 const double position[3],
                 const double direction[3],
                 double energy_mev,
                 double weight) {
    product->abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    product->struct_size = sizeof(*product);
    product->charge = charge;
    product->reserved = 0;
    product->history_id = incident->history_id;
    product->particle_id = particle_id;
    product->parent_particle_id = incident->particle_id;
    product->reserved_2 = 0;
    for (size_t index = 0; index < 3; ++index) {
        product->position_cm[index] = position[index];
        product->direction[index] = direction[index];
    }
    product->energy_mev = energy_mev;
    product->weight = weight;
}
}  // namespace

extern "C" int32_t mcdose_particle_dmlc_sample_forced_interaction_v1(
    const mcdose_particle_dmlc_photon_attenuation_v1 *attenuation,
    double uniform_random,
    mcdose_particle_dmlc_forced_interaction_v1 *interaction,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (diagnostic != nullptr && diagnostic_capacity > 0) {
        diagnostic[0] = '\0';
    }
    if (attenuation == nullptr || interaction == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "forced interaction contains a null pointer");
    }
    if (attenuation->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        attenuation->struct_size <
            sizeof(mcdose_particle_dmlc_photon_attenuation_v1) ||
        interaction->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        interaction->struct_size <
            sizeof(mcdose_particle_dmlc_forced_interaction_v1)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "forced interaction ABI version or size differs");
    }
    const double thickness = attenuation->total_longitudinal_thickness_cm;
    if (!std::isfinite(uniform_random) || uniform_random < 0.0 ||
        uniform_random > 1.0 || !valid_attenuation(attenuation)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "forced interaction inputs are invalid");
    }
    const double attenuation_per_longitudinal_cm =
        attenuation->optical_depth / thickness;
    const double inside_log =
        1.0 - uniform_random * attenuation->interaction_probability;
    double remaining = 0.0;
    if (uniform_random == 0.0) {
        remaining = thickness;
    } else if (uniform_random == 1.0 || inside_log <= 0.0) {
        remaining = 0.0;
    } else {
        remaining = thickness +
                    std::log(inside_log) / attenuation_per_longitudinal_cm;
    }
    if (!std::isfinite(remaining) || remaining < -thickness_tolerance_cm ||
        remaining > thickness + thickness_tolerance_cm) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "forced interaction sampled outside the MLC thickness");
    }
    remaining = std::clamp(remaining, 0.0, thickness);
    const double traversed = thickness - remaining;
    interaction->random_draw_count = 1;
    interaction->reserved = 0;
    interaction->uniform_random = uniform_random;
    interaction->traversed_longitudinal_thickness_cm = traversed;
    interaction->remaining_longitudinal_thickness_cm = remaining;
    interaction->traversed_path_length_cm = traversed / attenuation->direction_z;
    interaction->remaining_path_length_cm = remaining / attenuation->direction_z;
    interaction->interaction_optical_depth =
        attenuation_per_longitudinal_cm * traversed;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_locate_forced_interaction_v1(
    const mcdose_particle_dmlc_ray_v1 *ray,
    const mcdose_particle_dmlc_material_interval_v1 *material_intervals,
    uint64_t material_interval_count,
    const mcdose_particle_dmlc_photon_attenuation_v1 *attenuation,
    const mcdose_particle_dmlc_forced_interaction_v1 *interaction,
    mcdose_particle_dmlc_interaction_site_v1 *site,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (diagnostic != nullptr && diagnostic_capacity > 0) {
        diagnostic[0] = '\0';
    }
    if (ray == nullptr || material_intervals == nullptr || attenuation == nullptr ||
        interaction == nullptr || site == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "interaction-site locator contains a null pointer");
    }
    if (ray->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        ray->struct_size < sizeof(mcdose_particle_dmlc_ray_v1) ||
        attenuation->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        attenuation->struct_size <
            sizeof(mcdose_particle_dmlc_photon_attenuation_v1) ||
        interaction->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        interaction->struct_size <
            sizeof(mcdose_particle_dmlc_forced_interaction_v1) ||
        site->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        site->struct_size < sizeof(mcdose_particle_dmlc_interaction_site_v1)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "interaction-site locator ABI version or size differs");
    }
    if (material_interval_count == 0 ||
        material_interval_count > max_material_interval_count || !valid_ray(ray) ||
        !valid_attenuation(attenuation) ||
        !valid_interaction(interaction, attenuation) ||
        !nearly_equal(ray->direction[2], attenuation->direction_z)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "interaction-site locator inputs are invalid");
    }

    double total_material_path_cm = 0.0;
    double previous_end_cm = 0.0;
    for (uint64_t index = 0; index < material_interval_count; ++index) {
        const double start_cm = material_intervals[index].ray_path_start_cm;
        const double end_cm = material_intervals[index].ray_path_end_cm;
        if (!std::isfinite(start_cm) || !std::isfinite(end_cm) || start_cm < 0.0 ||
            end_cm <= start_cm ||
            (index > 0 && start_cm < previous_end_cm)) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity,
                        "material intervals are invalid, unordered, or overlapping");
        }
        total_material_path_cm += end_cm - start_cm;
        if (!std::isfinite(total_material_path_cm)) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity,
                        "material interval path length is not finite");
        }
        previous_end_cm = end_cm;
    }
    if (!nearly_equal(total_material_path_cm, attenuation->total_path_length_cm)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "material interval lengths differ from attenuation path length");
    }

    const double target_material_path_cm = interaction->traversed_path_length_cm;
    double material_path_before_interval_cm = 0.0;
    uint64_t selected_index = material_interval_count;
    double material_path_within_interval_cm = 0.0;
    for (uint64_t index = 0; index < material_interval_count; ++index) {
        const double interval_length_cm =
            material_intervals[index].ray_path_end_cm -
            material_intervals[index].ray_path_start_cm;
        if (target_material_path_cm <=
                material_path_before_interval_cm + interval_length_cm +
                    thickness_tolerance_cm ||
            index + 1 == material_interval_count) {
            selected_index = index;
            material_path_within_interval_cm = std::clamp(
                target_material_path_cm - material_path_before_interval_cm, 0.0,
                interval_length_cm);
            break;
        }
        material_path_before_interval_cm += interval_length_cm;
    }
    if (selected_index == material_interval_count) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "interaction material path could not be located");
    }

    const double ray_path_from_origin_cm =
        material_intervals[selected_index].ray_path_start_cm +
        material_path_within_interval_cm;
    site->interval_index = static_cast<uint32_t>(selected_index);
    site->interval_count = static_cast<uint32_t>(material_interval_count);
    site->material_path_before_interval_cm = material_path_before_interval_cm;
    site->material_path_within_interval_cm = material_path_within_interval_cm;
    site->traversed_material_path_cm = target_material_path_cm;
    site->remaining_material_path_cm = interaction->remaining_path_length_cm;
    site->ray_path_from_origin_cm = ray_path_from_origin_cm;
    for (size_t index = 0; index < 3; ++index) {
        site->position_cm[index] =
            ray->position_cm[index] + ray->direction[index] * ray_path_from_origin_cm;
    }
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_sample_compton_products_v1(
    const mcdose_particle_dmlc_particle_v1 *incident_photon,
    const mcdose_particle_dmlc_photon_attenuation_v1 *attenuation,
    const mcdose_particle_dmlc_forced_interaction_v1 *interaction,
    const double *uniform_random_values,
    uint64_t uniform_random_value_count,
    uint64_t scattered_photon_particle_id,
    uint64_t electron_particle_id,
    mcdose_particle_dmlc_compton_products_v1 *products,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (diagnostic != nullptr && diagnostic_capacity > 0) {
        diagnostic[0] = '\0';
    }
    if (incident_photon == nullptr || attenuation == nullptr ||
        interaction == nullptr || products == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "Compton product call contains a null pointer");
    }
    if (incident_photon->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        incident_photon->struct_size < sizeof(mcdose_particle_dmlc_particle_v1) ||
        attenuation->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        attenuation->struct_size <
            sizeof(mcdose_particle_dmlc_photon_attenuation_v1) ||
        interaction->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        interaction->struct_size <
            sizeof(mcdose_particle_dmlc_forced_interaction_v1) ||
        products->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        products->struct_size < sizeof(mcdose_particle_dmlc_compton_products_v1)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity, "Compton product ABI version or size differs");
    }
    products->random_draw_count = 0;
    products->rejection_count = 0;
    products->product_count = 0;
    std::fill(std::begin(products->reserved), std::end(products->reserved), 0);
    if ((uniform_random_value_count > 0 && uniform_random_values == nullptr) ||
        uniform_random_value_count > UINT32_MAX ||
        !valid_incident_photon(incident_photon) || !valid_attenuation(attenuation) ||
        !valid_interaction(interaction, attenuation) ||
        !nearly_equal(incident_photon->energy_mev, attenuation->energy_mev) ||
        attenuation->incoherent_interaction_fraction <= 0.0 ||
        scattered_photon_particle_id == 0 || electron_particle_id == 0 ||
        scattered_photon_particle_id == electron_particle_id ||
        scattered_photon_particle_id == incident_photon->particle_id ||
        electron_particle_id == incident_photon->particle_id) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "Compton product inputs are invalid");
    }

    uint64_t random_index = 0;
    auto draw = [&](double *value) -> int32_t {
        if (random_index >= uniform_random_value_count) {
            products->random_draw_count = static_cast<uint32_t>(random_index);
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY, diagnostic,
                        diagnostic_capacity, "Compton random buffer is exhausted");
        }
        const double candidate = uniform_random_values[random_index];
        if (!std::isfinite(candidate) || candidate < 0.0 || candidate >= 1.0) {
            products->random_draw_count = static_cast<uint32_t>(random_index);
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "Compton random value is outside [0,1)");
        }
        *value = candidate;
        ++random_index;
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    };

    const double scaled_energy = incident_photon->energy_mev / electron_rest_mass_mev;
    const double inverse_minimum_ratio = 1.0 + 2.0 * scaled_energy;
    const double alpha_1 = std::log(inverse_minimum_ratio);
    const double alpha_2 = scaled_energy * (inverse_minimum_ratio + 1.0) /
                           (inverse_minimum_ratio * inverse_minimum_ratio);
    const double alpha_sum = alpha_1 + alpha_2;
    double energy_ratio = 0.0;
    double photon_cosine = 0.0;
    uint32_t rejection_count = 0;
    while (true) {
        double selector = 0.0;
        int32_t status = draw(&selector);
        if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            return status;
        }
        if (alpha_1 >= selector * alpha_sum) {
            double inverse_sample = 0.0;
            status = draw(&inverse_sample);
            if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
                return status;
            }
            energy_ratio =
                std::exp(alpha_1 * inverse_sample) / inverse_minimum_ratio;
        } else {
            double ratio_sample = 0.0;
            double comparison = 0.0;
            status = draw(&ratio_sample);
            if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
                return status;
            }
            status = draw(&comparison);
            if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
                return status;
            }
            if (scaled_energy >= (scaled_energy + 1.0) * comparison) {
                double maximum_sample = 0.0;
                status = draw(&maximum_sample);
                if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
                    return status;
                }
                ratio_sample = std::max(ratio_sample, maximum_sample);
            }
            energy_ratio =
                (1.0 + (inverse_minimum_ratio - 1.0) * ratio_sample) /
                inverse_minimum_ratio;
        }
        const double one_minus_ratio = 1.0 - energy_ratio;
        const double one_minus_cosine = electron_rest_mass_mev * one_minus_ratio /
                                        (energy_ratio * incident_photon->energy_mev);
        photon_cosine = std::clamp(1.0 - one_minus_cosine, -1.0, 1.0);
        const double sine_squared =
            std::max(0.0, one_minus_cosine * (2.0 - one_minus_cosine));
        const double acceptance_probability =
            std::clamp(1.0 - energy_ratio * sine_squared /
                                 (1.0 + energy_ratio * energy_ratio),
                       0.0, 1.0);
        double acceptance = 0.0;
        status = draw(&acceptance);
        if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            return status;
        }
        if (acceptance <= acceptance_probability) {
            break;
        }
        ++rejection_count;
    }
    double azimuth_sample = 0.0;
    const int32_t azimuth_status = draw(&azimuth_sample);
    if (azimuth_status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return azimuth_status;
    }

    const double scattered_energy = energy_ratio * incident_photon->energy_mev;
    const double electron_energy = incident_photon->energy_mev - scattered_energy;
    const double electron_momentum =
        std::sqrt(electron_energy *
                  (electron_energy + 2.0 * electron_rest_mass_mev));
    if (!std::isfinite(scattered_energy) || !std::isfinite(electron_energy) ||
        scattered_energy <= 0.0 || electron_energy <= 0.0 ||
        electron_momentum <= 0.0) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "Compton product energy is invalid");
    }
    const double electron_cosine = std::clamp(
        (incident_photon->energy_mev - scattered_energy * photon_cosine) /
            electron_momentum,
        -1.0, 1.0);
    const double azimuth = azimuth_sample * two_pi;
    double scattered_direction[3] = {};
    double electron_direction[3] = {};
    rotate_from_incident(incident_photon->direction, photon_cosine, azimuth, 1.0,
                         scattered_direction);
    rotate_from_incident(incident_photon->direction, electron_cosine, azimuth, -1.0,
                         electron_direction);
    double interaction_position[3] = {};
    for (size_t index = 0; index < 3; ++index) {
        interaction_position[index] = incident_photon->position_cm[index];
    }
    const double product_weight =
        incident_photon->weight * attenuation->interaction_probability *
        attenuation->incoherent_interaction_fraction;
    if (!std::isfinite(product_weight) || product_weight <= 0.0) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "Compton product weight is invalid");
    }
    set_product(&products->scattered_photon, incident_photon,
                scattered_photon_particle_id, MCDOSE_PARTICLE_DMLC_PHOTON,
                interaction_position, scattered_direction, scattered_energy,
                product_weight);
    set_product(&products->electron, incident_photon, electron_particle_id,
                MCDOSE_PARTICLE_DMLC_ELECTRON, interaction_position,
                electron_direction, electron_energy, product_weight);
    products->random_draw_count = static_cast<uint32_t>(random_index);
    products->rejection_count = rejection_count;
    products->product_count = 2;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_propagate_photon_v1(
    const mcdose_particle_dmlc_particle_v1 *incident_photon,
    const mcdose_particle_dmlc_photon_material_attenuation_v1 *attenuation,
    double geometric_ray_path_cm,
    mcdose_particle_dmlc_propagated_photon_v1 *propagation,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (diagnostic != nullptr && diagnostic_capacity > 0) {
        diagnostic[0] = '\0';
    }
    if (incident_photon == nullptr || attenuation == nullptr ||
        propagation == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "photon propagation contains a null pointer");
    }
    if (incident_photon->abi_version !=
            MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        incident_photon->struct_size <
            sizeof(mcdose_particle_dmlc_particle_v1) ||
        attenuation->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        attenuation->struct_size <
            sizeof(mcdose_particle_dmlc_photon_material_attenuation_v1) ||
        propagation->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        propagation->struct_size <
            sizeof(mcdose_particle_dmlc_propagated_photon_v1)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "photon propagation ABI version or size differs");
    }
    if (!valid_propagating_photon(incident_photon) ||
        !valid_material_attenuation(attenuation) ||
        !nearly_equal(incident_photon->energy_mev, attenuation->energy_mev) ||
        !std::isfinite(geometric_ray_path_cm) || geometric_ray_path_cm < 0.0 ||
        geometric_ray_path_cm + thickness_tolerance_cm <
            attenuation->material_path_length_cm) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "photon propagation inputs are invalid");
    }
    propagation->has_particle = 0;
    propagation->reserved = 0;
    propagation->geometric_ray_path_cm = geometric_ray_path_cm;
    propagation->particle = {};
    const double output_weight = incident_photon->weight * attenuation->transmission;
    if (!std::isfinite(output_weight)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "photon propagation output weight is invalid");
    }
    if (output_weight == 0.0) {
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }
    propagation->particle = *incident_photon;
    for (size_t index = 0; index < 3; ++index) {
        propagation->particle.position_cm[index] =
            incident_photon->position_cm[index] +
            geometric_ray_path_cm * incident_photon->direction[index];
        if (!std::isfinite(propagation->particle.position_cm[index])) {
            propagation->particle = {};
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                        diagnostic, diagnostic_capacity,
                        "photon propagation output position is invalid");
        }
    }
    propagation->particle.weight = output_weight;
    propagation->has_particle = 1;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_finalize_equivalent_primary_v1(
    const mcdose_particle_dmlc_machine_context_v1 *machine,
    const mcdose_particle_dmlc_classification_v1 *classification,
    const mcdose_particle_dmlc_particle_v1 *incident_particle,
    mcdose_particle_dmlc_equivalent_primary_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (diagnostic != nullptr && diagnostic_capacity > 0) {
        diagnostic[0] = '\0';
    }
    if (machine == nullptr || classification == nullptr ||
        incident_particle == nullptr || result == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "equivalent primary call contains a null pointer");
    }
    if (classification->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        classification->struct_size < sizeof(*classification) ||
        incident_particle->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        incident_particle->struct_size < sizeof(*incident_particle) ||
        result->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        result->struct_size < sizeof(*result)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "equivalent primary ABI version or size differs");
    }
    double total_longitudinal_thickness = 0.0;
    bool outside = false;
    if (!valid_forward_particle(incident_particle) ||
        !valid_equivalent_classification(
            classification, &total_longitudinal_thickness, &outside)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "equivalent primary inputs are invalid");
    }

    result->has_particle = 0;
    result->reserved = 0;
    result->total_longitudinal_thickness_cm = total_longitudinal_thickness;
    result->photon_attenuation = {};
    result->particle = {};
    if (outside) {
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }
    if (incident_particle->charge != MCDOSE_PARTICLE_DMLC_PHOTON) {
        if (total_longitudinal_thickness == 0.0) {
            result->particle = *incident_particle;
            result->has_particle = 1;
        }
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }

    result->photon_attenuation.abi_version =
        MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result->photon_attenuation.struct_size =
        sizeof(result->photon_attenuation);
    const int32_t attenuation_status =
        mcdose_particle_dmlc_evaluate_photon_attenuation_v1(
            machine, classification, incident_particle->energy_mev,
            incident_particle->direction[2], &result->photon_attenuation,
            diagnostic, diagnostic_capacity);
    if (attenuation_status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        result->photon_attenuation = {};
        return attenuation_status;
    }
    const double output_weight =
        incident_particle->weight * result->photon_attenuation.primary_transmission;
    if (!std::isfinite(output_weight)) {
        result->photon_attenuation = {};
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "equivalent primary output weight is not finite");
    }
    if (output_weight == 0.0) {
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }
    result->particle = *incident_particle;
    result->particle.weight = output_weight;
    result->has_particle = 1;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t
mcdose_particle_dmlc_finalize_equivalent_scattered_photon_v1(
    const mcdose_particle_dmlc_machine_context_v1 *machine,
    const mcdose_particle_dmlc_classification_v1 *classification,
    const mcdose_particle_dmlc_particle_v1 *scattered_photon,
    const mcdose_particle_dmlc_forced_interaction_v1 *interaction,
    mcdose_particle_dmlc_equivalent_scattered_photon_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (diagnostic != nullptr && diagnostic_capacity > 0) {
        diagnostic[0] = '\0';
    }
    if (machine == nullptr || classification == nullptr ||
        scattered_photon == nullptr || interaction == nullptr ||
        result == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "equivalent scattered-photon call contains a null pointer");
    }
    if (classification->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        classification->struct_size < sizeof(*classification) ||
        scattered_photon->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        scattered_photon->struct_size < sizeof(*scattered_photon) ||
        interaction->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        interaction->struct_size < sizeof(*interaction) ||
        result->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        result->struct_size < sizeof(*result)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "equivalent scattered-photon ABI version or size differs");
    }
    double total_longitudinal_thickness = 0.0;
    bool outside = false;
    if (!valid_propagating_photon(scattered_photon) ||
        !valid_forced_interaction_record(interaction) ||
        !valid_equivalent_classification(
            classification, &total_longitudinal_thickness, &outside) ||
        outside ||
        !nearly_equal(
            interaction->traversed_longitudinal_thickness_cm +
                interaction->remaining_longitudinal_thickness_cm,
            total_longitudinal_thickness)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "equivalent scattered-photon inputs are invalid");
    }
    result->has_particle = 0;
    result->reserved = 0;
    result->reference_plane_displacement_cm = 0.0;
    result->material_attenuation = {};
    result->particle = {};
    const double direction_z = scattered_photon->direction[2];
    if (direction_z <= 0.0) {
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }
    const double remaining_path =
        interaction->remaining_longitudinal_thickness_cm / direction_z;
    if (!std::isfinite(remaining_path)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "equivalent scattered-photon path is not finite");
    }
    result->material_attenuation.abi_version =
        MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result->material_attenuation.struct_size =
        sizeof(result->material_attenuation);
    const int32_t attenuation_status =
        mcdose_particle_dmlc_evaluate_photon_material_path_v1(
            machine, scattered_photon->energy_mev, remaining_path,
            &result->material_attenuation, diagnostic, diagnostic_capacity);
    if (attenuation_status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        result->material_attenuation = {};
        return attenuation_status;
    }
    const double source_side_effective_entry =
        machine->section_z_exit_cm[0] -
        classification->resolved_longitudinal_thickness_cm[0];
    result->reference_plane_displacement_cm =
        scattered_photon->position_cm[2] - source_side_effective_entry -
        interaction->remaining_longitudinal_thickness_cm;
    const double output_weight =
        scattered_photon->weight * result->material_attenuation.transmission;
    if (!std::isfinite(result->reference_plane_displacement_cm) ||
        !std::isfinite(output_weight)) {
        result->material_attenuation = {};
        result->reference_plane_displacement_cm = 0.0;
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "equivalent scattered-photon output is not finite");
    }
    if (output_weight == 0.0) {
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }
    result->particle = *scattered_photon;
    result->particle.position_cm[0] =
        scattered_photon->position_cm[0] +
        scattered_photon->direction[0] / direction_z *
            result->reference_plane_displacement_cm;
    result->particle.position_cm[1] =
        scattered_photon->position_cm[1] +
        scattered_photon->direction[1] / direction_z *
            result->reference_plane_displacement_cm;
    result->particle.weight = output_weight;
    if (!std::isfinite(result->particle.position_cm[0]) ||
        !std::isfinite(result->particle.position_cm[1])) {
        result->particle = {};
        result->material_attenuation = {};
        result->reference_plane_displacement_cm = 0.0;
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "equivalent scattered-photon position is not finite");
    }
    result->has_particle = 1;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}
