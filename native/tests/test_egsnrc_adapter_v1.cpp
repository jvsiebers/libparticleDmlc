#include "mcdose_particle_dmlc_egsnrc_adapter_v1.h"

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::fprintf(stderr, "check failed: %s at %s:%d\n", #condition,  \
                         __FILE__, __LINE__);                                  \
            return 1;                                                          \
        }                                                                      \
    } while (false)

namespace {
constexpr double rest_mass_mev =
    MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV;

mcdose_particle_dmlc_particle_v1 particle(uint64_t particle_id,
                                          int32_t charge) {
    mcdose_particle_dmlc_particle_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    result.charge = charge;
    result.history_id = 17;
    result.particle_id = particle_id;
    result.parent_particle_id = 23;
    result.position_cm[0] = 1.25;
    result.position_cm[1] = -0.5;
    result.position_cm[2] = 40.5;
    result.direction[0] = 0.6;
    result.direction[2] = 0.8;
    result.energy_mev = charge == MCDOSE_PARTICLE_DMLC_PHOTON ? 1.5 : 4.5;
    result.weight = 0.25;
    return result;
}

mcdose_particle_dmlc_egsnrc_stack_entry_v1 entry_result() {
    mcdose_particle_dmlc_egsnrc_stack_entry_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}

mcdose_particle_dmlc_particle_v1 particle_result() {
    mcdose_particle_dmlc_particle_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}

mcdose_particle_dmlc_egsnrc_stack_result_v1 stack_result() {
    mcdose_particle_dmlc_egsnrc_stack_result_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}

mcdose_particle_dmlc_egsnrc_dispatch_result_v1 dispatch_result() {
    mcdose_particle_dmlc_egsnrc_dispatch_result_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    return result;
}

struct callback_capture {
    uint32_t call_count = 0;
    int32_t status = MCDOSE_PARTICLE_DMLC_STATUS_OK;
    mcdose_particle_dmlc_egsnrc_stack_entry_v1 entry = {};
};

int32_t shower_callback(
    void *user_data,
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 *entry) {
    auto *capture = static_cast<callback_capture *>(user_data);
    ++capture->call_count;
    capture->entry = *entry;
    return capture->status;
}
}  // namespace

