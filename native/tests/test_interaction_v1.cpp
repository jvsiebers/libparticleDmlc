#include "mcdose_particle_dmlc_interaction_v1.h"

#include <cmath>
#include <cstdio>

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::fprintf(stderr, "check failed: %s at %s:%d\n", #condition,  \
                         __FILE__, __LINE__);                                  \
            return 1;                                                          \
        }                                                                      \
    } while (false)

namespace {
mcdose_particle_dmlc_photon_attenuation_v1 attenuation() {
    mcdose_particle_dmlc_photon_attenuation_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.section_count = 2;
    result.energy_mev = 1.0;
    result.direction_z = 1.0;
    result.total_longitudinal_thickness_cm = 6.2;
    result.total_path_length_cm = 6.2;
    result.total_linear_attenuation_per_cm = 1.0;
    result.incoherent_linear_attenuation_per_cm = 0.1;
    result.optical_depth = 6.2;
    result.primary_transmission = std::exp(-6.2);
    result.interaction_probability = 1.0 - result.primary_transmission;
    result.incoherent_interaction_fraction = 0.1;
    return result;
}

mcdose_particle_dmlc_forced_interaction_v1 interaction() {
    mcdose_particle_dmlc_forced_interaction_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}

mcdose_particle_dmlc_ray_v1 ray() {
    mcdose_particle_dmlc_ray_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.position_cm[0] = 1.0;
    result.position_cm[1] = 2.0;
    result.position_cm[2] = 40.0;
    result.direction[2] = 1.0;
    return result;
}

mcdose_particle_dmlc_interaction_site_v1 interaction_site() {
    mcdose_particle_dmlc_interaction_site_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}

mcdose_particle_dmlc_particle_v1 incident_photon() {
    mcdose_particle_dmlc_particle_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.charge = MCDOSE_PARTICLE_DMLC_PHOTON;
    result.history_id = 17;
    result.particle_id = 23;
    result.position_cm[2] = 40.0;
    result.direction[2] = 1.0;
    result.energy_mev = 1.0;
    result.weight = 0.8;
    return result;
}

mcdose_particle_dmlc_compton_products_v1 products() {
    mcdose_particle_dmlc_compton_products_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}

mcdose_particle_dmlc_photon_material_attenuation_v1 material_attenuation(
    double path_length = 2.0) {
    mcdose_particle_dmlc_photon_material_attenuation_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.energy_mev = 1.0;
    result.material_path_length_cm = path_length;
    result.total_linear_attenuation_per_cm = 1.0;
    result.incoherent_linear_attenuation_per_cm = 0.1;
    result.optical_depth = path_length;
    result.transmission = std::exp(-path_length);
    result.interaction_probability = -std::expm1(-path_length);
    result.incoherent_interaction_fraction = 0.1;
    return result;
}

mcdose_particle_dmlc_propagated_photon_v1 propagation() {
    mcdose_particle_dmlc_propagated_photon_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}
}  // namespace

