#include "screen_fx.h"
#include "bn_blending.h"
#include "bn_core.h"
#include "bn_algorithm.h"

namespace game {

ScreenFx::ScreenFx() :
    _flash_timer(0),
    _flash_duration(0),
    _flashing(false),
    _flash_mode(0),
    _flash_alpha(1)
{
}

void ScreenFx::fade_out(int frames)
{
    _flashing = false; // Override any active flashing
    bn::blending::set_black_fade_color();
    
    if(frames <= 0)
    {
        bn::blending::set_fade_alpha(bn::fixed(1));
        bn::core::update();
        return;
    }
    
    bn::fixed start_alpha = bn::blending::fade_alpha();
    for(int i = 0; i <= frames; ++i)
    {
        bn::fixed t = bn::fixed(i) / frames;
        bn::fixed alpha = start_alpha + (bn::fixed(1) - start_alpha) * t;
        bn::blending::set_fade_alpha(alpha);
        bn::core::update();
    }
}

void ScreenFx::fade_in(int frames, bn::fixed target_alpha)
{
    _flashing = false;
    bn::blending::set_black_fade_color();
    
    if(frames <= 0)
    {
        bn::blending::set_fade_alpha(target_alpha);
        if(target_alpha == 0)
        {
            bn::blending::restore();
        }
        bn::core::update();
        return;
    }
    
    for(int i = frames; i >= 0; --i)
    {
        bn::fixed t = bn::fixed(i) / frames;
        bn::fixed alpha = target_alpha + (bn::fixed(1) - target_alpha) * t;
        bn::blending::set_fade_alpha(alpha);
        bn::core::update();
    }
    if(target_alpha == 0)
    {
        bn::blending::restore();
    }
}

void ScreenFx::flash_white(int frames)
{
    if(frames <= 0 || _flash_mode >= 2) return;
    if(_flash_mode == 1)
    {
        frames = bn::max(2, frames / 2);
    }
    
    bn::blending::set_white_fade_color();
    _flash_alpha = _flash_mode == 1 ? bn::fixed(0.5) : bn::fixed(1);
    bn::blending::set_fade_alpha(_flash_alpha);
    _flash_timer = frames;
    _flash_duration = frames;
    _flashing = true;
}

void ScreenFx::update()
{
    if(_flashing)
    {
        --_flash_timer;
        if(_flash_timer <= 0)
        {
            _flashing = false;
            bn::blending::restore();
        }
        else
        {
            bn::blending::set_white_fade_color();
            bn::blending::set_fade_alpha(
                _flash_alpha * bn::fixed(_flash_timer) / _flash_duration);
        }
    }
}

} // namespace game
