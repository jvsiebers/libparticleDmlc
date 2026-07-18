#include "mcdose_particle_dmlc_interaction_v1.h"

#include <cmath>
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
struct machine_fixture {
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
    mcdose_particle_dmlc_machine_v1 machine = {};

    machine_fixture() {
        machine.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        machine.struct_size = sizeof(mcdose_particle_dmlc_machine_v1);
        machine.leaf_pair_count = 2;
        machine.section_count = 2;
        machine.section_row_count = 4;
        machine.attenuation_point_count = 2;
        machine.source_axis_distance_cm = 100.0;
        machine.source_to_device_distance_cm = 50.0;
        machine.position_specification_distance_cm = 100.0;
        machine.material_density_g_cm3 = 17.0;
        machine.calibration_mode = MCDOSE_PARTICLE_DMLC_LEAF_CALIBRATION_IDENTITY;
        machine.motion_axis = MCDOSE_PARTICLE_DMLC_AXIS_X;
        machine.projection_ratio = 2.0;
        machine.physical_leaf_offset_cm = 0.0;
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

struct aperture_fixture {
    double bank_1[2] = {-1.0, -2.0};
    double bank_2[2] = {1.0, 2.0};
    mcdose_particle_dmlc_aperture_v1 aperture = {
        MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION,
        sizeof(mcdose_particle_dmlc_aperture_v1),
        2,
        0,
        bank_1,
        bank_2,
    };
};

mcdose_particle_dmlc_ray_v1 ray(double x, double y) {
    mcdose_particle_dmlc_ray_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(mcdose_particle_dmlc_ray_v1);
    result.position_cm[0] = x;
    result.position_cm[1] = y;
    result.position_cm[2] = 45.0;
    result.direction[2] = 1.0;
    return result;
}

mcdose_particle_dmlc_ray_v1 sloped_ray(double x, double y, double dx_dz) {
    auto result = ray(x, y);
    const double norm = std::sqrt(1.0 + dx_dz * dx_dz);
    result.direction[0] = dx_dz / norm;
    result.direction[2] = 1.0 / norm;
    return result;
}

mcdose_particle_dmlc_classification_v1 classification() {
    mcdose_particle_dmlc_classification_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(mcdose_particle_dmlc_classification_v1);
    return result;
}
}  // namespace

int main() {
    static_assert(
        sizeof(mcdose_particle_dmlc_photon_material_attenuation_v1) == 80);
    static_assert(sizeof(mcdose_particle_dmlc_equivalent_primary_v1) == 232);
    static_assert(
        sizeof(mcdose_particle_dmlc_equivalent_scattered_photon_v1) == 216);
    char diagnostic[256];
    machine_fixture fixture;
    mcdose_particle_dmlc_machine_context_v1 *machine = nullptr;
    CHECK(mcdose_particle_dmlc_create_machine_context_v1(
              &fixture.machine, &machine, diagnostic, sizeof(diagnostic)) == 0);
    aperture_fixture aperture;

    auto open_ray = ray(0.0, -0.5);
    auto open = classification();
    CHECK(mcdose_particle_dmlc_classify_ray_v1(
              machine, &aperture.aperture, &open_ray, &open, diagnostic,
              sizeof(diagnostic)) == 0);
    CHECK(open.section_count == 2);
    CHECK(open.region[0] == MCDOSE_PARTICLE_DMLC_SECTION_OPEN);
    CHECK(open.region[1] == MCDOSE_PARTICLE_DMLC_SECTION_OPEN);
    CHECK(open.leaf_pair_index[0] == 0);
    CHECK(open.following_position_cm[0] == -0.5);
    CHECK(open.leading_position_cm[0] == 0.5);
    CHECK(open.thickness_resolved[0] == 1);
    CHECK(open.resolved_longitudinal_thickness_cm[0] == 0.0);

    auto leading_ray = ray(1.1, -0.5);
    auto leading = classification();
    CHECK(mcdose_particle_dmlc_classify_ray_v1(
              machine, &aperture.aperture, &leading_ray, &leading, diagnostic,
              sizeof(diagnostic)) == 0);
    CHECK(leading.region[0] == MCDOSE_PARTICLE_DMLC_SECTION_LEADING_FULL);
    CHECK(leading.region[1] == MCDOSE_PARTICLE_DMLC_SECTION_LEADING_FULL);
    CHECK(leading.resolved_longitudinal_thickness_cm[0] == 3.0);
    CHECK(leading.resolved_longitudinal_thickness_cm[1] == 3.2);

    auto following_ray = ray(-1.1, -0.5);
    auto following = classification();
    CHECK(mcdose_particle_dmlc_classify_ray_v1(
              machine, &aperture.aperture, &following_ray, &following,
              diagnostic, sizeof(diagnostic)) == 0);
    CHECK(following.region[0] == MCDOSE_PARTICLE_DMLC_SECTION_FOLLOWING_FULL);

    auto tip_ray = ray(0.75, -0.5);
    auto tip = classification();
    CHECK(mcdose_particle_dmlc_classify_ray_v1(
              machine, &aperture.aperture, &tip_ray, &tip, diagnostic,
              sizeof(diagnostic)) == 0);
    CHECK(tip.region[0] == MCDOSE_PARTICLE_DMLC_SECTION_TIP_OR_CROSSING);
    CHECK(tip.thickness_resolved[0] == 1);
    CHECK(tip.resolved_longitudinal_thickness_cm[0] > 0.0);
    CHECK(tip.resolved_longitudinal_thickness_cm[0] < 3.0);

    auto outside_ray = ray(0.0, 2.0);
    auto outside = classification();
    CHECK(mcdose_particle_dmlc_classify_ray_v1(
              machine, &aperture.aperture, &outside_ray, &outside, diagnostic,
              sizeof(diagnostic)) == 0);
    CHECK(outside.region[0] == MCDOSE_PARTICLE_DMLC_SECTION_OUTSIDE);
    CHECK(outside.leaf_pair_index[0] == UINT32_MAX);

    auto boundary_ray = ray(0.0, 0.0);
    auto boundary = classification();
    CHECK(mcdose_particle_dmlc_classify_ray_v1(
              machine, &aperture.aperture, &boundary_ray, &boundary, diagnostic,
              sizeof(diagnostic)) == 0);
    CHECK(boundary.leaf_pair_index[0] == 0);

    // The patient-side tip faces upstream, so its tip tests run from the
    // physical exit plane back toward the entrance, as in legacy transport.
    auto lower_tip_orientation_ray =
        sloped_ray(-0.23333333333333334, -0.5, 1.0 / 6.0);
    auto lower_tip_orientation = classification();
    CHECK(mcdose_particle_dmlc_classify_ray_v1(
              machine, &aperture.aperture, &lower_tip_orientation_ray,
              &lower_tip_orientation, diagnostic, sizeof(diagnostic)) == 0);
    CHECK(lower_tip_orientation.region[1] ==
          MCDOSE_PARTICLE_DMLC_SECTION_LEADING_FULL);
    CHECK(lower_tip_orientation.resolved_longitudinal_thickness_cm[1] == 3.2);

    mcdose_particle_dmlc_photon_attenuation_v1 attenuation = {};
    attenuation.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    attenuation.struct_size =
        sizeof(mcdose_particle_dmlc_photon_attenuation_v1);
    CHECK(mcdose_particle_dmlc_evaluate_photon_attenuation_v1(
              machine, &leading, 1.0, 1.0, &attenuation, diagnostic,
              sizeof(diagnostic)) == 0);
    CHECK(attenuation.total_longitudinal_thickness_cm == 6.2);
    CHECK(attenuation.total_path_length_cm == 6.2);
    CHECK(attenuation.total_linear_attenuation_per_cm == 1.0);
    CHECK(std::fabs(attenuation.incoherent_linear_attenuation_per_cm - 0.1) <
          1.0e-12);
    CHECK(std::fabs(attenuation.optical_depth - 6.2) < 1.0e-12);
    CHECK(std::fabs(attenuation.primary_transmission - std::exp(-6.2)) <
          1.0e-12);
    CHECK(std::fabs(attenuation.interaction_probability -
                    (1.0 - std::exp(-6.2))) < 1.0e-12);
    CHECK(std::fabs(attenuation.incoherent_interaction_fraction - 0.1) <
          1.0e-12);

    mcdose_particle_dmlc_particle_v1 primary = {};
    primary.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    primary.struct_size = sizeof(primary);
    primary.charge = MCDOSE_PARTICLE_DMLC_PHOTON;
    primary.history_id = 17;
    primary.particle_id = 23;
    primary.position_cm[2] = 45.0;
    primary.direction[2] = 1.0;
    primary.energy_mev = 1.0;
    primary.weight = 0.8;
    mcdose_particle_dmlc_equivalent_primary_v1 finalized_primary = {};
    finalized_primary.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    finalized_primary.struct_size = sizeof(finalized_primary);
    CHECK(mcdose_particle_dmlc_finalize_equivalent_primary_v1(
              machine, &leading, &primary, &finalized_primary, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(finalized_primary.has_particle == 1);
    CHECK(finalized_primary.total_longitudinal_thickness_cm == 6.2);
    CHECK(std::fabs(finalized_primary.photon_attenuation.primary_transmission -
                    std::exp(-6.2)) < 1.0e-12);
    CHECK(std::fabs(finalized_primary.particle.weight - 0.8 * std::exp(-6.2)) <
          1.0e-12);
    CHECK(finalized_primary.particle.position_cm[2] == 45.0);

    auto open_primary = finalized_primary;
    CHECK(mcdose_particle_dmlc_finalize_equivalent_primary_v1(
              machine, &open, &primary, &open_primary, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(open_primary.has_particle == 1);
    CHECK(open_primary.total_longitudinal_thickness_cm == 0.0);
    CHECK(open_primary.photon_attenuation.primary_transmission == 1.0);
    CHECK(open_primary.particle.weight == primary.weight);

    auto charged_primary = primary;
    charged_primary.charge = MCDOSE_PARTICLE_DMLC_ELECTRON;
    auto blocked_electron = finalized_primary;
    CHECK(mcdose_particle_dmlc_finalize_equivalent_primary_v1(
              machine, &leading, &charged_primary, &blocked_electron, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(blocked_electron.has_particle == 0);
    CHECK(blocked_electron.photon_attenuation.abi_version == 0);
    auto open_electron = finalized_primary;
    CHECK(mcdose_particle_dmlc_finalize_equivalent_primary_v1(
              machine, &open, &charged_primary, &open_electron, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(open_electron.has_particle == 1);
    CHECK(open_electron.particle.charge == MCDOSE_PARTICLE_DMLC_ELECTRON);
    CHECK(open_electron.particle.energy_mev == charged_primary.energy_mev);

    auto outside_primary = finalized_primary;
    CHECK(mcdose_particle_dmlc_finalize_equivalent_primary_v1(
              machine, &outside, &primary, &outside_primary, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(outside_primary.has_particle == 0);
    CHECK(outside_primary.photon_attenuation.abi_version == 0);

    auto backward_primary = primary;
    backward_primary.direction[2] = -1.0;
    auto invalid_primary = finalized_primary;
    CHECK(mcdose_particle_dmlc_finalize_equivalent_primary_v1(
              machine, &leading, &backward_primary, &invalid_primary,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    mcdose_particle_dmlc_photon_material_attenuation_v1 material = {};
    material.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    material.struct_size = sizeof(material);
    CHECK(mcdose_particle_dmlc_evaluate_photon_material_path_v1(
              machine, 1.0, 6.2, &material, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(material.energy_mev == 1.0);
    CHECK(material.material_path_length_cm == 6.2);
    CHECK(material.total_linear_attenuation_per_cm == 1.0);
    CHECK(std::fabs(material.incoherent_linear_attenuation_per_cm - 0.1) <
          1.0e-12);
    CHECK(std::fabs(material.optical_depth - 6.2) < 1.0e-12);
    CHECK(std::fabs(material.transmission - std::exp(-6.2)) < 1.0e-12);
    CHECK(std::fabs(material.interaction_probability -
                    (1.0 - std::exp(-6.2))) < 1.0e-12);
    CHECK(std::fabs(material.incoherent_interaction_fraction - 0.1) <
          1.0e-12);

    auto open_material = material;
    CHECK(mcdose_particle_dmlc_evaluate_photon_material_path_v1(
              machine, 0.55, 0.0, &open_material, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(open_material.material_path_length_cm == 0.0);
    CHECK(open_material.optical_depth == 0.0);
    CHECK(open_material.transmission == 1.0);
    CHECK(open_material.interaction_probability == 0.0);

    auto invalid_material = material;
    CHECK(mcdose_particle_dmlc_evaluate_photon_material_path_v1(
              machine, 1.0, -1.0, &invalid_material, diagnostic,
              sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    invalid_material.abi_version = 2;
    CHECK(mcdose_particle_dmlc_evaluate_photon_material_path_v1(
              machine, 1.0, 1.0, &invalid_material, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH);

    mcdose_particle_dmlc_particle_v1 scattered = {};
    scattered.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    scattered.struct_size = sizeof(scattered);
    scattered.charge = MCDOSE_PARTICLE_DMLC_PHOTON;
    scattered.history_id = 17;
    scattered.particle_id = 24;
    scattered.parent_particle_id = 23;
    scattered.position_cm[2] = 45.0;
    scattered.direction[0] = 0.6;
    scattered.direction[2] = 0.8;
    scattered.energy_mev = 1.0;
    scattered.weight = 0.3;
    mcdose_particle_dmlc_forced_interaction_v1 forced = {};
    forced.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    forced.struct_size = sizeof(forced);
    forced.random_draw_count = 1;
    forced.uniform_random = 0.5;
    forced.traversed_longitudinal_thickness_cm = 3.8;
    forced.remaining_longitudinal_thickness_cm = 2.4;
    forced.traversed_path_length_cm = 3.8;
    forced.remaining_path_length_cm = 2.4;
    forced.interaction_optical_depth = 3.8;
    mcdose_particle_dmlc_equivalent_scattered_photon_v1 finalized = {};
    finalized.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    finalized.struct_size = sizeof(finalized);
    CHECK(mcdose_particle_dmlc_finalize_equivalent_scattered_photon_v1(
              machine, &leading, &scattered, &forced, &finalized, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(finalized.has_particle == 1);
    CHECK(std::fabs(finalized.material_attenuation.material_path_length_cm -
                    3.0) < 1.0e-12);
    CHECK(std::fabs(finalized.material_attenuation.transmission -
                    std::exp(-3.0)) < 1.0e-12);
    CHECK(std::fabs(finalized.reference_plane_displacement_cm + 4.4) <
          1.0e-12);
    CHECK(std::fabs(finalized.particle.position_cm[0] + 3.3) < 1.0e-12);
    CHECK(finalized.particle.position_cm[2] == 45.0);
    CHECK(std::fabs(finalized.particle.weight - 0.3 * std::exp(-3.0)) <
          1.0e-12);
    CHECK(finalized.particle.particle_id == scattered.particle_id);

    auto backward_scattered = scattered;
    backward_scattered.direction[0] = -0.6;
    backward_scattered.direction[2] = -0.8;
    auto backward_finalized = finalized;
    CHECK(mcdose_particle_dmlc_finalize_equivalent_scattered_photon_v1(
              machine, &leading, &backward_scattered, &forced,
              &backward_finalized, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(backward_finalized.has_particle == 0);
    CHECK(backward_finalized.material_attenuation.abi_version == 0);

    auto opaque_scattered = scattered;
    opaque_scattered.direction[0] = std::sqrt(1.0 - 1.0e-8);
    opaque_scattered.direction[2] = 1.0e-4;
    auto opaque_finalized = finalized;
    CHECK(mcdose_particle_dmlc_finalize_equivalent_scattered_photon_v1(
              machine, &leading, &opaque_scattered, &forced,
              &opaque_finalized, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(opaque_finalized.has_particle == 0);
    CHECK(opaque_finalized.material_attenuation.transmission == 0.0);

    auto unresolved = leading;
    unresolved.thickness_resolved[0] = 0;
    auto invalid_finalized = finalized;
    CHECK(mcdose_particle_dmlc_finalize_equivalent_scattered_photon_v1(
              machine, &unresolved, &scattered, &forced, &invalid_finalized,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    unresolved = leading;
    unresolved.thickness_resolved[1] = 0;
    CHECK(mcdose_particle_dmlc_finalize_equivalent_scattered_photon_v1(
              machine, &unresolved, &scattered, &forced, &invalid_finalized,
              diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    auto inconsistent_forced = forced;
    inconsistent_forced.remaining_longitudinal_thickness_cm = 2.3;
    CHECK(mcdose_particle_dmlc_finalize_equivalent_scattered_photon_v1(
              machine, &leading, &scattered, &inconsistent_forced,
              &invalid_finalized, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    auto open_attenuation = attenuation;
    CHECK(mcdose_particle_dmlc_evaluate_photon_attenuation_v1(
              machine, &open, 0.55, 0.5, &open_attenuation, diagnostic,
              sizeof(diagnostic)) == 0);
    CHECK(open_attenuation.total_path_length_cm == 0.0);
    CHECK(open_attenuation.primary_transmission == 1.0);
    CHECK(open_attenuation.interaction_probability == 0.0);

    auto outside_attenuation = attenuation;
    CHECK(mcdose_particle_dmlc_evaluate_photon_attenuation_v1(
              machine, &outside, 1.0, 1.0, &outside_attenuation, diagnostic,
              sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    auto invalid_ray = ray(0.0, 0.0);
    invalid_ray.direction[2] = 0.5;
    auto invalid = classification();
    CHECK(mcdose_particle_dmlc_classify_ray_v1(
              machine, &aperture.aperture, &invalid_ray, &invalid, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);

    aperture.aperture.leaf_pair_count = 1;
    CHECK(mcdose_particle_dmlc_classify_ray_v1(
              machine, &aperture.aperture, &open_ray, &invalid, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    mcdose_particle_dmlc_destroy_machine_context_v1(machine);

    machine_fixture negative_fixture;
    negative_fixture.thickness[0] = 0.05;
    negative_fixture.groove_thickness[0] = 0.1;
    mcdose_particle_dmlc_machine_context_v1 *negative_machine = nullptr;
    CHECK(mcdose_particle_dmlc_create_machine_context_v1(
              &negative_fixture.machine, &negative_machine, diagnostic,
              sizeof(diagnostic)) == 0);
    aperture_fixture negative_aperture;
    auto negative_ray = ray(1.1, -0.5);
    auto negative = classification();
    CHECK(mcdose_particle_dmlc_classify_ray_v1(
              negative_machine, &negative_aperture.aperture, &negative_ray,
              &negative, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    mcdose_particle_dmlc_destroy_machine_context_v1(negative_machine);
    return 0;
}
