#ifndef LEVEL_H
#define LEVEL_H

#include "bn_size.h"
#include "bn_optional.h"
#include "bn_camera_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_map_ptr.h"
#include "bn_regular_bg_map_item.h"
#include "bn_regular_bg_map_cell.h"
#include "constants.h"
#include "level_data.h"

namespace game {

class Player;

class Level {
public:
    Level();

    // Loads LevelDef layout, builds the background map cell buffer and creates the GBA regular background
    void load(const ChamberDef& def, uint8_t world_id);

    // Updates a specific tile in the tilemap and updates GBA VRAM
    void set_tile(int col, int row, uint16_t tile_id);

    // Retrieve a tile from the current level
    uint16_t get_tile(int col, int row) const;

    // Resolve an authored spawn marker to a collision-free supported Y position.
    bn::fixed settled_spawn_y(int tile_x, int tile_y, bool gravity_flipped,
                              int half_width, int half_height) const;

    // Checks player bounding box against tiles, resolves solid overlaps, sets ground state, handles death spikes
    void resolve_collisions(Player& player, bool gravity_flipped);

    // Check if the player is overlapping the exit door tile
    bool check_exit_door(const Player& player) const;
    const ChamberTransition* check_transition(const Player& player) const;

    // Attach GBA camera pointer to the background
    void set_camera(const bn::camera_ptr& camera);

    const bn::optional<bn::regular_bg_ptr>& bg() const { return _bg; }
    bn::optional<bn::regular_bg_ptr>& bg() { return _bg; }

    // Coordinate helpers (optimized with shifts)
    inline int get_tile_col(bn::fixed px) const { return (px.integer() + 120) >> 3; }
    inline int get_tile_row(bn::fixed py) const { return (py.integer() + 64) >> 3; }
    inline bn::fixed get_tile_x(int col) const  { return (col << 3) - 116; }
    inline bn::fixed get_tile_y(int row) const  { return (row << 3) - 60; }
    bool blocks_projectile(bn::fixed px, bn::fixed py) const {
        return is_tile_solid(
            get_tile(get_tile_col(px), get_tile_row(py)));
    }

private:
    uint16_t _tilemap[LEVEL_ROWS][LEVEL_COLS];

    // GBA cell buffer for VRAM updates (32x32 = 1024 cells)
    alignas(int) bn::regular_bg_map_cell _cells[32 * 32];
    bn::regular_bg_map_item _map_item;

    bn::optional<bn::regular_bg_ptr> _bg;
    bn::optional<bn::regular_bg_map_ptr> _bg_map;
    bn::optional<bn::sprite_ptr> _exit_sprite;

    int _exit_x;
    int _exit_y;
    bool _has_exit;
    const ChamberTransition* _transitions;
    uint8_t _transition_count;

    bool is_valid_tile(int col, int row) const {
        return col >= 0 && col < LEVEL_COLS && row >= 0 && row < LEVEL_ROWS;
    }

    bool is_tile_solid(uint16_t tile_id) const {
        return tile_id == tile::SOLID ||
               tile_id == tile::VANISH ||
               tile_id == tile::CONVEYOR_LEFT ||
               tile_id == tile::CONVEYOR_RIGHT ||
               tile_id == tile::INVISIBLE_WALL;
    }
};

} // namespace game

#endif // LEVEL_H
