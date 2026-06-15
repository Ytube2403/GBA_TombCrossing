# Tomb Crossing — Design Language

> *"A tomb should feel alive even when everything inside is dead."*

---

## 1. Tầm nhìn thiết kế (Design Vision)

### 1.1 Câu hỏi cốt lõi

Tomb Crossing không chỉ là một puzzle platformer — nó là hành trình của xác ướp **Kha-Men** xuyên qua 5 nền văn minh cổ đại. Mỗi pixel trên màn hình 240×160 phải trả lời được câu hỏi: *"Người chơi đang ở đâu trong lịch sử?"*

### 1.2 Phong cách tổng thể

| Yếu tố | Định hướng | Nguồn cảm hứng |
|---------|-----------|----------------|
| **Atmosphere** | Gothic cổ đại, bí ẩn | Castlevania: Aria of Sorrow |
| **Personality** | Hài hước đen, vui nhộn qua cái chết | Level Devil, Wario Land 4 |
| **Readability** | Rõ ràng tuyệt đối khi gameplay nhanh | Mega Man Zero, Sonic Advance |
| **Polish** | Mỗi tile là một tác phẩm nhỏ | The Legend of Zelda: Minish Cap |
| **Identity** | Egyptian-first, 5 nền văn minh riêng biệt | Adventure Island (tham khảo ảnh) |

### 1.3 Nguyên tắc thiết kế ("Pillars")

1. **Death is Fun** — Chết phải có phản hồi thị giác thú vị, không gây bực bội. Flash trắng ngắn, camera shake nhẹ, sprite vỡ ra.
2. **World Identity in 3 Seconds** — Người chơi phải nhận ra world nào trong 3 giây đầu tiên chỉ bằng palette + tile pattern.
3. **HUD là vô hình** — HUD phải tối thiểu đến mức người chơi quên nó tồn tại. Chỉ hiện thông tin cần thiết.
4. **Foreground Pops, Background Breathes** — Gameplay elements nổi bật, background chỉ gợi không khí.

---

## 2. Hệ thống màu sắc (Color System)

### 2.1 Triết lý màu GBA

GBA sử dụng 15-bit color (5-5-5 RGB = 32.768 màu), mỗi palette 16 màu. Game hiện có 16 BG palette + 16 sprite palette. **Chiến lược: dùng ít màu nhưng đúng màu.**

#### Quy tắc vàng:
- **Hue Shifting khi shading**: Shadow dịch về cool (xanh/tím), highlight dịch về warm (vàng/cam). KHÔNG chỉ giảm brightness.
- **Saturation cao cho foreground**, thấp cho background — tạo depth tự nhiên.
- **Outline 1px tối** (không phải đen tuyệt đối, mà là dark shade của hue gần nhất) cho mọi sprite gameplay.

### 2.2 Master Palette — Thương hiệu Tomb Crossing

Dưới đây là "signature colors" xuyên suốt game, xuất hiện ở mọi world:

```
┌─────────────────────────────────────────────────────────────────┐
│  BRAND COLORS (luôn có trong mọi palette)                      │
│                                                                 │
│  Bandage White   #EDE0C8  ███  Kha-Men bandage, text highlight │
│  Eye Gold        #FFD700  ███  Kha-Men eyes, gems, emphasis    │
│  Bone Dark       #8B7355  ███  Kha-Men wrap, outlines          │
│  Void Black      #0A0A14  ███  Deep shadow, BG base            │
│  Blood Red       #C83030  ███  Death, spikes, danger            │
│  Safe Green      #30C848  ███  Exit door, cleared check         │
└─────────────────────────────────────────────────────────────────┘
```

### 2.3 Per-World Palette

Mỗi world có 1 palette chính (BG + tiles) và 1 accent palette (traps + decorative). Dưới đây là color ramp cho từng world (4-5 shade mỗi hue):

#### World 1 — The Sands of Ra (Ai Cập)
```
  Palette Mood: Nóng, khô, cát sa mạc dưới ánh đuốc
  ┌────────────────────────────────────────────────┐
  │ Sand Ramp:    #1A0A00 → #8B7355 → #C4A35A → #EDE0C8
  │ Torch Accent: #1A0800 → #C04000 → #FF6B35 → #FFD700
  │ Stone Cool:   #1A1420 → #3D3040 → #5C4860 → #8B7888
  └────────────────────────────────────────────────┘
  BG tones: Deep brown-black (#1A0A00), sandy mid (#8B7355)
  Decorative: Hieroglyph gold (#FFD700), torch orange (#FF6B35)
  Danger: Spike gold-brown, sand sinkhole amber
```

