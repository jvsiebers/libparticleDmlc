#include "mcdose_particle_dmlc_producer_v1.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <vector>

namespace {
struct tagged_product {
    int32_t kind = 0;
    mcdose_particle_dmlc_particle_v1 particle = {};
};
}  // namespace

struct mcdose_particle_dmlc_producer_context_v1 {
    explicit mcdose_particle_dmlc_producer_context_v1(size_t total_pair_count)
        : bank_1_positions_cm(total_pair_count),
          bank_2_positions_cm(total_pair_count) {}

    mcdose_particle_dmlc_delivery_context_v1 *delivery = nullptr;
    mcdose_particle_dmlc_machine_context_v1 *machine = nullptr;
    mcdose_particle_dmlc_host_adapter_context_v1 *host_adapter = nullptr;
    uint64_t total_pair_count = 0;
    uint64_t mlc_opening_offset = 0;
    uint32_t mlc_leaf_pair_count = 0;
    bool jaw_tracking = false;
    uint64_t x_jaw_opening_offset = 0;
    uint64_t y_jaw_opening_offset = 0;
    double source_axis_distance_cm = 0.0;
    std::vector<double> bank_1_positions_cm;
    std::vector<double> bank_2_positions_cm;
    std::array<tagged_product, 2> products = {};
    uint32_t product_count = 0;
    uint32_t next_product = 0;
    mcdose_particle_dmlc_host_random_callback_v1 random_callback = nullptr;
    void *random_user_data = nullptr;
    mcdose_particle_dmlc_sampled_weight_callback_v1 sampled_weight_callback =
        nullptr;
    void *sampled_weight_user_data = nullptr;
};

namespace {
constexpr double geometry_tolerance = 1.0e-10;
constexpr double direction_tolerance = 1.0e-10;

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

bool nearly_equal(double first, double second) {
    const double scale = std::max({1.0, std::fabs(first), std::fabs(second)});
    return std::fabs(first - second) <= geometry_tolerance * scale;
}

bool zero_reserved(const uint32_t *values, size_t count) {
    for (size_t index = 0; index < count; ++index) {
        if (values[index] != 0) {
            return false;
        }
    }
    return true;
}

void reset_summary(mcdose_particle_dmlc_producer_summary_v1 *summary) {
    const uint32_t abi_version = summary->abi_version;
    const uint32_t struct_size = summary->struct_size;
    *summary = {};
    summary->abi_version = abi_version;
    summary->struct_size = struct_size;
}

void destroy_members(mcdose_particle_dmlc_producer_context_v1 *context) {
    if (context == nullptr) {
        return;
    }
    mcdose_particle_dmlc_destroy_host_adapter_context_v1(context->host_adapter);
    mcdose_particle_dmlc_destroy_machine_context_v1(context->machine);
    mcdose_particle_dmlc_destroy_delivery_context_v1(context->delivery);
    context->host_adapter = nullptr;
    context->machine = nullptr;
    context->delivery = nullptr;
}

int32_t sanitize_callback_status(int32_t status) {
    if (status < MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT ||
        status > MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED) {
        return MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED;
    }
    return status;
}

bool valid_photon_product_ids(
    const mcdose_particle_dmlc_particle_v1 *incident_particle,
    uint64_t scattered_photon_particle_id,
    uint64_t electron_particle_id) {
    return scattered_photon_particle_id != 0 && electron_particle_id != 0 &&
           scattered_photon_particle_id != electron_particle_id &&
           scattered_photon_particle_id != incident_particle->particle_id &&
           electron_particle_id != incident_particle->particle_id;
}

bool valid_incident_particle(
    const mcdose_particle_dmlc_particle_v1 *particle) {
    if (particle->charge < MCDOSE_PARTICLE_DMLC_ELECTRON ||
        particle->charge > MCDOSE_PARTICLE_DMLC_POSITRON ||
        particle->reserved != 0 || particle->reserved_2 != 0 ||
        particle->history_id == 0 || particle->particle_id == 0 ||
        !std::isfinite(particle->energy_mev) || particle->energy_mev <= 0.0 ||
        !std::isfinite(particle->weight) || particle->weight <= 0.0) {
        return false;
    }
    double norm_squared = 0.0;
    for (size_t coordinate = 0; coordinate < 3; ++coordinate) {
        if (!std::isfinite(particle->position_cm[coordinate]) ||
            !std::isfinite(particle->direction[coordinate])) {
            return false;
        }
        norm_squared +=
            particle->direction[coordinate] * particle->direction[coordinate];
    }
    return std::fabs(norm_squared - 1.0) <= direction_tolerance;
}

int32_t configure_jaw_tracking(
    const mcdose_particle_dmlc_delivery_v1 *delivery,
    mcdose_particle_dmlc_producer_context_v1 *context, char *diagnostic,
    size_t diagnostic_capacity) {
    std::array<int32_t, 2> jaw_device_indices = {-1, -1};
    for (uint32_t device = 0; device < delivery->device_count; ++device) {
        if (delivery->device_kinds[device] !=
            MCDOSE_PARTICLE_DMLC_DEVICE_JAW) {
            continue;
        }
        if (delivery->device_pair_counts[device] != 1) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                        diagnostic, diagnostic_capacity,
                        "producer jaw device must contain one pair");
        }
        const int32_t axis = delivery->device_axes[device];
        const size_t axis_index =
            axis == MCDOSE_PARTICLE_DMLC_AXIS_X ? 0u : 1u;
        if (jaw_device_indices[axis_index] >= 0) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                        diagnostic, diagnostic_capacity,
                        "producer delivery contains duplicate jaw axes");
        }
        jaw_device_indices[axis_index] = static_cast<int32_t>(device);
    }

    bool moving = false;
    for (int32_t device_index : jaw_device_indices) {
        if (device_index < 0) {
            continue;
        }
        const uint64_t opening_offset =
            delivery->device_opening_offsets[device_index];
        for (uint32_t point = 1; point < delivery->control_point_count;
             ++point) {
            const uint64_t position =
                static_cast<uint64_t>(point) * delivery->total_pair_count +
                opening_offset;
            if (!nearly_equal(delivery->bank_1_positions_cm[position],
                              delivery->bank_1_positions_cm[opening_offset]) ||
                !nearly_equal(delivery->bank_2_positions_cm[position],
                              delivery->bank_2_positions_cm[opening_offset])) {
                moving = true;
                break;
            }
        }
    }
    if (!moving) {
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }
    if (jaw_device_indices[0] < 0 || jaw_device_indices[1] < 0) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "jaw tracking requires one X and one Y jaw pair");
    }
    context->jaw_tracking = true;
    context->x_jaw_opening_offset = delivery->device_opening_offsets[
        static_cast<uint32_t>(jaw_device_indices[0])];
    context->y_jaw_opening_offset = delivery->device_opening_offsets[
        static_cast<uint32_t>(jaw_device_indices[1])];
    context->source_axis_distance_cm = delivery->source_axis_distance_cm;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

