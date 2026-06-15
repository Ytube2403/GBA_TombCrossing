#ifndef TRAP_VANISH_FLOOR_H
#define TRAP_VANISH_FLOOR_H

#include "trap_base.h"

namespace game {

// ─────────────────────────────────────────────────────────────────────────────
// TrapVanishFloor — floor tile that disappears after the player stands on it
//
// param_a = delay frames before vanishing (default: 30 = 0.5s)
// param_b = respawn frames (0 = never respawns)
//
// States: SOLID → SHAKING → GONE → (RESPAWNING → SOLID)
// ─────────────────────────────────────────────────────────────────────────────
class TrapVanishFloor : public TrapBase {
public:
    enum class State : uint8_t {
        SOLID,
        SHAKING,   // brief warning shake before disappear
        GONE,
        RESPAWNING
    };

    TrapVanishFloor(uint8_t tile_x, uint8_t tile_y,
                    uint8_t delay_frames, uint8_t respawn_frames)
        : TrapBase(TrapType::VANISH_FLOOR, tile_x, tile_y,
                   delay_frames, respawn_frames),
          _state(State::SOLID),
          _player_was_on_top(false)
    {}

    void update(Player& player, Level& level) override;

    State floor_state() const { return _state; }

    // Returns true when the tile should be treated as solid
    bool is_solid() const { return _state == State::SOLID || _state == State::SHAKING; }

private:
    State _state;
    bool  _player_was_on_top; // tracked between frames to detect first contact
    int   _left_col = -1;
    int   _right_col = -1;

    bool player_on_top(const Player& player) const;
};

} // namespace game

#endif // TRAP_VANISH_FLOOR_H
