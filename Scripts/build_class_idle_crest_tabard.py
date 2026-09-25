#!/usr/bin/env python3
# Copyright Project Ascendant. All Rights Reserved.
"""
build_class_idle_crest_tabard.py:
Generates 12 Class Idle Stances, Helm Crest Sockets & Tabard Overlays (360 assets total)
conforming strictly to SPEC-ART-2026-09-23-V2 and visual-006:
- 12 Classes:
  Vanguard, Ranger, Arcanist, Acolyte, Berserker, Shadowblade,
  Elementalist, Templar, VoidBlade, Chronomancer, DragonKnight, GodSlayer
- 3 Asset Categories:
  1. Idle Loops: 12 classes x 4 frames x 5 directions = 240 frames (128x128 px, Waist Y=80, Pivot 64,114)
  2. Helm Crest Sprites: 12 classes x 5 directions = 60 sprites (32x32 px, centered for Socket_HelmCrest 64,40)
  3. Tabard / Sash Sprites: 12 classes x 5 directions = 60 sprites (48x64 px, centered for Socket_Tabard 64,60)
  Total: 240 + 60 + 60 = 360 assets
"""

import os
import sys
import math
from PIL import Image, ImageDraw
import numpy as np

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)
ART_DIR = os.path.join(PROJECT_DIR, "Content", "art", "characters", "ClassIdentity")

CANVAS_W = 128
CANVAS_H = 128
WAIST_Y = 80
PIVOT_X = 64
PIVOT_Y = 114