bool blocked_by_tracked_jaws(
    const mcdose_particle_dmlc_producer_context_v1 *context,
    const mcdose_particle_dmlc_particle_v1 *particle) {
    const double scale =
        particle->position_cm[2] / context->source_axis_distance_cm;
    const double x_min =
        context->bank_1_positions_cm[context->x_jaw_opening_offset] * scale;
    const double x_max =
        context->bank_2_positions_cm[context->x_jaw_opening_offset] * scale;
    const double y_min =
        context->bank_1_positions_cm[context->y_jaw_opening_offset] * scale;
    const double y_max =
        context->bank_2_positions_cm[context->y_jaw_opening_offset] * scale;
    return particle->position_cm[0] < x_min ||
           particle->position_cm[0] > x_max ||
           particle->position_cm[1] < y_min ||
           particle->position_cm[1] > y_max;
}
}  // namespace

extern "C" int32_t mcdose_particle_dmlc_create_producer_context_v1(
    const mcdose_particle_dmlc_delivery_v1 *delivery,
    const mcdose_particle_dmlc_machine_v1 *machine,
    const mcdose_particle_dmlc_producer_config_v1 *config,
    mcdose_particle_dmlc_producer_context_v1 **context,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (delivery == nullptr || machine == nullptr || config == nullptr ||
        context == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "producer context creation contains a null pointer");
    }
    *context = nullptr;
    if (config->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        config->struct_size < sizeof(*config)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "producer config ABI version or size differs");
    }
    if (!zero_reserved(config->reserved, 4) ||
        config->generated_electron_policy !=
            MCDOSE_PARTICLE_DMLC_GENERATED_ELECTRON_DISCARD) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "producer config contains an unsupported policy or reserved value");
    }
    int32_t status = mcdose_particle_dmlc_validate_delivery_v1(
        delivery, diagnostic, diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return status;
    }
    status = mcdose_particle_dmlc_validate_machine_v1(
        machine, diagnostic, diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return status;
    }
    if (config->mlc_device_index >= delivery->device_count) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "producer MLC device index is outside the delivery");
    }
    const uint32_t mlc_index = config->mlc_device_index;
    if (delivery->device_kinds[mlc_index] !=
            MCDOSE_PARTICLE_DMLC_DEVICE_MLC ||
        delivery->device_axes[mlc_index] != machine->motion_axis ||
        delivery->device_pair_counts[mlc_index] != machine->leaf_pair_count ||
        !nearly_equal(delivery->source_axis_distance_cm,
                      machine->source_axis_distance_cm) ||
        (delivery->device_source_distance_present[mlc_index] != 0 &&
         !nearly_equal(delivery->device_source_distances_cm[mlc_index],
                       machine->source_to_device_distance_cm))) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "producer MLC device and commissioned machine are incompatible");
    }
    if (delivery->total_pair_count >
        static_cast<uint64_t>(std::numeric_limits<size_t>::max())) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "producer delivery arrays exceed host addressable size");
    }

    std::unique_ptr<mcdose_particle_dmlc_producer_context_v1> result;
    try {
        result = std::make_unique<mcdose_particle_dmlc_producer_context_v1>(
            static_cast<size_t>(delivery->total_pair_count));
    } catch (const std::bad_alloc &) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "producer context allocation failed");
    } catch (const std::length_error &) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "producer delivery arrays are too large");
    }
    result->total_pair_count = delivery->total_pair_count;
    result->mlc_opening_offset = delivery->device_opening_offsets[mlc_index];
    result->mlc_leaf_pair_count = machine->leaf_pair_count;

    status = configure_jaw_tracking(delivery, result.get(), diagnostic,
                                    diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return status;
    }

    status = mcdose_particle_dmlc_create_delivery_context_v1(
        delivery, &result->delivery, diagnostic, diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return status;
    }
    status = mcdose_particle_dmlc_create_machine_context_v1(
        machine, &result->machine, diagnostic, diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        destroy_members(result.get());
        return status;
    }
    status = mcdose_particle_dmlc_create_host_adapter_context_v1(
        2, &result->host_adapter, diagnostic, diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        destroy_members(result.get());
        return status;
    }
    *context = result.release();
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" void mcdose_particle_dmlc_destroy_producer_context_v1(
    mcdose_particle_dmlc_producer_context_v1 *context) {
    destroy_members(context);
    delete context;
}

