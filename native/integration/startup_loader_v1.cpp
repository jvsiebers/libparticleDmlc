#include "mcdose_particle_dmlc_startup_v1.h"

#include <sqlite3.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
constexpr const char *startup_schema =
    "mcdose.particle-dmlc-producer-startup/v1";
constexpr const char *delivery_schema =
    "mcdose.particle-dmlc-native-delivery/v1";
constexpr const char *machine_schema =
    "mcdose.particle-dmlc-native-machine/v1";
constexpr uint64_t maximum_array_bytes = UINT64_C(1073741824);

class load_error : public std::runtime_error {
  public:
    load_error(int32_t status, const std::string &message)
        : std::runtime_error(message), status_(status) {}
    int32_t status() const noexcept { return status_; }

  private:
    int32_t status_;
};

int32_t fail(int32_t status, char *diagnostic, size_t capacity,
             const char *message) {
    if (diagnostic != nullptr && capacity > 0) {
        std::snprintf(diagnostic, capacity, "%s", message);
    }
    return status;
}

class database {
  public:
    explicit database(const char *path) {
        const int initialize_result = sqlite3_initialize();
        if (initialize_result != SQLITE_OK) {
            throw load_error(
                MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                "startup SQLite initialization failed with status " +
                    std::to_string(initialize_result));
        }
        const int result = sqlite3_open_v2(
            path, &handle_, SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX, nullptr);
        if (result != SQLITE_OK) {
            const std::string message =
                handle_ == nullptr ? "could not open startup database"
                                   : sqlite3_errmsg(handle_);
            if (handle_ != nullptr) {
                sqlite3_close(handle_);
                handle_ = nullptr;
            }
            throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                             "startup SQLite open failed: " + message);
        }
    }
    ~database() { sqlite3_close(handle_); }
    database(const database &) = delete;
    database &operator=(const database &) = delete;
    sqlite3 *get() const noexcept { return handle_; }

  private:
    sqlite3 *handle_ = nullptr;
};

class statement {
  public:
    statement(sqlite3 *db, const char *sql) : db_(db) {
        if (sqlite3_prepare_v2(db, sql, -1, &handle_, nullptr) != SQLITE_OK) {
            throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                             std::string("startup SQLite prepare failed: ") +
                                 sqlite3_errmsg(db));
        }
    }
    ~statement() { sqlite3_finalize(handle_); }
    statement(const statement &) = delete;
    statement &operator=(const statement &) = delete;
    sqlite3_stmt *get() const noexcept { return handle_; }
    void bind_text(const char *value) {
        if (sqlite3_bind_text(handle_, 1, value, -1, SQLITE_TRANSIENT) !=
            SQLITE_OK) {
            throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                             "startup SQLite bind failed");
        }
    }
    bool row() {
        const int result = sqlite3_step(handle_);
        if (result == SQLITE_ROW) {
            return true;
        }
        if (result == SQLITE_DONE) {
            return false;
        }
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string("startup SQLite query failed: ") +
                             sqlite3_errmsg(db_));
    }

  private:
    sqlite3 *db_ = nullptr;
    sqlite3_stmt *handle_ = nullptr;
};

int64_t integer(sqlite3_stmt *row, int column, const char *name) {
    if (sqlite3_column_type(row, column) != SQLITE_INTEGER) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string(name) + " must be an SQLite integer");
    }
    return sqlite3_column_int64(row, column);
}

uint64_t unsigned_integer(sqlite3_stmt *row, int column, const char *name) {
    const int64_t value = integer(row, column, name);
    if (value < 0) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string(name) + " cannot be negative");
    }
    return static_cast<uint64_t>(value);
}

uint32_t unsigned_32(sqlite3_stmt *row, int column, const char *name) {
    const uint64_t value = unsigned_integer(row, column, name);
    if (value > std::numeric_limits<uint32_t>::max()) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string(name) + " exceeds uint32");
    }
    return static_cast<uint32_t>(value);
}

double real(sqlite3_stmt *row, int column, const char *name) {
    if (sqlite3_column_type(row, column) != SQLITE_FLOAT) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string(name) + " must be an SQLite real");
    }
    const double value = sqlite3_column_double(row, column);
    if (!std::isfinite(value)) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string(name) + " must be finite");
    }
    return value;
}

std::string text_value(sqlite3_stmt *row, int column, const char *name) {
    if (sqlite3_column_type(row, column) != SQLITE_TEXT) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string(name) + " must be SQLite text");
    }
    const auto *bytes = sqlite3_column_text(row, column);
    const int count = sqlite3_column_bytes(row, column);
    if (bytes == nullptr || count <= 0) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string(name) + " cannot be empty");
    }
    return std::string(reinterpret_cast<const char *>(bytes),
                       static_cast<size_t>(count));
}

void require_done(statement &query, const char *name) {
    if (query.row()) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string(name) + " must contain exactly one row");
    }
}

bool valid_sha256(const std::string &value) {
    return value.size() == 64 &&
           std::all_of(value.begin(), value.end(), [](unsigned char character) {
               return (character >= '0' && character <= '9') ||
                      (character >= 'a' && character <= 'f');
           });
}

uint64_t checked_product(uint64_t first, uint64_t second,
                         const char *name) {
    if (first != 0 && second > std::numeric_limits<uint64_t>::max() / first) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED,
                         std::string(name) + " element count overflows");
    }
    return first * second;
}

class sha256 {
  public:
    sha256() = default;

