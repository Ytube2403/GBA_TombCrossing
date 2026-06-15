#include "save_manager.h"
#include "bn_sram.h"
#include "bn_algorithm.h"

namespace game {

namespace {
    constexpr int BANK_OFFSETS[2] = { 0, 2048 };
    constexpr char BANK_MAGIC[4] = { 'T', 'C', 'B', '2' };

    struct [[gnu::packed]] LegacySaveData {
        char magic[4];
        uint8_t highest_level;
        uint8_t world_unlocked;
        uint8_t skills_unlocked;
        uint8_t preferred_skill;
        uint32_t total_deaths;
        uint16_t level_deaths[TOTAL_LEVELS];
        uint8_t checksum;
    };

    static_assert(sizeof(LegacySaveData) == 213, "Unexpected legacy save size");

    uint8_t legacy_checksum(const LegacySaveData& save)
    {
        uint8_t result = 0;
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&save);
        for(int index = 0; index < int(sizeof(LegacySaveData)) - 1; ++index)
        {
            result ^= bytes[index];
        }
        return result;
    }
}

uint16_t SaveManager::profile_checksum(const ProfileData& profile)
{
    uint16_t result = 0x4B1D;
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&profile);
    for(int index = 0; index < int(sizeof(ProfileData)) - 2; ++index)
    {
        result = uint16_t((result << 5) | (result >> 11));
        result ^= bytes[index];
    }
    return result;
}

uint32_t SaveManager::bank_checksum(const SaveBankV2& bank)
{
    uint32_t result = 2166136261u;
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&bank);
    for(int index = 0; index < int(sizeof(SaveBankV2)) - 4; ++index)
    {
        result ^= bytes[index];
        result *= 16777619u;
    }
    return result;
}

bool SaveManager::valid_profile(const ProfileData& profile)
{
    return profile.valid_marker == PROFILE_VALID_MARKER &&
           profile.name[0] != '\0' &&
           profile.checksum == profile_checksum(profile);
}

bool SaveManager::valid_bank(const SaveBankV2& bank)
{
    return bank.magic[0] == BANK_MAGIC[0] &&
           bank.magic[1] == BANK_MAGIC[1] &&
           bank.magic[2] == BANK_MAGIC[2] &&
           bank.magic[3] == BANK_MAGIC[3] &&
           bank.version == SAVE_VERSION &&
           bank.checksum == bank_checksum(bank);
}

void SaveManager::reset_profile(ProfileData& profile)
{
    profile = {};
    for(int room = 0; room < TOTAL_LEVELS; ++room)
    {
        profile.best_level_deaths[room] = 0xFFFF;
    }
}

void SaveManager::copy_name(char* destination, const char* source)
{
    int index = 0;
    for(; index < PROFILE_NAME_LENGTH && source[index]; ++index)
    {
        char value = source[index];
        destination[index] = value >= 'a' && value <= 'z' ? char(value - 'a' + 'A') : value;
    }
    destination[index] = '\0';
    for(++index; index <= PROFILE_NAME_LENGTH; ++index)
    {
        destination[index] = '\0';
    }
}

void SaveManager::initialize_fresh()
{
    _bank = {};
    _bank.magic[0] = BANK_MAGIC[0];
    _bank.magic[1] = BANK_MAGIC[1];
    _bank.magic[2] = BANK_MAGIC[2];
    _bank.magic[3] = BANK_MAGIC[3];
    _bank.version = SAVE_VERSION;
    _bank.active_slot = 0xFF;
    _bank.settings = GlobalSettings();
    for(ProfileData& profile : _bank.profiles)
    {
        reset_profile(profile);
    }
    _active_bank = -1;
}

