#include "text_renderer.h"
#include "bn_regular_bg_map_cell_info.h"

namespace game {

void TextRenderer::draw_text(bn::regular_bg_map_cell* cells, int x, int y, const bn::string_view& text)
{
    for(int i = 0; i < text.length(); ++i)
    {
        int col = x + i;
        if(col < 0 || col >= UI_COLS || y < 0 || y >= UI_ROWS) continue;
        
        char c = text[i];
        int tile_idx = 0; // default space
        
        if(c == '\x01')      tile_idx = 96; // Skull
        else if(c == '\x02') tile_idx = 97; // Up Arrow
        else if(c == '\x03') tile_idx = 98; // Down Arrow
        else if(c == '\x04') tile_idx = 99; // Lock
        else if(c == '\x05') tile_idx = 100; // Clear check
        else if(c == '\x06') tile_idx = 101; // Menu cursor
        else if(c == '\x07') tile_idx = 102; // Profile
        else if(c == '\x0E') tile_idx = 103; // Saving
        else if(c == '\x10') tile_idx = 104; // Panel top-left
        else if(c == '\x11') tile_idx = 105; // Panel top-right
        else if(c == '\x12') tile_idx = 106; // Panel bottom-left
        else if(c == '\x13') tile_idx = 107; // Panel bottom-right
        else if(c == '\x14') tile_idx = 108; // Panel horizontal
        else if(c == '\x15') tile_idx = 109; // Panel vertical
        else if(c >= 32 && c <= 126) tile_idx = c - 32;
        
        int cell_idx = y * 32 + col;
        bn::regular_bg_map_cell_info cell_info(cells[cell_idx]);
        cell_info.set_tile_index(tile_idx);
        cells[cell_idx] = cell_info.cell();
    }
}

void TextRenderer::draw_clipped(bn::regular_bg_map_cell* cells, int x, int y,
                                const bn::string_view& text, int max_width)
{
    int count = bn::min(max_width, int(text.length()));
    for(int i = 0; i < count; ++i)
    {
        char buffer[2] = { text[i], '\0' };
        draw_text(cells, x + i, y, bn::string_view(buffer, 1));
    }
}

int TextRenderer::centered_x(int text_length)
{
    return bn::max(0, (UI_COLS - text_length) / 2);
}

void TextRenderer::draw_centered(bn::regular_bg_map_cell* cells, int y, const bn::string_view& text)
{
    draw_clipped(cells, centered_x(int(text.length())), y, text, UI_COLS);
}

void TextRenderer::draw_panel(bn::regular_bg_map_cell* cells, int x, int y, int width, int height)
{
    if(width < 2 || height < 2)
    {
        return;
    }

    draw_text(cells, x, y, "\x10");
    draw_text(cells, x + width - 1, y, "\x11");
    draw_text(cells, x, y + height - 1, "\x12");
    draw_text(cells, x + width - 1, y + height - 1, "\x13");
    for(int col = 1; col < width - 1; ++col)
    {
        draw_text(cells, x + col, y, "\x14");
        draw_text(cells, x + col, y + height - 1, "\x14");
    }
    for(int row = 1; row < height - 1; ++row)
    {
        draw_text(cells, x, y + row, "\x15");
        draw_text(cells, x + width - 1, y + row, "\x15");
    }
}

void TextRenderer::clear_text(bn::regular_bg_map_cell* cells, int x, int y, int len)
{
    for(int i = 0; i < len; ++i)
    {
        int col = x + i;
        if(col < 0 || col >= UI_COLS || y < 0 || y >= UI_ROWS) continue;
        
        int cell_idx = y * 32 + col;
        bn::regular_bg_map_cell_info cell_info(cells[cell_idx]);
        cell_info.set_tile_index(0); // Space
        cells[cell_idx] = cell_info.cell();
    }
}

void TextRenderer::clear_visible(bn::regular_bg_map_cell* cells)
{
    for(int row = 0; row < UI_ROWS; ++row)
    {
        clear_text(cells, 0, row, UI_COLS);
    }
}

} // namespace game
