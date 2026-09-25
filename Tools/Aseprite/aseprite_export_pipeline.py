#!/usr/bin/env python3
# ==============================================================================
# aseprite_export_pipeline.py: Automated Spritesheet & Metadata Exporter
# Project: Project Ascendant (2.5D Isometric Hardcore ARPG MMO on UE 5.8)
# Conforms to: SPEC-ART-2026-09-23-V2 & Tools/Aseprite WBS
# ==============================================================================

import os
import sys
import json
import zlib
import struct
import shutil
import argparse
import subprocess
from PIL import Image

SOCKET_METADATA = {
    "foot_pivot": [64, 114],
    "waist_seam_y": 80,
    "socket_helm_crest": [64, 40],
    "socket_tabard": [64, 60],
    "hand_socket_r": [96, 76],
    "hand_socket_l": [32, 76],
    "native_grid": [128, 128]
}

def parse_aseprite_file(filepath):
    """
    Pure-Python parser for .aseprite / .ase binary files.
    Extracts frame images, durations, layers, and animation tags.
    Acts as headless fallback when desktop Aseprite CLI is not installed.
    """
    with open(filepath, "rb") as f:
        data = f.read()

    if len(data) < 128:
        raise ValueError("Tệp quá nhỏ so với tiêu chuẩn Aseprite Header 128 bytes")

    filesize, magic, frames_count, width, height, depth = struct.unpack_from("<I H H H H H", data, 0)
    if magic != 0xA5E0:
        raise ValueError(f"Mã magic không hợp lệ: {hex(magic)}, kỳ vọng 0xA5E0")

    offset = 128
    frames = []
    layers = []
    tags = []
    cels_by_frame = []

    for f_idx in range(frames_count):
        if offset >= len(data):
            break
        f_size, f_magic, old_chunks, dur, _, new_chunks = struct.unpack_from("<I H H H 2s I", data, offset)
        if f_magic != 0xF1FA:
            raise ValueError(f"Frame {f_idx}: Magic không hợp lệ {hex(f_magic)}, kỳ vọng 0xF1FA")

        chunk_count = new_chunks if new_chunks > 0 else old_chunks
        chunk_offset = offset + 16
        frame_cels = {}

        for _ in range(chunk_count):
            if chunk_offset >= offset + f_size:
                break
            c_size, c_type = struct.unpack_from("<I H", data, chunk_offset)
            c_data = data[chunk_offset + 6 : chunk_offset + c_size]

            # 0x2004: Layer Chunk
            if c_type == 0x2004:
                flags, l_type, _, _, _, blend, opacity = struct.unpack_from("<H H H H H H B", c_data, 0)
                name_len = struct.unpack_from("<H", c_data, 16)[0]
                name = c_data[18 : 18 + name_len].decode("utf-8", errors="ignore")
                layers.append({"name": name, "flags": flags, "opacity": opacity})

            # 0x2018: Tags Chunk
            elif c_type == 0x2018:
                num_tags = struct.unpack_from("<H", c_data, 0)[0]
                tag_off = 10
                for _ in range(num_tags):
                    f_from, f_to = struct.unpack_from("<HH", c_data, tag_off)
                    t_name_len = struct.unpack_from("<H", c_data, tag_off + 17)[0]
                    t_name = c_data[tag_off + 19 : tag_off + 19 + t_name_len].decode("utf-8", errors="ignore")
                    tags.append({"name": t_name, "from": f_from, "to": f_to})
                    tag_off += 19 + t_name_len

            # 0x2005: Cel Chunk
            elif c_type == 0x2005:
                # WORD Layer(2), SHORT X(2), SHORT Y(2), BYTE Opacity(1), WORD CelType(2), SHORT ZIndex(2), BYTE[5] Future
                # Total header = 16 bytes
                l_index, x, y, cel_opacity, cel_type, z_index = struct.unpack_from("<HhhBHh", c_data, 0)
                if cel_type == 2: # Compressed Image
                    w, h = struct.unpack_from("<HH", c_data, 16)
                    raw_pixels = zlib.decompress(c_data[20:])
                    frame_cels[l_index] = {"x": x, "y": y, "w": w, "h": h, "raw": raw_pixels}
                elif cel_type == 1: # Linked Cel
                    link_frame = struct.unpack_from("<H", c_data, 16)[0]
                    if link_frame < len(cels_by_frame) and l_index in cels_by_frame[link_frame]:
                        frame_cels[l_index] = cels_by_frame[link_frame][l_index]

            chunk_offset += c_size

        cels_by_frame.append(frame_cels)
        frames.append({"index": f_idx, "duration_ms": dur, "cels": frame_cels})
        offset += f_size

    # Composite images for each frame
    frame_images = []
    for f_idx, f_info in enumerate(frames):
        img = Image.new("RGBA", (width, height), (0, 0, 0, 0))
        # Draw from bottom layer to top layer
        for l_idx in sorted(f_info["cels"].keys()):
            cel = f_info["cels"][l_idx]
            cel_img = Image.frombytes("RGBA", (cel["w"], cel["h"]), cel["raw"])
            img.paste(cel_img, (cel["x"], cel["y"]), cel_img)
        frame_images.append(img)

    return {
        "width": width,
        "height": height,
        "num_frames": frames_count,
        "frames": frames,
        "frame_images": frame_images,
        "layers": layers,
        "tags": tags
    }

