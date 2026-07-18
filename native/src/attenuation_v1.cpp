#include "mcdose_particle_dmlc_transport_v1.h"

#include "context_v1.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {
constexpr double direction_tolerance = 1.0e-10;

int32_t fail(int32_t status, char *diagnostic, size_t capacity, const char *message) {
    if (diagnostic != nullptr && capacity > 0) {
        std::snprintf(diagnostic, capacity, "%s", message);
    }
    return status;
}

bool interpolate(const std::vector<double> &energy,
                 const std::vector<double> &values,
                 double requested,
                 double *result) {
    if (requested < energy.front()) {
        return false;
    }
    size_t start = 0;
    size_t stop = 1;
    if (requested >= energy.back()) {
        stop = energy.size() - 1;
        start = stop - 1;
    } else {
        const auto upper = std::upper_bound(energy.begin(), energy.end(), requested);
        stop = static_cast<size_t>(upper - energy.begin());
        start = stop - 1;
    }
    const double fraction =
        (requested - energy[start]) / (energy[stop] - energy[start]);
    *result = values[start] + fraction * (values[stop] - values[start]);
    return std::isfinite(*result) && *result >= 0.0;
}

bool evaluate_material_path(
    const mcdose_particle_dmlc_machine_context_v1 *machine,
    double energy_mev,
    double material_path_length_cm,
    double *total_coefficient,
    double *incoherent_coefficient,
    double *optical_depth,
    double *transmission,
    double *interaction_probability,
    double *incoherent_fraction) {
    if (!std::isfinite(energy_mev) || energy_mev <= 0.0 ||
        !std::isfinite(material_path_length_cm) ||
        material_path_length_cm < 0.0 ||
        !interpolate(machine->attenuation_energy_mev,
                     machine->total_linear_attenuation_per_cm, energy_mev,
                     total_coefficient) ||
        !interpolate(machine->attenuation_energy_mev,
                     machine->incoherent_linear_attenuation_per_cm, energy_mev,
                     incoherent_coefficient) ||
        *incoherent_coefficient > *total_coefficient) {
        return false;
    }
    *optical_depth = *total_coefficient * material_path_length_cm;
    if (!std::isfinite(*optical_depth)) {
        return false;
    }
    *transmission = std::exp(-*optical_depth);
    *interaction_probability = -std::expm1(-*optical_depth);
    *incoherent_fraction = *total_coefficient == 0.0
                               ? 0.0
                               : *incoherent_coefficient / *total_coefficient;
    return true;
}
}  // namespace

extern "C" int32_t mcdose_particle_dmlc_evaluate_photon_attenuation_v1(
    const mcdose_particle_dmlc_machine_context_v1 *machine,
    const mcdose_particle_dmlc_classification_v1 *classification,
    double energy_mev,
    double direction_z,
    mcdose_particle_dmlc_photon_attenuation_v1 *attenuation,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (diagnostic != nullptr && diagnostic_capacity > 0) {
        diagnostic[0] = '\0';
    }
    if (machine == nullptr || classification == nullptr || attenuation == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity, "attenuation call contains a null pointer");
    }
    if (classification->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        classification->struct_size <
            sizeof(mcdose_particle_dmlc_classification_v1) ||
        attenuation->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        attenuation->struct_size <
            sizeof(mcdose_particle_dmlc_photon_attenuation_v1)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity, "attenuation ABI version or size differs");
    }
    if (classification->section_count != 2 || classification->reserved != 0 ||
        !std::isfinite(energy_mev) || !std::isfinite(direction_z) || energy_mev <= 0 ||
        direction_z <= 0 || direction_z > 1.0 + direction_tolerance) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, "attenuation inputs are invalid");
    }
    double longitudinal_thickness = 0.0;
    for (size_t section = 0; section < 2; ++section) {
        const int32_t region = classification->region[section];
        const double thickness =
            classification->resolved_longitudinal_thickness_cm[section];
        if (classification->reserved_2[section] != 0 ||
            classification->thickness_resolved[section] != 1 ||
            region < MCDOSE_PARTICLE_DMLC_SECTION_OPEN ||
            region > MCDOSE_PARTICLE_DMLC_SECTION_OUTSIDE ||
            region == MCDOSE_PARTICLE_DMLC_SECTION_OUTSIDE ||
            !std::isfinite(thickness) || thickness < 0) {
            return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                        diagnostic_capacity,
                        "attenuation requires resolved in-field section geometry");
        }
        longitudinal_thickness += thickness;
    }
    const double path_length = longitudinal_thickness / direction_z;
    double total_coefficient = 0.0;
    double incoherent_coefficient = 0.0;
    double optical_depth = 0.0;
    double primary_transmission = 0.0;
    double interaction_probability = 0.0;
    double incoherent_fraction = 0.0;
    if (!evaluate_material_path(machine, energy_mev, path_length,
                                &total_coefficient, &incoherent_coefficient,
                                &optical_depth, &primary_transmission,
                                &interaction_probability,
                                &incoherent_fraction)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "attenuation energy, path, or coefficient is invalid");
    }

    attenuation->section_count = 2;
    attenuation->reserved = 0;
    attenuation->energy_mev = energy_mev;
    attenuation->direction_z = direction_z;
    attenuation->total_longitudinal_thickness_cm = longitudinal_thickness;
    attenuation->total_path_length_cm = path_length;
    attenuation->total_linear_attenuation_per_cm = total_coefficient;
    attenuation->incoherent_linear_attenuation_per_cm = incoherent_coefficient;
    attenuation->optical_depth = optical_depth;
    attenuation->primary_transmission = primary_transmission;
    attenuation->interaction_probability = interaction_probability;
    attenuation->incoherent_interaction_fraction = incoherent_fraction;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" int32_t mcdose_particle_dmlc_evaluate_photon_material_path_v1(
    const mcdose_particle_dmlc_machine_context_v1 *machine,
    double energy_mev,
    double material_path_length_cm,
    mcdose_particle_dmlc_photon_material_attenuation_v1 *attenuation,
    char *diagnostic,
    size_t diagnostic_capacity) {
    if (diagnostic != nullptr && diagnostic_capacity > 0) {
        diagnostic[0] = '\0';
    }
    if (machine == nullptr || attenuation == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "material attenuation call contains a null pointer");
    }
    if (attenuation->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        attenuation->struct_size <
            sizeof(mcdose_particle_dmlc_photon_material_attenuation_v1)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "material attenuation ABI version or size differs");
    }
    double total_coefficient = 0.0;
    double incoherent_coefficient = 0.0;
    double optical_depth = 0.0;
    double transmission = 0.0;
    double interaction_probability = 0.0;
    double incoherent_fraction = 0.0;
    if (!evaluate_material_path(machine, energy_mev, material_path_length_cm,
                                &total_coefficient, &incoherent_coefficient,
                                &optical_depth, &transmission,
                                &interaction_probability,
                                &incoherent_fraction)) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "material attenuation energy, path, or coefficient is invalid");
    }
    attenuation->reserved[0] = 0;
    attenuation->reserved[1] = 0;
    attenuation->energy_mev = energy_mev;
    attenuation->material_path_length_cm = material_path_length_cm;
    attenuation->total_linear_attenuation_per_cm = total_coefficient;
    attenuation->incoherent_linear_attenuation_per_cm = incoherent_coefficient;
    attenuation->optical_depth = optical_depth;
    attenuation->transmission = transmission;
    attenuation->interaction_probability = interaction_probability;
    attenuation->incoherent_interaction_fraction = incoherent_fraction;
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}
