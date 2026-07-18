#include "mcdose_particle_dmlc_host_adapter_v1.h"

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
mcdose_particle_dmlc_particle_v1 particle(uint64_t particle_id, int32_t charge) {
    mcdose_particle_dmlc_particle_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.charge = charge;
    result.history_id = 17;
    result.particle_id = particle_id;
    result.parent_particle_id = 23;
    result.position_cm[2] = 40.5;
    result.direction[2] = 1.0;
    result.energy_mev = charge == MCDOSE_PARTICLE_DMLC_PHOTON ? 1.5 : 4.5;
    result.weight = 0.25;
    return result;
}

mcdose_particle_dmlc_host_product_v1 host_result() {
    mcdose_particle_dmlc_host_product_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}

mcdose_particle_dmlc_photon_attenuation_v1 attenuation() {
    mcdose_particle_dmlc_photon_attenuation_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.section_count = 2;
    result.energy_mev = 1.5;
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

mcdose_particle_dmlc_forced_interaction_v1 interaction(
    const mcdose_particle_dmlc_photon_attenuation_v1 &input) {
    mcdose_particle_dmlc_forced_interaction_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.random_draw_count = 1;
    result.uniform_random = 0.5;
    result.remaining_longitudinal_thickness_cm =
        input.total_longitudinal_thickness_cm +
        std::log(1.0 - result.uniform_random * input.interaction_probability);
    result.traversed_longitudinal_thickness_cm =
        input.total_longitudinal_thickness_cm -
        result.remaining_longitudinal_thickness_cm;
    result.traversed_path_length_cm =
        result.traversed_longitudinal_thickness_cm;
    result.remaining_path_length_cm =
        result.remaining_longitudinal_thickness_cm;
    result.interaction_optical_depth = result.traversed_path_length_cm;
    return result;
}

mcdose_particle_dmlc_compton_products_v1 compton_products() {
    mcdose_particle_dmlc_compton_products_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}

struct random_state {
    const double *values;
    size_t value_count;
    size_t next_value;
};

int32_t next_random(void *user_data, double *uniform_random) {
    if (user_data == nullptr || uniform_random == nullptr) {
        return MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT;
    }
    auto *state = static_cast<random_state *>(user_data);
    if (state->next_value >= state->value_count) {
        return MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY;
    }
    *uniform_random = state->values[state->next_value];
    ++state->next_value;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}
}  // namespace

