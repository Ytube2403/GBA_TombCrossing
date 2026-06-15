#include "hud.h"
#include "text_renderer.h"
#include "bn_regular_bg_item.h"
#include "bn_regular_bg_tiles_items_hud_tiles.h"
#include "bn_memory.h"
#include "bn_string.h"
#include "skill.h"

namespace game {

HUD::HUD() :
    _map_item(_cells[0], bn::size(32, 32))
{
    bn::memory::clear(_cells);
    
    bn::regular_bg_item hud_bg_item(
        bn::regular_bg_tiles_items::hud_tiles,
        bn::regular_bg_tiles_items::hud_tiles_palette,
        _map_item
    );
    
    // Align map rows 0..19 and columns 0..29 with the visible 240x160 area.
    _bg = hud_bg_item.create_bg(8, 48);
    _bg->set_priority(0);
    _bg_map = _bg->map();
}

void HUD::update(int deaths, int level_num, const char* world_name, bool gravity_flipped,
                 const SkillState& skill)
{
    (void) world_name;
    if(!_bg_map.has_value()) return;
    
    ++_frame_counter;
    
    TextRenderer::clear_text(_cells, 0, 0, UI_COLS);
    TextRenderer::clear_text(_cells, 0, 1, UI_COLS);
    
    bn::string<16> death_str = "\x01";
    bn::string<8> num_str = bn::to_string<8>(deaths);
    for(int i = 0; i < 4 - num_str.length(); ++i)
    {
        death_str.append('0');
    }
    death_str.append(num_str.data());
    TextRenderer::draw_text(_cells, 19, 0, death_str.data());
    
    bn::string<16> room_str = "ROOM ";
    room_str.append(bn::to_string<8>(level_num + 1).data());
    TextRenderer::draw_text(_cells, 1, 0, room_str.data());
    
    bn::string<4> grav_str = gravity_flipped ? "[\x03]" : "[\x02]";
    TextRenderer::draw_text(_cells, 27, 0, grav_str.data());
    
    if(skill.selected != SkillType::NONE)
    {
        // Skill name
        int skill_idx = static_cast<int>(skill.selected);
        TextRenderer::draw_text(_cells, 1, 1, SKILL_INFO[skill_idx].name);

        if(!skill.used)
        {
            if((_frame_counter / 20) % 2 == 0)
            {
                TextRenderer::draw_text(_cells, 8, 1, "[B:READY]");
            }
        }
        else if(skill.active && skill.timer > 0)
        {
            int secs = (skill.timer + 59) / 60;
            bn::string<12> timer_str = "[";
            timer_str.append(bn::to_string<4>(secs).data());
            timer_str.append("SEC]  ");
            TextRenderer::draw_text(_cells, 8, 1, timer_str.data());
        }
        else if(skill.used)
        {
            TextRenderer::draw_text(_cells, 8, 1, "[USED]  ");
        }
    }

    _bg_map->reload_cells_ref();
}

void HUD::set_visible(bool visible)
{
    if(_bg.has_value())
    {
        _bg->set_visible(visible);
    }
}

} // namespace game