def export_file(input_path, output_dir, sheet_name=None):
    """Xuất một tệp .aseprite thành Spritesheet PNG và metadata JSON."""
    os.makedirs(output_dir, exist_ok=True)
    base_name = sheet_name or os.path.splitext(os.path.basename(input_path))[0]
    out_sheet_png = os.path.join(output_dir, f"{base_name}_sheet.png")
    out_meta_json = os.path.join(output_dir, f"{base_name}_meta.json")

    # Kiểm tra xem có CLI Aseprite hay không
    aseprite_bin = shutil.which("aseprite")
    if aseprite_bin:
        print(f"[Aseprite CLI] Phát hiện binary tại: {aseprite_bin}")
        cmd = [
            aseprite_bin, "-b",
            input_path,
            "--sheet", out_sheet_png,
            "--data", out_meta_json,
            "--format", "json-array",
            "--list-tags",
            "--list-slices"
        ]
        res = subprocess.run(cmd, capture_output=True, text=True)
        if res.returncode == 0:
            print(f"[Aseprite CLI] Xuất xưởng thành công qua Aseprite Native: {out_sheet_png}")
            # Inject Project Ascendant sockets metadata
            try:
                with open(out_meta_json, "r") as fp:
                    meta = json.load(fp)
                meta["project_ascendant_sockets"] = SOCKET_METADATA
                with open(out_meta_json, "w") as fp:
                    json.dump(meta, fp, indent=2)
            except Exception as e:
                print(f"Warning: Không thể inject socket metadata: {e}")
            return True
        else:
            print(f"[Aseprite CLI Warning] Lệnh native thất bại ({res.stderr}), chuyển sang fallback parser...")

    # Chạy Fallback Python Parser
    print(f"[Fallback Pipeline] Chạy Pure-Python Aseprite Engine cho: {input_path}")
    parsed = parse_aseprite_file(input_path)
    w, h = parsed["width"], parsed["height"]
    n_frames = parsed["num_frames"]

    # Ghép Spritesheet theo hàng ngang 1 x N
    sheet_w = w * n_frames
    sheet_h = h
    sheet_img = Image.new("RGBA", (sheet_w, sheet_h), (0, 0, 0, 0))

    frames_meta = []
    for idx, f_img in enumerate(parsed["frame_images"]):
        pos_x = idx * w
        sheet_img.paste(f_img, (pos_x, 0))
        f_meta = {
            "filename": f"{base_name}_{idx}.png",
            "frame": {"x": pos_x, "y": 0, "w": w, "h": h},
            "rotated": False,
            "trimmed": False,
            "spriteSourceSize": {"x": 0, "y": 0, "w": w, "h": h},
            "sourceSize": {"w": w, "h": h},
            "duration": parsed["frames"][idx]["duration_ms"]
        }
        frames_meta.append(f_meta)

    sheet_img.save(out_sheet_png)

    meta_doc = {
        "frames": frames_meta,
        "meta": {
            "app": "ProjectAscendant_AsepritePipeline",
            "version": "1.0",
            "image": os.path.basename(out_sheet_png),
            "format": "RGBA8888",
            "size": {"w": sheet_w, "h": sheet_h},
            "scale": "1",
            "frameTags": parsed["tags"],
            "layers": [l["name"] for l in parsed["layers"]],
            "project_ascendant_sockets": SOCKET_METADATA
        }
    }

    with open(out_meta_json, "w") as fp:
        json.dump(meta_doc, fp, indent=2)

    print(f"✅ Đã xuất Spritesheet: {out_sheet_png} ({sheet_w}x{sheet_h} px)")
    print(f"✅ Đã xuất Metadata:   {out_meta_json}")
    return True