int main() {
    static_assert(sizeof(mcdose_particle_dmlc_material_interval_v1) == 16);
    static_assert(sizeof(mcdose_particle_dmlc_interaction_site_v1) == 80);
    static_assert(sizeof(mcdose_particle_dmlc_propagated_photon_v1) == 136);
    char diagnostic[256];
    auto input = attenuation();

    auto first = interaction();
    CHECK(mcdose_particle_dmlc_sample_forced_interaction_v1(
              &input, 0.0, &first, diagnostic, sizeof(diagnostic)) == 0);
    CHECK(first.random_draw_count == 1);
    CHECK(first.traversed_longitudinal_thickness_cm == 0.0);
    CHECK(first.remaining_longitudinal_thickness_cm == 6.2);

    auto last = interaction();
    CHECK(mcdose_particle_dmlc_sample_forced_interaction_v1(
              &input, 1.0, &last, diagnostic, sizeof(diagnostic)) == 0);
    CHECK(last.traversed_longitudinal_thickness_cm == 6.2);
    CHECK(last.remaining_longitudinal_thickness_cm == 0.0);

    auto middle = interaction();
    CHECK(mcdose_particle_dmlc_sample_forced_interaction_v1(
              &input, 0.5, &middle, diagnostic, sizeof(diagnostic)) == 0);
    const double expected_remaining =
        6.2 + std::log(1.0 - 0.5 * input.interaction_probability);
    CHECK(std::fabs(middle.remaining_longitudinal_thickness_cm -
                    expected_remaining) < 1.0e-12);
    CHECK(std::fabs(middle.traversed_longitudinal_thickness_cm +
                        middle.remaining_longitudinal_thickness_cm -
                    6.2) < 1.0e-12);
    CHECK(std::fabs(middle.traversed_path_length_cm -
                    middle.traversed_longitudinal_thickness_cm) < 1.0e-12);
    CHECK(std::fabs(middle.interaction_optical_depth -
                    middle.traversed_path_length_cm) < 1.0e-12);

    const mcdose_particle_dmlc_material_interval_v1 material_intervals[] = {
        {2.0, 2.5},
        {5.0, 10.7},
    };
    const auto input_ray = ray();
    auto first_site = interaction_site();
    CHECK(mcdose_particle_dmlc_locate_forced_interaction_v1(
              &input_ray, material_intervals, 2, &input, &first, &first_site,
              diagnostic, sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(first_site.interval_index == 0);
    CHECK(first_site.material_path_within_interval_cm == 0.0);
    CHECK(first_site.ray_path_from_origin_cm == 2.0);
    CHECK(first_site.position_cm[0] == 1.0);
    CHECK(first_site.position_cm[1] == 2.0);
    CHECK(first_site.position_cm[2] == 42.0);

    auto middle_site = interaction_site();
    CHECK(mcdose_particle_dmlc_locate_forced_interaction_v1(
              &input_ray, material_intervals, 2, &input, &middle, &middle_site,
              diagnostic, sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(middle_site.interval_index == 1);
    CHECK(middle_site.interval_count == 2);
    CHECK(middle_site.material_path_before_interval_cm == 0.5);
    CHECK(std::fabs(middle_site.material_path_within_interval_cm -
                    (middle.traversed_path_length_cm - 0.5)) < 1.0e-12);
    CHECK(std::fabs(middle_site.ray_path_from_origin_cm -
                    (5.0 + middle.traversed_path_length_cm - 0.5)) < 1.0e-12);
    CHECK(std::fabs(middle_site.position_cm[2] -
                    (40.0 + middle_site.ray_path_from_origin_cm)) < 1.0e-12);

    auto last_site = interaction_site();
    CHECK(mcdose_particle_dmlc_locate_forced_interaction_v1(
              &input_ray, material_intervals, 2, &input, &last, &last_site,
              diagnostic, sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(last_site.interval_index == 1);
    CHECK(std::fabs(last_site.ray_path_from_origin_cm - 10.7) < 1.0e-12);
    CHECK(last_site.remaining_material_path_cm == 0.0);

    const mcdose_particle_dmlc_material_interval_v1 overlapping_intervals[] = {
        {2.0, 3.0},
        {2.5, 7.7},
    };
    auto invalid_site = interaction_site();
    CHECK(mcdose_particle_dmlc_locate_forced_interaction_v1(
              &input_ray, overlapping_intervals, 2, &input, &middle,
              &invalid_site, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    const mcdose_particle_dmlc_material_interval_v1 wrong_total[] = {
        {2.0, 3.0},
    };
    CHECK(mcdose_particle_dmlc_locate_forced_interaction_v1(
              &input_ray, wrong_total, 1, &input, &middle, &invalid_site,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    auto invalid = interaction();
    CHECK(mcdose_particle_dmlc_sample_forced_interaction_v1(
              &input, -0.1, &invalid, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    input.total_longitudinal_thickness_cm = 0.0;
    CHECK(mcdose_particle_dmlc_sample_forced_interaction_v1(
              &input, 0.5, &invalid, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    input = attenuation();
    input.total_path_length_cm += 0.1;
    CHECK(mcdose_particle_dmlc_sample_forced_interaction_v1(
              &input, 0.5, &invalid, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    input = attenuation();
    auto photon = incident_photon();
    auto output = products();
    double random_values[16];
    for (double &value : random_values) {
        value = 0.5;
    }
    CHECK(mcdose_particle_dmlc_sample_compton_products_v1(
              &photon, &input, &middle, random_values, 16, 24, 25, &output,
              diagnostic, sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(output.random_draw_count == 4);
    CHECK(output.rejection_count == 0);
    CHECK(output.product_count == 2);
    CHECK(output.scattered_photon.charge == MCDOSE_PARTICLE_DMLC_PHOTON);
    CHECK(output.electron.charge == MCDOSE_PARTICLE_DMLC_ELECTRON);
    CHECK(output.scattered_photon.history_id == photon.history_id);
    CHECK(output.scattered_photon.parent_particle_id == photon.particle_id);
    CHECK(output.scattered_photon.particle_id == 24);
    CHECK(output.electron.particle_id == 25);
    CHECK(std::fabs(output.scattered_photon.energy_mev +
                        output.electron.energy_mev -
                    photon.energy_mev) < 1.0e-12);
    CHECK(output.scattered_photon.position_cm[2] == photon.position_cm[2]);
    const double expected_weight =
        photon.weight * input.interaction_probability *
        input.incoherent_interaction_fraction;
    CHECK(std::fabs(output.scattered_photon.weight - expected_weight) < 1.0e-12);
    CHECK(std::fabs(output.electron.weight - expected_weight) < 1.0e-12);
    const double electron_momentum =
        std::sqrt(output.electron.energy_mev *
                  (output.electron.energy_mev + 2.0 * 0.51099906));
    for (size_t axis = 0; axis < 3; ++axis) {
        const double product_momentum =
            output.scattered_photon.energy_mev *
                output.scattered_photon.direction[axis] +
            electron_momentum * output.electron.direction[axis];
        CHECK(std::fabs(product_momentum - photon.energy_mev * photon.direction[axis]) <
              1.0e-12);
    }

    auto exhausted = products();
    CHECK(mcdose_particle_dmlc_sample_compton_products_v1(
              &photon, &input, &middle, random_values, 1, 24, 25, &exhausted,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY);
    CHECK(exhausted.random_draw_count == 1);
    auto invalid_random = products();
    const double endpoint_random[] = {1.0};
    CHECK(mcdose_particle_dmlc_sample_compton_products_v1(
              &photon, &input, &middle, endpoint_random, 1, 24, 25,
              &invalid_random, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    auto propagation_input = incident_photon();
    propagation_input.position_cm[0] = 1.0;
    propagation_input.position_cm[1] = 2.0;
    propagation_input.direction[0] = 0.6;
    propagation_input.direction[2] = 0.8;
    const auto propagation_attenuation = material_attenuation();
    auto propagated = propagation();
    CHECK(mcdose_particle_dmlc_propagate_photon_v1(
              &propagation_input, &propagation_attenuation, 3.0, &propagated,
              diagnostic, sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(propagated.has_particle == 1);
    CHECK(propagated.geometric_ray_path_cm == 3.0);
    CHECK(std::fabs(propagated.particle.position_cm[0] - 2.8) < 1.0e-12);
    CHECK(propagated.particle.position_cm[1] == 2.0);
    CHECK(std::fabs(propagated.particle.position_cm[2] - 42.4) < 1.0e-12);
    CHECK(std::fabs(propagated.particle.weight -
                    propagation_input.weight * std::exp(-2.0)) < 1.0e-12);
    CHECK(propagated.particle.history_id == propagation_input.history_id);
    CHECK(propagated.particle.particle_id == propagation_input.particle_id);
    CHECK(propagated.particle.parent_particle_id ==
          propagation_input.parent_particle_id);

    auto backward = propagation_input;
    backward.direction[0] = -0.6;
    backward.direction[2] = -0.8;
    auto backward_result = propagation();
    CHECK(mcdose_particle_dmlc_propagate_photon_v1(
              &backward, &propagation_attenuation, 3.0, &backward_result,
              diagnostic, sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(backward_result.has_particle == 1);
    CHECK(std::fabs(backward_result.particle.position_cm[2] - 37.6) < 1.0e-12);

    auto opaque_attenuation = material_attenuation(1000.0);
    auto absorbed = propagation();
    CHECK(opaque_attenuation.transmission == 0.0);
    CHECK(mcdose_particle_dmlc_propagate_photon_v1(
              &propagation_input, &opaque_attenuation, 1000.0, &absorbed,
              diagnostic, sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(absorbed.has_particle == 0);
    CHECK(absorbed.geometric_ray_path_cm == 1000.0);
    CHECK(absorbed.particle.abi_version == 0);

    auto invalid_propagation = propagation();
    CHECK(mcdose_particle_dmlc_propagate_photon_v1(
              &propagation_input, &propagation_attenuation, 1.5,
              &invalid_propagation, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    auto electron = propagation_input;
    electron.charge = MCDOSE_PARTICLE_DMLC_ELECTRON;
    CHECK(mcdose_particle_dmlc_propagate_photon_v1(
              &electron, &propagation_attenuation, 3.0, &invalid_propagation,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    auto inconsistent_attenuation = propagation_attenuation;
    inconsistent_attenuation.transmission = 0.5;
    CHECK(mcdose_particle_dmlc_propagate_photon_v1(
              &propagation_input, &inconsistent_attenuation, 3.0,
              &invalid_propagation, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    return 0;
}