int main() {
    static_assert(sizeof(mcdose_particle_dmlc_egsnrc_stack_entry_v1) == 120);
    static_assert(offsetof(mcdose_particle_dmlc_egsnrc_stack_entry_v1, iq) == 8);
    static_assert(offsetof(mcdose_particle_dmlc_egsnrc_stack_entry_v1, region) ==
                  12);
    static_assert(offsetof(mcdose_particle_dmlc_egsnrc_stack_entry_v1, latch) ==
                  16);
    static_assert(offsetof(mcdose_particle_dmlc_egsnrc_stack_entry_v1,
                           history_id) == 24);
    static_assert(offsetof(mcdose_particle_dmlc_egsnrc_stack_entry_v1,
                           total_energy_mev) == 56);
    static_assert(offsetof(mcdose_particle_dmlc_egsnrc_stack_entry_v1,
                           position_cm) == 64);
    static_assert(offsetof(mcdose_particle_dmlc_egsnrc_stack_entry_v1,
                           direction) == 88);
    static_assert(offsetof(mcdose_particle_dmlc_egsnrc_stack_entry_v1, weight) ==
                  112);
    static_assert(sizeof(mcdose_particle_dmlc_egsnrc_stack_result_v1) == 136);
    static_assert(sizeof(mcdose_particle_dmlc_egsnrc_dispatch_result_v1) == 24);

    char diagnostic[256];
    const auto photon = particle(24, MCDOSE_PARTICLE_DMLC_PHOTON);
    const auto electron = particle(25, MCDOSE_PARTICLE_DMLC_ELECTRON);
    const auto positron = particle(26, MCDOSE_PARTICLE_DMLC_POSITRON);

    auto photon_entry = entry_result();
    CHECK(mcdose_particle_dmlc_particle_to_egsnrc_stack_entry_v1(
              &photon, 7, -17, rest_mass_mev, &photon_entry, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(photon_entry.iq == 0);
    CHECK(photon_entry.region == 7);
    CHECK(photon_entry.latch == -17);
    CHECK(photon_entry.total_energy_mev == photon.energy_mev);
    CHECK(photon_entry.history_id == photon.history_id);

    auto electron_entry = entry_result();
    CHECK(mcdose_particle_dmlc_particle_to_egsnrc_stack_entry_v1(
              &electron, 8, 99, rest_mass_mev, &electron_entry, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(electron_entry.iq == -1);
    CHECK(electron_entry.total_energy_mev == electron.energy_mev + rest_mass_mev);

    auto positron_entry = entry_result();
    CHECK(mcdose_particle_dmlc_particle_to_egsnrc_stack_entry_v1(
              &positron, 9, 100, rest_mass_mev, &positron_entry, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(positron_entry.iq == 1);
    CHECK(positron_entry.total_energy_mev == positron.energy_mev + rest_mass_mev);

    callback_capture capture;
    auto primary_dispatch = dispatch_result();
    CHECK(mcdose_particle_dmlc_dispatch_egsnrc_product_v1(
              &electron_entry, MCDOSE_PARTICLE_DMLC_EGSNRC_PRIMARY,
              rest_mass_mev, shower_callback, &capture, &primary_dispatch,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(primary_dispatch.dispatched == 1);
    CHECK(primary_dispatch.discarded_by_policy == 0);
    CHECK(primary_dispatch.product_kind ==
          MCDOSE_PARTICLE_DMLC_EGSNRC_PRIMARY);
    CHECK(capture.call_count == 1);
    CHECK(capture.entry.latch == electron_entry.latch);

    auto scattered_dispatch = dispatch_result();
    CHECK(mcdose_particle_dmlc_dispatch_egsnrc_product_v1(
              &photon_entry, MCDOSE_PARTICLE_DMLC_EGSNRC_SCATTERED_PHOTON,
              rest_mass_mev, shower_callback, &capture, &scattered_dispatch,
              diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(scattered_dispatch.dispatched == 1);
    CHECK(capture.call_count == 2);

    auto generated_electron_dispatch = dispatch_result();
    CHECK(mcdose_particle_dmlc_dispatch_egsnrc_product_v1(
              &electron_entry,
              MCDOSE_PARTICLE_DMLC_EGSNRC_GENERATED_COMPTON_ELECTRON,
              rest_mass_mev, shower_callback, &capture,
              &generated_electron_dispatch,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(generated_electron_dispatch.dispatched == 0);
    CHECK(generated_electron_dispatch.discarded_by_policy == 1);
    CHECK(capture.call_count == 2);

    auto inconsistent_dispatch = dispatch_result();
    CHECK(mcdose_particle_dmlc_dispatch_egsnrc_product_v1(
              &electron_entry,
              MCDOSE_PARTICLE_DMLC_EGSNRC_SCATTERED_PHOTON,
              rest_mass_mev, shower_callback, &capture, &inconsistent_dispatch,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(capture.call_count == 2);

    auto rest_only_electron = electron_entry;
    rest_only_electron.total_energy_mev = rest_mass_mev;
    auto rest_only_dispatch = dispatch_result();
    CHECK(mcdose_particle_dmlc_dispatch_egsnrc_product_v1(
              &rest_only_electron, MCDOSE_PARTICLE_DMLC_EGSNRC_PRIMARY,
              rest_mass_mev, shower_callback, &capture, &rest_only_dispatch,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(capture.call_count == 2);

    capture.status = 999;
    auto failed_dispatch = dispatch_result();
    CHECK(mcdose_particle_dmlc_dispatch_egsnrc_product_v1(
              &photon_entry, MCDOSE_PARTICLE_DMLC_EGSNRC_PRIMARY,
              rest_mass_mev, shower_callback, &capture, &failed_dispatch,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(failed_dispatch.dispatched == 0);
    CHECK(capture.call_count == 3);
    capture.status = MCDOSE_PARTICLE_DMLC_STATUS_OK;

    auto round_trip = particle_result();
    CHECK(mcdose_particle_dmlc_egsnrc_stack_entry_to_particle_v1(
              &electron_entry, rest_mass_mev, &round_trip, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(round_trip.charge == electron.charge);
    CHECK(round_trip.history_id == electron.history_id);
    CHECK(round_trip.particle_id == electron.particle_id);
    CHECK(round_trip.parent_particle_id == electron.parent_particle_id);
    CHECK(round_trip.energy_mev == electron.energy_mev);
    CHECK(round_trip.position_cm[0] == electron.position_cm[0]);
    CHECK(round_trip.direction[2] == electron.direction[2]);
    CHECK(round_trip.weight == electron.weight);

    mcdose_particle_dmlc_egsnrc_stack_context_v1 *context = nullptr;
    CHECK(mcdose_particle_dmlc_create_egsnrc_stack_context_v1(
              2, rest_mass_mev, &context, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(context != nullptr);
    CHECK(mcdose_particle_dmlc_push_egsnrc_stack_entry_v1(
              context, &photon_entry, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(mcdose_particle_dmlc_push_egsnrc_stack_entry_v1(
              context, &electron_entry, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(mcdose_particle_dmlc_push_egsnrc_stack_entry_v1(
              context, &positron_entry, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY);

    auto first = stack_result();
    CHECK(mcdose_particle_dmlc_pop_egsnrc_stack_entry_v1(
              context, &first, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(first.has_entry == 1);
    CHECK(first.remaining_entry_count == 1);
    CHECK(first.entry.particle_id == electron.particle_id);

    auto second = stack_result();
    CHECK(mcdose_particle_dmlc_pop_egsnrc_stack_entry_v1(
              context, &second, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(second.has_entry == 1);
    CHECK(second.remaining_entry_count == 0);
    CHECK(second.entry.particle_id == photon.particle_id);

    auto empty = stack_result();
    CHECK(mcdose_particle_dmlc_pop_egsnrc_stack_entry_v1(
              context, &empty, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(empty.has_entry == 0);
    CHECK(empty.remaining_entry_count == 0);

    mcdose_particle_dmlc_egsnrc_stack_context_v1 *isolated = nullptr;
    CHECK(mcdose_particle_dmlc_create_egsnrc_stack_context_v1(
              1, rest_mass_mev, &isolated, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(mcdose_particle_dmlc_push_egsnrc_stack_entry_v1(
              isolated, &positron_entry, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    auto isolated_result = stack_result();
    CHECK(mcdose_particle_dmlc_pop_egsnrc_stack_entry_v1(
              isolated, &isolated_result, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(isolated_result.entry.particle_id == positron.particle_id);

    auto invalid_particle = electron;
    invalid_particle.direction[2] = 0.5;
    auto unchanged_entry = entry_result();
    CHECK(mcdose_particle_dmlc_particle_to_egsnrc_stack_entry_v1(
              &invalid_particle, 8, 99, rest_mass_mev, &unchanged_entry,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(unchanged_entry.iq == 0);
    CHECK(mcdose_particle_dmlc_particle_to_egsnrc_stack_entry_v1(
              &electron, 0, 99, rest_mass_mev, &unchanged_entry, diagnostic,
              sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(mcdose_particle_dmlc_particle_to_egsnrc_stack_entry_v1(
              &electron, 8, 99, std::numeric_limits<double>::quiet_NaN(),
              &unchanged_entry, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    auto rest_only = electron_entry;
    rest_only.total_energy_mev = rest_mass_mev;
    CHECK(mcdose_particle_dmlc_push_egsnrc_stack_entry_v1(
              context, &rest_only, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    auto unchanged_particle = particle_result();
    CHECK(mcdose_particle_dmlc_egsnrc_stack_entry_to_particle_v1(
              &rest_only, rest_mass_mev, &unchanged_particle, diagnostic,
              sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(unchanged_particle.history_id == 0);

    mcdose_particle_dmlc_destroy_egsnrc_stack_context_v1(isolated);
    mcdose_particle_dmlc_destroy_egsnrc_stack_context_v1(context);
    mcdose_particle_dmlc_destroy_egsnrc_stack_context_v1(nullptr);
    context = nullptr;
    CHECK(mcdose_particle_dmlc_create_egsnrc_stack_context_v1(
              0, rest_mass_mev, &context, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(context == nullptr);
    CHECK(mcdose_particle_dmlc_create_egsnrc_stack_context_v1(
              2, std::numeric_limits<double>::quiet_NaN(), &context, diagnostic,
              sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    return 0;
}