def validate_and_process_weapon(input_path, output_dir):
    """
    Validates a native 32x32 static weapon PNG against Art Gate criteria
    and promotes it to Content/Art/Weapons/ with standardized naming and sidecar metadata.
    """
    fname = os.path.basename(input_path)
    clean_name = fname.replace("_32x32_native.png", "").replace(".png", "")
    out_name = f"WPN_{clean_name}.png" if not clean_name.startswith("WPN_") else f"{clean_name}.png"
    out_path = os.path.join(output_dir, out_name)
    meta_path = os.path.join(output_dir, f"{os.path.splitext(out_name)[0]}_meta.json")

    with Image.open(input_path) as img:
        w, h = img.size
        mode = img.mode
        colors = img.getcolors(maxcolors=1000)
        num_colors = len(colors) if colors else 0
        bbox = img.getbbox()

        issues = []
        if w != 32 or h != 32:
            issues.append(f"Invalid dimensions {w}x{h} (expected 32x32)")

        semi_trans = []
        if mode == "RGBA":
            for y in range(h):
                for x in range(w):
                    r, g, b, a = img.getpixel((x, y))
                    if 0 < a < 255:
                        semi_trans.append((x, y, a))
        if semi_trans:
            issues.append(f"{len(semi_trans)} semi-transparent pixels detected (e.g. a={semi_trans[0][2]})")

        thumb = img.resize((16, 16), Image.NEAREST)
        thumb_bbox = thumb.getbbox()
        if not (thumb_bbox and (thumb_bbox[2] - thumb_bbox[0] >= 3) and (thumb_bbox[3] - thumb_bbox[1] >= 3)):
            issues.append("Silhouette legibility failed at 16x16 thumbnail")

        if num_colors > 32 or num_colors < 4:
            issues.append(f"Palette color count ({num_colors}) outside accepted range [4, 32]")

        if issues:
            return False, "REWORK_REQUIRED", issues, None, None

        # Passed Art Gate! Copy/promote to production
        os.makedirs(output_dir, exist_ok=True)
        img.save(out_path)

        meta = {
            "asset_name": out_name,
            "type": "static_weapon",
            "dimensions": [w, h],
            "colors": num_colors,
            "bounding_box": list(bbox) if bbox else [],
            "art_gate_status": "APPROVED",
            "source_legacy_path": os.path.relpath(input_path),
            "target_engine_path": f"/Game/Art/Weapons/{os.path.splitext(out_name)[0]}"
        }
        with open(meta_path, "w", encoding="utf-8") as f:
            json.dump(meta, f, indent=2)

        return True, "PASS", [], out_path, meta_path

