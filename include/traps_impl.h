#ifndef TRAPS_IMPL_H
#define TRAPS_IMPL_H

#include "trap_base.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"
#include "bn_vector.h"

namespace game {

// Forward declarations
class Level;

// ─────────────────────────────────────────────────────────────────────────────
// Universal Traps
// ─────────────────────────────────────────────────────────────────────────────

// FALLING_BLOCK (2)
class TrapFallingBlock : public TrapBase {
public:
    enum class State : uint8_t { WAITING, SHAKING, FALLING, LANDED };

    TrapFallingBlock(uint8_t tile_x, uint8_t tile_y, uint8_t delay_frames, uint8_t param_b)
        : TrapBase(TrapType::FALLING_BLOCK, tile_x, tile_y, delay_frames, param_b),
          _state(State::WAITING)
    {}

    void update(Player& player, Level& level) override;
    void force_activate() override;

private:
    State _state;
    bn::fixed _vy = 0;
    bn::fixed _curr_x = 0;
    bn::fixed _curr_y = 0;
    bn::optional<bn::sprite_ptr> _sprite;
    bool _initialized = false;
};

// CONVEYOR (4)
class TrapConveyor : public TrapBase {
public:
    TrapConveyor(uint8_t tile_x, uint8_t tile_y, uint8_t force_x10, uint8_t dir)
        : TrapBase(TrapType::CONVEYOR, tile_x, tile_y, force_x10, dir)
    {}

    void update(Player& player, Level& level) override;
};

// WIND (9)
class TrapWind : public TrapBase {
public:
    TrapWind(uint8_t tile_x, uint8_t tile_y, uint8_t force_x10, uint8_t dir)
        : TrapBase(TrapType::WIND, tile_x, tile_y, force_x10, dir)
    {}

    void update(Player& player, Level& level) override;
};

// INVISIBLE_WALL (8)
class TrapInvisibleWall : public TrapBase {
public:
    TrapInvisibleWall(uint8_t tile_x, uint8_t tile_y, uint8_t param_a, uint8_t param_b)
        : TrapBase(TrapType::INVISIBLE_WALL, tile_x, tile_y, param_a, param_b)
    {}

    void update(Player& player, Level& level) override;
    void init_tile(Level& level);

private:
    bool _initialized = false;
};

// FAKE_DOOR (6)
class TrapFakeDoor : public TrapBase {
public:
    TrapFakeDoor(uint8_t tile_x, uint8_t tile_y, uint8_t param_a, uint8_t param_b)
        : TrapBase(TrapType::FAKE_DOOR, tile_x, tile_y, param_a, param_b)
    {}

    void update(Player& player, Level& level) override;

private:
    bn::optional<bn::sprite_ptr> _sprite;
};

// DARK_ROOM (7)
class TrapDarkRoom : public TrapBase {
public:
    TrapDarkRoom(uint8_t tile_x, uint8_t tile_y, uint8_t radius_tiles, uint8_t param_b)
        : TrapBase(TrapType::DARK_ROOM, tile_x, tile_y, radius_tiles, param_b)
    {}

    void update(Player& player, Level& level) override;
};

// MOVING_PLATFORM (3)
class TrapMovingPlatform : public TrapBase {
public:
    TrapMovingPlatform(uint8_t tile_x, uint8_t tile_y, uint8_t speed_x10, uint8_t path_len)
        : TrapBase(TrapType::MOVING_PLATFORM, tile_x, tile_y, speed_x10, path_len)
    {}

    void update(Player& player, Level& level) override;

private:
    bn::fixed _curr_x = 0;
    bn::fixed _curr_y = 0;
    bn::fixed _vx = 0;
    bool _moving_forward = true;
    bn::optional<bn::sprite_ptr> _sprite;
    bool _initialized = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// World 1 – Egyptian
// ─────────────────────────────────────────────────────────────────────────────

// SAND_SINKHOLE (10)
class TrapSandSinkhole : public TrapBase {
public:
    TrapSandSinkhole(uint8_t tile_x, uint8_t tile_y, uint8_t sink_rate, uint8_t param_b)
        : TrapBase(TrapType::SAND_SINKHOLE, tile_x, tile_y, sink_rate, param_b)
    {}

    void update(Player& player, Level& level) override;

private:
    int _sink_timer = 0;
    int _idle_timer = 0;
    bn::optional<bn::sprite_ptr> _sprite;
};

// BOULDER (11) — Trigger-based: activates when player crosses tile_x column
//   param_a = speed × 10
//   param_b = direction: 0 = comes from left, 1 = comes from right
class TrapBoulder : public TrapBase {
public:
    enum class State : uint8_t { IDLE, ROLLING, COOLDOWN };

    TrapBoulder(uint8_t tile_x, uint8_t tile_y, uint8_t speed_x10, uint8_t dir)
        : TrapBase(TrapType::BOULDER, tile_x, tile_y, speed_x10, dir),
          _state(State::IDLE)
    {}

