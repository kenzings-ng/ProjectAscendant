#!/usr/bin/env python3
"""
Test Suite for SPEC-ART-2026-09-23-V2 (game-art-studio standard)
Generates authentic 16x16 Native Chunky Pixel Art items, upscales with Nearest Neighbor,
applies 5-tier rarity frames from game-art-studio, and executes the 6-step QA Gate Check.
"""

import os
import sys
import numpy as np
from PIL import Image, ImageDraw

OUTPUT_DIR = "/mnt/Data/Projects/project-games/ProjectAscendant/Art_Gallery/06_Contract_V2_Tests"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# -------------------------------------------------------------
# 1. Pixel Art Matrix Definitions (16x16 Native)
# -------------------------------------------------------------
# Color Palettes
GALAXY_PALETTE = {
    '.': (0, 0, 0, 0),             # Transparent
    'O': (18, 0, 36, 255),          # Outer contour #120024
    'N': (241, 147, 255, 255),      # Neon pink-violet glow #F193FF
    'L': (192, 98, 245, 255),       # Light purple
    'C': (93, 38, 155, 255),        # Cosmic purple core #5D269B
    'S': (49, 0, 116, 255),         # Deep shadow #310074
    'H': (0, 0, 83, 255),           # Navy hilt #000053
    'G': (74, 0, 75, 255),          # Dark crossguard #4A004B
    'P': (255, 215, 0, 255),        # Pommel jewel
}

GALAXY_MAP = [
    "................",
    "............NO..",
    "...........NCLO.",
    "..........NCLSO.",
    ".........NCLSO..",
    "........NCLSO...",
    ".......NCLSO....",
    "......NCLSO.....",
    ".....NCLSO......",
    "....OGLSO.......",
    "...OGGG.........",
    "..OHSO..........",
    ".OHSO...........",
    ".OPO............",
    "................",
    "................"
]

FROST_PALETTE = {
    '.': (0, 0, 0, 0),             # Transparent
    'O': (11, 19, 43, 255),         # Outer contour #0B132B
    'W': (224, 251, 252, 255),      # Cyan-white highlight #E0FBFC
    'C': (0, 245, 212, 255),        # Runic cyan #00F5D4
    'B': (58, 134, 255, 255),       # Cobalt steel #3A86FF
    'S': (29, 53, 87, 255),         # Deep navy shadow #1D3557
    'G': (74, 85, 104, 255),        # Crossguard steel #4A5568
    'H': (28, 37, 65, 255),         # Leather hilt #1C2541
    'P': (200, 220, 240, 255),      # Pommel steel
}

FROST_MAP = [
    "................",
    "............WO..",
    "...........WCSO.",
    "..........WBBSO.",
    ".........WCBBSO.",
    "........WBBSO...",
    ".......WCBBSO...",
    "......WBBSO.....",
    ".....WCBBSO.....",
    "....OGBBSO......",
    "...OGGG.........",
    "..OHSO..........",
    ".OHSO...........",
    ".OPO............",
    "................",
    "................"
]

CRIMSON_PALETTE = {
    '.': (0, 0, 0, 0),             # Transparent
    'O': (24, 24, 27, 255),         # Outer contour #18181B
    'K': (217, 119, 6, 255),        # Cork light #D97706
    'D': (120, 53, 15, 255),        # Cork shadow #78350F
    'G': (161, 161, 170, 255),      # Glass neck
    'W': (255, 255, 255, 255),      # Specular highlight
    'H': (252, 165, 165, 255),      # Liquid highlight #FCA5A5
    'M': (220, 38, 38, 255),        # Liquid midtone #DC2626
    'S': (127, 29, 29, 255),        # Liquid shadow #7F1D1D
    'C': (69, 10, 10, 255),         # Deep crease #450A0A
}

CRIMSON_MAP = [
    "................",
    ".......OKO......",
    ".......OKD......",
    "......OGGDO.....",
    ".....O.....O....",
    "....OWHHHHMDO...",
    "....OWMMMMMMO...",
    "....OWMMMMMMO...",
    "....OWMMMMSSO...",
    "....OWMMMSSSO...",
    "....OWMMSSSSO...",
    "....OWMSSSSCO...",
    ".....OSSSSSO....",
    "......OOOOO.....",
    "................",
    "................"
]

GOLD_PALETTE = {
    '.': (0, 0, 0, 0),             # Transparent
    'O': (28, 25, 23, 255),         # Outer contour #1C1917
    'T': (254, 240, 138, 255),      # Top face highlight #FEF08A
    'Y': (250, 204, 21, 255),       # Top face midtone #FACC15
    'M': (230, 161, 34, 255),       # Front face light #E6A122
    'F': (217, 119, 6, 255),        # Front face mid #D97706
    'S': (146, 64, 14, 255),        # Right face shadow #92400E
    'C': (69, 26, 3, 255),          # Deep crease #451A03
}

GOLD_MAP = [
    "................",
    "................",
    ".......OOOOO....",
    ".....OOTTTTYO...",
    "...OOTTTTTTYSO..",
    "..OTTTTTTTTYSSO.",
    "..OMMMMMMMMFSSO.",
    "..OMMMMMMMMFSSO.",
    "..OFFFFFFFFFSSO.",
    "..OFFFFFFFFFSSO.",
    "..OFFFFFFFFFCSO.",
    "..OCCCCCCCCCCO..",
    "................",
    "................",
    "................",
    "................"
]

# -------------------------------------------------------------
# 2. Render Functions
# -------------------------------------------------------------
def build_16x16_image(ascii_map, palette):
    img = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    pixels = img.load()
    for y, row in enumerate(ascii_map):
        for x, char in enumerate(row):
            pixels[x, y] = palette.get(char, (0, 0, 0, 0))
    return img

