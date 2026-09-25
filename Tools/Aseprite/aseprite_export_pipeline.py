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

def main():
    parser = argparse.ArgumentParser(description="Aseprite Batch Exporter for Project Ascendant")
    parser.add_argument("--input", "-i", help="Đường dẫn file .aseprite hoặc thư mục")
    parser.add_argument("--output-dir", "-o", default="Content/Art/Characters/MasterRigs/Exported", help="Thư mục xuất xưởng")
    parser.add_argument("--sheet-name", "-n", help="Tên file spritesheet")
    parser.add_argument("--batch", "-b", action="store_true", help="Xử lý hàng loạt toàn bộ file trong thư mục")
    args = parser.parse_args()

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