    void update(Player& player, Level& level) override;

private:
    State _state;
    bn::fixed _curr_x = 0;
    bn::fixed _curr_y = 0;
    bool _triggered = false;       // has player crossed trigger column?
    bn::optional<bn::sprite_ptr> _sprite;
    bool _initialized = false;
    int _anim_frame = 0;
    int _anim_timer = 0;
    int _cooldown_timer = 0;
};

// BOUNCE_BOULDER (23) — Falls diagonally, bounces once, then rolls horizontally
//   param_a = speed × 10
//   param_b = direction: 0 = from top-left, 1 = from top-right
class TrapBounceBoulder : public TrapBase {
public:
    enum class State : uint8_t { IDLE, FALLING, BOUNCING, ROLLING };

    TrapBounceBoulder(uint8_t tile_x, uint8_t tile_y, uint8_t speed_x10, uint8_t dir)
        : TrapBase(TrapType::BOUNCE_BOULDER, tile_x, tile_y, speed_x10, dir),
          _state(State::IDLE)
    {}

    void update(Player& player, Level& level) override;

private:
    State _state;
    bn::fixed _curr_x = 0;
    bn::fixed _curr_y = 0;
    bn::fixed _vy = 0;
    bn::fixed _ground_y = 0;      // Y where the boulder lands/bounces
    bool _triggered = false;
    bn::optional<bn::sprite_ptr> _sprite;
    bool _initialized = false;
    int _anim_frame = 0;
    int _anim_timer = 0;
    int _cooldown_timer = 0;
};

// AUTO_BOULDER (24) — Simplest boulder: auto-rolls periodically from a fixed side
//   param_a = speed × 10
//   param_b = direction: 0 = from left, 1 = from right
class TrapAutoBoulder : public TrapBase {
public:
    enum class State : uint8_t { COOLDOWN, ROLLING };

    TrapAutoBoulder(uint8_t tile_x, uint8_t tile_y, uint8_t speed_x10, uint8_t dir)
        : TrapBase(TrapType::AUTO_BOULDER, tile_x, tile_y, speed_x10, dir),
          _state(State::COOLDOWN)
    {}

    void update(Player& player, Level& level) override;

private:
    State _state;
    bn::fixed _curr_x = 0;
    bn::fixed _curr_y = 0;
    bn::optional<bn::sprite_ptr> _sprite;
    bool _initialized = false;
    int _anim_frame = 0;
    int _anim_timer = 0;
    int _cooldown_timer = 0;
};

// SARCOPHAGUS_POP (12)
class TrapSarcophagusPop : public TrapBase {
public:
    enum class State : uint8_t { IDLE, WARNING, POPPED };

    TrapSarcophagusPop(uint8_t tile_x, uint8_t tile_y, uint8_t delay_frames, uint8_t param_b)
        : TrapBase(TrapType::SARCOPHAGUS_POP, tile_x, tile_y, delay_frames, param_b),
          _state(State::IDLE)
    {}

    void update(Player& player, Level& level) override;
    void force_activate() override;

private:
    State _state;
    bn::fixed _curr_x = 0;
    bn::fixed _curr_y = 0;
    bn::optional<bn::sprite_ptr> _sprite;
    bool _initialized = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// World 2 – Royal
// ─────────────────────────────────────────────────────────────────────────────

// PENDULUM_AXE (13)
class TrapPendulumAxe : public TrapBase {
public:
    TrapPendulumAxe(uint8_t tile_x, uint8_t tile_y, uint8_t swing_period, uint8_t param_b)
        : TrapBase(TrapType::PENDULUM_AXE, tile_x, tile_y, swing_period, param_b)
    {}

    void update(Player& player, Level& level) override;

private:
    bn::fixed _curr_x = 0;
    bn::fixed _curr_y = 0;
    bn::optional<bn::sprite_ptr> _sprite;
    bool _initialized = false;
};

// ARROW_WALL (14)
class TrapArrowWall : public TrapBase {
public:
    struct Arrow {
        bn::fixed x;
        bn::fixed y;
        bn::optional<bn::sprite_ptr> sprite;
        bool active;
    };

    TrapArrowWall(uint8_t tile_x, uint8_t tile_y, uint8_t fire_interval, uint8_t dir)
        : TrapBase(TrapType::ARROW_WALL, tile_x, tile_y, fire_interval, dir)
    {}

    void update(Player& player, Level& level) override;

private:
    bn::vector<Arrow, 4> _arrows;
};

// ─────────────────────────────────────────────────────────────────────────────
// World 3 – Qin
// ─────────────────────────────────────────────────────────────────────────────

// TERRACOTTA_SOLDIER (15)
class TrapTerracottaSoldier : public TrapBase {
public:
    enum class State : uint8_t { IDLE, EXTENDING, EXTENDED, RETRACTING };

    TrapTerracottaSoldier(uint8_t tile_x, uint8_t tile_y, uint8_t proximity, uint8_t param_b)
        : TrapBase(TrapType::TERRACOTTA_SOLDIER, tile_x, tile_y, proximity, param_b),
          _state(State::IDLE)
    {}

