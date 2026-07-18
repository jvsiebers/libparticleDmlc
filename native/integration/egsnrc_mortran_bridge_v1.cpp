#include "mcdose_particle_dmlc_egsnrc_mortran_bridge_v1.h"

#include "egs_config1.h"

#include <cmath>
#include <cstdint>

static_assert(sizeof(EGS_I32) == sizeof(int32_t));
static_assert(sizeof(EGS_I64) == sizeof(uint64_t));

extern "C" void F77_OBJ_(
    mcdose_particle_dmlc_shower_v1,
    MCDOSE_PARTICLE_DMLC_SHOWER_V1)(
    EGS_I32 *iq,
    double *total_energy_mev,
    EGS_Float *x_cm,
    EGS_Float *y_cm,
    EGS_Float *z_cm,
    EGS_Float *u,
    EGS_Float *v,
    EGS_Float *w,
    EGS_I32 *region,
    EGS_Float *weight,
    EGS_I32 *latch,
    EGS_I32 *status);

namespace {
bool valid_bridge_value(double value) {
    const auto converted = static_cast<EGS_Float>(value);
    return std::isfinite(value) && std::isfinite(converted);
}
}  // namespace

extern "C" int32_t
mcdose_particle_dmlc_egsnrc_mortran_shower_callback_v1(
    void *user_data,
    const mcdose_particle_dmlc_egsnrc_stack_entry_v1 *entry) {
    (void)user_data;
    if (entry == nullptr ||
        entry->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        entry->struct_size < sizeof(*entry) || entry->reserved != 0 ||
        entry->reserved_2 != 0 || entry->iq < MCDOSE_PARTICLE_DMLC_ELECTRON ||
        entry->iq > MCDOSE_PARTICLE_DMLC_POSITRON || entry->region <= 0 ||
        !valid_bridge_value(entry->total_energy_mev) ||
        !valid_bridge_value(entry->weight)) {
        return MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED;
    }
    for (size_t axis = 0; axis < 3; ++axis) {
        if (!valid_bridge_value(entry->position_cm[axis]) ||
            !valid_bridge_value(entry->direction[axis])) {
            return MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED;
        }
    }

    EGS_I32 iq = static_cast<EGS_I32>(entry->iq);
    double total_energy_mev = entry->total_energy_mev;
    EGS_Float x_cm = static_cast<EGS_Float>(entry->position_cm[0]);
    EGS_Float y_cm = static_cast<EGS_Float>(entry->position_cm[1]);
    EGS_Float z_cm = static_cast<EGS_Float>(entry->position_cm[2]);
    EGS_Float u = static_cast<EGS_Float>(entry->direction[0]);
    EGS_Float v = static_cast<EGS_Float>(entry->direction[1]);
    EGS_Float w = static_cast<EGS_Float>(entry->direction[2]);
    EGS_I32 region = static_cast<EGS_I32>(entry->region);
    EGS_Float weight = static_cast<EGS_Float>(entry->weight);
    EGS_I32 latch = static_cast<EGS_I32>(entry->latch);
    EGS_I32 status = MCDOSE_PARTICLE_DMLC_STATUS_OK;
    F77_OBJ_(mcdose_particle_dmlc_shower_v1,
             MCDOSE_PARTICLE_DMLC_SHOWER_V1)(
        &iq, &total_energy_mev, &x_cm, &y_cm, &z_cm, &u, &v, &w, &region,
        &weight, &latch, &status);
    return static_cast<int32_t>(status);
}

extern "C" void F77_OBJ_(
    mcdose_particle_dmlc_dispatch_shower_v1,
    MCDOSE_PARTICLE_DMLC_DISPATCH_SHOWER_V1)(
    EGS_I32 *iq,
    double *total_energy_mev,
    EGS_Float *x_cm,
    EGS_Float *y_cm,
    EGS_Float *z_cm,
    EGS_Float *u,
    EGS_Float *v,
    EGS_Float *w,
    EGS_I32 *region,
    EGS_Float *weight,
    EGS_I32 *latch,
    EGS_I64 *history_id,
    EGS_I64 *particle_id,
    EGS_I64 *parent_particle_id,
    EGS_I32 *product_kind,
    double *electron_rest_mass_mev,
    EGS_I32 *dispatched,
    EGS_I32 *discarded_by_policy,
    EGS_I32 *status) {
    if (status == nullptr) {
        return;
    }
    *status = MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT;
    if (dispatched != nullptr) {
        *dispatched = 0;
    }
    if (discarded_by_policy != nullptr) {
        *discarded_by_policy = 0;
    }
    if (iq == nullptr || total_energy_mev == nullptr || x_cm == nullptr ||
        y_cm == nullptr || z_cm == nullptr || u == nullptr || v == nullptr ||
        w == nullptr || region == nullptr || weight == nullptr ||
        latch == nullptr || history_id == nullptr || particle_id == nullptr ||
        parent_particle_id == nullptr || product_kind == nullptr ||
        electron_rest_mass_mev == nullptr || dispatched == nullptr ||
        discarded_by_policy == nullptr) {
        return;
    }
    if (*history_id <= 0 || *particle_id <= 0 || *parent_particle_id < 0) {
        *status = MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED;
        return;
    }

    mcdose_particle_dmlc_egsnrc_stack_entry_v1 entry = {};
    entry.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    entry.struct_size = sizeof(entry);
    entry.iq = static_cast<int32_t>(*iq);
    entry.region = static_cast<int32_t>(*region);
    entry.latch = static_cast<int32_t>(*latch);
    entry.history_id = static_cast<uint64_t>(*history_id);
    entry.particle_id = static_cast<uint64_t>(*particle_id);
    entry.parent_particle_id = static_cast<uint64_t>(*parent_particle_id);
    entry.total_energy_mev = *total_energy_mev;
    entry.position_cm[0] = static_cast<double>(*x_cm);
    entry.position_cm[1] = static_cast<double>(*y_cm);
    entry.position_cm[2] = static_cast<double>(*z_cm);
    entry.direction[0] = static_cast<double>(*u);
    entry.direction[1] = static_cast<double>(*v);
    entry.direction[2] = static_cast<double>(*w);
    entry.weight = static_cast<double>(*weight);

    mcdose_particle_dmlc_egsnrc_dispatch_result_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    char diagnostic[512];
    const auto dispatch_status =
        mcdose_particle_dmlc_dispatch_egsnrc_product_v1(
            &entry, static_cast<int32_t>(*product_kind),
            *electron_rest_mass_mev,
            mcdose_particle_dmlc_egsnrc_mortran_shower_callback_v1, nullptr,
            &result, diagnostic, sizeof(diagnostic));
    *dispatched = static_cast<EGS_I32>(result.dispatched);
    *discarded_by_policy =
        static_cast<EGS_I32>(result.discarded_by_policy);
    *status = static_cast<EGS_I32>(dispatch_status);
}