    void update(const void *data, size_t size) {
        const auto *bytes = static_cast<const uint8_t *>(data);
        bit_count_ += static_cast<uint64_t>(size) * 8;
        while (size > 0) {
            const size_t available = block_.size() - block_size_;
            const size_t count = std::min(size, available);
            std::memcpy(block_.data() + block_size_, bytes, count);
            block_size_ += count;
            bytes += count;
            size -= count;
            if (block_size_ == block_.size()) {
                transform(block_.data());
                block_size_ = 0;
            }
        }
    }

    std::string finish() {
        const uint64_t message_bits = bit_count_;
        const uint8_t marker = 0x80;
        update(&marker, 1);
        const uint8_t zero = 0;
        while (block_size_ != 56) {
            update(&zero, 1);
        }
        uint8_t length[8];
        for (size_t index = 0; index < 8; ++index) {
            length[7 - index] =
                static_cast<uint8_t>((message_bits >> (index * 8)) & 0xffu);
        }
        update(length, sizeof(length));
        static constexpr char digits[] = "0123456789abcdef";
        std::string result(64, '0');
        for (size_t index = 0; index < state_.size(); ++index) {
            for (size_t byte = 0; byte < 4; ++byte) {
                const uint8_t value = static_cast<uint8_t>(
                    state_[index] >> ((3 - byte) * 8));
                result[index * 8 + byte * 2] = digits[value >> 4];
                result[index * 8 + byte * 2 + 1] = digits[value & 0x0f];
            }
        }
        return result;
    }

  private:
    static uint32_t rotate_right(uint32_t value, uint32_t count) {
        return (value >> count) | (value << (32 - count));
    }

    void transform(const uint8_t *block) {
        static constexpr std::array<uint32_t, 64> constants = {
            0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
            0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
            0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
            0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
            0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
            0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
            0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
            0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
            0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
            0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
            0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
            0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
            0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
            0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
            0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
            0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u};
        std::array<uint32_t, 64> words = {};
        for (size_t index = 0; index < 16; ++index) {
            words[index] =
                (static_cast<uint32_t>(block[index * 4]) << 24) |
                (static_cast<uint32_t>(block[index * 4 + 1]) << 16) |
                (static_cast<uint32_t>(block[index * 4 + 2]) << 8) |
                static_cast<uint32_t>(block[index * 4 + 3]);
        }
        for (size_t index = 16; index < words.size(); ++index) {
            const uint32_t s0 = rotate_right(words[index - 15], 7) ^
                                rotate_right(words[index - 15], 18) ^
                                (words[index - 15] >> 3);
            const uint32_t s1 = rotate_right(words[index - 2], 17) ^
                                rotate_right(words[index - 2], 19) ^
                                (words[index - 2] >> 10);
            words[index] = words[index - 16] + s0 + words[index - 7] + s1;
        }
        uint32_t a = state_[0];
        uint32_t b = state_[1];
        uint32_t c = state_[2];
        uint32_t d = state_[3];
        uint32_t e = state_[4];
        uint32_t f = state_[5];
        uint32_t g = state_[6];
        uint32_t h = state_[7];
        for (size_t index = 0; index < words.size(); ++index) {
            const uint32_t sum1 = rotate_right(e, 6) ^ rotate_right(e, 11) ^
                                  rotate_right(e, 25);
            const uint32_t choice = (e & f) ^ ((~e) & g);
            const uint32_t temporary1 =
                h + sum1 + choice + constants[index] + words[index];
            const uint32_t sum0 = rotate_right(a, 2) ^ rotate_right(a, 13) ^
                                  rotate_right(a, 22);
            const uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
            const uint32_t temporary2 = sum0 + majority;
            h = g;
            g = f;
            f = e;
            e = d + temporary1;
            d = c;
            c = b;
            b = a;
            a = temporary1 + temporary2;
        }
        state_[0] += a;
        state_[1] += b;
        state_[2] += c;
        state_[3] += d;
        state_[4] += e;
        state_[5] += f;
        state_[6] += g;
        state_[7] += h;
    }

    std::array<uint32_t, 8> state_ = {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u};
    std::array<uint8_t, 64> block_ = {};
    size_t block_size_ = 0;
    uint64_t bit_count_ = 0;
};

class payload_hash {
  public:
    payload_hash() {
        static constexpr char prefix[] =
            "mcdose.particle-dmlc-producer-startup-payload/v1";
        value_.update(prefix, sizeof(prefix));
    }

    void add_integer(const std::string &name, int64_t value) {
        uint8_t bytes[8];
        encode_64(static_cast<uint64_t>(value), bytes);
        add(name, 'I', bytes, sizeof(bytes));
    }
    void add_real(const std::string &name, double value) {
        uint64_t bits = 0;
        static_assert(sizeof(bits) == sizeof(value));
        std::memcpy(&bits, &value, sizeof(bits));
        uint8_t bytes[8];
        encode_64(bits, bytes);
        add(name, 'F', bytes, sizeof(bytes));
    }
    void add_text(const std::string &name, const std::string &value) {
        add(name, 'T', value.data(), value.size());
    }
    void add_blob(const std::string &name, const void *data, size_t size) {
        add(name, 'B', data, size);
    }
    std::string finish() { return value_.finish(); }

  private:
    static void encode_64(uint64_t value, uint8_t *bytes) {
        for (size_t index = 0; index < 8; ++index) {
            bytes[index] = static_cast<uint8_t>(value >> (index * 8));
        }
    }
    void add(const std::string &name, char kind, const void *data,
             size_t size) {
        if (name.size() > std::numeric_limits<uint16_t>::max()) {
            throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                             "startup payload field name is too long");
        }
        const uint16_t name_size = static_cast<uint16_t>(name.size());
        const uint8_t name_length[2] = {
            static_cast<uint8_t>(name_size),
            static_cast<uint8_t>(name_size >> 8)};
        value_.update(name_length, sizeof(name_length));
        value_.update(name.data(), name.size());
        value_.update(&kind, 1);
        uint8_t length[8];
        encode_64(static_cast<uint64_t>(size), length);
        value_.update(length, sizeof(length));
        if (size != 0) {
            value_.update(data, size);
        }
    }
    sha256 value_;
};

