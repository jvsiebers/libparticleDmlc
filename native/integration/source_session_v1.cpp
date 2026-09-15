#include "mcdose_particle_dmlc_source_session_v1.h"

#include <cmath>
#include <cstdio>
#include <memory>
#include <new>

struct mcdose_particle_dmlc_source_session_context_v1 {
    mcdose_particle_dmlc_producer_context_v1 *producer = nullptr;
    mcdose_particle_dmlc_next_incident_callback_v1 next_incident = nullptr;
    void *incident_user_data = nullptr;
    mcdose_particle_dmlc_source_incident_v1 incident = {};
    bool queue_active = false;
};

namespace {
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

void reset_result(mcdose_particle_dmlc_source_session_result_v1 *result) {
    const uint32_t abi_version = result->abi_version;
    const uint32_t struct_size = result->struct_size;
    *result = {};
    result->abi_version = abi_version;
    result->struct_size = struct_size;
}

bool valid_particle(const mcdose_particle_dmlc_particle_v1 &particle) {
    if (particle.abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        particle.struct_size < sizeof(particle) ||
        (particle.charge != MCDOSE_PARTICLE_DMLC_ELECTRON &&
         particle.charge != MCDOSE_PARTICLE_DMLC_PHOTON &&
         particle.charge != MCDOSE_PARTICLE_DMLC_POSITRON) ||
        !std::isfinite(particle.energy_mev) || particle.energy_mev <= 0.0 ||
        !std::isfinite(particle.weight) || particle.weight <= 0.0) {
        return false;
    }
    double direction_norm_squared = 0.0;
    for (size_t index = 0; index < 3; ++index) {
        if (!std::isfinite(particle.position_cm[index]) ||
            !std::isfinite(particle.direction[index])) {
            return false;
        }
        direction_norm_squared += particle.direction[index] * particle.direction[index];
    }
    return std::isfinite(direction_norm_squared) && direction_norm_squared > 0.0;
}

int32_t validate_incident(const mcdose_particle_dmlc_source_incident_v1 &incident,
                          char *diagnostic, size_t diagnostic_capacity) {
    if (incident.abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        incident.struct_size < sizeof(incident)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "source incident ABI version or size differs");
    }
    if (incident.has_incident > 1 || incident.starts_new_history > 1) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "source incident flags must be zero or one");
    }
    if (incident.has_incident == 0) {
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }
    if (!std::isfinite(incident.fractional_meterset) ||
        incident.fractional_meterset < 0.0 ||
        incident.fractional_meterset > 1.0 || !valid_particle(incident.particle)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "source incident contains invalid particle or meterset data");
    }
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

void copy_product(const mcdose_particle_dmlc_source_session_context_v1 *context,
                  int32_t product_kind,
                  const mcdose_particle_dmlc_particle_v1 &particle,
                  uint32_t remaining_product_count,
                  mcdose_particle_dmlc_source_session_result_v1 *result) {
    result->has_product = 1;
    result->remaining_product_count = remaining_product_count;
    result->product_kind = product_kind;
    result->starts_new_history = context->incident.starts_new_history;
    result->fractional_meterset = context->incident.fractional_meterset;
    result->source_history_id = context->incident.particle.history_id;
    result->particle = particle;
    result->particle.history_id = result->source_history_id;
}
}  // namespace

extern "C" int32_t mcdose_particle_dmlc_create_source_session_v1(
    const mcdose_particle_dmlc_source_session_config_v1 *config,
    mcdose_particle_dmlc_next_incident_callback_v1 next_incident,
    void *incident_user_data,
    mcdose_particle_dmlc_source_session_context_v1 **context,
    char *diagnostic, size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (config == nullptr || next_incident == nullptr || context == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "source session creation contains an invalid required argument");
    }
    *context = nullptr;
    if (config->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        config->struct_size < sizeof(*config)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "source session configuration ABI version or size differs");
    }
    try {
        auto result = std::make_unique<mcdose_particle_dmlc_source_session_context_v1>();
        result->producer = config->producer;
        result->next_incident = next_incident;
        result->incident_user_data = incident_user_data;
        *context = result.release();
    } catch (const std::bad_alloc &) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity, "source session context allocation failed");
    }
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" void mcdose_particle_dmlc_destroy_source_session_context_v1(
    mcdose_particle_dmlc_source_session_context_v1 *context) {
    delete context;
}

extern "C" int32_t mcdose_particle_dmlc_next_source_product_v1(
    mcdose_particle_dmlc_source_session_context_v1 *context,
    mcdose_particle_dmlc_source_session_result_v1 *result, char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr || result == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "source session drain contains a null pointer");
    }
    if (result->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        result->struct_size < sizeof(*result)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "source session result ABI version or size differs");
    }
    reset_result(result);
    for (;;) {
        if (context->queue_active) {
            mcdose_particle_dmlc_producer_product_v1 product = {};
            product.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
            product.struct_size = sizeof(product);
            const int32_t status = mcdose_particle_dmlc_next_producer_product_v1(
                context->producer, &product, diagnostic, diagnostic_capacity);
            if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
                return status;
            }
            if (product.has_product == 0) {
                return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                            diagnostic, diagnostic_capacity,
                            "source session producer queue ended unexpectedly");
            }
            context->queue_active = product.remaining_product_count != 0;
            copy_product(context, product.product_kind, product.particle,
                         product.remaining_product_count, result);
            return MCDOSE_PARTICLE_DMLC_STATUS_OK;
        }

        mcdose_particle_dmlc_source_incident_v1 incident = {};
        incident.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        incident.struct_size = sizeof(incident);
        const int32_t source_status = context->next_incident(
            context->incident_user_data, &incident, diagnostic, diagnostic_capacity);
        if (source_status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            return source_status;
        }
        const int32_t validation_status =
            validate_incident(incident, diagnostic, diagnostic_capacity);
        if (validation_status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            return validation_status;
        }
        if (incident.has_incident == 0) {
            return MCDOSE_PARTICLE_DMLC_STATUS_OK;
        }
        context->incident = incident;
        if (context->producer == nullptr) {
            copy_product(context, MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY,
                         incident.particle, 0, result);
            return MCDOSE_PARTICLE_DMLC_STATUS_OK;
        }
        mcdose_particle_dmlc_producer_summary_v1 summary = {};
        summary.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        summary.struct_size = sizeof(summary);
        const int32_t producer_status = mcdose_particle_dmlc_produce_v1(
            context->producer, &incident.particle, incident.fractional_meterset,
            incident.scattered_photon_particle_id, incident.electron_particle_id,
            &summary, diagnostic, diagnostic_capacity);
        if (producer_status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            return producer_status;
        }
        context->queue_active = summary.retained_product_count != 0;
        /* A blocked incident produces no product and the source advances. */
    }
}
