#ifndef TEXT_REVEAL_H
#define TEXT_REVEAL_H

namespace game {

class TextReveal {
public:
    void reset(int character_count, int frames_per_character)
    {
        _visible = frames_per_character <= 0 ? character_count : 0;
        _character_count = character_count;
        _frames_per_character = frames_per_character;
        _timer = 0;
    }

    bool update()
    {
        if(_visible >= _character_count || _frames_per_character <= 0)
        {
            return false;
        }
        if(++_timer >= _frames_per_character)
        {
            _timer = 0;
            ++_visible;
            return true;
        }
        return false;
    }

    int visible() const { return _visible; }

private:
    int _visible = 0;
    int _character_count = 0;
    int _frames_per_character = 1;
    int _timer = 0;
};

} // namespace game

#endif
