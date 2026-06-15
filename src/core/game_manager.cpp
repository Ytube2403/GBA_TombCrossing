#include "game_manager.h"
#include "level_data.h"
#include "sfx_player.h"
#include "text_renderer.h"

#include "bn_algorithm.h"
#include "bn_math.h"
#include "bn_bg_palette_ptr.h"
#include "bn_blending.h"
#include "bn_color.h"
#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_memory.h"
#include "bn_music.h"
#include "bn_music_items.h"
#include "bn_optional.h"
#include "bn_regular_bg_item.h"
#include "bn_regular_bg_tiles_items_hud_tiles.h"
#include "bn_sound.h"
#include "bn_sprite_items_player_kha_men.h"
#include "bn_sprite_items_world_cards.h"
#include "bn_sprite_items_frontend_art.h"
#include "bn_sprite_tiles_item.h"
#include "bn_sprite_tiles_ptr.h"
#include "bn_string.h"
#include "bn_string_view.h"

#include "bn_regular_bg_items_world1_bg.h"
#include "bn_regular_bg_items_world2_bg.h"
#include "bn_regular_bg_items_world3_bg.h"
#include "bn_regular_bg_items_world4_bg.h"
#include "bn_regular_bg_items_world5_bg.h"
#include "bn_sprite_items_title_logo.h"
extern "C" {
    game::GameManager* global_game_manager = nullptr;
}

namespace game {

namespace {
    constexpr const char* WORLD_TITLES[] = {
        "THE SANDS OF RA",
        "THE ROYAL CRYPTS",
        "TERRACOTTA LABYRINTH",
        "TEMPLE OF XIBALBA",
        "THE WARRIOR'S REST"
    };

    constexpr const char* WORLD_SHORT[] = {
        "EGYPT", "ROYAL", "QIN", "AZTEC", "KOFUN"
    };

    constexpr const char* WORLD_FLAVOR[] = {
        "ESCAPE THE MUMMY'S CURSE",
        "DODGE THE ROYAL AXES",
        "NAVIGATE STATUE MAZES",
        "SURVIVE RISING LAVA",
        "CONQUER THE SAMURAI SPIRIT"
    };

    constexpr const char* PROFILE_MENU_ITEMS[] = {
        "CONTINUE", "CHOOSE ROOM", "STATS", "OPTIONS", "DELETE PROFILE"
    };

    constexpr const char* PAUSE_MENU_ITEMS[] = {
        "CONTINUE", "RESTART ROOM", "ROOM SELECT", "OPTIONS"
    };

    constexpr const char* RESULT_MENU_ITEMS[] = {
        "NEXT ROOM", "RETRY ROOM", "ROOM SELECT"
    };

    constexpr const char* OPTION_NAMES[] = {
        "MUSIC", "SFX", "SCREEN FLASH", "TEXT SPEED", "WINDOW PALETTE"
    };

    constexpr const char* VOLUME_NAMES[] = { "OFF", "25%", "50%", "75%", "100%" };
    constexpr const char* FLASH_NAMES[] = { "FULL", "REDUCED", "OFF" };
    constexpr const char* TEXT_SPEED_NAMES[] = { "SLOW", "NORMAL", "FAST", "INSTANT" };
    constexpr const char* PALETTE_NAMES[] = { "SAND", "ROYAL", "JADE", "STONE" };
    constexpr const char* NAME_KEYS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ<*";

    void draw_cursor_line(bn::regular_bg_map_cell* cells, int y, bool selected, const char* label)
    {
        TextRenderer::draw_text(cells, 5, y, selected ? ">" : " ");
        TextRenderer::draw_clipped(cells, 7, y, label, 21);
    }

    int profile_total_deaths(const ProfileData* profile)
    {
        return profile ? int(bn::min(profile->total_deaths, uint32_t(MAX_DEATH_COUNT))) : 0;
    }

    int reveal_speed(const GlobalSettings& settings)
    {
        switch(TextSpeed(settings.text_speed))
        {
            case TextSpeed::SLOW: return 4;
            case TextSpeed::FAST: return 1;
            case TextSpeed::INSTANT: return 0;
            default: return 2;
        }
    }

