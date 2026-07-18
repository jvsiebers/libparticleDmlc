#include "mcdose_particle_dmlc_delivery_v1.h"

#include <cstdio>
#include <cstring>

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::fprintf(stderr, "check failed: %s at %s:%d\n", #condition,  \
                         __FILE__, __LINE__);                                  \
            return 1;                                                          \
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
    double meterset[4] = {0.0, 0.5, 0.5, 1.0};
    double gantry[4] = {0.0, 90.0, 90.0, 180.0};
    int32_t gantry_direction[4] = {2, 2, 2, 0};
    double collimator[4] = {15.0, 15.0, 15.0, 15.0};
    int32_t collimator_direction[4] = {0, 0, 0, 0};
    double support[4] = {0.0, 0.0, 0.0, 0.0};
    int32_t support_direction[4] = {0, 0, 0, 0};
    double isocenter[12] = {};
    double energy[4] = {6.0, 6.0, 6.0, 6.0};
    double dose_rate[4] = {600.0, 600.0, 600.0, 600.0};
    uint8_t dose_present[4] = {1, 1, 1, 1};
    double pitch[4] = {};
    double roll[4] = {};
    double bank_1[16] = {};
    double bank_2[16] = {};
    double boundaries[3] = {-1.0, 0.0, 1.0};
    mcdose_particle_dmlc_delivery_v1 delivery = {
        1,
        sizeof(mcdose_particle_dmlc_delivery_v1),
        1,
        3,
        4,
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
        gantry_direction,
        collimator,
        collimator_direction,
        support,
        support_direction,
        isocenter,
        energy,
        dose_rate,
        dose_present,
        pitch,
        roll,
        bank_1,
        bank_2,
        boundaries,
    };
};

int32_t validate(fixture &value, char *diagnostic, size_t capacity) {
    return mcdose_particle_dmlc_validate_delivery_v1(&value.delivery, diagnostic, capacity);
}
}  // namespace

int main() {
    char diagnostic[256];
    fixture valid;
    CHECK(validate(valid, diagnostic, sizeof(diagnostic)) == 0);
    CHECK(diagnostic[0] == '\0');

    for (size_t point = 0; point < 4; ++point) {
        for (size_t position = 0; position < 4; ++position) {
            valid.bank_1[point * 4 + position] =
                static_cast<double>(point * 10 + position);
            valid.bank_2[point * 4 + position] =
                -static_cast<double>(point * 10 + position);
        }
    }
    mcdose_particle_dmlc_delivery_context_v1 *context = nullptr;
    CHECK(mcdose_particle_dmlc_create_delivery_context_v1(
              &valid.delivery, &context, diagnostic, sizeof(diagnostic)) == 0);
    CHECK(context != nullptr);
    valid.bank_1[8] = 999.0;
    mcdose_particle_dmlc_sampled_state_v1 state = {};
    state.abi_version = 1;
    state.struct_size = sizeof(mcdose_particle_dmlc_sampled_state_v1);
    double bank_1[4];
    double bank_2[4];
    CHECK(mcdose_particle_dmlc_sample_delivery_v1(
              context, 0.5, &state, bank_1, bank_2, 4, diagnostic,
              sizeof(diagnostic)) == 0);
    CHECK(state.source_segment_index == 2);
    CHECK(state.interpolation_fraction == 0.0);
    CHECK(state.gantry_angle_deg == 90.0);
    CHECK(bank_1[0] == 20.0);
    CHECK(bank_2[3] == -23.0);

    CHECK(mcdose_particle_dmlc_sample_delivery_v1(
              context, 0.25, &state, bank_1, bank_2, 4, diagnostic,
              sizeof(diagnostic)) == 0);
    CHECK(state.source_segment_index == 0);
    CHECK(state.interpolation_fraction == 0.5);
    CHECK(state.gantry_angle_deg == 225.0);
    CHECK(bank_1[2] == 7.0);

    CHECK(mcdose_particle_dmlc_sample_delivery_v1(
              context, 1.0, &state, bank_1, bank_2, 4, diagnostic,
              sizeof(diagnostic)) == 0);
    CHECK(state.source_segment_index == 2);
    CHECK(state.interpolation_fraction == 1.0);
    CHECK(state.gantry_angle_deg == 180.0);
    CHECK(bank_1[0] == 30.0);

    CHECK(mcdose_particle_dmlc_sample_delivery_v1(
              context, 0.5, &state, bank_1, bank_2, 3, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY);
    CHECK(std::strstr(diagnostic, "capacity") != nullptr);
    CHECK(mcdose_particle_dmlc_sample_delivery_v1(
              context, -0.1, &state, bank_1, bank_2, 4, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT);
    mcdose_particle_dmlc_destroy_delivery_context_v1(context);
    mcdose_particle_dmlc_destroy_delivery_context_v1(nullptr);

    fixture abi;
    abi.delivery.abi_version = 2;
    CHECK(validate(abi, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH);
    CHECK(std::strstr(diagnostic, "ABI") != nullptr);

    fixture energy;
    energy.energy[1] = 10.0;
    CHECK(validate(energy, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(std::strstr(diagnostic, "v1 constant") != nullptr);

    fixture transition;
    transition.energy[2] = 10.0;
    transition.energy[3] = 10.0;
    CHECK(validate(transition, diagnostic, sizeof(diagnostic)) == 0);

    fixture pitch;
    pitch.pitch[1] = 1.0;
    CHECK(validate(pitch, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    fixture rotation;
    rotation.gantry_direction[0] = MCDOSE_PARTICLE_DMLC_ROTATION_NONE;
    CHECK(validate(rotation, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(std::strstr(diagnostic, "direction is NONE") != nullptr);

    fixture null_pointer;
    null_pointer.delivery.bank_1_positions_cm = nullptr;
    CHECK(validate(null_pointer, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT);

    CHECK(mcdose_particle_dmlc_validate_delivery_v1(nullptr, diagnostic,
                                                    sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT);
    return 0;
}
