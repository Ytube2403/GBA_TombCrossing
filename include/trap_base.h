#ifndef TRAP_BASE_H
#define TRAP_BASE_H

#include <cstdint>
#include "constants.h"

namespace game {

class Player;
class Level;

// ─────────────────────────────────────────────────────────────────────────────
// TrapBase — abstract base for all trap types
//
// Every concrete trap must implement:
//   update(Player&)  — per-frame logic
//   is_active()      — whether the trap is currently dangerous
//   tile_x/tile_y()  — position for spatial queries
// ─────────────────────────────────────────────────────────────────────────────
class TrapBase {
public:
    TrapBase(TrapType type, uint8_t tile_x, uint8_t tile_y,
             uint8_t param_a, uint8_t param_b)
        : _type(type), _tx(tile_x), _ty(tile_y),
          _pa(param_a), _pb(param_b), _active(false), _timer(0)
    {}

    virtual ~TrapBase() = default;

    virtual void update(Player& player, Level& level) = 0;
    virtual void force_activate() {}

    TrapType type()    const { return _type; }
    uint8_t  tile_x()  const { return _tx; }
    uint8_t  tile_y()  const { return _ty; }
    bool     is_active() const { return _active; }

protected:
    TrapType _type;
    uint8_t  _tx, _ty;   // tile position
    uint8_t  _pa, _pb;   // parameters from level data
    bool     _active;    // is this trap in a lethal/moving state?
    int      _timer;     // general-purpose timer
};

} // namespace game

#endif // TRAP_BASE_H
