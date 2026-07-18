#include "mcdose_particle_dmlc_host_adapter_v1.h"

#include <cmath>
#include <cstdio>
#include <new>
#include <vector>

struct mcdose_particle_dmlc_host_adapter_context_v1 {
    explicit mcdose_particle_dmlc_host_adapter_context_v1(uint32_t requested_capacity)
        : capacity(requested_capacity) {
        products.reserve(requested_capacity);
    }

    uint32_t capacity;
    std::vector<mcdose_particle_dmlc_particle_v1> products;
    size_t next_product = 0;
    mcdose_particle_dmlc_host_random_callback_v1 random_callback = nullptr;
    void *random_user_data = nullptr;
};

namespace {
constexpr uint32_t maximum_product_capacity = 64;
constexpr uint32_t maximum_random_draw_count = 4096;
constexpr double direction_tolerance = 1.0e-10;

int32_t fail(int32_t status, char *diagnostic, size_t capacity, const char *message) {
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

bool valid_particle(const mcdose_particle_dmlc_particle_v1 &particle) {
    if (particle.abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        particle.struct_size < sizeof(mcdose_particle_dmlc_particle_v1) ||
        particle.charge < MCDOSE_PARTICLE_DMLC_ELECTRON ||
        particle.charge > MCDOSE_PARTICLE_DMLC_POSITRON ||
        particle.reserved != 0 || particle.reserved_2 != 0 ||
        particle.history_id == 0 || particle.particle_id == 0 ||
        !std::isfinite(particle.energy_mev) || particle.energy_mev <= 0.0 ||
        !std::isfinite(particle.weight) || particle.weight <= 0.0) {
        return false;
    }
    double norm_squared = 0.0;
    for (size_t index = 0; index < 3; ++index) {
        if (!std::isfinite(particle.position_cm[index]) ||
            !std::isfinite(particle.direction[index])) {
            return false;
        }
        norm_squared += particle.direction[index] * particle.direction[index];
    }
    return std::fabs(norm_squared - 1.0) <= direction_tolerance;
}
}  // namespace

extern "C" int32_t mcdose_particle_dmlc_create_host_adapter_context_v1(
    uint32_t product_capacity,
    mcdose_particle_dmlc_host_adapter_context_v1 **context,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "host adapter context output is null");
    }
    *context = nullptr;
    if (product_capacity == 0 || product_capacity > maximum_product_capacity) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "host adapter capacity is outside [1,64]");
    }
    try {
        *context = new mcdose_particle_dmlc_host_adapter_context_v1(product_capacity);
    } catch (const std::bad_alloc &) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity, "host adapter context allocation failed");
    }
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" void mcdose_particle_dmlc_destroy_host_adapter_context_v1(
    mcdose_particle_dmlc_host_adapter_context_v1 *context) {
    delete context;
}

extern "C" int32_t mcdose_particle_dmlc_set_host_random_source_v1(
    mcdose_particle_dmlc_host_adapter_context_v1 *context,
    mcdose_particle_dmlc_host_random_callback_v1 callback,
    void *user_data,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr || callback == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "host adapter random source contains a null pointer");
    }
    context->random_callback = callback;
    context->random_user_data = user_data;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_sample_host_compton_products_v1(
    mcdose_particle_dmlc_host_adapter_context_v1 *context,
    const mcdose_particle_dmlc_particle_v1 *incident_photon,
    const mcdose_particle_dmlc_photon_attenuation_v1 *attenuation,
    const mcdose_particle_dmlc_forced_interaction_v1 *interaction,
    uint64_t scattered_photon_particle_id,
    uint64_t electron_particle_id,
    mcdose_particle_dmlc_compton_products_v1 *products,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr || products == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "host Compton sampling contains a null pointer");
    }
    if (context->random_callback == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "host adapter random source is not configured");
    }

    std::vector<double> random_values;
    try {
        random_values.reserve(16);
    } catch (const std::bad_alloc &) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "host Compton random buffer allocation failed");
    }
    while (true) {
        const int32_t status = mcdose_particle_dmlc_sample_compton_products_v1(
            incident_photon, attenuation, interaction,
            random_values.empty() ? nullptr : random_values.data(),
            random_values.size(), scattered_photon_particle_id,
            electron_particle_id, products, diagnostic, diagnostic_capacity);
        if (status == MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            if (products->random_draw_count != random_values.size()) {
                return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                            diagnostic, diagnostic_capacity,
                            "host and core random draw counts differ");
            }
            return status;
        }
        if (status != MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY) {
            return status;
        }
        if (products->random_draw_count != random_values.size()) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                        diagnostic, diagnostic_capacity,
                        "core random exhaustion count differs from host prefix");
        }
        if (random_values.size() >= maximum_random_draw_count) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY, diagnostic,
                        diagnostic_capacity,
                        "host Compton random draw limit was reached");
        }

        double uniform_random = 0.0;
        int32_t callback_status = context->random_callback(
            context->random_user_data, &uniform_random);
        if (callback_status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            products->random_draw_count =
                static_cast<uint32_t>(random_values.size());
            if (callback_status < MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT ||
                callback_status > MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED) {
                callback_status = MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED;
            }
            return fail(callback_status, diagnostic, diagnostic_capacity,
                        "host random callback failed");
        }
        if (!std::isfinite(uniform_random) || uniform_random < 0.0 ||
            uniform_random >= 1.0) {
            products->random_draw_count =
                static_cast<uint32_t>(random_values.size() + 1);
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity,
                        "host random callback returned a value outside [0,1)");
        }
        try {
            random_values.push_back(uniform_random);
        } catch (const std::bad_alloc &) {
            products->random_draw_count =
                static_cast<uint32_t>(random_values.size() + 1);
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                        diagnostic_capacity,
                        "host Compton random buffer allocation failed");
        }
    }
}

extern "C" int32_t mcdose_particle_dmlc_load_host_products_v1(
    mcdose_particle_dmlc_host_adapter_context_v1 *context,
    const mcdose_particle_dmlc_particle_v1 *products,
    uint32_t product_count,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr || (product_count > 0 && products == nullptr)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "host adapter load contains a null pointer");
    }
    if (context->next_product < context->products.size()) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "host adapter still contains undrained products");
    }
    if (product_count > context->capacity) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY, diagnostic,
                    diagnostic_capacity, "host adapter product capacity is insufficient");
    }
    if (product_count == 0) {
        context->products.clear();
        context->next_product = 0;
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }
    for (uint32_t index = 0; index < product_count; ++index) {
        if (!valid_particle(products[index])) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity, "host adapter product is invalid");
        }
    }
    context->products.assign(products, products + product_count);
    context->next_product = 0;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_next_host_product_v1(
    mcdose_particle_dmlc_host_adapter_context_v1 *context,
    mcdose_particle_dmlc_host_product_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr || result == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "host adapter next call contains a null pointer");
    }
    if (result->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        result->struct_size < sizeof(mcdose_particle_dmlc_host_product_v1)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity, "host adapter result ABI version or size differs");
    }
    if (context->next_product >= context->products.size()) {
        result->has_product = 0;
        result->remaining_product_count = 0;
        result->particle = {};
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }
    result->has_product = 1;
    result->particle = context->products[context->next_product];
    ++context->next_product;
    result->remaining_product_count =
        static_cast<uint32_t>(context->products.size() - context->next_product);
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}
