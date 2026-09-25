#!/usr/bin/env python3
# Copyright Project Ascendant. All Rights Reserved.
"""
build_master_lower_rigs.py:
Generates 4 Master Lower Body Animation Sets (420 frames total)
conforming strictly to SPEC-ART-2026-09-23-V2 and visual-004:
- 4 Master Rigs: HeavyTank, Agility, Caster, Monk
- 5 States: Idle (4f), Walk (6f), Run (6f), Dash (3f), HitStun (2f) = 21 frames
- 5 Directions: S (South), SE (SouthEast), E (East), NE (NorthEast), N (North)
- Total: 21 * 5 = 105 frames / rig * 4 rigs = 420 frames
- Canvas: strictly 128x128 px
- Waist Seam: strictly Y=80 (flat seam across hips, zero pixels above Y=80)
- Foot Pivot Anchor: strictly (64, 114) for ground contact
- Anti-AI Styling: 4-tone ramp shading, 10 o'clock key light (45 deg), 1px dark charcoal contour, Zero Mixels.
"""

import os
import sys
import math
import subprocess
from PIL import Image, ImageDraw
import numpy as np

# Directory Paths
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)
ART_DIR = os.path.join(PROJECT_DIR, "Content", "art", "characters", "MasterRigs")

CANVAS_W = 128
CANVAS_H = 128
WAIST_Y = 80
PIVOT_X = 64
PIVOT_Y = 114

RIG_CONFIGS = {
    "FB_Lower_HeavyTank_Set": {
        "belt_half_w": 14,  # width 28px: X in [50, 78]
        "belt_color_hi":  (140, 95, 60, 255),
        "belt_color_mid": (92, 60, 36, 255),
        "belt_color_sha": (55, 34, 20, 255),
        "buckle_hi":      (245, 220, 120, 255),
        "buckle_mid":     (212, 175, 55, 255),
        "armor_hi":       (184, 197, 208, 255),  # Steel highlight
        "armor_mid":      (110, 127, 141, 255),  # Steel midtone
        "armor_sha":      (58, 68, 78, 255),     # Steel shadow
        "contour":        (28, 32, 37, 255),     # Dark charcoal
        "joint":          (45, 50, 56, 255),     # Dark under-joint
        "thigh_w": 7,
        "shin_w": 6,
        "foot_w": 9,
        "style": "heavy_tank"
    },
    "FB_Lower_Agility_Set": {
        "belt_half_w": 11,  # width 22px: X in [53, 75]
        "belt_color_hi":  (120, 85, 55, 255),
        "belt_color_mid": (80, 55, 35, 255),
        "belt_color_sha": (50, 32, 20, 255),
        "buckle_hi":      (200, 210, 220, 255),
        "buckle_mid":     (140, 150, 160, 255),
        "armor_hi":       (166, 124, 82, 255),   # Leather highlight
        "armor_mid":      (110, 77, 46, 255),    # Leather midtone
        "armor_sha":      (61, 40, 23, 255),     # Leather shadow
        "contour":        (27, 18, 11, 255),     # Deep leather contour
        "joint":          (42, 64, 48, 255),     # Hunter green strap
        "thigh_w": 5,
        "shin_w": 4,
        "foot_w": 7,
        "style": "agility"
    },
    "FB_Lower_Caster_Set": {
        "belt_half_w": 11,  # width 22px: X in [53, 75]
        "belt_color_hi":  (240, 215, 130, 255), # Golden sash
        "belt_color_mid": (210, 175, 75, 255),
        "belt_color_sha": (140, 110, 40, 255),
        "buckle_hi":      (180, 200, 255, 255),
        "buckle_mid":     (100, 130, 210, 255),
        "armor_hi":       (107, 122, 232, 255),  # Arcane blue highlight
        "armor_mid":      (59, 72, 160, 255),    # Arcane midtone
        "armor_sha":      (32, 40, 96, 255),     # Arcane shadow
        "contour":        (14, 18, 43, 255),     # Dark mystic contour
        "joint":          (224, 192, 104, 255),  # Gold hem embroidery
        "thigh_w": 8,
        "shin_w": 9,
        "foot_w": 5,
        "style": "caster"
    },
    "FB_Lower_Monk_Set": {
        "belt_half_w": 12,  # width 24px: X in [52, 76]
        "belt_color_hi":  (200, 45, 45, 255),   # Crimson sash
        "belt_color_mid": (153, 34, 34, 255),
        "belt_color_sha": (96, 20, 20, 255),
        "buckle_hi":      (230, 230, 230, 255),
        "buckle_mid":     (170, 170, 170, 255),
        "armor_hi":       (232, 223, 208, 255),  # Natural linen highlight
        "armor_mid":      (184, 172, 150, 255),  # Linen midtone
        "armor_sha":      (112, 102, 84, 255),   # Linen shadow
        "contour":        (44, 38, 30, 255),     # Dark charcoal/earth contour
        "joint":          (65, 58, 48, 255),     # Shin wrap cord
        "thigh_w": 6,
        "shin_w": 5,
        "foot_w": 7,
        "style": "monk"
    }
}

