#include "mcdose_particle_dmlc_egsnrc_adapter_v1.h"

#include <cmath>
#include <cstdio>
#include <new>
#include <vector>

struct mcdose_particle_dmlc_egsnrc_stack_context_v1 {
    explicit mcdose_particle_dmlc_egsnrc_stack_context_v1(
        uint32_t requested_capacity,
        double requested_electron_rest_mass_mev)
        : capacity(requested_capacity),
          electron_rest_mass_mev(requested_electron_rest_mass_mev) {
        entries.reserve(requested_capacity);
    }

    uint32_t capacity;
    double electron_rest_mass_mev;
    std::vector<mcdose_particle_dmlc_egsnrc_stack_entry_v1> entries;
};

namespace {
constexpr uint32_t maximum_stack_capacity = 64;
constexpr double direction_tolerance = 1.0e-10;
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

bool valid_rest_mass(double electron_rest_mass_mev) {
    return std::isfinite(electron_rest_mass_mev) &&
           electron_rest_mass_mev > minimum_supported_rest_mass_mev &&
           electron_rest_mass_mev < maximum_supported_rest_mass_mev;
}

bool valid_direction_and_position(const double position_cm[3],
                                  const double direction[3]) {
    double norm_squared = 0.0;
    for (size_t index = 0; index < 3; ++index) {
        if (!std::isfinite(position_cm[index]) ||
            !std::isfinite(direction[index])) {
            return false;
        }
        norm_squared += direction[index] * direction[index];
    }
    return std::fabs(norm_squared - 1.0) <= direction_tolerance;
}

bool valid_particle_values(const mcdose_particle_dmlc_particle_v1 &particle) {
    return particle.charge >= MCDOSE_PARTICLE_DMLC_ELECTRON &&
           particle.charge <= MCDOSE_PARTICLE_DMLC_POSITRON &&
           particle.reserved == 0 && particle.reserved_2 == 0 &&
           particle.history_id != 0 && particle.particle_id != 0 &&
           std::isfinite(particle.energy_mev) && particle.energy_mev > 0.0 &&
           std::isfinite(particle.weight) && particle.weight > 0.0 &&
           valid_direction_and_position(particle.position_cm,
                                        particle.direction);
}

bool valid_entry_values(
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 &entry) {
    if (entry.iq < MCDOSE_PARTICLE_DMLC_ELECTRON ||
        entry.iq > MCDOSE_PARTICLE_DMLC_POSITRON || entry.region <= 0 ||
        entry.reserved != 0 || entry.reserved_2 != 0 ||
        entry.history_id == 0 || entry.particle_id == 0 ||
        !std::isfinite(entry.total_energy_mev) ||
        entry.total_energy_mev <= 0.0 || !std::isfinite(entry.weight) ||
        entry.weight <= 0.0 ||
        !valid_direction_and_position(entry.position_cm, entry.direction)) {
        return false;
    }
    return entry.iq == MCDOSE_PARTICLE_DMLC_PHOTON ||
           entry.total_energy_mev > minimum_supported_rest_mass_mev;
}

bool valid_entry_for_rest_mass(
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 &entry,
    double electron_rest_mass_mev) {
    return valid_entry_values(entry) &&
           (entry.iq == MCDOSE_PARTICLE_DMLC_PHOTON ||
            entry.total_energy_mev > electron_rest_mass_mev);
}

bool particle_abi_matches(const mcdose_particle_dmlc_particle_v1 &particle) {
    return particle.abi_version == MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION &&
           particle.struct_size >= sizeof(mcdose_particle_dmlc_particle_v1);
}

bool entry_abi_matches(
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 &entry) {
    return entry.abi_version == MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION &&
           entry.struct_size >=
               sizeof(mcdose_particle_dmlc_egsnrc_stack_entry_v1);
}
}  // namespace

