#!/usr/bin/env python3
# ==============================================================================
# environment_art_gate.py: Art Gate Validator for Environment & Foliage Assets
# Package: M6 — Environment & Vegetation Production Foundation
# Project: Project Ascendant (2.5D Isometric Hardcore ARPG MMO on UE 5.8)
# Conforms to: SPEC-ART-2026-09-23-V2
# ==============================================================================

import os
import sys
import glob
import json
from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
from aseprite_export_pipeline import parse_aseprite_file

MAX_PALETTE_COLORS = 32
MIN_PALETTE_COLORS = 2

def check_binary_alpha(img, name="asset"):
    w, h = img.size
    for y in range(h):
        for x in range(w):
            a = img.getpixel((x, y))[3]
            if 0 < a < 255:
                return False, f"{name}: semi-transparent pixel at ({x}, {y}) with alpha={a} (violates binary alpha)"
    return True, None

def check_palette_budget(img, name="asset"):
    colors = set()
    for c in (img.convert("RGBA").getcolors(maxcolors=100000) or []):
        colors.add(c[1])
    colors.discard((0, 0, 0, 0))
    if len(colors) > MAX_PALETTE_COLORS:
        return False, f"{name}: palette color count {len(colors)} exceeds limit of {MAX_PALETTE_COLORS}"
    if len(colors) < MIN_PALETTE_COLORS:
        return False, f"{name}: palette color count {len(colors)} is suspiciously empty (< {MIN_PALETTE_COLORS})"
    return True, len(colors)

def validate_environment_package(env_dir="Content/Art/Environment"):
    issues = []
    warnings = []
    inspected_count = 0
    passed_count = 0

    print("================================================================================")
    print("Environment Art Gate Validation — Package M6")
    print(f"Target Directory: {env_dir}")
    print("================================================================================")

    # 1. Inspect Individual Sprites
    png_files = sorted(glob.glob(f"{env_dir}/Sprites/**/*.png", recursive=True))
    if not png_files:
        return False, ["No PNG files found in Sprites directory!"]

    meta_file = f"{env_dir}/metadata/environment_metadata.json"
    metadata = {}
    if os.path.exists(meta_file):
        with open(meta_file, "r") as fp:
            metadata = json.load(fp)

    for p in png_files:
        inspected_count += 1
        rel_name = os.path.relpath(p, env_dir)
        try:
            img = Image.open(p).convert("RGBA")
        except Exception as e:
            issues.append(f"{rel_name}: failed to open image ({e})")
            continue

        # Alpha check
        ok, err = check_binary_alpha(img, rel_name)
        if not ok:
            issues.append(err)
            continue

        # Palette check
        ok, p_info = check_palette_budget(img, rel_name)
        if not ok:
            issues.append(p_info)
            continue

        # Bounding box check (non-empty)
        bbox = img.getbbox()
        if not bbox:
            issues.append(f"{rel_name}: fully transparent image (no art content)")
            continue

        # Dimensions check
        w, h = img.size
        if "Tiles" in p:
            if (w, h) != (32, 32):
                issues.append(f"{rel_name}: tile dimensions {w}x{h} != expected 32x32")
                continue
        elif "environment_terrain_tileset.png" in p:
            if (w, h) != (128, 128):
                issues.append(f"{rel_name}: atlas dimensions {w}x{h} != expected 128x128")
                continue

        passed_count += 1

    # 2. Inspect Aseprite Master Sources
    ase_files = sorted(glob.glob(f"{env_dir}/Source/*.aseprite"))
    for af in ase_files:
        inspected_count += 1
        rel_name = os.path.relpath(af, env_dir)
        try:
            parsed = parse_aseprite_file(af)
            if not parsed or parsed["num_frames"] < 1:
                issues.append(f"{rel_name}: parsed 0 frames from .aseprite file")
                continue
            passed_count += 1
        except Exception as e:
            issues.append(f"{rel_name}: parse error ({e})")

    # 3. Report
    print(f"\nInspection Summary:")
    print(f"  Total Assets Inspected: {inspected_count}")
    print(f"  Passed:                 {passed_count}")
    print(f"  Failed:                 {len(issues)}")

    if issues:
        print("\nArt Gate: FAIL")
        for i in issues:
            print(f"  [ISSUE] {i}")
        return False, issues
    else:
        print("\nArt Gate: PASS")
        print("  All environment assets satisfy binary alpha, palette budget, and grid constraints!")
        return True, []

if __name__ == "__main__":
    passed, issues = validate_environment_package()
    sys.exit(0 if passed else 1)