ARMOR_NAME_MAPPING = {
    "Armor_Arcanist_Scholar_Tunic.png": "ARM_Arcanist_01_Scholar_Tunic.png",
    "Armor_Leather_Scout_Vest.png": "ARM_Ranger_01_Leather_Scout_Vest.png",
    "Armor_Steel_Knight_Cuirass.png": "ARM_Vanguard_01_Steel_Knight_Cuirass.png",
    "Boots_Leather_Traveler_Boots.png": "ARM_Ranger_02_Leather_Traveler_Boots.png",
    "Greaves_Steel_Knight_Legguards.png": "ARM_Vanguard_02_Steel_Knight_Legguards.png",
    "Helmet_Leather_Ranger_Hood.png": "ARM_Ranger_03_Leather_Ranger_Hood.png",
    "Helmet_Steel_Knight_Closed.png": "ARM_Vanguard_03_Steel_Knight_Closed_Helm.png",
    "Helmet_Steel_Knight_Open.png": "ARM_Vanguard_04_Steel_Knight_Open_Helm.png"
}

OFFICIAL_4TONE_PALETTE = [
    (18, 18, 20), (30, 36, 44),
    (74, 88, 104), (148, 164, 180), (220, 228, 236), (255, 255, 255),
    (58, 36, 8), (140, 90, 20), (224, 168, 48), (255, 244, 176),
    (44, 24, 8), (92, 58, 30), (154, 106, 64), (208, 168, 120),
    (80, 40, 24), (176, 112, 80), (232, 176, 136), (255, 224, 192),
    (56, 0, 8), (128, 8, 24), (208, 32, 32), (255, 112, 96),
    (8, 16, 48), (16, 40, 120), (40, 96, 208), (112, 176, 255),
    (24, 4, 40), (64, 16, 104), (128, 48, 192), (208, 136, 255)
]

def map_to_official_palette(rgb):
    best_c = OFFICIAL_4TONE_PALETTE[0]
    best_d = float("inf")
    r, g, b = int(rgb[0]), int(rgb[1]), int(rgb[2])
    for c in OFFICIAL_4TONE_PALETTE:
        d = (r - c[0])**2 + (g - c[1])**2 + (b - c[2])**2
        if d < best_d:
            best_d = d
            best_c = c
    return best_c

def rework_armor_to_32x32(input_path):
    """
    Reworks high-res legacy armor asset to 32x32 pixel art:
    - Downsamples with LANCZOS filter
    - Quantizes to official 4-tone ramp palette
    - Normalizes alpha to strict binary {0, 255}
    """
    with Image.open(input_path) as raw:
        img = raw.convert("RGBA")
        small = img.resize((32, 32), Image.Resampling.LANCZOS)
        out_img = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
        for y in range(32):
            for x in range(32):
                r, g, b, a = small.getpixel((x, y))
                if a >= 128:
                    c = map_to_official_palette((r, g, b))
                    out_img.putpixel((x, y), (c[0], c[1], c[2], 255))
                else:
                    out_img.putpixel((x, y), (0, 0, 0, 0))
        return out_img

