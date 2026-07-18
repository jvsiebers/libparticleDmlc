#include "mcdose_particle_dmlc_startup_v1.h"

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
struct random_state {
    double values[7] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
    size_t next = 0;
};

int32_t next_random(void *user_data, double *value) {
    if (user_data == nullptr || value == nullptr) {
        return MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT;
    }
    auto *state = static_cast<random_state *>(user_data);
    if (state->next >= 7) {
        return MCDOSE_PARTICLE_DMLC_STATUS_OUTPUT_CAPACITY;
    }
    *value = state->values[state->next++];
    return MCDOSE_PARTICLE_DMLC_STATUS_OK;
}

bool digest(const char *value) {
    if (std::strlen(value) != 64) {
        return false;
    }
    for (size_t index = 0; index < 64; ++index) {
        const char character = value[index];
        if (!((character >= '0' && character <= '9') ||
              (character >= 'a' && character <= 'f'))) {
            return false;
        }
    }
    return true;
}
}  // namespace

int main(int argc, char **argv) {
    static_assert(sizeof(mcdose_particle_dmlc_startup_info_v1) == 232);
    CHECK(argc == 5);
    char diagnostic[512] = {};
    mcdose_particle_dmlc_startup_info_v1 info = {};
    info.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    info.struct_size = sizeof(info);
    mcdose_particle_dmlc_producer_context_v1 *context = nullptr;
    CHECK(mcdose_particle_dmlc_create_producer_from_startup_v1(
              argv[1], &context, &info, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(context != nullptr);
    CHECK(info.beam_number == 7);
    CHECK(info.mlc_device_index == 0);
    CHECK(info.device_count == 1);
    CHECK(info.control_point_count == 2);
    CHECK(info.leaf_pair_count == 2);
    CHECK(info.generated_electron_policy ==
          MCDOSE_PARTICLE_DMLC_GENERATED_ELECTRON_DISCARD);
    CHECK(digest(info.delivery_sha256));
    CHECK(digest(info.machine_sha256));
    CHECK(digest(info.payload_sha256));

    random_state random;
    CHECK(mcdose_particle_dmlc_set_producer_random_source_v1(
              context, next_random, &random, diagnostic, sizeof(diagnostic)) ==
          MCDOSE_PARTICLE_DMLC_STATUS_OK);
    mcdose_particle_dmlc_particle_v1 particle = {};
    particle.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    particle.struct_size = sizeof(particle);
    particle.charge = MCDOSE_PARTICLE_DMLC_PHOTON;
    particle.history_id = 17;
    particle.particle_id = 23;
    particle.position_cm[0] = 1.1;
    particle.position_cm[1] = -0.5;
    particle.position_cm[2] = 45.0;
    particle.direction[2] = 1.0;
    particle.energy_mev = 1.0;
    particle.weight = 0.25;
    mcdose_particle_dmlc_producer_summary_v1 summary = {};
    summary.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    summary.struct_size = sizeof(summary);
    CHECK(mcdose_particle_dmlc_produce_v1(
              context, &particle, 0.25, 24, 25, &summary, diagnostic,
              sizeof(diagnostic)) == MCDOSE_PARTICLE_DMLC_STATUS_OK);
    CHECK(summary.random_draw_count == 5);
    CHECK(random.next == 5);
    for (int32_t expected_kind = MCDOSE_PARTICLE_DMLC_PRODUCER_PRIMARY;
         expected_kind <= MCDOSE_PARTICLE_DMLC_PRODUCER_SCATTERED_PHOTON;
         ++expected_kind) {
        mcdose_particle_dmlc_producer_product_v1 product = {};
        product.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        product.struct_size = sizeof(product);
        CHECK(mcdose_particle_dmlc_next_producer_product_v1(
                  context, &product, diagnostic, sizeof(diagnostic)) ==
              MCDOSE_PARTICLE_DMLC_STATUS_OK);
        CHECK(product.has_product == 1);
        CHECK(product.product_kind == expected_kind);
    }
    mcdose_particle_dmlc_destroy_producer_context_v1(context);

    for (int index = 2; index < argc; ++index) {
        context = nullptr;
        info = {};
        info.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
        info.struct_size = sizeof(info);
        CHECK(mcdose_particle_dmlc_create_producer_from_startup_v1(
                  argv[index], &context, &info, diagnostic,
                  sizeof(diagnostic)) ==
              MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED);
        CHECK(context == nullptr);
    }
    return 0;
}
