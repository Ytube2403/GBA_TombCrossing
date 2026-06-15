#include "player.h"
#include "skill.h"
#include "sfx_player.h"
#include "constants.h"
#include "bn_keypad.h"
#include "bn_sprite_tiles_ptr.h"
#include "bn_sprite_tiles_item.h"
#include "bn_sprite_items_player_kha_men.h"

extern "C" {
    game::Player* global_player = nullptr;
}

namespace game {

namespace {
    // Animation frame counts per state
    constexpr int ANIM_FRAMES[]  = { 4, 6, 3, 2, 2, 8, 6, 4 };  // indexed by PlayerAnim
    // Animation ticks per frame (speed)
    constexpr int ANIM_SPEEDS[]  = { 8, 5, 5, 6, 4, 4, 6, 5 };
    constexpr int WIN_DURATION_FRAMES = 36;
}

Player::Player(bn::fixed spawn_x, bn::fixed spawn_y) :
    _x(spawn_x),
    _y(spawn_y),
    _vx(0),
    _vy(0),
    _alive(true),
    _on_ground(false),
    _was_on_ground(false),
    _facing_left(false),
    _coyote_timer(0),
    _jump_buffer_timer(0),
    _die_frame(0),
    _win_frame(0),
    _wall_contact_dir(0),
    _winning(false),
    _anim(PlayerAnim::IDLE),
    _anim_frame(0),
    _anim_timer(0),
    _sprite(bn::sprite_items::player_kha_men.create_sprite(spawn_x, spawn_y)),
    _external_vx(0),
    _gravity_flipped(false),
    _death_cause(DeathCause::NONE)
{
    global_player = this;
}

Player::~Player()
{
    global_player = nullptr;
}

void Player::reset(bn::fixed spawn_x, bn::fixed spawn_y)
{
    reset_motion(spawn_x, spawn_y, true);
}

void Player::relocate(bn::fixed spawn_x, bn::fixed spawn_y)
{
    reset_motion(spawn_x, spawn_y, false);
}

void Player::reset_motion(bn::fixed spawn_x, bn::fixed spawn_y, bool reset_skill)
{
    _x = spawn_x;
    _y = spawn_y;
    _vx = 0;
    _vy = 0;
    _alive = true;
    _on_ground = false;
    _was_on_ground = false;
    _facing_left = false;
    _coyote_timer = 0;
    _jump_buffer_timer = 0;
    _die_frame = 0;
    _win_frame = 0;
    _wall_contact_dir = 0;
    _winning = false;
    _external_vx = 0;
    _gravity_flipped = false;
    _death_cause = DeathCause::NONE;
    _half_w = BASE_HALF_W;
    _half_h = BASE_HALF_H;
    if(reset_skill)
    {
        _skill.used = false;
        _skill.active = false;
        _skill.timer = 0;
    }
    set_animation(PlayerAnim::IDLE);
    sync_sprite();
}

void Player::kill(DeathCause cause)
{
    if(!_alive || _winning) return;
    _alive = false;
    _death_cause = cause;
    _vx = 0;
    _vy = 0;
    _die_frame = 0;
    set_animation(PlayerAnim::DIE);
    SfxPlayer::play(SfxCue::PLAYER_DEATH);
}

void Player::start_win()
{
    if(!_alive || _winning)
    {
        return;
    }

    _winning = true;
    _win_frame = 0;
    _wall_contact_dir = 0;
    _vx = 0;
    _vy = 0;
    _external_vx = 0;
    set_animation(PlayerAnim::WIN);
}

bool Player::die_anim_done() const
{
    int total_frames = ANIM_FRAMES[static_cast<int>(PlayerAnim::DIE)];
    int speed        = ANIM_SPEEDS[static_cast<int>(PlayerAnim::DIE)];
    return _die_frame >= total_frames * speed;
}

bool Player::win_anim_done() const
{
    return _winning && _win_frame >= WIN_DURATION_FRAMES;
}

void Player::update(bool gravity_flipped)
{
    _gravity_flipped = gravity_flipped;
    _was_on_ground = _on_ground;
    _wall_contact_dir = 0;

    if(!_alive)
    {
        // Advance death animation counter
        ++_die_frame;
        return;
    }

    if(_winning)
    {
        ++_win_frame;
        _vx = 0;
        _vy = 0;
        _external_vx = 0;
        return;
    }

    handle_input(gravity_flipped);
    apply_physics(gravity_flipped);
    clamp_to_screen();
    update_skill();
}

void Player::update_graphics()
{
    update_animation();
    sync_sprite();
}

void Player::handle_input(bool gravity_flipped)
{
    // Horizontal movement
    if(bn::keypad::left_held())
    {
        _vx -= physics::MOVE_SPEED * bn::fixed(0.15);
        if(_vx < -physics::MOVE_SPEED) _vx = -physics::MOVE_SPEED;
        _facing_left = true;
    }
    else if(bn::keypad::right_held())
    {
        _vx += physics::MOVE_SPEED * bn::fixed(0.15);
        if(_vx > physics::MOVE_SPEED) _vx = physics::MOVE_SPEED;
        _facing_left = false;
    }
    else
    {
        // Friction
        _vx *= bn::fixed(0.75);
        if(_vx < bn::fixed(0.05) && _vx > bn::fixed(-0.05)) _vx = 0;
    }

    // Jump buffering: register A press even a few frames before landing
    if(bn::keypad::a_pressed())
    {
        _jump_buffer_timer = physics::JUMP_BUFFER_FRAMES;
    }

    // Jump execution: can jump during coyote window or on ground
    bool can_jump = (_on_ground || _coyote_timer > 0);
    if(_jump_buffer_timer > 0 && can_jump)
    {
        bn::fixed jump_force = physics::JUMP_FORCE;
        if(_skill.active && _skill.selected == SkillType::SPRING_BOOTS)
        {
            jump_force = physics::JUMP_FORCE * skill_const::SPRING_MULTIPLIER;
            _skill.active = false;  // consumed after jump
        }
        _vy = gravity_flipped ? -jump_force : jump_force;
        _jump_buffer_timer = 0;
        _coyote_timer = 0;
        _on_ground = false;
        SfxPlayer::play(SfxCue::PLAYER_JUMP);
    }

    if(_jump_buffer_timer > 0) --_jump_buffer_timer;

    // Variable jump height: release A early to cut jump short
    if(!gravity_flipped && bn::keypad::a_released() && _vy < 0)
    {
        _vy *= bn::fixed(0.5);
    }
    if(gravity_flipped && bn::keypad::a_released() && _vy > 0)
    {
        _vy *= bn::fixed(0.5);
    }
}

void Player::apply_physics(bool gravity_flipped)
{
    // Coyote time management
    if(_on_ground)
    {
        _coyote_timer = physics::COYOTE_FRAMES;

        // When on ground, apply a small grounding force instead of full gravity.
        // This prevents the jitter cycle (gravity pulls down 0.3px → collision
        // pushes back up → repeat), while still allowing collision detection
        // to confirm ground contact each frame.
        _vy = gravity_flipped ? bn::fixed(-0.5) : bn::fixed(0.5);
    }
    else
    {
        if(_coyote_timer > 0) --_coyote_timer;

        // Apply gravity only when airborne
        bn::fixed gravity = gravity_flipped ? -physics::GRAVITY : physics::GRAVITY;
        _vy += gravity;

        // Terminal velocity
        bn::fixed max_fall = gravity_flipped ? -physics::MAX_FALL_SPEED : physics::MAX_FALL_SPEED;
        if(!gravity_flipped && _vy > max_fall)  _vy = max_fall;
        if( gravity_flipped && _vy < max_fall)  _vy = max_fall;
    }

    // Integrate
    _x += _vx + _external_vx;
    _y += _vy;
    _external_vx = 0;

    // Ground flag reset — set back to true by Level::resolve_collisions()
    _on_ground = false;
}

void Player::clamp_to_screen()
{
    // Horizontal screen wrap prevention
    if(_x < SCREEN_LEFT  + _half_w) { _x = SCREEN_LEFT  + _half_w; _vx = 0; }
    if(_x > SCREEN_RIGHT - _half_w) { _x = SCREEN_RIGHT - _half_w; _vx = 0; }

    // Death pit check
    if(!_gravity_flipped && _y > GAMEPLAY_BOTTOM + 32)
    {
        kill(DeathCause::PIT);
    }
    else if(_gravity_flipped && _y < GAMEPLAY_TOP - 32)
    {
        kill(DeathCause::PIT);
    }
}

void Player::update_animation()
{
    // Choose target animation
    PlayerAnim target = _anim;

    if(_winning)
    {
        target = PlayerAnim::WIN;
    }
    else if(!_alive)
    {
        target = PlayerAnim::DIE;
    }
    else if(!_on_ground)
    {
        if(!_gravity_flipped)
        {
            target = (_vy < 0) ? PlayerAnim::JUMP_UP : PlayerAnim::FALL;
        }
        else
        {
            target = (_vy > 0) ? PlayerAnim::JUMP_UP : PlayerAnim::FALL;
        }
    }
    else if(!_was_on_ground) // Just landed!
    {
        target = PlayerAnim::LAND;
    }
    else if(_on_ground &&
            ((_wall_contact_dir < 0 && bn::keypad::left_held()) ||
             (_wall_contact_dir > 0 && bn::keypad::right_held())))
    {
        target = PlayerAnim::PUSH;
    }
    else if(_vx != 0)
    {
        target = PlayerAnim::WALK;
    }
    else
    {
        target = PlayerAnim::IDLE;
    }

    // Switch animation if needed (don't interrupt DIE or LAND mid-play)
    if(target != _anim &&
       _anim != PlayerAnim::DIE &&
       _anim != PlayerAnim::WIN)
    {
        // Allow LAND to play fully before switching.
        // Check using _anim_frame (which counts completed frames) rather than
        // _anim_timer (which resets every 'speed' ticks and thus can never
        // reach total*speed).
        if(_anim == PlayerAnim::LAND)
        {
            int total = ANIM_FRAMES[static_cast<int>(_anim)];
            if(_anim_frame >= total - 1)
                set_animation(target);
        }
        else
        {
            set_animation(target);
        }
    }

    // Advance frame
    ++_anim_timer;
    int speed = ANIM_SPEEDS[static_cast<int>(_anim)];
    int total = ANIM_FRAMES[static_cast<int>(_anim)];
    if(_anim_timer >= speed)
    {
        _anim_timer = 0;
        ++_anim_frame;
        if(_anim_frame >= total)
        {
            // Loop most animations; DIE and WIN hold last frame
            if(_anim == PlayerAnim::DIE || _anim == PlayerAnim::WIN)
                _anim_frame = total - 1;
            else
                _anim_frame = 0;
        }
    }
}

void Player::set_animation(PlayerAnim anim)
{
    _anim = anim;
    _anim_frame = 0;
    _anim_timer = 0;
}

void Player::sync_sprite()
{
    if(!_sprite.has_value()) return;

    _sprite->set_position(_x, _y);
    _sprite->set_horizontal_flip(_facing_left);
    _sprite->set_vertical_flip(_gravity_flipped);

    // Update sprite tile index — assumes sprite sheet is laid out as:
    // [IDLE frames] [WALK frames] [JUMP_UP] [FALL] [LAND] [DIE] [WIN] [PUSH]
    // TODO: replace with bn::sprite_animate_action for proper Butano animation
    // For now we manually compute frame offset
    int base_offsets[] = { 0, 4, 10, 13, 15, 17, 25, 31 };
    int tile_offset = base_offsets[static_cast<int>(_anim)] + _anim_frame;
    _sprite->set_tiles(bn::sprite_items::player_kha_men.tiles_item().create_tiles(tile_offset));
}

void Player::set_skill(SkillType skill)
{
    _skill.select(skill);
    _half_w = BASE_HALF_W;
    _half_h = BASE_HALF_H;
}

bool Player::try_activate_skill()
{
    if(!_skill.can_activate()) return false;

    switch(_skill.selected)
    {
        case SkillType::SPRING_BOOTS:
            if(!_on_ground) return false;  // must be on ground
            _skill.active = true;
            _skill.used = true;
            SfxPlayer::play(SfxCue::SKILL_ACTIVATE, bn::fixed(1.08));
            return true;

        case SkillType::DASH_CLOAK:
            if(_on_ground) return false;  // must be airborne
            _vx = _facing_left ? -skill_const::DASH_SPEED : skill_const::DASH_SPEED;
            _vy = 0;  // horizontal dash, no gravity during dash
            _skill.active = true;
            _skill.timer = skill_const::DASH_DURATION;
            _skill.used = true;
            SfxPlayer::play(SfxCue::SKILL_ACTIVATE, bn::fixed(0.94));
            return true;

        case SkillType::SHRINK_POTION:
            _half_w = skill_const::SHRINK_HALF_W;
            _half_h = skill_const::SHRINK_HALF_H;
            _skill.active = true;
            _skill.timer = skill_const::SHRINK_DURATION;
            _skill.used = true;
            SfxPlayer::play(SfxCue::SKILL_ACTIVATE, bn::fixed(0.82));
            return true;

        default:
            return false;
    }
}

void Player::update_skill()
{
    if(!_skill.active) return;

    switch(_skill.selected)
    {
        case SkillType::SPRING_BOOTS:
            // Active flag is cleared in handle_input after boosted jump executes
            break;

        case SkillType::DASH_CLOAK:
            if(_skill.timer > 0)
            {
                --_skill.timer;
                _vy = 0;  // maintain horizontal dash, suppress gravity
            }
            else
            {
                _skill.active = false;
            }
            break;

        case SkillType::SHRINK_POTION:
            if(_skill.timer > 0)
            {
                --_skill.timer;
            }
            else
            {
                // Restore normal hitbox
                _half_w = BASE_HALF_W;
                _half_h = BASE_HALF_H;
                _skill.active = false;
                // Push-out is handled by Level::resolve_collisions next frame
            }
            break;

        default:
            break;
    }
}

} // namespace game