def validate_and_process_armor(input_path, output_dir, rework=False):
    """
    Validates an armor PNG against Art Gate criteria
    and promotes it to Content/Art/Armor/ with standardized naming and sidecar metadata.
    """
    fname = os.path.basename(input_path)
    out_name = ARMOR_NAME_MAPPING.get(fname, f"ARM_{fname}" if not fname.startswith("ARM_") else fname)
    out_path = os.path.join(output_dir, out_name)
    meta_path = os.path.join(output_dir, f"{os.path.splitext(out_name)[0]}_meta.json")

    if rework:
        img = rework_armor_to_32x32(input_path)
    else:
        img = Image.open(input_path)

    w, h = img.size
    mode = img.mode
    colors = img.getcolors(maxcolors=2000)
    num_colors = len(colors) if colors else 0
    bbox = img.getbbox()

    issues = []
    if (w, h) not in [(32, 32), (128, 128)]:
        issues.append(f"Invalid dimensions {w}x{h} (expected 32x32 inventory icon or 128x128 rig)")

    semi_trans = []
    if mode == "RGBA":
        for y in range(h):
            for x in range(w):
                r, g, b, a = img.getpixel((x, y))
                if 0 < a < 255:
                    semi_trans.append((x, y, a))
    if semi_trans:
        issues.append(f"{len(semi_trans)} semi-transparent pixels detected (violates binary alpha)")

    thumb = img.resize((16, 16), Image.NEAREST)
    thumb_bbox = thumb.getbbox()
    if not (thumb_bbox and (thumb_bbox[2] - thumb_bbox[0] >= 3) and (thumb_bbox[3] - thumb_bbox[1] >= 3)):
        issues.append("Silhouette legibility failed at 16x16 thumbnail")

    if num_colors > 32 or num_colors < 4:
        issues.append(f"Palette color count ({num_colors if num_colors else '>2000'}) outside accepted range [4, 32]")

    if issues:
        if not rework:
            img.close()
        return False, "REWORK_REQUIRED", issues, None, None

    os.makedirs(output_dir, exist_ok=True)
    img.save(out_path)
    if not rework:
        img.close()

    meta = {
        "asset_name": out_name,
        "type": "armor_equipment",
        "dimensions": [w, h],
        "colors": num_colors,
        "bounding_box": list(bbox) if bbox else [],
        "art_gate_status": "APPROVED",
        "source_legacy_path": os.path.relpath(input_path),
        "target_engine_path": f"/Game/Art/Armor/{os.path.splitext(out_name)[0]}"
    }
    with open(meta_path, "w", encoding="utf-8") as f:
        json.dump(meta, f, indent=2)

    return True, "PASS", [], out_path, meta_path

PAPERDOLL_VISUAL_MAPPING = {
    "T_Hero_StarterCloth.png": {"visual_id": "Visual_StarterCloth", "layer": "BaseBody", "out_name": "PD_Hero_StarterCloth.png"},
    "T_Hero_IronArmor.png": {"visual_id": "Visual_IronArmor", "layer": "ChestArmor", "out_name": "PD_Hero_IronArmor.png"},
    "T_Hero_LeatherRanger.png": {"visual_id": "Visual_LeatherRanger", "layer": "ChestArmor", "out_name": "PD_Hero_LeatherRanger.png"},
    "T_Hero_ArcanistRobe.png": {"visual_id": "Visual_ArcanistRobe", "layer": "ChestArmor", "out_name": "PD_Hero_ArcanistRobe.png"},
}

# Derived so that, combined with foot_pivot Y=114, the implied waist proportion
# ((80-34)/(114-34) = 57.5% down from head-top to foot) lands near the rig's own
# waist_seam_y=80 without needing a manually-guessed source-image waist pixel row.
PAPERDOLL_HEAD_TOP_TARGET_Y = 34

def measure_paperdoll_anchors(img_rgba):
    """
    Measures objective, non-guessed calibration anchors from real alpha-channel
    data: topmost/bottommost opaque row, and the horizontal centerline of the
    head/neck band (NOT the full silhouette bbox, which is skewed by
    asymmetric held props like bows/staves/shields).
    """
    import numpy as np
    arr = np.array(img_rgba)
    alpha = arr[:, :, 3]
    opaque = alpha > 128
    rows = np.where(opaque.any(axis=1))[0]
    head_top_y = int(rows.min())
    foot_bottom_y = int(rows.max())

    head_band = opaque[head_top_y:head_top_y + 110, :]
    row_idx = min(head_top_y + 40, opaque.shape[0] - 1)
    row_mask = opaque[row_idx, :]
    xs = np.where(row_mask)[0]
    row_center_x = (xs.min() + xs.max()) / 2 if len(xs) else None
    ys_b, xs_b = np.where(head_band)
    com_x = float(xs_b.mean()) if len(xs_b) else None
    center_x = (row_center_x + com_x) / 2 if (row_center_x is not None and com_x is not None) else (row_center_x or com_x)

    return head_top_y, foot_bottom_y, center_x

