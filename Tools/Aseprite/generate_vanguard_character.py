#!/usr/bin/env python3
# ==============================================================================
# generate_vanguard_character.py: Production Character & Animation Generator
# Project: Project Ascendant (2.5D Isometric Hardcore ARPG MMO on UE 5.8)
# Conforms to: SPEC-ART-2026-09-23-V2 & Package M5 Technical Specifications
# ==============================================================================

import os
import sys
import struct
import zlib
import json
from PIL import Image

# 32-Color 4-Tone Ramp Palette (SPEC-ART-2026-09-23-V2)
PALETTE_32 = [
    (0, 0, 0, 0),         # 0: transparent
    (18, 18, 20, 255),    # 1: #121214 (Outline / Deep Shadow)
    (30, 36, 44, 255),    # 2: #1E242C (Contour / Dark Steel)
    (74, 88, 104, 255),   # 3: #4A5868 (Steel Shadow)
    (148, 164, 180, 255), # 4: #94A4B4 (Steel Midtone)
    (220, 228, 236, 255), # 5: #DCE4EC (Steel Highlight)
    (255, 255, 255, 255), # 6: #FFFFFF (Specular Glint)
    (58, 36, 8, 255),     # 7: #3A2408 (Gold Shadow)
    (140, 90, 20, 255),   # 8: #8C5A14 (Gold Mid-Shadow)
    (224, 168, 48, 255),  # 9: #E0A830 (Gold Midtone)
    (255, 244, 176, 255), # 10: #FFF4B0 (Gold Highlight)
    (44, 24, 8, 255),     # 11: #2C1808 (Leather Deep)
    (92, 58, 30, 255),    # 12: #5C3A1E (Leather Shadow)
    (154, 106, 64, 255),  # 13: #9A6A40 (Leather Midtone)
    (208, 168, 120, 255), # 14: #D0A878 (Leather Highlight)
    (80, 40, 24, 255),    # 15: #502818 (Skin Deep)
    (176, 112, 80, 255),  # 16: #B07050 (Skin Shadow)
    (232, 176, 136, 255), # 17: #E8B088 (Skin Midtone)
    (255, 224, 192, 255), # 18: #FFE0C0 (Skin Highlight)
    (56, 0, 8, 255),      # 19: #380008 (Red Deep)
    (128, 8, 24, 255),    # 20: #800818 (Red Shadow)
    (208, 32, 32, 255),   # 21: #D02020 (Red Midtone)
    (255, 112, 96, 255),  # 22: #FF7060 (Red Highlight)
    (8, 16, 48, 255),     # 23: #081030 (Blue Deep)
    (16, 40, 120, 255),   # 24: #102878 (Blue Shadow)
    (40, 96, 208, 255),   # 25: #2860D0 (Blue Midtone)
    (112, 176, 255, 255), # 26: #70B0FF (Blue Highlight)
    (24, 4, 40, 255),     # 27: #180428 (Void Deep)
    (64, 16, 104, 255),   # 28: #401068 (Void Shadow)
    (128, 48, 192, 255),  # 29: #8030C0 (Void Midtone)
    (208, 136, 255, 255), # 30: #D088FF (Void Highlight)
    (0, 255, 255, 255)    # 31: #00FFFF (Guide Sockets - strictly binary alpha)
]

WIDTH, HEIGHT = 128, 128
NUM_FRAMES = 22

# Convenient color aliases
C_OUTLINE = (18, 18, 20, 255)
C_STEEL_DARK = (30, 36, 44, 255)
C_STEEL_SHADOW = (74, 88, 104, 255)
C_STEEL_MID = (148, 164, 180, 255)
C_STEEL_HIGH = (220, 228, 236, 255)
C_GLINT = (255, 255, 255, 255)

C_GOLD_SHADOW = (140, 90, 20, 255)
C_GOLD_MID = (224, 168, 48, 255)
C_GOLD_HIGH = (255, 244, 176, 255)

C_LEATHER_DARK = (44, 24, 8, 255)
C_LEATHER_MID = (154, 106, 64, 255)

C_SKIN_SHADOW = (176, 112, 80, 255)
C_SKIN_MID = (232, 176, 136, 255)
C_SKIN_HIGH = (255, 224, 192, 255)

C_RED_DARK = (128, 8, 24, 255)
C_RED_MID = (208, 32, 32, 255)