struct loaded_startup {
    mcdose_particle_dmlc_delivery_v1 delivery = {};
    mcdose_particle_dmlc_machine_v1 machine = {};
    mcdose_particle_dmlc_producer_config_v1 config = {};
    std::string delivery_sha256;
    std::string machine_sha256;
    std::string expected_payload_sha256;

    std::vector<int32_t> device_kinds;
    std::vector<int32_t> device_axes;
    std::vector<uint32_t> device_pair_counts;
    std::vector<double> device_source_distances_cm;
    std::vector<uint8_t> device_source_distance_present;
    std::vector<uint64_t> device_opening_offsets;
    std::vector<uint64_t> device_boundary_offsets;
    std::vector<double> cumulative_meterset_fractions;
    std::vector<double> gantry_angles_deg;
    std::vector<int32_t> gantry_directions;
    std::vector<double> collimator_angles_deg;
    std::vector<int32_t> collimator_directions;
    std::vector<double> patient_support_angles_deg;
    std::vector<int32_t> patient_support_directions;
    std::vector<double> isocenter_patient_lps_cm;
    std::vector<double> nominal_beam_energies_mev;
    std::vector<double> dose_rates_mu_per_minute;
    std::vector<uint8_t> dose_rate_present;
    std::vector<double> tabletop_pitch_angles_deg;
    std::vector<double> tabletop_roll_angles_deg;
    std::vector<double> bank_1_positions_cm;
    std::vector<double> bank_2_positions_cm;
    std::vector<double> mlc_boundaries_at_isocenter_cm;

    std::vector<double> section_z_enter_cm;
    std::vector<double> section_z_center_cm;
    std::vector<double> section_z_exit_cm;
    std::vector<uint64_t> section_row_offsets;
    std::vector<double> section_y_start_cm;
    std::vector<double> section_y_end_cm;
    std::vector<uint32_t> section_leaf_pair_indices;
    std::vector<double> section_thickness_cm;
    std::vector<double> section_groove_thickness_cm;
    std::vector<double> section_groove_distance_from_tip_cm;
    std::vector<double> attenuation_energy_mev;
    std::vector<double> total_linear_attenuation_per_cm;
    std::vector<double> incoherent_linear_attenuation_per_cm;
    std::vector<double> calibration_specified_position_cm;
    std::vector<double> calibration_actual_projected_position_cm;
};

int64_t pragma_integer(sqlite3 *db, const char *sql) {
    statement query(db, sql);
    if (!query.row()) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup SQLite pragma returned no row");
    }
    const int64_t result = integer(query.get(), 0, "startup SQLite pragma");
    require_done(query, "startup SQLite pragma");
    return result;
}

void execute(sqlite3 *db, const char *sql) {
    char *message = nullptr;
    const int result = sqlite3_exec(db, sql, nullptr, nullptr, &message);
    if (result != SQLITE_OK) {
        const std::string detail = message == nullptr ? "unknown error" : message;
        sqlite3_free(message);
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup SQLite configuration failed: " + detail);
    }
}

void validate_columns(sqlite3 *db, const char *table,
                      const std::vector<std::string> &expected) {
    const std::string sql = "PRAGMA table_info(" + std::string(table) + ")";
    statement query(db, sql.c_str());
    std::vector<std::string> actual;
    while (query.row()) {
        actual.push_back(text_value(query.get(), 1, "startup column name"));
    }
    if (actual != expected) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string("startup SQLite table columns differ: ") +
                             table);
    }
}

void validate_database(sqlite3 *db) {
    execute(db, "PRAGMA query_only = ON");
    execute(db, "PRAGMA trusted_schema = OFF");
    if (pragma_integer(db, "PRAGMA application_id") !=
            MCDOSE_PARTICLE_DMLC_STARTUP_APPLICATION_ID ||
        pragma_integer(db, "PRAGMA user_version") !=
            MCDOSE_PARTICLE_DMLC_STARTUP_USER_VERSION) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup SQLite application id or version differs");
    }
    statement integrity(db, "PRAGMA integrity_check");
    if (!integrity.row() ||
        text_value(integrity.get(), 0, "startup integrity result") != "ok") {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup SQLite integrity check failed");
    }
    require_done(integrity, "startup SQLite integrity check");

    statement objects(
        db, "SELECT type, name FROM sqlite_schema "
            "WHERE name NOT LIKE 'sqlite_%' ORDER BY type, name");
    std::vector<std::pair<std::string, std::string>> actual;
    while (objects.row()) {
        actual.emplace_back(
            text_value(objects.get(), 0, "startup object type"),
            text_value(objects.get(), 1, "startup object name"));
    }
    const std::vector<std::pair<std::string, std::string>> expected = {
        {"table", "artifact"},
        {"table", "delivery"},
        {"table", "device_identifier"},
        {"table", "machine"},
        {"table", "machine_artifact"},
        {"table", "native_array"}};
    if (actual != expected) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup SQLite schema contains unexpected objects");
    }
    validate_columns(db, "artifact",
                     {"schema", "abi_version", "byte_order",
                      "generated_electron_policy", "mlc_device_index",
                      "delivery_sha256", "machine_sha256", "payload_sha256"});
    validate_columns(db, "delivery",
                     {"singleton", "schema", "abi_version", "beam_number",
                      "source_axis_distance_cm", "device_count",
                      "control_point_count", "total_pair_count",
                      "boundary_count"});
    validate_columns(db, "device_identifier",
                     {"device_index", "identifier"});
    validate_columns(
        db, "machine",
        {"singleton", "schema", "abi_version", "profile_identifier",
         "profile_revision", "leaf_pair_count", "section_count",
         "section_row_count", "attenuation_point_count",
         "calibration_point_count", "source_axis_distance_cm",
         "source_to_device_distance_cm", "position_specification_distance_cm",
         "material_density_g_cm3", "calibration_mode", "motion_axis",
         "touching_pairs_bypass_curve", "projection_ratio",
         "physical_leaf_offset_cm", "rounded_tip_maximum_thickness_cm",
         "rounded_tip_radius_cm", "rounded_tip_angle_deg",
         "rounded_tip_transition_distance_cm", "rounded_tip_section_count"});
    validate_columns(db, "machine_artifact",
                     {"artifact_index", "identifier", "sha256"});
    validate_columns(db, "native_array",
                     {"array_name", "dtype", "element_count", "data"});
}