int main() {
    char diagnostic[256];
    mcdose_particle_dmlc_host_adapter_context_v1 *context = nullptr;
    CHECK(mcdose_particle_dmlc_create_host_adapter_context_v1(
              2, &context, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(context != nullptr);

    mcdose_particle_dmlc_particle_v1 products[] = {
        particle(24, MCDOSE_PARTICLE_DMLC_PHOTON),
        particle(25, MCDOSE_PARTICLE_DMLC_ELECTRON),
    };
    CHECK(mcdose_particle_dmlc_load_host_products_v1(
              context, products, 2, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    products[0].energy_mev = 99.0;

    auto first = host_result();
    CHECK(mcdose_particle_dmlc_next_host_product_v1(
              context, &first, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(first.has_product == 1);
    CHECK(first.remaining_product_count == 1);
    CHECK(first.particle.particle_id == 24);
    CHECK(first.particle.energy_mev == 1.5);

    CHECK(mcdose_particle_dmlc_load_host_products_v1(
              context, products, 2, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    auto second = host_result();
    CHECK(mcdose_particle_dmlc_next_host_product_v1(
              context, &second, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(second.has_product == 1);
    CHECK(second.remaining_product_count == 0);
    CHECK(second.particle.particle_id == 25);

    auto empty = host_result();
    CHECK(mcdose_particle_dmlc_next_host_product_v1(
              context, &empty, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(empty.has_product == 0);
    CHECK(empty.remaining_product_count == 0);
    CHECK(mcdose_particle_dmlc_load_host_products_v1(
              context, nullptr, 0, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);

    const auto photon_input = particle(23, MCDOSE_PARTICLE_DMLC_PHOTON);
    const auto attenuation_input = attenuation();
    const auto interaction_input = interaction(attenuation_input);
    const double fixed_values[] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
    random_state fixed_state = {fixed_values, 8, 0};
    CHECK(mcdose_particle_dmlc_set_host_random_source_v1(
              context, next_random, &fixed_state, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    auto fixed_products = compton_products();
    CHECK(mcdose_particle_dmlc_sample_host_compton_products_v1(
              context, &photon_input, &attenuation_input, &interaction_input, 24,
              25, &fixed_products, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(fixed_state.next_value == 4);
    CHECK(fixed_products.random_draw_count == 4);
    CHECK(fixed_products.rejection_count == 0);
    CHECK(fixed_products.product_count == 2);

    const double rejection_values[] = {0.0, 0.5, 0.99, 0.5,
                                       0.5, 0.5, 0.5};
    random_state rejection_state = {rejection_values, 7, 0};
    CHECK(mcdose_particle_dmlc_set_host_random_source_v1(
              context, next_random, &rejection_state, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    auto rejected_once = compton_products();
    CHECK(mcdose_particle_dmlc_sample_host_compton_products_v1(
              context, &photon_input, &attenuation_input, &interaction_input, 26,
              27, &rejected_once, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(rejection_state.next_value == 7);
    CHECK(rejected_once.random_draw_count == 7);
    CHECK(rejected_once.rejection_count == 1);

    const double exhausted_values[] = {0.5, 0.5};
    random_state exhausted_state = {exhausted_values, 2, 0};
    CHECK(mcdose_particle_dmlc_set_host_random_source_v1(
              context, next_random, &exhausted_state, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    auto exhausted_products = compton_products();
    CHECK(mcdose_particle_dmlc_sample_host_compton_products_v1(
              context, &photon_input, &attenuation_input, &interaction_input, 28,
              29, &exhausted_products, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY);
    CHECK(exhausted_state.next_value == 2);
    CHECK(exhausted_products.random_draw_count == 2);

    const double invalid_values[] = {1.0};
    random_state invalid_state = {invalid_values, 1, 0};
    CHECK(mcdose_particle_dmlc_set_host_random_source_v1(
              context, next_random, &invalid_state, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    auto invalid_products = compton_products();
    CHECK(mcdose_particle_dmlc_sample_host_compton_products_v1(
              context, &photon_input, &attenuation_input, &interaction_input, 30,
              31, &invalid_products, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(invalid_state.next_value == 1);
    CHECK(invalid_products.random_draw_count == 1);

    random_state untouched_state = {fixed_values, 8, 0};
    CHECK(mcdose_particle_dmlc_set_host_random_source_v1(
              context, next_random, &untouched_state, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    auto invalid_photon = photon_input;
    invalid_photon.energy_mev = 2.0;
    auto invalid_input_products = compton_products();
    CHECK(mcdose_particle_dmlc_sample_host_compton_products_v1(
              context, &invalid_photon, &attenuation_input, &interaction_input,
              32, 33, &invalid_input_products, diagnostic,
              sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(untouched_state.next_value == 0);

    mcdose_particle_dmlc_particle_v1 too_many[] = {
        particle(26, MCDOSE_PARTICLE_DMLC_PHOTON),
        particle(27, MCDOSE_PARTICLE_DMLC_ELECTRON),
        particle(28, MCDOSE_PARTICLE_DMLC_PHOTON),
    };
    CHECK(mcdose_particle_dmlc_load_host_products_v1(
              context, too_many, 3, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY);
    too_many[0].direction[2] = 0.5;
    CHECK(mcdose_particle_dmlc_load_host_products_v1(
              context, too_many, 1, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    mcdose_particle_dmlc_destroy_host_adapter_context_v1(context);
    mcdose_particle_dmlc_destroy_host_adapter_context_v1(nullptr);
    context = nullptr;
    CHECK(mcdose_particle_dmlc_create_host_adapter_context_v1(
              0, &context, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(context == nullptr);
    return 0;
}