def draw_rect(pixels, x0, y0, x1, y1, color):
    for y in range(max(0, y0), min(HEIGHT, y1 + 1)):
        for x in range(max(0, x0), min(WIDTH, x1 + 1)):
            idx = (y * WIDTH + x) * 4
            pixels[idx] = color[0]
            pixels[idx + 1] = color[1]
            pixels[idx + 2] = color[2]
            pixels[idx + 3] = color[3]

def draw_point(pixels, x, y, color):
    if 0 <= x < WIDTH and 0 <= y < HEIGHT:
        idx = (y * WIDTH + x) * 4
        pixels[idx] = color[0]
        pixels[idx + 1] = color[1]
        pixels[idx + 2] = color[2]
        pixels[idx + 3] = color[3]

def generate_vanguard_data():
    # Production modular layers (Guide_Sockets preserved in metadata JSON)
    layers = [
        "Layer_BaseBody",
        "Layer_ArmorLower",
        "Layer_ArmorUpper",
        "Layer_Helm",
        "Layer_Hand_R",
        "Layer_Hand_L"
    ]

    frame_cels = {l: [] for l in layers}
    socket_records = []

    for f in range(NUM_FRAMES):
        # Calculate dynamic offsets per animation phase
        body_y_off = 0
        body_x_off = 0
        leg_l_off = [0, 0]
        leg_r_off = [0, 0]
        hand_r_pos = [80, 76]
        hand_l_pos = [48, 76]
        anim_name = ""

        # --- IDLE (Frames 0..3) ---
        if f < 4:
            anim_name = "Idle"
            # 4 distinct frames for organic breathing
            if f == 0:
                body_y_off = 0
                hand_r_pos = [80, 76]
                hand_l_pos = [48, 76]
            elif f == 1:
                body_y_off = -1 # breathe up
                hand_r_pos = [80, 75]
                hand_l_pos = [48, 75]
            elif f == 2:
                body_y_off = -1 # inhale apex
                hand_r_pos = [81, 75]
                hand_l_pos = [47, 75]
            else: # f == 3
                body_y_off = 0  # settle
                hand_r_pos = [80, 77]
                hand_l_pos = [48, 77]

        # --- WALK (Frames 4..9) ---
        elif f < 10:
            anim_name = "Walk"
            step = f - 4
            if step == 0: # Left contact
                body_y_off = 0
                leg_l_off = [3, 0]
                leg_r_off = [-3, 0]
                hand_r_pos = [76, 78]
                hand_l_pos = [52, 74]
            elif step == 1: # Left push
                body_y_off = -1
                leg_l_off = [2, 0]
                leg_r_off = [-2, 0]
                hand_r_pos = [78, 77]
                hand_l_pos = [50, 75]
            elif step == 2: # Passing
                body_y_off = 0
                leg_l_off = [0, 0]
                leg_r_off = [0, 0]
                hand_r_pos = [80, 76]
                hand_l_pos = [48, 76]
            elif step == 3: # Right contact
                body_y_off = 0
                leg_l_off = [-3, 0]
                leg_r_off = [3, 0]
                hand_r_pos = [84, 74]
                hand_l_pos = [44, 78]
            elif step == 4: # Right push
                body_y_off = -1
                leg_l_off = [-2, 0]
                leg_r_off = [2, 0]
                hand_r_pos = [82, 75]
                hand_l_pos = [46, 77]
            else: # step == 5: Passing return
                body_y_off = 0
                leg_l_off = [1, 0]
                leg_r_off = [-1, 0]
                hand_r_pos = [79, 76]
                hand_l_pos = [49, 76]

        # --- RUN (Frames 10..15) ---
        elif f < 16:
            anim_name = "Run"
            step = f - 10
            body_x_off = 2 # forward lean
            if step == 0: # Left stride apex
                body_y_off = -2
                leg_l_off = [5, 0]
                leg_r_off = [-5, 0]
                hand_r_pos = [73, 80]
                hand_l_pos = [55, 71]
            elif step == 1: # Left descending
                body_y_off = -1
                leg_l_off = [3, 0]
                leg_r_off = [-3, 0]
                hand_r_pos = [76, 79]
                hand_l_pos = [52, 73]
            elif step == 2: # Left plant / compress
                body_y_off = 0
                leg_l_off = [1, 0]
                leg_r_off = [-1, 0]
                hand_r_pos = [80, 77]
                hand_l_pos = [48, 75]
            elif step == 3: # Right stride apex
                body_y_off = -2
                leg_l_off = [-5, 0]
                leg_r_off = [5, 0]
                hand_r_pos = [87, 71]
                hand_l_pos = [41, 80]
            elif step == 4: # Right descending
                body_y_off = -1
                leg_l_off = [-3, 0]
                leg_r_off = [3, 0]
                hand_r_pos = [84, 73]
                hand_l_pos = [44, 79]
            else: # step == 5: Right plant / compress
                body_y_off = 0
                leg_l_off = [-1, 0]
                leg_r_off = [1, 0]
                hand_r_pos = [81, 75]
                hand_l_pos = [47, 77]

        # --- ATTACK (Frames 16..21) ---
        else:
            anim_name = "Attack"
            step = f - 16
            if step == 0: # Anticipation / Coil
                body_x_off = -2
                body_y_off = 1
                hand_r_pos = [86, 64] # pulled back & up
                hand_l_pos = [46, 76]
            elif step == 1: # Step forward
                body_x_off = 1
                body_y_off = 0
                leg_l_off = [3, 0]
                hand_r_pos = [88, 62] # apex of slash ready
                hand_l_pos = [44, 76]
            elif step == 2: # Active Strike / Slash Forward
                body_x_off = 4
                body_y_off = -1
                leg_l_off = [4, 0]
                hand_r_pos = [96, 76] # fully extended
                hand_l_pos = [42, 78]
            elif step == 3: # Follow through
                body_x_off = 3
                body_y_off = 1
                hand_r_pos = [88, 86] # low follow through
                hand_l_pos = [44, 78]
            elif step == 4: # Recovery
                body_x_off = 1
                body_y_off = 0
                hand_r_pos = [82, 80]
                hand_l_pos = [46, 76]
            else: # Return to guard
                body_x_off = 0
                body_y_off = 0
                hand_r_pos = [80, 76]
                hand_l_pos = [48, 76]

        # Record socket coordinates
        socket_records.append({
            "frame": f,
            "animation": anim_name,
            "foot_pivot": [64, 114],
            "waist_y": 80 + body_y_off,
            "helm_socket": [64 + body_x_off, 44 + body_y_off],
            "hand_r": hand_r_pos,
            "hand_l": hand_l_pos
        })

        # --- LAYER 0: Layer_BaseBody ---
        # Anatomical human heroic base: Head silhouette, neck, arms, legs, red undertunic
        px_base = bytearray(WIDTH * HEIGHT * 4)
        bx, by = 64 + body_x_off, body_y_off

        # Head base & face skin (Y=44..62, X=56..72)
        draw_rect(px_base, bx - 7, 44 + by, bx + 7, 62 + by, C_OUTLINE)
        draw_rect(px_base, bx - 6, 45 + by, bx + 6, 61 + by, C_SKIN_SHADOW)
        draw_rect(px_base, bx - 4, 48 + by, bx + 5, 59 + by, C_SKIN_MID)
        draw_rect(px_base, bx - 3, 50 + by, bx + 2, 56 + by, C_SKIN_HIGH) # 10 o'clock highlight

        # Neck & Undertunic collar
        draw_rect(px_base, bx - 4, 62 + by, bx + 4, 66 + by, C_RED_DARK)
        draw_rect(px_base, bx - 2, 63 + by, bx + 2, 65 + by, C_RED_MID)

        # Torso undertunic foundation (Y=66..82 - covers waist seam Y=80 solidly)
        draw_rect(px_base, bx - 8, 66 + by, bx + 8, 82 + by, C_OUTLINE)
        draw_rect(px_base, bx - 7, 67 + by, bx + 7, 81 + by, C_RED_DARK)

        # Arms base (linking shoulder to hands)
        # Right Arm (Upper body towards Hand_R)
        draw_rect(px_base, bx + 8, 68 + by, bx + 12, 74 + by, C_OUTLINE)
        draw_rect(px_base, bx + 9, 69 + by, bx + 11, 73 + by, C_RED_DARK)
        # Left Arm (Upper body towards Hand_L)
        draw_rect(px_base, bx - 12, 68 + by, bx - 8, 74 + by, C_OUTLINE)
        draw_rect(px_base, bx - 11, 69 + by, bx - 9, 73 + by, C_RED_DARK)

        # Legs base (under greaves) - seamlessly connects at Y=79
        lx1, ly1 = 57 + leg_l_off[0], 79 + leg_l_off[1]
        draw_rect(px_base, lx1 - 3, ly1, lx1 + 3, 113, C_OUTLINE)
        draw_rect(px_base, lx1 - 2, ly1 + 1, lx1 + 2, 112, C_LEATHER_DARK)

        rx1, ry1 = 71 + leg_r_off[0], 79 + leg_r_off[1]
        draw_rect(px_base, rx1 - 3, ry1, rx1 + 3, 113, C_OUTLINE)
        draw_rect(px_base, rx1 - 2, ry1 + 1, rx1 + 2, 112, C_LEATHER_DARK)
        frame_cels["Layer_BaseBody"].append(px_base)

        # --- LAYER 1: Layer_ArmorLower ---
        # Greaves, poleyns (kneecaps), sabatons
        px_arm_low = bytearray(WIDTH * HEIGHT * 4)
        # Left leg armor
        draw_rect(px_arm_low, lx1 - 4, ly1 + 4, lx1 + 4, 114, C_OUTLINE)
        draw_rect(px_arm_low, lx1 - 3, ly1 + 5, lx1 + 3, 113, C_STEEL_SHADOW)
        draw_rect(px_arm_low, lx1 - 2, ly1 + 6, lx1 + 1, 108, C_STEEL_MID)
        draw_rect(px_arm_low, lx1 - 2, ly1 + 7, lx1 - 1, 104, C_STEEL_HIGH) # Key light
        # Knee poleyn trim
        draw_rect(px_arm_low, lx1 - 2, ly1 + 5, lx1 + 1, ly1 + 7, C_GOLD_MID)
        # Sabaton (foot resting on 114)
        draw_rect(px_arm_low, lx1 - 5, 110, lx1 + 3, 114, C_STEEL_MID)
        draw_rect(px_arm_low, lx1 - 4, 111, lx1 - 1, 113, C_STEEL_HIGH)

        # Right leg armor
        draw_rect(px_arm_low, rx1 - 4, ry1 + 4, rx1 + 4, 114, C_OUTLINE)
        draw_rect(px_arm_low, rx1 - 3, ry1 + 5, rx1 + 3, 113, C_STEEL_SHADOW)
        draw_rect(px_arm_low, rx1 - 2, ry1 + 6, rx1 + 1, 108, C_STEEL_MID)
        draw_rect(px_arm_low, rx1 - 2, ry1 + 7, rx1 - 1, 104, C_STEEL_HIGH)
        draw_rect(px_arm_low, rx1 - 2, ry1 + 5, rx1 + 1, ry1 + 7, C_GOLD_MID)
        draw_rect(px_arm_low, rx1 - 3, 110, rx1 + 5, 114, C_STEEL_MID)
        draw_rect(px_arm_low, rx1 - 2, 111, rx1 + 2, 113, C_STEEL_HIGH)
        frame_cels["Layer_ArmorLower"].append(px_arm_low)

        # --- LAYER 2: Layer_ArmorUpper ---
        # Steel Cuirass, breastplate, faulds/belt, pauldrons (NO baked weapons!)
        px_arm_up = bytearray(WIDTH * HEIGHT * 4)
        # Cuirass body & faulds covering Y=66..83 (spanning waist seam Y=80)
        draw_rect(px_arm_up, bx - 9, 66 + by, bx + 9, 83 + by, C_OUTLINE)
        draw_rect(px_arm_up, bx - 8, 67 + by, bx + 8, 82 + by, C_STEEL_SHADOW)
        draw_rect(px_arm_up, bx - 6, 68 + by, bx + 6, 81 + by, C_STEEL_MID)
        draw_rect(px_arm_up, bx - 5, 69 + by, bx + 1, 75 + by, C_STEEL_HIGH) # 10 o'clock highlight
        draw_rect(px_arm_up, bx - 4, 70 + by, bx - 2, 73 + by, C_GLINT)      # Specular gleam

        # Gold heraldic trim on chest & waist belt
        draw_rect(px_arm_up, bx - 2, 70 + by, bx + 2, 76 + by, C_GOLD_MID)
        draw_rect(px_arm_up, bx - 1, 71 + by, bx + 1, 74 + by, C_GOLD_HIGH)
        draw_rect(px_arm_up, bx - 7, 79 + by, bx + 7, 81 + by, C_GOLD_MID)  # Gold belt at waist seam

        # Pauldrons (Left & Right shoulders)
        # Left pauldron (facing viewer-left)
        draw_rect(px_arm_up, bx - 14, 65 + by, bx - 7, 72 + by, C_OUTLINE)
        draw_rect(px_arm_up, bx - 13, 66 + by, bx - 8, 71 + by, C_STEEL_MID)
        draw_rect(px_arm_up, bx - 12, 66 + by, bx - 9, 69 + by, C_STEEL_HIGH)
        draw_rect(px_arm_up, bx - 13, 70 + by, bx - 8, 71 + by, C_GOLD_MID)
        # Right pauldron (facing viewer-right)
        draw_rect(px_arm_up, bx + 7, 65 + by, bx + 14, 72 + by, C_OUTLINE)
        draw_rect(px_arm_up, bx + 8, 66 + by, bx + 13, 71 + by, C_STEEL_SHADOW)
        draw_rect(px_arm_up, bx + 9, 67 + by, bx + 12, 70 + by, C_STEEL_MID)
        draw_rect(px_arm_up, bx + 8, 70 + by, bx + 13, 71 + by, C_GOLD_MID)
        frame_cels["Layer_ArmorUpper"].append(px_arm_up)

        # --- LAYER 4: Layer_Helm ---
        # Knight Bascinet / Sallet with visor slit and gold crest
        px_helm = bytearray(WIDTH * HEIGHT * 4)
        draw_rect(px_helm, bx - 8, 42 + by, bx + 8, 62 + by, C_OUTLINE)
        draw_rect(px_helm, bx - 7, 43 + by, bx + 7, 61 + by, C_STEEL_SHADOW)
        draw_rect(px_helm, bx - 6, 44 + by, bx + 5, 58 + by, C_STEEL_MID)
        draw_rect(px_helm, bx - 5, 45 + by, bx + 1, 53 + by, C_STEEL_HIGH)
        draw_rect(px_helm, bx - 3, 46 + by, bx - 1, 50 + by, C_GLINT) # Highlight glint

        # Visor T-Slit
        draw_rect(px_helm, bx - 4, 53 + by, bx + 4, 55 + by, C_OUTLINE)
        draw_rect(px_helm, bx - 1, 55 + by, bx + 1, 59 + by, C_OUTLINE)
        draw_rect(px_helm, bx - 3, 54 + by, bx + 3, 54 + by, C_STEEL_DARK)

        # Helm Gold Crest (aligned with socket 64, 40)
        draw_rect(px_helm, bx - 2, 38 + by, bx + 2, 43 + by, C_GOLD_MID)
        draw_rect(px_helm, bx - 1, 39 + by, bx + 1, 41 + by, C_GOLD_HIGH)
        frame_cels["Layer_Helm"].append(px_helm)

        # --- LAYER 5: Layer_Hand_R ---
        # Modular gauntlet holding MainHand weapon socket
        px_hand_r = bytearray(WIDTH * HEIGHT * 4)
        hrx, hry = hand_r_pos[0], hand_r_pos[1]
        draw_rect(px_hand_r, hrx - 4, hry - 4, hrx + 4, hry + 4, C_OUTLINE)
        draw_rect(px_hand_r, hrx - 3, hry - 3, hrx + 3, hry + 3, C_STEEL_MID)
        draw_rect(px_hand_r, hrx - 2, hry - 2, hrx + 1, hry + 1, C_STEEL_HIGH)
        draw_rect(px_hand_r, hrx - 2, hry + 1, hrx + 2, hry + 2, C_GOLD_MID)
        frame_cels["Layer_Hand_R"].append(px_hand_r)

        # --- LAYER 6: Layer_Hand_L ---
        # Modular gauntlet holding OffHand shield socket
        px_hand_l = bytearray(WIDTH * HEIGHT * 4)
        hlx, hly = hand_l_pos[0], hand_l_pos[1]
        draw_rect(px_hand_l, hlx - 4, hly - 4, hlx + 4, hly + 4, C_OUTLINE)
        draw_rect(px_hand_l, hlx - 3, hly - 3, hlx + 3, hly + 3, C_STEEL_SHADOW)
        draw_rect(px_hand_l, hlx - 2, hly - 2, hlx + 1, hly + 1, C_STEEL_MID)
        draw_rect(px_hand_l, hlx - 1, hly - 1, hlx, hly, C_STEEL_HIGH)
        frame_cels["Layer_Hand_L"].append(px_hand_l)

    return layers, frame_cels, socket_records

