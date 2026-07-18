#include "mcdose_particle_dmlc_egsnrc_source_v1.h"

#include <cstdio>
#include <cstring>
#include <utility>

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::fprintf(stderr, "check failed: %s at %s:%d\n", #condition,  \
                         __FILE__, __LINE__);                                  \
            return 1;                                                          \
        }                                                                      \
    } while (false)

namespace {
struct random_state {
    double values[7] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
    size_t next = 0;
};

int32_t next_random(void *user_data, double *value) {
    if (user_data == nullptr || value == nullptr) {
        return MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT;
    }
    auto *state = static_cast<random_state *>(user_data);
    if (state->next >= 7) {
        return MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY;
    }
    *value = state->values[state->next++];
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

mcdose_particle_dmlc_egsnrc_stack_entry_v1 incident() {
    mcdose_particle_dmlc_egsnrc_stack_entry_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.iq = MCDOSE_PARTICLE_DMLC_PHOTON;
    result.region = 3;
    result.latch = 42;
    result.history_id = 17;
    result.particle_id = 23;
    result.total_energy_mev = 1.0;
    result.position_cm[0] = 1.1;
    result.position_cm[1] = -0.5;
    result.position_cm[2] = 45.0;
    result.direction[2] = 1.0;
    result.weight = 0.25;
    return result;
}

mcdose_particle_dmlc_producer_summary_v1 summary() {
    mcdose_particle_dmlc_producer_summary_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}

mcdose_particle_dmlc_egsnrc_source_result_v1 source_result() {
    mcdose_particle_dmlc_egsnrc_source_result_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}

int test_jaw_behavior(const char *static_path, const char *tracked_path,
                      char *diagnostic, size_t diagnostic_capacity) {
    random_state random;
    auto input = incident();
    input.iq = MCDOSE_PARTICLE_DMLC_ELECTRON;
    input.total_energy_mev =
        4.0 + MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV;
    input.position_cm[0] = 0.5;

    for (const auto &test : {std::pair{static_path, true},
                             std::pair{tracked_path, false}}) {
        mcdose_particle_dmlc_startup_info_v1 info = {};
        info.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        info.struct_size = sizeof(info);
        mcdose_particle_dmlc_egsnrc_source_context_v1 *context = nullptr;
        CHECK(mcdose_particle_dmlc_create_egsnrc_source_from_startup_v1(
                  test.first,
                  MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV,
                  next_random, &random, &context, &info, diagnostic,
                  diagnostic_capacity) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
        auto result_summary = summary();
        CHECK(mcdose_particle_dmlc_begin_egsnrc_source_particle_v1(
                  context, &input, 1.0, 0, 0, &result_summary, diagnostic,
                  diagnostic_capacity) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
        CHECK(result_summary.random_draw_count == 0);
        CHECK(result_summary.fractional_meterset == 1.0);
        CHECK(result_summary.interpolation_fraction == 1.0);
        CHECK(result_summary.retained_product_count == (test.second ? 1u : 0u));
        auto output = source_result();
        CHECK(mcdose_particle_dmlc_next_egsnrc_source_entry_v1(
                  context, &output, diagnostic, diagnostic_capacity) ==
              MCDOSE_PARTICLE_DMLC_STATUS_OK);
        CHECK(output.has_entry == (test.second ? 1u : 0u));
        mcdose_particle_dmlc_destroy_egsnrc_source_context_v1(context);
    }
    CHECK(random.next == 0);

    mcdose_particle_dmlc_startup_info_v1 info = {};
    info.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    info.struct_size = sizeof(info);
    mcdose_particle_dmlc_egsnrc_source_context_v1 *context = nullptr;
    CHECK(mcdose_particle_dmlc_create_egsnrc_source_from_startup_v1(
              tracked_path,
              MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV,
              next_random, &random, &context, &info, diagnostic,
              diagnostic_capacity) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    auto wide_summary = summary();
    CHECK(mcdose_particle_dmlc_begin_egsnrc_source_particle_v1(
              context, &input, 0.0, 0, 0, &wide_summary, diagnostic,
              diagnostic_capacity) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(wide_summary.retained_product_count == 1);
    auto wide_output = source_result();
    CHECK(mcdose_particle_dmlc_next_egsnrc_source_entry_v1(
              context, &wide_output, diagnostic, diagnostic_capacity) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(wide_output.has_entry == 1);
    CHECK(wide_output.product_kind == MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY);
    CHECK(wide_output.entry.weight == input.weight);
    mcdose_particle_dmlc_destroy_egsnrc_source_context_v1(context);
    CHECK(random.next == 0);
    return 0;
}
}  // namespace

int main(int argc, char **argv) {
    static_assert(sizeof(mcdose_particle_dmlc_egsnrc_source_result_v1) == 144);
    CHECK(argc == 2 || argc == 4);
    char diagnostic[512] = {};
    random_state random;
    mcdose_particle_dmlc_startup_info_v1 info = {};
    info.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    info.struct_size = sizeof(info);
    mcdose_particle_dmlc_egsnrc_source_context_v1 *context = nullptr;
    CHECK(mcdose_particle_dmlc_create_egsnrc_source_from_startup_v1(
              argv[1], MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV,
              next_random, &random, &context, &info, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(context != nullptr);

    auto input = incident();
    auto produced = summary();
    CHECK(mcdose_particle_dmlc_begin_egsnrc_source_particle_v1(
              context, &input, 0.25, 24, 25, &produced, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(produced.retained_product_count == 2);
    CHECK(produced.random_draw_count == 5);
    CHECK(produced.source_segment_index == 0);
    CHECK(produced.interpolation_fraction == 0.25);
    CHECK(random.next == 5);

    auto undrained = summary();
    undrained.retained_product_count = 99;
    CHECK(mcdose_particle_dmlc_begin_egsnrc_source_particle_v1(
              context, &input, 0.5, 26, 27, &undrained, diagnostic,
              sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(std::strstr(diagnostic, "drained") != nullptr);
    CHECK(undrained.retained_product_count == 0);
    CHECK(random.next == 5);

    auto primary = source_result();
    CHECK(mcdose_particle_dmlc_next_egsnrc_source_entry_v1(
              context, &primary, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(primary.has_entry == 1);
    CHECK(primary.remaining_entry_count == 1);
    CHECK(primary.product_kind == MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY);
    CHECK(primary.entry.iq == MCDOSE_PARTICLE_DMLC_PHOTON);
    CHECK(primary.entry.region == input.region);
    CHECK(primary.entry.latch == input.latch);
    CHECK(primary.entry.history_id == input.history_id);
    CHECK(primary.entry.particle_id == input.particle_id);
    CHECK(primary.entry.parent_particle_id == input.parent_particle_id);
    CHECK(primary.entry.total_energy_mev == input.total_energy_mev);

    auto scattered = source_result();
    CHECK(mcdose_particle_dmlc_next_egsnrc_source_entry_v1(
              context, &scattered, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(scattered.has_entry == 1);
    CHECK(scattered.remaining_entry_count == 0);
    CHECK(scattered.product_kind ==
          MCDOSE_PARTICLE_DMLC_PRODUCER_SCATTERED_PHOTON);
    CHECK(scattered.entry.region == input.region);
    CHECK(scattered.entry.latch == input.latch);
    CHECK(scattered.entry.history_id == input.history_id);
    CHECK(scattered.entry.particle_id == 24);
    CHECK(scattered.entry.parent_particle_id == input.particle_id);

    auto empty = source_result();
    CHECK(mcdose_particle_dmlc_next_egsnrc_source_entry_v1(
              context, &empty, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(empty.has_entry == 0);
    CHECK(empty.remaining_entry_count == 0);
    CHECK(empty.product_kind == 0);

    auto invalid_electron = incident();
    invalid_electron.iq = MCDOSE_PARTICLE_DMLC_ELECTRON;
    invalid_electron.total_energy_mev =
        MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV;
    auto invalid_summary = summary();
    CHECK(mcdose_particle_dmlc_begin_egsnrc_source_particle_v1(
              context, &invalid_electron, 0.25, 26, 27, &invalid_summary,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(random.next == 5);

    auto electron = incident();
    electron.iq = MCDOSE_PARTICLE_DMLC_ELECTRON;
    electron.total_energy_mev =
        4.0 + MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV;
    electron.position_cm[0] = 0.0;
    auto electron_summary = summary();
    CHECK(mcdose_particle_dmlc_begin_egsnrc_source_particle_v1(
              context, &electron, 0.25, 26, 27, &electron_summary, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(electron_summary.retained_product_count == 1);
    CHECK(electron_summary.random_draw_count == 0);
    CHECK(random.next == 5);
    auto electron_product = source_result();
    CHECK(mcdose_particle_dmlc_next_egsnrc_source_entry_v1(
              context, &electron_product, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(electron_product.has_entry == 1);
    CHECK(electron_product.remaining_entry_count == 0);
    CHECK(electron_product.entry.iq == MCDOSE_PARTICLE_DMLC_ELECTRON);
    CHECK(electron_product.entry.total_energy_mev ==
          electron.total_energy_mev);
    CHECK(electron_product.entry.weight == electron.weight);

    mcdose_particle_dmlc_destroy_egsnrc_source_context_v1(context);
    if (argc == 4) {
        CHECK(test_jaw_behavior(argv[2], argv[3], diagnostic,
                                sizeof(diagnostic)) == 0);
    }
    return 0;
}
