#include "mcdose_particle_dmlc_egsnrc_source_v1.h"

#include "egs_config1.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <new>

static_assert(sizeof(EGS_I32) == sizeof(int32_t));
static_assert(sizeof(EGS_I64) == sizeof(uint64_t));
static_assert(sizeof(uintptr_t) <= sizeof(EGS_I64));

extern "C" void F77_OBJ_(
    mcdose_particle_dmlc_source_random_v1,
    MCDOSE_PARTICLE_DMLC_SOURCE_RANDOM_V1)(EGS_Float *uniform_random,
                                           EGS_I32 *status);

namespace {
struct mortran_source_context {
    mcdose_particle_dmlc_egsnrc_source_context_v1 *source = nullptr;
    uint64_t next_history_id = 1;
    uint64_t next_particle_id = 1;
};

int32_t next_random(void *, double *uniform_random) {
    if (uniform_random == nullptr) {
        return MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT;
    }
    EGS_Float value = 0;
    EGS_I32 status = MCDOSE_PARTICLE_DMLC_STATUS_OK;
    F77_OBJ_(mcdose_particle_dmlc_source_random_v1,
             MCDOSE_PARTICLE_DMLC_SOURCE_RANDOM_V1)(&value, &status);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return static_cast<int32_t>(status);
    }
    *uniform_random = static_cast<double>(value);
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

mortran_source_context *from_handle(EGS_I64 value) {
    return reinterpret_cast<mortran_source_context *>(
        static_cast<uintptr_t>(value));
}

EGS_I64 to_handle(mortran_source_context *value) {
    return static_cast<EGS_I64>(reinterpret_cast<uintptr_t>(value));
}

bool valid_egs_float(double value) {
    const auto converted = static_cast<EGS_Float>(value);
    return std::isfinite(value) && std::isfinite(converted);
}

bool ids_available(const mortran_source_context &context) {
    return context.next_history_id != std::numeric_limits<uint64_t>::max() &&
           context.next_particle_id <=
               std::numeric_limits<uint64_t>::max() - 2;
}

void report_failure(const char *operation, int32_t status,
                    const char *diagnostic) {
    std::fprintf(stderr, "mcdose particleDmlc %s failed with status %d: %s\n",
                 operation, status,
                 diagnostic != nullptr && diagnostic[0] != '\0'
                     ? diagnostic
                     : "no diagnostic was provided");
}
}  // namespace