def normalize_paperdoll_layer(input_path):
    """
    Normalizes a legacy high-res (e.g. 1024x1024) continuous-tone Paperdoll
    illustration into a 128x128 production layer aligned to the project's
    master-rig sockets (foot_pivot=(64,114), waist_seam_y=80), using uniform
    isotropic scaling (no distortion), official 4-tone-ramp palette
    quantization, and binary alpha - mirroring the M2 armor rework approach.
    """
    with Image.open(input_path) as raw:
        src = raw.convert("RGBA")

    head_top_y, foot_bottom_y, center_x = measure_paperdoll_anchors(src)
    source_height = foot_bottom_y - head_top_y
    target_height = 114 - PAPERDOLL_HEAD_TOP_TARGET_Y
    scale = target_height / source_height

    new_w = max(1, round(src.width * scale))
    new_h = max(1, round(src.height * scale))
    resized = src.resize((new_w, new_h), Image.Resampling.LANCZOS)

    paste_x = round(64 - center_x * scale)
    paste_y = round(114 - foot_bottom_y * scale)

    canvas = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    canvas.paste(resized, (paste_x, paste_y), resized)

    out_img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    for y in range(128):
        for x in range(128):
            r, g, b, a = canvas.getpixel((x, y))
            if a >= 128:
                c = map_to_official_palette((r, g, b))
                out_img.putpixel((x, y), (c[0], c[1], c[2], 255))
            else:
                out_img.putpixel((x, y), (0, 0, 0, 0))

    metrics = {
        "source_head_top_y": head_top_y,
        "source_foot_bottom_y": foot_bottom_y,
        "source_center_x": center_x,
        "source_height_px": source_height,
        "scale_factor": scale,
        "paste_offset": [paste_x, paste_y],
    }
    return out_img, metrics

def validate_paperdoll_art_gate(img):
    """
    Art Gate for normalized Paperdoll layers: 128x128 geometry, binary alpha,
    opaque coverage at the foot-pivot and waist-seam rows, 16x16 silhouette
    legibility, and palette bounds consistent with weapon/armor Art Gate rules.
    """
    issues = []
    w, h = img.size
    if (w, h) != (128, 128):
        issues.append(f"Invalid dimensions {w}x{h} (expected 128x128)")

    semi_trans = 0
    for y in range(h):
        for x in range(w):
            a = img.getpixel((x, y))[3]
            if 0 < a < 255:
                semi_trans += 1
    if semi_trans:
        issues.append(f"{semi_trans} semi-transparent pixels detected (violates binary alpha)")

    # Tolerance-based check rather than requiring an opaque pixel at the exact
    # (fx, fy) coordinate: a bipedal character in an action stance plants its
    # feet away from the spine centerline, and LANCZOS downsampling softens
    # edge alpha by ~1px. What matters is the silhouette's lowest opaque row
    # (its ground contact) lands within tolerance of the foot_pivot Y.
    fx, fy = SOCKET_METADATA["foot_pivot"]
    FOOT_TOLERANCE_PX = 2
    bbox = img.getbbox()
    if not bbox:
        issues.append("Layer is fully transparent")
    else:
        actual_bottom = bbox[3] - 1
        if abs(actual_bottom - fy) > FOOT_TOLERANCE_PX:
            issues.append(f"Silhouette bottom row {actual_bottom} is more than {FOOT_TOLERANCE_PX}px from foot_pivot Y={fy} (character may be floating/misaligned)")

    waist_y = SOCKET_METADATA["waist_seam_y"]
    waist_row_opaque = any(img.getpixel((x, waist_y))[3] > 0 for x in range(w))
    if not waist_row_opaque:
        issues.append(f"No opaque pixel on waist_seam_y={waist_y} row (unexpected for a torso/full-body layer)")

    thumb = img.resize((16, 16), Image.NEAREST)
    thumb_bbox = thumb.getbbox()
    if not (thumb_bbox and (thumb_bbox[2] - thumb_bbox[0] >= 3) and (thumb_bbox[3] - thumb_bbox[1] >= 3)):
        issues.append("Silhouette legibility failed at 16x16 thumbnail")

    colors = img.convert("RGBA").getcolors(maxcolors=2000)
    num_colors = len(colors) if colors else 0
    if num_colors > 32 or num_colors < 4:
        issues.append(f"Palette color count ({num_colors if num_colors else '>2000'}) outside accepted range [4, 32]")

    return (len(issues) == 0), issues

