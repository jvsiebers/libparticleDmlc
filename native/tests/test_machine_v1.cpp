#include "mcdose_particle_dmlc_machine_v1.h"

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
    double energy[2] = {0.1, 1.0};
    double total_attenuation[2] = {10.0, 1.0};
    double incoherent_attenuation[2] = {1.0, 0.1};
    double calibration_specified[2] = {-1.0, 1.0};
    double calibration_actual[2] = {-1.1, 1.1};
    mcdose_particle_dmlc_machine_v1 machine = {};

    fixture() {
        machine.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        machine.struct_size = sizeof(mcdose_particle_dmlc_machine_v1);
        machine.leaf_pair_count = 2;
        machine.section_count = 2;
        machine.section_row_count = 4;
        machine.attenuation_point_count = 2;
        machine.calibration_point_count = 2;
        machine.source_axis_distance_cm = 100.0;
        machine.source_to_device_distance_cm = 50.0;
        machine.position_specification_distance_cm = 100.0;
        machine.material_density_g_cm3 = 17.0;
        machine.calibration_mode =
            MCDOSE_PARTICLE_DMLC_LEAF_CALIBRATION_PIECEWISE_LINEAR;
        machine.motion_axis = MCDOSE_PARTICLE_DMLC_AXIS_X;
        machine.touching_pairs_bypass_curve = 1;
        machine.projection_ratio = 2.0;
        machine.physical_leaf_offset_cm = 0.001;
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
        machine.calibration_specified_position_cm = calibration_specified;
        machine.calibration_actual_projected_position_cm = calibration_actual;
    }
};

int32_t validate(fixture &value, char *diagnostic, size_t capacity) {
    return mcdose_particle_dmlc_validate_machine_v1(&value.machine, diagnostic,
                                                     capacity);
}
}  // namespace

int main() {
    char diagnostic[256];
    fixture valid;
    CHECK(validate(valid, diagnostic, sizeof(diagnostic)) == 0);
    CHECK(diagnostic[0] == '\0');

    mcdose_particle_dmlc_machine_context_v1 *context = nullptr;
    CHECK(mcdose_particle_dmlc_create_machine_context_v1(
              &valid.machine, &context, diagnostic, sizeof(diagnostic)) == 0);
    CHECK(context != nullptr);
    valid.thickness[0] = 999.0;
    mcdose_particle_dmlc_destroy_machine_context_v1(context);
    mcdose_particle_dmlc_destroy_machine_context_v1(nullptr);

    fixture abi;
    abi.machine.abi_version = 2;
    CHECK(validate(abi, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH);
    CHECK(std::strstr(diagnostic, "ABI") != nullptr);

    fixture planes;
    planes.section_enter[1] = 50.1;
    CHECK(validate(planes, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(std::strstr(diagnostic, "contiguous") != nullptr);

    fixture offsets;
    offsets.row_offsets[1] = 0;
    CHECK(validate(offsets, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    fixture intervals;
    intervals.y_start[1] = 0.1;
    CHECK(validate(intervals, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    fixture identity;
    identity.machine.calibration_mode =
        MCDOSE_PARTICLE_DMLC_LEAF_CALIBRATION_IDENTITY;
    CHECK(validate(identity, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    fixture axis;
    axis.machine.motion_axis = 0;
    CHECK(validate(axis, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    fixture attenuation;
    attenuation.incoherent_attenuation[0] = attenuation.total_attenuation[0] + 0.1;
    CHECK(validate(attenuation, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    fixture null_array;
    null_array.machine.section_y_end_cm = nullptr;
    CHECK(validate(null_array, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT);

    CHECK(mcdose_particle_dmlc_validate_machine_v1(nullptr, diagnostic,
                                                   sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT);
    CHECK(mcdose_particle_dmlc_create_machine_context_v1(
              &valid.machine, nullptr, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT);
    return 0;
}