template <typename T>
void load_array(sqlite3 *db, payload_hash &payload, const char *name,
                const char *dtype, uint64_t expected_count,
                std::vector<T> &destination) {
    if (expected_count > maximum_array_bytes / sizeof(T) ||
        expected_count > static_cast<uint64_t>(destination.max_size())) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED,
                         std::string("startup array is too large: ") + name);
    }
    statement query(
        db, "SELECT dtype, element_count, data FROM native_array "
            "WHERE array_name = ?");
    query.bind_text(name);
    if (!query.row()) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string("startup array is missing: ") + name);
    }
    const std::string stored_dtype =
        text_value(query.get(), 0, "startup array dtype");
    const uint64_t count =
        unsigned_integer(query.get(), 1, "startup array element count");
    if (stored_dtype != dtype || count != expected_count ||
        sqlite3_column_type(query.get(), 2) != SQLITE_BLOB) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string("startup array metadata differs: ") + name);
    }
    const int byte_count = sqlite3_column_bytes(query.get(), 2);
    const uint64_t expected_bytes = checked_product(count, sizeof(T), name);
    if (byte_count < 0 || static_cast<uint64_t>(byte_count) != expected_bytes) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string("startup array byte count differs: ") +
                             name);
    }
    const void *data = sqlite3_column_blob(query.get(), 2);
    if (byte_count > 0 && data == nullptr) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         std::string("startup array data is null: ") + name);
    }
    const std::string prefix = "native_array." + std::string(name);
    payload.add_text(prefix + ".dtype", stored_dtype);
    payload.add_integer(prefix + ".element_count",
                        static_cast<int64_t>(count));
    payload.add_blob(prefix + ".data", data,
                     static_cast<size_t>(byte_count));
    destination.resize(static_cast<size_t>(count));
    if (byte_count > 0) {
        std::memcpy(destination.data(), data, static_cast<size_t>(byte_count));
    }
    require_done(query, "startup native array");
}

