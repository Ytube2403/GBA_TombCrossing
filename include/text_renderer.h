#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "bn_string_view.h"
#include "bn_regular_bg_map_cell.h"
#include "constants.h"

namespace game {

class TextRenderer {
public:
    static void draw_text(bn::regular_bg_map_cell* cells, int x, int y, const bn::string_view& text);
    static void draw_clipped(bn::regular_bg_map_cell* cells, int x, int y,
                             const bn::string_view& text, int max_width);
    static void draw_centered(bn::regular_bg_map_cell* cells, int y, const bn::string_view& text);
    static void draw_panel(bn::regular_bg_map_cell* cells, int x, int y, int width, int height);
    static void clear_text(bn::regular_bg_map_cell* cells, int x, int y, int len);
    static void clear_visible(bn::regular_bg_map_cell* cells);
    static int centered_x(int text_length);
};

} // namespace game

#endif // TEXT_RENDERER_H