def process_paperdoll(input_path, output_dir):
    fname = os.path.basename(input_path)
    mapping = PAPERDOLL_VISUAL_MAPPING.get(fname)
    if not mapping:
        return False, "REVIEW_REQUIRED", [f"No PaperdollVisualId mapping known for {fname}"], None, None

    out_name = mapping["out_name"]
    out_path = os.path.join(output_dir, out_name)
    meta_path = os.path.join(output_dir, f"{os.path.splitext(out_name)[0]}_meta.json")

    img, metrics = normalize_paperdoll_layer(input_path)
    passed, issues = validate_paperdoll_art_gate(img)
    if not passed:
        return False, "REWORK_REQUIRED", issues, None, None

    os.makedirs(output_dir, exist_ok=True)
    img.save(out_path)

    meta = {
        "source": os.path.relpath(input_path),
        "production_asset": out_name,
        "canvas": [128, 128],
        "pivot": SOCKET_METADATA["foot_pivot"],
        "waist_y": SOCKET_METADATA["waist_seam_y"],
        "alpha": "binary",
        "lighting": "10_oclock",
        "status": "approved",
        "paperdoll_visual_id": mapping["visual_id"],
        "paperdoll_layer": mapping["layer"],
        "target_engine_path": f"/Game/Art/Characters/Paperdoll/{os.path.splitext(out_name)[0]}",
        "normalization_metrics": metrics,
    }
    with open(meta_path, "w", encoding="utf-8") as f:
        json.dump(meta, f, indent=2)

    return True, "PASS", [], out_path, meta_path

