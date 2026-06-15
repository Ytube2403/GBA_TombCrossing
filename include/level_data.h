#ifndef LEVEL_DATA_H
#define LEVEL_DATA_H

#include <cstdint>
#include "constants.h"

namespace game {

struct TrapPlacement {
    TrapType type;
    uint8_t tile_x;
    uint8_t tile_y;
    uint8_t param_a;
    uint8_t param_b;
};

enum class ChamberTransitionDirection : uint8_t {
    LEFT = 0,
    RIGHT = 1,
    DOWN = 2
};

struct ChamberTransition {
    uint8_t tile_x;
    uint8_t tile_y;
    uint8_t width;
    uint8_t height;
    uint8_t target_chamber;
    uint8_t target_x;
    uint8_t target_y;
    ChamberTransitionDirection direction;
};

struct ChamberDef {
    uint16_t tilemap[LEVEL_ROWS][LEVEL_COLS];
    TrapPlacement traps[MAX_TRAPS_PER_LEVEL];
    ChamberTransition transitions[MAX_TRANSITIONS_PER_CHAMBER];
    uint8_t trap_count;
    uint8_t transition_count;
    uint8_t spawn_x;
    uint8_t spawn_y;
    uint8_t exit_x;
    uint8_t exit_y;
    bool has_exit;
};

struct LevelDef {
    const ChamberDef* chambers;
    uint8_t chamber_count;
    uint8_t start_chamber;
    uint8_t world_id;
    uint8_t level_index;
    const char* name;
};

extern const LevelDef WORLD1_LEVELS[LEVELS_PER_WORLD];
extern const LevelDef WORLD2_LEVELS[LEVELS_PER_WORLD];
extern const LevelDef WORLD3_LEVELS[LEVELS_PER_WORLD];
extern const LevelDef WORLD4_LEVELS[LEVELS_PER_WORLD];
extern const LevelDef WORLD5_LEVELS[LEVELS_PER_WORLD];

inline const LevelDef& get_level(int index)
{
    const LevelDef* tables[5] = {
        WORLD1_LEVELS, WORLD2_LEVELS, WORLD3_LEVELS,
        WORLD4_LEVELS, WORLD5_LEVELS
    };
    int world = index / LEVELS_PER_WORLD;
    int local = index % LEVELS_PER_WORLD;
    return tables[world][local];
}

} // namespace game

#endif
