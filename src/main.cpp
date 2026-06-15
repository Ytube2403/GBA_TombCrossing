#include "bn_core.h"
#include "game_manager.h"

int main()
{
    bn::core::init();

    game::GameManager manager;
    manager.run();
}
