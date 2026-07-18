#include "mcdose_particle_dmlc_egsnrc_source_v1.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <new>

struct mcdose_particle_dmlc_egsnrc_source_context_v1 {
    mcdose_particle_dmlc_producer_context_v1 *producer = nullptr;
    mcdose_particle_dmlc_source_output_correction_config_v1 output_correction = {};
    double electron_rest_mass_mev = 0.0;
    int32_t region = 0;
    int32_t latch = 0;
    bool queue_active = false;
};

namespace {
constexpr double minimum_supported_rest_mass_mev = 0.5;
constexpr double maximum_supported_rest_mass_mev = 0.52;

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

void reset_result(mcdose_particle_dmlc_egsnrc_source_result_v1 *result) {
    const uint32_t abi_version = result->abi_version;
    const uint32_t struct_size = result->struct_size;
    *result = {};
    result->abi_version = abi_version;
    result->struct_size = struct_size;
}

void reset_summary(mcdose_particle_dmlc_producer_summary_v1 *summary) {
    const uint32_t abi_version = summary->abi_version;
    const uint32_t struct_size = summary->struct_size;
    *summary = {};
    summary->abi_version = abi_version;
    summary->struct_size = struct_size;
}

bool valid_rest_mass(double value) {
    return std::isfinite(value) && value > minimum_supported_rest_mass_mev &&
           value < maximum_supported_rest_mass_mev;
}

int32_t create_source(
    const char *startup_path, const char *output_correction_path,
    double electron_rest_mass_mev,
    mcdose_particle_dmlc_host_random_callback_v1 random_callback,
    void *random_user_data,
    mcdose_particle_dmlc_egsnrc_source_context_v1 **context,
    mcdose_particle_dmlc_startup_info_v1 *startup_info,
    mcdose_particle_dmlc_source_output_correction_info_v1 *correction_info,
    char *diagnostic, size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    const bool has_correction =
        output_correction_path != nullptr && output_correction_path[0] != '\0';
    if (startup_path == nullptr || random_callback == nullptr ||
        context == nullptr || (correction_info != nullptr && !has_correction)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc source creation contains an invalid required argument");
    }
    *context = nullptr;
    if (!valid_rest_mass(electron_rest_mass_mev)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc source electron rest mass is invalid");
    }
    if ((startup_info != nullptr &&
         (startup_info->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
          startup_info->struct_size < sizeof(*startup_info))) ||
        (correction_info != nullptr &&
         (correction_info->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
          correction_info->struct_size < sizeof(*correction_info)))) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc source information ABI version or size differs");
    }
    std::unique_ptr<mcdose_particle_dmlc_egsnrc_source_context_v1> result;
    try {
        result =
            std::make_unique<mcdose_particle_dmlc_egsnrc_source_context_v1>();
    } catch (const std::bad_alloc &) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc source context allocation failed");
    }
    mcdose_particle_dmlc_startup_info_v1 loaded_startup = {};
    loaded_startup.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    loaded_startup.struct_size = sizeof(loaded_startup);
    int32_t status = mcdose_particle_dmlc_create_producer_from_startup_v1(
        startup_path, &result->producer, &loaded_startup, diagnostic,
        diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return status;
    }
    mcdose_particle_dmlc_source_output_correction_info_v1 loaded_correction = {};
    if (has_correction) {
        result->output_correction.abi_version =
            MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        result->output_correction.struct_size = sizeof(result->output_correction);
        loaded_correction.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        loaded_correction.struct_size = sizeof(loaded_correction);
        status = mcdose_particle_dmlc_load_source_output_correction_v1(
            output_correction_path, &result->output_correction,
            &loaded_correction, diagnostic, diagnostic_capacity);
        if (status == MCDOSE_PARTICLE_DMLC_STATUS_OK &&
            std::strcmp(loaded_correction.startup_payload_sha256,
                        loaded_startup.payload_sha256) != 0) {
            status = fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                          diagnostic, diagnostic_capacity,
                          "source-output correction startup payload differs");
        }
        if (status == MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            status =
                mcdose_particle_dmlc_set_producer_sampled_weight_callback_v1(
                    result->producer,
                    mcdose_particle_dmlc_apply_source_output_correction_v1,
                    &result->output_correction, diagnostic,
                    diagnostic_capacity);
        }
        if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            mcdose_particle_dmlc_destroy_producer_context_v1(result->producer);
            result->producer = nullptr;
            return status;
        }
    }
    status = mcdose_particle_dmlc_set_producer_random_source_v1(
        result->producer, random_callback, random_user_data, diagnostic,
        diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        mcdose_particle_dmlc_destroy_producer_context_v1(result->producer);
        result->producer = nullptr;
        return status;
    }
    result->electron_rest_mass_mev = electron_rest_mass_mev;
    if (startup_info != nullptr) {
        *startup_info = loaded_startup;
    }
    if (correction_info != nullptr) {
        *correction_info = loaded_correction;
    }
    *context = result.release();
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}
}  // namespace