loaded_startup load_startup(sqlite3 *db, payload_hash &payload) {
    loaded_startup result;
    result.delivery.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.delivery.struct_size = sizeof(result.delivery);
    result.machine.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.machine.struct_size = sizeof(result.machine);
    result.config.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
    result.config.struct_size = sizeof(result.config);

    statement artifact(
        db, "SELECT schema, abi_version, byte_order, "
            "generated_electron_policy, mlc_device_index, delivery_sha256, "
            "machine_sha256, payload_sha256 FROM artifact");
    if (!artifact.row()) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup artifact row is missing");
    }
    const std::string artifact_schema =
        text_value(artifact.get(), 0, "startup schema");
    const uint32_t artifact_abi =
        unsigned_32(artifact.get(), 1, "startup ABI version");
    const std::string byte_order =
        text_value(artifact.get(), 2, "startup byte order");
    result.config.generated_electron_policy =
        unsigned_32(artifact.get(), 3, "generated-electron policy");
    result.config.mlc_device_index =
        unsigned_32(artifact.get(), 4, "MLC device index");
    result.delivery_sha256 =
        text_value(artifact.get(), 5, "delivery SHA-256");
    result.machine_sha256 =
        text_value(artifact.get(), 6, "machine SHA-256");
    result.expected_payload_sha256 =
        text_value(artifact.get(), 7, "payload SHA-256");
    require_done(artifact, "startup artifact table");
    if (artifact_schema != startup_schema ||
        artifact_abi != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
        byte_order != "little" ||
        result.config.generated_electron_policy !=
            MCDOSE_PARTICLE_DMLC_GENERATED_ELECTRON_DISCARD ||
        !valid_sha256(result.delivery_sha256) ||
        !valid_sha256(result.machine_sha256) ||
        !valid_sha256(result.expected_payload_sha256)) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup artifact identity or policy differs");
    }
    payload.add_text("artifact.schema", artifact_schema);
    payload.add_integer("artifact.abi_version", artifact_abi);
    payload.add_text("artifact.byte_order", byte_order);
    payload.add_integer("artifact.generated_electron_policy",
                        result.config.generated_electron_policy);
    payload.add_integer("artifact.mlc_device_index",
                        result.config.mlc_device_index);
    payload.add_text("artifact.delivery_sha256", result.delivery_sha256);
    payload.add_text("artifact.machine_sha256", result.machine_sha256);

    statement delivery(
        db, "SELECT singleton, schema, abi_version, beam_number, "
            "source_axis_distance_cm, device_count, control_point_count, "
            "total_pair_count, boundary_count FROM delivery");
    if (!delivery.row()) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup delivery row is missing");
    }
    if (integer(delivery.get(), 0, "delivery singleton") != 1) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup delivery singleton differs");
    }
    const std::string stored_delivery_schema =
        text_value(delivery.get(), 1, "delivery schema");
    const uint32_t delivery_abi =
        unsigned_32(delivery.get(), 2, "delivery ABI version");
    const int64_t beam_number =
        integer(delivery.get(), 3, "delivery beam number");
    if (beam_number < std::numeric_limits<int32_t>::min() ||
        beam_number > std::numeric_limits<int32_t>::max()) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "delivery beam number exceeds int32");
    }
    result.delivery.beam_number = static_cast<int32_t>(beam_number);
    result.delivery.source_axis_distance_cm =
        real(delivery.get(), 4, "delivery source-axis distance");
    result.delivery.device_count =
        unsigned_32(delivery.get(), 5, "delivery device count");
    result.delivery.control_point_count =
        unsigned_32(delivery.get(), 6, "delivery control-point count");
    result.delivery.total_pair_count =
        unsigned_integer(delivery.get(), 7, "delivery total pair count");
    result.delivery.boundary_count =
        unsigned_integer(delivery.get(), 8, "delivery boundary count");
    require_done(delivery, "startup delivery table");
    if (stored_delivery_schema != delivery_schema ||
        delivery_abi != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup delivery schema or ABI differs");
    }
    payload.add_text("delivery.schema", stored_delivery_schema);
    payload.add_integer("delivery.abi_version", delivery_abi);
    payload.add_integer("delivery.beam_number", beam_number);
    payload.add_real("delivery.source_axis_distance_cm",
                     result.delivery.source_axis_distance_cm);
    payload.add_integer("delivery.device_count", result.delivery.device_count);
    payload.add_integer("delivery.control_point_count",
                        result.delivery.control_point_count);
    payload.add_integer("delivery.total_pair_count",
                        static_cast<int64_t>(result.delivery.total_pair_count));
    payload.add_integer("delivery.boundary_count",
                        static_cast<int64_t>(result.delivery.boundary_count));

    statement identifiers(
        db, "SELECT device_index, identifier FROM device_identifier "
            "ORDER BY device_index");
    uint32_t device_index = 0;
    while (identifiers.row()) {
        if (unsigned_32(identifiers.get(), 0, "device identifier index") !=
                device_index ||
            device_index >= result.delivery.device_count) {
            throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                             "startup device identifier indices differ");
        }
        payload.add_text("device_identifier." + std::to_string(device_index),
                         text_value(identifiers.get(), 1,
                                    "device identifier"));
        ++device_index;
    }
    if (device_index != result.delivery.device_count) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup device identifier count differs");
    }

    statement machine(
        db, "SELECT singleton, schema, abi_version, profile_identifier, "
            "profile_revision, leaf_pair_count, section_count, "
            "section_row_count, attenuation_point_count, "
            "calibration_point_count, source_axis_distance_cm, "
            "source_to_device_distance_cm, position_specification_distance_cm, "
            "material_density_g_cm3, calibration_mode, motion_axis, "
            "touching_pairs_bypass_curve, projection_ratio, "
            "physical_leaf_offset_cm, rounded_tip_maximum_thickness_cm, "
            "rounded_tip_radius_cm, rounded_tip_angle_deg, "
            "rounded_tip_transition_distance_cm, rounded_tip_section_count "
            "FROM machine");
    if (!machine.row()) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup machine row is missing");
    }
    if (integer(machine.get(), 0, "machine singleton") != 1) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup machine singleton differs");
    }
    const std::string stored_machine_schema =
        text_value(machine.get(), 1, "machine schema");
    const uint32_t machine_abi =
        unsigned_32(machine.get(), 2, "machine ABI version");
    const std::string profile_identifier =
        text_value(machine.get(), 3, "machine profile identifier");
    const std::string profile_revision =
        text_value(machine.get(), 4, "machine profile revision");
    result.machine.leaf_pair_count =
        unsigned_32(machine.get(), 5, "machine leaf pair count");
    result.machine.section_count =
        unsigned_32(machine.get(), 6, "machine section count");
    result.machine.section_row_count =
        unsigned_integer(machine.get(), 7, "machine section row count");
    result.machine.attenuation_point_count =
        unsigned_integer(machine.get(), 8, "machine attenuation point count");
    result.machine.calibration_point_count =
        unsigned_integer(machine.get(), 9, "machine calibration point count");
    result.machine.source_axis_distance_cm =
        real(machine.get(), 10, "machine source-axis distance");
    result.machine.source_to_device_distance_cm =
        real(machine.get(), 11, "machine source-to-device distance");
    result.machine.position_specification_distance_cm =
        real(machine.get(), 12, "machine position-specification distance");
    result.machine.material_density_g_cm3 =
        real(machine.get(), 13, "machine material density");
    const int64_t calibration_mode =
        integer(machine.get(), 14, "machine calibration mode");
    const int64_t motion_axis =
        integer(machine.get(), 15, "machine motion axis");
    const uint32_t touching_pairs =
        unsigned_32(machine.get(), 16, "machine touching-pairs flag");
    if (calibration_mode < std::numeric_limits<int32_t>::min() ||
        calibration_mode > std::numeric_limits<int32_t>::max() ||
        motion_axis < std::numeric_limits<int32_t>::min() ||
        motion_axis > std::numeric_limits<int32_t>::max() ||
        touching_pairs > 1) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup machine enum or flag differs");
    }
    result.machine.calibration_mode = static_cast<int32_t>(calibration_mode);
    result.machine.motion_axis = static_cast<int32_t>(motion_axis);
    result.machine.touching_pairs_bypass_curve = touching_pairs;
    result.machine.projection_ratio =
        real(machine.get(), 17, "machine projection ratio");
    result.machine.physical_leaf_offset_cm =
        real(machine.get(), 18, "machine physical leaf offset");
    result.machine.rounded_tip_maximum_thickness_cm =
        real(machine.get(), 19, "machine rounded-tip maximum thickness");
    result.machine.rounded_tip_radius_cm =
        real(machine.get(), 20, "machine rounded-tip radius");
    result.machine.rounded_tip_angle_deg =
        real(machine.get(), 21, "machine rounded-tip angle");
    result.machine.rounded_tip_transition_distance_cm =
        real(machine.get(), 22, "machine rounded-tip transition distance");
    result.machine.rounded_tip_section_count =
        unsigned_32(machine.get(), 23, "machine rounded-tip section count");
    require_done(machine, "startup machine table");
    if (stored_machine_schema != machine_schema ||
        machine_abi != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup machine schema or ABI differs");
    }
    payload.add_text("machine.schema", stored_machine_schema);
    payload.add_integer("machine.abi_version", machine_abi);
    payload.add_text("machine.profile_identifier", profile_identifier);
    payload.add_text("machine.profile_revision", profile_revision);
    payload.add_integer("machine.leaf_pair_count",
                        result.machine.leaf_pair_count);
    payload.add_integer("machine.section_count", result.machine.section_count);
    payload.add_integer("machine.section_row_count",
                        static_cast<int64_t>(result.machine.section_row_count));
    payload.add_integer(
        "machine.attenuation_point_count",
        static_cast<int64_t>(result.machine.attenuation_point_count));
    payload.add_integer(
        "machine.calibration_point_count",
        static_cast<int64_t>(result.machine.calibration_point_count));
    payload.add_real("machine.source_axis_distance_cm",
                     result.machine.source_axis_distance_cm);
    payload.add_real("machine.source_to_device_distance_cm",
                     result.machine.source_to_device_distance_cm);
    payload.add_real("machine.position_specification_distance_cm",
                     result.machine.position_specification_distance_cm);
    payload.add_real("machine.material_density_g_cm3",
                     result.machine.material_density_g_cm3);
    payload.add_integer("machine.calibration_mode", calibration_mode);
    payload.add_integer("machine.motion_axis", motion_axis);
    payload.add_integer("machine.touching_pairs_bypass_curve", touching_pairs);
    payload.add_real("machine.projection_ratio", result.machine.projection_ratio);
    payload.add_real("machine.physical_leaf_offset_cm",
                     result.machine.physical_leaf_offset_cm);
    payload.add_real("machine.rounded_tip_maximum_thickness_cm",
                     result.machine.rounded_tip_maximum_thickness_cm);
    payload.add_real("machine.rounded_tip_radius_cm",
                     result.machine.rounded_tip_radius_cm);
    payload.add_real("machine.rounded_tip_angle_deg",
                     result.machine.rounded_tip_angle_deg);
    payload.add_real("machine.rounded_tip_transition_distance_cm",
                     result.machine.rounded_tip_transition_distance_cm);
    payload.add_integer("machine.rounded_tip_section_count",
                        result.machine.rounded_tip_section_count);

    statement artifacts(
        db, "SELECT artifact_index, identifier, sha256 FROM machine_artifact "
            "ORDER BY artifact_index");
    uint64_t artifact_index = 0;
    while (artifacts.row()) {
        if (unsigned_integer(artifacts.get(), 0, "machine artifact index") !=
            artifact_index) {
            throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                             "startup machine artifact indices differ");
        }
        const std::string identifier =
            text_value(artifacts.get(), 1, "machine artifact identifier");
        const std::string digest =
            text_value(artifacts.get(), 2, "machine artifact SHA-256");
        if (!valid_sha256(digest)) {
            throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                             "startup machine artifact SHA-256 differs");
        }
        const std::string prefix =
            "machine_artifact." + std::to_string(artifact_index);
        payload.add_text(prefix + ".identifier", identifier);
        payload.add_text(prefix + ".sha256", digest);
        ++artifact_index;
    }

    statement array_count(db, "SELECT count(*) FROM native_array");
    if (!array_count.row() ||
        unsigned_integer(array_count.get(), 0, "native array row count") != 38) {
        throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                         "startup native array count differs");
    }
    require_done(array_count, "startup native array count");

    const uint64_t device_count = result.delivery.device_count;
    const uint64_t control_points = result.delivery.control_point_count;
    const uint64_t total_pairs = result.delivery.total_pair_count;
    load_array(db, payload, "delivery.device_kinds", "<i4", device_count,
               result.device_kinds);
    load_array(db, payload, "delivery.device_axes", "<i4", device_count,
               result.device_axes);
    load_array(db, payload, "delivery.device_pair_counts", "<u4",
               device_count, result.device_pair_counts);
    load_array(db, payload, "delivery.device_source_distances_cm", "<f8",
               device_count, result.device_source_distances_cm);
    load_array(db, payload, "delivery.device_source_distance_present", "|u1",
               device_count, result.device_source_distance_present);
    load_array(db, payload, "delivery.device_opening_offsets", "<u8",
               device_count + 1, result.device_opening_offsets);
    load_array(db, payload, "delivery.device_boundary_offsets", "<u8",
               device_count + 1, result.device_boundary_offsets);
    load_array(db, payload, "delivery.cumulative_meterset_fractions", "<f8",
               control_points, result.cumulative_meterset_fractions);
    load_array(db, payload, "delivery.gantry_angles_deg", "<f8",
               control_points, result.gantry_angles_deg);
    load_array(db, payload, "delivery.gantry_directions", "<i4",
               control_points, result.gantry_directions);
    load_array(db, payload, "delivery.collimator_angles_deg", "<f8",
               control_points, result.collimator_angles_deg);
    load_array(db, payload, "delivery.collimator_directions", "<i4",
               control_points, result.collimator_directions);
    load_array(db, payload, "delivery.patient_support_angles_deg", "<f8",
               control_points, result.patient_support_angles_deg);
    load_array(db, payload, "delivery.patient_support_directions", "<i4",
               control_points, result.patient_support_directions);
    load_array(db, payload, "delivery.isocenter_patient_lps_cm", "<f8",
               checked_product(control_points, 3, "delivery isocenter"),
               result.isocenter_patient_lps_cm);
    load_array(db, payload, "delivery.nominal_beam_energies_mev", "<f8",
               control_points, result.nominal_beam_energies_mev);
    load_array(db, payload, "delivery.dose_rates_mu_per_minute", "<f8",
               control_points, result.dose_rates_mu_per_minute);
    load_array(db, payload, "delivery.dose_rate_present", "|u1",
               control_points, result.dose_rate_present);
    load_array(db, payload, "delivery.tabletop_pitch_angles_deg", "<f8",
               control_points, result.tabletop_pitch_angles_deg);
    load_array(db, payload, "delivery.tabletop_roll_angles_deg", "<f8",
               control_points, result.tabletop_roll_angles_deg);
    const uint64_t position_count =
        checked_product(control_points, total_pairs, "delivery positions");
    load_array(db, payload, "delivery.bank_1_positions_cm", "<f8",
               position_count, result.bank_1_positions_cm);
    load_array(db, payload, "delivery.bank_2_positions_cm", "<f8",
               position_count, result.bank_2_positions_cm);
    load_array(db, payload, "delivery.mlc_boundaries_at_isocenter_cm", "<f8",
               result.delivery.boundary_count,
               result.mlc_boundaries_at_isocenter_cm);

    const uint64_t sections = result.machine.section_count;
    const uint64_t rows = result.machine.section_row_count;
    const uint64_t attenuation = result.machine.attenuation_point_count;
    const uint64_t calibration = result.machine.calibration_point_count;
    load_array(db, payload, "machine.section_z_enter_cm", "<f8", sections,
               result.section_z_enter_cm);
    load_array(db, payload, "machine.section_z_center_cm", "<f8", sections,
               result.section_z_center_cm);
    load_array(db, payload, "machine.section_z_exit_cm", "<f8", sections,
               result.section_z_exit_cm);
    load_array(db, payload, "machine.section_row_offsets", "<u8", sections + 1,
               result.section_row_offsets);
    load_array(db, payload, "machine.section_y_start_cm", "<f8", rows,
               result.section_y_start_cm);
    load_array(db, payload, "machine.section_y_end_cm", "<f8", rows,
               result.section_y_end_cm);
    load_array(db, payload, "machine.section_leaf_pair_indices", "<u4", rows,
               result.section_leaf_pair_indices);
    load_array(db, payload, "machine.section_thickness_cm", "<f8", rows,
               result.section_thickness_cm);
    load_array(db, payload, "machine.section_groove_thickness_cm", "<f8", rows,
               result.section_groove_thickness_cm);
    load_array(db, payload, "machine.section_groove_distance_from_tip_cm",
               "<f8", rows, result.section_groove_distance_from_tip_cm);
    load_array(db, payload, "machine.attenuation_energy_mev", "<f8",
               attenuation, result.attenuation_energy_mev);
    load_array(db, payload, "machine.total_linear_attenuation_per_cm", "<f8",
               attenuation, result.total_linear_attenuation_per_cm);
    load_array(db, payload, "machine.incoherent_linear_attenuation_per_cm",
               "<f8", attenuation,
               result.incoherent_linear_attenuation_per_cm);
    load_array(db, payload, "machine.calibration_specified_position_cm", "<f8",
               calibration, result.calibration_specified_position_cm);
    load_array(db, payload, "machine.calibration_actual_projected_position_cm",
               "<f8", calibration,
               result.calibration_actual_projected_position_cm);

    result.delivery.device_kinds = result.device_kinds.data();
    result.delivery.device_axes = result.device_axes.data();
    result.delivery.device_pair_counts = result.device_pair_counts.data();
    result.delivery.device_source_distances_cm =
        result.device_source_distances_cm.data();
    result.delivery.device_source_distance_present =
        result.device_source_distance_present.data();
    result.delivery.device_opening_offsets =
        result.device_opening_offsets.data();
    result.delivery.device_boundary_offsets =
        result.device_boundary_offsets.data();
    result.delivery.cumulative_meterset_fractions =
        result.cumulative_meterset_fractions.data();
    result.delivery.gantry_angles_deg = result.gantry_angles_deg.data();
    result.delivery.gantry_directions = result.gantry_directions.data();
    result.delivery.collimator_angles_deg = result.collimator_angles_deg.data();
    result.delivery.collimator_directions =
        result.collimator_directions.data();
    result.delivery.patient_support_angles_deg =
        result.patient_support_angles_deg.data();
    result.delivery.patient_support_directions =
        result.patient_support_directions.data();
    result.delivery.isocenter_patient_lps_cm =
        result.isocenter_patient_lps_cm.data();
    result.delivery.nominal_beam_energies_mev =
        result.nominal_beam_energies_mev.data();
    result.delivery.dose_rates_mu_per_minute =
        result.dose_rates_mu_per_minute.data();
    result.delivery.dose_rate_present = result.dose_rate_present.data();
    result.delivery.tabletop_pitch_angles_deg =
        result.tabletop_pitch_angles_deg.data();
    result.delivery.tabletop_roll_angles_deg =
        result.tabletop_roll_angles_deg.data();
    result.delivery.bank_1_positions_cm = result.bank_1_positions_cm.data();
    result.delivery.bank_2_positions_cm = result.bank_2_positions_cm.data();
    result.delivery.mlc_boundaries_at_isocenter_cm =
        result.mlc_boundaries_at_isocenter_cm.data();

    result.machine.section_z_enter_cm = result.section_z_enter_cm.data();
    result.machine.section_z_center_cm = result.section_z_center_cm.data();
    result.machine.section_z_exit_cm = result.section_z_exit_cm.data();
    result.machine.section_row_offsets = result.section_row_offsets.data();
    result.machine.section_y_start_cm = result.section_y_start_cm.data();
    result.machine.section_y_end_cm = result.section_y_end_cm.data();
    result.machine.section_leaf_pair_indices =
        result.section_leaf_pair_indices.data();
    result.machine.section_thickness_cm = result.section_thickness_cm.data();
    result.machine.section_groove_thickness_cm =
        result.section_groove_thickness_cm.data();
    result.machine.section_groove_distance_from_tip_cm =
        result.section_groove_distance_from_tip_cm.data();
    result.machine.attenuation_energy_mev = result.attenuation_energy_mev.data();
    result.machine.total_linear_attenuation_per_cm =
        result.total_linear_attenuation_per_cm.data();
    result.machine.incoherent_linear_attenuation_per_cm =
        result.incoherent_linear_attenuation_per_cm.data();
    result.machine.calibration_specified_position_cm =
        result.calibration_specified_position_cm.data();
    result.machine.calibration_actual_projected_position_cm =
        result.calibration_actual_projected_position_cm.data();
    return result;
}

