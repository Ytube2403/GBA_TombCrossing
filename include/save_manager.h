#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include "constants.h"
#include <cstdint>

namespace game {

constexpr int PROFILE_COUNT = 3;
constexpr int PROFILE_NAME_LENGTH = 8;
constexpr int ROOM_CLEAR_BYTES = (TOTAL_LEVELS + 7) / 8;
constexpr uint8_t PROFILE_VALID_MARKER = 0xA5;
constexpr uint8_t SAVE_VERSION = 2;

enum class FlashMode : uint8_t { FULL, REDUCED, OFF };
enum class TextSpeed : uint8_t { SLOW, NORMAL, FAST, INSTANT };

struct [[gnu::packed]] GlobalSettings {
    uint8_t music_volume = 4;
    uint8_t sfx_volume = 4;
    uint8_t flash_mode = uint8_t(FlashMode::FULL);
    uint8_t text_speed = uint8_t(TextSpeed::NORMAL);
    uint8_t window_palette = 0;
};

struct [[gnu::packed]] ProfileData {
    uint8_t valid_marker;
    char name[PROFILE_NAME_LENGTH + 1];
    uint8_t next_room;
    uint8_t completed;
    uint8_t room_clear[ROOM_CLEAR_BYTES];
    uint32_t total_deaths;
    uint32_t play_frames;
    uint16_t level_deaths[TOTAL_LEVELS];
    uint16_t best_level_deaths[TOTAL_LEVELS];
    uint8_t preferred_skill;
    uint8_t reserved[3];
    uint16_t checksum;
};

struct [[gnu::packed]] SaveBankV2 {
    char magic[4];
    uint8_t version;
    uint8_t active_slot;
    uint8_t notice_flags;
    uint8_t reserved;
    uint32_t generation;
    GlobalSettings settings;
    ProfileData profiles[PROFILE_COUNT];
    uint32_t checksum;
};

enum class SaveLoadResult : uint8_t {
    LOADED,
    FRESH,
    MIGRATED,
    RECOVERED
};

class SaveManager {
public:
    SaveLoadResult load();
    void commit();

    bool has_profile(int slot) const;
    bool profile_damaged(int slot) const;
    const ProfileData& profile(int slot) const;
    ProfileData& profile(int slot);
    void create_profile(int slot, const char* name);
    void delete_profile(int slot);
    void select_profile(int slot);
    int active_slot() const { return _bank.active_slot < PROFILE_COUNT ? _bank.active_slot : -1; }
    ProfileData* active_profile();
    const ProfileData* active_profile() const;

    void record_death(int room);
    bool mark_room_cleared(int room, int deaths);
    bool room_cleared(int slot, int room) const;
    int cleared_count(int slot, int world = -1) const;
    int highest_unlocked_room(int slot) const;

    GlobalSettings& settings() { return _bank.settings; }
    const GlobalSettings& settings() const { return _bank.settings; }
    bool migration_notice_pending() const { return (_bank.notice_flags & 1) != 0; }
    void acknowledge_migration();
    bool unlock_all_rooms(int up_to_room);

private:
    SaveBankV2 _bank = {};
    int _active_bank = -1;
    bool _damaged[PROFILE_COUNT] = {};

    static uint16_t profile_checksum(const ProfileData& profile);
    static uint32_t bank_checksum(const SaveBankV2& bank);
    static bool valid_profile(const ProfileData& profile);
    static bool valid_bank(const SaveBankV2& bank);
    static void reset_profile(ProfileData& profile);
    static void copy_name(char* destination, const char* source);
    void initialize_fresh();
    bool migrate_legacy();
    void repair_profiles();
};

static_assert(sizeof(SaveBankV2) < 2048, "Save bank must fit in its SRAM partition");

} // namespace game

#endif
