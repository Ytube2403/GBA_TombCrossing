#ifndef TRAP_GRAVITY_FLIP_H
#define TRAP_GRAVITY_FLIP_H

#include "trap_base.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"

namespace game {

// ─────────────────────────────────────────────────────────────────────────────
// TrapGravityFlip — zone that flips gravity for the entire level
//
// param_a = 0: flip when player enters zone | 1: flip on timer
// param_b = timer interval (frames) when param_a == 1
//
// When active, GameManager passes gravity_flipped=true to Player::update()
// and all moving-platform traps reverse their vertical direction.
// ─────────────────────────────────────────────────────────────────────────────
class TrapGravityFlip : public TrapBase {
public:
    TrapGravityFlip(uint8_t tile_x, uint8_t tile_y,
                    uint8_t trigger_mode, uint8_t interval_frames)
        : TrapBase(TrapType::GRAVITY_FLIP, tile_x, tile_y,
                   trigger_mode, interval_frames),
          _flipped(false), _player_inside(false),
          _flip_requested(false), _flip_sfx_pending(false)
    {}

    void update(Player& player, Level& level) override;

    // GameManager queries this each frame
    bool is_flipped() const { return _flipped; }
    bool consume_flip_request() {
        bool value = _flip_requested;
        _flip_requested = false;
        return value;
    }
    void set_flipped(bool value) {
        _flipped = value;
        if(_sprite.has_value()) {
            _sprite->set_vertical_flip(value);
        }
    }
    bool consume_sfx_request() {
        bool v = _flip_sfx_pending;
        _flip_sfx_pending = false;
        return v;
    }

private:
    bool _flipped;
    bool _player_inside;
    bool _flip_requested;
    bool _flip_sfx_pending; // set true whenever a flip occurs so audio can play
    bn::optional<bn::sprite_ptr> _sprite;

    bool player_in_zone(const Player& player) const;
};

} // namespace game

#endif // TRAP_GRAVITY_FLIP_H