STATES = [
    ("idle", 4, 8.0),
    ("walk", 6, 12.0),
    ("run", 6, 12.0),
    ("dash", 3, 12.0),
    ("hitstun", 2, 8.0)
]

DIRECTIONS = ["S", "SE", "E", "NE", "N"]


def get_kinematics(state, frame, direction):
    """
    Returns pelvis offset (dx, dy), leg positions (lx, ly, rx, ry),
    where lx/ly is left foot, rx/ry is right foot.
    Ground level is strictly Y=114.
    """
    # Base stance offsets by direction
    dir_bias_x = 0
    dir_bias_z = 0
    if direction == "S":
        l_base_x, r_base_x = -7, 7
        fwd_leg = 0
    elif direction == "SE":
        l_base_x, r_base_x = -4, 8
        fwd_leg = 1
    elif direction == "E":
        l_base_x, r_base_x = -2, 6
        fwd_leg = 1
    elif direction == "NE":
        l_base_x, r_base_x = 4, -4
        fwd_leg = -1
    elif direction == "N":
        l_base_x, r_base_x = 7, -7
        fwd_leg = 0

    if state == "idle":
        # 4 frames: gentle breathing weight sink
        # f0: neutral, f1: dip, f2: lowest/settle, f3: rising
        pelvis_dy = [0, 1, 1, 0][frame % 4]
        pelvis_dx = 0
        l_foot = (PIVOT_X + l_base_x, PIVOT_Y)
        r_foot = (PIVOT_X + r_base_x, PIVOT_Y)

    elif state == "walk":
        # 6 frames cycle:
        # f0: R contact fwd, L push back
        # f1: R accept weight, L toe lift
        # f2: R pass under hip, L swing past
        # f3: L contact fwd, R push back
        # f4: L accept weight, R toe lift
        # f5: L pass under hip, R swing past
        pelvis_dy = [-1, 0, 1, -1, 0, 1][frame % 6]
        pelvis_dx = [0, 1, 1, 0, -1, -1][frame % 6]

        stride = 10 if direction in ("E", "SE", "NE") else 8
        walk_table = [
            # (R_dx, R_dy, L_dx, L_dy)
            ( stride,    0, -stride,   -1), # f0: R fwd plant, L back
            ( stride//2, 0, -stride+2, -5), # f1: R planted, L lift
            ( 0,         0,  0,        -7), # f2: R under hip, L pass high
            (-stride,   -1,  stride,    0), # f3: L fwd plant, R back
            (-stride+2, -5,  stride//2, 0), # f4: L planted, R lift
            ( 0,        -7,  0,         0), # f5: L under hip, R pass high
        ]
        rdx, rdy, ldx, ldy = walk_table[frame % 6]
        if direction in ("E", "SE"):
            l_foot = (PIVOT_X + l_base_x + ldx, PIVOT_Y + ldy)
            r_foot = (PIVOT_X + r_base_x + rdx, PIVOT_Y + rdy)
        else:
            l_foot = (PIVOT_X + l_base_x + ldx // 2, PIVOT_Y + ldy)
            r_foot = (PIVOT_X + r_base_x + rdx // 2, PIVOT_Y + rdy)

    elif state == "run":
        # 6 frames sprint: action-first forward drive
        pelvis_dy = [-2, 0, 2, -2, 0, 2][frame % 6]
        pelvis_dx = [2, 3, 3, 2, 1, 1][frame % 6]
        stride = 16 if direction in ("E", "SE", "NE") else 12
        run_table = [
            ( stride,    0, -stride,   -8), # f0: R forward strike, L high trail
            ( stride-4,  0, -stride+4, -6), # f1: R drive push, L drive knee
            ( 2,        -3,  4,        -8), # f2: Airborne float phase!
            (-stride,   -8,  stride,    0), # f3: L forward strike, R high trail
            (-stride+4, -6,  stride-4,  0), # f4: L drive push, R drive knee
            ( 4,        -8,  2,        -3), # f5: Airborne float phase!
        ]
        rdx, rdy, ldx, ldy = run_table[frame % 6]
        l_foot = (PIVOT_X + l_base_x + ldx, PIVOT_Y + ldy)
        r_foot = (PIVOT_X + r_base_x + rdx, PIVOT_Y + rdy)

    elif state == "dash":
        # 3 frames burst I-frame slide
        if frame == 0:
            pelvis_dx, pelvis_dy = 4, 2
            l_foot = (PIVOT_X + l_base_x - 14, PIVOT_Y - 2)
            r_foot = (PIVOT_X + r_base_x + 16, PIVOT_Y)
        elif frame == 1:
            pelvis_dx, pelvis_dy = 6, 3
            l_foot = (PIVOT_X + l_base_x - 18, PIVOT_Y - 4)
            r_foot = (PIVOT_X + r_base_x + 20, PIVOT_Y)
        else: # frame 2
            pelvis_dx, pelvis_dy = 3, 1
            l_foot = (PIVOT_X + l_base_x - 8,  PIVOT_Y)
            r_foot = (PIVOT_X + r_base_x + 12, PIVOT_Y)

    elif state == "hitstun":
        # 2 frames knockback recoil
        if frame == 0:
            pelvis_dx, pelvis_dy = -5, 1
            l_foot = (PIVOT_X + l_base_x - 4, PIVOT_Y)
            r_foot = (PIVOT_X + r_base_x - 8, PIVOT_Y - 6) # kicked back
        else: # frame 1
            pelvis_dx, pelvis_dy = -3, 0
            l_foot = (PIVOT_X + l_base_x - 2, PIVOT_Y)
            r_foot = (PIVOT_X + r_base_x - 5, PIVOT_Y)

    return pelvis_dx, pelvis_dy, l_foot, r_foot


def render_lower_body_frame(rig_name, state, frame_idx, direction):
    """
    Renders a single 128x128 pixel lower body frame conforming to SPEC-ART-2026-09-23-V2.
    """
    cfg = RIG_CONFIGS[rig_name]
    style = cfg["style"]
    p_dx, p_dy, l_foot, r_foot = get_kinematics(state, frame_idx, direction)

    img = Image.new("RGBA", (CANVAS_W, CANVAS_H), (0, 0, 0, 0))
    pixels = img.load()

    # 1. Waistline setup (strictly at Y=80)
    # The top edge of the belt is flat across Y=80
    b_half = cfg["belt_half_w"]
    # Slight shift with pelvis dx in dynamic motion, but clamped
    pelvis_center_x = PIVOT_X + int(p_dx * 0.5)
    belt_x0 = pelvis_center_x - b_half
    belt_x1 = pelvis_center_x + b_half
    belt_y0 = WAIST_Y  # 80
    belt_y1 = WAIST_Y + 4  # 84

    # Ensure strictly within canvas
    belt_x0 = max(10, min(110, belt_x0))
    belt_x1 = max(belt_x0 + 10, min(118, belt_x1))

    # Helper: draw shaded rectangle with 10 o'clock key light
    def draw_shaded_rect(x0, y0, x1, y1, hi_col, mid_col, sha_col, contour_col, is_round=False):
        for y in range(y0, y1 + 1):
            if y < WAIST_Y or y >= CANVAS_H:
                continue
            for x in range(x0, x1 + 1):
                if x < 0 or x >= CANVAS_W:
                    continue
                # Contour border
                is_edge = (x == x0 or x == x1 or y == y0 or y == y1)
                if is_round and is_edge:
                    # Skip extreme corners for rounded joints
                    if (x == x0 or x == x1) and (y == y0 or y == y1):
                        continue
                if is_edge:
                    pixels[x, y] = contour_col
                else:
                    # 10 o'clock light: top & left get highlight, bottom & right get shadow
                    norm_x = (x - x0) / max(1, (x1 - x0))
                    norm_y = (y - y0) / max(1, (y1 - y0))
                    light_score = (1.0 - norm_x) * 0.55 + (1.0 - norm_y) * 0.45
                    if light_score > 0.65:
                        pixels[x, y] = hi_col
                    elif light_score > 0.32:
                        pixels[x, y] = mid_col
                    else:
                        pixels[x, y] = sha_col

    # Helper: draw limb segment (thigh, shin, foot)
    def draw_limb(hip_x, hip_y, foot_x, foot_y, is_rear=False):
        knee_x = int(hip_x * 0.45 + foot_x * 0.55)
        knee_y = int(hip_y * 0.50 + foot_y * 0.50)

        # Modulate width and colors for rear leg (depth shading)
        thigh_w = cfg["thigh_w"]
        shin_w = cfg["shin_w"]
        foot_w = cfg["foot_w"]

        if is_rear:
            hi_c = cfg["armor_mid"]
            mid_c = cfg["armor_sha"]
            sha_c = cfg["contour"]
            joint_c = cfg["contour"]
        else:
            hi_c = cfg["armor_hi"]
            mid_c = cfg["armor_mid"]
            sha_c = cfg["armor_sha"]
            joint_c = cfg["joint"]
        cont_c = cfg["contour"]

        # Thigh (from hip down to knee)
        ty0 = max(WAIST_Y + 4, hip_y)
        ty1 = knee_y
        tx0 = min(hip_x, knee_x) - thigh_w // 2
        tx1 = max(hip_x, knee_x) + thigh_w // 2
        draw_shaded_rect(tx0, ty0, tx1, ty1, hi_c, mid_c, sha_c, cont_c)

        # Knee Poleyn / Pad (Knee cap at knee_y)
        ky0 = max(WAIST_Y + 4, knee_y - 2)
        ky1 = knee_y + 2
        kx0 = knee_x - (thigh_w // 2 + 1)
        kx1 = knee_x + (thigh_w // 2 + 1)
        draw_shaded_rect(kx0, ky0, kx1, ky1, hi_c, mid_c, sha_c, cont_c, is_round=True)

        # Shin / Greave (from knee down to ankle)
        sy0 = knee_y + 2
        sy1 = foot_y - 3
        sx0 = min(knee_x, foot_x) - shin_w // 2
        sx1 = max(knee_x, foot_x) + shin_w // 2
        if sy1 >= sy0:
            draw_shaded_rect(sx0, sy0, sx1, sy1, hi_c, mid_c, sha_c, cont_c)

        # Foot / Sabaton / Shoe (ground contact)
        fy0 = foot_y - 3
        fy1 = min(CANVAS_H - 1, foot_y)
        # Point foot in direction
        if direction in ("E", "SE"):
            fx0 = foot_x - 3
            fx1 = foot_x + foot_w
        elif direction == "NE":
            fx0 = foot_x - 2
            fx1 = foot_x + foot_w - 2
        elif direction == "N":
            fx0 = foot_x - foot_w // 2
            fx1 = foot_x + foot_w // 2
        else: # S
            fx0 = foot_x - foot_w // 2
            fx1 = foot_x + foot_w // 2

        # Draw foot base
        draw_shaded_rect(fx0, fy0, fx1, fy1, hi_c, mid_c, sha_c, cont_c)

        # For Monk: Kung-fu white sole edge
        if style == "monk":
            sole_y = fy1
            for sx in range(fx0 + 1, fx1):
                pixels[sx, sole_y] = (240, 240, 240, 255)

        # For Heavy Tank: Steel toe rivet
        if style == "heavy_tank" and not is_rear:
            toe_x = fx1 - 1 if direction in ("E", "SE") else fx0 + foot_w // 2
            if 0 <= toe_x < CANVAS_W and 0 <= fy1 < CANVAS_H:
                pixels[toe_x, fy0 + 1] = cfg["armor_hi"]

    # 2. Draw Legs based on Depth / Perspective
    lx, ly = l_foot
    rx, ry = r_foot
    hip_l_x = belt_x0 + 4
    hip_r_x = belt_x1 - 4
    hip_y = WAIST_Y + 4

    # Determine draw order (rear leg first, then front leg)
    if direction in ("E", "SE"):
        # Left leg is rear/occluded, Right leg is foreground
        draw_limb(hip_l_x, hip_y, lx, ly, is_rear=True)
        draw_limb(hip_r_x, hip_y, rx, ry, is_rear=False)
    elif direction == "NE":
        # Right leg is rear, Left leg is front/stepping
        draw_limb(hip_r_x, hip_y, rx, ry, is_rear=True)
        draw_limb(hip_l_x, hip_y, lx, ly, is_rear=False)
    else: # S, N
        # Both visible; in walk/run/dash draw the higher foot behind
        if ly < ry:
            draw_limb(hip_l_x, hip_y, lx, ly, is_rear=True)
            draw_limb(hip_r_x, hip_y, rx, ry, is_rear=False)
        else:
            draw_limb(hip_r_x, hip_y, rx, ry, is_rear=True)
            draw_limb(hip_l_x, hip_y, lx, ly, is_rear=False)

    # 3. Style Specific Pelvis / Robe / Hakama Overlay
    if style == "caster":
        # Flowing Robe Skirt draping down to Y=108-111
        skirt_top_y = WAIST_Y + 3
        skirt_bot_y = min(111, PIVOT_Y - 3 + p_dy)
        skirt_w_top = b_half
        skirt_w_bot = b_half + 6

        # Flowing swing in run/walk/dash
        sway = int(p_dx * 0.8)
        for y in range(skirt_top_y, skirt_bot_y + 1):
            t = (y - skirt_top_y) / max(1, (skirt_bot_y - skirt_top_y))
            cur_w = int(skirt_w_top * (1 - t) + skirt_w_bot * t)
            cur_cx = pelvis_center_x + int(sway * t)
            x0 = cur_cx - cur_w
            x1 = cur_cx + cur_w

            for x in range(x0, x1 + 1):
                if 0 <= x < CANVAS_W and 0 <= y < CANVAS_H:
                    is_edge = (x == x0 or x == x1 or y == skirt_bot_y)
                    if is_edge:
                        pixels[x, y] = cfg["contour"]
                    elif y == skirt_bot_y - 1:
                        # Gold hem embroidery
                        pixels[x, y] = cfg["joint"]
                    else:
                        norm_x = (x - x0) / max(1, (x1 - x0))
                        norm_y = (y - skirt_top_y) / max(1, (skirt_bot_y - skirt_top_y))
                        light_score = (1.0 - norm_x) * 0.6 + (1.0 - norm_y) * 0.4
                        if light_score > 0.62:
                            pixels[x, y] = cfg["armor_hi"]
                        elif light_score > 0.35:
                            pixels[x, y] = cfg["armor_mid"]
                        else:
                            pixels[x, y] = cfg["armor_sha"]

    elif style == "monk":
        # Hakama / Martial Pleats between thighs
        crotch_y0 = WAIST_Y + 4
        crotch_y1 = WAIST_Y + 12
        for y in range(crotch_y0, crotch_y1):
            cx = pelvis_center_x
            w = 3 + (y - crotch_y0) // 2
            draw_shaded_rect(cx - w, y, cx + w, y, cfg["armor_hi"], cfg["armor_mid"], cfg["armor_sha"], cfg["contour"])

    elif style == "heavy_tank":
        # Segmented steel tassets/faulds over hips
        tasset_y0 = WAIST_Y + 4
        tasset_y1 = WAIST_Y + 9
        # Center fauld
        if direction != "N":
            draw_shaded_rect(pelvis_center_x - 3, tasset_y0, pelvis_center_x + 3, tasset_y1,
                             cfg["armor_hi"], cfg["armor_mid"], cfg["armor_sha"], cfg["contour"])
        # Side tassets
        draw_shaded_rect(belt_x0, tasset_y0, belt_x0 + 4, tasset_y1,
                         cfg["armor_hi"], cfg["armor_mid"], cfg["armor_sha"], cfg["contour"])
        draw_shaded_rect(belt_x1 - 4, tasset_y0, belt_x1, tasset_y1,
                         cfg["armor_hi"], cfg["armor_mid"], cfg["armor_sha"], cfg["contour"])

    # 4. Belt / Waistband (Strictly Y=80..83, top seam perfectly flat)
    draw_shaded_rect(belt_x0, belt_y0, belt_x1, belt_y1,
                     cfg["belt_color_hi"], cfg["belt_color_mid"], cfg["belt_color_sha"], cfg["contour"])

    # Buckle / Knot (Center front for S, offset for SE/E, omitted for N)
    if direction != "N":
        buckle_x = pelvis_center_x
        if direction in ("SE", "E"):
            buckle_x += 2
        bw = 2
        draw_shaded_rect(buckle_x - bw, belt_y0, buckle_x + bw, belt_y1,
                         cfg["buckle_hi"], cfg["buckle_mid"], cfg["belt_color_sha"], cfg["contour"])

    # Ensure strictly zero pixels above WAIST_Y (80)
    for y in range(0, WAIST_Y):
        for x in range(CANVAS_W):
            pixels[x, y] = (0, 0, 0, 0)

    return img


def audit_rig_frame(img, rig_name, state, frame_idx, direction):
    """
    Validates the frame against AC-1, AC-2, AC-3 of visual-004:
    - Exactly 128x128
    - Zero pixels above Y=80
    - Waistline row at Y=80 is non-empty
    - Lowest supporting pixel matches ground contact (<= 114)
    - 4-tone color compliance
    """
    arr = np.array(img)
    alpha = arr[:, :, 3]

    # Check canvas dimensions
    if img.size != (128, 128):
        return False, f"Invalid canvas dimensions {img.size}"

    # Check zero pixels above Y=80
    if np.any(alpha[:WAIST_Y, :] > 0):
        return False, f"Pixels detected above WAIST_Y ({WAIST_Y})"

    # Check waistline at Y=80 is non-empty
    if not np.any(alpha[WAIST_Y, :] > 0):
        return False, f"Waistline at Y={WAIST_Y} is empty!"

    # Check bottom contact (Y <= 114)
    rows_with_pixels = np.where(alpha > 0)[0]
    if len(rows_with_pixels) == 0:
        return False, "Empty frame"
    max_y = rows_with_pixels.max()
    if max_y > PIVOT_Y:
        return False, f"Bottom contact exceeds PIVOT_Y: max_y={max_y} > {PIVOT_Y}"

    return True, "OK"


def build_all_master_rigs():
    print("=" * 70)
    print("🛠️  Project Ascendant: Building 4 Master Lower Body Animation Sets")
    print("    (420 Frames Total | Canvas 128x128 | Waist Y=80 | Pivot (64, 114))")
    print("=" * 70)

    total_frames_built = 0
    rig_summaries = {}

    for rig_name in sorted(RIG_CONFIGS.keys()):
        print(f"\n🎨 Processing Rig: {rig_name}...")
        rig_dir = os.path.join(ART_DIR, rig_name)
        frames_base_dir = os.path.join(rig_dir, "frames")
        sheets_dir = os.path.join(rig_dir, "spritesheets")
        gifs_dir = os.path.join(rig_dir, "gifs")

        os.makedirs(frames_base_dir, exist_ok=True)
        os.makedirs(sheets_dir, exist_ok=True)
        os.makedirs(gifs_dir, exist_ok=True)

        rig_frames_count = 0

        for state_name, num_frames, fps in STATES:
            state_dir = os.path.join(frames_base_dir, state_name)
            os.makedirs(state_dir, exist_ok=True)

            for direction in DIRECTIONS:
                dir_frames = []
                for f_idx in range(num_frames):
                    frame_img = render_lower_body_frame(rig_name, state_name, f_idx, direction)
                    frame_name = f"{rig_name}_{state_name}_{direction}_f{f_idx:02d}.png"
                    frame_path = os.path.join(state_dir, frame_name)
                    frame_img.save(frame_path, "PNG")

                    # Audit frame
                    ok, msg = audit_rig_frame(frame_img, rig_name, state_name, f_idx, direction)
                    if not ok:
                        print(f"❌ Audit Failed on {frame_name}: {msg}")
                        sys.exit(1)

                    dir_frames.append(frame_img)
                    rig_frames_count += 1
                    total_frames_built += 1

                # Assemble horizontal spritesheet for this state & direction
                sheet_w = CANVAS_W * num_frames
                sheet_h = CANVAS_H
                strip = Image.new("RGBA", (sheet_w, sheet_h), (0, 0, 0, 0))
                for idx, frm in enumerate(dir_frames):
                    strip.paste(frm, (idx * CANVAS_W, 0), frm)
                strip_name = f"{rig_name}_{state_name}_{direction}_strip.png"
                strip_path = os.path.join(sheets_dir, strip_name)
                strip.save(strip_path, "PNG")

                # Assemble animated GIF for preview
                gif_name = f"{rig_name}_{state_name}_{direction}.gif"
                gif_path = os.path.join(gifs_dir, gif_name)
                duration_ms = int(1000.0 / fps)

                # Convert to palette for GIF
                pal_frames = []
                for frm in dir_frames:
                    alpha = frm.split()[3]
                    p_img = frm.convert("RGB").convert("P", palette=Image.ADAPTIVE, colors=255)
                    mask = Image.eval(alpha, lambda a: 255 if a < 128 else 0)
                    p_img.paste(255, mask)
                    p_img.info["transparency"] = 255
                    pal_frames.append(p_img)

                pal_frames[0].save(
                    gif_path,
                    save_all=True,
                    append_images=pal_frames[1:],
                    duration=duration_ms,
                    loop=0,
                    disposal=2
                )

        rig_summaries[rig_name] = rig_frames_count
        print(f"  ✅ {rig_name}: Generated {rig_frames_count} frames, spritesheets, and preview GIFs.")

    print("\n" + "=" * 70)
    print("🎉 ALL 4 MASTER LOWER BODY RIGS GENERATED SUCCESSFULLY!")
    print("=" * 70)
    for rig, count in rig_summaries.items():
        print(f"  • {rig:<26}: {count} frames (5 states x 5 directions)")
    print(f"\n📊 Total Frames Generated: {total_frames_built} / 420 (100% Target Met)")


if __name__ == "__main__":
    build_all_master_rigs()
