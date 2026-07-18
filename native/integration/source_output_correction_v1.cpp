#include "mcdose_particle_dmlc_source_output_correction_v1.h"

#include "sha256_v1.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <limits>
#include <string>

namespace {
constexpr size_t payload_size = 192;
constexpr size_t digest_size = 32;
constexpr size_t artifact_size = payload_size + digest_size;
constexpr std::array<uint8_t, 8> artifact_magic = {
    'M', 'C', 'D', 'S', 'O', 'C', '0', '1'};

int32_t fail(int32_t status, char *diagnostic, size_t capacity,
             const char *message) {
    if (diagnostic != nullptr && capacity > 0) {
        std::snprintf(diagnostic, capacity, "%s", message);
    }
    return status;
}

void clear_diagnostic(char *diagnostic, size_t capacity) {
    if (diagnostic != nullptr && capacity > 0) {
        diagnostic[0] = '\0';
    }
}

uint32_t decode_u32(const uint8_t *bytes) {
    return static_cast<uint32_t>(bytes[0]) |
           (static_cast<uint32_t>(bytes[1]) << 8) |
           (static_cast<uint32_t>(bytes[2]) << 16) |
           (static_cast<uint32_t>(bytes[3]) << 24);
}

uint64_t decode_u64(const uint8_t *bytes) {
    uint64_t result = 0;
    for (size_t index = 0; index < 8; ++index) {
        result |= static_cast<uint64_t>(bytes[index]) << (index * 8);
    }
    return result;
}

double decode_double(const uint8_t *bytes) {
    const uint64_t bits = decode_u64(bytes);
    double result = 0.0;
    static_assert(sizeof(bits) == sizeof(result));
    static_assert(std::numeric_limits<double>::is_iec559);
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

std::string hexadecimal(const uint8_t *bytes, size_t count) {
    static constexpr char digits[] = "0123456789abcdef";
    std::string result(count * 2, '0');
    for (size_t index = 0; index < count; ++index) {
        result[index * 2] = digits[bytes[index] >> 4];
        result[index * 2 + 1] = digits[bytes[index] & 0x0f];
    }
    return result;
}

bool zero_reserved(const uint32_t *values, size_t count) {
    for (size_t index = 0; index < count; ++index) {
        if (values[index] != 0) {
            return false;
        }
    }
    return true;
}

bool valid_config(
    const mcdose_particle_dmlc_source_output_correction_config_v1 *config) {
    if (config->model !=
            MCDOSE_PARTICLE_DMLC_SOURCE_OUTPUT_CORRECTION_LEGACY_MCV_FIELD_SIZE ||
        (config->x2_mapping !=
             MCDOSE_PARTICLE_DMLC_SOURCE_OUTPUT_CORRECTION_DICOM_X2 &&
         config->x2_mapping !=
             MCDOSE_PARTICLE_DMLC_SOURCE_OUTPUT_CORRECTION_LEGACY_MCV_Y2) ||
        config->total_pair_count == 0 ||
        config->x_jaw_opening_offset >= config->total_pair_count ||
        config->y_jaw_opening_offset >= config->total_pair_count ||
        config->x_jaw_opening_offset == config->y_jaw_opening_offset ||
        !zero_reserved(config->reserved, 4)) {
        return false;
    }
    const std::array<double, 14> values = {
        config->universal_baseline,
        config->universal_amplitude,
        config->universal_exponential_rate_per_cm,
        config->r_y_intercept_percent,
        config->r_y_linear_percent_per_cm,
        config->r_y_cubic_percent_per_cm3,
        config->r_x_intercept_percent,
        config->r_x_linear_percent_per_cm,
        config->p_y_linear_per_cm,
        config->p_y_cubic_per_cm3,
        config->reference_jaws_cm_y1_y2_x1_x2[0],
        config->reference_jaws_cm_y1_y2_x1_x2[1],
        config->reference_jaws_cm_y1_y2_x1_x2[2],
        config->reference_jaws_cm_y1_y2_x1_x2[3]};
    return config->universal_exponential_rate_per_cm >= 0.0 &&
           std::all_of(values.begin(), values.end(), [](double value) {
               return std::isfinite(value);
           });
}

std::array<double, 3> one_jaw(
    const mcdose_particle_dmlc_source_output_correction_config_v1 *config,
    double y_cm, double x_cm) {
    return {
        config->r_y_intercept_percent +
            config->r_y_linear_percent_per_cm * y_cm +
            config->r_y_cubic_percent_per_cm3 * y_cm * y_cm * y_cm,
        config->r_x_intercept_percent +
            config->r_x_linear_percent_per_cm * x_cm,
        config->p_y_linear_per_cm * y_cm +
            config->p_y_cubic_per_cm3 * y_cm * y_cm * y_cm};
}

double relative_backscatter(
    const mcdose_particle_dmlc_source_output_correction_config_v1 *config,
    double y1_cm, double y2_cm, double x1_cm, double x2_cm) {
    const auto first = one_jaw(config, y1_cm, x1_cm);
    const auto second = one_jaw(config, y2_cm, x2_cm);
    const double r_y = first[0] + second[0];
    const double r_x_at_y20 = first[1] + second[1];
    const double p_y = first[2] + second[2];
    return r_y + r_x_at_y20 * p_y;
}
}  // namespace

extern "C" int32_t mcdose_particle_dmlc_load_source_output_correction_v1(
    const char *path,
    mcdose_particle_dmlc_source_output_correction_config_v1 *config,
    mcdose_particle_dmlc_source_output_correction_info_v1 *info,
    char *diagnostic, size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (path == nullptr || config == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "source-output correction loader contains a null pointer");
    }
    if (config->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        config->struct_size < sizeof(*config) ||
        (info != nullptr &&
         (info->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
          info->struct_size < sizeof(*info)))) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "source-output correction ABI version or size differs");
    }
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream || stream.tellg() != static_cast<std::streamoff>(artifact_size)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "source-output correction file size differs");
    }
    stream.seekg(0);
    std::array<uint8_t, artifact_size> bytes = {};
    stream.read(reinterpret_cast<char *>(bytes.data()), bytes.size());
    if (!stream || stream.gcount() != static_cast<std::streamsize>(bytes.size())) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "source-output correction file read failed");
    }
    if (!std::equal(artifact_magic.begin(), artifact_magic.end(), bytes.begin()) ||
        decode_u32(bytes.data() + 8) != 1 || decode_u32(bytes.data() + 20) != 0) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "source-output correction artifact identity differs");
    }
    mcdose_particle_dmlc_internal::sha256 digest;
    static constexpr char prefix[] =
        "mcdose.particle-dmlc-source-output-correction-payload/v1";
    digest.update(prefix, sizeof(prefix));
    digest.update(bytes.data(), payload_size);
    const std::string actual_digest = digest.finish();
    if (actual_digest != hexadecimal(bytes.data() + payload_size, digest_size)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "source-output correction payload SHA-256 differs");
    }

    mcdose_particle_dmlc_source_output_correction_config_v1 loaded = {};
    loaded.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    loaded.struct_size = sizeof(loaded);
    loaded.model = decode_u32(bytes.data() + 12);
    loaded.x2_mapping = decode_u32(bytes.data() + 16);
    loaded.x_jaw_opening_offset = decode_u64(bytes.data() + 24);
    loaded.y_jaw_opening_offset = decode_u64(bytes.data() + 32);
    loaded.total_pair_count = decode_u64(bytes.data() + 40);
    std::array<double, 14> coefficients = {};
    for (size_t index = 0; index < 14; ++index) {
        coefficients[index] = decode_double(bytes.data() + 80 + index * 8);
    }
    loaded.universal_baseline = coefficients[0];
    loaded.universal_amplitude = coefficients[1];
    loaded.universal_exponential_rate_per_cm = coefficients[2];
    loaded.r_y_intercept_percent = coefficients[3];
    loaded.r_y_linear_percent_per_cm = coefficients[4];
    loaded.r_y_cubic_percent_per_cm3 = coefficients[5];
    loaded.r_x_intercept_percent = coefficients[6];
    loaded.r_x_linear_percent_per_cm = coefficients[7];
    loaded.p_y_linear_per_cm = coefficients[8];
    loaded.p_y_cubic_per_cm3 = coefficients[9];
    std::copy(coefficients.begin() + 10, coefficients.end(),
              loaded.reference_jaws_cm_y1_y2_x1_x2);
    if (!valid_config(&loaded)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "source-output correction model fields are invalid");
    }
    *config = loaded;
    if (info != nullptr) {
        mcdose_particle_dmlc_source_output_correction_info_v1 loaded_info = {};
        loaded_info.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        loaded_info.struct_size = sizeof(loaded_info);
        const std::string startup_digest = hexadecimal(bytes.data() + 48, digest_size);
        std::memcpy(loaded_info.startup_payload_sha256,
                    startup_digest.c_str(), startup_digest.size() + 1);
        std::memcpy(loaded_info.correction_payload_sha256,
                    actual_digest.c_str(), actual_digest.size() + 1);
        *info = loaded_info;
    }
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_evaluate_source_output_correction_v1(
    const mcdose_particle_dmlc_source_output_correction_config_v1 *config,
    const double *bank_1_positions_cm, const double *bank_2_positions_cm,
    uint64_t position_count,
    mcdose_particle_dmlc_source_output_correction_result_v1 *result,
    char *diagnostic, size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (config == nullptr || bank_1_positions_cm == nullptr ||
        bank_2_positions_cm == nullptr || result == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "source-output correction evaluation contains a null pointer");
    }
    if (config->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        config->struct_size < sizeof(*config) ||
        result->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        result->struct_size < sizeof(*result)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "source-output correction evaluation ABI differs");
    }
    if (!valid_config(config) || position_count != config->total_pair_count) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "source-output correction configuration or opening count differs");
    }
    const uint64_t x = config->x_jaw_opening_offset;
    const uint64_t y = config->y_jaw_opening_offset;
    const double x1_cm = -bank_1_positions_cm[x];
    const double x2_cm = bank_2_positions_cm[x];
    const double y1_cm = -bank_1_positions_cm[y];
    const double y2_cm = bank_2_positions_cm[y];
    if (!std::isfinite(x1_cm) || !std::isfinite(x2_cm) ||
        !std::isfinite(y1_cm) || !std::isfinite(y2_cm)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "source-output correction sampled jaws are not finite");
    }
    const double universal_first =
        config->universal_baseline +
        config->universal_amplitude *
            (1.0 - std::exp(-config->universal_exponential_rate_per_cm *
                            2.0 * y2_cm));
    const double universal_second =
        config->universal_baseline +
        config->universal_amplitude *
            (1.0 - std::exp(-config->universal_exponential_rate_per_cm *
                            2.0 * y1_cm));
    const double universal = (universal_first + universal_second) / 2.0;
    const double model_x2 =
        config->x2_mapping ==
                MCDOSE_PARTICLE_DMLC_SOURCE_OUTPUT_CORRECTION_LEGACY_MCV_Y2
            ? y2_cm
            : x2_cm;
    const double current =
        relative_backscatter(config, y1_cm, y2_cm, x1_cm, model_x2);
    const double *reference = config->reference_jaws_cm_y1_y2_x1_x2;
    const double reference_value = relative_backscatter(
        config, reference[0], reference[1], reference[2], reference[3]);
    const double backscatter =
        (1.0 + reference_value / 100.0) / (1.0 + current / 100.0);
    const double product = universal * backscatter;
    if (!std::isfinite(universal) || universal <= 0.0 ||
        !std::isfinite(backscatter) || backscatter <= 0.0 ||
        !std::isfinite(product) || product <= 0.0) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "source-output correction produced an invalid factor");
    }
    const uint32_t abi_version = result->abi_version;
    const uint32_t struct_size = result->struct_size;
    *result = {};
    result->abi_version = abi_version;
    result->struct_size = struct_size;
    result->universal_field_factor = universal;
    result->ion_chamber_backscatter_factor = backscatter;
    result->product = product;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_apply_source_output_correction_v1(
    void *user_data,
    const mcdose_particle_dmlc_sampled_state_v1 *sampled_state,
    const double *bank_1_positions_cm, const double *bank_2_positions_cm,
    uint64_t position_count, double *weight_factor, char *diagnostic,
    size_t diagnostic_capacity) {
    if (user_data == nullptr || sampled_state == nullptr || weight_factor == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "source-output correction callback contains a null pointer");
    }
    if (sampled_state->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        sampled_state->struct_size < sizeof(*sampled_state)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "source-output correction sampled-state ABI differs");
    }
    mcdose_particle_dmlc_source_output_correction_result_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    const int32_t status =
        mcdose_particle_dmlc_evaluate_source_output_correction_v1(
            static_cast<const mcdose_particle_dmlc_source_output_correction_config_v1 *>(
                user_data),
            bank_1_positions_cm, bank_2_positions_cm, position_count, &result,
            diagnostic, diagnostic_capacity);
    if (status == MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        *weight_factor = result.product;
    }
    return status;
}