    void update(Player& player, Level& level) override;
    void force_activate() override;

private:
    State _state;
    bn::fixed _curr_x = 0;
    bn::fixed _curr_y = 0;
    bn::optional<bn::sprite_ptr> _sprite;
    bool _initialized = false;
};

// PRESSURE_PLATE (16)
class TrapPressurePlate : public TrapBase {
public:
    TrapPressurePlate(uint8_t tile_x, uint8_t tile_y, uint8_t target_idx, uint8_t param_b)
        : TrapBase(TrapType::PRESSURE_PLATE, tile_x, tile_y, target_idx, param_b)
    {}

    void update(Player& player, Level& level) override;
    void update_with_manager(Player& player, class TrapManager& manager);

private:
    bool _stepped_on = false;
    bn::optional<bn::sprite_ptr> _sprite;
};

// ─────────────────────────────────────────────────────────────────────────────
// World 4 – Aztec
// ─────────────────────────────────────────────────────────────────────────────

// DART_FROG (17)
class TrapDartFrog : public TrapBase {
public:
    struct Dart {
        bn::fixed x;
        bn::fixed y;
        bn::optional<bn::sprite_ptr> sprite;
        bool active;
    };

    TrapDartFrog(uint8_t tile_x, uint8_t tile_y, uint8_t fire_interval, uint8_t dir)
        : TrapBase(TrapType::DART_FROG, tile_x, tile_y, fire_interval, dir)
    {}

    void update(Player& player, Level& level) override;

private:
    bn::vector<Dart, 4> _darts;
};

// LAVA_RISE (18)
class TrapLavaRise : public TrapBase {
public:
    TrapLavaRise(uint8_t tile_x, uint8_t tile_y, uint8_t rise_rate, uint8_t peak_y)
        : TrapBase(TrapType::LAVA_RISE, tile_x, tile_y, rise_rate, peak_y)
    {}

    void update(Player& player, Level& level) override;

private:
    bn::fixed _curr_y = 0;
    bn::vector<bn::sprite_ptr, 15> _sprites;
    bool _initialized = false;
};

// CRUMBLE_FLOOR (19)
class TrapCrumbleFloor : public TrapBase {
public:
    enum class State : uint8_t { SOLID, CRACKED, GONE, RESPAWNING };

    TrapCrumbleFloor(uint8_t tile_x, uint8_t tile_y, uint8_t steps, uint8_t respawn_delay)
        : TrapBase(TrapType::CRUMBLE_FLOOR, tile_x, tile_y, steps, respawn_delay),
          _state(State::SOLID), _step_count(steps)
    {}

    void update(Player& player, Level& level) override;
    
    State floor_state() const { return _state; }
    bool is_solid() const { return _state == State::SOLID || _state == State::CRACKED; }

private:
    State _state;
    uint8_t _step_count;
    bool _player_was_on_top = false;

    bool player_on_top(const Player& player) const;
};

// ─────────────────────────────────────────────────────────────────────────────
// World 5 – Japanese Kofun
// ─────────────────────────────────────────────────────────────────────────────

// BAMBOO_SPIKE (20)
class TrapBambooSpike : public TrapBase {
public:
    enum class State : uint8_t { HIDDEN, GROWING, GROWN, RETRACTING };

    TrapBambooSpike(uint8_t tile_x, uint8_t tile_y, uint8_t proximity, uint8_t param_b)
        : TrapBase(TrapType::BAMBOO_SPIKE, tile_x, tile_y, proximity, param_b),
          _state(State::HIDDEN)
    {}

    void update(Player& player, Level& level) override;
    void force_activate() override;

private:
    State _state;
    bn::optional<bn::sprite_ptr> _sprite;
    bool _initialized = false;

    bool player_in_range(const Player& player) const;
};

// SAMURAI_STATUE (21)
class TrapSamuraiStatue : public TrapBase {
public:
    enum class State : uint8_t { IDLE, WARNING, SWINGING };

    TrapSamuraiStatue(uint8_t tile_x, uint8_t tile_y, uint8_t warning_frames, uint8_t param_b)
        : TrapBase(TrapType::SAMURAI_STATUE, tile_x, tile_y, warning_frames, param_b),
          _state(State::IDLE)
    {}

    void update(Player& player, Level& level) override;
    void force_activate() override;

private:
    State _state;
    bn::optional<bn::sprite_ptr> _sprite;
    bool _initialized = false;
};

// PAPER_FLOOR (22)
class TrapPaperFloor : public TrapBase {
public:
    enum class State : uint8_t { SOLID, RIPPED, GONE };

    TrapPaperFloor(uint8_t tile_x, uint8_t tile_y, uint8_t hold_frames, uint8_t param_b)
        : TrapBase(TrapType::PAPER_FLOOR, tile_x, tile_y, hold_frames, param_b),
          _state(State::SOLID)
    {}

    void update(Player& player, Level& level) override;

    State floor_state() const { return _state; }
    bool is_solid() const { return _state == State::SOLID; }

private:
    State _state;
    bool _player_was_on_top = false;

    bool player_on_top(const Player& player) const;
};

} // namespace game

#endif // TRAPS_IMPL_H
