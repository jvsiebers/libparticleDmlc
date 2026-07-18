#include "mcdose_particle_dmlc_egsnrc_source_v1.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::fprintf(stderr, "check failed: %s at %s:%d\n", #condition,  \
                         __FILE__, __LINE__);                                  \
            return 1;                                                          \
        }                                                                      \
    } while (false)

namespace {
int32_t next_random(void *, double *value) {
    if (value == nullptr) {
        return MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT;
    }
    *value = 0.5;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

mcdose_particle_dmlc_egsnrc_stack_entry_v1 electron() {
    mcdose_particle_dmlc_egsnrc_stack_entry_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.iq = MCDOSE_PARTICLE_DMLC_ELECTRON;
    result.region = 3;
    result.latch = 42;
    result.history_id = 17;
    result.particle_id = 23;
    result.total_energy_mev =
        4.0 + MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV;
    result.position_cm[0] = 0.5;
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
}  // namespace

int main(int argc, char **argv) {
    CHECK(argc == 3);
    char diagnostic[512] = {};
    mcdose_particle_dmlc_egsnrc_source_context_v1 *plain = nullptr;
    mcdose_particle_dmlc_egsnrc_source_context_v1 *corrected = nullptr;
    mcdose_particle_dmlc_startup_info_v1 plain_info = {};
    plain_info.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    plain_info.struct_size = sizeof(plain_info);
    mcdose_particle_dmlc_startup_info_v1 corrected_info = plain_info;
    mcdose_particle_dmlc_source_output_correction_info_v1 correction_info = {};
    correction_info.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    correction_info.struct_size = sizeof(correction_info);
    mcdose_particle_dmlc_egsnrc_source_context_v1 *rejected = nullptr;
    CHECK(mcdose_particle_dmlc_create_egsnrc_source_from_startup_with_output_correction_v1(
              argv[1], "",
              MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV,
              next_random, nullptr, &rejected, nullptr, nullptr, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT);
    CHECK(rejected == nullptr);
    CHECK(mcdose_particle_dmlc_create_egsnrc_source_from_startup_v1(
              argv[1], MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV,
              next_random, nullptr, &plain, &plain_info, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(mcdose_particle_dmlc_create_egsnrc_source_from_startup_with_output_correction_v1(
              argv[1], argv[2],
              MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV,
              next_random, nullptr, &corrected, &corrected_info,
              &correction_info, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(std::strcmp(plain_info.payload_sha256,
                      correction_info.startup_payload_sha256) == 0);

    const auto input = electron();
    auto plain_summary = summary();
    auto corrected_summary = summary();
    CHECK(mcdose_particle_dmlc_begin_egsnrc_source_particle_v1(
              plain, &input, 0.5, 24, 25, &plain_summary, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(mcdose_particle_dmlc_begin_egsnrc_source_particle_v1(
              corrected, &input, 0.5, 24, 25, &corrected_summary, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    auto plain_product = source_result();
    auto corrected_product = source_result();
    CHECK(mcdose_particle_dmlc_next_egsnrc_source_entry_v1(
              plain, &plain_product, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(mcdose_particle_dmlc_next_egsnrc_source_entry_v1(
              corrected, &corrected_product, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(plain_product.has_entry == 1 && corrected_product.has_entry == 1);
    CHECK(plain_product.entry.iq == corrected_product.entry.iq);
    CHECK(plain_product.entry.total_energy_mev ==
          corrected_product.entry.total_energy_mev);
    CHECK(std::fabs(corrected_product.entry.weight /
                        plain_product.entry.weight -
                    1.0145107694635511) < 1.0e-13);
    CHECK(plain_summary.random_draw_count == 0);
    CHECK(corrected_summary.random_draw_count == 0);

    mcdose_particle_dmlc_destroy_egsnrc_source_context_v1(plain);
    mcdose_particle_dmlc_destroy_egsnrc_source_context_v1(corrected);
    return 0;
}
