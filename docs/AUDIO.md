# Tomb Crossing Audio

## Direction

The title and frontend use `tomb_crossing_theme.mod`. Gameplay has no music or
ambient bed: movement and trap sounds carry all timing information.

All sound effects are original synthesized assets. No external recordings or
third-party sound libraries are used.

## Runtime Budget

- Backend: Butano Maxmod
- Mixing rate: 16 kHz
- Output assets: mono, unsigned 8-bit PCM WAV
- Maximum simultaneous SFX: 4
- Main theme: MOD tracker module, frontend only

## Cue Matrix

| Group | Cues | Runtime policy |
|---|---|---|
| UI | `ui_move`, `ui_confirm`, `ui_back`, `ui_denied` | Lowest priority; short cooldowns |
| Player | `player_jump`, `player_land`, `player_death`, `skill_activate` | Death always outranks movement |
| Progress | `room_clear`, `gravity_flip` | Critical cues; protected under contention |
| Traps | `trap_warning`, `plate_click`, `projectile_fire`, `spike_extend`, `stone_move`, `stone_impact`, `floor_crack`, `floor_break`, `lava_rise` | Hazard cues outrank action and UI |

Arrow and dart launch events share `projectile_fire` with different playback
speeds. Boulder variants share the stone movement and impact vocabulary.

Audio previews are available in `docs/audio-previews/`.
