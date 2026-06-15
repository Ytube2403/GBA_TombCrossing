#ifndef CAMERA_H
#define CAMERA_H

#include "bn_camera_ptr.h"
#include "bn_fixed.h"
#include "bn_random.h"

namespace game {

class Player;

class Camera {
public:
    Camera();

    // Updates camera position (shake offsets, follow center logic)
    void update(const Player& player);

    // Triggers a screen-shake effect
    void shake(int duration = 15, bn::fixed intensity = 2.0);

    // Get the underlying Butano camera pointer to attach to BGs or sprites
    const bn::camera_ptr& camera_ptr() const { return _camera; }

private:
    bn::camera_ptr _camera;
    bn::random     _random;

    bn::fixed      _base_x;
    bn::fixed      _base_y;

    int            _shake_timer;
    bn::fixed      _shake_intensity;
};

} // namespace game

#endif // CAMERA_H
