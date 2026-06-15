#include "level.h"
#include "sfx_player.h"
#include "player.h"
#include "bn_memory.h"
#include "bn_regular_bg_item.h"
#include "bn_regular_bg_map_cell_info.h"
#include "bn_regular_bg_tiles_items_world1_tiles.h"
#include "bn_regular_bg_tiles_items_world2_tiles.h"
#include "bn_regular_bg_tiles_items_world3_tiles.h"
#include "bn_regular_bg_tiles_items_world4_tiles.h"
#include "bn_regular_bg_tiles_items_world5_tiles.h"
#include "bn_sprite_items_exit_door.h"

namespace game {

Level::Level() :
    _map_item(_cells[0], bn::size(32, 32)),
    _exit_x(0),
    _exit_y(0),
    _has_exit(false),
    _transitions(nullptr),
    _transition_count(0)
{
    // Initialize cell buffer to empty tiles
    bn::memory::clear(_cells);
}

void Level::load(const ChamberDef& def, uint8_t world_id)
{
    // Reset background map references and old sprite first to prevent VRAM block leaks
    _bg_map.reset();
    _bg.reset();
    _exit_sprite.reset();

    _exit_x = def.exit_x;
    _exit_y = def.exit_y;
    _has_exit = def.has_exit;
    _transitions = def.transitions;
    _transition_count = def.transition_count;

    // Copy tilemap layout
    for(int r = 0; r < LEVEL_ROWS; ++r)
    {
        for(int c = 0; c < LEVEL_COLS; ++c)
        {
            _tilemap[r][c] = def.tilemap[r][c];
        }
    }

    // Build regular GBA background map buffer
    bn::memory::clear(_cells);
    
    // Fill HUD rows (0 and 1) with EMPTY tiles
    for(int c = 0; c < 32; ++c)
    {
        bn::regular_bg_map_cell_info cell_info(_cells[c]);
        cell_info.set_tile_index(tile::EMPTY);
        _cells[c] = cell_info.cell();
        
        bn::regular_bg_map_cell_info cell_info_2(_cells[32 + c]);
        cell_info_2.set_tile_index(tile::EMPTY);
        _cells[32 + c] = cell_info_2.cell();
    }

    // Fill gameplay rows (2 to 19)
    for(int r = 0; r < LEVEL_ROWS; ++r)
    {
        for(int c = 0; c < LEVEL_COLS; ++c)
        {
            int cell_idx = (r + 2) * 32 + c;
            bn::regular_bg_map_cell_info cell_info(_cells[cell_idx]);
            
            // SPAWN and EXIT are not directly drawn as tiles in the background
            uint16_t tile_id = _tilemap[r][c];
            if(tile_id == tile::SPAWN || tile_id == tile::EXIT) 
            {
                tile_id = tile::EMPTY;
            }
            
            cell_info.set_tile_index(tile_id);
            _cells[cell_idx] = cell_info.cell();
        }
    }

    // Create the background at aligned screen-space offset using the tileset
    bn::regular_bg_item custom_bg_item(
        bn::regular_bg_tiles_items::world1_tiles,
        bn::regular_bg_tiles_items::world1_tiles_palette,
        _map_item
    );
    if(world_id == 1)
    {
        custom_bg_item = bn::regular_bg_item(
            bn::regular_bg_tiles_items::world2_tiles,
            bn::regular_bg_tiles_items::world2_tiles_palette,
            _map_item
        );
    }
    else if(world_id == 2)
    {
        custom_bg_item = bn::regular_bg_item(
            bn::regular_bg_tiles_items::world3_tiles,
            bn::regular_bg_tiles_items::world3_tiles_palette,
            _map_item
        );
    }
    else if(world_id == 3)
    {
        custom_bg_item = bn::regular_bg_item(
            bn::regular_bg_tiles_items::world4_tiles,
            bn::regular_bg_tiles_items::world4_tiles_palette,
            _map_item
        );
    }
    else if(world_id == 4)
    {
        custom_bg_item = bn::regular_bg_item(
            bn::regular_bg_tiles_items::world5_tiles,
            bn::regular_bg_tiles_items::world5_tiles_palette,
            _map_item
        );
    }
    _bg = custom_bg_item.create_bg(8, 48);
    _bg->set_priority(2);
    _bg_map = _bg->map();

    if(_has_exit)
    {
        bn::fixed door_x = get_tile_x(_exit_x) - 4;
        bn::fixed door_y = get_tile_y(_exit_y) - 12;
        _exit_sprite = bn::sprite_items::exit_door.create_sprite(door_x, door_y);
    }
}

void Level::set_tile(int col, int row, uint16_t tile_id)
{
    if(!is_valid_tile(col, row)) return;
    
    _tilemap[row][col] = tile_id;

    if(_bg_map.has_value())
    {
        int cell_idx = (row + 2) * 32 + col;
        bn::regular_bg_map_cell_info cell_info(_cells[cell_idx]);
        cell_info.set_tile_index(tile_id == tile::SPAWN ? tile::EMPTY : tile_id);
        _cells[cell_idx] = cell_info.cell();
        _bg_map->reload_cells_ref();
    }
}

uint16_t Level::get_tile(int col, int row) const
{
    if(!is_valid_tile(col, row)) return tile::SOLID; // Clamp boundary as solid
    return _tilemap[row][col];
}

bn::fixed Level::settled_spawn_y(int tile_x, int tile_y, bool gravity_flipped,
                                 int half_width, int half_height) const
{
    int center_x = SCREEN_LEFT.integer() + tile_x * TILE_SIZE + TILE_SIZE / 2;
    int left_col = get_tile_col(bn::fixed(center_x - half_width + 1));
    int right_col = get_tile_col(bn::fixed(center_x + half_width - 1));

    if(gravity_flipped)
    {
        for(int row = tile_y; row >= 0; --row)
        {
            for(int col = left_col; col <= right_col; ++col)
            {
                if(is_tile_solid(get_tile(col, row)))
                {
                    bn::fixed support_bottom = get_tile_y(row) + 4;
                    return support_bottom + half_height + bn::fixed(0.01);
                }
            }
        }
    }
    else
    {
        for(int row = tile_y; row < LEVEL_ROWS; ++row)
        {
            for(int col = left_col; col <= right_col; ++col)
            {
                if(is_tile_solid(get_tile(col, row)))
                {
                    bn::fixed support_top = get_tile_y(row) - 4;
                    return support_top - half_height - bn::fixed(0.01);
                }
            }
        }
    }

    return GAMEPLAY_TOP + tile_y * TILE_SIZE + TILE_SIZE / 2;
}

void Level::set_camera(const bn::camera_ptr& camera)
{
    if(_bg.has_value())
    {
        _bg->set_camera(camera);
    }
    if(_exit_sprite.has_value())
    {
        _exit_sprite->set_camera(camera);
    }
}

bool Level::check_exit_door(const Player& player) const
{
    if(!_has_exit)
    {
        return false;
    }

    int door_left   = SCREEN_LEFT.integer() + _exit_x * TILE_SIZE;
    int door_right  = door_left + TILE_SIZE;
    int door_top    = GAMEPLAY_TOP.integer() + _exit_y * TILE_SIZE;
    int door_bottom = door_top + TILE_SIZE;

    return (player.left() < door_right && player.right() > door_left &&
            player.top() < door_bottom && player.bottom() > door_top);
}

const ChamberTransition* Level::check_transition(const Player& player) const
{
    for(int index = 0; index < _transition_count; ++index)
    {
        const ChamberTransition& transition = _transitions[index];
        int left = SCREEN_LEFT.integer() + transition.tile_x * TILE_SIZE;
        int right = left + transition.width * TILE_SIZE;
        int top = GAMEPLAY_TOP.integer() + transition.tile_y * TILE_SIZE;
        int bottom = top + transition.height * TILE_SIZE;
        if(player.left() < right && player.right() > left &&
           player.top() < bottom && player.bottom() > top)
        {
            return &transition;
        }
    }
    return nullptr;
}

void Level::resolve_collisions(Player& player, bool gravity_flipped)
{
    player.clear_wall_contact();

    // --- Phase 1: Horizontal Collision (X Axis) ---
    int p_left   = player.left();
    int p_right  = player.right();
    int p_top    = player.top();
    int p_bottom = player.bottom();

    // Shrink vertical scan range by 1px to avoid catching floor/ceiling tiles
    // when player is flush against them (prevents horizontal vs vertical
    // collision ambiguity at corners). One pixel still catches a platform
    // side when the player is too tall for the gap below it.
    int row_start = get_tile_row(bn::fixed(p_top + 1));
    int row_end   = get_tile_row(bn::fixed(p_bottom - 1));

    if(player.vx() > 0) // Moving right
    {
        int col = get_tile_col(bn::fixed(p_right));
        bool collision = false;
        
        for(int r = row_start; r <= row_end; ++r)
        {
            uint16_t t = get_tile(col, r);
            if(t == tile::SPIKE_UP || t == tile::SPIKE_DOWN) 
            {
                player.kill(DeathCause::TILE_HAZARD);
                return;
            }
            if(is_tile_solid(t))
            {
                collision = true;
            }
        }
        
        if(collision)
        {
            bn::fixed tile_left = get_tile_x(col) - 4;
            // Push the player edge to just before the solid tile.
            player.push_out_x(tile_left - player.half_width() -
                              player.x() - bn::fixed(0.01));
            player.stop_horizontal();
            player.set_wall_contact(1);
        }
    }
    else if(player.vx() < 0) // Moving left
    {
        int col = get_tile_col(bn::fixed(p_left));
        bool collision = false;
        
        for(int r = row_start; r <= row_end; ++r)
        {
            uint16_t t = get_tile(col, r);
            if(t == tile::SPIKE_UP || t == tile::SPIKE_DOWN) 
            {
                player.kill(DeathCause::TILE_HAZARD);
                return;
            }
            if(is_tile_solid(t))
            {
                collision = true;
            }
        }
        
        if(collision)
        {
            bn::fixed tile_right = get_tile_x(col) + 4;
            player.push_out_x(tile_right + player.half_width() -
                              player.x() + bn::fixed(0.01));
            player.stop_horizontal();
            player.set_wall_contact(-1);
        }
    }

    // --- Phase 2: Vertical Collision (Y Axis) ---
    // Recalculate positions after horizontal push
    p_left   = player.left();
    p_right  = player.right();
    p_top    = player.top();
    p_bottom = player.bottom();

    // Sample inside the horizontal edges to avoid treating a flush wall as a
    // floor or ceiling. Vertical collision only checks the leading edge of
    // the hitbox; scanning the whole body can snap the player onto a platform
    // after merely touching its side.
    int col_start = get_tile_col(bn::fixed(p_left + 1));
    int col_end   = get_tile_col(bn::fixed(p_right - 1));

    if(!gravity_flipped)
    {
        if(player.vy() >= 0) // Falling/Landing
        {
            int landing_row = get_tile_row(bn::fixed(p_bottom));
            bool collision = false;

            for(int c = col_start; c <= col_end; ++c)
            {
                uint16_t t = get_tile(c, landing_row);
                if(t == tile::SPIKE_UP || t == tile::SPIKE_DOWN)
                {
                    player.kill(DeathCause::TILE_HAZARD);
                    return;
                }
                if(is_tile_solid(t))
                {
                    collision = true;
                    break;
                }
            }

            if(collision)
            {
                bn::fixed tile_top = get_tile_y(landing_row) - 4;
                // Place the player just above the tile to prevent re-penetration.
                player.push_out_y(tile_top - player.half_height() -
                                  player.y() - bn::fixed(0.01));
                player.stop_vertical();
                if(!player.was_on_ground())
                {
                    SfxPlayer::play(SfxCue::PLAYER_LAND);
                }
                player.set_on_ground(true);

                // Conveyors apply force
                bn::fixed conveyor_force = 0;
                for(int c = col_start; c <= col_end; ++c)
                {
                    uint16_t t = get_tile(c, landing_row);
                    if(t == tile::CONVEYOR_LEFT)
                    {
                        conveyor_force = -bn::fixed(0.5);
                    }
                    else if(t == tile::CONVEYOR_RIGHT)
                    {
                        conveyor_force = bn::fixed(0.5);
                    }
                }
                if(conveyor_force != 0)
                {
                    player.apply_conveyor(conveyor_force);
                }
            }
        }
        else if(player.vy() < 0) // Jumping / Ceiling bump
        {
            int ceiling_row = get_tile_row(bn::fixed(p_top));
            bool collision = false;

            for(int c = col_start; c <= col_end; ++c)
            {
                uint16_t t = get_tile(c, ceiling_row);
                if(t == tile::SPIKE_UP || t == tile::SPIKE_DOWN)
                {
                    player.kill(DeathCause::TILE_HAZARD);
                    return;
                }
                if(is_tile_solid(t))
                {
                    collision = true;
                    break;
                }
            }

            if(collision)
            {
                bn::fixed tile_bottom = get_tile_y(ceiling_row) + 4;
                player.push_out_y(tile_bottom + player.half_height() -
                                  player.y() + bn::fixed(0.01));
                player.stop_vertical();
            }
        }
    }
    else // Gravity is flipped! Landing is on the ceiling (top)
    {
        if(player.vy() <= 0) // Falling upwards towards ceiling
        {
            int landing_row = get_tile_row(bn::fixed(p_top));
            bool collision = false;

            for(int c = col_start; c <= col_end; ++c)
            {
                uint16_t t = get_tile(c, landing_row);
                if(t == tile::SPIKE_UP || t == tile::SPIKE_DOWN)
                {
                    player.kill(DeathCause::TILE_HAZARD);
                    return;
                }
                if(is_tile_solid(t))
                {
                    collision = true;
                    break;
                }
            }

            if(collision)
            {
                bn::fixed tile_bottom = get_tile_y(landing_row) + 4;
                player.push_out_y(tile_bottom + player.half_height() -
                                  player.y() + bn::fixed(0.01));
                player.stop_vertical();
                if(!player.was_on_ground())
                {
                    SfxPlayer::play(SfxCue::PLAYER_LAND);
                }
                player.set_on_ground(true);

                bn::fixed conveyor_force = 0;
                for(int c = col_start; c <= col_end; ++c)
                {
                    uint16_t t = get_tile(c, landing_row);
                    if(t == tile::CONVEYOR_LEFT)
                    {
                        conveyor_force = -bn::fixed(0.5);
                    }
                    else if(t == tile::CONVEYOR_RIGHT)
                    {
                        conveyor_force = bn::fixed(0.5);
                    }
                }
                if(conveyor_force != 0)
                {
                    player.apply_conveyor(conveyor_force);
                }
            }
        }
        else if(player.vy() > 0) // Jumping downwards (hitting floor)
        {
            int floor_row = get_tile_row(bn::fixed(p_bottom));
            bool collision = false;

            for(int c = col_start; c <= col_end; ++c)
            {
                uint16_t t = get_tile(c, floor_row);
                if(t == tile::SPIKE_UP || t == tile::SPIKE_DOWN)
                {
                    player.kill(DeathCause::TILE_HAZARD);
                    return;
                }
                if(is_tile_solid(t))
                {
                    collision = true;
                    break;
                }
            }

            if(collision)
            {
                bn::fixed tile_top = get_tile_y(floor_row) - 4;
                player.push_out_y(tile_top - player.half_height() -
                                  player.y() - bn::fixed(0.01));
                player.stop_vertical();
            }
        }
    }
}

} // namespace game
