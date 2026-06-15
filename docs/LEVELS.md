# Level Design Guide — Tomb Crossing

## Overview

Các level được thiết kế trong **Tiled Map Editor** và lưu trong thư mục
`levels/`.

---

## Tiled Setup

### Cài đặt
- Download: https://www.mapeditor.org/
- Version khuyến nghị: 1.10+

### Map Properties
```
Map size:   30 × 18 tiles
Tile size:  8 × 8 pixels
Orientation: Orthogonal
Render order: Right-down
```

### Layer Structure (bắt buộc)

| Layer Name | Type | Mục đích |
|---|---|---|
| `Tiles` | Tile Layer | Tile ID map — solid/floor/decoration |
| `Objects` | Object Layer | Spawn, exit, trap placements |

### Tileset IDs (phải khớp với constants.h)

| ID | Tile | Ghi chú |
|---|---|---|
| 0 | EMPTY | Air / no collision |
| 1 | SOLID | Standard wall/platform |
| 2 | SPIKE_UP | Static floor spike (instant kill) |
| 3 | SPIKE_DOWN | Static ceiling spike |
| 4 | LADDER | Climbable |
| 5 | VANISH | Visually like floor, handled by TrapVanishFloor |
| 6 | CONVEYOR_LEFT | Arrow-left surface |
| 7 | CONVEYOR_RIGHT | Arrow-right surface |
| 8 | EXIT | Exit door (real) |
| 9 | SPAWN | Player start (not rendered) |

---

## Object Layer Convention

Mỗi object trong layer `Objects` phải có:
- **Type / Class**: `spawn`, `exit`, hoặc `trap`

### Spawn Object
- Type: `spawn`
- Đặt tại vị trí tile mà player bắt đầu

### Exit Object
- Type: `exit`
- Đặt tại vị trí exit door tile

### Trap Object
- Type: `trap`
- Custom Properties:

| Property | Type | Mô tả |
|---|---|---|
| `trap_type` | int | TrapType enum value (xem bảng bên dưới) |
| `param_a` | int | Tham số 1 (ý nghĩa phụ thuộc loại trap) |
| `param_b` | int | Tham số 2 |

### Trap Type Reference

| trap_type | TrapType | param_a | param_b |
|---|---|---|---|
| 0 | VANISH_FLOOR | delay frames (30) | respawn frames (0=never) |
| 1 | POP_SPIKE | proximity tiles (2) | active frames (60) |
| 2 | FALLING_BLOCK | drop delay frames (20) | 0 |
| 3 | MOVING_PLATFORM | speed×10 (15) | path length tiles |
| 4 | CONVEYOR | force×10 (5) | 0 |
| 5 | GRAVITY_FLIP | 0=proximity / 1=timer | interval frames |
| 6 | FAKE_DOOR | 0 | 0 |
| 7 | DARK_ROOM | window radius tiles (3) | 0 |
| 8 | INVISIBLE_WALL | 0 | 0 |
| 9 | WIND | force×10 | direction (0=left, 1=right) |
| 10 | SAND_SINKHOLE | sink rate (5) | 0 |
| 11 | BOULDER | speed×10 (20) | trigger dir: 0=from left / 1=from right |
| 12 | SARCOPHAGUS_POP | delay frames | 0 |
| 13 | PENDULUM_AXE | swing period (90) | 0 |
| 14 | ARROW_WALL | fire interval (120) | 0=left / 1=right |
| 15 | TERRACOTTA_SOLDIER | proximity tiles (3) | 0 |
| 16 | PRESSURE_PLATE | target trap index | 0 |
| 17 | DART_FROG | fire interval (180) | 0=left / 1=right |
| 18 | LAVA_RISE | rise rate (2) | peak y tile |
| 19 | CRUMBLE_FLOOR | steps before crumble (2) | respawn frames |
| 20 | BAMBOO_SPIKE | proximity tiles (2) | 0 |
| 21 | SAMURAI_STATUE | delay frames (60) | 0 |
| 22 | PAPER_FLOOR | hold frames (45) | 0 |
| 23 | BOUNCE_BOULDER | speed×10 (20) | 0=from top-left / 1=from top-right |
| 24 | AUTO_BOULDER | speed×10 (20) | 0=from left / 1=from right |

---

## World Themes & Trap Palette

### World 1 — Egyptian (Maps 1–20)
**Introduce**: T01 (Vanish), T02 (Pop Spike), T11 (Sand Sinkhole), T12 (Boulder)
- 1–5: Tutorial. One trap at a time. Obvious telegraphing.
- 6–10: Two trap types. Introduce fake-out timing.
- 11–15: Combo of 2 traps. Player must plan.
- 16–19: Three traps. Tight timings.
- 20: Boss room — use BOULDER + POP_SPIKE rain combo.

### World 2 — Royal (Maps 21–40)
**Introduce**: T13 (Pendulum), T14 (Arrow Wall), T06 (Gravity Flip first appearance!)
- 21–25: Rolling axe timing puzzles.
- 26–30: Arrow walls + conveyor combos.
- 31–35: First gravity flip levels.
- 36–39: Dark room + gravity flip together.
- 40: Boss room.

### World 3 — Qin (Maps 41–60) ⏳ COMING SOON
**Introduce**: T15 (Terracotta Soldier), T16 (Pressure Plate), T07 (Fake Door)
- Heavy use of fake doors and pressure plate chains.
- ⚠️ **Development paused — content is not included in the current release.**

### World 4 — Aztec (Maps 61–80) ⏳ COMING SOON
**Introduce**: T17 (Dart Frog), T18 (Lava Rise), T19 (Crumble Floor)
- Lava-rise time pressure + crumble floor combos.
- ⚠️ **Development paused — content not yet available.**

### World 5 — Kofun (Maps 81–100) ⏳ COMING SOON
**Introduce**: T20 (Bamboo Spike), T21 (Samurai Statue), T22 (Paper Floor)
- All trap types available. Master-level combinations.
- ⚠️ **Development paused — content not yet available.**

---

## Design Rules (Level Devil philosophy)

1. **Không có visual warning rõ ràng** — bẫy phải bất ngờ lần đầu
2. **Pattern learnable** — sau vài lần chết, người chơi phải có thể suy ra
3. **Instant death only** — không có HP, không có hearts
4. **Instant restart** — chết → bấm A → chơi lại ngay
5. **Short rooms** — mỗi level kết thúc trong <30 giây khi biết pattern
6. **One "surprise" per room** — tránh overwhelming trong early levels
7. **Exit visible from spawn** — người chơi thấy target ngay
