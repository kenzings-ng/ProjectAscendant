#!/usr/bin/env python3
# ==============================================================================
# build_vanguard_character_template.py: Aseprite Source Template Generator
# Package: M5 - New Vanguard Character Production Foundation
# Project: Project Ascendant (2.5D Isometric Hardcore ARPG MMO on UE 5.8)
#
# Generates an EMPTY, layer/tag-correct .aseprite source file for the new
# Vanguard character. Reuses the exact binary-writing technique already
# proven by Tools/Aseprite/build_aseprite_templates.py (master rig templates)
# and the exact socket constants already defined in
# Tools/Aseprite/aseprite_export_pipeline.py (SOCKET_METADATA) - no global
# convention is redefined here.
#
# This script produces STRUCTURE ONLY (canvas, layers, animation tags, guide
# socket markers). It does not and cannot generate the actual character
# illustration content - that is genuine original pixel-art authoring work
# to be done by an artist inside Aseprite, using this file as the starting
# point (open it, draw on the body-part layers, keep the Guide_Sockets
# layer for alignment reference, then hide/delete it before final export).
# ==============================================================================

import struct
import zlib
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from aseprite_export_pipeline import SOCKET_METADATA

FOOT_PIVOT = SOCKET_METADATA["foot_pivot"]          # (64, 114)
WAIST_Y = SOCKET_METADATA["waist_seam_y"]            # 80
MAINHAND_SOCKET = SOCKET_METADATA["hand_socket_r"]   # (96, 76)
OFFHAND_SOCKET = SOCKET_METADATA["hand_socket_l"]    # (32, 76)

PALETTE_32 = [
    (0, 0, 0, 0),
    (18, 18, 20, 255), (30, 36, 44, 255), (74, 88, 104, 255), (148, 164, 180, 255),
    (220, 228, 236, 255), (255, 255, 255, 255),
    (58, 36, 8, 255), (140, 90, 20, 255), (224, 168, 48, 255), (255, 244, 176, 255),
    (44, 24, 8, 255), (92, 58, 30, 255), (154, 106, 64, 255), (208, 168, 120, 255),
    (80, 40, 24, 255), (176, 112, 80, 255), (232, 176, 136, 255), (255, 224, 192, 255),
    (56, 0, 8, 255), (128, 8, 24, 255), (208, 32, 32, 255), (255, 112, 96, 255),
    (8, 16, 48, 255), (16, 40, 120, 255), (40, 96, 208, 255), (112, 176, 255, 255),
    (24, 4, 40, 255), (64, 16, 104, 255), (128, 48, 192, 255), (208, 136, 255, 255),
    (0, 255, 255, 200),
]

# Animation tags: (from_frame, to_frame, duration_ms, name)
# Frame counts are the UPPER end of the spec's target ranges (Idle 4, Walk 6,
# Run 6, Attack 8) - the artist may trim Attack down to as few as 6 frames if
# every remaining frame carries intentional motion (anticipation/action/
# impact/recovery); trimming frames is a content decision, not a template
# structure change.
ANIM_TAGS = [
    (0, 3, "Idle", 150),
    (4, 9, "Walk", 100),
    (10, 15, "Run", 80),
    (16, 23, "Attack", 90),
]

# Bottom-to-top compositing order, per the M5 spec's layer hierarchy
# (Body sub-layers, then Armor, then Equipment, then Effects, then the
# non-exported Guide_Sockets reference layer on top).
LAYER_NAMES = [
    "Layer_BaseBody",
    "Layer_Legs",
    "Layer_Feet",
    "Layer_Torso",
    "Layer_Arms",
    "Layer_Hands",
    "Layer_Head",
    "Layer_Hair",
    "Layer_Armor_VanguardArmor",
    "Layer_Equipment_Mainhand",
    "Layer_Equipment_Offhand",
    "Layer_Effects",
    "Guide_Sockets",
]

