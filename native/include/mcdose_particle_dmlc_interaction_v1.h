#ifndef MCDOSE_PARTICLE_DMLC_INTERACTION_V1_H
#define MCDOSE_PARTICLE_DMLC_INTERACTION_V1_H

#include "mcdose_particle_dmlc_transport_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mcdose_particle_dmlc_forced_interaction_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t random_draw_count;
    uint32_t reserved;
    double uniform_random;
    double traversed_longitudinal_thickness_cm;
    double remaining_longitudinal_thickness_cm;
    double traversed_path_length_cm;
    double remaining_path_length_cm;
    double interaction_optical_depth;
} mcdose_particle_dmlc_forced_interaction_v1;

/*
 * One exact material interval along a normalized input ray. Distances are cm
 * from the ray origin. Intervals supplied to the locator must be ordered and
 * non-overlapping; gaps represent non-material geometry.
 */
typedef struct mcdose_particle_dmlc_material_interval_v1 {
    double ray_path_start_cm;
    double ray_path_end_cm;
} mcdose_particle_dmlc_material_interval_v1;

typedef struct mcdose_particle_dmlc_interaction_site_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t interval_index;
    uint32_t interval_count;
    double material_path_before_interval_cm;
    double material_path_within_interval_cm;
    double traversed_material_path_cm;
    double remaining_material_path_cm;
    double ray_path_from_origin_cm;
    double position_cm[3];
} mcdose_particle_dmlc_interaction_site_v1;

enum mcdose_particle_dmlc_charge_v1 {
    MCDOSE_PARTICLE_DMLC_ELECTRON = -1,
    MCDOSE_PARTICLE_DMLC_PHOTON = 0,
    MCDOSE_PARTICLE_DMLC_POSITRON = 1
};

typedef struct mcdose_particle_dmlc_particle_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    int32_t charge;
    uint32_t reserved;
    uint64_t history_id;
    uint64_t particle_id;
    uint64_t parent_particle_id;
    uint64_t reserved_2;
    double position_cm[3];
    double direction[3];
    double energy_mev;
    double weight;
} mcdose_particle_dmlc_particle_v1;

/*
 * Primary particle after one commissioned equivalent-thickness classification.
 * Photons retain the evaluated attenuation record. Charged particles pass only
 * when the resolved longitudinal thickness is exactly zero, matching the
 * commissioned incident-electron policy. Outside particles return empty.
 */
typedef struct mcdose_particle_dmlc_equivalent_primary_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t has_particle;
    uint32_t reserved;
    double total_longitudinal_thickness_cm;
    mcdose_particle_dmlc_photon_attenuation_v1 photon_attenuation;
    mcdose_particle_dmlc_particle_v1 particle;
} mcdose_particle_dmlc_equivalent_primary_v1;

typedef struct mcdose_particle_dmlc_compton_products_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t random_draw_count;
    uint32_t rejection_count;
    uint32_t product_count;
    uint32_t reserved[3];
    mcdose_particle_dmlc_particle_v1 scattered_photon;
    mcdose_particle_dmlc_particle_v1 electron;
} mcdose_particle_dmlc_compton_products_v1;

typedef struct mcdose_particle_dmlc_propagated_photon_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t has_particle;
    uint32_t reserved;
    double geometric_ray_path_cm;
    mcdose_particle_dmlc_particle_v1 particle;
} mcdose_particle_dmlc_propagated_photon_v1;

/*
 * Final scattered-photon representation for the particleDmlc equivalent-
 * thickness model. The particle remains on its input phase-space reference
 * plane; x/y are projected using the historical infinitely-thin-MLC model.
 */
typedef struct mcdose_particle_dmlc_equivalent_scattered_photon_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t has_particle;
    uint32_t reserved;
    double reference_plane_displacement_cm;
    mcdose_particle_dmlc_photon_material_attenuation_v1 material_attenuation;
    mcdose_particle_dmlc_particle_v1 particle;
} mcdose_particle_dmlc_equivalent_scattered_photon_v1;