CLASSES = {
    "Vanguard": {
        "theme": "Steel Guardian",
        "armor_hi": (184, 197, 208, 255), "armor_mid": (110, 127, 141, 255), "armor_sha": (58, 68, 78, 255),
        "accent": (200, 30, 30, 255), # Red plume
        "crest_name": "Crest_Vanguard_RedPlume", "crest_shape": "plume",
        "tabard_name": "Tabard_Vanguard_CrimsonSash", "tabard_color": (180, 25, 25, 255),
        "stance": "shield_guard", "weapon_reach_x": 16, "weapon_angle": 45
    },
    "Ranger": {
        "theme": "Nomad Archer",
        "armor_hi": (166, 124, 82, 255), "armor_mid": (110, 77, 46, 255), "armor_sha": (61, 40, 23, 255),
        "accent": (50, 110, 60, 255), # Forest green
        "crest_name": "Crest_Ranger_HawkFeather", "crest_shape": "feather",
        "tabard_name": "Tabard_Ranger_NomadCloak", "tabard_color": (40, 90, 50, 255),
        "stance": "bow_over_shoulder", "weapon_reach_x": 12, "weapon_angle": -60
    },
    "Arcanist": {
        "theme": "Scholarly Mage",
        "armor_hi": (107, 122, 232, 255), "armor_mid": (59, 72, 160, 255), "armor_sha": (32, 40, 96, 255),
        "accent": (100, 220, 255, 255), # Sapphire crystal
        "crest_name": "Crest_Arcanist_SapphireShard", "crest_shape": "shard",
        "tabard_name": "Tabard_Arcanist_RunicStole", "tabard_color": (60, 40, 130, 255),
        "stance": "staff_upright", "weapon_reach_x": 10, "weapon_angle": -90
    },
    "Acolyte": {
        "theme": "Martial Monk",
        "armor_hi": (232, 223, 208, 255), "armor_mid": (184, 172, 150, 255), "armor_sha": (112, 102, 84, 255),
        "accent": (240, 200, 80, 255), # Golden halo
        "crest_name": "Crest_Acolyte_HaloCirclet", "crest_shape": "halo",
        "tabard_name": "Tabard_Acolyte_HolyCross", "tabard_color": (210, 180, 70, 255),
        "stance": "horse_stance", "weapon_reach_x": 0, "weapon_angle": 0
    },
    "Berserker": {
        "theme": "Raging Savage",
        "armor_hi": (150, 130, 110, 255), "armor_mid": (90, 75, 60, 255), "armor_sha": (50, 40, 30, 255),
        "accent": (160, 40, 20, 255), # Blood red
        "crest_name": "Crest_Berserker_BeastHorns", "crest_shape": "horns",
        "tabard_name": "Tabard_Berserker_WolfPelt", "tabard_color": (100, 80, 70, 255),
        "stance": "slouched_greatsword", "weapon_reach_x": 18, "weapon_angle": -40
    },
    "Shadowblade": {
        "theme": "Stealth Assassin",
        "armor_hi": (100, 90, 95, 255), "armor_mid": (60, 50, 55, 255), "armor_sha": (30, 25, 28, 255),
        "accent": (140, 40, 160, 255), # Shadow purple
        "crest_name": "Crest_Shadowblade_ShadowCowl", "crest_shape": "cowl",
        "tabard_name": "Tabard_Shadowblade_ShadowSash", "tabard_color": (90, 30, 110, 255),
        "stance": "crouched_x_daggers", "weapon_reach_x": 14, "weapon_angle": 25
    },
    "Elementalist": {
        "theme": "Dual Elementalist",
        "armor_hi": (220, 130, 70, 255), "armor_mid": (150, 70, 40, 255), "armor_sha": (80, 35, 20, 255),
        "accent": (80, 200, 255, 255), # Ice & Fire
        "crest_name": "Crest_Elementalist_TriCrown", "crest_shape": "tri_crown",
        "tabard_name": "Tabard_Elementalist_DualCloak", "tabard_color": (180, 60, 30, 255),
        "stance": "levitating_orbs", "weapon_reach_x": 12, "weapon_angle": 0
    },
    "Templar": {
        "theme": "Holy Paladin",
        "armor_hi": (220, 215, 200, 255), "armor_mid": (150, 145, 130, 255), "armor_sha": (85, 80, 70, 255),
        "accent": (245, 215, 60, 255), # Divine Gold
        "crest_name": "Crest_Templar_SteelWings", "crest_shape": "wings",
        "tabard_name": "Tabard_Templar_GoldCross", "tabard_color": (220, 185, 45, 255),
        "stance": "tower_shield_planted", "weapon_reach_x": 16, "weapon_angle": 90
    },
    "VoidBlade": {
        "theme": "Abyssal Duelist",
        "armor_hi": (130, 110, 150, 255), "armor_mid": (75, 60, 95, 255), "armor_sha": (40, 30, 55, 255),
        "accent": (180, 70, 240, 255), # Void Violet
        "crest_name": "Crest_VoidBlade_VoidHorns", "crest_shape": "void_horns",
        "tabard_name": "Tabard_VoidBlade_VoidTatters", "tabard_color": (100, 40, 140, 255),
        "stance": "zigzag_blade_low", "weapon_reach_x": 18, "weapon_angle": 30
    },
    "Chronomancer": {
        "theme": "Time Scholar",
        "armor_hi": (180, 175, 140, 255), "armor_mid": (120, 115, 85, 255), "armor_sha": (65, 60, 45, 255),
        "accent": (220, 190, 80, 255), # Brass pendulum
        "crest_name": "Crest_Chronomancer_TimePendulum", "crest_shape": "pendulum",
        "tabard_name": "Tabard_Chronomancer_HourglassRibbon", "tabard_color": (150, 130, 60, 255),
        "stance": "pendulum_orbit", "weapon_reach_x": 10, "weapon_angle": 45
    },
    "DragonKnight": {
        "theme": "Draconic Warrior",
        "armor_hi": (170, 60, 50, 255), "armor_mid": (110, 35, 30, 255), "armor_sha": (55, 18, 15, 255),
        "accent": (230, 140, 30, 255), # Dragon ember
        "crest_name": "Crest_DragonKnight_DragonHorns", "crest_shape": "dragon_horns",
        "tabard_name": "Tabard_DragonKnight_DragonScales", "tabard_color": (140, 30, 25, 255),
        "stance": "polearm_extended", "weapon_reach_x": 24, "weapon_angle": 15
    },
    "GodSlayer": {
        "theme": "Eternal Wanderer",
        "armor_hi": (90, 95, 100, 255), "armor_mid": (50, 55, 60, 255), "armor_sha": (25, 28, 32, 255),
        "accent": (15, 15, 18, 255), # Void black
        "crest_name": "Crest_GodSlayer_ThornCrown", "crest_shape": "thorn_crown",
        "tabard_name": "Tabard_GodSlayer_AbyssalShroud", "tabard_color": (35, 35, 42, 255),
        "stance": "greatsword_ground_rest", "weapon_reach_x": 12, "weapon_angle": 90
    }
}

DIRECTIONS = ["S", "SE", "E", "NE", "N"]
CONTOUR_COLOR = (24, 24, 28, 255)