void copy_hash(char *destination, const std::string &source) {
    std::memcpy(destination, source.c_str(), source.size() + 1);
}
}  // namespace

extern "C" int32_t mcdose_particle_dmlc_create_producer_from_startup_v1(
    const char *path, mcdose_particle_dmlc_producer_context_v1 **context,
    mcdose_particle_dmlc_startup_info_v1 *info, char *diagnostic,
    size_t diagnostic_capacity) {
    if (diagnostic != nullptr && diagnostic_capacity > 0) {
        diagnostic[0] = '\0';
    }
    if (path == nullptr || context == nullptr) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_INVALID_ARGUMENT, diagnostic,
                    diagnostic_capacity,
                    "startup loader contains a null required pointer");
    }
    *context = nullptr;
    if (info != nullptr &&
        (info->abi_version != MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION ||
         info->struct_size < sizeof(*info))) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ABI_MISMATCH, diagnostic,
                    diagnostic_capacity,
                    "startup info ABI version or size differs");
    }
    const uint16_t endian_probe = 1;
    if (*reinterpret_cast<const uint8_t *>(&endian_probe) != 1) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "startup loader version 1 requires a little-endian host");
    }
    try {
        database source(path);
        validate_database(source.get());
        payload_hash payload;
        loaded_startup loaded = load_startup(source.get(), payload);
        const std::string actual_payload_sha256 = payload.finish();
        if (actual_payload_sha256 != loaded.expected_payload_sha256) {
            throw load_error(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED,
                             "startup payload SHA-256 differs");
        }
        const int32_t status = mcdose_particle_dmlc_create_producer_context_v1(
            &loaded.delivery, &loaded.machine, &loaded.config, context,
            diagnostic, diagnostic_capacity);
        if (status != MCDOSE_PARTICLE_DMLC_STATUS_OK) {
            return status;
        }
        if (info != nullptr) {
            mcdose_particle_dmlc_startup_info_v1 result = {};
            result.abi_version = MCDOSE_PARTICLE_DMLC_NATIVE_ABI_VERSION;
            result.struct_size = sizeof(result);
            result.beam_number = loaded.delivery.beam_number;
            result.mlc_device_index = loaded.config.mlc_device_index;
            result.device_count = loaded.delivery.device_count;
            result.control_point_count = loaded.delivery.control_point_count;
            result.leaf_pair_count = loaded.machine.leaf_pair_count;
            result.generated_electron_policy =
                loaded.config.generated_electron_policy;
            copy_hash(result.delivery_sha256, loaded.delivery_sha256);
            copy_hash(result.machine_sha256, loaded.machine_sha256);
            copy_hash(result.payload_sha256, actual_payload_sha256);
            *info = result;
        }
        return MCDOSE_PARTICLE_DMLC_STATUS_OK;
    } catch (const load_error &error) {
        return fail(error.status(), diagnostic, diagnostic_capacity,
                    error.what());
    } catch (const std::bad_alloc &) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "startup loader allocation failed");
    } catch (const std::length_error &) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_ALLOCATION_FAILED, diagnostic,
                    diagnostic_capacity,
                    "startup loader array is too large");
    } catch (const std::exception &error) {
        return fail(MCDOSE_PARTICLE_DMLC_STATUS_VALIDATION_FAILED, diagnostic,
                    diagnostic_capacity, error.what());
    }
}
