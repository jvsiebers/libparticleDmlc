#include "mcdose_particle_dmlc_producer_v1.h"

#include <array>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <thread>
#include <vector>

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::fprintf(stderr, "check failed: %s at %s:%d\n", #condition,  \
                         __FILE__, __LINE__);                                  \
            return 1;                                                          \
        }                                                                      \
    } while (false)

#define WORKER_CHECK(condition)                                                \
    do {                                                                       \
        if (!(condition)) {                                                    \
            return __LINE__;                                                   \
        }                                                                      \
    } while (false)

namespace {
struct fixture {
    int32_t kinds[3] = {1, 1, 2};
    int32_t axes[3] = {1, 2, 1};
    uint32_t pairs[3] = {1, 1, 2};
    double distances[3] = {36.7, 28.0, 50.0};
    uint8_t distance_present[3] = {1, 1, 1};
    uint64_t opening_offsets[4] = {0, 1, 2, 4};
    uint64_t boundary_offsets[4] = {0, 0, 0, 3};
    double meterset[2] = {0.0, 1.0};
    double gantry[2] = {0.0, 0.0};
    int32_t directions[2] = {0, 0};
    double collimator[2] = {0.0, 0.0};
    double support[2] = {0.0, 0.0};
    double isocenter[6] = {};
    double nominal_energy[2] = {6.0, 6.0};
    double dose_rate[2] = {600.0, 600.0};
    uint8_t dose_present[2] = {1, 1};
    double pitch[2] = {};
    double roll[2] = {};
    double bank_1[8] = {-10.0, -10.0, -1.0, -2.0,
                         -10.0, -10.0, -0.5, -2.0};
    double bank_2[8] = {10.0, 10.0, 1.0, 2.0,
                         10.0, 10.0, 0.5, 2.0};
    double boundaries[3] = {-1.0, 0.0, 1.0};
    mcdose_particle_dmlc_delivery_v1 delivery = {
        1,
        sizeof(mcdose_particle_dmlc_delivery_v1),
        7,
        3,
        2,
        0,
        4,
        3,
        100.0,
        kinds,
        axes,
        pairs,
        distances,
        distance_present,
        opening_offsets,
        boundary_offsets,
        meterset,
        gantry,
        directions,
        collimator,
        directions,
        support,
        directions,
        isocenter,
        nominal_energy,
        dose_rate,
        dose_present,
        pitch,
        roll,
        bank_1,
        bank_2,
        boundaries,
    };

    double section_enter[2] = {47.0, 50.0};
    double section_center[2] = {48.5, 51.5};
    double section_exit[2] = {50.0, 53.0};
    uint64_t row_offsets[3] = {0, 2, 4};
    double y_start[4] = {-1.0, 0.0, -1.0, 0.0};
    double y_end[4] = {0.0, 1.0, 0.0, 1.0};
    uint32_t leaf_indices[4] = {0, 1, 0, 1};
    double thickness[4] = {3.0, 3.1, 3.2, 3.3};
    double groove_thickness[4] = {0.0, 0.1, 0.0, 0.1};
    double groove_distance[4] = {0.0, 1.0, 0.0, 1.0};
    double energy[2] = {0.1, 2.0};
    double total_attenuation[2] = {10.0, 1.0};
    double incoherent_attenuation[2] = {1.0, 0.1};
    mcdose_particle_dmlc_machine_v1 machine = {};

