#include "mcdose_particle_dmlc_egsnrc_mortran_bridge_v1.h"

#include "egs_config1.h"

#include <cmath>
#include <cstdio>

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::fprintf(stderr, "check failed: %s at %s:%d\n", #condition,  \
                         __FILE__, __LINE__);                                  \
            return 1;                                                          \
        }                                                                      \
    } while (false)

namespace {
uint32_t shower_call_count = 0;
EGS_I32 observed_iq = 0;
EGS_I32 observed_region = 0;
EGS_I32 observed_latchi = 0;
double observed_energy = 0.0;
EGS_Float observed_x = 0.0;
EGS_Float observed_weight = 0.0;
}  // namespace

extern "C" void F77_OBJ_(
    mcdose_particle_dmlc_shower_v1,
    MCDOSE_PARTICLE_DMLC_SHOWER_V1)(
    EGS_I32 *iq,
    double *total_energy_mev,
    EGS_Float *x_cm,
    EGS_Float *,
    EGS_Float *,
    EGS_Float *,
    EGS_Float *,
    EGS_Float *,
    EGS_I32 *region,
    EGS_Float *weight,
    EGS_I32 *latch,
    EGS_I32 *status) {
    ++shower_call_count;
    observed_iq = *iq;
    observed_region = *region;
    observed_latchi = *latch;
    observed_energy = *total_energy_mev;
    observed_x = *x_cm;
    observed_weight = *weight;
    *status = MCDOSE_PARTICLE_DMLC_STATUS_OK;
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
    EGS_I32 *status);

int main() {
    mcdose_particle_dmlc_egsnrc_stack_entry_v1 entry = {};
    entry.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    entry.struct_size = sizeof(entry);
    entry.iq = MCDOSE_PARTICLE_DMLC_PHOTON;
    entry.region = 7;
    entry.latch = -17;
    entry.history_id = 17;
    entry.particle_id = 24;
    entry.total_energy_mev = 1.5;
    entry.position_cm[0] = 1.25;
    entry.position_cm[1] = -0.5;
    entry.position_cm[2] = 40.5;
    entry.direction[0] = 0.6;
    entry.direction[2] = 0.8;
    entry.weight = 0.25;

    mcdose_particle_dmlc_egsnrc_dispatch_result_v1 result = {};
    result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.struct_size = sizeof(result);
    char diagnostic[256];
    CHECK(mcdose_particle_dmlc_dispatch_egsnrc_product_v1(
              &entry, MCDOSE_PARTICLE_DMLC_EGSNRC_SCATTERED_PHOTON,
              MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV,
              mcdose_particle_dmlc_egsnrc_mortran_shower_callback_v1, nullptr,
              &result, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(result.dispatched == 1);
    CHECK(result.discarded_by_policy == 0);
    CHECK(shower_call_count == 1);
    CHECK(observed_iq == entry.iq);
    CHECK(observed_region == entry.region);
    CHECK(observed_latchi == entry.latch);
    CHECK(observed_energy == entry.total_energy_mev);
    CHECK(std::fabs(observed_x - entry.position_cm[0]) < 1.0e-6);
    CHECK(std::fabs(observed_weight - entry.weight) < 1.0e-6);

    EGS_I32 iq = entry.iq;
    double total_energy_mev = entry.total_energy_mev;
    EGS_Float x_cm = entry.position_cm[0];
    EGS_Float y_cm = entry.position_cm[1];
    EGS_Float z_cm = entry.position_cm[2];
    EGS_Float u = entry.direction[0];
    EGS_Float v = entry.direction[1];
    EGS_Float w = entry.direction[2];
    EGS_I32 region = entry.region;
    EGS_Float weight = entry.weight;
    EGS_I32 latch = entry.latch;
    EGS_I64 history_id = entry.history_id;
    EGS_I64 particle_id = entry.particle_id;
    EGS_I64 parent_particle_id = entry.parent_particle_id;
    EGS_I32 product_kind = MCDOSE_PARTICLE_DMLC_EGSNRC_PRIMARY;
    double electron_rest_mass_mev =
        MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV;
    EGS_I32 dispatched = 0;
    EGS_I32 discarded_by_policy = 0;
    EGS_I32 status = -1;
    F77_OBJ_(mcdose_particle_dmlc_dispatch_shower_v1,
             MCDOSE_PARTICLE_DMLC_DISPATCH_SHOWER_V1)(
        &iq, &total_energy_mev, &x_cm, &y_cm, &z_cm, &u, &v, &w, &region,
        &weight, &latch, &history_id, &particle_id, &parent_particle_id,
        &product_kind, &electron_rest_mass_mev, &dispatched,
        &discarded_by_policy, &status);
    CHECK(status == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(dispatched == 1);
    CHECK(discarded_by_policy == 0);
    CHECK(shower_call_count == 2);

    iq = MCDOSE_PARTICLE_DMLC_ELECTRON;
    total_energy_mev = electron_rest_mass_mev + 1.0;
    product_kind =
        MCDOSE_PARTICLE_DMLC_EGSNRC_GENERATED_COMPTON_ELECTRON;
    dispatched = -1;
    discarded_by_policy = -1;
    status = -1;
    F77_OBJ_(mcdose_particle_dmlc_dispatch_shower_v1,
             MCDOSE_PARTICLE_DMLC_DISPATCH_SHOWER_V1)(
        &iq, &total_energy_mev, &x_cm, &y_cm, &z_cm, &u, &v, &w, &region,
        &weight, &latch, &history_id, &particle_id, &parent_particle_id,
        &product_kind, &electron_rest_mass_mev, &dispatched,
        &discarded_by_policy, &status);
    CHECK(status == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(dispatched == 0);
    CHECK(discarded_by_policy == 1);
    CHECK(shower_call_count == 2);

    total_energy_mev = electron_rest_mass_mev;
    product_kind = MCDOSE_PARTICLE_DMLC_EGSNRC_PRIMARY;
    dispatched = -1;
    discarded_by_policy = -1;
    status = -1;
    F77_OBJ_(mcdose_particle_dmlc_dispatch_shower_v1,
             MCDOSE_PARTICLE_DMLC_DISPATCH_SHOWER_V1)(
        &iq, &total_energy_mev, &x_cm, &y_cm, &z_cm, &u, &v, &w, &region,
        &weight, &latch, &history_id, &particle_id, &parent_particle_id,
        &product_kind, &electron_rest_mass_mev, &dispatched,
        &discarded_by_policy, &status);
    CHECK(status == MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(dispatched == 0);
    CHECK(discarded_by_policy == 0);
    CHECK(shower_call_count == 2);
    return 0;
}