extern "C" int32_t mcdose_particle_dmlc_particle_to_egsnrc_stack_entry_v1(
    const mcdose_particle_dmlc_particle_v1 *particle,
    int32_t region,
    int32_t latch,
    double electron_rest_mass_mev,
    mcdose_particle_dmlc_egsnrc_stack_entry_v1 *entry,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (particle == nullptr || entry == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc stack translation contains a null pointer");
    }
    if (!particle_abi_matches(*particle) || !entry_abi_matches(*entry)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc stack translation ABI version or size differs");
    }
    if (!valid_particle_values(*particle) || region <= 0 ||
        !valid_rest_mass(electron_rest_mass_mev)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "particle, region, or electron rest mass is invalid");
    }

    mcdose_particle_dmlc_egsnrc_stack_entry_v1 translated = {};
    translated.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    translated.struct_size = sizeof(translated);
    translated.iq = particle->charge;
    translated.region = region;
    translated.latch = latch;
    translated.history_id = particle->history_id;
    translated.particle_id = particle->particle_id;
    translated.parent_particle_id = particle->parent_particle_id;
    translated.total_energy_mev =
        particle->energy_mev +
        (particle->charge == MCDOSE_PARTICLE_DMLC_PHOTON
             ? 0.0
             : electron_rest_mass_mev);
    for (size_t index = 0; index < 3; ++index) {
        translated.position_cm[index] = particle->position_cm[index];
        translated.direction[index] = particle->direction[index];
    }
    translated.weight = particle->weight;
    *entry = translated;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_egsnrc_stack_entry_to_particle_v1(
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 *entry,
    double electron_rest_mass_mev,
    mcdose_particle_dmlc_particle_v1 *particle,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (entry == nullptr || particle == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc particle translation contains a null pointer");
    }
    if (!entry_abi_matches(*entry) || !particle_abi_matches(*particle)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc particle translation ABI version or size differs");
    }
    if (!valid_rest_mass(electron_rest_mass_mev) ||
        !valid_entry_for_rest_mass(*entry, electron_rest_mass_mev)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc stack entry or electron rest mass is invalid");
    }

    mcdose_particle_dmlc_particle_v1 translated = {};
    translated.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    translated.struct_size = sizeof(translated);
    translated.charge = entry->iq;
    translated.history_id = entry->history_id;
    translated.particle_id = entry->particle_id;
    translated.parent_particle_id = entry->parent_particle_id;
    for (size_t index = 0; index < 3; ++index) {
        translated.position_cm[index] = entry->position_cm[index];
        translated.direction[index] = entry->direction[index];
    }
    translated.energy_mev =
        entry->total_energy_mev -
        (entry->iq == MCDOSE_PARTICLE_DMLC_PHOTON
             ? 0.0
             : electron_rest_mass_mev);
    translated.weight = entry->weight;
    *particle = translated;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_dispatch_egsnrc_product_v1(
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 *entry,
    int32_t product_kind,
    double electron_rest_mass_mev,
    mcdose_particle_dmlc_egsnrc_shower_callback_v1 callback,
    void *user_data,
    mcdose_particle_dmlc_egsnrc_dispatch_result_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (entry == nullptr || callback == nullptr || result == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc product dispatch contains a null pointer");
    }
    if (!entry_abi_matches(*entry) ||
        result->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        result->struct_size <
            sizeof(mcdose_particle_dmlc_egsnrc_dispatch_result_v1)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc product dispatch ABI version or size differs");
    }
    if (!valid_rest_mass(electron_rest_mass_mev) ||
        !valid_entry_for_rest_mass(*entry, electron_rest_mass_mev) ||
        product_kind < MCDOSE_PARTICLE_DMLC_EGSNRC_PRIMARY ||
        product_kind >
            MCDOSE_PARTICLE_DMLC_EGSNRC_GENERATED_COMPTON_ELECTRON ||
        (product_kind == MCDOSE_PARTICLE_DMLC_EGSNRC_SCATTERED_PHOTON &&
         entry->iq != MCDOSE_PARTICLE_DMLC_PHOTON) ||
        (product_kind ==
             MCDOSE_PARTICLE_DMLC_EGSNRC_GENERATED_COMPTON_ELECTRON &&
         entry->iq != MCDOSE_PARTICLE_DMLC_ELECTRON)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc product kind, stack entry, or electron rest mass "
                    "is inconsistent");
    }

    result->dispatched = 0;
    result->discarded_by_policy = 0;
    result->product_kind = product_kind;
    result->reserved = 0;
    if (product_kind ==
        MCDOSE_PARTICLE_DMLC_EGSNRC_GENERATED_COMPTON_ELECTRON) {
        result->discarded_by_policy = 1;
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }

    int32_t callback_status = callback(user_data, entry);
    if (callback_status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        if (callback_status < MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT ||
            callback_status > MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED) {
            callback_status = MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED;
        }
        return fail(callback_status, diagnostic, diagnostic_capacity,
                    "EGSnrc SHOWER callback failed");
    }
    result->dispatched = 1;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_create_egsnrc_stack_context_v1(
    uint32_t entry_capacity,
    double electron_rest_mass_mev,
    mcdose_particle_dmlc_egsnrc_stack_context_v1 **context,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc stack context output is null");
    }
    *context = nullptr;
    if (entry_capacity == 0 || entry_capacity > maximum_stack_capacity ||
        !valid_rest_mass(electron_rest_mass_mev)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc stack capacity or electron rest mass is invalid");
    }
    try {
        *context = new mcdose_particle_dmlc_egsnrc_stack_context_v1(
            entry_capacity, electron_rest_mass_mev);
    } catch (const std::bad_alloc &) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc stack context allocation failed");
    }
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" void mcdose_particle_dmlc_destroy_egsnrc_stack_context_v1(
    mcdose_particle_dmlc_egsnrc_stack_context_v1 *context) {
    delete context;
}

extern "C" int32_t mcdose_particle_dmlc_push_egsnrc_stack_entry_v1(
    mcdose_particle_dmlc_egsnrc_stack_context_v1 *context,
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 *entry,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr || entry == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc stack push contains a null pointer");
    }
    if (!entry_abi_matches(*entry)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc stack entry ABI version or size differs");
    }
    if (!valid_entry_for_rest_mass(*entry,
                                   context->electron_rest_mass_mev)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "EGSnrc stack entry is invalid");
    }
    if (context->entries.size() >= context->capacity) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY, diagnostic,
                    diagnostic_capacity, "EGSnrc stack capacity is exhausted");
    }
    try {
        context->entries.push_back(*entry);
    } catch (const std::bad_alloc &) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity, "EGSnrc stack push allocation failed");
    }
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_pop_egsnrc_stack_entry_v1(
    mcdose_particle_dmlc_egsnrc_stack_context_v1 *context,
    mcdose_particle_dmlc_egsnrc_stack_result_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr || result == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc stack pop contains a null pointer");
    }
    if (result->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        result->struct_size <
            sizeof(mcdose_particle_dmlc_egsnrc_stack_result_v1)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "EGSnrc stack result ABI version or size differs");
    }
    if (context->entries.empty()) {
        result->has_entry = 0;
        result->remaining_entry_count = 0;
        result->entry = {};
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }
    result->has_entry = 1;
    result->entry = context->entries.back();
    context->entries.pop_back();
    result->remaining_entry_count =
        static_cast<uint32_t>(context->entries.size());
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}