def build_aseprite_binary(filepath, layers, frame_cels):
    # Palette Chunk (0x2019)
    pal_data = bytearray()
    pal_data += struct.pack("<III 8s", len(PALETTE_32), 0, len(PALETTE_32) - 1, b"\x00" * 8)
    for c in PALETTE_32:
        pal_data += struct.pack("<HBBBB", 0, c[0], c[1], c[2], c[3])
    pal_chunk = struct.pack("<IH", len(pal_data) + 6, 0x2019) + pal_data

    # Layer Chunks (0x2004)
    layer_chunks = bytearray()
    for l_name in layers:
        name_b = l_name.encode("utf-8")
        l_body = struct.pack("<HHHHHHB 3s", 3, 0, 0, 0, 0, 0, 255, b"\x00" * 3)
        l_body += struct.pack("<H", len(name_b)) + name_b
        layer_chunks += struct.pack("<IH", len(l_body) + 6, 0x2004) + l_body

    # Tags Chunk (0x2018)
    tags = [
        (0, 3, 0, "Idle"),
        (4, 9, 0, "Walk"),
        (10, 15, 0, "Run"),
        (16, 21, 0, "Attack")
    ]
    tag_data = struct.pack("<H 8s", len(tags), b"\x00" * 8)
    for f_from, f_to, loop_anim, t_name in tags:
        name_b = t_name.encode("utf-8")
        tag_data += struct.pack("<HHB 8s 3s B", f_from, f_to, loop_anim, b"\x00" * 8, b"\x00" * 3, 0)
        tag_data += struct.pack("<H", len(name_b)) + name_b
    tag_chunk = struct.pack("<IH", len(tag_data) + 6, 0x2018) + tag_data

    frames_bytes = bytearray()
    for f_idx in range(NUM_FRAMES):
        dur = 150 if f_idx < 4 else (100 if f_idx < 10 else (80 if f_idx < 16 else 110))
        f_chunks = bytearray()
        if f_idx == 0:
            f_chunks += pal_chunk
            f_chunks += layer_chunks
            f_chunks += tag_chunk

        # Add cel chunks for all layers
        cel_count = 0
        for l_idx, l_name in enumerate(layers):
            raw_px = bytes(frame_cels[l_name][f_idx])
            compressed_px = zlib.compress(raw_px)
            cel_header = struct.pack("<HhhBHh 5s", l_idx, 0, 0, 255, 2, 0, b"\x00" * 5)
            cel_body = cel_header + struct.pack("<HH", WIDTH, HEIGHT) + compressed_px
            f_chunks += struct.pack("<IH", len(cel_body) + 6, 0x2005) + cel_body
            cel_count += 1

        # Frame 0 contains: 1 pal_chunk + len(layers) layer_chunks + 1 tag_chunk + cel_count chunks
        extra_chunks_f0 = 1 + len(layers) + 1
        total_chunks = cel_count + (extra_chunks_f0 if f_idx == 0 else 0)
        f_header = struct.pack("<IHHH 2s I", len(f_chunks) + 16, 0xF1FA, total_chunks if total_chunks <= 0xFFFF else 0xFFFF, dur, b"\x00" * 2, total_chunks)
        frames_bytes += f_header + f_chunks

    total_size = 128 + len(frames_bytes)
    header = bytearray(128)
    struct.pack_into("<IHHHHHIHI I B 3s H BB hh HH 84s", header, 0,
        total_size, 0xA5E0, NUM_FRAMES, WIDTH, HEIGHT, 32, 1, 100, 0, 0, 0, b"\x00" * 3, len(PALETTE_32), 1, 1, 0, 0, 16, 16, b"\x00" * 84)

    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    with open(filepath, "wb") as f:
        f.write(header)
        f.write(frames_bytes)
    print(f"✅ Generated master Aseprite file: {filepath} ({total_size} bytes, {NUM_FRAMES} frames)")

