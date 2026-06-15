#include "trap_manager.h"
#include "trap_gravity_flip.h"
#include "trap_pop_spike.h"
#include "trap_vanish_floor.h"
#include "sfx_player.h"
#include "bn_sprite_items_spike_pop.h"
#include "bn_sprite_tiles_ptr.h"
#include "bn_window.h"
#include "player.h"
#include "level.h"
#include "traps_impl.h"
#include "bn_sprite_items_gravity_rune.h"

namespace game {

// ─────────────────────────────────────────────────────────────────────────────
// TrapManager Implementation
// ─────────────────────────────────────────────────────────────────────────────

void TrapManager::load(const ChamberDef& chamber, bool gravity_flipped)
{
    clear();
    _gravity_flipped = gravity_flipped;
    for(int i = 0; i < chamber.trap_count; ++i)
    {
        if(_traps.full())
        {
            break;
        }
        bn::unique_ptr<TrapBase> trap = make_trap(chamber.traps[i]);
        if(trap)
        {
            _traps.push_back(bn::move(trap));
        }
    }
    set_gravity_flipped(_gravity_flipped);
}

void TrapManager::update(Player& player, Level& level)
{
    for(auto& trap : _traps)
    {
        trap->update(player, level);
        if(trap->type() == TrapType::GRAVITY_FLIP)
        {
            auto* flip_trap = static_cast<TrapGravityFlip*>(trap.get());
            if(flip_trap->consume_flip_request())
            {
                _gravity_flipped = !_gravity_flipped;
            }
        }
    }
    
    // Coordination: check if pressure plates triggered other traps
    for(auto& trap : _traps)
    {
        if(trap->type() == TrapType::PRESSURE_PLATE)
        {
            auto* plate = static_cast<TrapPressurePlate*>(trap.get());
            plate->update_with_manager(player, *this);
        }
    }
    
    set_gravity_flipped(_gravity_flipped);
}

void TrapManager::clear(bool preserve_gravity)
{
    _traps.clear();
    bn::window::internal().restore();
    bn::window::outside().restore();
    if(!preserve_gravity)
    {
        _gravity_flipped = false;
    }
}

void TrapManager::set_gravity_flipped(bool value)
{
    _gravity_flipped = value;
    for(auto& trap : _traps)
    {
        if(trap->type() == TrapType::GRAVITY_FLIP)
        {
            static_cast<TrapGravityFlip*>(trap.get())->set_flipped(value);
        }
    }
}

bool TrapManager::consume_flip_sfx()
{
    bool pending = false;
    for(auto& trap : _traps)
    {
        if(trap->type() == TrapType::GRAVITY_FLIP)
        {
            auto* flip_trap = static_cast<TrapGravityFlip*>(trap.get());
            if(flip_trap->consume_sfx_request())
            {
                pending = true;
            }
        }
    }
    return pending;
}

bn::unique_ptr<TrapBase> TrapManager::make_trap(const TrapPlacement& p)
{
    switch(p.type)
    {
        case TrapType::GRAVITY_FLIP:
            return bn::unique_ptr<TrapBase>(new TrapGravityFlip(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::POP_SPIKE:
            {
                TrapPopSpike::Direction dir = TrapPopSpike::Direction::UP;
                if(p.param_b > 100)
                    dir = TrapPopSpike::Direction::DOWN;
                return bn::unique_ptr<TrapBase>(new TrapPopSpike(p.tile_x, p.tile_y, p.param_a, p.param_b, dir));
            }
        case TrapType::VANISH_FLOOR:
            return bn::unique_ptr<TrapBase>(new TrapVanishFloor(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::FALLING_BLOCK:
            return bn::unique_ptr<TrapBase>(new TrapFallingBlock(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::CONVEYOR:
            return bn::unique_ptr<TrapBase>(new TrapConveyor(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::WIND:
            return bn::unique_ptr<TrapBase>(new TrapWind(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::INVISIBLE_WALL:
            return bn::unique_ptr<TrapBase>(new TrapInvisibleWall(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::FAKE_DOOR:
            return bn::unique_ptr<TrapBase>(new TrapFakeDoor(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::DARK_ROOM:
            return bn::unique_ptr<TrapBase>(new TrapDarkRoom(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::MOVING_PLATFORM:
            return bn::unique_ptr<TrapBase>(new TrapMovingPlatform(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::SAND_SINKHOLE:
            return bn::unique_ptr<TrapBase>(new TrapSandSinkhole(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::BOULDER:
            return bn::unique_ptr<TrapBase>(new TrapBoulder(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::SARCOPHAGUS_POP:
            return bn::unique_ptr<TrapBase>(new TrapSarcophagusPop(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::PENDULUM_AXE:
            return bn::unique_ptr<TrapBase>(new TrapPendulumAxe(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::ARROW_WALL:
            return bn::unique_ptr<TrapBase>(new TrapArrowWall(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::TERRACOTTA_SOLDIER:
            return bn::unique_ptr<TrapBase>(new TrapTerracottaSoldier(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::PRESSURE_PLATE:
            return bn::unique_ptr<TrapBase>(new TrapPressurePlate(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::DART_FROG:
            return bn::unique_ptr<TrapBase>(new TrapDartFrog(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::LAVA_RISE:
            return bn::unique_ptr<TrapBase>(new TrapLavaRise(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::CRUMBLE_FLOOR:
            return bn::unique_ptr<TrapBase>(new TrapCrumbleFloor(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::BAMBOO_SPIKE:
            return bn::unique_ptr<TrapBase>(new TrapBambooSpike(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::SAMURAI_STATUE:
            return bn::unique_ptr<TrapBase>(new TrapSamuraiStatue(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::PAPER_FLOOR:
            return bn::unique_ptr<TrapBase>(new TrapPaperFloor(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::BOUNCE_BOULDER:
            return bn::unique_ptr<TrapBase>(new TrapBounceBoulder(p.tile_x, p.tile_y, p.param_a, p.param_b));
        case TrapType::AUTO_BOULDER:
            return bn::unique_ptr<TrapBase>(new TrapAutoBoulder(p.tile_x, p.tile_y, p.param_a, p.param_b));
        default:
            return nullptr;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TrapGravityFlip Implementation
// ─────────────────────────────────────────────────────────────────────────────

void TrapGravityFlip::update(Player& player, Level& level)
{
    (void)level;
    if(!_sprite.has_value())
    {
        bn::fixed wx = SCREEN_LEFT + _tx * TILE_SIZE + TILE_SIZE / 2;
        bn::fixed wy = GAMEPLAY_TOP + _ty * TILE_SIZE + TILE_SIZE / 2;
        _sprite = bn::sprite_items::gravity_rune.create_sprite(wx, wy);
    }

    if(_pa == 0) // Touch trigger
    {
        bool in_zone = player_in_zone(player);
        if(in_zone && !_player_inside)
        {
            _flip_requested = true;
            _flip_sfx_pending = true;
        }
        _player_inside = in_zone;
    }
    else // Timer trigger
    {
        if(_pb > 0)
        {
            _timer++;
            if(_timer >= _pb)
            {
                _flip_requested = true;
                _flip_sfx_pending = true;
                _timer = 0;
            }
        }
    }

    _sprite->set_vertical_flip(_flipped);
}

bool TrapGravityFlip::player_in_zone(const Player& player) const
{
    int tile_left = SCREEN_LEFT.integer() + _tx * TILE_SIZE;
    int tile_right = tile_left + TILE_SIZE;
    int tile_top = GAMEPLAY_TOP.integer() + _ty * TILE_SIZE;
    int tile_bottom = tile_top + TILE_SIZE;
    return player.left() < tile_right && player.right() > tile_left &&
           player.top() < tile_bottom && player.bottom() >= tile_top - 2;
}

// ─────────────────────────────────────────────────────────────────────────────
// TrapPopSpike Implementation
// ─────────────────────────────────────────────────────────────────────────────

void TrapPopSpike::update(Player& player, Level& level)
{
    (void)level;
    if(!_sprite.has_value())
    {
        bn::fixed wx = SCREEN_LEFT + _tx * TILE_SIZE + TILE_SIZE/2;
        // Base position: center of the FLOOR tile (embedded in ground)
        bn::fixed wy = GAMEPLAY_TOP + _ty * TILE_SIZE + TILE_SIZE/2;
        _sprite = bn::sprite_items::spike_pop.create_sprite(wx, wy);
        _sprite->set_vertical_flip(_dir == Direction::DOWN);
        _sprite->set_visible(false);
        // Render BEHIND the tilemap BG so ground tiles clip the base
        _sprite->set_bg_priority(3);
    }

    // How far above the ground surface the tip pokes (pixels)
    constexpr int SPIKE_RISE = 6;
    // Base Y = center of the floor tile (fully hidden)
    bn::fixed base_y = GAMEPLAY_TOP + _ty * TILE_SIZE + TILE_SIZE/2;

    switch(_state)
    {
        case State::HIDDEN:
            _active = false;
            _sprite->set_visible(false);
            _sprite->set_y(base_y);  // stay embedded
            if(player_in_range(player))
            {
                _state = State::EXTENDING;
                _timer = 0;
                _sprite->set_visible(true);
                SfxPlayer::play(SfxCue::SPIKE_EXTEND);
            }
            break;
        case State::EXTENDING:
            _active = true;
            _sprite->set_visible(true);
            {
                int frame = bn::min(3, _timer / 3);
                _sprite->set_tiles(bn::sprite_items::spike_pop.tiles_item().create_tiles(frame));
                // Rise: from base_y (hidden) to base_y - SPIKE_RISE (tip above ground)
                bn::fixed progress = bn::fixed(_timer) / 12;
                if(_dir == Direction::UP)
                {
                    _sprite->set_y(base_y - progress * SPIKE_RISE);
                }
                else
                {
                    _sprite->set_y(base_y + progress * SPIKE_RISE);
                }
            }
            _timer++;
            if(_timer >= 12)
            {
                _state = State::EXTENDED;
                _timer = 0;
            }
            break;
        case State::EXTENDED:
            _active = true;
            _sprite->set_visible(true);
            _sprite->set_tiles(bn::sprite_items::spike_pop.tiles_item().create_tiles(3));
            // Stay at peak position
            if(_dir == Direction::UP)
            {
                _sprite->set_y(base_y - SPIKE_RISE);
            }
            else
            {
                _sprite->set_y(base_y + SPIKE_RISE);
            }
            if(_pb > 0)
            {
                _timer++;
                if(_timer >= _pb)
                {
                    _state = State::RETRACTING;
                    _timer = 0;
                }
            }
            break;
        case State::RETRACTING:
            _active = (_timer < 6);
            _sprite->set_visible(true);
            {
                int frame = bn::max(0, 3 - _timer / 3);
                _sprite->set_tiles(bn::sprite_items::spike_pop.tiles_item().create_tiles(frame));
                // Sink: from peak back to base_y (hidden)
                bn::fixed progress = bn::fixed(_timer) / 12;
                if(_dir == Direction::UP)
                {
                    _sprite->set_y(base_y - SPIKE_RISE + progress * SPIKE_RISE);
                }
                else
                {
                    _sprite->set_y(base_y + SPIKE_RISE - progress * SPIKE_RISE);
                }
            }
            _timer++;
            if(_timer >= 12)
            {
                _state = State::HIDDEN;
                _active = false;
                _timer = 0;
                _sprite->set_y(base_y);
            }
            break;
        default:
            break;
    }

    if(_active)
    {
        int tile_left = SCREEN_LEFT.integer() + _tx * TILE_SIZE;
        int tile_right = tile_left + TILE_SIZE;
        int tile_top = GAMEPLAY_TOP.integer() + _ty * TILE_SIZE;
        int tile_bottom = tile_top + TILE_SIZE;

        if(player.left() < tile_right && player.right() > tile_left &&
           player.top() < tile_bottom && player.bottom() >= tile_top - 2)
        {
            player.kill();
        }
    }
}

bool TrapPopSpike::player_in_range(const Player& player) const
{
    int px_tile = (player.x() - SCREEN_LEFT).integer() / TILE_SIZE;
    int py_tile = (player.y() - GAMEPLAY_TOP).integer() / TILE_SIZE;
    int dist_x = px_tile - _tx;
    int dist_y = py_tile - _ty;
    if(dist_x < 0) dist_x = -dist_x;
    if(dist_y < 0) dist_y = -dist_y;
    return dist_x <= _pa && dist_y <= _pa;
}

// ─────────────────────────────────────────────────────────────────────────────
// TrapVanishFloor Implementation
// ─────────────────────────────────────────────────────────────────────────────

void TrapVanishFloor::update(Player& player, Level& level)
{
    if(_left_col == -1)
    {
        _left_col = _tx;
        while(_left_col > 0 && level.get_tile(_left_col - 1, _ty) == tile::VANISH)
        {
            _left_col--;
        }
        _right_col = _tx;
        while(_right_col < LEVEL_COLS - 1 && level.get_tile(_right_col + 1, _ty) == tile::VANISH)
        {
            _right_col++;
        }
    }

    bool on_top = player_on_top(player);

    switch(_state)
    {
        case State::SOLID:
            if(on_top)
            {
                _state = State::SHAKING;
                _timer = 0;
                SfxPlayer::play(SfxCue::FLOOR_CRACK);
            }
            break;
        case State::SHAKING:
            _timer++;
            if(_timer >= _pa)
            {
                _state = State::GONE;
                _timer = 0;
                SfxPlayer::play(SfxCue::FLOOR_BREAK);
            }
            break;
        case State::GONE:
            if(_pb > 0)
            {
                _timer++;
                if(_timer >= _pb)
                {
                    _state = State::SOLID;
                    _timer = 0;
                }
            }
            break;
        case State::RESPAWNING:
            _state = State::SOLID;
            _timer = 0;
            break;
        default:
            break;
    }

    _player_was_on_top = on_top;
    
    uint16_t target_tile = is_solid() ? tile::VANISH : tile::EMPTY;
    for(int c = _left_col; c <= _right_col; ++c)
    {
        level.set_tile(c, _ty, target_tile);
    }
}

bool TrapVanishFloor::player_on_top(const Player& player) const
{
    int left_c = (_left_col == -1) ? _tx : _left_col;
    int right_c = (_right_col == -1) ? _tx : _right_col;
    int tile_left = SCREEN_LEFT.integer() + left_c * TILE_SIZE;
    int tile_right = SCREEN_LEFT.integer() + (right_c + 1) * TILE_SIZE;
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

} // namespace game
