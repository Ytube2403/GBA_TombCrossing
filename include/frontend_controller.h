#ifndef FRONTEND_CONTROLLER_H
#define FRONTEND_CONTROLLER_H

#include <cstdint>

namespace game {

enum class FrontendActionType : uint8_t {
    NONE,
    MOVED,
    CONFIRM,
    BACK,
    START,
    PAGE_LEFT,
    PAGE_RIGHT
};

struct FrontendAction {
    FrontendActionType type = FrontendActionType::NONE;
    int value = 0;
};

class FrontendController {
public:
    void reset(int item_count, int cursor = 0);
    FrontendAction update(bool horizontal = false, bool pages = false);

    int cursor() const { return _cursor; }
    int item_count() const { return _item_count; }
    void set_cursor(int cursor);

private:
    int _cursor = 0;
    int _item_count = 0;
};

} // namespace game

#endif