def render_crest_sprite(crest_shape, accent_col, direction):
    """
    Renders 32x32 Helm Crest sprite.
    Socket center at (16, 16) inside the 32x32 box, perfectly snapping to (64, 40) on head.
    """
    img = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    pixels = img.load()
    cx, cy = 16, 16

    # Offset crest slightly by direction
    dir_dx = 0 if direction in ("S", "N") else (2 if direction in ("SE", "E") else -2)

    if crest_shape == "plume": # Vanguard red plume
        for y in range(4, 20):
            w = 2 + (20 - y) // 4
            for x in range(cx + dir_dx - w, cx + dir_dx + w + 1):
                pixels[x, y] = accent_col
    elif crest_shape == "feather": # Ranger feather
        for s in range(14):
            x = cx + dir_dx + s // 2
            y = cy - s
            if 0 <= x < 32 and 0 <= y < 32:
                pixels[x, y] = accent_col
                pixels[x - 1, y] = CONTOUR_COLOR
    elif crest_shape == "shard": # Arcanist sapphire shard
        for y in range(6, 18):
            w = 3 - abs(y - 12) // 2
            for x in range(cx + dir_dx - w, cx + dir_dx + w + 1):
                pixels[x, y] = accent_col
    elif crest_shape == "halo": # Acolyte halo circlet
        for deg in range(0, 360, 15):
            rad = math.radians(deg)
            px = int(cx + dir_dx + 6 * math.cos(rad))
            py = int(cy - 2 + 3 * math.sin(rad))
            if 0 <= px < 32 and 0 <= py < 32:
                pixels[px, py] = accent_col
    elif crest_shape == "horns": # Berserker horns
        for d in [-1, 1]:
            for s in range(8):
                px = cx + d * (3 + s)
                py = cy - 2 - s // 2
                if 0 <= px < 32 and 0 <= py < 32:
                    pixels[px, py] = accent_col
    elif crest_shape == "cowl": # Shadowblade cowl
        for y in range(8, 20):
            for x in range(cx - 5, cx + 6):
                pixels[x, y] = accent_col
    elif crest_shape == "tri_crown": # Elementalist 3 gems
        for off in [-4, 0, 4]:
            for dy in [-1, 0, 1]:
                for dx in [-1, 0, 1]:
                    pixels[cx + off + dx, cy - 4 + dy] = accent_col
    elif crest_shape == "wings": # Templar steel wings
        for d in [-1, 1]:
            for s in range(7):
                px = cx + d * (4 + s)
                py = cy - s
                if 0 <= px < 32 and 0 <= py < 32:
                    pixels[px, py] = (220, 220, 220, 255)
    elif crest_shape == "void_horns": # Void horns
        for d in [-1, 1]:
            for s in range(9):
                px = cx + d * (2 + s)
                py = cy - s - (s ** 2) // 10
                if 0 <= px < 32 and 0 <= py < 32:
                    pixels[px, py] = accent_col
    elif crest_shape == "pendulum": # Chronomancer pendulum
        for y in range(4, 16):
            pixels[cx + dir_dx, y] = (220, 190, 80, 255)
        pixels[cx + dir_dx, 16] = (255, 230, 120, 255)
    elif crest_shape == "dragon_horns": # Dragon horns swept back
        for d in [-1, 1]:
            for s in range(10):
                px = cx + d * (3 + s)
                py = cy - s - 2
                if 0 <= px < 32 and 0 <= py < 32:
                    pixels[px, py] = accent_col
    elif crest_shape == "thorn_crown": # Thorn crown
        for x in range(cx - 7, cx + 8):
            pixels[x, cy - 2] = (40, 40, 45, 255)
            if x % 3 == 0:
                pixels[x, cy - 4] = (20, 20, 24, 255)

    return img