    fixture() {
        machine.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        machine.struct_size = sizeof(machine);
        machine.leaf_pair_count = 2;
        machine.section_count = 2;
        machine.section_row_count = 4;
        machine.attenuation_point_count = 2;
        machine.source_axis_distance_cm = 100.0;
        machine.source_to_device_distance_cm = 50.0;
        machine.position_specification_distance_cm = 100.0;
        machine.material_density_g_cm3 = 17.0;
        machine.calibration_mode =
            MCDOSE_PARTICLE_DMLC_LEAF_CALIBRATION_IDENTITY;
        machine.motion_axis = MCDOSE_PARTICLE_DMLC_AXIS_X;
        machine.projection_ratio = 2.0;
        machine.rounded_tip_maximum_thickness_cm = 6.0;
        machine.rounded_tip_radius_cm = 8.0;
        machine.rounded_tip_angle_deg = 11.0;
        machine.rounded_tip_transition_distance_cm = 0.5;
        machine.rounded_tip_section_count = 12;
        machine.section_z_enter_cm = section_enter;
        machine.section_z_center_cm = section_center;
        machine.section_z_exit_cm = section_exit;
        machine.section_row_offsets = row_offsets;
        machine.section_y_start_cm = y_start;
        machine.section_y_end_cm = y_end;
        machine.section_leaf_pair_indices = leaf_indices;
        machine.section_thickness_cm = thickness;
        machine.section_groove_thickness_cm = groove_thickness;
        machine.section_groove_distance_from_tip_cm = groove_distance;
        machine.attenuation_energy_mev = energy;
        machine.total_linear_attenuation_per_cm = total_attenuation;
        machine.incoherent_linear_attenuation_per_cm = incoherent_attenuation;
    }
};

mcdose_particle_dmlc_producer_config_v1 config(uint32_t mlc_index) {
    mcdose_particle_dmlc_producer_config_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.mlc_device_index = mlc_index;
    result.generated_electron_policy =
        MCDOSE_PARTICLE_DMLC_GENERATED_ELECTRON_DISCARD;
    return result;
}

mcdose_particle_dmlc_particle_v1 particle(int32_t charge, double x, double y) {
    mcdose_particle_dmlc_particle_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.charge = charge;
    result.history_id = 17;
    result.particle_id = 23;
    result.position_cm[0] = x;
    result.position_cm[1] = y;
    result.position_cm[2] = 45.0;
    result.direction[2] = 1.0;
    result.energy_mev = charge == MCDOSE_PARTICLE_DMLC_PHOTON ? 1.0 : 4.0;
    result.weight = 0.25;
    return result;
}

mcdose_particle_dmlc_producer_summary_v1 summary() {
    mcdose_particle_dmlc_producer_summary_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}

mcdose_particle_dmlc_producer_product_v1 product() {
    mcdose_particle_dmlc_producer_product_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}

struct random_state {
    const double *values;
    size_t value_count;
    size_t next_value;
};

int32_t next_random(void *user_data, double *uniform_random) {
    if (user_data == nullptr || uniform_random == nullptr) {
        return MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT;
    }
    auto *state = static_cast<random_state *>(user_data);
    if (state->next_value >= state->value_count) {
        return MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY;
    }
    *uniform_random = state->values[state->next_value];
    ++state->next_value;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

struct sampled_weight_state {
    double factor;
    size_t callback_count;
};

int32_t sampled_weight(
    void *user_data,
    const mcdose_particle_dmlc_sampled_state_v1 *sampled_state,
    const double *bank_1_positions_cm,
    const double *bank_2_positions_cm,
    uint64_t position_count,
    double *weight_factor,
    char *,
    size_t) {
    if (user_data == nullptr || sampled_state == nullptr ||
        bank_1_positions_cm == nullptr || bank_2_positions_cm == nullptr ||
        weight_factor == nullptr || position_count != 4 ||
        sampled_state->source_segment_index != 0 ||
        sampled_state->fractional_meterset != 0.25 ||
        sampled_state->interpolation_fraction != 0.25) {
        return MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED;
    }
    auto *state = static_cast<sampled_weight_state *>(user_data);
    ++state->callback_count;
    *weight_factor = state->factor;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

int run_concurrent_worker(
    const fixture &value,
    size_t worker_index,
    std::atomic<size_t> &ready_worker_count,
    std::atomic<bool> &start,
    double &normalized_primary_weight,
    double &normalized_scattered_weight) {
    ready_worker_count.fetch_add(1, std::memory_order_release);
    while (!start.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }

    char diagnostic[256] = {};
    auto producer_config = config(2);
    mcdose_particle_dmlc_producer_context_v1 *raw_context = nullptr;
    WORKER_CHECK(mcdose_particle_dmlc_create_producer_context_v1(
                     &value.delivery, &value.machine, &producer_config,
                     &raw_context, diagnostic, sizeof(diagnostic)) ==
                 MCDOSE_PARTICLE_DMLC_STATUS_OK);
    WORKER_CHECK(raw_context != nullptr);
    using context_pointer = std::unique_ptr<
        mcdose_particle_dmlc_producer_context_v1,
        decltype(&mcdose_particle_dmlc_destroy_producer_context_v1)>;
    context_pointer context(
        raw_context, mcdose_particle_dmlc_destroy_producer_context_v1);

    constexpr size_t iteration_count = 128;
    constexpr size_t draws_per_iteration = 5;
    std::vector<double> random_values(
        iteration_count * draws_per_iteration, 0.5);
    random_state random = {random_values.data(), random_values.size(), 0};
    WORKER_CHECK(mcdose_particle_dmlc_set_producer_random_source_v1(
                     context.get(), next_random, &random, diagnostic,
                     sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    sampled_weight_state weight = {
        1.0 + static_cast<double>(worker_index) / 100.0, 0};
    WORKER_CHECK(mcdose_particle_dmlc_set_producer_sampled_weight_callback_v1(
                     context.get(), sampled_weight, &weight, diagnostic,
                     sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);

    for (size_t index = 0; index < iteration_count; ++index) {
        auto incident =
            particle(MCDOSE_PARTICLE_DMLC_PHOTON, 1.1, -0.5);
        incident.history_id =
            1 + worker_index * iteration_count + index;
        incident.particle_id =
            1 + (worker_index * iteration_count + index) * 3;
        const uint64_t scattered_id = incident.particle_id + 1;
        const uint64_t electron_id = incident.particle_id + 2;
        auto produced = summary();
        WORKER_CHECK(mcdose_particle_dmlc_produce_v1(
                         context.get(), &incident, 0.25, scattered_id,
                         electron_id, &produced, diagnostic,
                         sizeof(diagnostic)) ==
                     MCDOSE_PARTICLE_DMLC_STATUS_OK);
        WORKER_CHECK(produced.retained_product_count == 2);
        WORKER_CHECK(produced.random_draw_count == draws_per_iteration);
        WORKER_CHECK(produced.primary_retained == 1);
        WORKER_CHECK(produced.scattered_photon_retained == 1);
        WORKER_CHECK(produced.generated_compton_electron_discarded == 1);

        auto primary = product();
        WORKER_CHECK(mcdose_particle_dmlc_next_producer_product_v1(
                         context.get(), &primary, diagnostic,
                         sizeof(diagnostic)) ==
                     MCDOSE_PARTICLE_DMLC_STATUS_OK);
        WORKER_CHECK(primary.has_product == 1);
        WORKER_CHECK(primary.remaining_product_count == 1);
        WORKER_CHECK(primary.product_kind ==
                     MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY);
        WORKER_CHECK(primary.particle.history_id == incident.history_id);
        WORKER_CHECK(primary.particle.particle_id == incident.particle_id);

        auto scattered = product();
        WORKER_CHECK(mcdose_particle_dmlc_next_producer_product_v1(
                         context.get(), &scattered, diagnostic,
                         sizeof(diagnostic)) ==
                     MCDOSE_PARTICLE_DMLC_STATUS_OK);
        WORKER_CHECK(scattered.has_product == 1);
        WORKER_CHECK(scattered.remaining_product_count == 0);
        WORKER_CHECK(scattered.product_kind ==
                     MCDOSE_PARTICLE_DMLC_PRODUCER_SCATTERED_PHOTON);
        WORKER_CHECK(scattered.particle.history_id == incident.history_id);
        WORKER_CHECK(scattered.particle.particle_id == scattered_id);
        WORKER_CHECK(scattered.particle.parent_particle_id ==
                     incident.particle_id);

        auto empty = product();
        WORKER_CHECK(mcdose_particle_dmlc_next_producer_product_v1(
                         context.get(), &empty, diagnostic,
                         sizeof(diagnostic)) ==
                     MCDOSE_PARTICLE_DMLC_STATUS_OK);
        WORKER_CHECK(empty.has_product == 0);
        if (index == 0) {
            normalized_primary_weight = primary.particle.weight / weight.factor;
            normalized_scattered_weight =
                scattered.particle.weight / weight.factor;
        } else {
            WORKER_CHECK(primary.particle.weight / weight.factor ==
                         normalized_primary_weight);
            WORKER_CHECK(scattered.particle.weight / weight.factor ==
                         normalized_scattered_weight);
        }
    }
    WORKER_CHECK(random.next_value == iteration_count * draws_per_iteration);
    WORKER_CHECK(weight.callback_count == iteration_count);
    return 0;
}

int run_concurrent_context_test(const fixture &value) {
    constexpr size_t worker_count = 8;
    std::array<std::thread, worker_count> workers;
    std::array<int, worker_count> worker_results = {};
    std::array<double, worker_count> normalized_primary_weights = {};
    std::array<double, worker_count> normalized_scattered_weights = {};
    std::atomic<size_t> ready_worker_count = 0;
    std::atomic<bool> start = false;

    for (size_t worker_index = 0; worker_index < worker_count; ++worker_index) {
        workers[worker_index] = std::thread([&, worker_index]() {
            worker_results[worker_index] = run_concurrent_worker(
                value, worker_index, ready_worker_count, start,
                normalized_primary_weights[worker_index],
                normalized_scattered_weights[worker_index]);
        });
    }
    while (ready_worker_count.load(std::memory_order_acquire) < worker_count) {
        std::this_thread::yield();
    }
    start.store(true, std::memory_order_release);
    for (auto &worker : workers) {
        worker.join();
    }
    for (size_t worker_index = 0; worker_index < worker_count; ++worker_index) {
        if (worker_results[worker_index] != 0) {
            std::fprintf(stderr, "concurrent worker %zu failed at line %d\n",
                         worker_index, worker_results[worker_index]);
            return 1;
        }
        if (std::fabs(normalized_primary_weights[worker_index] -
                      normalized_primary_weights[0]) > 1.0e-12 ||
            std::fabs(normalized_scattered_weights[worker_index] -
                      normalized_scattered_weights[0]) > 1.0e-12) {
            std::fprintf(stderr,
                         "concurrent worker %zu produced a different weight\n",
                         worker_index);
            return 1;
        }
    }
    return 0;
}
}  // namespace

int main() {
    static_assert(sizeof(mcdose_particle_dmlc_producer_config_v1) == 32);
    static_assert(sizeof(mcdose_particle_dmlc_producer_summary_v1) == 48);
    static_assert(sizeof(mcdose_particle_dmlc_producer_product_v1) == 136);
    char diagnostic[256];
    fixture value;

    auto invalid_config = config(0);
    mcdose_particle_dmlc_producer_context_v1 *context = nullptr;
    CHECK(mcdose_particle_dmlc_create_producer_context_v1(
              &value.delivery, &value.machine, &invalid_config, &context,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(context == nullptr);
    CHECK(std::strstr(diagnostic, "incompatible") != nullptr);

    auto producer_config = config(2);
    CHECK(mcdose_particle_dmlc_create_producer_context_v1(
              &value.delivery, &value.machine, &producer_config, &context,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(context != nullptr);

    const double fixed_values[] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
    random_state fixed = {fixed_values, 7, 0};
    CHECK(mcdose_particle_dmlc_set_producer_random_source_v1(
              context, next_random, &fixed, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);

    auto blocked_photon = particle(MCDOSE_PARTICLE_DMLC_PHOTON, 1.1, -0.5);
    auto blocked_summary = summary();
    CHECK(mcdose_particle_dmlc_produce_v1(
              context, &blocked_photon, 0.25, 24, 25, &blocked_summary,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(blocked_summary.retained_product_count == 2);
    CHECK(blocked_summary.random_draw_count == 5);
    CHECK(blocked_summary.source_segment_index == 0);
    CHECK(blocked_summary.interpolation_fraction == 0.25);
    CHECK(blocked_summary.primary_retained == 1);
    CHECK(blocked_summary.scattered_photon_retained == 1);
    CHECK(blocked_summary.generated_compton_electron_discarded == 1);
    CHECK(fixed.next_value == 5);

    auto undrained_summary = summary();
    CHECK(mcdose_particle_dmlc_produce_v1(
              context, &blocked_photon, 0.5, 26, 27, &undrained_summary,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(fixed.next_value == 5);

    auto primary = product();
    CHECK(mcdose_particle_dmlc_next_producer_product_v1(
              context, &primary, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(primary.has_product == 1);
    CHECK(primary.product_kind == MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY);
    CHECK(primary.remaining_product_count == 1);
    CHECK(primary.particle.particle_id == 23);
    CHECK(primary.particle.parent_particle_id == 0);
    CHECK(primary.particle.weight < blocked_photon.weight);

    auto scattered = product();
    CHECK(mcdose_particle_dmlc_next_producer_product_v1(
              context, &scattered, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(scattered.has_product == 1);
    CHECK(scattered.product_kind ==
          MCDOSE_PARTICLE_DMLC_PRODUCER_SCATTERED_PHOTON);
    CHECK(scattered.remaining_product_count == 0);
    CHECK(scattered.particle.particle_id == 24);
    CHECK(scattered.particle.parent_particle_id == 23);

    auto empty = product();
    CHECK(mcdose_particle_dmlc_next_producer_product_v1(
              context, &empty, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(empty.has_product == 0);
    CHECK(empty.product_kind == 0);

    const size_t before_open = fixed.next_value;
    auto open_electron = particle(MCDOSE_PARTICLE_DMLC_ELECTRON, 0.0, -0.5);
    auto open_summary = summary();
    CHECK(mcdose_particle_dmlc_produce_v1(
              context, &open_electron, 0.5, 0, 0, &open_summary, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(open_summary.retained_product_count == 1);
    CHECK(open_summary.random_draw_count == 0);
    CHECK(fixed.next_value == before_open);
    auto open_product = product();
    CHECK(mcdose_particle_dmlc_next_producer_product_v1(
              context, &open_product, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(open_product.product_kind == MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY);
    CHECK(open_product.particle.charge == MCDOSE_PARTICLE_DMLC_ELECTRON);

    auto blocked_electron = particle(MCDOSE_PARTICLE_DMLC_ELECTRON, 1.1, -0.5);
    auto electron_summary = summary();
    CHECK(mcdose_particle_dmlc_produce_v1(
              context, &blocked_electron, 0.25, 0, 0, &electron_summary,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(electron_summary.retained_product_count == 0);
    CHECK(electron_summary.random_draw_count == 0);

    random_state exhausted = {fixed_values, 1, 0};
    CHECK(mcdose_particle_dmlc_set_producer_random_source_v1(
              context, next_random, &exhausted, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    auto exhausted_summary = summary();
    CHECK(mcdose_particle_dmlc_produce_v1(
              context, &blocked_photon, 0.25, 30, 31, &exhausted_summary,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY);
    CHECK(exhausted_summary.random_draw_count == 1);
    auto after_failure = product();
    CHECK(mcdose_particle_dmlc_next_producer_product_v1(
              context, &after_failure, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(after_failure.has_product == 0);

    auto invalid_ids_summary = summary();
    CHECK(mcdose_particle_dmlc_produce_v1(
              context, &blocked_photon, 0.25, 23, 31,
              &invalid_ids_summary, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(exhausted.next_value == 1);

    mcdose_particle_dmlc_destroy_producer_context_v1(context);
    CHECK(run_concurrent_context_test(value) == 0);

    value.bank_1[4] = -1.0;
    value.bank_2[4] = 1.0;
    value.bank_1[5] = -1.0;
    value.bank_2[5] = 4.0;
    context = nullptr;
    CHECK(mcdose_particle_dmlc_create_producer_context_v1(
              &value.delivery, &value.machine, &producer_config, &context,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);

    random_state moving_jaw_random = {fixed_values, 7, 0};
    CHECK(mcdose_particle_dmlc_set_producer_random_source_v1(
              context, next_random, &moving_jaw_random, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);

    auto jaw_blocked = particle(MCDOSE_PARTICLE_DMLC_PHOTON, 0.5, -0.5);
    auto jaw_blocked_summary = summary();
    CHECK(mcdose_particle_dmlc_produce_v1(
              context, &jaw_blocked, 1.0, 40, 41, &jaw_blocked_summary,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(jaw_blocked_summary.retained_product_count == 0);
    CHECK(jaw_blocked_summary.random_draw_count == 0);
    CHECK(jaw_blocked_summary.source_segment_index == 0);
    CHECK(jaw_blocked_summary.fractional_meterset == 1.0);
    CHECK(jaw_blocked_summary.interpolation_fraction == 1.0);
    CHECK(moving_jaw_random.next_value == 0);
    auto jaw_blocked_product = product();
    CHECK(mcdose_particle_dmlc_next_producer_product_v1(
              context, &jaw_blocked_product, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(jaw_blocked_product.has_product == 0);

    auto y_jaw_blocked = particle(MCDOSE_PARTICLE_DMLC_ELECTRON, 0.0, 0.75);
    auto y_jaw_blocked_summary = summary();
    CHECK(mcdose_particle_dmlc_produce_v1(
              context, &y_jaw_blocked, 1.0, 0, 0, &y_jaw_blocked_summary,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(y_jaw_blocked_summary.retained_product_count == 0);
    auto y_jaw_blocked_product = product();
    CHECK(mcdose_particle_dmlc_next_producer_product_v1(
              context, &y_jaw_blocked_product, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(y_jaw_blocked_product.has_product == 0);

    auto invalid_position = jaw_blocked;
    invalid_position.position_cm[0] = std::nan("");
    auto invalid_position_summary = summary();
    CHECK(mcdose_particle_dmlc_produce_v1(
              context, &invalid_position, 1.0, 42, 43,
              &invalid_position_summary, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    auto invalid_plane = jaw_blocked;
    invalid_plane.position_cm[2] = 0.0;
    auto invalid_plane_summary = summary();
    CHECK(mcdose_particle_dmlc_produce_v1(
              context, &invalid_plane, 1.0, 44, 45, &invalid_plane_summary,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(std::strstr(diagnostic, "positive incident Z") != nullptr);

    auto jaw_open = particle(MCDOSE_PARTICLE_DMLC_ELECTRON, 0.0, -0.5);
    auto jaw_open_summary = summary();
    CHECK(mcdose_particle_dmlc_produce_v1(
              context, &jaw_open, 1.0, 0, 0, &jaw_open_summary, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(jaw_open_summary.retained_product_count == 1);
    CHECK(jaw_open_summary.random_draw_count == 0);
    auto jaw_open_product = product();
    CHECK(mcdose_particle_dmlc_next_producer_product_v1(
              context, &jaw_open_product, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(jaw_open_product.has_product == 1);
    CHECK(jaw_open_product.product_kind ==
          MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY);

    mcdose_particle_dmlc_destroy_producer_context_v1(context);
    mcdose_particle_dmlc_destroy_producer_context_v1(nullptr);
    return 0;
}