bool SaveManager::migrate_legacy()
{
    LegacySaveData legacy = {};
    bn::sram::read(legacy);
    if(legacy.magic[0] != SAVE_MAGIC[0] || legacy.magic[1] != SAVE_MAGIC[1] ||
       legacy.magic[2] != SAVE_MAGIC[2] || legacy.magic[3] != SAVE_MAGIC[3] ||
       legacy.checksum != legacy_checksum(legacy))
    {
        return false;
    }

    initialize_fresh();
    create_profile(0, "KHA");
    ProfileData& migrated = _bank.profiles[0];
    migrated.next_room = legacy.highest_level > TOTAL_LEVELS ? TOTAL_LEVELS : legacy.highest_level;
    migrated.completed = migrated.next_room >= TOTAL_LEVELS;
    migrated.total_deaths = legacy.total_deaths;
    migrated.preferred_skill = legacy.preferred_skill;
    for(int room = 0; room < TOTAL_LEVELS; ++room)
    {
        migrated.level_deaths[room] = legacy.level_deaths[room];
        if(room < migrated.next_room)
        {
            migrated.room_clear[room / 8] |= uint8_t(1u << (room % 8));
            migrated.best_level_deaths[room] = legacy.level_deaths[room];
        }
    }
    migrated.checksum = profile_checksum(migrated);
    _bank.active_slot = 0;
    _bank.notice_flags |= 1;
    return true;
}

void SaveManager::repair_profiles()
{
    for(int slot = 0; slot < PROFILE_COUNT; ++slot)
    {
        ProfileData& profile_data = _bank.profiles[slot];
        _damaged[slot] = profile_data.valid_marker == PROFILE_VALID_MARKER && !valid_profile(profile_data);
        if(profile_data.valid_marker != PROFILE_VALID_MARKER)
        {
            reset_profile(profile_data);
        }
    }
    if(_bank.active_slot >= PROFILE_COUNT || !valid_profile(_bank.profiles[_bank.active_slot]))
    {
        _bank.active_slot = 0xFF;
    }
}

SaveLoadResult SaveManager::load()
{
    SaveBankV2 banks[2] = {};
    bn::sram::read_offset(banks[0], BANK_OFFSETS[0]);
    bn::sram::read_offset(banks[1], BANK_OFFSETS[1]);
    bool valid0 = valid_bank(banks[0]);
    bool valid1 = valid_bank(banks[1]);

    if(valid0 || valid1)
    {
        if(valid0 && valid1)
        {
            _active_bank = banks[1].generation > banks[0].generation ? 1 : 0;
        }
        else
        {
            _active_bank = valid1 ? 1 : 0;
        }
        _bank = banks[_active_bank];
        repair_profiles();
        return valid0 && valid1 ? SaveLoadResult::LOADED : SaveLoadResult::RECOVERED;
    }

    if(migrate_legacy())
    {
        commit();
        commit();
        return SaveLoadResult::MIGRATED;
    }

    initialize_fresh();
    commit();
    commit();
    return SaveLoadResult::FRESH;
}

void SaveManager::commit()
{
    for(ProfileData& profile_data : _bank.profiles)
    {
        if(profile_data.valid_marker == PROFILE_VALID_MARKER && !_damaged[&profile_data - _bank.profiles])
        {
            profile_data.checksum = profile_checksum(profile_data);
        }
    }
    ++_bank.generation;
    _bank.checksum = bank_checksum(_bank);
    int target_bank = _active_bank == 0 ? 1 : 0;
    if(_active_bank < 0)
    {
        target_bank = 1; // Preserve a possible legacy save until migration has completed.
    }
    bn::sram::write_offset(_bank, BANK_OFFSETS[target_bank]);
    _active_bank = target_bank;
}

bool SaveManager::has_profile(int slot) const
{
    if(slot < 0 || slot >= PROFILE_COUNT || _damaged[slot])
    {
        return false;
    }
    const ProfileData& profile_data = _bank.profiles[slot];
    return profile_data.valid_marker == PROFILE_VALID_MARKER && profile_data.name[0] != '\0';
}

bool SaveManager::profile_damaged(int slot) const
{
    return slot >= 0 && slot < PROFILE_COUNT && _damaged[slot];
}

const ProfileData& SaveManager::profile(int slot) const
{
    return _bank.profiles[slot];
}

ProfileData& SaveManager::profile(int slot)
{
    return _bank.profiles[slot];
}

void SaveManager::create_profile(int slot, const char* name)
{
    if(slot < 0 || slot >= PROFILE_COUNT)
    {
        return;
    }
    ProfileData& profile_data = _bank.profiles[slot];
    reset_profile(profile_data);
    profile_data.valid_marker = PROFILE_VALID_MARKER;
    copy_name(profile_data.name, name && name[0] ? name : "KHA");
    profile_data.checksum = profile_checksum(profile_data);
    _damaged[slot] = false;
    _bank.active_slot = uint8_t(slot);
}

