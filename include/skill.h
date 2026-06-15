#ifndef SKILL_H
#define SKILL_H

#include "bn_fixed.h"
#include <cstdint>

namespace game {

enum class SkillType : uint8_t {
    NONE          = 0,
    SPRING_BOOTS  = 1,  // World 2+: higher jump
    DASH_CLOAK    = 2,  // World 2+: air dash
    SHRINK_POTION = 3,  // World 3+: become tiny
    COUNT         = 4
};

struct SkillState {
    SkillType selected = SkillType::NONE;
    bool      used     = false;
    bool      active   = false;
    uint16_t  timer    = 0;

    void reset() { selected = SkillType::NONE; used = false; active = false; timer = 0; }
    void select(SkillType s) { selected = s; used = false; active = false; timer = 0; }
    bool can_activate() const { return selected != SkillType::NONE && !used; }
};

struct SkillInfo {
    const char* name;
    const char* desc;
    uint8_t     unlock_world;   // minimum WorldId value to unlock
};

constexpr SkillInfo SKILL_INFO[] = {
    { "NONE",     "",              0 },
    { "BOOTS",    "SUPER JUMP x1", 1 },  // ROYAL = 1
    { "CLOAK",    "AIR DASH x1",   1 },  // ROYAL = 1
    { "SHRINK",   "TINY 3SEC x1",  2 },  // QIN = 2
};

namespace skill_const {
    constexpr bn::fixed SPRING_MULTIPLIER  = bn::fixed(1.8);
    constexpr bn::fixed DASH_SPEED         = bn::fixed(5.0);
    constexpr int       DASH_DURATION      = 10;     // frames
    constexpr int       SHRINK_DURATION    = 180;    // frames (3 seconds)
    constexpr int       SHRINK_HALF_W      = 3;      // 6px wide (under 1 tile)
    constexpr int       SHRINK_HALF_H      = 3;      // 6px tall (under 1 tile)
}

} // namespace game

#endif // SKILL_H
