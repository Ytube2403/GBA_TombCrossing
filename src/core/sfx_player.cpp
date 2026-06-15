#include "sfx_player.h"

#include "bn_algorithm.h"
#include "bn_sound_item.h"
#include "bn_sound_items.h"

namespace game {
namespace {

struct CueConfig {
    bn::sound_item item;
    bn::fixed gain;
    int priority;
    uint8_t cooldown_frames;
};

constexpr CueConfig CONFIGS[] = {
    { bn::sound_items::ui_move,         bn::fixed(0.55),  10,  2 },
    { bn::sound_items::ui_confirm,      bn::fixed(0.65),  14,  4 },
    { bn::sound_items::ui_back,         bn::fixed(0.60),  13,  4 },
    { bn::sound_items::ui_denied,       bn::fixed(0.72),  18,  8 },
    { bn::sound_items::player_jump,     bn::fixed(0.78),  42,  4 },
    { bn::sound_items::player_land,     bn::fixed(0.62),  36,  5 },
    { bn::sound_items::player_death,    bn::fixed(0.92), 100, 20 },
    { bn::sound_items::skill_activate,  bn::fixed(0.82),  55,  8 },
    { bn::sound_items::room_clear,      bn::fixed(0.88),  96, 24 },
    { bn::sound_items::gravity_flip,    bn::fixed(0.86),  92, 12 },
    { bn::sound_items::trap_warning,    bn::fixed(0.72),  48,  6 },
    { bn::sound_items::plate_click,     bn::fixed(0.68),  44,  6 },
    { bn::sound_items::projectile_fire, bn::fixed(0.76),  68,  3 },
    { bn::sound_items::spike_extend,    bn::fixed(0.82),  72,  5 },
    { bn::sound_items::stone_move,      bn::fixed(0.76),  62,  8 },
    { bn::sound_items::stone_impact,    bn::fixed(0.90),  78,  6 },
    { bn::sound_items::floor_crack,     bn::fixed(0.70),  58,  8 },
    { bn::sound_items::floor_break,     bn::fixed(0.84),  76,  8 },
    { bn::sound_items::lava_rise,       bn::fixed(0.78),  70, 18 },
};

static_assert(int(SfxCue::COUNT) == int(sizeof(CONFIGS) / sizeof(CONFIGS[0])));

uint8_t cooldowns[int(SfxCue::COUNT)] = {};

}

void SfxPlayer::update()
{
    for(uint8_t& cooldown : cooldowns)
    {
        if(cooldown)
        {
            --cooldown;
        }
    }
}

void SfxPlayer::reset()
{
    for(uint8_t& cooldown : cooldowns)
    {
        cooldown = 0;
    }
}

bool SfxPlayer::play(SfxCue cue, bn::fixed speed, bn::fixed panning)
{
    int index = int(cue);
    if(cooldowns[index])
    {
        return false;
    }

    const CueConfig& config = CONFIGS[index];
    config.item.play_with_priority(
        config.priority,
        config.gain,
        bn::max(bn::fixed(0.5), bn::min(speed, bn::fixed(2))),
        bn::max(bn::fixed(-0.75), bn::min(panning, bn::fixed(0.75))));
    cooldowns[index] = config.cooldown_frames;
    return true;
}

} // namespace game