void SaveManager::delete_profile(int slot)
{
    if(slot < 0 || slot >= PROFILE_COUNT)
    {
        return;
    }
    reset_profile(_bank.profiles[slot]);
    _damaged[slot] = false;
    if(_bank.active_slot == slot)
    {
        _bank.active_slot = 0xFF;
    }
    commit();
}

void SaveManager::select_profile(int slot)
{
    if(has_profile(slot))
    {
        _bank.active_slot = uint8_t(slot);
    }
}

ProfileData* SaveManager::active_profile()
{
    int slot = active_slot();
    return slot >= 0 && has_profile(slot) ? &_bank.profiles[slot] : nullptr;
}

const ProfileData* SaveManager::active_profile() const
{
    int slot = active_slot();
    return slot >= 0 && has_profile(slot) ? &_bank.profiles[slot] : nullptr;
}

void SaveManager::record_death(int room)
{
    ProfileData* profile_data = active_profile();
    if(!profile_data || room < 0 || room >= TOTAL_LEVELS)
    {
        return;
    }
    if(profile_data->total_deaths < 0xFFFFFFFFu)
    {
        ++profile_data->total_deaths;
    }
    if(profile_data->level_deaths[room] < 0xFFFF)
    {
        ++profile_data->level_deaths[room];
    }
}

bool SaveManager::mark_room_cleared(int room, int deaths)
{
    ProfileData* profile_data = active_profile();
    if(!profile_data || room < 0 || room >= TOTAL_LEVELS)
    {
        return false;
    }
    bool first_clear = !room_cleared(active_slot(), room);
    profile_data->room_clear[room / 8] |= uint8_t(1u << (room % 8));
    uint16_t deaths_value = uint16_t(deaths < 0 ? 0 : deaths > 0xFFFF ? 0xFFFF : deaths);
    if(profile_data->best_level_deaths[room] == 0xFFFF || deaths_value < profile_data->best_level_deaths[room])
    {
        profile_data->best_level_deaths[room] = deaths_value;
    }
    int next_room = room + 1;
    if(next_room > profile_data->next_room)
    {
        profile_data->next_room = uint8_t(next_room);
    }
    if(next_room >= TOTAL_LEVELS)
    {
        profile_data->completed = 1;
        profile_data->next_room = TOTAL_LEVELS;
    }
    return first_clear;
}

bool SaveManager::room_cleared(int slot, int room) const
{
    if(!has_profile(slot) || room < 0 || room >= TOTAL_LEVELS)
    {
        return false;
    }
    return (_bank.profiles[slot].room_clear[room / 8] & uint8_t(1u << (room % 8))) != 0;
}

int SaveManager::cleared_count(int slot, int world) const
{
    if(!has_profile(slot))
    {
        return 0;
    }
    int first = world >= 0 ? world * LEVELS_PER_WORLD : 0;
    int last = world >= 0 ? first + LEVELS_PER_WORLD : TOTAL_LEVELS;
    int result = 0;
    for(int room = first; room < last; ++room)
    {
        if(room_cleared(slot, room))
        {
            ++result;
        }
    }
    return result;
}

int SaveManager::highest_unlocked_room(int slot) const
{
    return has_profile(slot) ? bn::min(int(_bank.profiles[slot].next_room), TOTAL_LEVELS) : 0;
}

void SaveManager::acknowledge_migration()
{
    if(migration_notice_pending())
    {
        _bank.notice_flags &= uint8_t(~1u);
        commit();
    }
}

bool SaveManager::unlock_all_rooms(int up_to_room)
{
    ProfileData* profile_data = active_profile();
    if(!profile_data || up_to_room <= 0)
    {
        return false;
    }
    int limit = up_to_room > TOTAL_LEVELS ? TOTAL_LEVELS : up_to_room;
    for(int room = 0; room < limit; ++room)
    {
        profile_data->room_clear[room / 8] |= uint8_t(1u << (room % 8));
        if(profile_data->best_level_deaths[room] == 0xFFFF)
        {
            profile_data->best_level_deaths[room] = 0;
        }
    }
    profile_data->next_room = uint8_t(limit);
    if(limit >= TOTAL_LEVELS)
    {
        profile_data->completed = 1;
    }
    profile_data->checksum = profile_checksum(*profile_data);
    commit();
    return true;
}

} // namespace game