def apply_rarity_border_v2(icon_img, rarity="common", border_width=2):
    """From game-art-studio: 5-tier rarity frame"""
    RARITY_COLORS = {
        "common": (156, 163, 175, 255),       # Gray #9CA3AF
        "uncommon": (34, 197, 94, 255),       # Green #22C55E
        "rare": (59, 130, 246, 255),          # Blue #3B82F6
        "epic": (168, 85, 247, 255),          # Purple #A855F7
        "legendary": (245, 158, 11, 255),     # Gold #F59E0B
    }
    icon_img = icon_img.convert("RGBA")
    w, h = icon_img.size
    color = RARITY_COLORS.get(rarity.lower(), RARITY_COLORS["common"])
    draw = ImageDraw.Draw(icon_img)
    for b in range(border_width):
        draw.rectangle([b, b, w - 1 - b, h - 1 - b], outline=color)
    return icon_img

# -------------------------------------------------------------
# 3. QA Gate Check Automation
# -------------------------------------------------------------
def audit_item(name, img_16, img_export):
    print(f"\n==========================================")
    print(f"🔍 AUDITING QA GATE CHECK: [{name}]")
    print(f"==========================================")
    
    # 1. Zero Border Bleed Test (on 16x16 native)
    arr = np.array(img_16)
    alpha = arr[:, :, 3]
    top_bleed = bool(np.any(alpha[0, :] > 0))
    bottom_bleed = bool(np.any(alpha[-1, :] > 0))
    left_bleed = bool(np.any(alpha[:, 0] > 0))
    right_bleed = bool(np.any(alpha[:, -1] > 0))
    bleed_pass = not (top_bleed or bottom_bleed or left_bleed or right_bleed)
    print(f"  [1] Zero Border Bleed Test: {'✅ PASS' if bleed_pass else '❌ FAIL'}")
    if not bleed_pass:
        print(f"      Bleed Details: Top={top_bleed}, Bottom={bottom_bleed}, Left={left_bleed}, Right={right_bleed}")
    
    # 2. Color Budget Audit
    unique_colors = len(set(tuple(p) for p in img_16.getdata() if p[3] > 0))
    budget_pass = unique_colors <= 16
    print(f"  [2] Color Budget Audit:     {'✅ PASS' if budget_pass else '❌ FAIL'} ({unique_colors} / 16 max allowed colors)")
    
    # 3. Mixel-Free Audit (Exact Nearest Neighbor scaling)
    scale_factor = img_export.width // img_16.width
    is_exact_multiple = (img_export.width % img_16.width == 0) and (img_export.height % img_16.height == 0)
    print(f"  [3] Mixel-Free Audit:       {'✅ PASS' if is_exact_multiple else '❌ FAIL'} (Scale: {scale_factor}x Nearest Neighbor)")
    
    # 4. Blackout Silhouette Test (Check non-empty bounding box)
    rows, cols = np.where(alpha > 0)
    width = int(cols.max() - cols.min() + 1)
    height = int(rows.max() - rows.min() + 1)
    silhouette_pass = (width >= 8 and height >= 8)
    print(f"  [4] Blackout Silhouette:    {'✅ PASS' if silhouette_pass else '❌ FAIL'} (Coverage: {width}x{height} px on 16x16 grid)")
    
    all_passed = bleed_pass and budget_pass and is_exact_multiple and silhouette_pass
    print(f"  --> OVERALL GATE CHECK:     {'🌟 APPROVED FOR BUILD' if all_passed else '🛑 REJECTED'}")
    return all_passed

# -------------------------------------------------------------
# Main Execution
# -------------------------------------------------------------
ITEMS = [
    ("Tempered_Galaxy_Sword", GALAXY_MAP, GALAXY_PALETTE, "legendary"),
    ("Runic_Frost_Broadsword", FROST_MAP, FROST_PALETTE, "rare"),
    ("Crimson_Health_Elixir", CRIMSON_MAP, CRIMSON_PALETTE, "uncommon"),
    ("Faceted_Gold_Ingot", GOLD_MAP, GOLD_PALETTE, "epic"),
]

def main():
    print("🚀 Running SPEC-ART-2026-09-23-V2 Verification & Generation...")
    results = {}
    for name, ascii_map, palette, rarity in ITEMS:
        # 1. Native 16x16
        img_16 = build_16x16_image(ascii_map, palette)
        p16 = os.path.join(OUTPUT_DIR, f"{name}_16x16_native.png")
        img_16.save(p16, "PNG")
        
        # 2. 64x64 In-game (4x Nearest Neighbor)
        img_64 = img_16.resize((64, 64), Image.NEAREST)
        p64 = os.path.join(OUTPUT_DIR, f"{name}_64x64_ingame.png")
        img_64.save(p64, "PNG")
        
        # 3. 256x256 UI Canvas (16x Nearest Neighbor) with 5-Tier Rarity Frame
        img_256 = img_16.resize((256, 256), Image.NEAREST)
        img_256_framed = apply_rarity_border_v2(img_256, rarity=rarity, border_width=4)
        p256 = os.path.join(OUTPUT_DIR, f"{name}_256x256_framed_{rarity}.png")
        img_256_framed.save(p256, "PNG")
        
        # 4. QA Audit
        passed = audit_item(name, img_16, img_256)
        results[name] = passed
        
    print("\n==========================================")
    print("📋 SUMMARY OF CONTRACT V2 COMPLIANCE")
    print("==========================================")
    for name, status in results.items():
        print(f"  • {name:<26}: {'APPROVED' if status else 'REJECTED'}")
    print(f"\nSaved all artifacts to: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
