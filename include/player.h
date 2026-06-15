#ifndef PLAYER_H
#define PLAYER_H

#include "bn_sprite_ptr.h"
#include "bn_camera_ptr.h"
#include "bn_fixed.h"
#include "bn_optional.h"
#include "constants.h"
#include "skill.h"

namespace game {

enum class DeathCause : uint8_t {
    NONE = 0,
    TRAP = 1,
    TILE_HAZARD = 2,
    PIT = 3,
};

// ─────────────────────────────────────────────────────────────────────────────
// Player — Kha-Men the mummy
//
// Handles: input, physics (gravity/jump/coyote time), animation state,
//          hitbox query, death, and gravity-flip mode.
// ─────────────────────────────────────────────────────────────────────────────
class Player {
public:
    explicit Player(bn::fixed spawn_x, bn::fixed spawn_y);
    ~Player();

    // Called once per frame from GameManager
    void update(bool gravity_flipped = false);
    void update_graphics();

    // Trigger instant death (plays die animation then signals ready-to-restart)
    void kill(DeathCause cause = DeathCause::TRAP);
    void start_win();

    // Reset to a spawn position (after death restart)
    void reset(bn::fixed spawn_x, bn::fixed spawn_y);
    void relocate(bn::fixed spawn_x, bn::fixed spawn_y);

    // ── Position & physics ──────────────────────────────────
    bn::fixed x() const { return _x; }
    bn::fixed y() const { return _y; }
    bn::fixed vx() const { return _vx; }
    bn::fixed vy() const { return _vy; }
    int half_width() const { return _half_w; }
    int half_height() const { return _half_h; }

    // Pixel-space AABB (16×32, centred on _x/_y)
    int left()   const { return _x.integer() - _half_w; }
    int right()  const { return _x.integer() + _half_w; }
    int top()    const { return _y.integer() - _half_h; }
    int bottom() const { return _y.integer() + _half_h; }

    // ── State queries ────────────────────────────────────────
    bool is_alive()         const { return _alive; }
    bool is_on_ground()     const { return _on_ground; }
    bool was_on_ground()    const { return _was_on_ground; }
    bool is_facing_left()   const { return _facing_left; }
    bool gravity_flipped()  const { return _gravity_flipped; }
    bool is_winning()       const { return _winning; }
    bool die_anim_done()    const; // true once death animation finishes
    bool win_anim_done()    const;
    PlayerAnim anim_state() const { return _anim; }
    DeathCause death_cause() const { return _death_cause; }

    // ── Collision response helpers (called by Level) ─────────
    void set_on_ground(bool v) { _on_ground = v; }
    void stop_vertical()       { _vy = 0; }
    void stop_horizontal()     { _vx = 0; }
    void clear_wall_contact()  { _wall_contact_dir = 0; }
    void set_wall_contact(int direction) { _wall_contact_dir = direction; }
    void push_out_x(bn::fixed delta) { _x += delta; }
    void push_out_y(bn::fixed delta) { _y += delta; }

    // ── Camera follow support ────────────────────────────────
    void set_camera(const bn::camera_ptr& camera)
    {
        if(_sprite.has_value())
        {
            _sprite->set_camera(camera);
        }
    }

    // ── External velocity modifiers (traps) ──────────────────
    void apply_conveyor(bn::fixed force) { _external_vx += force; }
    void apply_wind(bn::fixed force)     { _external_vx += force; }

    // ── Skill system ─────────────────────────────────────────
    void         set_skill(SkillType skill);
    bool         try_activate_skill();
    void         update_skill();
    bool         is_skill_active() const { return _skill.active; }
    SkillType    active_skill_type() const { return _skill.selected; }
    const SkillState& skill_state() const { return _skill; }

private:
    static constexpr int BASE_HALF_W = physics::PLAYER_HALF_WIDTH;
    static constexpr int BASE_HALF_H = physics::PLAYER_HALF_HEIGHT;

    bn::fixed _x, _y;       // world position (screen-space, Butano centre)
    bn::fixed _vx, _vy;     // velocity (px/frame)

    bool _alive;
    bool _on_ground;
    bool _was_on_ground;
    bool _facing_left;
    int  _coyote_timer;      // coyote-time frames remaining
    int  _jump_buffer_timer; // jump-buffer frames remaining
    int  _die_frame;         // frame counter for death animation
    int  _win_frame;
    int  _wall_contact_dir;
    bool _winning;

    PlayerAnim _anim;        // current animation state
    int        _anim_frame;  // current frame within animation
    int        _anim_timer;  // ticks since last frame advance

    bn::optional<bn::sprite_ptr> _sprite;
    bn::fixed _external_vx;
    bool _gravity_flipped;
    DeathCause _death_cause;
    SkillState _skill;
    int _half_w = BASE_HALF_W;
    int _half_h = BASE_HALF_H;

    void handle_input(bool gravity_flipped);
    void apply_physics(bool gravity_flipped);
    void clamp_to_screen();
    void update_animation();
    void set_animation(PlayerAnim anim);
    void sync_sprite();
    void reset_motion(bn::fixed spawn_x, bn::fixed spawn_y, bool reset_skill);
};

} // namespace game

extern "C" {
    extern game::Player* global_player;
}

#endif // PLAYER_H