#### World 2 — The Royal Crypts (Gothic)
```
  Palette Mood: Tối, sang trọng, gold trên purple
  ┌────────────────────────────────────────────────┐
  │ Royal Ramp:   #0A0020 → #2D0A3E → #5C2080 → #8040B0
  │ Gold Accent:  #3D2800 → #8B6800 → #C4A000 → #FFD700
  │ Bone Ramp:    #504830 → #8B8060 → #C4B890 → #F5F0DC
  └────────────────────────────────────────────────┘
  BG tones: Deep purple-black (#0A0020), moody violet (#2D0A3E)
  Decorative: Gold trim (#FFD700), candle warm (#C4A000)
  Danger: Pendulum silver (#8B8B8B), axe glint (#F0F0FF)
```

#### World 3 — Terracotta Labyrinth (Qin Dynasty)
```
  Palette Mood: Đất nung, ngọc bích, imperial weight
  ┌────────────────────────────────────────────────┐
  │ Terra Ramp:   #1A0800 → #602000 → #C04000 → #E07030
  │ Jade Accent:  #001A10 → #005030 → #00A86B → #40D090
  │ Ink Ramp:     #0A0A0A → #1A1A1A → #303030 → #505050
  └────────────────────────────────────────────────┘
  BG tones: Charcoal black (#0A0A0A), terracotta dark (#602000)
  Decorative: Jade green (#00A86B), red lacquer (#C04000)
  Danger: Soldier terracotta (#C04000), pressure plate jade (#40D090)
```

#### World 4 — Temple of Xibalba (Aztec)
```
  Palette Mood: Nóng, nguy hiểm, lava glow
  ┌────────────────────────────────────────────────┐
  │ Stone Ramp:   #101820 → #304050 → #607D8B → #90A0A8
  │ Lava Ramp:    #1A0000 → #801000 → #FF3000 → #FF6B35
  │ Jungle Ramp:  #001A00 → #0A3010 → #1B5E20 → #308040
  └────────────────────────────────────────────────┘
  BG tones: Dark teal-grey (#101820), mossy stone (#304050)
  Decorative: Jaguar gold (#FFD700), vine green (#308040)
  Danger: Lava orange-red (#FF3000), dart frog poison green (#40FF40)
```

#### World 5 — The Warrior's Rest (Kofun)
```
  Palette Mood: Tĩnh lặng, gỗ cũ, đèn đá
  ┌────────────────────────────────────────────────┐
  │ Wood Ramp:    #100800 → #2C1810 → #503828 → #785840
  │ Stone Ramp:   #303030 → #5C5850 → #8B8680 → #B8B0A8
  │ Bamboo Ramp:  #1A2800 → #3D5010 → #6B8E23 → #90B040
  └────────────────────────────────────────────────┘
  BG tones: Dark wood (#100800), paper warm (#B8B0A8)
  Decorative: Lantern amber (#FFB040), bamboo green (#6B8E23)
  Danger: Samurai steel (#607080), paper tear white (#F0E8D8)
```

---

## 3. Typography & Biểu tượng (Font System)

### 3.1 Thiết kế font

Game sử dụng font bitmap 8×8 custom. **Hiện tại font chỉ là functional — cần nâng cấp thành một phần của identity.**

