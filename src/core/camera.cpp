#include "camera.h"
#include "player.h"

namespace game {

Camera::Camera() :
    _camera(bn::camera_ptr::create(0, 0)),
    _base_x(0),
    _base_y(0),
    _shake_timer(0),
    _shake_intensity(0)
{
}

void Camera::update(const Player& player)
{
    (void)player; // follow player is not needed because the play area is screen sized and static

    // The play area matches screen coordinates exactly (240x160 centered at 0,0)
    // Thus the base camera remains centered at (0, 0)
    _base_x = 0;
    _base_y = 0;

    if(_shake_timer > 0)
    {
        --_shake_timer;
        // Generate random offsets in the range [-intensity, intensity]
        bn::fixed dx = _random.get_fixed(-_shake_intensity, _shake_intensity);
        bn::fixed dy = _random.get_fixed(-_shake_intensity, _shake_intensity);
        _camera.set_position(_base_x + dx, _base_y + dy);
    }
    else
    {
        _camera.set_position(_base_x, _base_y);
    }
}

void Camera::shake(int duration, bn::fixed intensity)
{
    _shake_timer = bn::max(_shake_timer, duration);
    _shake_intensity = bn::max(_shake_intensity, intensity);
}

} // namespace game
