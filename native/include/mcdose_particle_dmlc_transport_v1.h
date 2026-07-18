#ifndef MCDOSE_PARTICLE_DMLC_TRANSPORT_V1_H
#define MCDOSE_PARTICLE_DMLC_TRANSPORT_V1_H

#include "mcdose_particle_dmlc_machine_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

enum mcdose_particle_dmlc_section_region_v1 {
    MCDOSE_PARTICLE_DMLC_SECTION_OPEN = 0,
    MCDOSE_PARTICLE_DMLC_SECTION_LEADING_FULL = 1,
    MCDOSE_PARTICLE_DMLC_SECTION_FOLLOWING_FULL = 2,
    MCDOSE_PARTICLE_DMLC_SECTION_TIP_OR_CROSSING = 3,
    MCDOSE_PARTICLE_DMLC_SECTION_OUTSIDE = 4
};

typedef struct mcdose_particle_dmlc_aperture_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t leaf_pair_count;
    uint32_t reserved;
    const double *bank_1_positions_at_isocenter_cm;
    const double *bank_2_positions_at_isocenter_cm;
} mcdose_particle_dmlc_aperture_v1;

typedef struct mcdose_particle_dmlc_ray_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    double position_cm[3];
    double direction[3];
} mcdose_particle_dmlc_ray_v1;

/*
 * Version 1 machine profiles contain exactly two longitudinal sections.
 * Open, full-leaf, rounded-tip, and boundary-crossing intersections resolve a
 * longitudinal thickness. The region remains explicit so later interaction
 * transport can retain primary-tip and primary-leaf categories.
 */
typedef struct mcdose_particle_dmlc_classification_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t section_count;
    uint32_t reserved;
    int32_t region[2];
    uint32_t leaf_pair_index[2];
    uint32_t thickness_resolved[2];
    uint32_t reserved_2[2];
    double transverse_center_cm[2];
    double motion_entry_cm[2];
    double motion_center_cm[2];
    double motion_exit_cm[2];
    double following_position_cm[2];
    double leading_position_cm[2];
    double resolved_longitudinal_thickness_cm[2];
} mcdose_particle_dmlc_classification_v1;

typedef struct mcdose_particle_dmlc_photon_attenuation_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t section_count;
    uint32_t reserved;
    double energy_mev;
    double direction_z;
    double total_longitudinal_thickness_cm;
    double total_path_length_cm;
    double total_linear_attenuation_per_cm;
    double incoherent_linear_attenuation_per_cm;
    double optical_depth;
    double primary_transmission;
    double interaction_probability;
    double incoherent_interaction_fraction;
} mcdose_particle_dmlc_photon_attenuation_v1;

/*
 * Geometry-neutral attenuation over a caller-resolved material path. This is
 * suitable for both primary and scattered photons; gaps in the corresponding
 * geometric ray path are deliberately excluded from material_path_length_cm.
 */
typedef struct mcdose_particle_dmlc_photon_material_attenuation_v1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t reserved[2];
    double energy_mev;
    double material_path_length_cm;
    double total_linear_attenuation_per_cm;
    double incoherent_linear_attenuation_per_cm;
    double optical_depth;
    double transmission;
    double interaction_probability;
    double incoherent_interaction_fraction;
} mcdose_particle_dmlc_photon_material_attenuation_v1;

MCDOSE_PARTICLE_DMLC_API int32_t mcdose_particle_dmlc_classify_ray_v1(
    const mcdose_particle_dmlc_machine_context_v1 *machine,
    const mcdose_particle_dmlc_aperture_v1 *aperture,
    const mcdose_particle_dmlc_ray_v1 *ray,
    mcdose_particle_dmlc_classification_v1 *classification,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_evaluate_photon_attenuation_v1(
    const mcdose_particle_dmlc_machine_context_v1 *machine,
    const mcdose_particle_dmlc_classification_v1 *classification,
    double energy_mev,
    double direction_z,
    mcdose_particle_dmlc_photon_attenuation_v1 *attenuation,
    char *diagnostic,
    size_t diagnostic_capacity);

MCDOSE_PARTICLE_DMLC_API int32_t
mcdose_particle_dmlc_evaluate_photon_material_path_v1(
    const mcdose_particle_dmlc_machine_context_v1 *machine,
    double energy_mev,
    double material_path_length_cm,
    mcdose_particle_dmlc_photon_material_attenuation_v1 *attenuation,
    char *diagnostic,
    size_t diagnostic_capacity);

#ifdef __cplusplus
}
#endif

#endif
