#ifndef HUD_H
#define HUD_H

#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_map_ptr.h"
#include "bn_regular_bg_map_item.h"
#include "bn_regular_bg_map_cell.h"
#include "bn_optional.h"
#include "skill.h"

namespace game {

class HUD {
public:
    HUD();
    
    void update(int deaths, int level_num, const char* world_name, bool gravity_flipped,
                const SkillState& skill);
    void set_visible(bool visible);

private:
    alignas(int) bn::regular_bg_map_cell _cells[32 * 32];
    bn::regular_bg_map_item _map_item;
    bn::optional<bn::regular_bg_ptr> _bg;
    bn::optional<bn::regular_bg_map_ptr> _bg_map;
    int _frame_counter = 0;
};

} // namespace game

#endif // HUD_H
