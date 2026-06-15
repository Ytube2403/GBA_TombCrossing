#ifndef TRAP_MANAGER_H
#define TRAP_MANAGER_H

#include "bn_vector.h"
#include "bn_unique_ptr.h"
#include "trap_base.h"
#include "level_data.h"

namespace game {

class Player;

// ─────────────────────────────────────────────────────────────────────────────
// TrapManager — owns and updates all active traps for the current level
//
// Usage:
//   1. Call load(level_def) when a level starts
//   2. Call update(player) every frame
//   3. Query gravity_flipped() to pass to Player::update()
//   4. Call clear() when leaving a level
// ─────────────────────────────────────────────────────────────────────────────
class TrapManager {
public:
    static constexpr int MAX_TRAPS = MAX_TRAPS_PER_LEVEL;

    TrapManager() = default;

    // Instantiate traps from level data (called on level load)
    void load(const ChamberDef& chamber, bool gravity_flipped = false);

    // Per-frame update — may call player.kill() if a trap hits
    void update(Player& player, Level& level);

    // Destroy all trap instances (call before loading next level)
    void clear(bool preserve_gravity = false);

    // Global gravity state — read by GameManager to pass into Player::update()
    bool gravity_flipped() const { return _gravity_flipped; }
    void set_gravity_flipped(bool value);

    // Did a gravity flip SFX fire this frame?
    bool consume_flip_sfx();

    // Get reference to the active traps
    const bn::vector<bn::unique_ptr<TrapBase>, MAX_TRAPS_PER_LEVEL>& traps() const { return _traps; }
    bn::vector<bn::unique_ptr<TrapBase>, MAX_TRAPS_PER_LEVEL>& traps() { return _traps; }

private:
    bn::vector<bn::unique_ptr<TrapBase>, MAX_TRAPS> _traps;
    bool _gravity_flipped = false;

    bn::unique_ptr<TrapBase> make_trap(const TrapPlacement& p);
};

} // namespace game

#endif // TRAP_MANAGER_H