def render_tabard_sprite(tabard_col, direction):
    """
    Renders 48x64 Tabard / Sash Overlay sprite.
    Socket center at (24, 32) inside 48x64 box, perfectly snapping to (64, 60) on chest.
    Fits seamlessly inside chest cutout of all 9 armor sets.
    """
    img = Image.new("RGBA", (48, 64), (0, 0, 0, 0))
    pixels = img.load()
    cx = 24

    # Tabard hangs vertically from chest (Y=12) down to hem (Y=52)
    tab_w = 7 if direction == "S" else (5 if direction in ("SE", "NE") else 3)
    tab_y0 = 12
    tab_y1 = 52

    for y in range(tab_y0, tab_y1):
        # Slight flare towards bottom
        t = (y - tab_y0) / (tab_y1 - tab_y0)
        cur_w = int(tab_w + t * 2)
        x0 = cx - cur_w
        x1 = cx + cur_w
        for x in range(x0, x1 + 1):
            if x == x0 or x == x1 or y == tab_y1 - 1:
                pixels[x, y] = CONTOUR_COLOR
            else:
                pixels[x, y] = tabard_col

    # Center crest gold stripe
    if direction in ("S", "SE"):
        for y in range(tab_y0 + 4, tab_y1 - 4):
            pixels[cx, y] = (240, 215, 75, 255)

    return img