    void draw_revealed_line(bn::regular_bg_map_cell* cells, int y, const char* text,
                            int start, int visible)
    {
        int length = 0;
        const volatile char* scan = text;
        while(*scan)
        {
            ++scan;
            ++length;
        }
        int count = bn::max(0, bn::min(length, visible - start));
        char buffer[31] = {};
        for(int index = 0; index < count && index < 30; ++index)
        {
            buffer[index] = text[index];
        }
        TextRenderer::clear_text(cells, 0, y, UI_COLS);
        TextRenderer::draw_centered(cells, y, buffer);
    }
}

GameManager::GameManager() :
    _state(GameState::BOOT),
    _current_level(0),
    _frame(0),
    _death_count(0),
    _current_chamber(0),
    _gravity_flipped(false),
    _return_state(GameState::TITLE),
    _menu_map_item(_menu_cells[0], bn::size(32, 32)),
    _first_transition(true),
    _load_result(SaveLoadResult::FRESH),
    _selected_profile(0),
    _world_select_idx(0),
    _room_cursor(0),
    _skill_cursor(0),
    _selected_skill(SkillType::NONE),
    _profile_name(),
    _profile_name_length(0),
    _result_first_clear(false),
    _result_action_buffered(false),
    _world_result_idx(0),
    _confirm_action(ConfirmAction::NONE)
{
    global_game_manager = this;
    _load_result = _save_manager.load();
    apply_settings();

    bn::memory::clear(_menu_cells);
    bn::regular_bg_item menu_bg_item(
        bn::regular_bg_tiles_items::hud_tiles,
        bn::regular_bg_tiles_items::hud_tiles_palette,
        _menu_map_item
    );
    // A 32x32 map is centered by default; this aligns map rows 0..19 and
    // columns 0..29 with the 240x160 viewport.
    _menu_bg = menu_bg_item.create_bg(8, 48);
    _menu_bg->set_priority(1);
    _menu_bg_map = _menu_bg->map();
    apply_settings();
}

void GameManager::run()
{
    transition_to(GameState::BOOT);

    while(true)
    {
        _frame = (_frame + 1) % 32760;
        SfxPlayer::update();

        switch(_state)
        {
            case GameState::BOOT:             update_boot();             break;
            case GameState::TITLE:            update_title();            break;
            case GameState::PROFILE_SELECT:   update_profile_select();   break;
            case GameState::PROFILE_CREATE:   update_profile_create();   break;
            case GameState::PROFILE_MENU:     update_profile_menu();     break;
            case GameState::WORLD_SELECT:     update_world_select();     break;
            case GameState::ROOM_SELECT:      update_room_select();      break;
            case GameState::WORLD_INTRO:      update_world_intro();      break;
            case GameState::SKILL_SELECT:     update_skill_select();     break;
            case GameState::PLAYING:          update_playing();          break;
            case GameState::PAUSED:           update_paused();           break;
            case GameState::DEAD:             update_dead();             break;
            case GameState::LEVEL_CLEAR:      update_level_clear();      break;
            case GameState::WORLD_RESULT:     update_world_result();     break;
            case GameState::ENDING:           update_ending();           break;
            case GameState::CREDITS:          update_credits();          break;
            case GameState::GAME_COMPLETE:    update_game_complete();    break;
            case GameState::STATS:            update_stats();            break;
            case GameState::OPTIONS:          update_options();          break;
            case GameState::SAVE_RECOVERY:    update_save_recovery();    break;
            case GameState::CONFIRM_DIALOG:   update_confirm_dialog();   break;
            default:                          BN_ERROR("Invalid game state");
        }

        bn::core::update();
    }
}

const ProfileData* GameManager::active_profile() const
{
    return _save_manager.active_profile();
}

ProfileData* GameManager::active_profile()
{
    return _save_manager.active_profile();
}

void GameManager::update_boot()
{
    if(_frame == 32)
    {
        fade_out(8);
    }
    if(_frame == 40)
    {
        draw_boot();
        if(_menu_bg_map)
        {
            _menu_bg_map->reload_cells_ref();
        }
        fade_in(8);
    }
    if(_frame >= 80 || bn::keypad::start_pressed() || bn::keypad::a_pressed())
    {
        if(_load_result == SaveLoadResult::RECOVERED)
        {
            transition_to(GameState::SAVE_RECOVERY);
        }
        else
        {
            transition_to(GameState::TITLE);
        }
    }
}

void GameManager::update_title()
{
    if(_title_mummy_sprite.has_value())
    {
        int idle_frame = (_frame / 8) % 4;
        _title_mummy_sprite->set_tiles(
            bn::sprite_items::player_kha_men.tiles_item().create_tiles(idle_frame));
        constexpr int wave[] = { 0, 1, 2, 3, 2, 1, 0, -1, -2, -3, -2, -1 };
        _title_mummy_sprite->set_position(0, 15 + wave[(_frame / 6) % 12]);
    }

    if(_menu_bg)
    {
        bn::bg_palette_ptr palette = _menu_bg->palette();
        bn::fixed sin_val = bn::degrees_lut_sin((_frame * 4) % 360);
        int color_val = 20 + (sin_val * 11).round_integer();
        palette.set_color(1, bn::color(color_val, color_val, color_val));
    }

    if(_menu_bg_map)
    {
        _menu_bg_map->reload_cells_ref();
    }

    if(bn::keypad::start_pressed() || bn::keypad::a_pressed())
    {
        SfxPlayer::play(SfxCue::UI_CONFIRM);
        transition_to(GameState::PROFILE_SELECT);
    }
}

void GameManager::update_profile_select()
{
    FrontendAction action = _frontend.update();
    if(action.type == FrontendActionType::MOVED)
    {
        _selected_profile = action.value;
        draw_profile_select();
    }
    else if(action.type == FrontendActionType::CONFIRM ||
            action.type == FrontendActionType::START)
    {
        _selected_profile = _frontend.cursor();
        if(_save_manager.profile_damaged(_selected_profile))
        {
            _return_state = GameState::PROFILE_SELECT;
            transition_to(GameState::SAVE_RECOVERY);
        }
        else if(_save_manager.has_profile(_selected_profile))
        {
            _save_manager.select_profile(_selected_profile);
            _save_manager.commit();
            if(_save_manager.migration_notice_pending())
            {
                _save_manager.acknowledge_migration();
            }
            transition_to(GameState::PROFILE_MENU);
        }
        else
        {
            _profile_name_length = 0;
            _profile_name[0] = '\0';
            transition_to(GameState::PROFILE_CREATE);
        }
    }
    else if(action.type == FrontendActionType::BACK)
    {
        transition_to(GameState::TITLE);
    }
}

void GameManager::update_profile_create()
{
    int cursor = _frontend.cursor();
    int new_cursor = cursor;
    if(bn::keypad::left_pressed() && cursor % 7 > 0) --new_cursor;
    if(bn::keypad::right_pressed() && cursor % 7 < 6 && cursor + 1 < 28) ++new_cursor;
    if(bn::keypad::up_pressed() && cursor >= 7) new_cursor -= 7;
    if(bn::keypad::down_pressed() && cursor + 7 < 28) new_cursor += 7;
    if(new_cursor != cursor)
    {
        _frontend.set_cursor(new_cursor);
        SfxPlayer::play(SfxCue::UI_MOVE);
        draw_profile_create();
    }

    if(bn::keypad::a_pressed())
    {
        SfxPlayer::play(SfxCue::UI_CONFIRM);
        char key = NAME_KEYS[_frontend.cursor()];
        if(key == '<')
        {
            if(_profile_name_length > 0)
            {
                _profile_name[--_profile_name_length] = '\0';
            }
        }
        else if(key == '*')
        {
            if(_profile_name_length > 0)
            {
                open_confirm(ConfirmAction::CREATE_PROFILE, GameState::PROFILE_CREATE);
                return;
            }
        }
        else if(_profile_name_length < PROFILE_NAME_LENGTH)
        {
            _profile_name[_profile_name_length++] = key;
            _profile_name[_profile_name_length] = '\0';
        }
        draw_profile_create();
    }
    else if(bn::keypad::b_pressed())
    {
        SfxPlayer::play(SfxCue::UI_BACK);
        if(_profile_name_length > 0)
        {
            _profile_name[--_profile_name_length] = '\0';
            draw_profile_create();
        }
        else
        {
            transition_to(GameState::PROFILE_SELECT);
        }
    }
    else if(bn::keypad::start_pressed() && _profile_name_length > 0)
    {
        SfxPlayer::play(SfxCue::UI_CONFIRM);
        open_confirm(ConfirmAction::CREATE_PROFILE, GameState::PROFILE_CREATE);
    }
}

void GameManager::update_profile_menu()
{
    // ── Cheat code: Konami sequence ↑↑↓↓←→←→BA ──
    // Expected sequence indexed 0..9
    {
        enum CheatKey : uint8_t { CK_UP, CK_DOWN, CK_LEFT, CK_RIGHT, CK_B, CK_A };
        constexpr CheatKey SEQUENCE[] = {
            CK_UP, CK_UP, CK_DOWN, CK_DOWN,
            CK_LEFT, CK_RIGHT, CK_LEFT, CK_RIGHT,
            CK_B, CK_A
        };
        constexpr int SEQ_LEN = 10;

        // Detect which cheat-relevant key was pressed this frame (if any)
        CheatKey pressed_key = CK_UP; // dummy init
        bool any_cheat_key = false;
        if(bn::keypad::up_pressed())        { pressed_key = CK_UP;    any_cheat_key = true; }
        else if(bn::keypad::down_pressed()) { pressed_key = CK_DOWN;  any_cheat_key = true; }
        else if(bn::keypad::left_pressed()) { pressed_key = CK_LEFT;  any_cheat_key = true; }
        else if(bn::keypad::right_pressed()){ pressed_key = CK_RIGHT; any_cheat_key = true; }
        else if(bn::keypad::b_pressed())    { pressed_key = CK_B;     any_cheat_key = true; }
        else if(bn::keypad::a_pressed())    { pressed_key = CK_A;     any_cheat_key = true; }

        if(any_cheat_key)
        {
            if(pressed_key == SEQUENCE[_cheat_seq_idx])
            {
                ++_cheat_seq_idx;
                if(_cheat_seq_idx >= SEQ_LEN)
                {
                    // Cheat activated — unlock all rooms in active worlds
                    _cheat_seq_idx = 0;
                    int unlock_limit = ACTIVE_WORLDS * LEVELS_PER_WORLD;
                    if(_save_manager.unlock_all_rooms(unlock_limit))
                    {
                        SfxPlayer::play(SfxCue::UI_CONFIRM);
                        draw_profile_menu();
                        if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
                    }
                    return; // consume this frame's input
                }
            }
            else
            {
                // Wrong key — reset, but re-check if this key starts the sequence
                _cheat_seq_idx = (pressed_key == SEQUENCE[0]) ? 1 : 0;
            }
        }
    }

    FrontendAction action = _frontend.update();
    if(action.type == FrontendActionType::MOVED)
    {
        draw_profile_menu();
    }
    else if(action.type == FrontendActionType::BACK)
    {
        transition_to(GameState::PROFILE_SELECT);
    }
    else if(action.type == FrontendActionType::CONFIRM ||
            action.type == FrontendActionType::START)
    {
        switch(_frontend.cursor())
        {
            case 0: continue_profile(); break;
            case 1:
                _world_select_idx = bn::min(_save_manager.highest_unlocked_room(
                    _save_manager.active_slot()) / LEVELS_PER_WORLD, ACTIVE_WORLDS - 1);
                transition_to(GameState::WORLD_SELECT);
                break;
            case 2: transition_to(GameState::STATS); break;
            case 3:
                _return_state = GameState::PROFILE_MENU;
                transition_to(GameState::OPTIONS);
                break;
            case 4: open_confirm(ConfirmAction::DELETE_PROFILE, GameState::PROFILE_MENU); break;
            default: break;
        }
    }
}

void GameManager::continue_profile()
{
    const ProfileData* profile_data = active_profile();
    if(!profile_data)
    {
        transition_to(GameState::PROFILE_SELECT);
        return;
    }
    if(profile_data->completed)
    {
        transition_to(GameState::GAME_COMPLETE);
        return;
    }

    int next_room = bn::min(int(profile_data->next_room), TOTAL_LEVELS);
    if(next_room >= ACTIVE_WORLDS * LEVELS_PER_WORLD)
    {
        SfxPlayer::play(SfxCue::UI_DENIED);
        draw_profile_menu();
        return;
    }
    _current_level = next_room;
    _world_select_idx = _current_level / LEVELS_PER_WORLD;
    transition_to(GameState::WORLD_INTRO);
}

void GameManager::update_world_select()
{
    int old_world = _world_select_idx;
    if((bn::keypad::left_pressed() || bn::keypad::l_pressed()) && _world_select_idx > 0)
    {
        --_world_select_idx;
    }
    if((bn::keypad::right_pressed() || bn::keypad::r_pressed()) && _world_select_idx < 4)
    {
        ++_world_select_idx;
    }
    if(old_world != _world_select_idx)
    {
        SfxPlayer::play(SfxCue::UI_MOVE);
        if(_world_card_sprite)
        {
            _world_card_sprite->set_tiles(
                bn::sprite_items::world_cards.tiles_item().create_tiles(_world_select_idx));
        }
        draw_world_select_static();
    }

    if(bn::keypad::a_pressed() || bn::keypad::start_pressed())
    {
        int unlocked_room = _save_manager.highest_unlocked_room(_save_manager.active_slot());
        bool unlocked = _world_select_idx == 0 ||
                        unlocked_room >= _world_select_idx * LEVELS_PER_WORLD;
        if(_world_select_idx >= ACTIVE_WORLDS || !unlocked)
        {
            SfxPlayer::play(SfxCue::UI_DENIED);
        }
        else
        {
            SfxPlayer::play(SfxCue::UI_CONFIRM);
            int local_room = bn::min(LEVELS_PER_WORLD - 1,
                bn::max(0, unlocked_room - _world_select_idx * LEVELS_PER_WORLD));
            _room_cursor = local_room;
            transition_to(GameState::ROOM_SELECT);
        }
    }
    else if(bn::keypad::b_pressed())
    {
        SfxPlayer::play(SfxCue::UI_BACK);
        transition_to(GameState::PROFILE_MENU);
    }
}

void GameManager::update_room_select()
{
    int cursor = _room_cursor;
    if(bn::keypad::left_pressed() && cursor % 5 > 0) --cursor;
    if(bn::keypad::right_pressed() && cursor % 5 < 4) ++cursor;
    if(bn::keypad::up_pressed() && cursor >= 5) cursor -= 5;
    if(bn::keypad::down_pressed() && cursor + 5 < LEVELS_PER_WORLD) cursor += 5;

    if(bn::keypad::l_pressed() && _world_select_idx > 0)
    {
        --_world_select_idx;
        cursor = 0;
    }
    if(bn::keypad::r_pressed() && _world_select_idx + 1 < ACTIVE_WORLDS)
    {
        int unlocked = _save_manager.highest_unlocked_room(_save_manager.active_slot());
        if(unlocked >= (_world_select_idx + 1) * LEVELS_PER_WORLD)
        {
            ++_world_select_idx;
            cursor = 0;
        }
    }
    if(cursor != _room_cursor)
    {
        _room_cursor = cursor;
        SfxPlayer::play(SfxCue::UI_MOVE);
        draw_room_select();
    }

    if(bn::keypad::a_pressed() || bn::keypad::start_pressed())
    {
        int room = _world_select_idx * LEVELS_PER_WORLD + _room_cursor;
        const ProfileData* profile_data = active_profile();
        bool available = profile_data &&
            (room <= int(profile_data->next_room) ||
             _save_manager.room_cleared(_save_manager.active_slot(), room));
        if(available && _world_select_idx < ACTIVE_WORLDS)
        {
            SfxPlayer::play(SfxCue::UI_CONFIRM);
            _current_level = room;
            start_selected_room();
        }
        else
        {
            SfxPlayer::play(SfxCue::UI_DENIED);
        }
    }
    else if(bn::keypad::b_pressed())
    {
        SfxPlayer::play(SfxCue::UI_BACK);
        transition_to(GameState::WORLD_SELECT);
    }
}

void GameManager::start_selected_room()
{
    if(_current_level < 0 || _current_level >= TOTAL_LEVELS)
    {
        transition_to(GameState::PROFILE_MENU);
        return;
    }
    if(_current_level % LEVELS_PER_WORLD == 0)
    {
        transition_to(GameState::WORLD_INTRO);
    }
    else
    {
        int world = _current_level / LEVELS_PER_WORLD;
        if(world >= 1)
        {
            transition_to(GameState::SKILL_SELECT);
        }
        else
        {
            _selected_skill = SkillType::NONE;
            transition_to(GameState::PLAYING, true, false);
        }
    }
}

void GameManager::update_world_intro()
{
    if(_text_reveal.update())
    {
        draw_world_intro_static();
        if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
    }
    int timeout = 180;
    switch(TextSpeed(_save_manager.settings().text_speed))
    {
        case TextSpeed::SLOW: timeout = 240; break;
        case TextSpeed::FAST: timeout = 120; break;
        case TextSpeed::INSTANT: timeout = 90; break;
        default: break;
    }
    if(bn::keypad::a_pressed() || bn::keypad::start_pressed() || _frame >= timeout)
    {
        if(_frame < timeout)
        {
            SfxPlayer::play(SfxCue::UI_CONFIRM);
        }
        int world = _current_level / LEVELS_PER_WORLD;
        if(world >= 1)
        {
            transition_to(GameState::SKILL_SELECT);
        }
        else
        {
            _selected_skill = SkillType::NONE;
            transition_to(GameState::PLAYING, true, false);
        }
    }
    else if(bn::keypad::b_pressed())
    {
        SfxPlayer::play(SfxCue::UI_BACK);
        transition_to(GameState::ROOM_SELECT);
    }
}

void GameManager::update_skill_select()
{
    int available = _current_level / LEVELS_PER_WORLD >= 2 ? 3 : 2;
    FrontendAction action = _frontend.update();
    if(action.type == FrontendActionType::MOVED)
    {
        _skill_cursor = uint8_t(action.value);
        draw_skill_select_menu();
    }
    else if(action.type == FrontendActionType::CONFIRM ||
            action.type == FrontendActionType::START)
    {
        _skill_cursor = uint8_t(_frontend.cursor());
        _selected_skill = _skill_cursor < available ?
            SkillType(_skill_cursor + 1) : SkillType::NONE;
        if(ProfileData* profile_data = active_profile())
        {
            profile_data->preferred_skill = uint8_t(_selected_skill);
        }
        transition_to(GameState::PLAYING, true, false);
    }
    else if(action.type == FrontendActionType::BACK)
    {
        _selected_skill = SkillType::NONE;
        transition_to(GameState::PLAYING, true, false);
    }
}

void GameManager::update_playing()
{
    if(!_player)
    {
        return;
    }
    if(ProfileData* profile_data = active_profile())
    {
        if(profile_data->play_frames < 0xFFFFFFFFu)
        {
            ++profile_data->play_frames;
        }
    }

    if(bn::keypad::start_pressed())
    {
        transition_to(GameState::PAUSED);
        return;
    }
    if(_player->is_alive() && !_player->is_winning() && bn::keypad::b_pressed())
    {
        _player->try_activate_skill();
    }

    // ── Exit check FIRST: if player is touching exit, they win immediately ──
    // This prevents the frustrating case where boulder kills at the exit door
    if(_player->is_alive() && !_player->is_winning())
    {
        if(const ChamberTransition* transition = _level.check_transition(*_player))
        {
            transition_chamber(*transition);
            return;
        }
    }

    if(_player->is_alive() && !_player->is_winning() &&
       _level.check_exit_door(*_player))
    {
        SfxPlayer::play(SfxCue::ROOM_CLEAR);
        _player->start_win();
    }

    if(_player->is_winning())
    {
        _player->update(_gravity_flipped);
        _player->update_graphics();
        _camera.update(*_player);
        _screen_fx.update();

        SkillState no_skill;
        const SkillState& skill = _player ? _player->skill_state() : no_skill;
        _hud.update(_death_count, _current_level, "", _gravity_flipped, skill);

        if(_player->win_anim_done())
        {
            transition_to(GameState::LEVEL_CLEAR);
        }
        return;
    }

    bool was_alive = _player->is_alive();
    _trap_manager.update(*_player, _level);
    _gravity_flipped = _trap_manager.gravity_flipped();
    _player->update(_gravity_flipped);
    _level.resolve_collisions(*_player, _gravity_flipped);
    _player->update_graphics();

    if(_trap_manager.consume_flip_sfx())
    {
        _screen_fx.flash_white(12);
        _camera.shake(15, bn::fixed(2.5));
        SfxPlayer::play(SfxCue::GRAVITY_FLIP);
    }
    if(was_alive && !_player->is_alive())
    {
        _camera.shake(15, bn::fixed(2));
        _screen_fx.flash_white(10);
    }

    if(!_player->is_alive() && _player->die_anim_done())
    {
        if(_death_count < MAX_DEATH_COUNT)
        {
            ++_death_count;
        }
        _save_manager.record_death(_current_level);
        if(_death_count % 5 == 0)
        {
            _save_manager.commit();
        }
        transition_to(GameState::DEAD);
        return;
    }

    _camera.update(*_player);
    _screen_fx.update();
    SkillState no_skill;
    const SkillState& skill = _player ? _player->skill_state() : no_skill;
    _hud.update(_death_count, _current_level, "", _gravity_flipped, skill);
}

void GameManager::update_paused()
{
    FrontendAction action = _frontend.update();
    if(action.type == FrontendActionType::MOVED)
    {
        draw_pause_menu();
    }
    else if(action.type == FrontendActionType::BACK ||
            action.type == FrontendActionType::START)
    {
        transition_to(GameState::PLAYING);
    }
    else if(action.type == FrontendActionType::CONFIRM)
    {
        switch(_frontend.cursor())
        {
            case 0: transition_to(GameState::PLAYING); break;
            case 1: open_confirm(ConfirmAction::RESTART_ROOM, GameState::PAUSED); break;
            case 2: open_confirm(ConfirmAction::EXIT_TO_ROOM_SELECT, GameState::PAUSED); break;
            case 3:
                _return_state = GameState::PAUSED;
                transition_to(GameState::OPTIONS);
                break;
            default: break;
        }
    }
}

void GameManager::update_dead()
{
    if(_frame < 8)
    {
        draw_dead();
    }
    if(_frame >= 12 &&
       (bn::keypad::a_pressed() || bn::keypad::b_pressed() || bn::keypad::start_pressed()))
    {
        SfxPlayer::play(SfxCue::UI_CONFIRM);
        transition_to(GameState::PLAYING, false, true);
    }
}

void GameManager::update_level_clear()
{
    if(_frame < 60 && _frame % 5 == 0)
    {
        draw_room_result();
    }
    
    if(_frame < 45)
    {
        if(bn::keypad::a_pressed() || bn::keypad::start_pressed())
        {
            _result_action_buffered = true;
        }
        return;
    }

    if(_result_action_buffered)
    {
        _result_action_buffered = false;
        advance_after_result();
        return;
    }

    FrontendAction action = _frontend.update();
    if(action.type == FrontendActionType::MOVED)
    {
        draw_room_result();
    }
    else if(action.type == FrontendActionType::START)
    {
        advance_after_result();
    }
    else if(action.type == FrontendActionType::CONFIRM)
    {
        switch(_frontend.cursor())
        {
            case 0: advance_after_result(); break;
            case 1: transition_to(GameState::PLAYING, false, true); break;
            case 2:
                _world_select_idx = _current_level / LEVELS_PER_WORLD;
                _room_cursor = _current_level % LEVELS_PER_WORLD;
                transition_to(GameState::ROOM_SELECT);
                break;
            default: break;
        }
    }
}

void GameManager::advance_after_result()
{
    int next = _current_level + 1;
    if(next >= TOTAL_LEVELS)
    {
        transition_to(GameState::ENDING);
    }
    else if(_result_first_clear && next % LEVELS_PER_WORLD == 0)
    {
        _world_result_idx = _current_level / LEVELS_PER_WORLD;
        transition_to(GameState::WORLD_RESULT);
    }
    else
    {
        _current_level = next;
        start_selected_room();
    }
}

void GameManager::update_world_result()
{
    FrontendAction action = _frontend.update();
    if(action.type == FrontendActionType::MOVED)
    {
        draw_world_result();
    }
    else if(action.type == FrontendActionType::CONFIRM ||
            action.type == FrontendActionType::START)
    {
        if(_frontend.cursor() == 0)
        {
            int next_world = _world_result_idx + 1;
            if(next_world < ACTIVE_WORLDS)
            {
                _current_level = next_world * LEVELS_PER_WORLD;
                _world_select_idx = next_world;
                transition_to(GameState::WORLD_INTRO);
            }
            else
            {
                transition_to(GameState::PROFILE_MENU);
            }
        }
        else
        {
            _world_select_idx = bn::min(_world_result_idx + 1, 4);
            transition_to(GameState::WORLD_SELECT);
        }
    }
    else if(action.type == FrontendActionType::BACK)
    {
        transition_to(GameState::WORLD_SELECT);
    }
}

void GameManager::update_ending()
{
    if(_text_reveal.update())
    {
        draw_ending();
        if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
    }
    if(_frame >= 240 || bn::keypad::a_pressed() || bn::keypad::start_pressed())
    {
        if(_frame < 240)
        {
            SfxPlayer::play(SfxCue::UI_CONFIRM);
        }
        transition_to(GameState::CREDITS);
    }
}

void GameManager::update_credits()
{
    if(_text_reveal.update())
    {
        draw_credits();
        if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
    }
    if(_frame >= 300 || bn::keypad::a_pressed() || bn::keypad::start_pressed())
    {
        if(_frame < 300)
        {
            SfxPlayer::play(SfxCue::UI_CONFIRM);
        }
        transition_to(GameState::GAME_COMPLETE);
    }
}

void GameManager::update_game_complete()
{
    if(bn::keypad::a_pressed() || bn::keypad::b_pressed() || bn::keypad::start_pressed())
    {
        SfxPlayer::play(bn::keypad::b_pressed() ? SfxCue::UI_BACK : SfxCue::UI_CONFIRM);
        transition_to(GameState::PROFILE_MENU);
    }
}

void GameManager::update_stats()
{
    if(bn::keypad::a_pressed() || bn::keypad::b_pressed() || bn::keypad::start_pressed())
    {
        SfxPlayer::play(bn::keypad::b_pressed() ? SfxCue::UI_BACK : SfxCue::UI_CONFIRM);
        transition_to(GameState::PROFILE_MENU);
    }
}

void GameManager::update_options()
{
    GlobalSettings& settings = _save_manager.settings();
    bool changed = false;
    if(bn::keypad::up_pressed() && _frontend.cursor() > 0)
    {
        _frontend.set_cursor(_frontend.cursor() - 1);
        changed = true;
    }
    if(bn::keypad::down_pressed() && _frontend.cursor() < 4)
    {
        _frontend.set_cursor(_frontend.cursor() + 1);
        changed = true;
    }
    int direction = bn::keypad::left_pressed() ? -1 :
                    bn::keypad::right_pressed() || bn::keypad::a_pressed() ? 1 : 0;
    if(direction)
    {
        uint8_t* value = nullptr;
        int max_value = 0;
        switch(_frontend.cursor())
        {
            case 0: value = &settings.music_volume; max_value = 4; break;
            case 1: value = &settings.sfx_volume; max_value = 4; break;
            case 2: value = &settings.flash_mode; max_value = 2; break;
            case 3: value = &settings.text_speed; max_value = 3; break;
            case 4: value = &settings.window_palette; max_value = 3; break;
            default: break;
        }
        if(value)
        {
            int next = int(*value) + direction;
            if(next < 0) next = max_value;
            if(next > max_value) next = 0;
            *value = uint8_t(next);
            apply_settings();
            changed = true;
        }
    }
    if(changed)
    {
        SfxPlayer::play(SfxCue::UI_MOVE);
        draw_options();
    }
    if(bn::keypad::b_pressed() || bn::keypad::start_pressed())
    {
        SfxPlayer::play(SfxCue::UI_BACK);
        _save_manager.commit();
        transition_to(_return_state);
    }
}

void GameManager::update_save_recovery()
{
    if(_save_manager.profile_damaged(_selected_profile))
    {
        FrontendAction action = _frontend.update();
        if(action.type == FrontendActionType::MOVED)
        {
            draw_save_recovery();
        }
        else if(action.type == FrontendActionType::CONFIRM)
        {
            if(_frontend.cursor() == 0)
            {
                _save_manager.delete_profile(_selected_profile);
            }
            transition_to(GameState::PROFILE_SELECT);
        }
        else if(action.type == FrontendActionType::BACK)
        {
            transition_to(GameState::PROFILE_SELECT);
        }
    }
    else if(bn::keypad::a_pressed() || bn::keypad::b_pressed() || bn::keypad::start_pressed())
    {
        SfxPlayer::play(bn::keypad::b_pressed() ? SfxCue::UI_BACK : SfxCue::UI_CONFIRM);
        transition_to(GameState::TITLE);
    }
}

void GameManager::open_confirm(ConfirmAction action, GameState return_state)
{
    _confirm_action = action;
    _return_state = return_state;
    transition_to(GameState::CONFIRM_DIALOG);
}

void GameManager::update_confirm_dialog()
{
    FrontendAction action = _frontend.update();
    if(action.type == FrontendActionType::MOVED)
    {
        draw_confirm_dialog();
    }
    else if(action.type == FrontendActionType::BACK)
    {
        _confirm_action = ConfirmAction::NONE;
        transition_to(_return_state);
    }
    else if(action.type == FrontendActionType::CONFIRM ||
            action.type == FrontendActionType::START)
    {
        if(_frontend.cursor() != 0)
        {
            _confirm_action = ConfirmAction::NONE;
            transition_to(_return_state);
            return;
        }

        ConfirmAction confirmed = _confirm_action;
        _confirm_action = ConfirmAction::NONE;
        switch(confirmed)
        {
            case ConfirmAction::CREATE_PROFILE:
                _save_manager.create_profile(_selected_profile, _profile_name);
                _save_manager.commit();
                _world_select_idx = 0;
                transition_to(GameState::WORLD_SELECT);
                break;
            case ConfirmAction::DELETE_PROFILE:
                _save_manager.delete_profile(_save_manager.active_slot());
                transition_to(GameState::PROFILE_SELECT);
                break;
            case ConfirmAction::RESTART_ROOM:
                transition_to(GameState::PLAYING, false, true);
                break;
            case ConfirmAction::EXIT_TO_ROOM_SELECT:
                _save_manager.commit();
                _world_select_idx = _current_level / LEVELS_PER_WORLD;
                _room_cursor = _current_level % LEVELS_PER_WORLD;
                transition_to(GameState::ROOM_SELECT);
                break;
            default:
                transition_to(_return_state);
                break;
        }
    }
}

void GameManager::transition_to(GameState new_state, bool reload, bool restart)
{
    bool fast = (_state == GameState::PLAYING && new_state == GameState::PAUSED) ||
                (_state == GameState::PAUSED && new_state == GameState::PLAYING) ||
                new_state == GameState::DEAD ||
                (_state == GameState::DEAD && new_state == GameState::PLAYING);
    if(!_first_transition && !fast)
    {
        fade_out(8);
    }
    _first_transition = false;

    if(new_state == GameState::TITLE)
    {
        _title_mummy_sprite = bn::sprite_items::player_kha_men.create_sprite(0, 15);
        _title_logo_sprite = bn::sprite_items::title_logo.create_sprite(0, -40);
        _title_logo_sprite->set_bg_priority(0);
    }
    else
    {
        _title_mummy_sprite.reset();
        _title_logo_sprite.reset();
        if(_menu_bg)
        {
            bn::bg_palette_ptr palette = _menu_bg->palette();
            palette.set_color(1, bn::color(31, 31, 31));
        }
    }
    _world_card_sprite.reset();
    _frontend_art_sprite.reset();

    if(new_state == GameState::WORLD_SELECT || new_state == GameState::WORLD_INTRO)
    {
        int world = new_state == GameState::WORLD_SELECT ?
            _world_select_idx : _current_level / LEVELS_PER_WORLD;
        world = bn::max(0, bn::min(4, world));
        int card_y = new_state == GameState::WORLD_INTRO ? -4 : 8;
        _world_card_sprite = bn::sprite_items::world_cards.create_sprite(0, card_y);
        _world_card_sprite->set_tiles(
            bn::sprite_items::world_cards.tiles_item().create_tiles(world));
        _world_card_sprite->set_bg_priority(0);
    }
    else
    {
        int art_frame = -1;
        if(new_state == GameState::PROFILE_SELECT || new_state == GameState::PROFILE_CREATE ||
           new_state == GameState::PROFILE_MENU) art_frame = 1;
        else if(new_state == GameState::LEVEL_CLEAR) art_frame = 2;
        else if(new_state == GameState::WORLD_RESULT) art_frame = 3;
        else if(new_state == GameState::ENDING || new_state == GameState::CREDITS) art_frame = 4;
        else if(new_state == GameState::GAME_COMPLETE) art_frame = 5;
        if(art_frame >= 0)
        {
            _frontend_art_sprite = bn::sprite_items::frontend_art.create_sprite(88, -48);
            _frontend_art_sprite->set_tiles(
                bn::sprite_items::frontend_art.tiles_item().create_tiles(art_frame));
            _frontend_art_sprite->set_bg_priority(0);
        }
    }

    if(new_state == GameState::PLAYING)
    {
        if(bn::music::playing_item())
        {
            bn::music::stop();
        }
        _hud.set_visible(true);
        _menu_bg->set_visible(false);
        if(reload)
        {
            load_level(_current_level);
            if(_player)
            {
                _player->set_skill(_selected_skill);
            }
        }
        else if(restart)
        {
            restart_level();
        }
    }
    else
    {
        _hud.set_visible(false);
        _menu_bg->set_visible(true);
    }

    if(new_state == GameState::LEVEL_CLEAR)
    {
        _result_action_buffered = false;
        _result_first_clear = _save_manager.mark_room_cleared(_current_level, _death_count);
        _save_manager.commit();
    }

    if(new_state == GameState::WORLD_INTRO)
    {
        int world = bn::max(0, bn::min(4, _current_level / LEVELS_PER_WORLD));
        int character_count = bn::string_view(WORLD_TITLES[world]).size() +
                              bn::string_view(WORLD_FLAVOR[world]).size();
        _text_reveal.reset(character_count, reveal_speed(_save_manager.settings()));
    }
    else if(new_state == GameState::ENDING)
    {
        _text_reveal.reset(68, reveal_speed(_save_manager.settings()));
    }
    else if(new_state == GameState::CREDITS)
    {
        _text_reveal.reset(79, reveal_speed(_save_manager.settings()));
    }

    _state = new_state;
    _frame = 0;
    
    bool is_gameplay = new_state == GameState::PLAYING || new_state == GameState::PAUSED ||
                       new_state == GameState::DEAD || new_state == GameState::LEVEL_CLEAR ||
                       new_state == GameState::SKILL_SELECT;
    if(!is_gameplay)
    {
        _world_bg.reset();
        if(new_state != GameState::BOOT)
        {
            bn::optional<bn::music_item> playing = bn::music::playing_item();
            if(!playing || playing.value() != bn::music_items::tomb_crossing_theme)
            {
                bn::music_items::tomb_crossing_theme.play(
                    bn::fixed(_save_manager.settings().music_volume) / 4);
            }
        }
        apply_settings();
    }

    draw_current_state();

    if(!fast)
    {
        fade_in(8, new_state == GameState::PAUSED ? bn::fixed(0.5) : bn::fixed(0));
    }
}

void GameManager::draw_current_state()
{
    TextRenderer::clear_visible(_menu_cells);
    switch(_state)
    {
        case GameState::BOOT: draw_boot(); break;
        case GameState::TITLE: draw_title(); break;
        case GameState::PROFILE_SELECT:
            _frontend.reset(PROFILE_COUNT, _selected_profile);
            draw_profile_select();
            break;
        case GameState::PROFILE_CREATE:
            _frontend.reset(28);
            draw_profile_create();
            break;
        case GameState::PROFILE_MENU:
            _frontend.reset(5);
            draw_profile_menu();
            break;
        case GameState::WORLD_SELECT: draw_world_select_static(); break;
        case GameState::ROOM_SELECT: draw_room_select(); break;
        case GameState::WORLD_INTRO: draw_world_intro_static(); break;
        case GameState::SKILL_SELECT:
        {
            int skills = _current_level / LEVELS_PER_WORLD >= 2 ? 3 : 2;
            int preferred = 0;
            if(const ProfileData* profile_data = active_profile())
            {
                preferred = profile_data->preferred_skill > 0 ?
                    bn::min(profile_data->preferred_skill - 1, skills) : skills;
            }
            _frontend.reset(skills + 1, preferred);
            _skill_cursor = uint8_t(preferred);
            draw_skill_select_menu();
            break;
        }
        case GameState::PAUSED:
            _frontend.reset(4);
            draw_pause_menu();
            break;
        case GameState::DEAD: draw_dead(); break;
        case GameState::LEVEL_CLEAR:
            _frontend.reset(3);
            draw_room_result();
            break;
        case GameState::WORLD_RESULT:
            _frontend.reset(2);
            draw_world_result();
            break;
        case GameState::ENDING: draw_ending(); break;
        case GameState::CREDITS: draw_credits(); break;
        case GameState::GAME_COMPLETE: draw_game_complete(); break;
        case GameState::STATS: draw_stats(); break;
        case GameState::OPTIONS:
            _frontend.reset(5);
            draw_options();
            break;
        case GameState::SAVE_RECOVERY:
            _frontend.reset(2);
            draw_save_recovery();
            break;
        case GameState::CONFIRM_DIALOG:
            _frontend.reset(2);
            draw_confirm_dialog();
            break;
        default: break;
    }
    if(_menu_bg_map)
    {
        _menu_bg_map->reload_cells_ref();
    }
}

void GameManager::draw_boot()
{
    TextRenderer::clear_visible(_menu_cells);
    if(_frame < 40)
    {
        TextRenderer::draw_centered(_menu_cells, 8, "QUANDH TEAM");
    }
    else
    {
        TextRenderer::draw_centered(_menu_cells, 8, "POWERED BY BUTANO");
    }
}

void GameManager::draw_title()
{
    TextRenderer::draw_centered(_menu_cells, 7, "100 ROOMS. ONE ESCAPE.");
    TextRenderer::draw_centered(_menu_cells, 15, "PRESS START");
}

void GameManager::draw_profile_select()
{
    TextRenderer::clear_visible(_menu_cells);
    TextRenderer::draw_centered(_menu_cells, 1, "SELECT PROFILE");
    for(int slot = 0; slot < PROFILE_COUNT; ++slot)
    {
        int y = 3 + slot * 5;
        TextRenderer::draw_panel(_menu_cells, 2, y, 26, 4);
        TextRenderer::draw_text(_menu_cells, 3, y + 1, slot == _frontend.cursor() ? ">" : " ");
        if(_save_manager.profile_damaged(slot))
        {
            TextRenderer::draw_text(_menu_cells, 5, y + 1, "DAMAGED - RECOVERY");
        }
        else if(_save_manager.has_profile(slot))
        {
            const ProfileData& profile_data = _save_manager.profile(slot);
            TextRenderer::draw_clipped(_menu_cells, 5, y + 1, profile_data.name, 8);
            bn::string<20> detail = profile_data.completed ? " COMPLETE" : " ROOM ";
            if(!profile_data.completed)
            {
                detail.append(bn::to_string<4>(bn::min(100, int(profile_data.next_room) + 1)));
            }
            TextRenderer::draw_text(_menu_cells, 14, y + 1, detail);

            bn::string<24> metadata = "WORLD ";
            int world = profile_data.completed ? 5 :
                bn::min(5, int(profile_data.next_room) / LEVELS_PER_WORLD + 1);
            metadata.append(bn::to_string<4>(world));
            metadata.append("  DEATHS ");
            metadata.append(bn::to_string<12>(profile_total_deaths(&profile_data)));
            TextRenderer::draw_clipped(_menu_cells, 5, y + 2, metadata, 22);
        }
        else
        {
            TextRenderer::draw_text(_menu_cells, 5, y + 1, "EMPTY SLOT");
        }
    }
    if(_save_manager.migration_notice_pending())
    {
        TextRenderer::draw_centered(_menu_cells, 17, "OLD SAVE MOVED TO SLOT 1");
    }
    else
    {
        TextRenderer::draw_centered(_menu_cells, 18, "A:SELECT  B:TITLE");
    }
    if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
}

void GameManager::draw_profile_create()
{
    TextRenderer::clear_visible(_menu_cells);
    TextRenderer::draw_centered(_menu_cells, 1, "NAME YOUR EXPLORER");
    TextRenderer::draw_panel(_menu_cells, 8, 3, 14, 3);
    TextRenderer::draw_clipped(_menu_cells, 11, 4,
        _profile_name_length ? _profile_name : "_", PROFILE_NAME_LENGTH);

    for(int index = 0; index < 28; ++index)
    {
        int x = 4 + (index % 7) * 3;
        int y = 8 + (index / 7) * 2;
        char label[2] = { NAME_KEYS[index], '\0' };
        if(index == _frontend.cursor())
        {
            TextRenderer::draw_text(_menu_cells, x - 1, y, ">");
        }
        TextRenderer::draw_text(_menu_cells, x, y, label);
    }
    TextRenderer::draw_text(_menu_cells, 3, 16, "<:DEL  *:END  START:END");
    TextRenderer::draw_centered(_menu_cells, 18, "B:DELETE / BACK");
    if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
}

void GameManager::draw_profile_menu()
{
    TextRenderer::clear_visible(_menu_cells);
    const ProfileData* profile_data = active_profile();
    TextRenderer::draw_centered(_menu_cells, 1, profile_data ? profile_data->name : "PROFILE");
    for(int item = 0; item < 5; ++item)
    {
        draw_cursor_line(_menu_cells, 5 + item * 2, item == _frontend.cursor(),
                         PROFILE_MENU_ITEMS[item]);
    }
    if(profile_data && !profile_data->completed &&
       profile_data->next_room >= ACTIVE_WORLDS * LEVELS_PER_WORLD)
    {
        TextRenderer::draw_centered(_menu_cells, 16, "CURRENT CONTENT COMPLETE");
    }
    TextRenderer::draw_centered(_menu_cells, 18, "A:SELECT  B:PROFILES");
    if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
}

void GameManager::draw_world_select_static()
{
    if(_menu_bg)
    {
        constexpr bn::color world_accents[] = {
            bn::color(24, 19, 8),   // W1 Egypt: sandy gold
            bn::color(18, 12, 28),  // W2 Royal: deep purple
            bn::color(24, 8, 0),    // W3 Qin: terracotta
            bn::color(12, 16, 20),  // W4 Aztec: stone teal
            bn::color(11, 8, 4),    // W5 Kofun: dark wood
        };
        bn::bg_palette_ptr palette = _menu_bg->palette();
        palette.set_color(3, world_accents[bn::min(4, _world_select_idx)]);
    }

    TextRenderer::clear_visible(_menu_cells);
    TextRenderer::draw_centered(_menu_cells, 1, "CHOOSE WORLD");
    TextRenderer::draw_panel(_menu_cells, 3, 3, 24, 14);

    bn::string<20> world_line = "<  WORLD ";
    world_line.append(bn::to_string<4>(_world_select_idx + 1));
    world_line.append("  >");
    TextRenderer::draw_centered(_menu_cells, 4, world_line);

    int unlocked_room = _save_manager.highest_unlocked_room(_save_manager.active_slot());
    bool unlocked = _world_select_idx == 0 ||
                    unlocked_room >= _world_select_idx * LEVELS_PER_WORLD;
    if(_world_select_idx >= ACTIVE_WORLDS)
    {
        TextRenderer::draw_centered(_menu_cells, 16, "COMING SOON");
    }
    else if(!unlocked)
    {
        bn::string<24> lock = "CLEAR WORLD ";
        lock.append(bn::to_string<4>(_world_select_idx));
        lock.append(" TO UNLOCK");
        TextRenderer::draw_centered(_menu_cells, 16, lock);
    }
    else
    {
        bn::string<24> progress = "CLEARED ";
        progress.append(bn::to_string<4>(_save_manager.cleared_count(
            _save_manager.active_slot(), _world_select_idx)));
        progress.append("/20");
        TextRenderer::draw_centered(_menu_cells, 16, progress);
    }
    TextRenderer::draw_centered(_menu_cells, 18, "A:ROOMS  L/R:MOVE  B:BACK");
    if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
}

void GameManager::draw_room_select()
{
    TextRenderer::clear_visible(_menu_cells);
    bn::string<24> title = "WORLD ";
    title.append(bn::to_string<4>(_world_select_idx + 1));
    title.append(" - SELECT ROOM");
    TextRenderer::draw_centered(_menu_cells, 1, title);

    const ProfileData* profile_data = active_profile();
    for(int local = 0; local < LEVELS_PER_WORLD; ++local)
    {
        int room = _world_select_idx * LEVELS_PER_WORLD + local;
        int x = 2 + (local % 5) * 6;
        int y = 4 + (local / 5) * 3;
        bool cleared = _save_manager.room_cleared(_save_manager.active_slot(), room);
        bool available = profile_data &&
            (room <= int(profile_data->next_room) || cleared);
        TextRenderer::draw_text(_menu_cells, x, y, local == _room_cursor ? ">" : " ");
        TextRenderer::draw_text(_menu_cells, x + 1, y, cleared ? "*" : available ? " " : "\x04");
        bn::string<4> number = bn::to_string<4>(room + 1);
        if(room + 1 < 10)
        {
            TextRenderer::draw_text(_menu_cells, x + 2, y, "0");
            TextRenderer::draw_text(_menu_cells, x + 3, y, number);
        }
        else
        {
            TextRenderer::draw_text(_menu_cells, x + 2, y, number);
        }
    }

    int selected_room = _world_select_idx * LEVELS_PER_WORLD + _room_cursor;
    bn::string<24> details = "ROOM ";
    details.append(bn::to_string<4>(selected_room + 1));
    if(const ProfileData* selected_profile = active_profile())
    {
        uint16_t best = selected_profile->best_level_deaths[selected_room];
        if(best != 0xFFFF)
        {
            details.append("  BEST ");
            details.append(bn::to_string<8>(best));
            details.append("D");
        }
    }
    TextRenderer::draw_centered(_menu_cells, 16, details);
    TextRenderer::draw_centered(_menu_cells, 18, "A:PLAY  B:WORLD  L/R:WORLD");
    if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
}

void GameManager::draw_world_intro_static()
{
    int world = bn::max(0, bn::min(4, _current_level / LEVELS_PER_WORLD));
    int title_length = bn::string_view(WORLD_TITLES[world]).size();
    TextRenderer::draw_panel(_menu_cells, 2, 2, 26, 15);
    draw_revealed_line(_menu_cells, 4, WORLD_TITLES[world], 0, _text_reveal.visible());
    draw_revealed_line(_menu_cells, 14, WORLD_FLAVOR[world], title_length,
                       _text_reveal.visible());
    TextRenderer::draw_centered(_menu_cells, 18, "A:CONTINUE  B:ROOMS");
}

void GameManager::draw_skill_select_menu()
{
    TextRenderer::clear_visible(_menu_cells);
    TextRenderer::draw_centered(_menu_cells, 1, "CHOOSE ONE RELIC");
    int skill_count = _current_level / LEVELS_PER_WORLD >= 2 ? 3 : 2;
    for(int item = 0; item <= skill_count; ++item)
    {
        const char* label = item < skill_count ? SKILL_INFO[item + 1].name : "NO RELIC";
        draw_cursor_line(_menu_cells, 5 + item * 2, item == _frontend.cursor(), label);
    }
    if(_frontend.cursor() < skill_count)
    {
        TextRenderer::draw_panel(_menu_cells, 3, 13, 24, 3);
        TextRenderer::draw_centered(_menu_cells, 14,
            SKILL_INFO[_frontend.cursor() + 1].desc);
    }
    TextRenderer::draw_centered(_menu_cells, 17, "ONE USE IN THIS ROOM");
    TextRenderer::draw_centered(_menu_cells, 18, "A:SELECT  B:NO RELIC");
    if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
}

void GameManager::draw_pause_menu()
{
    TextRenderer::clear_visible(_menu_cells);
    TextRenderer::draw_panel(_menu_cells, 4, 3, 22, 14);
    TextRenderer::draw_centered(_menu_cells, 5, "PAUSED");
    for(int item = 0; item < 4; ++item)
    {
        draw_cursor_line(_menu_cells, 8 + item * 2, item == _frontend.cursor(),
                         PAUSE_MENU_ITEMS[item]);
    }
    TextRenderer::draw_centered(_menu_cells, 18, "A:SELECT  B:CONTINUE");
    if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
}

void GameManager::draw_dead()
{
    TextRenderer::clear_visible(_menu_cells);
    int offset = 0;
    if(_frame < 8)
    {
        offset = (_frame % 2 == 0) ? 1 : -1;
    }
    int cx = TextRenderer::centered_x(8) + offset;
    TextRenderer::draw_text(_menu_cells, cx, 6, "YOU DIED");
    
    bn::string<24> deaths = "ROOM DEATHS: ";
    deaths.append(bn::to_string<8>(_death_count));
    TextRenderer::draw_centered(_menu_cells, 10, deaths);
    TextRenderer::draw_centered(_menu_cells, 14, "A/B/START: RETRY");
}

void GameManager::draw_room_result()
{
    TextRenderer::clear_visible(_menu_cells);
    
    if(_frame >= 0)
    {
        bn::string<24> clear = "ROOM ";
        clear.append(bn::to_string<4>(_current_level + 1));
        clear.append(" CLEAR");
        TextRenderer::draw_centered(_menu_cells, 2, clear);
    }

    if(_frame >= 15)
    {
        bn::string<20> deaths = "DEATHS ";
        if(_death_count == 0)
        {
            deaths.append("PERFECT!");
        }
        else
        {
            deaths.append(bn::to_string<8>(_death_count));
        }
        TextRenderer::draw_centered(_menu_cells, 5, deaths);
    }
    
    if(_frame >= 25)
    {
        if(const ProfileData* profile_data = active_profile())
        {
            uint16_t best = profile_data->best_level_deaths[_current_level];
            bn::string<20> best_line = "BEST ";
            best_line.append(bn::to_string<8>(best == 0xFFFF ? 0 : best));
            TextRenderer::draw_centered(_menu_cells, 6, best_line);
        }
    }
    
    if(_frame >= 35)
    {
        if(active_profile())
        {
            bn::string<20> progress = "WORLD ";
            progress.append(bn::to_string<4>(_save_manager.cleared_count(
                _save_manager.active_slot(), _current_level / LEVELS_PER_WORLD)));
            progress.append("/20");
            TextRenderer::draw_centered(_menu_cells, 7, progress);
        }
    }
    
    if(_frame >= 45)
    {
        for(int item = 0; item < 3; ++item)
        {
            draw_cursor_line(_menu_cells, 10 + item * 2, item == _frontend.cursor(),
                             RESULT_MENU_ITEMS[item]);
        }
    }
    
    if(_frame >= 55)
    {
        TextRenderer::draw_text(_menu_cells, 21, 5, "\x0E SAVED");
        TextRenderer::draw_centered(_menu_cells, 18, "A:SELECT  START:NEXT");
    }
    
    if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
}

void GameManager::draw_world_result()
{
    TextRenderer::clear_visible(_menu_cells);
    TextRenderer::draw_centered(_menu_cells, 2, "WORLD ESCAPED");
    TextRenderer::draw_centered(_menu_cells, 4, WORLD_TITLES[_world_result_idx]);
    TextRenderer::draw_centered(_menu_cells, 6, "20 / 20 ROOMS");
    if(const ProfileData* profile_data = active_profile())
    {
        uint32_t world_deaths = 0;
        int first_room = _world_result_idx * LEVELS_PER_WORLD;
        for(int room = first_room; room < first_room + LEVELS_PER_WORLD; ++room)
        {
            world_deaths += profile_data->level_deaths[room];
        }
        bn::string<24> deaths = "WORLD DEATHS ";
        deaths.append(bn::to_string<12>(world_deaths));
        TextRenderer::draw_centered(_menu_cells, 8, deaths);
    }
    if(_world_result_idx + 1 < ACTIVE_WORLDS)
    {
        bn::string<24> unlock = "WORLD ";
        unlock.append(bn::to_string<4>(_world_result_idx + 2));
        unlock.append(" UNLOCKED");
        TextRenderer::draw_centered(_menu_cells, 10, unlock);
    }
    else
    {
        TextRenderer::draw_centered(_menu_cells, 10, "CURRENT CONTENT COMPLETE");
    }
    draw_cursor_line(_menu_cells, 13, _frontend.cursor() == 0,
                     _world_result_idx + 1 < ACTIVE_WORLDS ? "NEXT WORLD" : "PROFILE MENU");
    draw_cursor_line(_menu_cells, 15, _frontend.cursor() == 1, "WORLD SELECT");
    if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
}

void GameManager::draw_ending()
{
    constexpr const char* LINE_1 = "THE LAST SEAL BREAKS.";
    constexpr const char* LINE_2 = "SUNLIGHT FINDS KHA-MEN";
    constexpr const char* LINE_3 = "AFTER THOUSANDS OF YEARS.";
    int line_1_end = bn::string_view(LINE_1).size();
    int line_2_end = line_1_end + bn::string_view(LINE_2).size();
    TextRenderer::draw_panel(_menu_cells, 2, 3, 26, 14);
    draw_revealed_line(_menu_cells, 6, LINE_1, 0, _text_reveal.visible());
    draw_revealed_line(_menu_cells, 9, LINE_2, line_1_end, _text_reveal.visible());
    draw_revealed_line(_menu_cells, 11, LINE_3, line_2_end, _text_reveal.visible());
    TextRenderer::draw_centered(_menu_cells, 16, "A:CONTINUE");
}

void GameManager::draw_credits()
{
    constexpr const char* LINES[] = {
        "TOMB CROSSING", "DESIGN AND CODE", "QUANDH TEAM",
        "POWERED BY BUTANO", "THANKS FOR PLAYING"
    };
    constexpr int ROWS[] = { 2, 6, 8, 11, 15 };
    int start = 0;
    for(int index = 0; index < 5; ++index)
    {
        draw_revealed_line(_menu_cells, ROWS[index], LINES[index], start,
                           _text_reveal.visible());
        start += bn::string_view(LINES[index]).size();
    }
}

void GameManager::draw_game_complete()
{
    TextRenderer::draw_centered(_menu_cells, 2, "KHA-MEN IS FREE");
    TextRenderer::draw_centered(_menu_cells, 5, "100 / 100 ROOMS");
    if(const ProfileData* profile_data = active_profile())
    {
        bn::string<24> deaths = "TOTAL DEATHS ";
        deaths.append(bn::to_string<16>(profile_total_deaths(profile_data)));
        TextRenderer::draw_centered(_menu_cells, 8, deaths);
    }
    TextRenderer::draw_centered(_menu_cells, 13, "COMPLETION BADGE: GOLD");
    TextRenderer::draw_centered(_menu_cells, 17, "A:PROFILE MENU");
}

void GameManager::draw_stats()
{
    TextRenderer::draw_centered(_menu_cells, 1, "PROFILE STATS");
    const ProfileData* profile_data = active_profile();
    if(!profile_data)
    {
        return;
    }
    bn::string<24> rooms = "ROOMS CLEARED ";
    rooms.append(bn::to_string<8>(_save_manager.cleared_count(_save_manager.active_slot())));
    rooms.append("/100");
    TextRenderer::draw_text(_menu_cells, 3, 4, rooms);

    bn::string<24> deaths = "TOTAL DEATHS ";
    deaths.append(bn::to_string<16>(profile_total_deaths(profile_data)));
    TextRenderer::draw_text(_menu_cells, 3, 6, deaths);

    uint32_t seconds = profile_data->play_frames / 60;
    bn::string<24> time = "PLAY TIME ";
    time.append(bn::to_string<12>(seconds / 3600));
    time.append("H ");
    time.append(bn::to_string<8>((seconds / 60) % 60));
    time.append("M");
    TextRenderer::draw_text(_menu_cells, 3, 8, time);

    int hardest = 0;
    int easiest = -1;
    uint16_t max_deaths = 0;
    uint16_t min_best = 0xFFFF;
    for(int room = 0; room < TOTAL_LEVELS; ++room)
    {
        if(profile_data->level_deaths[room] > max_deaths)
        {
            max_deaths = profile_data->level_deaths[room];
            hardest = room;
        }
        if(profile_data->best_level_deaths[room] != 0xFFFF &&
           profile_data->best_level_deaths[room] < min_best)
        {
            min_best = profile_data->best_level_deaths[room];
            easiest = room;
        }
    }
    bn::string<24> hard = "HARDEST ROOM ";
    hard.append(bn::to_string<4>(hardest + 1));
    TextRenderer::draw_text(_menu_cells, 3, 10, hard);
    if(easiest >= 0)
    {
        bn::string<24> best = "BEST ROOM ";
        best.append(bn::to_string<4>(easiest + 1));
        TextRenderer::draw_text(_menu_cells, 3, 12, best);
    }
    TextRenderer::draw_text(_menu_cells, 3, 14,
        profile_data->completed ? "STATUS COMPLETE" : "STATUS EXPLORING");
    TextRenderer::draw_centered(_menu_cells, 18, "B:BACK");
}

void GameManager::draw_options()
{
    TextRenderer::clear_visible(_menu_cells);
    TextRenderer::draw_centered(_menu_cells, 1, "OPTIONS");
    const GlobalSettings& settings = _save_manager.settings();
    const char* values[] = {
        VOLUME_NAMES[bn::min(4, int(settings.music_volume))],
        VOLUME_NAMES[bn::min(4, int(settings.sfx_volume))],
        FLASH_NAMES[bn::min(2, int(settings.flash_mode))],
        TEXT_SPEED_NAMES[bn::min(3, int(settings.text_speed))],
        PALETTE_NAMES[bn::min(3, int(settings.window_palette))]
    };
    for(int item = 0; item < 5; ++item)
    {
        int y = 5 + item * 2;
        TextRenderer::draw_text(_menu_cells, 2, y, item == _frontend.cursor() ? ">" : " ");
        TextRenderer::draw_text(_menu_cells, 4, y, OPTION_NAMES[item]);
        TextRenderer::draw_text(_menu_cells, 20, y, "<");
        TextRenderer::draw_clipped(_menu_cells, 21, y, values[item], 7);
        TextRenderer::draw_text(_menu_cells, 28, y, ">");
    }
    TextRenderer::draw_centered(_menu_cells, 18, "LEFT/RIGHT:CHANGE  B:SAVE");
    if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
}

void GameManager::draw_save_recovery()
{
    TextRenderer::clear_visible(_menu_cells);
    if(_save_manager.profile_damaged(_selected_profile))
    {
        TextRenderer::draw_centered(_menu_cells, 4, "PROFILE DATA DAMAGED");
        TextRenderer::draw_centered(_menu_cells, 7, "THIS SLOT CANNOT LOAD");
        draw_cursor_line(_menu_cells, 11, _frontend.cursor() == 0, "RESET SLOT");
        draw_cursor_line(_menu_cells, 13, _frontend.cursor() == 1, "KEEP DATA / BACK");
    }
    else
    {
        TextRenderer::draw_centered(_menu_cells, 5, "SAVE BACKUP RESTORED");
        TextRenderer::draw_centered(_menu_cells, 8, "YOUR PROFILES ARE SAFE");
        TextRenderer::draw_centered(_menu_cells, 13, "PRESS A");
    }
    if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
}

void GameManager::draw_confirm_dialog()
{
    TextRenderer::clear_visible(_menu_cells);
    const char* prompt = "ARE YOU SURE?";
    switch(_confirm_action)
    {
        case ConfirmAction::CREATE_PROFILE: prompt = "CREATE THIS PROFILE?"; break;
        case ConfirmAction::DELETE_PROFILE: prompt = "DELETE THIS PROFILE?"; break;
        case ConfirmAction::RESTART_ROOM: prompt = "RESTART THIS ROOM?"; break;
        case ConfirmAction::EXIT_TO_ROOM_SELECT: prompt = "LEAVE THIS RUN?"; break;
        default: break;
    }
    TextRenderer::draw_panel(_menu_cells, 3, 5, 24, 10);
    TextRenderer::draw_centered(_menu_cells, 7, prompt);
    if(_confirm_action == ConfirmAction::CREATE_PROFILE)
    {
        TextRenderer::draw_centered(_menu_cells, 9, _profile_name);
    }
    draw_cursor_line(_menu_cells, 11, _frontend.cursor() == 0, "YES");
    draw_cursor_line(_menu_cells, 13, _frontend.cursor() == 1, "NO");
    if(_menu_bg_map) _menu_bg_map->reload_cells_ref();
}

void GameManager::load_level(int level_index)
{
    _world_bg.reset();
    _current_level = bn::max(0, bn::min(level_index, TOTAL_LEVELS - 1));
    _death_count = 0;
    _gravity_flipped = false;
    const LevelDef& def = get_level(_current_level);
    _current_chamber = def.start_chamber;

    int world = _current_level / LEVELS_PER_WORLD;
    if(world == 0) _world_bg = bn::regular_bg_items::world1_bg.create_bg(0, 0);
    else if(world == 1) _world_bg = bn::regular_bg_items::world2_bg.create_bg(0, 0);
    else if(world == 2) _world_bg = bn::regular_bg_items::world3_bg.create_bg(0, 0);
    else if(world == 3) _world_bg = bn::regular_bg_items::world4_bg.create_bg(0, 0);
    else if(world == 4) _world_bg = bn::regular_bg_items::world5_bg.create_bg(0, 0);
    
    if(_world_bg)
    {
        _world_bg->set_priority(3);
    }

    const ChamberDef& chamber = def.chambers[_current_chamber];
    load_chamber(_current_chamber, chamber.spawn_x, chamber.spawn_y, false);
}

void GameManager::restart_level()
{
    _gravity_flipped = false;
    const LevelDef& def = get_level(_current_level);
    _current_chamber = def.start_chamber;
    const ChamberDef& chamber = def.chambers[_current_chamber];
    load_chamber(_current_chamber, chamber.spawn_x, chamber.spawn_y, false);
}

void GameManager::load_chamber(uint8_t chamber_index, uint8_t spawn_x, uint8_t spawn_y,
                               bool preserve_player)
{
    const LevelDef& def = get_level(_current_level);
    if(chamber_index >= def.chamber_count)
    {
        return;
    }

    _current_chamber = chamber_index;
    bool chamber_gravity = preserve_player ? _gravity_flipped : false;
    _gravity_flipped = chamber_gravity;
    const ChamberDef& chamber = def.chambers[chamber_index];
    _trap_manager.clear();
    _level.load(chamber, def.world_id);
    _level.set_camera(_camera.camera_ptr());

    bn::fixed px = SCREEN_LEFT + spawn_x * TILE_SIZE + TILE_SIZE / 2;
    bn::fixed py = _level.settled_spawn_y(
        spawn_x, spawn_y, chamber_gravity,
        physics::PLAYER_HALF_WIDTH, physics::PLAYER_HALF_HEIGHT);
    if(_player)
    {
        if(preserve_player)
        {
            _player->relocate(px, py);
        }
        else
        {
            _player->reset(px, py);
        }
    }
    else
    {
        _player.emplace(px, py);
    }
    _player->set_camera(_camera.camera_ptr());
    _trap_manager.load(chamber, chamber_gravity);
}

void GameManager::transition_chamber(const ChamberTransition& transition)
{
    const LevelDef& def = get_level(_current_level);
    if(transition.target_chamber >= def.chamber_count)
    {
        return;
    }

    fade_out(6);
    load_chamber(transition.target_chamber, transition.target_x, transition.target_y, true);
    fade_in(6);
}

int GameManager::get_available_skills(SkillType* out_skills, int max_count)
{
    int count = 0;
    int world = _current_level / LEVELS_PER_WORLD;
    if(world >= 1 && count < max_count)
    {
        out_skills[count++] = SkillType::SPRING_BOOTS;
        if(count < max_count) out_skills[count++] = SkillType::DASH_CLOAK;
    }
    if(world >= 2 && count < max_count)
    {
        out_skills[count++] = SkillType::SHRINK_POTION;
    }
    return count;
}

void GameManager::apply_settings()
{
    const GlobalSettings& settings = _save_manager.settings();
    if(bn::music::playing_item())
    {
        bn::music::set_volume(bn::fixed(bn::min(4, int(settings.music_volume))) / 4);
    }
    bn::sound::set_master_volume(bn::fixed(bn::min(4, int(settings.sfx_volume))) / 4);
    _screen_fx.set_flash_mode(bn::min(2, int(settings.flash_mode)));
    if(_menu_bg)
    {
        constexpr bn::color accents[] = {
            bn::color(24, 19, 8),
            bn::color(18, 12, 28),
            bn::color(7, 23, 16),
            bn::color(17, 18, 20)
        };
        bn::bg_palette_ptr palette = _menu_bg->palette();
        palette.set_color(3, accents[bn::min(3, int(settings.window_palette))]);
    }
}

void GameManager::fade_out(int frames)
{
    _screen_fx.fade_out(frames);
}

void GameManager::fade_in(int frames, bn::fixed target_alpha)
{
    _screen_fx.fade_in(frames, target_alpha);
}

} // namespace game