extern "C" void F77_OBJ_(
    mcdose_particle_dmlc_source_init_v1,
    MCDOSE_PARTICLE_DMLC_SOURCE_INIT_V1)(
    EGS_I64 *source_handle, EGS_Float *electron_rest_mass_mev,
    EGS_I32 *beam_number, EGS_I32 *mlc_device_index, EGS_I32 *status) {
    if (status == nullptr) {
        return;
    }
    *status = MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT;
    if (source_handle == nullptr || electron_rest_mass_mev == nullptr ||
        beam_number == nullptr || mlc_device_index == nullptr) {
        return;
    }
    *source_handle = 0;
    *beam_number = 0;
    *mlc_device_index = 0;
    const char *path =
        std::getenv("MCDOSE_PARTICLE_DMLC_STARTUP_ARTIFACT");
    if (path == nullptr || path[0] == '\0') {
        *status = MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED;
        return;
    }
    std::unique_ptr<mortran_source_context> result;
    try {
        result = std::make_unique<mortran_source_context>();
    } catch (const std::bad_alloc &) {
        *status = MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED;
        return;
    }
    mcdose_particle_dmlc_startup_info_v1 info = {};
    info.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    info.struct_size = sizeof(info);
    const char *correction_path = std::getenv(
        "MCDOSE_PARTICLE_DMLC_OUTPUT_CORRECTION_ARTIFACT");
    char diagnostic[512];
    int32_t create_status = MCDOSE_PARTICLE_DMLC_STATUS_OK;
    if (correction_path != nullptr && correction_path[0] != '\0') {
        mcdose_particle_dmlc_source_output_correction_info_v1 correction_info = {};
        correction_info.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        correction_info.struct_size = sizeof(correction_info);
        create_status =
            mcdose_particle_dmlc_create_egsnrc_source_from_startup_with_output_correction_v1(
                path, correction_path,
                static_cast<double>(*electron_rest_mass_mev), next_random,
                nullptr, &result->source, &info, &correction_info, diagnostic,
                sizeof(diagnostic));
    } else {
        create_status = mcdose_particle_dmlc_create_egsnrc_source_from_startup_v1(
            path, static_cast<double>(*electron_rest_mass_mev), next_random,
            nullptr, &result->source, &info, diagnostic,
            sizeof(diagnostic));
    }
    if (create_status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        report_failure("source initialization", create_status, diagnostic);
        *status = static_cast<EGS_I32>(create_status);
        return;
    }
    if (info.mlc_device_index >
        static_cast<uint32_t>(std::numeric_limits<EGS_I32>::max())) {
        mcdose_particle_dmlc_destroy_egsnrc_source_context_v1(
            result->source);
        result->source = nullptr;
        *status = MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED;
        return;
    }
    *beam_number = static_cast<EGS_I32>(info.beam_number);
    *mlc_device_index = static_cast<EGS_I32>(info.mlc_device_index);
    *source_handle = to_handle(result.release());
    *status = MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" void F77_OBJ_(
    mcdose_particle_dmlc_source_sample_v1,
    MCDOSE_PARTICLE_DMLC_SOURCE_SAMPLE_V1)(
    EGS_I64 *source_handle, EGS_I32 *begin_new_particle, EGS_I32 *iq,
    EGS_Float *total_energy_mev, EGS_Float *x_cm, EGS_Float *y_cm,
    EGS_Float *z_cm, EGS_Float *u, EGS_Float *v, EGS_Float *w,
    EGS_Float *weight, EGS_I32 *latch, EGS_Float *fractional_meterset,
    EGS_I32 *product_kind, EGS_I32 *remaining_product_count,
    EGS_I32 *random_draw_count, EGS_I64 *history_id,
    EGS_I64 *particle_id, EGS_I64 *parent_particle_id, EGS_I32 *status) {
    if (status == nullptr) {
        return;
    }
    *status = MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT;
    if (source_handle == nullptr || begin_new_particle == nullptr ||
        iq == nullptr || total_energy_mev == nullptr || x_cm == nullptr ||
        y_cm == nullptr || z_cm == nullptr || u == nullptr || v == nullptr ||
        w == nullptr || weight == nullptr || latch == nullptr ||
        fractional_meterset == nullptr || product_kind == nullptr ||
        remaining_product_count == nullptr || random_draw_count == nullptr ||
        history_id == nullptr || particle_id == nullptr ||
        parent_particle_id == nullptr) {
        return;
    }
    *product_kind = 0;
    *remaining_product_count = 0;
    *random_draw_count = 0;
    *history_id = 0;
    *particle_id = 0;
    *parent_particle_id = 0;
    auto *context = from_handle(*source_handle);
    if (context == nullptr || context->source == nullptr ||
        (*begin_new_particle != 0 && *begin_new_particle != 1)) {
        *status = MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED;
        return;
    }
    if (*begin_new_particle == 1) {
        if (!ids_available(*context)) {
            *status = MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED;
            return;
        }
        mcdose_particle_dmlc_egsnrc_stack_entry_v1 incident = {};
        incident.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        incident.struct_size = sizeof(incident);
        incident.iq = static_cast<int32_t>(*iq);
        incident.region = 1;
        incident.latch = static_cast<int32_t>(*latch);
        incident.history_id = context->next_history_id;
        incident.particle_id = context->next_particle_id;
        incident.total_energy_mev = static_cast<double>(*total_energy_mev);
        incident.position_cm[0] = static_cast<double>(*x_cm);
        incident.position_cm[1] = static_cast<double>(*y_cm);
        incident.position_cm[2] = static_cast<double>(*z_cm);
        incident.direction[0] = static_cast<double>(*u);
        incident.direction[1] = static_cast<double>(*v);
        incident.direction[2] = static_cast<double>(*w);
        incident.weight = static_cast<double>(*weight);
        mcdose_particle_dmlc_producer_summary_v1 summary = {};
        summary.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        summary.struct_size = sizeof(summary);
        char diagnostic[512];
        const int32_t begin_status =
            mcdose_particle_dmlc_begin_egsnrc_source_particle_v1(
                context->source, &incident,
                static_cast<double>(*fractional_meterset),
                context->next_particle_id + 1,
                context->next_particle_id + 2, &summary, diagnostic,
                sizeof(diagnostic));
        *random_draw_count = static_cast<EGS_I32>(summary.random_draw_count);
        if (begin_status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            report_failure("source begin", begin_status, diagnostic);
            std::fprintf(
                stderr,
                "mcdose incident iq=%d energy=%g position=(%g,%g,%g) "
                "direction=(%g,%g,%g) weight=%g fMU=%g\n",
                static_cast<int>(*iq), static_cast<double>(*total_energy_mev),
                static_cast<double>(*x_cm), static_cast<double>(*y_cm),
                static_cast<double>(*z_cm), static_cast<double>(*u),
                static_cast<double>(*v), static_cast<double>(*w),
                static_cast<double>(*weight),
                static_cast<double>(*fractional_meterset));
            *status = static_cast<EGS_I32>(begin_status);
            return;
        }
        ++context->next_history_id;
        context->next_particle_id += 3;
    }

    mcdose_particle_dmlc_egsnrc_source_result_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    char diagnostic[512];
    const int32_t next_status =
        mcdose_particle_dmlc_next_egsnrc_source_entry_v1(
            context->source, &result, diagnostic, sizeof(diagnostic));
    if (next_status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        report_failure("source drain", next_status, diagnostic);
        *status = static_cast<EGS_I32>(next_status);
        return;
    }
    if (result.has_entry == 0) {
        *weight = 0;
        *status = MCDOSE_PARTICLE_DMLC_STATUS_OK;
        return;
    }
    const auto &entry = result.entry;
    if (!valid_egs_float(entry.position_cm[0]) ||
        !valid_egs_float(entry.position_cm[1]) ||
        !valid_egs_float(entry.position_cm[2]) ||
        !valid_egs_float(entry.direction[0]) ||
        !valid_egs_float(entry.direction[1]) ||
        !valid_egs_float(entry.direction[2]) ||
        !valid_egs_float(entry.total_energy_mev) ||
        !valid_egs_float(entry.weight) ||
        entry.history_id > static_cast<uint64_t>(
                               std::numeric_limits<EGS_I64>::max()) ||
        entry.particle_id > static_cast<uint64_t>(
                                std::numeric_limits<EGS_I64>::max()) ||
        entry.parent_particle_id > static_cast<uint64_t>(
                                       std::numeric_limits<EGS_I64>::max())) {
        *status = MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED;
        return;
    }
    *iq = static_cast<EGS_I32>(entry.iq);
    *total_energy_mev = static_cast<EGS_Float>(entry.total_energy_mev);
    *x_cm = static_cast<EGS_Float>(entry.position_cm[0]);
    *y_cm = static_cast<EGS_Float>(entry.position_cm[1]);
    *z_cm = static_cast<EGS_Float>(entry.position_cm[2]);
    *u = static_cast<EGS_Float>(entry.direction[0]);
    *v = static_cast<EGS_Float>(entry.direction[1]);
    *w = static_cast<EGS_Float>(entry.direction[2]);
    *weight = static_cast<EGS_Float>(entry.weight);
    *latch = static_cast<EGS_I32>(entry.latch);
    *product_kind = static_cast<EGS_I32>(result.product_kind);
    *remaining_product_count =
        static_cast<EGS_I32>(result.remaining_entry_count);
    *history_id = static_cast<EGS_I64>(entry.history_id);
    *particle_id = static_cast<EGS_I64>(entry.particle_id);
    *parent_particle_id = static_cast<EGS_I64>(entry.parent_particle_id);
    *status = MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" void F77_OBJ_(
    mcdose_particle_dmlc_source_finish_v1,
    MCDOSE_PARTICLE_DMLC_SOURCE_FINISH_V1)(EGS_I64 *source_handle,
                                           EGS_I32 *status) {
    if (status == nullptr) {
        return;
    }
    *status = MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT;
    if (source_handle == nullptr) {
        return;
    }
    auto *context = from_handle(*source_handle);
    if (context == nullptr) {
        *status = MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED;
        return;
    }
    mcdose_particle_dmlc_destroy_egsnrc_source_context_v1(context->source);
    context->source = nullptr;
    delete context;
    *source_handle = 0;
    *status = MCDOSE_PARTICLE_DMLC_STATUS_OK;
}
