#include "mcdose_particle_dmlc_source_output_correction_v1.h"

#include <algorithm>
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
bool close(double first, double second) {
    return std::fabs(first - second) <= 1.0e-13 *
           std::max({1.0, std::fabs(first), std::fabs(second)});
}

mcdose_particle_dmlc_source_output_correction_config_v1 config() {
    mcdose_particle_dmlc_source_output_correction_config_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.model =
        MCDOSE_PARTICLE_DMLC_SOURCE_OUTPUT_CORRECTION_LEGACY_MCV_FIELD_SIZE;
    result.x2_mapping =
        MCDOSE_PARTICLE_DMLC_SOURCE_OUTPUT_CORRECTION_LEGACY_MCV_Y2;
    result.x_jaw_opening_offset = 0;
    result.y_jaw_opening_offset = 1;
    result.total_pair_count = 4;
    result.universal_baseline = 0.9808;
    result.universal_amplitude = 0.0431;
    result.universal_exponential_rate_per_cm = 0.1324;
    result.r_y_intercept_percent = 1.54;
    result.r_y_linear_percent_per_cm = -8.45e-2;
    result.r_y_cubic_percent_per_cm3 = 4.47e-5;
    result.r_x_intercept_percent = 0.40;
    result.r_x_linear_percent_per_cm = -1.87e-2;
    result.p_y_linear_per_cm = 3.95e-2;
    result.p_y_cubic_per_cm3 = -3.55e-5;
    for (double &value : result.reference_jaws_cm_y1_y2_x1_x2) {
        value = 5.0;
    }
    return result;
}

mcdose_particle_dmlc_source_output_correction_result_v1 output() {
    mcdose_particle_dmlc_source_output_correction_result_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}
}  // namespace

int main(int argc, char **argv) {
    char diagnostic[512] = {};
    auto value = config();
    const double bank_1[4] = {-5.5, -5.5, -20.0, -20.0};
    const double bank_2[4] = {5.5, 5.5, 20.0, 20.0};
    auto result = output();
    CHECK(mcdose_particle_dmlc_evaluate_source_output_correction_v1(
              &value, bank_1, bank_2, 4, &result, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(close(result.universal_field_factor, 1.0138545178400526));
    CHECK(close(result.ion_chamber_backscatter_factor, 1.0006472838182905));
    CHECK(close(result.product, 1.0145107694635511));
    CHECK(mcdose_particle_dmlc_evaluate_source_output_correction_v1(
              &value, bank_1, bank_2, 5, &result, diagnostic,
              sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    if (argc == 2) {
        mcdose_particle_dmlc_source_output_correction_config_v1 loaded = {};
        loaded.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        loaded.struct_size = sizeof(loaded);
        mcdose_particle_dmlc_source_output_correction_info_v1 info = {};
        info.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        info.struct_size = sizeof(info);
        CHECK(mcdose_particle_dmlc_load_source_output_correction_v1(
                  argv[1], &loaded, &info, diagnostic,
                  sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
        CHECK(std::strlen(info.startup_payload_sha256) == 64);
        CHECK(std::strlen(info.correction_payload_sha256) == 64);
        auto loaded_result = output();
        CHECK(mcdose_particle_dmlc_evaluate_source_output_correction_v1(
                  &loaded, bank_1, bank_2, 4, &loaded_result, diagnostic,
                  sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
        CHECK(close(loaded_result.product, result.product));
    } else {
        CHECK(argc == 1);
    }
    return 0;
}
