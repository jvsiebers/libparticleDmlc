#include "mcdose_particle_dmlc_egsnrc_source_v1.h"

#include "egs_config1.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::fprintf(stderr, "check failed: %s at %s:%d\n", #condition,  \
                         __FILE__, __LINE__);                                  \
            return 1;                                                          \
        }                                                                      \
    } while (false)

namespace {
uint32_t random_call_count = 0;

bool close(double left, double right) {
    return std::fabs(left - right) < 1.0e-6;
}

bool set_startup_path(const char *path) {
#ifdef _WIN32
    return _putenv_s("MCDOSE_PARTICLE_DMLC_STARTUP_ARTIFACT", path) == 0;
#else
    return setenv("MCDOSE_PARTICLE_DMLC_STARTUP_ARTIFACT", path, 1) == 0;
#endif
}
}  // namespace

extern "C" void F77_OBJ_(
    mcdose_particle_dmlc_source_random_v1,
    MCDOSE_PARTICLE_DMLC_SOURCE_RANDOM_V1)(EGS_Float *uniform_random,
                                           EGS_I32 *status) {
    ++random_call_count;
    *uniform_random = static_cast<EGS_Float>(0.5);
    *status = MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

extern "C" void F77_OBJ_(
    mcdose_particle_dmlc_source_init_v1,
    MCDOSE_PARTICLE_DMLC_SOURCE_INIT_V1)(
    EGS_I64 *source_handle, EGS_Float *electron_rest_mass_mev,
    EGS_I32 *beam_number, EGS_I32 *mlc_device_index, EGS_I32 *status);

extern "C" void F77_OBJ_(
    mcdose_particle_dmlc_source_sample_v1,
    MCDOSE_PARTICLE_DMLC_SOURCE_SAMPLE_V1)(
    EGS_I64 *source_handle, EGS_I32 *begin_new_particle, EGS_I32 *iq,
    EGS_Float *total_energy_mev, EGS_Float *x_cm, EGS_Float *y_cm,
    EGS_Float *z_cm, EGS_Float *u, EGS_Float *v, EGS_Float *w,
    EGS_Float *weight, EGS_I32 *latch, EGS_Float *fractional_meterset,
    EGS_I32 *product_kind, EGS_I32 *remaining_product_count,
    EGS_I32 *random_draw_count, EGS_I64 *history_id,
    EGS_I64 *particle_id, EGS_I64 *parent_particle_id, EGS_I32 *status);

extern "C" void F77_OBJ_(
    mcdose_particle_dmlc_source_finish_v1,
    MCDOSE_PARTICLE_DMLC_SOURCE_FINISH_V1)(EGS_I64 *source_handle,
                                           EGS_I32 *status);

int main(int argc, char **argv) {
    static_assert(sizeof(EGS_I32) == sizeof(int32_t));
    static_assert(sizeof(EGS_I64) == sizeof(uint64_t));
    CHECK(argc == 2);
    CHECK(set_startup_path(argv[1]));

    EGS_I64 source_handle = 0;
    EGS_Float electron_rest_mass_mev = static_cast<EGS_Float>(
        MCDOSE_PARTICLE_DMLC_EGSNRC_ELECTRON_REST_MASS_MEV);
    EGS_I32 beam_number = 0;
    EGS_I32 mlc_device_index = -1;
    EGS_I32 status = -1;
    F77_OBJ_(mcdose_particle_dmlc_source_init_v1,
             MCDOSE_PARTICLE_DMLC_SOURCE_INIT_V1)(
        &source_handle, &electron_rest_mass_mev, &beam_number,
        &mlc_device_index, &status);
    CHECK(status == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(source_handle != 0);
    CHECK(beam_number == 7);
    CHECK(mlc_device_index == 0);

    EGS_I32 begin_new_particle = 1;
    EGS_I32 iq = MCDOSE_PARTICLE_DMLC_PHOTON;
    EGS_Float total_energy_mev = 1.0;
    EGS_Float x_cm = static_cast<EGS_Float>(1.1);
    EGS_Float y_cm = static_cast<EGS_Float>(-0.5);
    EGS_Float z_cm = static_cast<EGS_Float>(45.0);
    EGS_Float u = 0;
    EGS_Float v = 0;
    EGS_Float w = 1;
    EGS_Float weight = static_cast<EGS_Float>(0.25);
    EGS_I32 latch = 42;
    EGS_Float fractional_meterset = static_cast<EGS_Float>(0.25);
    EGS_I32 product_kind = 0;
    EGS_I32 remaining_product_count = 0;
    EGS_I32 random_draw_count = 0;
    EGS_I64 history_id = 0;
    EGS_I64 particle_id = 0;
    EGS_I64 parent_particle_id = 0;

    F77_OBJ_(mcdose_particle_dmlc_source_sample_v1,
             MCDOSE_PARTICLE_DMLC_SOURCE_SAMPLE_V1)(
        &source_handle, &begin_new_particle, &iq, &total_energy_mev, &x_cm,
        &y_cm, &z_cm, &u, &v, &w, &weight, &latch, &fractional_meterset,
        &product_kind, &remaining_product_count, &random_draw_count,
        &history_id, &particle_id, &parent_particle_id, &status);
    CHECK(status == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(product_kind == MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY);
    CHECK(remaining_product_count == 1);
    CHECK(random_draw_count == 5);
    CHECK(random_call_count == 5);
    CHECK(history_id == 1);
    CHECK(particle_id == 1);
    CHECK(parent_particle_id == 0);
    CHECK(iq == MCDOSE_PARTICLE_DMLC_PHOTON);
    CHECK(close(total_energy_mev, 1.0));
    CHECK(close(x_cm, 1.1));
    CHECK(close(y_cm, -0.5));
    CHECK(close(z_cm, 45.0));
    CHECK(std::isfinite(weight));
    CHECK(weight > 0);
    CHECK(weight <= static_cast<EGS_Float>(0.25));
    CHECK(latch == 42);

    random_draw_count = -1;
    status = -1;
    F77_OBJ_(mcdose_particle_dmlc_source_sample_v1,
             MCDOSE_PARTICLE_DMLC_SOURCE_SAMPLE_V1)(
        &source_handle, &begin_new_particle, &iq, &total_energy_mev, &x_cm,
        &y_cm, &z_cm, &u, &v, &w, &weight, &latch, &fractional_meterset,
        &product_kind, &remaining_product_count, &random_draw_count,
        &history_id, &particle_id, &parent_particle_id, &status);
    CHECK(status == MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
    CHECK(random_draw_count == 0);
    CHECK(random_call_count == 5);

    begin_new_particle = 0;
    status = -1;
    F77_OBJ_(mcdose_particle_dmlc_source_sample_v1,
             MCDOSE_PARTICLE_DMLC_SOURCE_SAMPLE_V1)(
        &source_handle, &begin_new_particle, &iq, &total_energy_mev, &x_cm,
        &y_cm, &z_cm, &u, &v, &w, &weight, &latch, &fractional_meterset,
        &product_kind, &remaining_product_count, &random_draw_count,
        &history_id, &particle_id, &parent_particle_id, &status);
    CHECK(status == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(product_kind == MCDOSE_PARTICLE_DMLC_PRODUCER_SCATTERED_PHOTON);
    CHECK(remaining_product_count == 0);
    CHECK(random_draw_count == 0);
    CHECK(history_id == 1);
    CHECK(particle_id == 2);
    CHECK(parent_particle_id == 1);
    CHECK(random_call_count == 5);
    CHECK(std::isfinite(weight));
    CHECK(weight > 0);
    CHECK(weight <= static_cast<EGS_Float>(0.25));

    begin_new_particle = 1;
    iq = MCDOSE_PARTICLE_DMLC_ELECTRON;
    total_energy_mev = 4.0 + electron_rest_mass_mev;
    x_cm = 0;
    y_cm = static_cast<EGS_Float>(-0.5);
    z_cm = static_cast<EGS_Float>(45.0);
    u = 0;
    v = 0;
    w = 1;
    const EGS_Float open_weight = static_cast<EGS_Float>(0.73125);
    weight = open_weight;
    product_kind = 0;
    remaining_product_count = -1;
    random_draw_count = -1;
    status = -1;
    F77_OBJ_(mcdose_particle_dmlc_source_sample_v1,
             MCDOSE_PARTICLE_DMLC_SOURCE_SAMPLE_V1)(
        &source_handle, &begin_new_particle, &iq, &total_energy_mev, &x_cm,
        &y_cm, &z_cm, &u, &v, &w, &weight, &latch, &fractional_meterset,
        &product_kind, &remaining_product_count, &random_draw_count,
        &history_id, &particle_id, &parent_particle_id, &status);
    CHECK(status == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(product_kind == MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY);
    CHECK(remaining_product_count == 0);
    CHECK(random_draw_count == 0);
    CHECK(random_call_count == 5);
    CHECK(history_id == 2);
    CHECK(particle_id == 4);
    CHECK(parent_particle_id == 0);
    CHECK(iq == MCDOSE_PARTICLE_DMLC_ELECTRON);
    CHECK(close(total_energy_mev, 4.0 + electron_rest_mass_mev));
    CHECK(weight == open_weight);

    status = -1;
    F77_OBJ_(mcdose_particle_dmlc_source_finish_v1,
             MCDOSE_PARTICLE_DMLC_SOURCE_FINISH_V1)(&source_handle, &status);
    CHECK(status == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(source_handle == 0);
    return 0;
}
