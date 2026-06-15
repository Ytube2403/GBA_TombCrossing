#ifndef SFX_PLAYER_H
#define SFX_PLAYER_H

#include "bn_fixed.h"
#include <cstdint>

namespace game {

enum class SfxCue : uint8_t {
    UI_MOVE,
    UI_CONFIRM,
    UI_BACK,
    UI_DENIED,
    PLAYER_JUMP,
    PLAYER_LAND,
    PLAYER_DEATH,
    SKILL_ACTIVATE,
    ROOM_CLEAR,
    GRAVITY_FLIP,
    TRAP_WARNING,
    PLATE_CLICK,
    PROJECTILE_FIRE,
    SPIKE_EXTEND,
    STONE_MOVE,
    STONE_IMPACT,
    FLOOR_CRACK,
    FLOOR_BREAK,
    LAVA_RISE,
    COUNT
};

class SfxPlayer {
public:
    static void update();
    static void reset();
    static bool play(SfxCue cue, bn::fixed speed = 1, bn::fixed panning = 0);
};

} // namespace game

#endif
