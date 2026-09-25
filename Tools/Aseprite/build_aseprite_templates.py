#!/usr/bin/env python3
# ==============================================================================
# build_aseprite_templates.py: Exact Binary Generator for .aseprite Templates
# Project: Project Ascendant (2.5D Isometric Hardcore ARPG MMO on UE 5.8)
# Conforms strictly to: ase-file-specs.md (Aseprite official specification)
# ==============================================================================

import struct
import zlib
import os

PALETTE_32 = [
    (0, 0, 0, 0),         # 0: transparent
    (18, 18, 20, 255),    # 1: #121214 (Viền bao ngoài 1px)
    (30, 36, 44, 255),    # 2: #1E242C (Contour kim loại sẫm)
    (74, 88, 104, 255),   # 3: #4A5868 (Steel Shadow)
    (148, 164, 180, 255), # 4: #94A4B4 (Steel Midtone)
    (220, 228, 236, 255), # 5: #DCE4EC (Steel Highlight)
    (255, 255, 255, 255), # 6: #FFFFFF (Glint)
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
    (0, 255, 255, 200)    # 31: #00FFFF (Guide Sockets)
]

def build_aseprite_template(filepath, rig_name):
    width, height = 128, 128
    num_frames = 16

    # 1. Palette Chunk (0x2019)
    pal_data = bytearray()
    pal_data += struct.pack("<III 8s", len(PALETTE_32), 0, len(PALETTE_32) - 1, b"\x00" * 8)
    for c in PALETTE_32:
        pal_data += struct.pack("<HBBBB", 0, c[0], c[1], c[2], c[3])
    pal_chunk = struct.pack("<IH", len(pal_data) + 6, 0x2019) + pal_data

    # 2. Layers Chunk (0x2004)
    layers = [
        "Guide_Sockets",
        "Layer_LowerBody",
        "Layer_UpperBody",
        "Layer_Tabard",
        "Layer_HelmCrest",
        "Layer_Hand_R",
        "Layer_Hand_L"
    ]
    layer_chunks = bytearray()
    for l_name in layers:
        name_b = l_name.encode("utf-8")
        # flags(2), type(2), child(2), w(2), h(2), blend(2), opacity(1), future(3)
        l_body = struct.pack("<HHHHHHB 3s", 3, 0, 0, 0, 0, 0, 255, b"\x00" * 3)
        l_body += struct.pack("<H", len(name_b)) + name_b
        layer_chunks += struct.pack("<IH", len(l_body) + 6, 0x2004) + l_body

    # 3. Tags Chunk (0x2018)
    tags = [
        (0, 3, 0, "Idle"),
        (4, 9, 0, "Walk"),
        (10, 15, 0, "Run")
    ]
    tag_data = struct.pack("<H 8s", len(tags), b"\x00" * 8)
    for f_from, f_to, loop_anim, t_name in tags:
        name_b = t_name.encode("utf-8")
        tag_data += struct.pack("<HHB 8s 3s B", f_from, f_to, loop_anim, b"\x00" * 8, b"\x00" * 3, 0)
        tag_data += struct.pack("<H", len(name_b)) + name_b
    tag_chunk = struct.pack("<IH", len(tag_data) + 6, 0x2018) + tag_data

    # 4. Guide cel pixels (Raw RGBA)
    pixels = bytearray(width * height * 4)
    def set_px(x, y, r, g, b, a):
        if 0 <= x < width and 0 <= y < height:
            idx = (y * width + x) * 4
            pixels[idx] = r
            pixels[idx + 1] = g
            pixels[idx + 2] = b
            pixels[idx + 3] = a

    # Waist seam Y=80
    for x in range(48, 81):
        set_px(x, 80, 255, 100, 100, 180)
    # Foot Pivot (64, 114)
    for d in range(-2, 3):
        set_px(64 + d, 114, 255, 255, 0, 255)
        set_px(64, 114 + d, 255, 255, 0, 255)
    # Helm Crest (64, 40)
    for d in range(-2, 3):
        set_px(64 + d, 40, 0, 255, 255, 200)
        set_px(64, 40 + d, 0, 255, 255, 200)
    # Tabard (64, 60)
    for d in range(-2, 3):
        set_px(64 + d, 60, 0, 255, 255, 200)
        set_px(64, 60 + d, 0, 255, 255, 200)
    # Hand_R (96, 76)
    for d in range(-2, 3):
        set_px(96 + d, 76, 0, 255, 255, 200)
        set_px(96, 76 + d, 0, 255, 255, 200)
    # Hand_L (32, 76)
    for d in range(-2, 3):
        set_px(32 + d, 76, 0, 255, 255, 200)
        set_px(32, 76 + d, 0, 255, 255, 200)

    compressed_px = zlib.compress(bytes(pixels))

    # Cel Chunk (0x2005) header:
    # WORD Layer_index(2) + SHORT x(2) + SHORT y(2) + BYTE opacity(1) + WORD cel_type(2) + SHORT z_index(2) + BYTE[5] future
    # Total = 16 bytes
    cel_header = struct.pack("<HhhBHh 5s", 0, 0, 0, 255, 2, 0, b"\x00" * 5)
    cel_body = cel_header + struct.pack("<HH", width, height) + compressed_px
    cel_chunk = struct.pack("<IH", len(cel_body) + 6, 0x2005) + cel_body

    # Linked cel chunk for other frames
    link_header = struct.pack("<HhhBHh 5s", 0, 0, 0, 255, 1, 0, b"\x00" * 5)
    link_body = link_header + struct.pack("<H", 0)
    link_chunk = struct.pack("<IH", len(link_body) + 6, 0x2005) + link_body

    frames_bytes = bytearray()
    for f_idx in range(num_frames):
        dur = 150 if f_idx < 4 else (100 if f_idx < 10 else 80)
        f_chunks = bytearray()
        if f_idx == 0:
            f_chunks += pal_chunk
            f_chunks += layer_chunks
            f_chunks += tag_chunk
            f_chunks += cel_chunk
            chunk_count = 1 + len(layers) + 1 + 1
        else:
            f_chunks += link_chunk
            chunk_count = 1

        f_header = struct.pack("<IHHH 2s I", len(f_chunks) + 16, 0xF1FA, chunk_count if chunk_count <= 0xFFFF else 0xFFFF, dur, b"\x00" * 2, chunk_count)
        frames_bytes += f_header + f_chunks

    total_size = 128 + len(frames_bytes)
    header = bytearray(128)
    struct.pack_into("<IHHHHHIHI I B 3s H BB hh HH 84s", header, 0,
        total_size, 0xA5E0, num_frames, width, height, 32, 1, 100, 0, 0, 0, b"\x00" * 3, len(PALETTE_32), 1, 1, 0, 0, 16, 16, b"\x00" * 84)

    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    with open(filepath, "wb") as f:
        f.write(header)
        f.write(frames_bytes)
    print(f"Generated {filepath} ({total_size} B, {num_frames} frames, Rig: {rig_name})")

if __name__ == "__main__":
    out_dir = "Tools/Aseprite/templates"
    build_aseprite_template(f"{out_dir}/master_rig_01.aseprite", "HeavyTank")
    build_aseprite_template(f"{out_dir}/master_rig_02.aseprite", "Agility")
    build_aseprite_template(f"{out_dir}/master_rig_03.aseprite", "Caster")
    build_aseprite_template(f"{out_dir}/master_rig_04.aseprite", "Monk")
