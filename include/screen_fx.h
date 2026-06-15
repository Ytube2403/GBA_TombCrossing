#ifndef SCREEN_FX_H
#define SCREEN_FX_H

#include "bn_fixed.h"

namespace game {

class ScreenFx {
public:
    ScreenFx();

    // Blocking fade out (clear to black)
    void fade_out(int frames = 16);

    // Blocking fade in (black to clear/target_alpha)
    void fade_in(int frames = 16, bn::fixed target_alpha = 0);

    // Non-blocking white flash (decays dynamically in game loop)
    void flash_white(int frames = 10);
    void set_flash_mode(int mode) { _flash_mode = mode; }

    // Non-blocking update method, must be called once per frame in gameplay loop
    void update();

private:
    int  _flash_timer;
    int  _flash_duration;
    bool _flashing;
    int  _flash_mode;
    bn::fixed _flash_alpha;
};

} // namespace game

#endif // SCREEN_FX_H