/*
 * Samples one interaction conditional on interaction within the resolved MLC
 * path. The caller owns random-number generation and supplies one value in
 * [0,1]. The result records an exact draw count for deterministic replay.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_sample_forced_interaction_v1(
    const mcdose_particle_dmlc_photon_attenuation_v1 *attenuation,
    double uniform_random,
    mcdose_particle_dmlc_forced_interaction_v1 *interaction,
    char *diagnostic,
    size_t diagnostic_capacity);

/*
 * Maps material-only interaction distance onto an absolute ray position using
 * exact material intervals supplied by the transport geometry. The interval
 * lengths must sum to attenuation.total_path_length_cm. At an exact internal
 * boundary, the upstream interval end is selected deterministically.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_locate_forced_interaction_v1(
    const mcdose_particle_dmlc_ray_v1 *ray,
    const mcdose_particle_dmlc_material_interval_v1 *material_intervals,
    uint64_t material_interval_count,
    const mcdose_particle_dmlc_photon_attenuation_v1 *attenuation,
    const mcdose_particle_dmlc_forced_interaction_v1 *interaction,
    mcdose_particle_dmlc_interaction_site_v1 *site,
    char *diagnostic,
    size_t diagnostic_capacity);

/*
 * Samples free-electron Klein-Nishina products. The incident photon position is
 * copied to both products. The equivalent-thickness model interprets it as the
 * input reference-plane coordinate; an optional solid-geometry caller may use
 * a resolved physical interaction site instead.
 * Random values are supplied by the caller in [0,1); no library RNG is used.
 * Product weight is incident weight times the forced-interaction probability
 * times the commissioned incoherent fraction. Further material transport is
 * deliberately outside this call.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_sample_compton_products_v1(
    const mcdose_particle_dmlc_particle_v1 *incident_photon,
    const mcdose_particle_dmlc_photon_attenuation_v1 *attenuation,
    const mcdose_particle_dmlc_forced_interaction_v1 *interaction,
    const double *uniform_random_values,
    uint64_t uniform_random_value_count,
    uint64_t scattered_photon_particle_id,
    uint64_t electron_particle_id,
    mcdose_particle_dmlc_compton_products_v1 *products,
    char *diagnostic,
    size_t diagnostic_capacity);

/*
 * Propagates a photon over a caller-resolved geometric path and applies the
 * commissioned attenuation for the material-only portion. Gaps are allowed,
 * so geometric_ray_path_cm may exceed material_path_length_cm. No direction
 * is discarded and no random numbers are consumed.
 */
MCDOSE_PARTICLE_DMLC_API int32_t mcdose_particle_dmlc_propagate_photon_v1(
    const mcdose_particle_dmlc_particle_v1 *incident_photon,
    const mcdose_particle_dmlc_photon_material_attenuation_v1 *attenuation,
    double geometric_ray_path_cm,
    mcdose_particle_dmlc_propagated_photon_v1 *propagation,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_finalize_equivalent_primary_v1(
    const mcdose_particle_dmlc_machine_context_v1 *machine,
    const mcdose_particle_dmlc_classification_v1 *classification,
    const mcdose_particle_dmlc_particle_v1 *incident_particle,
    mcdose_particle_dmlc_equivalent_primary_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity);

/*
 * Applies the commissioned attenuation through the forced interaction's
 * remaining equivalent longitudinal thickness and projects the scattered
 * photon back to its input reference plane. A non-forward scattered photon is
 * returned as explicit empty state, matching the commissioned model.
 */
MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_finalize_equivalent_scattered_photon_v1(
    const mcdose_particle_dmlc_machine_context_v1 *machine,
    const mcdose_particle_dmlc_classification_v1 *classification,
    const mcdose_particle_dmlc_particle_v1 *scattered_photon,
    const mcdose_particle_dmlc_forced_interaction_v1 *interaction,
    mcdose_particle_dmlc_equivalent_scattered_photon_v1 *result,
    char *diagnostic,
    size_t diagnostic_capacity);

#ifdef __cplusplus
}
#endif

#endif
