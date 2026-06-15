#ifndef TRAP_POP_SPIKE_H
#define TRAP_POP_SPIKE_H

#include "trap_base.h"
#include "bn_sprite_ptr.h"
#include "bn_optional.h"

namespace game {

// ─────────────────────────────────────────────────────────────────────────────
// TrapPopSpike — spike that pops out of floor/ceiling when triggered
//
// param_a = trigger proximity (tiles, 1–4)
// param_b = active frames before retracting (0 = stays out forever)
//
// States: HIDDEN → EXTENDING → EXTENDED → RETRACTING → HIDDEN
// ─────────────────────────────────────────────────────────────────────────────
class TrapPopSpike : public TrapBase {
public:
    enum class State : uint8_t { HIDDEN, EXTENDING, EXTENDED, RETRACTING };
    enum class Direction : uint8_t { UP, DOWN }; // which way the spike points

    TrapPopSpike(uint8_t tile_x, uint8_t tile_y,
                 uint8_t proximity_tiles, uint8_t active_frames,
                 Direction dir = Direction::UP)
        : TrapBase(TrapType::POP_SPIKE, tile_x, tile_y,
                   proximity_tiles, active_frames),
          _state(State::HIDDEN), _dir(dir)
    {}

    void update(Player& player, Level& level) override;

    State     spike_state() const { return _state; }
    Direction direction()   const { return _dir; }

private:
    State     _state;
    Direction _dir;
    int       _extend_timer; // frames spent extending
    bn::optional<bn::sprite_ptr> _sprite;

    bool player_in_range(const Player& player) const;
};

} // namespace game

#endif // TRAP_POP_SPIKE_H
