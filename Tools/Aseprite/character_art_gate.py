#!/usr/bin/env python3
# ==============================================================================
# character_art_gate.py: Art Gate validator for full character animation sets
# Package: M5 - New Vanguard Character Production Foundation
# Project: Project Ascendant (2.5D Isometric Hardcore ARPG MMO on UE 5.8)
#
# Deterministic, automatable checks only. Standalone module - does not modify
# aseprite_export_pipeline.py - reuses its SOCKET_METADATA and
# parse_aseprite_file()/export_file() as the source of truth for geometry.
#
# What this CANNOT check (left as explicit manual QA - see MANUAL_QA_CHECKLIST
# below): whether the art is well-drawn, whether the 10-o'clock lighting
# direction is actually followed, whether motion reads as intentional per
# frame, whether the silhouette is *appealing* at reduced sizes (only whether
# it clears a minimum non-empty-pixel-count bar). Do not represent a PASS from
# this module as a substitute for human visual review.
# ==============================================================================

import os
import sys
from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
from aseprite_export_pipeline import SOCKET_METADATA, parse_aseprite_file

TARGET_FRAME_COUNTS = {
    "Idle": (4, 4),
    "Walk": (6, 6),
    "Run": (6, 6),
    "Attack": (6, 10),
}

FOOT_TOLERANCE_PX = 2
MAX_PALETTE_COLORS = 32
MIN_PALETTE_COLORS = 4

MANUAL_QA_CHECKLIST = [
    "10 o'clock lighting direction is actually followed (not automatable - requires human visual review)",
    "Motion reads as intentional per frame, not mechanical looping (Attack anticipation/action/impact/recovery beats are readable)",
    "Silhouette is genuinely appealing/readable at 64x64, 32x32, 16x16 - this module only checks a minimum non-empty pixel-count bar, not visual quality",
    "Material separation (skin/cloth/leather/metal/hair) reads clearly, not just 'palette count is within range'",
    "Facing-direction convention matches what the existing runtime actually expects (see M5 report Section 6)",
]

def _opaque_bbox(img):
    return img.getbbox()

def _binary_alpha_ok(img):
    w, h = img.size
    for y in range(h):
        for x in range(w):
            a = img.getpixel((x, y))[3]
            if 0 < a < 255:
                return False, (x, y, a)
    return True, None

def validate_character_animation_art_gate(frame_images, tags):
    """
    frame_images: list of PIL RGBA Image objects, one per frame, in the same
                  order/index as the source .aseprite frames.
    tags: list of {"name","from","to"} dicts as returned by
          aseprite_export_pipeline.parse_aseprite_file().

    Returns (passed: bool, issues: list[str], warnings: list[str]).
    `issues` are hard Art Gate failures. `warnings` are anomalies worth a
    human look but not automatic rejections (frame-count padding heuristics,
    silhouette-area outliers) - matching the project's existing convention of
    flagging judgment calls rather than silently auto-rejecting them.
    """
    issues = []
    warnings = []

    if not frame_images:
        return False, ["No frames supplied - nothing to validate"], []

    fx, fy = SOCKET_METADATA["foot_pivot"]
    waist_y = SOCKET_METADATA["waist_seam_y"]
    canvas_w, canvas_h = SOCKET_METADATA["native_grid"]

    foot_rows = []
    for i, img in enumerate(frame_images):
        w, h = img.size
        if (w, h) != (canvas_w, canvas_h):
            issues.append(f"Frame {i}: dimensions {w}x{h} != expected {canvas_w}x{canvas_h}")
            continue

        ok, bad_px = _binary_alpha_ok(img)
        if not ok:
            issues.append(f"Frame {i}: semi-transparent pixel at {bad_px} (violates binary alpha)")

        bbox = _opaque_bbox(img)
        if not bbox:
            issues.append(f"Frame {i}: fully transparent (no character content)")
            continue

        actual_bottom = bbox[3] - 1
        foot_rows.append(actual_bottom)
        if abs(actual_bottom - fy) > FOOT_TOLERANCE_PX:
            issues.append(f"Frame {i}: silhouette bottom row {actual_bottom} more than {FOOT_TOLERANCE_PX}px from foot_pivot Y={fy} (pivot drift)")

        waist_row_opaque = any(img.getpixel((x, waist_y))[3] > 0 for x in range(canvas_w))
        if not waist_row_opaque:
            issues.append(f"Frame {i}: no opaque pixel on waist_seam_y={waist_y} row")

        for size, min_extent in [(64, 6), (32, 3), (16, 2)]:
            thumb = img.resize((size, size), Image.NEAREST)
            tb = thumb.getbbox()
            if not (tb and (tb[2] - tb[0] >= min_extent) and (tb[3] - tb[1] >= min_extent)):
                issues.append(f"Frame {i}: silhouette illegible at {size}x{size} thumbnail")

    if foot_rows and (max(foot_rows) - min(foot_rows) > FOOT_TOLERANCE_PX):
        warnings.append(f"Foot contact row varies by {max(foot_rows) - min(foot_rows)}px across all frames (expected: consistent ground contact for Idle/Walk/Run/Attack, no jump content)")

    all_colors = set()
    for img in frame_images:
        for c in (img.convert("RGBA").getcolors(maxcolors=100000) or []):
            all_colors.add(c[1])
    all_colors.discard((0, 0, 0, 0))
    if len(all_colors) > MAX_PALETTE_COLORS or len(all_colors) < MIN_PALETTE_COLORS:
        issues.append(f"Combined palette across all frames has {len(all_colors)} colors, outside accepted range [{MIN_PALETTE_COLORS}, {MAX_PALETTE_COLORS}]")

    for tag in tags:
        name = tag["name"]
        count = tag["to"] - tag["from"] + 1
        if name in TARGET_FRAME_COUNTS:
            lo, hi = TARGET_FRAME_COUNTS[name]
            if not (lo <= count <= hi):
                issues.append(f"Tag '{name}': {count} frames, outside target range [{lo}, {hi}]")
        else:
            warnings.append(f"Tag '{name}' has no target frame-count convention defined - not validated")

        seq = frame_images[tag["from"]:tag["to"] + 1]
        for i in range(len(seq) - 1):
            if seq[i].tobytes() == seq[i + 1].tobytes():
                warnings.append(f"Tag '{name}': frames {tag['from']+i} and {tag['from']+i+1} are pixel-identical (possible padding - every frame should carry intentional motion)")

    return (len(issues) == 0), issues, warnings

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: character_art_gate.py <path-to-.aseprite>")
        sys.exit(1)
    parsed = parse_aseprite_file(sys.argv[1])
    passed, issues, warnings = validate_character_animation_art_gate(parsed["frame_images"], parsed["tags"])
    print(f"Art Gate: {'PASS' if passed else 'FAIL'}")
    for i in issues:
        print(f"  [ISSUE] {i}")
    for w in warnings:
        print(f"  [WARN]  {w}")
    print("\nManual QA checklist (not automatable):")
    for m in MANUAL_QA_CHECKLIST:
        print(f"  - {m}")
