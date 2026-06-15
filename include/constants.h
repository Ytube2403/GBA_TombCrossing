#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "bn_fixed.h"

namespace game {

// ─────────────────────────────────────────────
// Screen dimensions (GBA native: 240×160)
// ─────────────────────────────────────────────
constexpr int SCREEN_WIDTH  = 240;
constexpr int SCREEN_HEIGHT = 160;
constexpr int UI_COLS       = SCREEN_WIDTH / 8;
constexpr int UI_ROWS       = SCREEN_HEIGHT / 8;
constexpr int UI_SAFE_LEFT  = 1;
constexpr int UI_SAFE_RIGHT = UI_COLS - 2;
constexpr int UI_SAFE_TOP   = 1;
constexpr int UI_SAFE_BOTTOM = UI_ROWS - 2;

// Tile dimensions (GBA standard tile: 8×8 px)
constexpr int TILE_SIZE     = 8;

// Level grid: 30 tiles wide × 18 tiles tall (144px gameplay + 16px HUD)
constexpr int LEVEL_COLS    = 30;
constexpr int LEVEL_ROWS    = 18;
constexpr int MAX_CHAMBERS_PER_LEVEL = 4;
constexpr int MAX_TRANSITIONS_PER_CHAMBER = 8;

// GBA coordinate system: (0,0) = screen center in Butano
constexpr bn::fixed SCREEN_LEFT   = -120;  // -SCREEN_WIDTH  / 2
constexpr bn::fixed SCREEN_RIGHT  =  120;  //  SCREEN_WIDTH  / 2
constexpr bn::fixed SCREEN_TOP    =  -80;  // -SCREEN_HEIGHT / 2
constexpr bn::fixed SCREEN_BOTTOM =   80;  //  SCREEN_HEIGHT / 2

// HUD occupies the top 16px; gameplay area is 240×144
constexpr int HUD_HEIGHT       = 16;
constexpr int GAMEPLAY_HEIGHT  = SCREEN_HEIGHT - HUD_HEIGHT;  // 144 px
constexpr bn::fixed GAMEPLAY_TOP    = SCREEN_TOP + HUD_HEIGHT;  // -64
constexpr bn::fixed GAMEPLAY_BOTTOM = SCREEN_BOTTOM;            //  80

// ─────────────────────────────────────────────
// Player physics
// ─────────────────────────────────────────────
namespace physics {
    constexpr uint16_t PHYSICS_VERSION = 3;
    constexpr int       PLAYER_HALF_WIDTH = 7;
    constexpr int       PLAYER_HALF_HEIGHT = 13;
    constexpr bn::fixed GRAVITY         = bn::fixed(0.3);
    constexpr bn::fixed JUMP_FORCE      = bn::fixed(-4.5);
    constexpr bn::fixed MOVE_SPEED      = bn::fixed(1.5);
    constexpr bn::fixed MAX_FALL_SPEED  = bn::fixed(4.0);
    constexpr int       COYOTE_FRAMES   = 6;   // grace frames after leaving a ledge
    constexpr int       JUMP_BUFFER_FRAMES = 8; // frames to buffer a jump input
}

// ─────────────────────────────────────────────
// Tile IDs — must match Tiled tileset numbering
// ─────────────────────────────────────────────
namespace tile {
    constexpr uint16_t EMPTY           = 0;
    constexpr uint16_t SOLID           = 1;   // standard platform / wall
    constexpr uint16_t SPIKE_UP        = 2;   // static spike (points up)
    constexpr uint16_t SPIKE_DOWN      = 3;   // static spike (points down)
    constexpr uint16_t LADDER          = 4;   // climbable ladder
    constexpr uint16_t VANISH          = 5;   // vanish-floor tile
    constexpr uint16_t CONVEYOR_LEFT   = 6;
    constexpr uint16_t CONVEYOR_RIGHT  = 7;
    constexpr uint16_t EXIT            = 8;   // real exit door
    constexpr uint16_t SPAWN           = 9;   // player spawn point (not rendered)
    constexpr uint16_t INVISIBLE_WALL  = 15;  // solid but completely invisible tile
    // 10–14 reserved for theme decorative tiles
}

// ─────────────────────────────────────────────
// Trap type IDs — matches TrapPlacement::type
// ─────────────────────────────────────────────
enum class TrapType : uint8_t {
    // Universal traps
    VANISH_FLOOR    = 0,
    POP_SPIKE       = 1,
    FALLING_BLOCK   = 2,
    MOVING_PLATFORM = 3,
    CONVEYOR        = 4,
    GRAVITY_FLIP    = 5,
    FAKE_DOOR       = 6,
    DARK_ROOM       = 7,
    INVISIBLE_WALL  = 8,
    WIND            = 9,
    // World 1 – Egyptian
    SAND_SINKHOLE   = 10,
    BOULDER         = 11,
    SARCOPHAGUS_POP = 12,
    // World 2 – Royal
    PENDULUM_AXE    = 13,
    ARROW_WALL      = 14,
    // World 3 – Qin
    TERRACOTTA_SOLDIER = 15,
    PRESSURE_PLATE  = 16,
    // World 4 – Aztec
    DART_FROG       = 17,
    LAVA_RISE       = 18,
    CRUMBLE_FLOOR   = 19,
    // World 5 – Japanese Kofun
    BAMBOO_SPIKE    = 20,
    SAMURAI_STATUE  = 21,
    PAPER_FLOOR     = 22,
    BOUNCE_BOULDER  = 23,
    AUTO_BOULDER    = 24,
    COUNT
};

// ─────────────────────────────────────────────
// Game states
// ─────────────────────────────────────────────
enum class GameState {
    TITLE = 0,
    WORLD_SELECT = 1,
    PLAYING = 2,
    PAUSED = 3,
    DEAD = 4,
    LEVEL_CLEAR = 5,    // Room result
    WORLD_INTRO = 6,
    GAME_COMPLETE = 7,
    SKILL_SELECT = 8,
    BOOT = 9,
    PROFILE_SELECT = 10,
    PROFILE_CREATE = 11,
    PROFILE_MENU = 12,
    ROOM_SELECT = 13,
    WORLD_RESULT = 14,
    ENDING = 15,
    CREDITS = 16,
    STATS = 17,
    OPTIONS = 18,
    SAVE_RECOVERY = 19,
    CONFIRM_DIALOG = 20,
};

// ─────────────────────────────────────────────
// World IDs
// ─────────────────────────────────────────────
enum class WorldId : uint8_t {
    EGYPT   = 0,  // Maps  1–20
    ROYAL   = 1,  // Maps 21–40
    QIN     = 2,  // Maps 41–60
    AZTEC   = 3,  // Maps 61–80
    KOFUN   = 4,  // Maps 81–100
    COUNT   = 5
};

// ─────────────────────────────────────────────
// Player animation states
// ─────────────────────────────────────────────
enum class PlayerAnim : uint8_t {
    IDLE      = 0,
    WALK      = 1,
    JUMP_UP   = 2,
    FALL      = 3,
    LAND      = 4,
    DIE       = 5,
    WIN       = 6,
    PUSH      = 7,
    COUNT
};

// ─────────────────────────────────────────────
// Game constants
// ─────────────────────────────────────────────
constexpr int TOTAL_LEVELS          = 100;
constexpr int LEVELS_PER_WORLD      = 20;
constexpr int MAX_TRAPS_PER_LEVEL   = 32;
constexpr int ACTIVE_WORLDS          = 2;   // Worlds 3-5 are "Coming Soon"
constexpr int MAX_DEATH_COUNT       = 9999; // display cap

// SRAM magic for save validation
constexpr char SAVE_MAGIC[4] = {'T', 'C', 'R', 'S'};

} // namespace game

#endif // CONSTANTS_H