extern "C" int32_t mcdose_particle_dmlc_set_producer_random_source_v1(
    mcdose_particle_dmlc_producer_context_v1 *context,
    mcdose_particle_dmlc_host_random_callback_v1 callback,
    void *user_data,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr || callback == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "producer random source contains a null pointer");
    }
    const int32_t status = mcdose_particle_dmlc_set_host_random_source_v1(
        context->host_adapter, callback, user_data, diagnostic,
        diagnostic_capacity);
    if (status == MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        context->random_callback = callback;
        context->random_user_data = user_data;
    }
    return status;
}

extern "C" int32_t
mcdose_particle_dmlc_set_producer_sampled_weight_callback_v1(
    mcdose_particle_dmlc_producer_context_v1 *context,
    mcdose_particle_dmlc_sampled_weight_callback_v1 callback,
    void *user_data, char *diagnostic, size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr || callback == nullptr || user_data == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "producer sampled-weight callback contains a null pointer");
    }
    if (context->next_product < context->product_count ||
        context->sampled_weight_callback != nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "producer sampled-weight callback cannot be replaced");
    }
    context->sampled_weight_callback = callback;
    context->sampled_weight_user_data = user_data;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_produce_v1(
    mcdose_particle_dmlc_producer_context_v1 *context,
    const mcdose_particle_dmlc_particle_v1 *incident_particle,
    double fractional_meterset,
    uint64_t scattered_photon_particle_id,
    uint64_t electron_particle_id,
    mcdose_particle_dmlc_producer_summary_v1 *summary,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr || incident_particle == nullptr || summary == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "producer call contains a null pointer");
    }
    if (summary->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        summary->struct_size < sizeof(*summary) ||
        incident_particle->abi_version !=
            MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        incident_particle->struct_size < sizeof(*incident_particle)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "producer input or summary ABI version or size differs");
    }
    reset_summary(summary);
    if (context->next_product < context->product_count) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "producer still contains undrained products");
    }
    context->product_count = 0;
    context->next_product = 0;
    if (!valid_incident_particle(incident_particle)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "producer incident particle is invalid");
    }
    if (incident_particle->charge == MCDOSE_PARTICLE_DMLC_PHOTON &&
        !valid_photon_product_ids(incident_particle,
                                  scattered_photon_particle_id,
                                  electron_particle_id)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "producer photon product identifiers are invalid");
    }

    mcdose_particle_dmlc_sampled_state_v1 sampled_state = {};
    sampled_state.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    sampled_state.struct_size = sizeof(sampled_state);
    int32_t status = mcdose_particle_dmlc_sample_delivery_v1(
        context->delivery, fractional_meterset, &sampled_state,
        context->bank_1_positions_cm.data(),
        context->bank_2_positions_cm.data(), context->total_pair_count,
        diagnostic, diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return status;
    }

    mcdose_particle_dmlc_particle_v1 corrected_incident = *incident_particle;
    const mcdose_particle_dmlc_particle_v1 *transport_incident = incident_particle;
    if (context->sampled_weight_callback != nullptr) {
        double weight_factor = 0.0;
        status = context->sampled_weight_callback(
            context->sampled_weight_user_data, &sampled_state,
            context->bank_1_positions_cm.data(),
            context->bank_2_positions_cm.data(), context->total_pair_count,
            &weight_factor, diagnostic, diagnostic_capacity);
        if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            return sanitize_callback_status(status);
        }
        corrected_incident.weight *= weight_factor;
        if (!std::isfinite(weight_factor) || weight_factor <= 0.0 ||
            !std::isfinite(corrected_incident.weight) ||
            corrected_incident.weight <= 0.0) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                        diagnostic, diagnostic_capacity,
                        "producer sampled-weight callback returned an invalid factor");
        }
        transport_incident = &corrected_incident;
    }

    summary->source_segment_index = sampled_state.source_segment_index;
    summary->fractional_meterset = sampled_state.fractional_meterset;
    summary->interpolation_fraction = sampled_state.interpolation_fraction;
    if (context->jaw_tracking) {
        if (!std::isfinite(incident_particle->position_cm[2]) ||
            incident_particle->position_cm[2] <= 0.0) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                        diagnostic, diagnostic_capacity,
                        "jaw tracking requires a positive incident Z position");
        }
        if (blocked_by_tracked_jaws(context, transport_incident)) {
            return MCDOSE_PARTICLE_DMLC_STATUS_OK;
        }
    }

    const size_t mlc_offset = static_cast<size_t>(context->mlc_opening_offset);
    mcdose_particle_dmlc_aperture_v1 aperture = {};
    aperture.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    aperture.struct_size = sizeof(aperture);
    aperture.leaf_pair_count = context->mlc_leaf_pair_count;
    aperture.bank_1_positions_at_isocenter_cm =
        context->bank_1_positions_cm.data() + mlc_offset;
    aperture.bank_2_positions_at_isocenter_cm =
        context->bank_2_positions_cm.data() + mlc_offset;

    mcdose_particle_dmlc_ray_v1 ray = {};
    ray.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    ray.struct_size = sizeof(ray);
    std::copy(std::begin(transport_incident->position_cm),
              std::end(transport_incident->position_cm),
              std::begin(ray.position_cm));
    std::copy(std::begin(transport_incident->direction),
              std::end(transport_incident->direction),
              std::begin(ray.direction));
    mcdose_particle_dmlc_classification_v1 classification = {};
    classification.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    classification.struct_size = sizeof(classification);
    status = mcdose_particle_dmlc_classify_ray_v1(
        context->machine, &aperture, &ray, &classification, diagnostic,
        diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return status;
    }

    mcdose_particle_dmlc_equivalent_primary_v1 primary = {};
    primary.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    primary.struct_size = sizeof(primary);
    status = mcdose_particle_dmlc_finalize_equivalent_primary_v1(
        context->machine, &classification, transport_incident, &primary,
        diagnostic, diagnostic_capacity);
    if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
        return status;
    }

    std::array<tagged_product, 2> pending = {};
    uint32_t pending_count = 0;
    if (primary.has_particle != 0) {
        pending[pending_count].kind = MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY;
        pending[pending_count].particle = primary.particle;
        ++pending_count;
    }
    uint32_t scattered_retained = 0;
    uint32_t generated_electron_discarded = 0;
    uint32_t total_random_draw_count = 0;
    if (transport_incident->charge == MCDOSE_PARTICLE_DMLC_PHOTON &&
        primary.photon_attenuation.interaction_probability > 0.0 &&
        primary.photon_attenuation.incoherent_interaction_fraction > 0.0) {
        if (context->random_callback == nullptr) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                        diagnostic, diagnostic_capacity,
                        "producer random source is not configured");
        }
        double interaction_random = 0.0;
        int32_t callback_status = context->random_callback(
            context->random_user_data, &interaction_random);
        if (callback_status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            return fail(sanitize_callback_status(callback_status), diagnostic,
                        diagnostic_capacity,
                        "producer random callback failed");
        }
        total_random_draw_count = 1;
        summary->random_draw_count = total_random_draw_count;
        if (!std::isfinite(interaction_random) || interaction_random < 0.0 ||
            interaction_random >= 1.0) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                        diagnostic, diagnostic_capacity,
                        "producer random callback returned a value outside [0,1)");
        }

        mcdose_particle_dmlc_forced_interaction_v1 interaction = {};
        interaction.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        interaction.struct_size = sizeof(interaction);
        status = mcdose_particle_dmlc_sample_forced_interaction_v1(
            &primary.photon_attenuation, interaction_random, &interaction,
            diagnostic, diagnostic_capacity);
        if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            return status;
        }

        mcdose_particle_dmlc_compton_products_v1 compton = {};
        compton.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        compton.struct_size = sizeof(compton);
        status = mcdose_particle_dmlc_sample_host_compton_products_v1(
            context->host_adapter, transport_incident,
            &primary.photon_attenuation, &interaction,
            scattered_photon_particle_id, electron_particle_id, &compton,
            diagnostic, diagnostic_capacity);
        total_random_draw_count += compton.random_draw_count;
        summary->random_draw_count = total_random_draw_count;
        if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            return status;
        }
        if (compton.product_count != 2) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                        diagnostic, diagnostic_capacity,
                        "producer Compton sampler returned an unexpected product count");
        }
        generated_electron_discarded = 1;

        mcdose_particle_dmlc_equivalent_scattered_photon_v1 scattered = {};
        scattered.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        scattered.struct_size = sizeof(scattered);
        status = mcdose_particle_dmlc_finalize_equivalent_scattered_photon_v1(
            context->machine, &classification, &compton.scattered_photon,
            &interaction, &scattered, diagnostic, diagnostic_capacity);
        if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            return status;
        }
        if (scattered.has_particle != 0) {
            pending[pending_count].kind =
                MCDOSE_PARTICLE_DMLC_PRODUCER_SCATTERED_PHOTON;
            pending[pending_count].particle = scattered.particle;
            ++pending_count;
            scattered_retained = 1;
        }
    }

    context->products = pending;
    context->product_count = pending_count;
    context->next_product = 0;
    summary->retained_product_count = pending_count;
    summary->random_draw_count = total_random_draw_count;
    summary->primary_retained = primary.has_particle != 0 ? 1u : 0u;
    summary->scattered_photon_retained = scattered_retained;
    summary->generated_compton_electron_discarded =
        generated_electron_discarded;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_next_producer_product_v1(
    mcdose_particle_dmlc_producer_context_v1 *context,
    mcdose_particle_dmlc_producer_product_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity) {
    clear_diagnostic(diagnostic, diagnostic_capacity);
    if (context == nullptr || result == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "producer next-product call contains a null pointer");
    }
    if (result->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        result->struct_size < sizeof(*result)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "producer product ABI version or size differs");
    }
    result->has_product = 0;
    result->remaining_product_count = 0;
    result->product_kind = 0;
    result->reserved = 0;
    result->particle = {};
    if (context->next_product >= context->product_count) {
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }
    const tagged_product &product = context->products[context->next_product];
    ++context->next_product;
    result->has_product = 1;
    result->remaining_product_count =
        context->product_count - context->next_product;
    result->product_kind = product.kind;
    result->particle = product.particle;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}