extern "C" int32_t
mcdose_particle_dmlc_create_egsnrc_source_from_startup_v1(
    const char *path, double electron_rest_mass_mev,
    mcdose_particle_dmlc_host_random_callback_v1 random_callback,
    void *random_user_data,
    mcdose_particle_dmlc_egsnrc_source_context_v1 **context,
    mcdose_particle_dmlc_startup_info_v1 *startup_info, char *diagnostic,
    size_t diagnostic_capacity) {
    return create_source(path, nullptr, electron_rest_mass_mev, random_callback,
                         random_user_data, context, startup_info, nullptr,
                         diagnostic, diagnostic_capacity);
}

extern "C" int32_t
mcdose_particle_dmlc_create_egsnrc_source_from_startup_with_output_correction_v1(
    const char *startup_path, const char *output_correction_path,
    double electron_rest_mass_mev,
    mcdose_particle_dmlc_host_random_callback_v1 random_callback,
    void *random_user_data,
    mcdose_particle_dmlc_egsnrc_source_context_v1 **context,
    mcdose_particle_dmlc_startup_info_v1 *startup_info,
    mcdose_particle_dmlc_source_output_correction_info_v1 *correction_info,
    char *diagnostic, size_t diagnostic_capacity) {
    if (output_correction_path == nullptr || output_correction_path[0] == '\0') {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "corrected EGSnrc source requires an output-correction path");
    }
    return create_source(startup_path, output_correction_path,
                         electron_rest_mass_mev, random_callback,
                         random_user_data, context, startup_info,
                         correction_info, diagnostic, diagnostic_capacity);
}

extern "C" void mcdose_particle_dmlc_destroy_egsnrc_source_context_v1(
    mcdose_particle_dmlc_egsnrc_source_context_v1 *context) {
    if (context == nullptr) {
        return;
    }
    mcdose_particle_dmlc_destroy_producer_context_v1(context->producer);
    context->producer = nullptr;
    delete context;
}

extern "C" int32_t mcdose_particle_dmlc_begin_egsnrc_source_particle_v1(
    mcdose_particle_dmlc_egsnrc_source_context_v1 *context,
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 *incident_entry,
    double fractional_meterset, uint64_t scattered_photon_particle_id,
    uint64_t electron_particle_id,
    mcdose_particle_dmlc_producer_summary_v1 *summary, char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr || incident_entry == nullptr || summary == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc source begin contains a null pointer");
    }
    if (summary->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        summary->struct_size < sizeof(*summary)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc source summary ABI version or size differs");
    }
    reset_summary(summary);
    if (context->queue_active) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc source product queue must be drained first");
    }
    mcdose_particle_dmlc_particle_v1 particle = {};
    particle.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    particle.struct_size = sizeof(particle);
    int32_t status =
        mcdose_particle_dmlc_egsnrc_stack_entry_to_particle_v1(
            incident_entry, context->electron_rest_mass_mev, &particle,
            diagnostic, diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return status;
    }
    status = mcdose_particle_dmlc_produce_v1(
        context->producer, &particle, fractional_meterset,
        scattered_photon_particle_id, electron_particle_id, summary,
        diagnostic, diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return status;
    }
    context->region = incident_entry->region;
    context->latch = incident_entry->latch;
    context->queue_active = summary->retained_product_count != 0;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_next_egsnrc_source_entry_v1(
    mcdose_particle_dmlc_egsnrc_source_context_v1 *context,
    mcdose_particle_dmlc_egsnrc_source_result_v1 *result, char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr || result == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc source drain contains a null pointer");
    }
    if (result->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        result->struct_size < sizeof(*result)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc source result ABI version or size differs");
    }
    reset_result(result);
    mcdose_particle_dmlc_producer_product_v1 product = {};
    product.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    product.struct_size = sizeof(product);
    int32_t status = mcdose_particle_dmlc_next_producer_product_v1(
        context->producer, &product, diagnostic, diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return status;
    }
    if (product.has_product == 0) {
        if (context->queue_active) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                        diagnostic, diagnostic_capacity,
                        "EGSnrc source producer queue ended unexpectedly");
        }
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }
    result->entry.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result->entry.struct_size = sizeof(result->entry);
    status = mcdose_particle_dmlc_particle_to_egsnrc_stack_entry_v1(
        &product.particle, context->region, context->latch,
        context->electron_rest_mass_mev, &result->entry, diagnostic,
        diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        context->queue_active = product.remaining_product_count != 0;
        return status;
    }
    result->has_entry = 1;
    result->remaining_entry_count = product.remaining_product_count;
    result->product_kind = product.product_kind;
    context->queue_active = product.remaining_product_count != 0;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}
