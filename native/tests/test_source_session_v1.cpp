#include "mcdose_particle_dmlc_source_session_v1.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::fprintf(stderr, "check failed: %s at %s:%d\\n", #condition,  \
                         __FILE__, __LINE__);                                  \
            return 1;                                                          \
        }                                                                      \
    } while (false)

namespace {
mcdose_particle_dmlc_source_session_config_v1 config(
    mcdose_particle_dmlc_producer_context_v1 *producer = nullptr) {
    mcdose_particle_dmlc_source_session_config_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.producer = producer;
    return result;
}

mcdose_particle_dmlc_source_session_result_v1 result() {
    mcdose_particle_dmlc_source_session_result_v1 value = {};
    value.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    value.struct_size = sizeof(value);
    return value;
}

mcdose_particle_dmlc_source_session_summary_v1 session_summary() {
    mcdose_particle_dmlc_source_session_summary_v1 value = {};
    value.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    value.struct_size = sizeof(value);
    return value;
}

mcdose_particle_dmlc_source_incident_v1 incident(uint64_t history_id,
                                                   uint64_t particle_id,
                                                   uint32_t starts_new_history) {
    mcdose_particle_dmlc_source_incident_v1 value = {};
    value.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    value.struct_size = sizeof(value);
    value.has_incident = 1;
    value.starts_new_history = starts_new_history;
    value.fractional_meterset = 0.25;
    value.z_last_cm = 42.0;
    value.latch = 19;
    value.photon_history = 1;
    value.scattered_photon_particle_id = particle_id + 1000;
    value.electron_particle_id = particle_id + 2000;
    value.particle.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    value.particle.struct_size = sizeof(value.particle);
    value.particle.charge = MCDOSE_PARTICLE_DMLC_PHOTON;
    value.particle.history_id = history_id;
    value.particle.particle_id = particle_id;
    value.particle.position_cm[2] = 45.0;
    value.particle.direction[2] = 1.0;
    value.particle.energy_mev = 1.0;
    value.particle.weight = 0.25;
    return value;
}

struct source_state {
    std::vector<mcdose_particle_dmlc_source_incident_v1> incidents;
    size_t next = 0;
};

int32_t next_incident(void *user_data,
                      mcdose_particle_dmlc_source_incident_v1 *value, char *,
                      size_t) {
    if (user_data == nullptr || value == nullptr) {
        return MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT;
    }
    auto *state = static_cast<source_state *>(user_data);
    if (state->next >= state->incidents.size()) {
        value->has_incident = 0;
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    }
    *value = state->incidents[state->next++];
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

struct fixture {
    int32_t kinds[1] = {MCDOSE_PARTICLE_DMLC_DEVICE_MLC};
    int32_t axes[1] = {MCDOSE_PARTICLE_DMLC_AXIS_X};
    uint32_t pairs[1] = {1};
    double distances[1] = {50.0};
    uint8_t distance_present[1] = {1};
    uint64_t opening_offsets[2] = {0, 1};
    uint64_t boundary_offsets[2] = {0, 2};
    double meterset[2] = {0.0, 1.0};
    double zeroes[6] = {};
    int32_t directions[2] = {};
    double energy[2] = {6.0, 6.0};
    double dose_rate[2] = {600.0, 600.0};
    uint8_t dose_present[2] = {1, 1};
    double bank_1[2] = {-1.0, -1.0};
    double bank_2[2] = {1.0, 1.0};
    double boundaries[2] = {-1.0, 1.0};
    mcdose_particle_dmlc_delivery_v1 delivery = {
        MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION,
        sizeof(mcdose_particle_dmlc_delivery_v1),
        7,
        1,
        2,
        0,
        1,
        2,
        100.0,
        kinds,
        axes,
        pairs,
        distances,
        distance_present,
        opening_offsets,
        boundary_offsets,
        meterset,
        zeroes,
        directions,
        zeroes,
        directions,
        zeroes,
        directions,
        zeroes,
        energy,
        dose_rate,
        dose_present,
        zeroes,
        zeroes,
        bank_1,
        bank_2,
        boundaries,
    };
    double section_enter[2] = {47.0, 50.0};
    double section_center[2] = {48.5, 51.5};
    double section_exit[2] = {50.0, 53.0};
    uint64_t row_offsets[3] = {0, 1, 2};
    double y_start[2] = {-1.0, -1.0};
    double y_end[2] = {1.0, 1.0};
    uint32_t leaf_indices[2] = {0, 0};
    double thickness[2] = {3.0, 3.2};
    double groove[2] = {0.0, 0.0};
    double attenuation_energy[2] = {0.1, 2.0};
    double total_attenuation[2] = {10.0, 1.0};
    double incoherent_attenuation[2] = {1.0, 0.1};
    mcdose_particle_dmlc_machine_v1 machine = {};

    fixture() {
        machine.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        machine.struct_size = sizeof(machine);
        machine.leaf_pair_count = 1;
        machine.section_count = 2;
        machine.section_row_count = 2;
        machine.attenuation_point_count = 2;
        machine.source_axis_distance_cm = 100.0;
        machine.source_to_device_distance_cm = 50.0;
        machine.position_specification_distance_cm = 100.0;
        machine.material_density_g_cm3 = 17.0;
        machine.calibration_mode = MCDOSE_PARTICLE_DMLC_LEAF_CALIBRATION_IDENTITY;
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
        machine.section_groove_thickness_cm = groove;
        machine.section_groove_distance_from_tip_cm = groove;
        machine.attenuation_energy_mev = attenuation_energy;
        machine.total_linear_attenuation_per_cm = total_attenuation;
        machine.incoherent_linear_attenuation_per_cm = incoherent_attenuation;
    }
};

struct random_state {
    std::array<double, 8> values = {0.5, 0.5, 0.5, 0.5,
                                    0.5, 0.5, 0.5, 0.5};
    size_t next = 0;
};

int32_t next_random(void *user_data, double *value) {
    if (user_data == nullptr || value == nullptr) {
        return MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT;
    }
    auto *state = static_cast<random_state *>(user_data);
    if (state->next >= state->values.size()) {
        return MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY;
    }
    *value = state->values[state->next++];
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}
}  // namespace

int main() {
    char diagnostic[512] = {};
    auto invalid_config = config();
    invalid_config.abi_version = 0;
    mcdose_particle_dmlc_source_session_context_v1 *invalid_raw = nullptr;
    CHECK(mcdose_particle_dmlc_create_source_session_v1(
              &invalid_config, next_incident, nullptr, &invalid_raw, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH);
    CHECK(invalid_raw == nullptr);
    invalid_config = config();
    invalid_config.reserved[0] = 1;
    CHECK(mcdose_particle_dmlc_create_source_session_v1(
              &invalid_config, next_incident, nullptr, &invalid_raw, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(invalid_raw == nullptr);

    source_state direct = {{incident(41, 101, 1), incident(41, 102, 0)}};
    auto direct_config = config();
    mcdose_particle_dmlc_source_session_context_v1 *direct_raw = nullptr;
    CHECK(mcdose_particle_dmlc_create_source_session_v1(
              &direct_config, next_incident, &direct, &direct_raw, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    using session_pointer = std::unique_ptr<
        mcdose_particle_dmlc_source_session_context_v1,
        decltype(&mcdose_particle_dmlc_destroy_source_session_context_v1)>;
    session_pointer direct_session(
        direct_raw, mcdose_particle_dmlc_destroy_source_session_context_v1);
    auto first = result();
    CHECK(mcdose_particle_dmlc_next_source_product_v1(
              direct_session.get(), &first, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(first.has_product == 1);
    CHECK(first.product_kind == MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY);
    CHECK(first.starts_new_history == 1);
    CHECK(first.latch == 19);
    CHECK(first.photon_history == 1);
    CHECK(first.z_last_cm == 42.0);
    CHECK(first.source_history_id == 41);
    CHECK(first.particle.history_id == 41);
    CHECK(first.particle.particle_id == 101);
    auto invalid_result = result();
    invalid_result.abi_version = 0;
    CHECK(mcdose_particle_dmlc_next_source_product_v1(
              direct_session.get(), &invalid_result, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH);
    auto second = result();
    CHECK(mcdose_particle_dmlc_next_source_product_v1(
              direct_session.get(), &second, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(second.has_product == 1);
    CHECK(second.starts_new_history == 0);
    CHECK(second.latch == 19);
    CHECK(second.photon_history == 1);
    CHECK(second.z_last_cm == 42.0);
    CHECK(second.source_history_id == 41);
    auto exhausted = result();
    CHECK(mcdose_particle_dmlc_next_source_product_v1(
              direct_session.get(), &exhausted, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(exhausted.has_product == 0);
    auto direct_summary = session_summary();
    CHECK(mcdose_particle_dmlc_get_source_session_summary_v1(
              direct_session.get(), &direct_summary, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(direct_summary.source_exhausted == 1);
    CHECK(direct_summary.incident_callback_count == 3);
    CHECK(direct_summary.incident_particle_count == 2);
    CHECK(direct_summary.observed_source_history_count == 1);
    CHECK(direct_summary.emitted_product_count == 2);
    CHECK(direct_summary.pass_through_product_count == 2);
    CHECK(direct_summary.producer_incident_count == 0);
    CHECK(direct_summary.last_source_history_id == 41);
    auto exhausted_again = result();
    CHECK(mcdose_particle_dmlc_next_source_product_v1(
              direct_session.get(), &exhausted_again, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(exhausted_again.has_product == 0);
    auto repeated_summary = session_summary();
    CHECK(mcdose_particle_dmlc_get_source_session_summary_v1(
              direct_session.get(), &repeated_summary, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(repeated_summary.incident_callback_count == 3);

    source_state batch = {{incident(41, 101, 1), incident(41, 102, 0)}};
    mcdose_particle_dmlc_source_session_context_v1 *batch_raw = nullptr;
    CHECK(mcdose_particle_dmlc_create_source_session_v1(
              &direct_config, next_incident, &batch, &batch_raw, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    session_pointer batch_session(
        batch_raw, mcdose_particle_dmlc_destroy_source_session_context_v1);
    std::array<mcdose_particle_dmlc_source_session_result_v1, 3> batch_results = {
        result(), result(), result()};
    uint32_t batch_count = 0;
    CHECK(mcdose_particle_dmlc_next_source_products_v1(
              batch_session.get(), batch_results.data(),
              static_cast<uint32_t>(batch_results.size()), &batch_count,
              diagnostic, sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(batch_count == 2);
    CHECK(batch_results[0].has_product == 1);
    CHECK(batch_results[0].particle.particle_id == 101);
    CHECK(batch_results[0].source_history_id == 41);
    CHECK(batch_results[0].starts_new_history == 1);
    CHECK(batch_results[0].latch == 19);
    CHECK(batch_results[0].photon_history == 1);
    CHECK(batch_results[0].z_last_cm == 42.0);
    CHECK(batch_results[1].has_product == 1);
    CHECK(batch_results[1].particle.particle_id == 102);
    CHECK(batch_results[1].source_history_id == 41);
    CHECK(batch_results[1].starts_new_history == 0);
    CHECK(batch_results[1].latch == 19);
    CHECK(batch_results[1].photon_history == 1);
    CHECK(batch_results[1].z_last_cm == 42.0);
    CHECK(batch_results[2].has_product == 0);
    auto mixed_result = result();
    CHECK(mcdose_particle_dmlc_next_source_product_v1(
              batch_session.get(), &mixed_result, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(std::strstr(diagnostic, "mix") != nullptr);

    source_state invalid_incident = {{incident(99, 201, 2)}};
    mcdose_particle_dmlc_source_session_context_v1 *invalid_session_raw = nullptr;
    CHECK(mcdose_particle_dmlc_create_source_session_v1(
              &direct_config, next_incident, &invalid_incident,
              &invalid_session_raw, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    session_pointer invalid_session(
        invalid_session_raw, mcdose_particle_dmlc_destroy_source_session_context_v1);
    auto invalid_incident_result = result();
    CHECK(mcdose_particle_dmlc_next_source_product_v1(
              invalid_session.get(), &invalid_incident_result, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(std::strstr(diagnostic, "flags") != nullptr);
    auto invalid_summary = session_summary();
    CHECK(mcdose_particle_dmlc_get_source_session_summary_v1(
              invalid_session.get(), &invalid_summary, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(invalid_summary.incident_callback_count == 1);
    CHECK(invalid_summary.incident_particle_count == 0);
    CHECK(invalid_summary.incident_rejection_count == 1);

    fixture values;
    mcdose_particle_dmlc_producer_config_v1 producer_config = {};
    producer_config.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    producer_config.struct_size = sizeof(producer_config);
    producer_config.mlc_device_index = 0;
    producer_config.generated_electron_policy =
        MCDOSE_PARTICLE_DMLC_GENERATED_ELECTRON_DISCARD;
    mcdose_particle_dmlc_producer_context_v1 *producer_raw = nullptr;
    CHECK(mcdose_particle_dmlc_create_producer_context_v1(
              &values.delivery, &values.machine, &producer_config, &producer_raw,
              diagnostic, sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    using producer_pointer = std::unique_ptr<
        mcdose_particle_dmlc_producer_context_v1,
        decltype(&mcdose_particle_dmlc_destroy_producer_context_v1)>;
    producer_pointer producer(
        producer_raw, mcdose_particle_dmlc_destroy_producer_context_v1);
    random_state random;
    CHECK(mcdose_particle_dmlc_set_producer_random_source_v1(
              producer.get(), next_random, &random, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    source_state transported = {{incident(77, 301, 1)}};
    transported.incidents[0].particle.position_cm[0] = 1.1;
    transported.incidents[0].particle.position_cm[1] = -0.5;
    auto transported_config = config(producer.get());
    mcdose_particle_dmlc_source_session_context_v1 *transported_raw = nullptr;
    CHECK(mcdose_particle_dmlc_create_source_session_v1(
              &transported_config, next_incident, &transported, &transported_raw,
              diagnostic, sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    session_pointer transported_session(
        transported_raw, mcdose_particle_dmlc_destroy_source_session_context_v1);
    auto transported_result = result();
    CHECK(mcdose_particle_dmlc_next_source_product_v1(
              transported_session.get(), &transported_result, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(transported_result.has_product == 1);
    CHECK(transported_result.product_kind == MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY);
    CHECK(transported_result.remaining_product_count == 1);
    CHECK(transported_result.starts_new_history == 1);
    CHECK(transported_result.latch == 19);
    CHECK(transported_result.photon_history == 1);
    CHECK(transported_result.z_last_cm == 42.0);
    CHECK(transported_result.source_history_id == 77);
    CHECK(transported_result.particle.history_id == 77);
    CHECK(transported_result.particle.particle_id == 301);
    CHECK(std::isfinite(transported_result.particle.weight));
    auto scattered_result = result();
    CHECK(mcdose_particle_dmlc_next_source_product_v1(
              transported_session.get(), &scattered_result, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(scattered_result.has_product == 1);
    CHECK(scattered_result.product_kind ==
          MCDOSE_PARTICLE_DMLC_PRODUCER_SCATTERED_PHOTON);
    CHECK(scattered_result.remaining_product_count == 0);
    CHECK(scattered_result.starts_new_history == 1);
    CHECK(scattered_result.latch == 19);
    CHECK(scattered_result.photon_history == 1);
    CHECK(scattered_result.z_last_cm == 42.0);
    CHECK(scattered_result.source_history_id == 77);
    CHECK(scattered_result.particle.history_id == 77);
    CHECK(scattered_result.particle.parent_particle_id == 301);
    auto transported_summary = session_summary();
    CHECK(mcdose_particle_dmlc_get_source_session_summary_v1(
              transported_session.get(), &transported_summary, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(transported_summary.source_exhausted == 0);
    CHECK(transported_summary.incident_callback_count == 1);
    CHECK(transported_summary.incident_particle_count == 1);
    CHECK(transported_summary.observed_source_history_count == 1);
    CHECK(transported_summary.emitted_product_count == 2);
    CHECK(transported_summary.pass_through_product_count == 0);
    CHECK(transported_summary.producer_incident_count == 1);
    CHECK(transported_summary.producer_blocked_incident_count == 0);
    CHECK(transported_summary.producer_retained_product_count == 2);
    CHECK(transported_summary.producer_primary_retained_count == 1);
    CHECK(transported_summary.producer_scattered_photon_retained_count == 1);
    CHECK(transported_summary.producer_generated_electron_discarded_count == 1);
    CHECK(transported_summary.producer_random_draw_count == random.next);
    CHECK(transported_summary.last_source_history_id == 77);

    mcdose_particle_dmlc_producer_context_v1 *batch_producer_raw = nullptr;
    CHECK(mcdose_particle_dmlc_create_producer_context_v1(
              &values.delivery, &values.machine, &producer_config,
              &batch_producer_raw, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    producer_pointer batch_producer(
        batch_producer_raw, mcdose_particle_dmlc_destroy_producer_context_v1);
    random_state batch_random;
    CHECK(mcdose_particle_dmlc_set_producer_random_source_v1(
              batch_producer.get(), next_random, &batch_random, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    source_state transported_batch = {{incident(77, 301, 1)}};
    transported_batch.incidents[0].particle.position_cm[0] = 1.1;
    transported_batch.incidents[0].particle.position_cm[1] = -0.5;
    auto transported_batch_config = config(batch_producer.get());
    mcdose_particle_dmlc_source_session_context_v1 *transported_batch_raw =
        nullptr;
    CHECK(mcdose_particle_dmlc_create_source_session_v1(
              &transported_batch_config, next_incident, &transported_batch,
              &transported_batch_raw, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    session_pointer transported_batch_session(
        transported_batch_raw,
        mcdose_particle_dmlc_destroy_source_session_context_v1);
    std::array<mcdose_particle_dmlc_source_session_result_v1, 2>
        transported_batch_results = {result(), result()};
    uint32_t transported_batch_count = 0;
    CHECK(mcdose_particle_dmlc_next_source_products_v1(
              transported_batch_session.get(), transported_batch_results.data(),
              static_cast<uint32_t>(transported_batch_results.size()),
              &transported_batch_count, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(transported_batch_count == 2);
    CHECK(transported_batch_results[0].has_product == 1);
    CHECK(transported_batch_results[0].product_kind ==
          transported_result.product_kind);
    CHECK(transported_batch_results[0].remaining_product_count ==
          transported_result.remaining_product_count);
    CHECK(transported_batch_results[0].source_history_id ==
          transported_result.source_history_id);
    CHECK(transported_batch_results[0].latch == transported_result.latch);
    CHECK(transported_batch_results[0].photon_history ==
          transported_result.photon_history);
    CHECK(transported_batch_results[0].z_last_cm ==
          transported_result.z_last_cm);
    CHECK(transported_batch_results[0].particle.history_id ==
          transported_result.particle.history_id);
    CHECK(transported_batch_results[0].particle.particle_id ==
          transported_result.particle.particle_id);
    CHECK(transported_batch_results[0].particle.energy_mev ==
          transported_result.particle.energy_mev);
    CHECK(transported_batch_results[0].particle.weight ==
          transported_result.particle.weight);
    CHECK(transported_batch_results[1].has_product == 1);
    CHECK(transported_batch_results[1].product_kind ==
          scattered_result.product_kind);
    CHECK(transported_batch_results[1].remaining_product_count ==
          scattered_result.remaining_product_count);
    CHECK(transported_batch_results[1].source_history_id ==
          scattered_result.source_history_id);
    CHECK(transported_batch_results[1].latch == scattered_result.latch);
    CHECK(transported_batch_results[1].photon_history ==
          scattered_result.photon_history);
    CHECK(transported_batch_results[1].z_last_cm ==
          scattered_result.z_last_cm);
    CHECK(transported_batch_results[1].particle.history_id ==
          scattered_result.particle.history_id);
    CHECK(transported_batch_results[1].particle.parent_particle_id ==
          scattered_result.particle.parent_particle_id);
    CHECK(transported_batch_results[1].particle.energy_mev ==
          scattered_result.particle.energy_mev);
    CHECK(transported_batch_results[1].particle.weight ==
          scattered_result.particle.weight);
    return 0;
}
