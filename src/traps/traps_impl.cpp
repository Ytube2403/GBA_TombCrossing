#include "traps_impl.h"
#include "player.h"
#include "level.h"
#include "sfx_player.h"
#include "trap_manager.h"
#include "bn_math.h"
#include "bn_fixed.h"
#include "bn_sprite_tiles_ptr.h"
#include "bn_sprite_tiles_item.h"
#include "bn_sprite_items_boulder.h"
#include "bn_sprite_items_sand_sinkhole.h"
#include "bn_sprite_items_moving_platform.h"
#include "bn_sprite_items_pendulum_axe.h"
#include "bn_sprite_items_arrow.h"
#include "bn_sprite_items_terracotta_soldier.h"
#include "bn_sprite_items_samurai_statue.h"
#include "bn_sprite_items_bamboo_grow.h"
#include "bn_sprite_items_lava_rise.h"
#include "bn_sprite_items_dart.h"
#include "bn_sprite_items_exit_door.h"
#include "bn_sprite_items_pressure_plate.h"
#include "bn_rect_window.h"
#include "bn_window.h"

namespace game {

// Helper to check standard AABB collision between player and a trap bounding box
static bool check_player_collision(const Player& player, bn::fixed tx, bn::fixed ty, int width, int height)
{
    bn::fixed half_w = bn::fixed(width) / 2;
    bn::fixed half_h = bn::fixed(height) / 2;
    return player.left() < (tx + half_w).integer() && player.right() > (tx - half_w).integer() &&
           player.top() < (ty + half_h).integer() && player.bottom() > (ty - half_h).integer();
}

// Helper to get tile world coordinates
static bn::fixed get_tile_world_x(int col) { return (col * 8) - 116; }
static bn::fixed get_tile_world_y(int row) { return (row * 8) - 60; }

// ─────────────────────────────────────────────────────────────────────────────
// Universal Traps
// ─────────────────────────────────────────────────────────────────────────────

// TrapFallingBlock
void TrapFallingBlock::update(Player& player, Level& level)
{
    if(!_initialized)
    {
        _curr_x = get_tile_world_x(_tx);
        _curr_y = get_tile_world_y(_ty);
        _initialized = true;
    }

    if(!_sprite.has_value())
    {
        // Use boulder as visual representation — hidden until triggered
        _sprite = bn::sprite_items::boulder.create_sprite(_curr_x, _curr_y);
        _sprite->set_visible(false);
    }

    switch(_state)
    {
        case State::WAITING:
            {
                bn::fixed px = player.x();
                bn::fixed trigger_dist = _pb == 0 ? bn::fixed(8) : bn::fixed(_pb * 8);
                if(bn::abs(px - _curr_x) <= trigger_dist && player.bottom() > _curr_y)
                {
                    _state = State::SHAKING;
                    _timer = 0;
                    _sprite->set_visible(true);  // reveal when triggered
                    SfxPlayer::play(SfxCue::TRAP_WARNING);
                }
            }
            break;

        case State::SHAKING:
            _timer++;
            if(_timer >= _pa) // _pa = delay_frames
            {
                _state = State::FALLING;
                _vy = 0;
                SfxPlayer::play(SfxCue::STONE_MOVE);
            }
            else
            {
                _sprite->set_x(_curr_x + ((_timer % 2 == 0) ? -1 : 1));
            }
            break;

        case State::FALLING:
            _vy += physics::GRAVITY;
            if(_vy > physics::MAX_FALL_SPEED)
            {
                _vy = physics::MAX_FALL_SPEED;
            }
            _curr_y += _vy;

            // Check tile collision at bottom of block
            {
                int col = level.get_tile_col(_curr_x);
                int row = level.get_tile_row(_curr_y + 8);
                if(level.get_tile(col, row) == tile::SOLID || _curr_y + 8 >= GAMEPLAY_BOTTOM)
                {
                    _curr_y = level.get_tile_y(row - 1);
                    _state = State::LANDED;
                    _vy = 0;
                    level.set_tile(col, row - 1, tile::SOLID);
                    SfxPlayer::play(SfxCue::STONE_IMPACT);
                }
            }

            // Check collision with player
            if(check_player_collision(player, _curr_x, _curr_y, 16, 16))
            {
                player.kill();
            }
            break;

        case State::LANDED:
            break;
        default:
            break;
    }

    if(_state != State::SHAKING)
    {
        _sprite->set_position(_curr_x, _curr_y);
    }
}

// TrapConveyor
void TrapConveyor::update(Player& player, Level& level)
{
    (void)level;
    int tile_left = SCREEN_LEFT.integer() + _tx * TILE_SIZE;
    int tile_right = tile_left + TILE_SIZE;
    int tile_top = GAMEPLAY_TOP.integer() + _ty * TILE_SIZE;

    bool horiz_overlap = player.left() < tile_right && player.right() > tile_left;
    bool vert_on_top;
    if(!player.gravity_flipped())
    {
        vert_on_top = (player.bottom() >= tile_top - 2) && (player.bottom() <= tile_top + 2);
    }
    else
    {
        int tile_bottom = tile_top + TILE_SIZE;
        vert_on_top = (player.top() >= tile_bottom - 2) && (player.top() <= tile_bottom + 2);
    }

    if(horiz_overlap && vert_on_top && player.is_on_ground())
    {
        bn::fixed force = bn::fixed(int(_pa)) / 10.0;
        player.apply_conveyor(_pb == 0 ? -force : force);
    }
}

// TrapWind
void TrapWind::update(Player& player, Level& level)
{
    (void)level;
    bn::fixed wx = get_tile_world_x(_tx);
    bn::fixed wy = get_tile_world_y(_ty);
    if(bn::abs(player.x() - wx) < 12 && bn::abs(player.y() - wy) < 12)
    {
        bn::fixed force = bn::fixed(int(_pa)) / 10.0;
        player.apply_wind(_pb == 0 ? -force : force);
    }
}

// TrapInvisibleWall
void TrapInvisibleWall::update(Player& player, Level& level)
{
    (void)player;
    if(!_initialized)
    {
        level.set_tile(_tx, _ty, tile::INVISIBLE_WALL);
        _initialized = true;
    }
}

// TrapFakeDoor
void TrapFakeDoor::update(Player& player, Level& level)
{
    (void)level;
    bn::fixed dx = get_tile_world_x(_tx);
    bn::fixed dy = get_tile_world_y(_ty);
    if(!_sprite.has_value())
    {
        _sprite = bn::sprite_items::exit_door.create_sprite(dx - 4, dy - 12);
    }
    if(check_player_collision(player, dx, dy, 16, 16))
    {
        player.kill();
    }
}

// TrapDarkRoom
void TrapDarkRoom::update(Player& player, Level& level)
{
    bn::rect_window internal_window = bn::rect_window::internal();
    if(level.bg().has_value() && level.bg()->camera().has_value())
    {
        internal_window.set_camera(level.bg()->camera().value());
    }
    
    bn::fixed radius_px = _pa * 8;
    internal_window.set_boundaries(
        player.y() - radius_px,
        player.x() - radius_px,
        player.y() + radius_px,
        player.x() + radius_px
    );
    
    bn::window outside_window = bn::window::outside();
    bn::window inside_window = bn::window::internal();
    inside_window.set_show_all();
    outside_window.set_show_sprites(false);
    if(level.bg().has_value())
    {
        outside_window.set_show_bg(level.bg().value(), false);
    }
}

// TrapMovingPlatform
void TrapMovingPlatform::update(Player& player, Level& level)
{
    (void)level;
    if(!_initialized)
    {
        _curr_x = get_tile_world_x(_tx);
        _curr_y = get_tile_world_y(_ty);
        _initialized = true;
    }

    if(!_sprite.has_value())
    {
        _sprite = bn::sprite_items::moving_platform.create_sprite(_curr_x, _curr_y);
    }

    bn::fixed speed = bn::fixed(int(_pa)) / 10.0;
    bn::fixed limit_left = get_tile_world_x(_tx);
    bn::fixed limit_right = limit_left + _pb * 8;

    if(_moving_forward)
    {
        _curr_x += speed;
        if(_curr_x >= limit_right)
        {
            _curr_x = limit_right;
            _moving_forward = false;
        }
    }
    else
    {
        _curr_x -= speed;
        if(_curr_x <= limit_left)
        {
            _curr_x = limit_left;
            _moving_forward = true;
        }
    }

    _sprite->set_position(_curr_x, _curr_y);

    int platform_left = _curr_x.integer() - 16;
    int platform_right = _curr_x.integer() + 16;
    int platform_top = _curr_y.integer() - 4;

    bool horiz_overlap = player.left() < platform_right && player.right() > platform_left;
    bool vert_on_top;
    if(!player.gravity_flipped())
    {
        vert_on_top = (player.bottom() >= platform_top - 2) && (player.bottom() <= platform_top + 2);
    }
    else
    {
        int platform_bottom = _curr_y.integer() + 4;
        vert_on_top = (player.top() >= platform_bottom - 2) && (player.top() <= platform_bottom + 2);
    }

    if(horiz_overlap && vert_on_top && player.is_on_ground())
    {
        if(!player.gravity_flipped())
        {
            player.push_out_y(platform_top - player.half_height() - player.y());
        }
        else
        {
            int platform_bottom = _curr_y.integer() + 4;
            player.push_out_y(platform_bottom + player.half_height() - player.y());
        }
        player.apply_conveyor(_moving_forward ? speed : -speed);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// World 1 – Egyptian
// ─────────────────────────────────────────────────────────────────────────────

// TrapSandSinkhole
void TrapSandSinkhole::update(Player& player, Level& level)
{
    (void)level;
    if(!_sprite.has_value())
    {
        _sprite = bn::sprite_items::sand_sinkhole.create_sprite(
            get_tile_world_x(_tx), get_tile_world_y(_ty) - 4);
        _sprite->set_bg_priority(1);
    }

    int tile_left = SCREEN_LEFT.integer() + _tx * TILE_SIZE;
    int tile_right = tile_left + TILE_SIZE;
    int tile_top = GAMEPLAY_TOP.integer() + _ty * TILE_SIZE;
    bool horizontal_overlap = player.left() < tile_right && player.right() > tile_left;
    // Trap updates run before tile collision resolves the current frame, so
    // is_on_ground() can briefly be false while the feet still touch sand.
    bool standing_on_sand = horizontal_overlap &&
        player.bottom() >= tile_top - 3 && player.bottom() <= tile_top + 3;

    if(standing_on_sand)
    {
        ++_sink_timer;
        ++_idle_timer;
        _sprite->set_tiles(
            bn::sprite_items::sand_sinkhole.tiles_item().create_tiles(
                (_sink_timer / 4) % 2));
        _sprite->set_x(get_tile_world_x(_tx) + (_sink_timer % 2 ? 1 : -1));
        int sink_frames = bn::max(18, 45 - int(_pa) * 3);
        if(_sink_timer >= sink_frames)
        {
            player.kill();
        }
    }
    else
    {
        _sink_timer = 0;
        ++_idle_timer;
        _sprite->set_tiles(
            bn::sprite_items::sand_sinkhole.tiles_item().create_tiles(
                (_idle_timer / 20) % 2));
        _sprite->set_x(get_tile_world_x(_tx));
    }
}

// TrapBoulder — Trigger-based: waits for player to cross tile_x, then rolls
void TrapBoulder::update(Player& player, Level& level)
{
    (void)level;
    if(!_initialized)
    {
        _curr_x = get_tile_world_x(_tx);
        _curr_y = get_tile_world_y(_ty);
        _initialized = true;
        _state = State::IDLE;
        // Grace period: don't trigger for the first 30 frames (0.5s)
        // so boulder never kills at spawn
        int configured_grace = (_pb >> 1) * 30;
        _cooldown_timer = configured_grace > 0 ? configured_grace : 30;
    }

    bn::fixed speed = bn::fixed(int(_pa)) / 10.0;
    bool from_right = (_pb & 1) != 0;

    switch(_state)
    {
        case State::IDLE:
        {
            // Grace period countdown
            if(_cooldown_timer > 0)
            {
                _cooldown_timer--;
                break;
            }

            // Trigger when player crosses tile_x column (either direction)
            int player_col = (player.x() - SCREEN_LEFT).integer() / TILE_SIZE;
            if(!_triggered)
            {
                // Trigger when player advances past tile_x column
                // (player walks left→right toward exit in normal layout)
                if(player_col >= _tx) _triggered = true;
            }

            if(_triggered)
            {
                // Spawn from configured side
                if(from_right)
                {
                    _curr_x = SCREEN_RIGHT + 16;
                }
                else
                {
                    _curr_x = SCREEN_LEFT - 16;
                }

                if(!_sprite.has_value())
                {
                    _sprite = bn::sprite_items::boulder.create_sprite(_curr_x, _curr_y);
                }
                else
                {
                    _sprite->set_position(_curr_x, _curr_y);
                    _sprite->set_visible(true);
                }

                _state = State::ROLLING;
                _anim_frame = 0;
                _anim_timer = 0;
                SfxPlayer::play(SfxCue::STONE_MOVE);
            }
            break;
        }

        case State::ROLLING:
        {
            if(from_right)
            {
                _curr_x -= speed;
            }
            else
            {
                _curr_x += speed;
            }

            // Rolling animation
            _anim_timer++;
            if(_anim_timer >= 6)
            {
                _anim_timer = 0;
                _anim_frame = (_anim_frame + 1) % 4;
                _sprite->set_tiles(bn::sprite_items::boulder.tiles_item().create_tiles(_anim_frame));
            }
            _sprite->set_horizontal_flip(from_right);
            _sprite->set_position(_curr_x, _curr_y);

            // Collision
            if(check_player_collision(player, _curr_x, _curr_y, 14, 14))
            {
                player.kill();
            }

            // Off-screen → cooldown
            if((from_right && _curr_x < SCREEN_LEFT - 20) ||
               (!from_right && _curr_x > SCREEN_RIGHT + 20))
            {
                _sprite->set_visible(false);
                _cooldown_timer = 90;  // ~1.5 seconds before re-arming
                _state = State::COOLDOWN;
            }
            break;
        }

        case State::COOLDOWN:
        {
            _cooldown_timer--;
            if(_cooldown_timer <= 0)
            {
                // Re-arm: go back to IDLE, keep _triggered so it fires again immediately
                _state = State::IDLE;
            }
            break;
        }

        default:
            break;
    }
}

// TrapBounceBoulder — Falls diagonally from top, bounces once, then rolls
void TrapBounceBoulder::update(Player& player, Level& level)
{
    (void)level;
    if(!_initialized)
    {
        _curr_x = get_tile_world_x(_tx);
        _curr_y = get_tile_world_y(_ty);
        _ground_y = _curr_y;  // tile_y defines the ground/bounce row
        _initialized = true;
        _state = State::IDLE;
        _cooldown_timer = 30;  // grace period
    }

    bn::fixed speed = bn::fixed(int(_pa)) / 10.0;
    bool from_right = (_pb != 0);

    switch(_state)
    {
        case State::IDLE:
        {
            // Grace period countdown
            if(_cooldown_timer > 0)
            {
                _cooldown_timer--;
                break;
            }

            // Trigger when player crosses tile_x column
            int player_col = (player.x() - SCREEN_LEFT).integer() / TILE_SIZE;
            if(!_triggered)
            {
                // Always trigger when player moves past tile_x (toward exit)
                if(player_col >= _tx) _triggered = true;
            }

            if(_triggered)
            {
                // Spawn from top corner
                if(from_right)
                {
                    _curr_x = SCREEN_RIGHT + 16;
                }
                else
                {
                    _curr_x = SCREEN_LEFT - 16;
                }
                _curr_y = GAMEPLAY_TOP - 16;  // start above screen
                _vy = 0;

                if(!_sprite.has_value())
                {
                    _sprite = bn::sprite_items::boulder.create_sprite(_curr_x, _curr_y);
                }
                else
                {
                    _sprite->set_position(_curr_x, _curr_y);
                    _sprite->set_visible(true);
                }

                _state = State::FALLING;
                _anim_frame = 0;
                _anim_timer = 0;
                SfxPlayer::play(SfxCue::STONE_MOVE);
            }
            break;
        }

        case State::FALLING:
        {
            // Move diagonally: horizontal + vertical
            if(from_right)
            {
                _curr_x -= speed;
            }
            else
            {
                _curr_x += speed;
            }
            _vy += physics::GRAVITY;
            if(_vy > physics::MAX_FALL_SPEED) _vy = physics::MAX_FALL_SPEED;
            _curr_y += _vy;

            // Hit ground → bounce
            if(_curr_y >= _ground_y)
            {
                _curr_y = _ground_y;
                _vy = -bn::fixed(3.5);  // bounce upward
                _state = State::BOUNCING;
                SfxPlayer::play(SfxCue::STONE_IMPACT);
            }

            // Animate
            _anim_timer++;
            if(_anim_timer >= 4)
            {
                _anim_timer = 0;
                _anim_frame = (_anim_frame + 1) % 4;
                _sprite->set_tiles(bn::sprite_items::boulder.tiles_item().create_tiles(_anim_frame));
            }
            _sprite->set_horizontal_flip(from_right);
            _sprite->set_position(_curr_x, _curr_y);

            if(check_player_collision(player, _curr_x, _curr_y, 14, 14))
            {
                player.kill();
            }
            break;
        }

        case State::BOUNCING:
        {
            // Continue horizontal + vertical (arc)
            if(from_right)
            {
                _curr_x -= speed;
            }
            else
            {
                _curr_x += speed;
            }
            _vy += physics::GRAVITY;
            _curr_y += _vy;

            // Land again → roll
            if(_curr_y >= _ground_y)
            {
                _curr_y = _ground_y;
                _vy = 0;
                _state = State::ROLLING;
                SfxPlayer::play(SfxCue::STONE_MOVE);
            }

            _anim_timer++;
            if(_anim_timer >= 4)
            {
                _anim_timer = 0;
                _anim_frame = (_anim_frame + 1) % 4;
                _sprite->set_tiles(bn::sprite_items::boulder.tiles_item().create_tiles(_anim_frame));
            }
            _sprite->set_horizontal_flip(from_right);
            _sprite->set_position(_curr_x, _curr_y);

            if(check_player_collision(player, _curr_x, _curr_y, 14, 14))
            {
                player.kill();
            }
            break;
        }

        case State::ROLLING:
        {
            // Straight horizontal roll at ground level
            if(from_right)
            {
                _curr_x -= speed;
            }
            else
            {
                _curr_x += speed;
            }

            _anim_timer++;
            if(_anim_timer >= 6)
            {
                _anim_timer = 0;
                _anim_frame = (_anim_frame + 1) % 4;
                _sprite->set_tiles(bn::sprite_items::boulder.tiles_item().create_tiles(_anim_frame));
            }
            _sprite->set_horizontal_flip(from_right);
            _sprite->set_position(_curr_x, _curr_y);

            if(check_player_collision(player, _curr_x, _curr_y, 14, 14))
            {
                player.kill();
            }

            // Off-screen → cooldown
            if((from_right && _curr_x < SCREEN_LEFT - 20) ||
               (!from_right && _curr_x > SCREEN_RIGHT + 20))
            {
                _sprite->set_visible(false);
                _cooldown_timer = 120;  // 2 seconds before re-arming
                _state = State::IDLE;
                // _triggered stays true so it fires again immediately
            }
            break;
        }

        default:
            break;
    }
}

// TrapAutoBoulder — Auto-rolls periodically from a fixed side
void TrapAutoBoulder::update(Player& player, Level& level)
{
    (void)level;
    if(!_initialized)
    {
        _curr_y = get_tile_world_y(_ty);
        _initialized = true;
        _state = State::COOLDOWN;
        _cooldown_timer = 60;  // 1 second initial delay
    }

    bn::fixed speed = bn::fixed(int(_pa)) / 10.0;
    bool from_right = (_pb != 0);

    switch(_state)
    {
        case State::COOLDOWN:
        {
            _cooldown_timer--;
            if(_cooldown_timer <= 0)
            {
                // Spawn from configured side
                if(from_right)
                {
                    _curr_x = SCREEN_RIGHT + 16;
                }
                else
                {
                    _curr_x = SCREEN_LEFT - 16;
                }

                if(!_sprite.has_value())
                {
                    _sprite = bn::sprite_items::boulder.create_sprite(_curr_x, _curr_y);
                }
                else
                {
                    _sprite->set_position(_curr_x, _curr_y);
                    _sprite->set_visible(true);
                }

                _state = State::ROLLING;
                _anim_frame = 0;
                _anim_timer = 0;
                SfxPlayer::play(SfxCue::STONE_MOVE);
            }
            break;
        }

        case State::ROLLING:
        {
            if(from_right)
            {
                _curr_x -= speed;
            }
            else
            {
                _curr_x += speed;
            }

            // Rolling animation
            _anim_timer++;
            if(_anim_timer >= 6)
            {
                _anim_timer = 0;
                _anim_frame = (_anim_frame + 1) % 4;
                _sprite->set_tiles(bn::sprite_items::boulder.tiles_item().create_tiles(_anim_frame));
            }
            _sprite->set_horizontal_flip(from_right);
            _sprite->set_position(_curr_x, _curr_y);

            // Collision
            if(check_player_collision(player, _curr_x, _curr_y, 14, 14))
            {
                player.kill();
            }

            // Off-screen → cooldown
            if((from_right && _curr_x < SCREEN_LEFT - 20) ||
               (!from_right && _curr_x > SCREEN_RIGHT + 20))
            {
                _sprite->set_visible(false);
                // tile_x encodes cooldown: tile_x × 10 frames (e.g. 12 = 120f = 2s)
                _cooldown_timer = _tx > 0 ? _tx * 10 : 90;
                _state = State::COOLDOWN;
            }
            break;
        }

        default:
            break;
    }
}

// TrapSarcophagusPop
void TrapSarcophagusPop::update(Player& player, Level& level)
{
    (void)level;
    if(!_initialized)
    {
        _curr_x = get_tile_world_x(_tx);
        _curr_y = get_tile_world_y(_ty);
        _state = State::IDLE;
        _timer = 0;
        _initialized = true;
    }

    if(!_sprite.has_value())
    {
        // NOTE: Uses terracotta_soldier as a visual placeholder since no sarcophagus asset exists
        _sprite = bn::sprite_items::terracotta_soldier.create_sprite(_curr_x, _curr_y);
        _sprite->set_horizontal_flip(_pb == 0);
        _sprite->set_visible(false);
    }

    _timer++;
    switch(_state)
    {
        case State::IDLE:
            if(_timer >= _pa)
            {
                _state = State::WARNING;
                _timer = 0;
                _sprite->set_visible(true);
                SfxPlayer::play(SfxCue::TRAP_WARNING);
            }
            break;

        case State::WARNING:
            if(_timer >= 30)
            {
                _state = State::POPPED;
                _timer = 0;
                SfxPlayer::play(SfxCue::SPIKE_EXTEND);
            }
            else
            {
                _sprite->set_x(_curr_x + ((_timer % 2 == 0) ? -1 : 1));
            }
            break;

        case State::POPPED:
            if(_timer >= 60)
            {
                _state = State::IDLE;
                _timer = 0;
                _sprite->set_visible(false);
            }
            else
            {
                bn::fixed pop_offset = (_pb == 0 ? -12 : 12);
                _sprite->set_position(_curr_x + pop_offset, _curr_y);

                if(check_player_collision(player, _curr_x + pop_offset, _curr_y, 16, 32))
                {
                    player.kill();
                }
            }
            break;
        default:
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// World 2 – Royal
// ─────────────────────────────────────────────────────────────────────────────

// TrapPendulumAxe
void TrapPendulumAxe::update(Player& player, Level& level)
{
    (void)level;
    if(!_initialized)
    {
        _curr_x = get_tile_world_x(_tx);
        _curr_y = get_tile_world_y(_ty);
        _timer = 0;
        _initialized = true;
    }

    if(!_sprite.has_value())
    {
        _sprite = bn::sprite_items::pendulum_axe.create_sprite(_curr_x, _curr_y);
    }

    _timer++;
    int period = _pa > 0 ? _pa : 60;
    int radius = _pb > 0 ? _pb : 24;
    bn::fixed angle = bn::degrees_sin((_timer * 360) / period);

    bn::fixed ax = _curr_x + angle * radius;
    bn::fixed ay = _curr_y + bn::degrees_cos((_timer * 360) / period) * radius;

    _sprite->set_position(ax, ay);
    bn::fixed rotation = angle * 45;
    if(rotation < 0)
    {
        rotation += 360;
    }
    _sprite->set_rotation_angle(rotation);

    if(check_player_collision(player, ax, ay, 12, 12))
    {
        player.kill();
    }
}

// TrapArrowWall
void TrapArrowWall::update(Player& player, Level& level)
{
    _timer++;
    if(_timer >= _pa)
    {
        _timer = 0;
        if(_arrows.size() < 4)
        {
            Arrow arrow;
            arrow.x = get_tile_world_x(_tx);
            arrow.y = get_tile_world_y(_ty);
            arrow.sprite = bn::sprite_items::arrow.create_sprite(arrow.x, arrow.y);
            arrow.sprite->set_horizontal_flip(_pb == 1);
            arrow.active = true;
            _arrows.push_back(bn::move(arrow));
            SfxPlayer::play(
                SfxCue::PROJECTILE_FIRE, bn::fixed(1.05),
                _pb == 0 ? bn::fixed(-0.18) : bn::fixed(0.18));
        }
    }

    for(int i = 0; i < _arrows.size(); )
    {
        auto& arrow = _arrows[i];
        arrow.x += (_pb == 0 ? -2.0 : 2.0);
        arrow.sprite->set_position(arrow.x, arrow.y);

        if(arrow.x < SCREEN_LEFT - 8 || arrow.x > SCREEN_RIGHT + 8)
        {
            _arrows.erase(_arrows.begin() + i);
        }
        else
        {
            if(level.blocks_projectile(arrow.x, arrow.y))
            {
                _arrows.erase(_arrows.begin() + i);
            }
            else
            {
                if(check_player_collision(player, arrow.x, arrow.y, 8, 8))
                {
                    player.kill();
                }
                i++;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// World 3 – Qin
// ─────────────────────────────────────────────────────────────────────────────

// TrapTerracottaSoldier
void TrapTerracottaSoldier::update(Player& player, Level& level)
{
    (void)level;
    if(!_initialized)
    {
        _curr_x = get_tile_world_x(_tx);
        _curr_y = get_tile_world_y(_ty);
        _state = State::IDLE;
        _timer = 0;
        _initialized = true;
    }

    if(!_sprite.has_value())
    {
        _sprite = bn::sprite_items::terracotta_soldier.create_sprite(_curr_x, _curr_y);
        _sprite->set_horizontal_flip(_pb == 0);
    }

    switch(_state)
    {
        case State::IDLE:
            _sprite->set_position(_curr_x, _curr_y);
            if(bn::abs(player.x() - _curr_x) <= _pa * 8 && bn::abs(player.y() - _curr_y) <= _pa * 8)
            {
                _state = State::EXTENDING;
                _timer = 0;
                SfxPlayer::play(SfxCue::SPIKE_EXTEND);
            }
            break;

        case State::EXTENDING:
            _timer++;
            {
                bn::fixed offset = (_pb == 0 ? -bn::fixed(_timer) : bn::fixed(_timer));
                _sprite->set_position(_curr_x + offset, _curr_y);
                if(_timer >= 12)
                {
                    _state = State::EXTENDED;
                    _timer = 0;
                }
            }
            break;

        case State::EXTENDED:
            _timer++;
            if(_timer >= 30)
            {
                _state = State::RETRACTING;
                _timer = 0;
            }
            break;

        case State::RETRACTING:
            _timer++;
            {
                bn::fixed offset = (_pb == 0 ? -bn::fixed(12 - _timer) : bn::fixed(12 - _timer));
                _sprite->set_position(_curr_x + offset, _curr_y);
                if(_timer >= 12)
                {
                    _state = State::IDLE;
                    _timer = 0;
                }
            }
            break;
        default:
            break;
    }

    if(_state != State::IDLE)
    {
        if(check_player_collision(player, _sprite->x(), _sprite->y(), 16, 32))
        {
            player.kill();
        }
    }
}

// TrapPressurePlate
void TrapPressurePlate::update(Player& player, Level& level)
{
    (void)player;
    (void)level;
}

void TrapPressurePlate::update_with_manager(Player& player, TrapManager& manager)
{
    if(!_sprite.has_value())
    {
        _sprite = bn::sprite_items::pressure_plate.create_sprite(
            get_tile_world_x(_tx), get_tile_world_y(_ty) - 3);
    }

    int tile_left = SCREEN_LEFT.integer() + _tx * TILE_SIZE;
    int tile_right = tile_left + TILE_SIZE;
    int tile_top = GAMEPLAY_TOP.integer() + _ty * TILE_SIZE;

    bool horiz_overlap = player.left() < tile_right && player.right() > tile_left;
    bool vert_on_top;
    if(!player.gravity_flipped())
    {
        vert_on_top = (player.bottom() >= tile_top - 2) && (player.bottom() <= tile_top + 2);
    }
    else
    {
        int tile_bottom = tile_top + TILE_SIZE;
        vert_on_top = (player.top() >= tile_bottom - 2) && (player.top() <= tile_bottom + 2);
    }

    bool on_top = horiz_overlap && vert_on_top && player.is_on_ground();
    _sprite->set_tiles(
        bn::sprite_items::pressure_plate.tiles_item().create_tiles(on_top ? 1 : 0));

    if(on_top)
    {
        if(!_stepped_on)
        {
            _stepped_on = true;
            SfxPlayer::play(SfxCue::PLATE_CLICK);
            auto& traps = manager.traps();
            if(_pa < traps.size())
            {
                traps[_pa]->force_activate();
            }
        }
    }
    else
    {
        _stepped_on = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// World 4 – Aztec
// ─────────────────────────────────────────────────────────────────────────────

// TrapDartFrog
void TrapDartFrog::update(Player& player, Level& level)
{
    (void)level;
    _timer++;
    if(_timer >= _pa)
    {
        _timer = 0;
        if(_darts.size() < 4)
        {
            Dart dart;
            dart.x = get_tile_world_x(_tx);
            dart.y = get_tile_world_y(_ty);
            dart.sprite = bn::sprite_items::dart.create_sprite(dart.x, dart.y);
            dart.sprite->set_horizontal_flip(_pb == 1);
            dart.active = true;
            _darts.push_back(bn::move(dart));
            SfxPlayer::play(
                SfxCue::PROJECTILE_FIRE, bn::fixed(1.24),
                _pb == 0 ? bn::fixed(-0.12) : bn::fixed(0.12));
        }
    }

    for(int i = 0; i < _darts.size(); )
    {
        auto& dart = _darts[i];
        dart.x += (_pb == 0 ? -1.5 : 1.5);
        dart.sprite->set_position(dart.x, dart.y);

        if(dart.x < SCREEN_LEFT - 8 || dart.x > SCREEN_RIGHT + 8)
        {
            _darts.erase(_darts.begin() + i);
        }
        else
        {
            if(check_player_collision(player, dart.x, dart.y, 6, 6))
            {
                player.kill();
            }
            i++;
        }
    }
}

// TrapLavaRise
void TrapLavaRise::update(Player& player, Level& level)
{
    (void)level;
    if(!_initialized)
    {
        _curr_y = GAMEPLAY_BOTTOM;
        _initialized = true;
        SfxPlayer::play(SfxCue::LAVA_RISE);
    }

    if(_sprites.empty())
    {
        for(int i = 0; i < 15; ++i)
        {
            bn::fixed wx = SCREEN_LEFT + i * 16 + 8;
            _sprites.push_back(bn::sprite_items::lava_rise.create_sprite(wx, _curr_y));
        }
    }

    bn::fixed speed = bn::fixed(int(_pa)) / 100.0;
    _curr_y -= speed;

    bn::fixed peak = get_tile_world_y(_pb);
    if(_curr_y < peak)
    {
        _curr_y = peak;
    }

    for(bn::sprite_ptr& sprite : _sprites)
    {
        sprite.set_y(_curr_y);
    }

    bool collides = false;
    if(!player.gravity_flipped())
    {
        collides = (player.bottom() >= _curr_y.integer());
    }
    else
    {
        collides = (player.top() >= _curr_y.integer());
    }
    if(collides)
    {
        player.kill();
    }
}


// TrapCrumbleFloor
void TrapCrumbleFloor::update(Player& player, Level& level)
{
    bool on_top = player_on_top(player);

    switch(_state)
    {
        case State::SOLID:
            if(on_top)
            {
                _state = State::CRACKED;
                _timer = 0;
                SfxPlayer::play(SfxCue::FLOOR_CRACK);
            }
            break;

        case State::CRACKED:
            _timer++;
            {
                int delay = _step_count > 0 ? _step_count : 30;
                if(_timer >= delay)
                {
                    _state = State::GONE;
                    _timer = 0;
                    level.set_tile(_tx, _ty, tile::EMPTY);
                    SfxPlayer::play(SfxCue::FLOOR_BREAK);
                }
            }
            break;

        case State::GONE:
            if(_pb > 0)
            {
                _timer++;
                if(_timer >= _pb)
                {
                    _state = State::RESPAWNING;
                    _timer = 0;
                }
            }
            break;

        case State::RESPAWNING:
            level.set_tile(_tx, _ty, tile::SOLID);
            _state = State::SOLID;
            break;
        default:
            break;
    }

    _player_was_on_top = on_top;
}

bool TrapCrumbleFloor::player_on_top(const Player& player) const
{
    int tile_left = SCREEN_LEFT.integer() + _tx * TILE_SIZE;
    int tile_right = tile_left + TILE_SIZE;
    int tile_top = GAMEPLAY_TOP.integer() + _ty * TILE_SIZE;

    bool horiz_overlap = player.left() < tile_right && player.right() > tile_left;
    bool vert_on_top;
    if(!player.gravity_flipped())
    {
        vert_on_top = (player.bottom() >= tile_top - 2) && (player.bottom() <= tile_top + 2);
    }
    else
    {
        int tile_bottom = tile_top + TILE_SIZE;
        vert_on_top = (player.top() >= tile_bottom - 2) && (player.top() <= tile_bottom + 2);
    }

    return horiz_overlap && vert_on_top && player.is_on_ground();
}

// ─────────────────────────────────────────────────────────────────────────────
// World 5 – Japanese Kofun
// ─────────────────────────────────────────────────────────────────────────────

// TrapBambooSpike
void TrapBambooSpike::update(Player& player, Level& level)
{
    (void)level;
    if(!_initialized)
    {
        _state = State::HIDDEN;
        _timer = 0;
        _initialized = true;
    }

    if(!_sprite.has_value())
    {
        _sprite = bn::sprite_items::bamboo_grow.create_sprite(get_tile_world_x(_tx), get_tile_world_y(_ty));
        _sprite->set_visible(false);
    }

    switch(_state)
    {
        case State::HIDDEN:
            _sprite->set_visible(false);
            if(player_in_range(player))
            {
                _state = State::GROWING;
                _timer = 0;
                _sprite->set_visible(true);
                SfxPlayer::play(SfxCue::SPIKE_EXTEND, bn::fixed(0.88));
            }
            break;

        case State::GROWING:
            _timer++;
            if(_timer >= 30)
            {
                _state = State::GROWN;
                _timer = 0;
                _sprite->set_tiles(bn::sprite_items::bamboo_grow.tiles_item().create_tiles(4));
            }
            else
            {
                _sprite->set_tiles(bn::sprite_items::bamboo_grow.tiles_item().create_tiles(_timer / 6));
            }
            
            // Check collision with partial hitbox during growing state
            {
                int frame = bn::min(4, _timer / 6);
                int h = (frame + 1) * 6;
                bn::fixed partial_y;
                if(!player.gravity_flipped())
                {
                    partial_y = _sprite->y() + 16 - bn::fixed(h) / 2;
                }
                else
                {
                    partial_y = _sprite->y() - 16 + bn::fixed(h) / 2;
                }
                if(check_player_collision(player, _sprite->x(), partial_y, 8, h))
                {
                    player.kill();
                }
            }
            break;

        case State::GROWN:
            _timer++;
            _sprite->set_tiles(bn::sprite_items::bamboo_grow.tiles_item().create_tiles(4));
            if(_timer >= 60)
            {
                _state = State::RETRACTING;
                _timer = 0;
            }
            if(check_player_collision(player, _sprite->x(), _sprite->y(), 8, 32))
            {
                player.kill();
            }
            break;

        case State::RETRACTING:
            _timer++;
            if(_timer >= 30)
            {
                _state = State::HIDDEN;
                _timer = 0;
            }
            else
            {
                int frame = 4 - (_timer / 6);
                if(frame < 0) frame = 0;
                _sprite->set_tiles(bn::sprite_items::bamboo_grow.tiles_item().create_tiles(frame));
            }
            break;
        default:
            break;
    }
}

bool TrapBambooSpike::player_in_range(const Player& player) const
{
    bn::fixed wx = get_tile_world_x(_tx);
    bn::fixed wy = get_tile_world_y(_ty);
    return bn::abs(player.x() - wx) <= _pa * 8 && bn::abs(player.y() - wy) <= _pa * 8;
}

// TrapSamuraiStatue
void TrapSamuraiStatue::update(Player& player, Level& level)
{
    (void)level;
    if(!_initialized)
    {
        _state = State::IDLE;
        _timer = 0;
        _initialized = true;
    }

    if(!_sprite.has_value())
    {
        _sprite = bn::sprite_items::samurai_statue.create_sprite(get_tile_world_x(_tx), get_tile_world_y(_ty));
    }

    switch(_state)
    {
        case State::IDLE:
            _sprite->set_tiles(bn::sprite_items::samurai_statue.tiles_item().create_tiles(0));
            if(bn::abs(player.x() - _sprite->x()) <= _pb * 8 && bn::abs(player.y() - _sprite->y()) <= _pb * 8)
            {
                _state = State::WARNING;
                _timer = 0;
                SfxPlayer::play(SfxCue::TRAP_WARNING);
            }
            break;

        case State::WARNING:
            _timer++;
            if(_timer >= _pa)
            {
                _state = State::SWINGING;
                _timer = 0;
                SfxPlayer::play(SfxCue::SPIKE_EXTEND, bn::fixed(0.78));
            }
            else
            {
                _sprite->set_x(get_tile_world_x(_tx) + ((_timer % 2 == 0) ? -1 : 1));
            }
            break;

        case State::SWINGING:
            _timer++;
            if(_timer >= 15)
            {
                _state = State::IDLE;
                _timer = 0;
            }
            else
            {
                _sprite->set_x(get_tile_world_x(_tx));
                int frame_idx = 0;
                if(bn::sprite_items::samurai_statue.tiles_item().graphics_count() > 1)
                {
                    frame_idx = 1;
                }
                _sprite->set_tiles(bn::sprite_items::samurai_statue.tiles_item().create_tiles(frame_idx));

                if(check_player_collision(player, _sprite->x(), _sprite->y(), 24, 32))
                {
                    player.kill();
                }
            }
            break;
        default:
            break;
    }
}

// TrapPaperFloor
void TrapPaperFloor::update(Player& player, Level& level)
{
    bool on_top = player_on_top(player);

    switch(_state)
    {
        case State::SOLID:
            if(on_top)
            {
                if(_timer == 0)
                {
                    SfxPlayer::play(SfxCue::FLOOR_CRACK, bn::fixed(1.12));
                }
                _timer++;
                if(_timer >= _pa)
                {
                    _state = State::RIPPED;
                    _timer = 0;
                    level.set_tile(_tx, _ty, tile::EMPTY);
                    SfxPlayer::play(SfxCue::FLOOR_BREAK, bn::fixed(1.15));
                }
            }
            else
            {
                _timer = 0;
            }
            break;

        case State::RIPPED:
            _state = State::GONE;
            break;

        case State::GONE:
            break;
        default:
            break;
    }

    _player_was_on_top = on_top;
}

bool TrapPaperFloor::player_on_top(const Player& player) const
{
    int tile_left = SCREEN_LEFT.integer() + _tx * TILE_SIZE;
    int tile_right = tile_left + TILE_SIZE;
    int tile_top = GAMEPLAY_TOP.integer() + _ty * TILE_SIZE;

    bool horiz_overlap = player.left() < tile_right && player.right() > tile_left;
    bool vert_on_top;
    if(!player.gravity_flipped())
    {
        vert_on_top = (player.bottom() >= tile_top - 2) && (player.bottom() <= tile_top + 2);
    }
    else
    {
        int tile_bottom = tile_top + TILE_SIZE;
        vert_on_top = (player.top() >= tile_bottom - 2) && (player.top() <= tile_bottom + 2);
    }

    return horiz_overlap && vert_on_top && player.is_on_ground();
}

void TrapFallingBlock::force_activate()
{
    if(_state == State::WAITING)
    {
        _state = State::SHAKING;
        _timer = 0;
        SfxPlayer::play(SfxCue::TRAP_WARNING);
    }
}

void TrapTerracottaSoldier::force_activate()
{
    if(_state == State::IDLE)
    {
        _state = State::EXTENDING;
        _timer = 0;
        SfxPlayer::play(SfxCue::SPIKE_EXTEND);
    }
}

void TrapSarcophagusPop::force_activate()
{
    if(_state == State::IDLE)
    {
        _state = State::WARNING;
        _timer = 0;
        if(_sprite.has_value())
        {
            _sprite->set_visible(true);
        }
        SfxPlayer::play(SfxCue::TRAP_WARNING);
    }
}

void TrapBambooSpike::force_activate()
{
    if(_state == State::HIDDEN)
    {
        _state = State::GROWING;
        _timer = 0;
        if(_sprite.has_value())
        {
            _sprite->set_visible(true);
        }
        SfxPlayer::play(SfxCue::SPIKE_EXTEND, bn::fixed(0.88));
    }
}

void TrapSamuraiStatue::force_activate()
{
    if(_state == State::IDLE)
    {
        _state = State::WARNING;
        _timer = 0;
        SfxPlayer::play(SfxCue::TRAP_WARNING);
    }
}

} // namespace game
