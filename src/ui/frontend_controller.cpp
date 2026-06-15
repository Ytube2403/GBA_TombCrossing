#include "frontend_controller.h"
#include "sfx_player.h"
#include "bn_keypad.h"
#include "bn_algorithm.h"

namespace game {

void FrontendController::reset(int item_count, int cursor)
{
    _item_count = bn::max(0, item_count);
    set_cursor(cursor);
}

void FrontendController::set_cursor(int cursor)
{
    _cursor = _item_count > 0 ? bn::max(0, bn::min(cursor, _item_count - 1)) : 0;
}

FrontendAction FrontendController::update(bool horizontal, bool pages)
{
    if(pages && bn::keypad::l_pressed())
    {
        SfxPlayer::play(SfxCue::UI_MOVE);
        return { FrontendActionType::PAGE_LEFT, _cursor };
    }
    if(pages && bn::keypad::r_pressed())
    {
        SfxPlayer::play(SfxCue::UI_MOVE);
        return { FrontendActionType::PAGE_RIGHT, _cursor };
    }

    bool previous = horizontal ?
        (bn::keypad::left_pressed() || bn::keypad::up_pressed()) :
        bn::keypad::up_pressed();
    bool next = horizontal ?
        (bn::keypad::right_pressed() || bn::keypad::down_pressed()) :
        bn::keypad::down_pressed();

    int old_cursor = _cursor;
    if(previous && _cursor > 0)
    {
        --_cursor;
    }
    else if(next && _cursor + 1 < _item_count)
    {
        ++_cursor;
    }
    if(_cursor != old_cursor)
    {
        SfxPlayer::play(SfxCue::UI_MOVE);
        return { FrontendActionType::MOVED, _cursor };
    }
    if(bn::keypad::a_pressed())
    {
        SfxPlayer::play(SfxCue::UI_CONFIRM);
        return { FrontendActionType::CONFIRM, _cursor };
    }
    if(bn::keypad::b_pressed())
    {
        SfxPlayer::play(SfxCue::UI_BACK);
        return { FrontendActionType::BACK, _cursor };
    }
    if(bn::keypad::start_pressed())
    {
        SfxPlayer::play(SfxCue::UI_CONFIRM);
        return { FrontendActionType::START, _cursor };
    }
    return {};
}

} // namespace game