def render_class_idle_frame(class_name, frame_idx, direction):
    """
    Renders 128x128 full character Idle Stance for the class.
    Shows the class-unique stance (e.g. Vanguard shield 40%, Ranger bow high, etc.)
    with a 4-frame breathing cycle.
    """
    cfg = CLASSES[class_name]
    img = Image.new("RGBA", (CANVAS_W, CANVAS_H), (0, 0, 0, 0))
    pixels = img.load()

    # Breathing vertical offset: 4 frames cyclic
    breath_dy = [0, -1, -1, 0][frame_idx % 4]

    # Stance specific limb/torso offsets
    stance = cfg["stance"]
    reach_x = cfg["weapon_reach_x"]

    # 1. Lower Body base (Legs & Feet planted at 114)
    lx0, lx1 = PIVOT_X - 10, PIVOT_X - 3
    rx0, rx1 = PIVOT_X + 3,  PIVOT_X + 10
    if stance == "horse_stance": # Acolyte wider stance
        lx0 -= 3; rx1 += 3
    elif stance == "crouched_x_daggers": # Shadowblade lower
        breath_dy += 3

    for y in range(WAIST_Y, PIVOT_Y + 1):
        for x in range(lx0, lx1 + 1):
            is_edge = (x == lx0 or x == lx1 or y == PIVOT_Y)
            pixels[x, y] = CONTOUR_COLOR if is_edge else cfg["armor_mid"]
        for x in range(rx0, rx1 + 1):
            is_edge = (x == rx0 or x == rx1 or y == PIVOT_Y)
            pixels[x, y] = CONTOUR_COLOR if is_edge else cfg["armor_mid"]

    # 2. Torso (Y=56..80)
    torso_y0 = 56 + breath_dy
    torso_y1 = WAIST_Y
    tx0 = PIVOT_X - 12
    tx1 = PIVOT_X + 12
    for y in range(torso_y0, torso_y1 + 1):
        for x in range(tx0, tx1 + 1):
            is_edge = (x == tx0 or x == tx1 or y == torso_y0 or y == torso_y1)
            pixels[x, y] = CONTOUR_COLOR if is_edge else cfg["armor_mid"]

    # 3. Tabard overlay onto Torso
    tab_w = 4
    for y in range(torso_y0 + 2, torso_y1):
        for x in range(PIVOT_X - tab_w, PIVOT_X + tab_w + 1):
            pixels[x, y] = cfg["tabard_color"]

    # 4. Head & Helmet (Y=36..54)
    head_cy = 44 + breath_dy
    hx0 = PIVOT_X - 8
    hx1 = PIVOT_X + 8
    hy0 = head_cy - 8
    hy1 = head_cy + 8
    for y in range(hy0, hy1 + 1):
        for x in range(hx0, hx1 + 1):
            is_edge = (x == hx0 or x == hx1 or y == hy0 or y == hy1)
            pixels[x, y] = CONTOUR_COLOR if is_edge else cfg["armor_hi"]

    # Visor slit for S & SE
    if direction in ("S", "SE", "E"):
        for x in range(PIVOT_X - 4, PIVOT_X + 5):
            pixels[x, head_cy] = CONTOUR_COLOR
        pixels[PIVOT_X - 2, head_cy] = (255, 230, 180, 255)
        pixels[PIVOT_X + 2, head_cy] = (255, 230, 180, 255)

    # 5. Helm Crest on top of head
    crest_img = render_crest_sprite(cfg["crest_shape"], cfg["accent"], direction)
    # Paste centered at Socket_HelmCrest (64, 40 + breath_dy)
    img.paste(crest_img, (PIVOT_X - 16, 40 + breath_dy - 16), crest_img)

    # 6. Weapon Stance Silhouette Signature (Extending past torso boundary to ensure Silhouette QA Gate Check pass)
    if stance == "shield_guard": # Vanguard shield
        sx0, sx1 = PIVOT_X - 18, PIVOT_X - 3
        sy0, sy1 = 52 + breath_dy, 78 + breath_dy
        for y in range(sy0, sy1):
            for x in range(sx0, sx1):
                is_edge = (x == sx0 or x == sx1 - 1 or y == sy0 or y == sy1 - 1)
                pixels[x, y] = CONTOUR_COLOR if is_edge else cfg["armor_hi"]
    elif stance == "bow_over_shoulder": # Ranger bow high
        bx = PIVOT_X + 16
        for y in range(24 + breath_dy, 68 + breath_dy):
            pixels[bx, y] = (140, 95, 50, 255)
            pixels[bx + 1, y] = (90, 60, 30, 255)
    elif stance == "staff_upright": # Arcanist staff 90 deg
        sx = PIVOT_X + 16
        for y in range(26 + breath_dy, 80 + breath_dy):
            pixels[sx, y] = (190, 160, 70, 255)
        for dy in [-2, -1, 0, 1, 2]:
            for dx in [-2, -1, 0, 1, 2]:
                pixels[sx + dx, 24 + breath_dy + dy] = cfg["accent"]
    elif stance == "horse_stance": # Acolyte braced fists
        for fx in [PIVOT_X - 15, PIVOT_X + 15]:
            for dy in [-2, -1, 0, 1, 2]:
                for dx in [-2, -1, 0, 1, 2]:
                    pixels[fx + dx, 74 + breath_dy + dy] = cfg["armor_hi"]
    elif stance == "slouched_greatsword": # Berserker massive greatsword over shoulder
        gx = PIVOT_X + 18
        for y in range(30 + breath_dy, 75 + breath_dy):
            for w in [-2, -1, 0, 1, 2, 3]:
                pixels[gx + w, y] = (150, 130, 110, 255) if w <= 1 else CONTOUR_COLOR
    elif stance == "crouched_x_daggers": # Shadowblade wide dual daggers
        for dx, dy_off in [(-16, 2), (16, 2)]:
            for s in range(8):
                pixels[PIVOT_X + dx + (s if dx > 0 else -s), 72 + breath_dy + s // 2] = (180, 180, 200, 255)
    elif stance == "levitating_orbs": # Elementalist floating fire/ice orbs
        for ox, ocol in [(PIVOT_X - 18, (240, 80, 30, 255)), (PIVOT_X + 18, (60, 200, 255, 255))]:
            for dy in [-2, -1, 0, 1, 2]:
                for dx in [-2, -1, 0, 1, 2]:
                    pixels[ox + dx, 58 + breath_dy + dy] = ocol
    elif stance == "tower_shield_planted": # Templar massive shield to ground
        tx0, tx1 = PIVOT_X + 8, PIVOT_X + 22
        ty0, ty1 = 48 + breath_dy, PIVOT_Y
        for y in range(ty0, ty1):
            for x in range(tx0, tx1):
                is_edge = (x == tx0 or x == tx1 - 1 or y == ty0 or y == ty1 - 1)
                pixels[x, y] = CONTOUR_COLOR if is_edge else (215, 215, 215, 255)
    elif stance == "zigzag_blade_low": # Void Blade low zigzag blade
        vx = PIVOT_X + 18
        for y in range(54 + breath_dy, 85 + breath_dy):
            zig = (y % 4) - 2
            pixels[vx + zig, y] = cfg["accent"]
            pixels[vx + zig + 1, y] = CONTOUR_COLOR
    elif stance == "pendulum_orbit": # Chronomancer orbiting clock pendulum
        cx_orb = PIVOT_X + 18
        cy_orb = 54 + breath_dy
        for dy in [-3, -2, -1, 0, 1, 2, 3]:
            for dx in [-3, -2, -1, 0, 1, 2, 3]:
                if abs(dx) + abs(dy) <= 4:
                    pixels[cx_orb + dx, cy_orb + dy] = (240, 210, 90, 255)
    elif stance == "polearm_extended": # Dragon Knight long polearm
        px = PIVOT_X + 18
        for y in range(22 + breath_dy, 90 + breath_dy):
            pixels[px, y] = (180, 120, 60, 255)
        # Spearhead
        for dy in range(-6, 1):
            w = (6 + dy) // 2
            for wx in range(px - w, px + w + 1):
                pixels[wx, 22 + breath_dy + dy] = cfg["accent"]
    elif stance == "greatsword_ground_rest": # God Slayer colossal blade in ground
        gx = PIVOT_X + 16
        for y in range(32 + breath_dy, PIVOT_Y):
            for w in [-2, -1, 0, 1, 2]:
                pixels[gx + w, y] = (90, 95, 105, 255) if w <= 0 else CONTOUR_COLOR

    return img


def build_all_class_identity_assets():
    print("=" * 70)
    print("👑  Project Ascendant: Building 12 Class Identity Assets (360 Total)")
    print("    (240 Idle Frames | 60 Helm Crest Sprites | 60 Tabard Sprites)")
    print("=" * 70)

    idle_dir = os.path.join(ART_DIR, "IdleStances")
    crest_dir = os.path.join(ART_DIR, "HelmCrests")
    tabard_dir = os.path.join(ART_DIR, "Tabards")

    os.makedirs(idle_dir, exist_ok=True)
    os.makedirs(crest_dir, exist_ok=True)
    os.makedirs(tabard_dir, exist_ok=True)

    counts = {"idle": 0, "crest": 0, "tabard": 0}

    for class_name, cfg in CLASSES.items():
        print(f"\n✨ Class: {class_name} ({cfg['theme']})")

        # 1. 4-frame Idle Loop x 5 directions = 20 frames
        class_idle_dir = os.path.join(idle_dir, f"FB_{class_name}_Idle")
        os.makedirs(class_idle_dir, exist_ok=True)
        for direction in DIRECTIONS:
            dir_frames = []
            for f_idx in range(4):
                frame_img = render_class_idle_frame(class_name, f_idx, direction)
                fname = f"FB_{class_name}_Idle_{direction}_f{f_idx:02d}.png"
                fpath = os.path.join(class_idle_dir, fname)
                frame_img.save(fpath, "PNG")
                dir_frames.append(frame_img)
                counts["idle"] += 1

            # GIF preview
            gif_name = f"FB_{class_name}_Idle_{direction}.gif"
            gif_path = os.path.join(class_idle_dir, gif_name)
            pal_frames = []
            for frm in dir_frames:
                alpha = frm.split()[3]
                p_img = frm.convert("RGB").convert("P", palette=Image.ADAPTIVE, colors=255)
                mask = Image.eval(alpha, lambda a: 255 if a < 128 else 0)
                p_img.paste(255, mask)
                p_img.info["transparency"] = 255
                pal_frames.append(p_img)
            pal_frames[0].save(gif_path, save_all=True, append_images=pal_frames[1:], duration=125, loop=0, disposal=2)

        # 2. Helm Crest 32x32 x 5 directions = 5 sprites
        for direction in DIRECTIONS:
            crest_img = render_crest_sprite(cfg["crest_shape"], cfg["accent"], direction)
            cname = f"{cfg['crest_name']}_{direction}.png"
            cpath = os.path.join(crest_dir, cname)
            crest_img.save(cpath, "PNG")
            counts["crest"] += 1

        # 3. Tabard / Sash 48x64 x 5 directions = 5 sprites
        for direction in DIRECTIONS:
            tabard_img = render_tabard_sprite(cfg["tabard_color"], direction)
            tname = f"{cfg['tabard_name']}_{direction}.png"
            tpath = os.path.join(tabard_dir, tname)
            tabard_img.save(tpath, "PNG")
            counts["tabard"] += 1

        print(f"  ✅ Built: 20 Idle Frames + 5 Crest Sprites + 5 Tabard Sprites")

    print("\n" + "=" * 70)
    print("🎉 ALL 360 CLASS IDENTITY ASSETS GENERATED!")
    print(f"  • Idle Stance Frames: {counts['idle']} / 240")
    print(f"  • Helm Crest Sprites: {counts['crest']} / 60")
    print(f"  • Tabard Sprites:     {counts['tabard']} / 60")
    print(f"  • Total Assets:       {sum(counts.values())} / 360 (100% Target Met)")
    print("=" * 70)


if __name__ == "__main__":
    build_all_class_identity_assets()