def main():
    parser = argparse.ArgumentParser(description="Aseprite Batch Exporter for Project Ascendant")
    parser.add_argument("--input", "-i", help="Đường dẫn file .aseprite hoặc thư mục")
    parser.add_argument("--output-dir", "-o", default="Content/Art/Characters/MasterRigs/Exported", help="Thư mục xuất xưởng")
    parser.add_argument("--sheet-name", "-n", help="Tên file spritesheet")
    parser.add_argument("--batch", "-b", action="store_true", help="Xử lý hàng loạt toàn bộ file trong thư mục")
    parser.add_argument("--weapons", "-w", action="store_true", help="Xử lý và chuẩn hóa tài nguyên vũ khí tĩnh 32x32 qua Art Gate")
    parser.add_argument("--armor", "-a", action="store_true", help="Xử lý và chuẩn hóa tài nguyên áo giáp qua Art Gate")
    parser.add_argument("--paperdoll", "-p", action="store_true", help="Chuẩn hóa các layer Paperdoll legacy theo master rig (128x128, pivot 64,114, waist Y=80) qua Art Gate")
    parser.add_argument("--rework", "-r", action="store_true", help="Áp dụng quy trình tái thiết pixel-art 32x32 cho tài sản cần sửa đổi")
    args = parser.parse_args()

    if args.paperdoll:
        target = args.input or "Content/art/characters/paperdoll"
        out_dir = args.output_dir if args.output_dir != "Content/Art/Characters/MasterRigs/Exported" else "Content/Art/Characters/Paperdoll"
        if os.path.isdir(target):
            files = sorted([os.path.join(target, f) for f in os.listdir(target)
                             if f.endswith(".png") and os.path.basename(f) in PAPERDOLL_VISUAL_MAPPING])
            print(f"Bắt đầu chuẩn hóa {len(files)} layer Paperdoll...")
            pass_cnt = 0
            fail_cnt = 0
            for f in files:
                success, status, issues, out_p, meta_p = process_paperdoll(f, out_dir)
                if success:
                    print(f"  [PASS] {os.path.basename(f)} -> {os.path.basename(out_p)}")
                    pass_cnt += 1
                else:
                    print(f"  [{status}] {os.path.basename(f)}: {', '.join(issues)}")
                    fail_cnt += 1
            print(f"Hoàn thành chuẩn hóa Paperdoll: {pass_cnt} PASS, {fail_cnt} FAIL/REVIEW.")
        else:
            success, status, issues, out_p, meta_p = process_paperdoll(target, out_dir)
            if success:
                print(f"  [PASS] {target} -> {out_p}")
            else:
                print(f"  [{status}] {target}: {', '.join(issues)}")
        return

    if args.armor:
        target = args.input or "Art_Gallery/legacy/armor"
        out_dir = args.output_dir if args.output_dir != "Content/Art/Characters/MasterRigs/Exported" else "Content/Art/Armor"
        if os.path.isdir(target):
            files = sorted([os.path.join(target, f) for f in os.listdir(target) if f.endswith(".png")])
            print(f"Bắt đầu xử lý {len(files)} tài sản áo giáp (rework={args.rework})...")
            pass_cnt = 0
            fail_cnt = 0
            for f in files:
                success, status, issues, out_p, meta_p = validate_and_process_armor(f, out_dir, rework=args.rework)
                if success:
                    print(f"  [PASS] {os.path.basename(f)} -> {os.path.basename(out_p)}")
                    pass_cnt += 1
                else:
                    print(f"  [{status}] {os.path.basename(f)}: {', '.join(issues)}")
                    fail_cnt += 1
            print(f"Hoàn thành kiểm tra áo giáp: {pass_cnt} PASS, {fail_cnt} REWORK_REQUIRED.")
        else:
            success, status, issues, out_p, meta_p = validate_and_process_armor(target, out_dir, rework=args.rework)
            if success:
                print(f"  [PASS] {target} -> {out_p}")
            else:
                print(f"  [{status}] {target}: {', '.join(issues)}")
        return

    if args.weapons:
        target = args.input or "Art_Gallery/legacy/weapons"
        out_dir = args.output_dir if args.output_dir != "Content/Art/Characters/MasterRigs/Exported" else "Content/Art/Weapons"
        if os.path.isdir(target):
            files = sorted([os.path.join(target, f) for f in os.listdir(target) if f.endswith("_32x32_native.png")])
            print(f"Bắt đầu xử lý {len(files)} vũ khí tĩnh 32x32...")
            pass_cnt = 0
            fail_cnt = 0
            for f in files:
                success, status, issues, out_p, meta_p = validate_and_process_weapon(f, out_dir)
                if success:
                    print(f"  [PASS] {os.path.basename(f)} -> {os.path.basename(out_p)}")
                    pass_cnt += 1
                else:
                    print(f"  [REWORK_REQUIRED] {os.path.basename(f)}: {', '.join(issues)}")
                    fail_cnt += 1
            print(f"Hoàn thành: {pass_cnt} PASS, {fail_cnt} REWORK_REQUIRED.")
        else:
            success, status, issues, out_p, meta_p = validate_and_process_weapon(target, out_dir)
            if success:
                print(f"  [PASS] {target} -> {out_p}")
            else:
                print(f"  [{status}] {target}: {', '.join(issues)}")
        return

    target = args.input or "Tools/Aseprite/templates"
    if args.batch or os.path.isdir(target):
        files = [os.path.join(target, f) for f in sorted(os.listdir(target)) if f.endswith(".aseprite")]
        print(f"Bắt đầu xuất xưởng hàng loạt {len(files)} tệp .aseprite...")
        for f in files:
            export_file(f, args.output_dir)
    else:
        export_file(target, args.output_dir, args.sheet_name)

if __name__ == "__main__":
    main()