#### Quy tắc font mới:
| Thuộc tính | Giá trị |
|-----------|--------|
| Kích thước | 8×8 px per glyph, 95 ASCII chars |
| Outline | 1px dark outline cho readability trên mọi BG |
| Weight | Regular (gameplay), Bold-feel cho title (wider strokes) |
| Màu chính | Bandage White (#EDE0C8) — text thường |
| Màu nhấn | Eye Gold (#FFD700) — số liệu, highlight |
| Màu mờ | Bone mid (#8B8060) — text phụ, disabled |
| Màu nguy hiểm | Blood Red (#C83030) — death count, warning |

#### Glyph đặc biệt (custom tiles trong hud_tiles.bmp):

| Code | Tile | Ý nghĩa | Thiết kế |
|------|------|---------|---------|
| `\x01` | 96 | 💀 Skull | Kha-Men skull, không generic — có bandage wrap |
| `\x02` | 97 | ↑ Gravity Normal | Mũi tên lên, viền gold |
| `\x03` | 98 | ↓ Gravity Flipped | Mũi tên xuống, viền đỏ (nguy hiểm!) |
| `\x04` | 99 | 🔒 Lock | Scarab lock, không padlock generic |
| `\x05` | 100 | ✓ Cleared | Ankh cross — biểu tượng sự sống |
| `\x06` | 101 | ▶ Menu Cursor | Mũi tên với motif pyramid nhỏ |
| `\x07` | 102 | 👤 Profile | Mummy face icon |
| `\x10-\x15` | 104-109 | Panel border | Viền có motif hieroglyph |

> **Học từ Adventure Island**: Title screen dùng font lớn, stylized, gradient gold. "TOMB CROSSING" cần có treatment tương tự — không chỉ là text 8×8.

---

## 4. Thiết kế UI theo màn hình (Screen-by-Screen UI Design)

### 4.1 Boot Screen — Ấn tượng đầu tiên

**Vấn đề hiện tại**: `draw_boot()` chỉ vẽ panel + text "TOMB CROSSING" + "A BUTANO GBA ADVENTURE". Nhàm chán.

**Thiết kế mới:**
```
┌──────────────────────────────────┐
│         (dark fade-in)           │
│                                  │
│        ⊹ QUANDH TEAM ⊹          │  ← studio logo, 60 frame
│                                  │
│         (cross-fade)             │
│                                  │
│     ⊹ POWERED BY BUTANO ⊹       │  ← engine credit, 45 frame
│                                  │
└──────────────────────────────────┘
```
- Nền đen hoàn toàn, text fade in/out
- Tổng: ~2 giây, skipable bằng START/A

### 4.2 Title Screen — Bộ mặt của game

**Vấn đề hiện tại**: Text "TOMB CROSSING" nhỏ 8×8, mummy sprite bobbing ở giữa, "PRESS START" nhấp nháy. Thiếu impact.

**Thiết kế mới** (học từ Adventure Island title):
```
┌──────────────────────────────────┐  y=0
│                                  │
│    ████████████████████████      │  ← Logo sprite lớn (64×16 hoặc
│    ██ TOMB  CROSSING  ██        │     2×32×16 sprites ghép)
│    ████████████████████████      │  ← Gold gradient + dark outline
│                                  │
│        ╔══════════╗              │
│        ║  🧟‍♂️ Kha-Men  ║         │  ← Sprite idle animation
│        ║  bobbing  ║             │     Đuốc 2 bên flicker
│        ╚══════════╝              │
│                                  │
│     「100 ROOMS. ONE ESCAPE.」   │  ← Tagline, Bone mid color
│                                  │
│        ▸ PRESS START ◂           │  ← Nhấp nháy 1Hz, gold
│                                  │
│  © 2026 ANTIGRAVITY              │  ← Font nhỏ, mờ
└──────────────────────────────────┘  y=19
```

**Key elements:**
- **Logo lớn** bằng sprite (không phải text tile) — gradient gold/brown như Adventure Island
- **Kha-Men sprite** bobbing sin-wave (đã có), thêm 2 torch sprites ở 2 bên
- **Background**: Egyptian BG (parallax nếu có) — hiện `graphics/bg/` rỗng, cần tạo
- **"PRESS START"** bằng Eye Gold (#FFD700), nhấp nháy với fade (không on/off cứng)

### 4.3 Profile Select — Rõ ràng & ấm áp

**Vấn đề hiện tại**: 3 panel trắng giống nhau, text trắng, thiếu personality.

**Thiết kế mới:**
```
┌──────────────────────────────────┐
│        SELECT PROFILE            │  ← Title, gold
│                                  │
│  ┌──────────────────────────┐    │
│  │ 🧟 KHA     ROOM 23  47D │    │  ← Active slot: viền gold glow
│  │    WORLD 2  ROYAL CRYPTS │    │     World palette accent color
│  └──────────────────────────┘    │
│  ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐    │
│  │ 🔒 EMPTY SLOT            │    │  ← Dimmed, dashed border
│  └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘    │
│  ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐    │
│  │ 🔒 EMPTY SLOT            │    │
│  └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘    │
│                                  │
│  A:SELECT          B:TITLE       │  ← Footer hint, Bone mid
└──────────────────────────────────┘
```

**Cải thiện:**
- Slot có profile: viền sáng, hiện mummy icon, world accent color
- Slot trống: viền mờ (dashed), text dim
- Slot được chọn: glow effect (2-frame animation, border sáng hơn)
- Damaged slot: viền đỏ nhấp nháy

### 4.4 World Select — Showcase từng nền văn minh

**Vấn đề hiện tại**: Panel text, world card sprite 64×32 nhỏ, thiếu immersive.

**Thiết kế mới** (học từ Wario Land 4 hub):
```
┌──────────────────────────────────┐
│         CHOOSE WORLD             │
│  ┌──────────────────────────┐    │
│  │                          │    │
│  │   ╔════════════════╗     │    │  ← World card lớn, có artwork
│  │ ◀ ║  WORLD 2       ║ ▶  │    │     riêng theo civilization
│  │   ║ ROYAL CRYPTS   ║    │    │
│  │   ║  [Gothic Art]  ║    │    │  ← 64×32 sprite đặc trưng
│  │   ╚════════════════╝     │    │
│  │                          │    │
│  │     07 / 20 CLEARED      │    │  ← Progress bar visual
│  │     ████████░░░░░░░░░    │    │     (không chỉ text)
│  │     NEXT: ROOM 28        │    │
│  └──────────────────────────┘    │
│                                  │
│  A:ROOMS   B:PROFILE   L/R      │
└──────────────────────────────────┘
```

**Mỗi world card cần có:**
- Artwork đặc trưng (scarab / crown / terracotta soldier / jaguar / katana)
- Palette BG đổi theo world (dùng `set_color` trên BG palette)
- Locked world: artwork xám + lock icon overlay
- Coming Soon: artwork mờ + "COMING SOON" badge

### 4.5 Gameplay HUD — Tối thiểu nhưng đẹp

**Vấn đề hiện tại**: HUD chiếm 16px trên cùng, toàn text trắng, thiếu visual hierarchy.

**Thiết kế mới:**
```
Row 0:  ROOM 23        D:0047        [↕]
        ^^^^^^         ^^^^^^        ^^^
        Gold text      Red if >50    Green=normal
                       White if <50  Red=flipped

Row 1 (chỉ khi có skill):
        BOOTS  [B:READY]
        ^^^^^  ^^^^^^^^^
        Gold   Nhấp nháy gold/dim (đã có, giữ nguyên)
```

**Cải thiện chi tiết:**
- "ROOM" label bằng Bone mid, số bằng Bandage White
- Death count "D:" bằng skull icon (\x01) thay text "D"
- Gravity icon: ↑ xanh lá (#30C848) khi normal, ↓ đỏ (#C83030) khi flipped
- **Separator**: 1px horizontal line dưới HUD (dark line, gần như invisible)
- Khi death count vượt 50 trong 1 room → số chuyển đỏ (cảm giác pressure)

### 4.6 Death Screen — Nhanh & có impact

**Vấn đề hiện tại**: "YOU DIED" text + death count + "A/B/START: RETRY". Functional nhưng nhạt.

**Thiết kế mới** (học từ Level Devil):
```
┌──────────────────────────────────┐
│                                  │
│                                  │
│          ☠ YOU DIED ☠           │  ← Large, Bandage White
│                                  │     2-frame shake animation
│        ROOM DEATHS: 004         │  ← Blood Red nếu >10
│                                  │
│                                  │
│        A/B/START: RETRY          │  ← Nhấp nháy subtle
│                                  │
│                                  │
└──────────────────────────────────┘
```

**Cải thiện:**
- "YOU DIED" xuất hiện với 2-frame camera shake text effect (text lệch 1px left-right)
- Toàn bộ screen tint đỏ nhẹ (dùng blending nếu Butano hỗ trợ) — KHÔNG che gameplay
- Death count cao (>10) → text đỏ, thêm icon skull
- **Input guard 12 frame** — đã implement, giữ nguyên
- Tổng thời gian death → control lại: < 500ms

### 4.7 Room Result — Phần thưởng cho effort

**Vấn đề hiện tại**: Text-based result, thiếu celebratory feel.

**Thiết kế mới** (học từ Adventure Island bonus screen):
```
┌──────────────────────────────────┐
│                                  │
│     ✦ ROOM 23 CLEAR ✦           │  ← Gold, celebration feel
│                                  │
│     DEATHS .............. 4     │  ← Dot leader cho readability
│     BEST ................ 2     │
│     WORLD ............. 8/20    │
│                                  │  ← Frontend art sprite (trophy?)
│     ▸ NEXT ROOM                  │
│       RETRY ROOM                 │
│       ROOM SELECT                │
│                                  │
│            💾 SAVED              │
│     A:SELECT     START:NEXT      │
└──────────────────────────────────┘
```

**Cải thiện:**
- "CLEAR" text có animation: slide-in từ trên + flash gold
- Stats xuất hiện tuần tự (stagger 10 frame/line) — như Adventure Island bonus tally
- Death count 0 → "PERFECT!" text gold nhấp nháy → thêm ý nghĩa
- Frontend art sprite (đã có `frontend_art.bmp` frame 2) → dùng trophy/ankh

### 4.8 World Intro — Cinematic mini

**Vấn đề hiện tại**: Panel + text reveal + world card. OK nhưng thiếu drama.

**Thiết kế mới:**
```
┌──────────────────────────────────┐
│  ┌──────────────────────────┐    │
│  │                          │    │
│  │      [World Card Art]    │    │  ← Full-width artwork
│  │                          │    │
│  │   ═══════════════════    │    │  ← Decorative separator
│  │                          │    │
│  │   THE ROYAL CRYPTS       │    │  ← Text reveal, gold
│  │                          │    │
│  │                          │    │
│  │   "DODGE THE ROYAL AXES" │    │  ← Flavor text, italic feel
│  │                          │    │
│  │                          │    │
│  └──────────────────────────┘    │
│                                  │
│  A:CONTINUE          B:ROOMS     │
└──────────────────────────────────┘
```

**Cải thiện:**
- BG palette đổi sang world color (đã có `apply_settings` cho window palette)
- Text reveal giữ nguyên (tốt rồi), thêm SFX tick cho mỗi ký tự
- World title bằng Gold, flavor text bằng Bandage White

---

## 5. Thiết kế nhân vật & sprite (Character Art)

### 5.1 Kha-Men — Nhân vật chính

**Specs hiện tại**: 16×32 px sprite sheet, 8 animation states.

**Ngôn ngữ thiết kế cho Kha-Men:**

| Yếu tố | Quy tắc |
|--------|---------|
| **Silhouette** | Nhận diện ngay ở 16×32: đầu tròn + bandage trails + dáng nhỏ |
| **Bandage** | Off-white (#EDE0C8) là màu chiếm diện tích lớn nhất — brand color |
| **Eyes** | 2 pixel gold (#FFD700) sáng — "linh hồn" của nhân vật |
| **Outline** | Dark wrap (#8B7355) 1px — KHÔNG dùng pure black |
| **Shadow** | Shift sang cool: bandage shadow = #A89878 (brown-cool) |
| **Proportion** | Head:Body = 1:1.5 — chibi feel, dễ thương dù là xác ướp |

**Animation priorities (frame count):**

| State | Frames | Notes |
|-------|--------|-------|
| IDLE | 4 | Bandage phất nhẹ, mắt chớp 1 lần |
| WALK | 6 | Chân nhỏ bước, bandage trail | 
| JUMP_UP | 3 | Arms up, bandage stream |
| FALL | 2 | Arms out, anticipation |
| LAND | 2 | Squash + stretch 1 frame |
| DIE | 8 | **Quan trọng nhất**: vỡ thành bandage pieces, mắt tắt |
| WIN | 4 | Dance/celebrate — personality moment |
| PUSH | 3 | Push against wall — comedy |

> **Học từ Wario Land 4**: Kha-Men cần expressive death. Mỗi cái chết là một "micro-cartoon". Die animation 8 frame phải kể 1 câu chuyện nhỏ.

### 5.2 Enemy/Trap Sprite Guidelines

| Quy tắc | Áp dụng |
|---------|---------|
| **Danger = Warm** | Mọi thứ giết player phải có accent warm (red/orange/gold) |
| **Safe = Cool** | Platform/moving platform dùng cool tones (blue-grey, green) |
| **Readable silhouette** | 16×16 traps phải nhận diện ở khoảng cách 1 screen |
| **2-frame minimum** | Mọi trap ít nhất 2 frame animation (idle + active) |
| **Flash trước khi giết** | Pop spike, sarcophagus: flash warning 4-8 frame trước |

---

## 6. Thiết kế Tile & Environment

### 6.1 Tile Design Philosophy

Mỗi tileset world (128×8 px = 16 tiles × 8×8) phải tuân theo:

```
Tile Anatomy (8×8 grid):
┌────────┐
│ outline │  ← 1px viền tối (not pure black)
│ ┌────┐ │
│ │fill│ │  ← 3-4 shade color ramp
│ │    │ │
│ └────┘ │
│ detail  │  ← 1-2px decorative element
└────────┘
```

#### Quy tắc cho từng loại tile:

| Tile | Quy tắc thiết kế |
|------|------------------|
| **SOLID (1)** | Texture stone/brick theo world. KHÔNG flat color. Cần visible edge/joint lines |
| **SPIKE_UP (2)** | Sharp triangle, gold highlight ở tip, dark base. Phải "nhọn" ngay cả ở 8×8 |
| **SPIKE_DOWN (3)** | Flip vertical của SPIKE_UP |
| **VANISH (5)** | Giống SOLID nhưng có crack pattern — visual hint "sẽ vỡ" |
| **CONVEYOR (6,7)** | Arrow pattern on surface, 2 tiles = animation direction |
| **EXIT (8)** | Scarab door — golden glow, khác biệt rõ mọi thứ khác. Eye-catching nhất trên screen |
| **Decorative (10-14)** | Hieroglyphs, carvings, wall patterns — background depth |

### 6.2 Per-World Tile Identity

| World | Material | Key Pattern | Unique Element |
|-------|----------|-------------|----------------|
| W1 Egypt | Sandstone | Horizontal brick lines | Hieroglyph symbols mỗi 4-5 tile |
| W2 Royal | Polished stone | Large block, gold trim | Stained glass pattern tiles |
| W3 Qin | Terracotta brick | Small square brick | Jade inlay decorative |
| W4 Aztec | Carved stone | Stepped pyramid pattern | Jaguar face carvings |
| W5 Kofun | Wood + stone | Horizontal wood planks | Bamboo vertical accents |

### 6.3 Background Design

**Thiết kế BG cho từng world** (256×256 px, đơn sắc tối giản):

Để đảm bảo tính rõ ràng (readability) tuyệt đối cho phần gạch màn chơi (foreground), toàn bộ phần nền của màn chơi đã được tinh chỉnh thành dạng **màu đơn sắc trầm tối** (flat solid colors), hoàn toàn không có các cột đá hay họa tiết phức tạp gây xao nhãng:

| World | Background Color | RGB / Palette Index |
|-------|-----------|--------|
| W1 Egypt | Màu xanh đen trời đêm sa mạc | `#08040A` (Index 1) |
| W2 Royal | Màu tím đen trầm | `#09040E` (Index 1) |
| W3 Qin | Màu mực đen xám | `#0C0A09` (Index 1) |
| W4 Aztec | Màu xanh đá phiến tối | `#0A0D08` (Index 1) |
| W5 Kofun | Màu lục trầm tối (kèm đom đóm đơn pixel) | `#080C06` (Index 1) |

> **Quy tắc thiết kế**: Phần nền (background) phải ở dạng tối giản nhất có thể để làm nổi bật hoàn toàn các khối gạch vững chắc (foreground) và các chướng ngại vật/bẫy nguy hiểm. Lớp map gạch màn chơi được xếp ở độ ưu tiên hiển thị (priority) là **2**, nằm phía trước lớp nền (độ ưu tiên **3**).

---

## 7. Animation & Transition Language

### 7.1 Transition System

| Transition | Thời gian | Kiểu | Dùng khi |
|-----------|-----------|------|----------|
| **Fade Black** | 8 frame out + 8 frame in | Alpha blend | Chuyển scene thông thường |
| **Fast Cut** | 0 frame | Instant | PLAYING ↔ PAUSED, DEAD → PLAYING |
| **Iris Out** | 12 frame | Circular wipe to black | Level clear → Result (nếu implement được) |
| **Flash White** | 4-10 frame | White overlay | Death, gravity flip |

> **Hiện tại**: Game dùng `fade_out(8)` / `fade_in(8)` cho mọi transition. Giữ nguyên nhưng thêm variation:
> - Death: flash_white(10) → đã có, tốt
> - Level clear: flash_white(4) → giữ nguyên
> - Gravity flip: flash_white(12) + camera shake — đã có, rất tốt

### 7.2 Micro-Animation Catalog

| Element | Animation | Frames | Trigger |
|---------|----------|--------|---------|
| **Menu cursor** | Bob left-right 2px | 4 frame loop | Idle trên menu |
| **"PRESS START"** | Fade in/out (not on/off) | 30 frame cycle | Title idle |
| **Cleared check** | Scale up từ 0 → full | 4 frame | Room marked cleared |
| **Lock icon** | Shake 1px khi attempt | 3 frame | Player chọn locked item |
| **Gold text** | Shimmer highlight sweep | 8 frame | Achievement/completion |
| **Torch** | Flicker 3 brightness levels | 6 frame random | BG decoration |
| **World card** | Subtle parallax on D-pad | Continuous | World select browse |

### 7.3 Screen Shake Specs

| Event | Duration | Amplitude | Decay |
|-------|----------|-----------|-------|
| Player death | 15 frame | 2.0 px | Linear |
| Gravity flip | 15 frame | 2.5 px | Linear |
| Boulder spawn | 8 frame | 1.5 px | Quick |
| Boss clear | 20 frame | 3.0 px | Slow |

---

## 8. Sound Design Language

### 8.1 Âm thanh & Thiết kế thị giác liên kết

Mỗi sound cue phải đi kèm visual feedback:

| Sound | Visual Pair |
|-------|------------|
| `player_death` | Flash white + camera shake + death animation |
| `player_jump` | Squash frame (1f) trước jump |
| `player_land` | Stretch frame (1f) + dust particle nếu còn sprite budget |
| `room_clear` | Door glow animation + screen iris/fade |
| `spike_extend` | Spike highlight + camera micro-shake |
| `gravity_flip` | Full screen flash + camera shake + HUD gravity icon flip |
| `ui_move` | Menu cursor có visual bob |
| `trap_warning` | Trap shake hoặc warning animation |
| `floor_crack` / `floor_break` | Cracked tile state rồi tile biến mất |

### 8.2 Music Policy

`tomb_crossing_theme.mod` phát ở title và frontend. Gameplay chủ động tắt
nhạc và không có ambience để các cue projectile, trap, floor và movement luôn
dễ đọc. Xem `docs/AUDIO.md` để biết cue matrix và priority policy.

---

## 9. Exit Door — Ngôi sao của mỗi room

**Exit door là element quan trọng nhất ngoài player** — nó là mục tiêu, là phần thưởng, là lý do tồn tại của room.

### Thiết kế Exit Door:
```
┌────────────┐
│ ┌────────┐ │   16×16 sprite
│ │ SCARAB │ │   ← Scarab beetle motif
│ │  GOLD  │ │   ← Gold glow (#FFD700)
│ │ ≋≋≋≋≋≋ │ │   ← Shimmer animation 4 frame
│ └────────┘ │
└────────────┘
```

| Property | Value |
|----------|-------|
| Size | 8×16 (hiện tại), nên upgrade lên sprite 16×16 nếu budget cho phép |
| Color | Gold dominant — sáng nhất trên screen (trừ spike flash) |
| Animation | 4-frame shimmer — luôn chuyển động, luôn gọi mời |
| Visibility | Phải nhìn thấy được từ BẤT KỲ đâu trên screen |

---

## 10. Implementation Priority

### Phase 1 — Quick Wins (Không thay đổi code structure)

| Item | Effort | Impact |
|------|--------|--------|
| Cải thiện hud_tiles.bmp palette (thêm gold, red tones) | Low | Medium |
| Cải thiện world_cards.bmp artwork | Medium | High |
| Tạo ít nhất 1 background BMP cho W1 | Medium | **Very High** |
| Cải thiện font readability (outline) | Low | Medium |
| Đổi "PRESS START" sang fade thay on/off | Low | Medium |

### Phase 2 — Visual Identity (Code changes nhỏ)

| Item | Effort | Impact |
|------|--------|--------|
| Title screen logo sprite (thay text tiles) | Medium | **Very High** |
| Đổi HUD death count "D" thành skull icon | Low | Medium |
| Gravity icon đổi màu (green/red) | Low | High |
| Room Result stagger animation | Medium | High |
| World Select palette shift theo world | Low | High |

### Phase 3 — Polish (Code changes đáng kể)

| Item | Effort | Impact |
|------|--------|--------|
| Per-world BG art (5 backgrounds) | High | **Critical** |
| Tileset art upgrade (5 world tilesets) | High | **Critical** |
| Player sprite redraw (professional quality) | High | Very High |
| Trap sprite overhaul | High | High |
| Particle effects (dust, torch sparks) | Medium | Medium |

---

## 11. Tham chiếu thiết kế (Reference Gallery)

### Nên học từ:

| Game | Bài học |
|------|--------|
| **Castlevania: Aria of Sorrow** | Atmosphere qua restraint — ít màu, nhiều mood |
| **Mega Man Zero** | Contrast — nền tối, nhân vật sáng, đọc rõ ngay |
| **Zelda: Minish Cap** | Polish — mỗi tile đều được chăm chút |
| **Wario Land 4** | Personality — death animation kể chuyện |
| **Adventure Island** | Title screen impact — logo lớn, gradient gold |
| **Level Devil** | Death is fun — chết nhanh, retry nhanh, vui |

### Nên tránh:

| Anti-pattern | Tại sao |
|-------------|---------|
| Flat single-color tiles | Trông như placeholder, không có depth |
| Pure black outlines everywhere | Quá harsh, thiếu character |
| Text-only screens | Nhàm chán, thiếu visual interest |
| Same palette mọi world | Mất world identity |
| Overcrowded HUD | Che gameplay, gây stress |

---

## 12. Kết luận

Tomb Crossing có nền tảng gameplay vững chắc với save system và 23 loại trap.
Visual layer tiếp tục được hoàn thiện theo hướng rõ silhouette, nhất quán chủ
đề và dễ đọc trên màn hình GBA.

**3 thay đổi có impact lớn nhất:**

1. 🎨 **Tạo background art cho World 1** — Hiện tại game không có BG, chỉ có solid color. Thêm 1 BG parallax đơn giản (silhouette columns + starry sky) sẽ thay đổi hoàn toàn feel.

2. 🏛️ **Title screen với logo sprite** — Thay "TOMB CROSSING" text 8×8 bằng sprite logo 64×16 gradient gold. Đây là first impression.

3. 🎭 **Per-world palette application** — Code đã có `window_palette` setting. Mở rộng concept này: khi chuyển world, BG palette đổi theo world color ramp. Không cần art mới, chỉ cần palette swap.

> *Một game puzzle platformer hay khiến người chơi muốn chết thêm lần nữa. Thiết kế tốt khiến họ muốn nhìn thêm lần nữa.*

---

## 13. Implementation Status (2026-06-11)

### Completed

- 30 x 20 safe-area layout helpers and aligned frontend/HUD tilemaps.
- Dual-bank save, three profiles, migration/recovery, Room Select, Results, Stats, Options, Ending and Credits flows.
- Gold/red UI palette, outlined font glyphs, skull and gravity HUD icons.
- 64 x 32 title logo sprite and smooth title prompt palette fade.
- 64 x 64 per-world cards with distinct landmarks and World Select palette shifts.
- Staggered Room Result tally with an input guard until actions are visible.
- Deterministic indexed 4bpp asset generators and per-world background art.
- Boot studio/engine cross-fade and standard 8-frame frontend transitions.

### Remaining Polish

- Professional redraw pass for player, tilesets and trap sprites.
- Animated exit-door shimmer and ambient particles.
- Native-hardware visual review for sprite priority, palette pressure and OAM budget on every trap-heavy room.
- Additional visual regression captures for all frontend states.
