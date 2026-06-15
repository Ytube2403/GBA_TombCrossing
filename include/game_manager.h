#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "bn_optional.h"
#include "bn_sprite_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_map_item.h"
#include "bn_regular_bg_map_cell.h"
#include "bn_regular_bg_map_ptr.h"
#include "constants.h"
#include "player.h"
#include "trap_manager.h"
#include "level.h"
#include "camera.h"
#include "screen_fx.h"
#include "hud.h"
#include "skill.h"
#include "save_manager.h"
#include "frontend_controller.h"
#include "text_reveal.h"

namespace game {

// ─────────────────────────────────────────────────────────────────────────────
// GameManager — top-level orchestrator
// ─────────────────────────────────────────────────────────────────────────────
class GameManager {
public:
    GameManager();
    void run(); // main loop — never returns
    int current_level_index() const { return _current_level; }
    GameState state() const { return _state; }

private:
    GameState _state;
    int       _current_level;  // 0–99
    int       _frame;          // global frame counter
    int       _death_count;    // deaths on current level
    uint8_t   _current_chamber;
    bool      _gravity_flipped;
    GameState _return_state;

    bn::optional<Player>     _player;
    TrapManager              _trap_manager;
    bn::optional<bn::regular_bg_ptr> _bg;
    bn::optional<bn::regular_bg_ptr> _world_bg;
    Level                    _level;
    Camera                   _camera;
    ScreenFx                 _screen_fx;
    HUD                      _hud;
    alignas(int) bn::regular_bg_map_cell _menu_cells[32 * 32];
    bn::regular_bg_map_item  _menu_map_item;
    bn::optional<bn::regular_bg_ptr> _menu_bg;
    bn::optional<bn::regular_bg_map_ptr> _menu_bg_map;
    bn::optional<bn::sprite_ptr> _title_mummy_sprite;
    bn::optional<bn::sprite_ptr> _title_logo_sprite;
    bn::optional<bn::sprite_ptr> _world_card_sprite;
    bn::optional<bn::sprite_ptr> _frontend_art_sprite;
    bool                     _first_transition;
    SaveManager              _save_manager;
    SaveLoadResult           _load_result;
    FrontendController       _frontend;
    TextReveal               _text_reveal;
    int                      _selected_profile;
    int                      _world_select_idx;
    int                      _room_cursor;
    uint8_t  _skill_cursor;
    SkillType _selected_skill;
    char _profile_name[PROFILE_NAME_LENGTH + 1];
    int _profile_name_length;
    bool _result_first_clear;
    bool _result_action_buffered;
    int _world_result_idx;

    enum class ConfirmAction : uint8_t {
        NONE,
        CREATE_PROFILE,
        DELETE_PROFILE,
        RESTART_ROOM,
        EXIT_TO_ROOM_SELECT
    };
    ConfirmAction _confirm_action;
    int _cheat_seq_idx = 0;  // Konami code tracker: ↑↑↓↓←→←→BA

    void update_boot();
    void update_title();
    void update_profile_select();
    void update_profile_create();
    void update_profile_menu();
    void update_world_select();
    void update_room_select();
    void update_playing();
    void update_paused();
    void update_dead();
    void update_level_clear();
    void update_world_intro();
    void update_game_complete();
    void update_world_result();
    void update_ending();
    void update_credits();
    void update_stats();
    void update_options();
    void update_save_recovery();
    void update_confirm_dialog();

    void transition_to(GameState new_state, bool reload = false, bool restart = false);
    void load_level(int level_index);
    void restart_level();
    void load_chamber(uint8_t chamber_index, uint8_t spawn_x, uint8_t spawn_y,
                      bool preserve_player);
    void transition_chamber(const ChamberTransition& transition);
    void draw_current_state();
    void draw_boot();
    void draw_title();
    void draw_profile_select();
    void draw_profile_create();
    void draw_profile_menu();
    void draw_world_select_static();
    void draw_room_select();
    void draw_world_intro_static();
    void update_skill_select();
    void draw_skill_select_menu();
    void draw_pause_menu();
    void draw_dead();
    void draw_room_result();
    void draw_world_result();
    void draw_ending();
    void draw_credits();
    void draw_game_complete();
    void draw_stats();
    void draw_options();
    void draw_save_recovery();
    void draw_confirm_dialog();
    int  get_available_skills(SkillType* out_skills, int max_count);
    void open_confirm(ConfirmAction action, GameState return_state);
    void continue_profile();
    void start_selected_room();
    void advance_after_result();
    void apply_settings();
    const ProfileData* active_profile() const;
    ProfileData* active_profile();

    void fade_out(int frames = 16);
    void fade_in(int frames = 16, bn::fixed target_alpha = 0);
};

} // namespace game

extern "C" {
    extern game::GameManager* global_game_manager;
}

#endif // GAME_MANAGER_H