def build_vanguard_template(filepath):
    width, height = 128, 128
    total_frames = ANIM_TAGS[-1][1] + 1  # 24

    # 1. Palette Chunk (0x2019)
    pal_data = bytearray()
    pal_data += struct.pack("<III 8s", len(PALETTE_32), 0, len(PALETTE_32) - 1, b"\x00" * 8)
    for c in PALETTE_32:
        pal_data += struct.pack("<HBBBB", 0, c[0], c[1], c[2], c[3])
    pal_chunk = struct.pack("<IH", len(pal_data) + 6, 0x2019) + pal_data

    # 2. Layers Chunk (0x2004) - one per name, bottom-to-top
    layer_chunks = bytearray()
    for l_name in LAYER_NAMES:
        name_b = l_name.encode("utf-8")
        l_body = struct.pack("<HHHHHHB 3s", 3, 0, 0, 0, 0, 0, 255, b"\x00" * 3)
        l_body += struct.pack("<H", len(name_b)) + name_b
        layer_chunks += struct.pack("<IH", len(l_body) + 6, 0x2004) + l_body
    guide_layer_index = len(LAYER_NAMES) - 1

    # 3. Tags Chunk (0x2018)
    tag_data = struct.pack("<H 8s", len(ANIM_TAGS), b"\x00" * 8)
    for f_from, f_to, t_name, _dur in ANIM_TAGS:
        name_b = t_name.encode("utf-8")
        tag_data += struct.pack("<HHB 8s 3s B", f_from, f_to, 0, b"\x00" * 8, b"\x00" * 3, 0)
        tag_data += struct.pack("<H", len(name_b)) + name_b
    tag_chunk = struct.pack("<IH", len(tag_data) + 6, 0x2018) + tag_data

    # 4. Guide cel pixels (Raw RGBA) - drawn on the Guide_Sockets layer only,
    # using the project's EXISTING socket constants (not redefined here).
    pixels = bytearray(width * height * 4)
    def set_px(x, y, r, g, b, a):
        if 0 <= x < width and 0 <= y < height:
            idx = (y * width + x) * 4
            pixels[idx], pixels[idx + 1], pixels[idx + 2], pixels[idx + 3] = r, g, b, a

    for x in range(48, 81):
        set_px(x, WAIST_Y, 255, 100, 100, 180)
    fx, fy = FOOT_PIVOT
    for d in range(-2, 3):
        set_px(fx + d, fy, 255, 255, 0, 255)
        set_px(fx, fy + d, 255, 255, 0, 255)
    mx, my = MAINHAND_SOCKET
    for d in range(-2, 3):
        set_px(mx + d, my, 0, 255, 255, 200)
        set_px(mx, my + d, 0, 255, 255, 200)
    ox, oy = OFFHAND_SOCKET
    for d in range(-2, 3):
        set_px(ox + d, oy, 0, 255, 255, 200)
        set_px(ox, oy + d, 0, 255, 255, 200)

    compressed_px = zlib.compress(bytes(pixels))
    guide_cel_header = struct.pack("<HhhBHh 5s", guide_layer_index, 0, 0, 255, 2, 0, b"\x00" * 5)
    guide_cel_body = guide_cel_header + struct.pack("<HH", width, height) + compressed_px
    guide_cel_chunk = struct.pack("<IH", len(guide_cel_body) + 6, 0x2005) + guide_cel_body

    # Linked guide cel for every other frame (same guide markers throughout)
    link_header = struct.pack("<HhhBHh 5s", guide_layer_index, 0, 0, 255, 1, 0, b"\x00" * 5)
    link_body = link_header + struct.pack("<H", 0)
    link_chunk = struct.pack("<IH", len(link_body) + 6, 0x2005) + link_body

    frames_bytes = bytearray()
    for f_idx in range(total_frames):
        dur = next(d for f0, f1, _n, d in ANIM_TAGS if f0 <= f_idx <= f1)
        f_chunks = bytearray()
        if f_idx == 0:
            f_chunks += pal_chunk
            f_chunks += layer_chunks
            f_chunks += tag_chunk
            f_chunks += guide_cel_chunk
            chunk_count = 1 + len(LAYER_NAMES) + 1 + 1
        else:
            f_chunks += link_chunk
            chunk_count = 1

        f_header = struct.pack("<IHHH 2s I", len(f_chunks) + 16, 0xF1FA,
                                chunk_count if chunk_count <= 0xFFFF else 0xFFFF,
                                dur, b"\x00" * 2, chunk_count)
        frames_bytes += f_header + f_chunks

    total_size = 128 + len(frames_bytes)
    header = bytearray(128)
    struct.pack_into("<IHHHHHIHI I B 3s H BB hh HH 84s", header, 0,
        total_size, 0xA5E0, total_frames, width, height, 32, 1, 100, 0, 0, 0,
        b"\x00" * 3, len(PALETTE_32), 1, 1, 0, 0, 16, 16, b"\x00" * 84)

    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    with open(filepath, "wb") as f:
        f.write(header)
        f.write(frames_bytes)
    print(f"Generated {filepath} ({total_size} B, {total_frames} frames, {len(LAYER_NAMES)} layers)")
    return filepath

if __name__ == "__main__":
    build_vanguard_template("Tools/Aseprite/templates/vanguard_character_source.aseprite")