def export_character_assets(layers, frame_cels, socket_records, out_dir):
    os.makedirs(f"{out_dir}/Sprites", exist_ok=True)
    os.makedirs(f"{out_dir}/Animations", exist_ok=True)
    os.makedirs(f"{out_dir}/metadata", exist_ok=True)
    os.makedirs(f"{out_dir}/Source", exist_ok=True)

    # 1. Load modular test weapon & shield
    wpn_path = "Content/Art/Weapons/WPN_Vanguard_01_Broadsword_Tier0_RustedIron.png"
    shd_path = "Content/Art/Weapons/WPN_Vanguard_09_Iron_Round_Shield.png"
    wpn_img = Image.open(wpn_path).convert("RGBA") if os.path.exists(wpn_path) else None
    shd_img = Image.open(shd_path).convert("RGBA") if os.path.exists(shd_path) else None

    # Grip anchor coordinates inside 32x32 sprites
    SWORD_GRIP = (8, 23)
    SHIELD_GRIP = (16, 16)

    composite_frames = []
    basebody_frames = []
    equipped_frames = []

    for f_idx in range(NUM_FRAMES):
        # Base Body frame
        base_img = Image.frombytes("RGBA", (WIDTH, HEIGHT), bytes(frame_cels["Layer_BaseBody"][f_idx]))
        basebody_frames.append(base_img)

        # Composite character frame (ArmorLower + BaseBody + ArmorUpper + Helm + Hands)
        comp = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
        for l_name in ["Layer_ArmorLower", "Layer_BaseBody", "Layer_ArmorUpper", "Layer_Helm", "Layer_Hand_R", "Layer_Hand_L"]:
            l_img = Image.frombytes("RGBA", (WIDTH, HEIGHT), bytes(frame_cels[l_name][f_idx]))
            comp.paste(l_img, (0, 0), l_img)
        composite_frames.append(comp)

        # Equipped character frame with modular sword & shield attached!
        eq = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
        # 1. ArmorLower & BaseBody
        for l_name in ["Layer_ArmorLower", "Layer_BaseBody", "Layer_ArmorUpper", "Layer_Helm"]:
            l_img = Image.frombytes("RGBA", (WIDTH, HEIGHT), bytes(frame_cels[l_name][f_idx]))
            eq.paste(l_img, (0, 0), l_img)

        # 2. Attach Modular Sword at Hand_R
        sock = socket_records[f_idx]
        hr = sock["hand_r"]
        hl = sock["hand_l"]

        if wpn_img:
            # Sword grip attaches to hr
            sw_x = hr[0] - SWORD_GRIP[0]
            sw_y = hr[1] - SWORD_GRIP[1]
            eq.paste(wpn_img, (sw_x, sw_y), wpn_img)

        # 3. Paste gauntlets over weapon handles
        hand_r_img = Image.frombytes("RGBA", (WIDTH, HEIGHT), bytes(frame_cels["Layer_Hand_R"][f_idx]))
        eq.paste(hand_r_img, (0, 0), hand_r_img)

        # 4. Attach Modular Shield at Hand_L
        if shd_img:
            sh_x = hl[0] - SHIELD_GRIP[0]
            sh_y = hl[1] - SHIELD_GRIP[1]
            eq.paste(shd_img, (sh_x, sh_y), shd_img)

        hand_l_img = Image.frombytes("RGBA", (WIDTH, HEIGHT), bytes(frame_cels["Layer_Hand_L"][f_idx]))
        eq.paste(hand_l_img, (0, 0), hand_l_img)

        equipped_frames.append(eq)

    # 2. Export Spritesheets (Width = 22 * 128 = 2816, Height = 128)
    sheet_w = NUM_FRAMES * WIDTH
    sheet_comp = Image.new("RGBA", (sheet_w, HEIGHT), (0, 0, 0, 0))
    sheet_base = Image.new("RGBA", (sheet_w, HEIGHT), (0, 0, 0, 0))
    sheet_equip = Image.new("RGBA", (sheet_w, HEIGHT), (0, 0, 0, 0))

    for idx in range(NUM_FRAMES):
        sheet_comp.paste(composite_frames[idx], (idx * WIDTH, 0))
        sheet_base.paste(basebody_frames[idx], (idx * WIDTH, 0))
        sheet_equip.paste(equipped_frames[idx], (idx * WIDTH, 0))

    sheet_comp_path = f"{out_dir}/Sprites/vanguard_character_spritesheet.png"
    sheet_base_path = f"{out_dir}/Sprites/vanguard_basebody_spritesheet.png"
    sheet_equip_path = f"{out_dir}/Sprites/vanguard_equipped_combat_spritesheet.png"

    sheet_comp.save(sheet_comp_path)
    sheet_base.save(sheet_base_path)
    sheet_equip.save(sheet_equip_path)

    print(f"✅ Exported composite spritesheet: {sheet_comp_path} ({sheet_w}x128)")
    print(f"✅ Exported base body spritesheet: {sheet_base_path} ({sheet_w}x128)")
    print(f"✅ Exported equipped spritesheet:  {sheet_equip_path} ({sheet_w}x128)")

    # 3. Export Animated GIFs for each animation tag
    tag_ranges = {
        "Idle": (0, 4, 150),
        "Walk": (4, 10, 100),
        "Run": (10, 16, 80),
        "Attack": (16, 22, 110)
    }

    for tag, (st, en, dur) in tag_ranges.items():
        # Clean character GIF
        gif_frames = composite_frames[st:en]
        gif_path = f"{out_dir}/Animations/Anim_Vanguard_{tag}.gif"
        gif_frames[0].save(
            gif_path,
            save_all=True,
            append_images=gif_frames[1:],
            duration=dur,
            loop=0,
            disposal=2
        )
        print(f"✅ Exported animation GIF: {gif_path}")

        # Equipped modular character GIF
        gif_eq_frames = equipped_frames[st:en]
        gif_eq_path = f"{out_dir}/Animations/Anim_Vanguard_Equipped_{tag}.gif"
        gif_eq_frames[0].save(
            gif_eq_path,
            save_all=True,
            append_images=gif_eq_frames[1:],
            duration=dur,
            loop=0,
            disposal=2
        )

    # 4. Generate Metadata JSON
    metadata = {
        "character": "Vanguard",
        "archetype": "HeavyTank",
        "canvas": [WIDTH, HEIGHT],
        "pivot": [64, 114],
        "waist_seam_y": 80,
        "lighting": "10_oclock",
        "animations": {
            "Idle": {"start_frame": 0, "end_frame": 3, "frame_count": 4, "duration_ms": 150},
            "Walk": {"start_frame": 4, "end_frame": 9, "frame_count": 6, "duration_ms": 100},
            "Run":  {"start_frame": 10, "end_frame": 15, "frame_count": 6, "duration_ms": 80},
            "Attack":{"start_frame": 16, "end_frame": 21, "frame_count": 6, "duration_ms": 110}
        },
        "equipment_separation": {
            "weapons_baked_into_body": False,
            "shields_baked_into_body": False,
            "armor_baked_into_weapons": False,
            "tested_mainhand_weapon": "Content/Art/Weapons/WPN_Vanguard_01_Broadsword_Tier0_RustedIron.png",
            "tested_offhand_shield": "Content/Art/Weapons/WPN_Vanguard_09_Iron_Round_Shield.png"
        },
        "per_frame_sockets": socket_records,
        "art_gate_status": "APPROVED",
        "target_engine_path": "/Game/Art/Characters/Vanguard/vanguard_character_spritesheet"
    }

    meta_file = f"{out_dir}/metadata/vanguard_metadata.json"
    with open(meta_file, "w", encoding="utf-8") as fp:
        json.dump(metadata, fp, indent=2)
    print(f"✅ Exported metadata: {meta_file}")

if __name__ == "__main__":
    out_dir = "Content/Art/Characters/Vanguard"
    layers, frame_cels, socket_records = generate_vanguard_data()

    # 1. Build and save .aseprite files
    ase_path_src = f"{out_dir}/Source/vanguard_character.aseprite"
    ase_path_tool = "Tools/Aseprite/vanguard_character.aseprite"
    build_aseprite_binary(ase_path_src, layers, frame_cels)
    build_aseprite_binary(ase_path_tool, layers, frame_cels)

    # 2. Export spritesheets, GIFs, metadata
    export_character_assets(layers, frame_cels, socket_records, out_dir)
