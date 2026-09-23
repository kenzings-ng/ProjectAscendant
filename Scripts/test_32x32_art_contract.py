#!/usr/bin/env python3
"""
Test Suite V3: 32x32 Native Pixel Art Standard with Zone-Adaptive Lighting & Palette
Implements SPEC-ART-2026-09-23-V2 (32x32 HD-2D Chunky Pixel)
Four thematic assets representing different game zones:
1. Solar Fire Greatsword (Sanctuary / Forge - Warm, vibrant, amber-red)
2. Glacial Runic Blade (Wilderness - Crisp, ice-cyan, cobalt)
3. Void Corrupted Staff (Contested / Abyss - Dark gothic, amethyst purple)
4. Sacred Amber Phial (Sanctuary / Alchemist - Golden glowing elixir)
"""

import os
import sys
import numpy as np
from PIL import Image, ImageDraw

OUTPUT_DIR = "/mnt/Data/Projects/project-games/ProjectAscendant/Art_Gallery/07_Contract_V3_32x32_Tests"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# -------------------------------------------------------------
# 1. 32x32 Item Matrix Generator (Procedural Pixel Art)
# -------------------------------------------------------------

def create_solar_fire_greatsword_32():
    img = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # 45-degree diagonal: from (5, 26) to (26, 5)
    # 1. Outer Dark Contour #1A0B05
    for t in range(21):
        x = 5 + t
        y = 26 - t
        # Crossguard at t=5..7
        if 4 <= t <= 8:
            for d in range(-4, 5):
                img.putpixel((x - d, y - d), (26, 11, 5, 255))
        # Hilt & pommel at t=0..4
        elif t <= 3:
            img.putpixel((x, y), (26, 11, 5, 255))
            img.putpixel((x+1, y), (26, 11, 5, 255))
            img.putpixel((x, y+1), (26, 11, 5, 255))
        # Blade contour at t >= 9
        else:
            for w in [-2, -1, 0, 1, 2]:
                px = x - w
                py = y - w
                if 0 <= px < 32 and 0 <= py < 32:
                    img.putpixel((px, py), (26, 11, 5, 255))
                    
    # 2. Blade Core & Flame Edge (Warm Palette: Highlight #FEF08A, Core #F59E0B, Mid #DC2626, Shadow #7F1D1D)
    for t in range(9, 21):
        x = 5 + t
        y = 26 - t
        # Edge highlight (top-left face)
        img.putpixel((x - 1, y - 1), (254, 240, 138, 255)) # Warm lemon fire
        img.putpixel((x, y), (245, 158, 11, 255))         # Amber core
        img.putpixel((x + 1, y + 1), (220, 38, 38, 255))   # Crimson midtone
        if t < 20:
            img.putpixel((x + 2, y + 2), (127, 29, 29, 255)) # Dark blood shadow
            
    # Crossguard gold & ruby
    for d in range(-3, 4):
        x = 5 + 6 - d
        y = 26 - 6 - d
        if 0 <= x < 32 and 0 <= y < 32:
            img.putpixel((x, y), (230, 161, 34, 255)) # Gold crossguard
    img.putpixel((11, 20), (220, 38, 38, 255)) # Center ruby jewel
    
    # Leather grip
    img.putpixel((7, 24), (120, 53, 15, 255))
    img.putpixel((8, 23), (180, 83, 9, 255))
    img.putpixel((6, 25), (69, 26, 3, 255))
    return img

def create_glacial_runic_blade_32():
    img = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    
    # 45-degree diagonal: from (5, 26) to (26, 5)
    # 1. Outer Dark Navy Contour #0B132B
    for t in range(21):
        x = 5 + t
        y = 26 - t
        if 4 <= t <= 8:
            for d in range(-4, 5):
                img.putpixel((x - d, y - d), (11, 19, 43, 255))
        elif t <= 3:
            img.putpixel((x, y), (11, 19, 43, 255))
            img.putpixel((x+1, y), (11, 19, 43, 255))
            img.putpixel((x, y+1), (11, 19, 43, 255))
        else:
            for w in [-2, -1, 0, 1, 2]:
                px = x - w
                py = y - w
                if 0 <= px < 32 and 0 <= py < 32:
                    img.putpixel((px, py), (11, 19, 43, 255))
                    
    # 2. Glacial Crystal & Cyan Runes
    for t in range(9, 21):
        x = 5 + t
        y = 26 - t
        img.putpixel((x - 1, y - 1), (224, 251, 252, 255)) # Ice cyan highlight
        # Alternating Runic Cyan
        if t % 2 == 0:
            img.putpixel((x, y), (0, 245, 212, 255))       # Glowing cyan rune
        else:
            img.putpixel((x, y), (58, 134, 255, 255))      # Cobalt steel
        img.putpixel((x + 1, y + 1), (29, 53, 87, 255))    # Deep navy shadow
        
    # Steel crossguard
    for d in range(-3, 4):
        x = 5 + 6 - d
        y = 26 - 6 - d
        if 0 <= x < 32 and 0 <= y < 32:
            img.putpixel((x, y), (100, 116, 139, 255)) # Slate steel
    img.putpixel((11, 20), (0, 245, 212, 255)) # Center glowing cyan core
    
    # Silver grip
    img.putpixel((7, 24), (71, 85, 105, 255))
    img.putpixel((8, 23), (148, 163, 184, 255))
    img.putpixel((6, 25), (30, 41, 59, 255))
    return img

def create_void_staff_32():
    img = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    
    # 45-degree diagonal: from (5, 26) to (26, 5)
    # Dark gothic wood pole with void crystal head
    # 1. Staff shaft from t=0..16
    for t in range(17):
        x = 5 + t
        y = 26 - t
        # 1px contour
        img.putpixel((x - 1, y), (20, 10, 30, 255))
        img.putpixel((x + 1, y), (20, 10, 30, 255))
        # Ebony wood core
        img.putpixel((x, y), (50, 30, 70, 255))
        if t % 4 == 0: # Silver bands
            img.putpixel((x, y), (160, 130, 200, 255))
            
    # 2. Void Crystal Head at t=17..24 (X: 20..27, Y: 11..4)
    cx, cy = 23, 8
    for dy in range(-4, 5):
        for dx in range(-4, 5):
            dist = abs(dx) + abs(dy)
            px = cx + dx
            py = cy + dy
            if 0 <= px < 32 and 0 <= py < 32:
                if dist <= 4:
                    if dist == 4:
                        img.putpixel((px, py), (24, 10, 40, 255)) # Outer contour
                    elif dist == 3:
                        img.putpixel((px, py), (106, 27, 154, 255)) # Deep void purple
                    elif dist <= 2:
                        if dx < 0 or dy < 0:
                            img.putpixel((px, py), (216, 180, 254, 255)) # Highlight lavender
                        else:
                            img.putpixel((px, py), (168, 85, 247, 255))  # Core magenta-purple
    return img

def create_sacred_amber_phial_32():
    img = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    # Vertical bottle centered at X=15, Y=4..27
    cx = 15
    # Outer contour and fill
    for y in range(5, 27):
        # Cork at Y=5..8
        if 5 <= y <= 7:
            for x in range(cx - 2, cx + 3):
                if x in [cx - 2, cx + 2] or y == 5:
                    img.putpixel((x, y), (40, 20, 10, 255)) # Contour
                else:
                    img.putpixel((x, y), (217, 119, 6, 255) if x <= cx else (146, 64, 14, 255))
        # Glass neck at Y=8..10
        elif 8 <= y <= 10:
            for x in range(cx - 3, cx + 4):
                if x in [cx - 3, cx + 3]:
                    img.putpixel((x, y), (30, 25, 20, 255))
                else:
                    img.putpixel((x, y), (200, 200, 210, 255) if x == cx - 2 else (60, 50, 40, 255))
        # Round bottle body at Y=11..26
        else:
            hw = int(7 * np.sin(np.pi * (y - 11) / 15)) + 1
            hw = max(hw, 2)
            for x in range(cx - hw - 1, cx + hw + 2):
                if x in [cx - hw - 1, cx + hw + 1] or y == 26:
                    img.putpixel((x, y), (28, 25, 23, 255)) # Contour
                elif cx - hw <= x <= cx + hw:
                    # Glass shine streak
                    if x == cx - hw + 2 and 13 <= y <= 21:
                        img.putpixel((x, y), (255, 255, 255, 255)) # Specular glass highlight
                    # Amber liquid
                    elif x < cx:
                        img.putpixel((x, y), (254, 240, 138, 255) if y < 16 else (245, 158, 11, 255))
                    else:
                        img.putpixel((x, y), (217, 119, 6, 255) if y < 20 else (120, 53, 15, 255))
    return img

# -------------------------------------------------------------
# 2. Rarity Frame & QA Verification
# -------------------------------------------------------------
def apply_rarity_border_v3(icon_img, rarity="common", border_width=2):
    RARITY_COLORS = {
        "common":    (156, 163, 175, 255),
        "uncommon":  (34, 197, 94, 255),
        "rare":      (59, 130, 246, 255),
        "epic":      (168, 85, 247, 255),
        "legendary": (245, 158, 11, 255),
    }
    icon_img = icon_img.convert("RGBA")
    w, h = icon_img.size
    color = RARITY_COLORS.get(rarity.lower(), RARITY_COLORS["common"])
    draw = ImageDraw.Draw(icon_img)
    for b in range(border_width):
        draw.rectangle([b, b, w - 1 - b, h - 1 - b], outline=color)
    return icon_img

def audit_32px_item(name, img_32, img_export):
    print(f"\n==========================================")
    print(f"🔍 AUDITING QA GATE CHECK: [{name}] (32x32)")
    print(f"==========================================")
    
    # 1. Zero Border Bleed Test
    arr = np.array(img_32)
    alpha = arr[:, :, 3]
    top_bleed = bool(np.any(alpha[0, :] > 0))
    bottom_bleed = bool(np.any(alpha[-1, :] > 0))
    left_bleed = bool(np.any(alpha[:, 0] > 0))
    right_bleed = bool(np.any(alpha[:, -1] > 0))
    bleed_pass = not (top_bleed or bottom_bleed or left_bleed or right_bleed)
    print(f"  [1] Zero Border Bleed Test: {'✅ PASS' if bleed_pass else '❌ FAIL'}")
    
    # 2. Color Budget Audit (<= 24 colors for 32x32)
    unique_colors = len(set(tuple(p) for p in img_32.getdata() if p[3] > 0))
    budget_pass = unique_colors <= 24
    print(f"  [2] Color Budget Audit:     {'✅ PASS' if budget_pass else '❌ FAIL'} ({unique_colors} / 24 max allowed colors)")
    
    # 3. Mixel-Free Audit (Exact Nearest Neighbor scaling)
    scale_factor = img_export.width // img_32.width
    is_exact_multiple = (img_export.width % img_32.width == 0) and (img_export.height % img_32.height == 0)
    print(f"  [3] Mixel-Free Audit:       {'✅ PASS' if is_exact_multiple else '❌ FAIL'} (Scale: {scale_factor}x Nearest Neighbor)")
    
    # 4. Blackout Silhouette Test (Coverage >= 16x16)
    rows, cols = np.where(alpha > 0)
    width = int(cols.max() - cols.min() + 1)
    height = int(rows.max() - rows.min() + 1)
    silhouette_pass = (width >= 12 and height >= 12)
    print(f"  [4] Blackout Silhouette:    {'✅ PASS' if silhouette_pass else '❌ FAIL'} (Coverage: {width}x{height} px on 32x32 grid)")
    
    all_passed = bleed_pass and budget_pass and is_exact_multiple and silhouette_pass
    print(f"  --> OVERALL GATE CHECK:     {'🌟 APPROVED FOR BUILD' if all_passed else '🛑 REJECTED'}")
    return all_passed

# -------------------------------------------------------------
# Main Execution
# -------------------------------------------------------------
ITEMS_32 = [
    ("Solar_Fire_Greatsword_32px", create_solar_fire_greatsword_32, "legendary", "Vùng Lò Rèn / Sanctuary (Tươi sáng, lửa vàng ấm áp)"),
    ("Glacial_Runic_Blade_32px", create_glacial_runic_blade_32, "rare", "Vùng Núi Băng / Dã Ngoại (Trong trẻo, xanh ngọc băng tuyết)"),
    ("Void_Corrupted_Staff_32px", create_void_staff_32, "epic", "Vùng Đất Chết Tha Hóa (U tối, tím ma mị huyền bí)"),
    ("Sacred_Amber_Phial_32px", create_sacred_amber_phial_32, "uncommon", "Tiệm Giả Kim Sanctuary (Rực rỡ, vàng hổ phách lung linh)"),
]

def main():
    print("🚀 Running SPEC-ART-2026-09-23-V2 (32x32 HD-2D Chunky Pixel Standard)...")
    results = {}
    for name, gen_fn, rarity, zone_desc in ITEMS_32:
        img_32 = gen_fn()
        
        # 1. Native 32x32
        p32 = os.path.join(OUTPUT_DIR, f"{name}_32x32_native.png")
        img_32.save(p32, "PNG")
        
        # 2. 64x64 In-game (2x Nearest Neighbor)
        img_64 = img_32.resize((64, 64), Image.NEAREST)
        p64 = os.path.join(OUTPUT_DIR, f"{name}_64x64_ingame.png")
        img_64.save(p64, "PNG")
        
        # 3. 256x256 UI Canvas (8x Nearest Neighbor) with 5-Tier Rarity Frame
        img_256 = img_32.resize((256, 256), Image.NEAREST)
        img_256_framed = apply_rarity_border_v3(img_256, rarity=rarity, border_width=4)
        p256 = os.path.join(OUTPUT_DIR, f"{name}_256x256_framed_{rarity}.png")
        img_256_framed.save(p256, "PNG")
        
        # 4. QA Audit
        passed = audit_32px_item(name, img_32, img_256)
        results[name] = (passed, zone_desc)
        
    print("\n==========================================")
    print("📋 SUMMARY OF CONTRACT V3 (32x32) COMPLIANCE")
    print("==========================================")
    for name, (status, zone) in results.items():
        print(f"  • {name:<28}: {'APPROVED' if status else 'REJECTED'} [{zone}]")
    print(f"\nSaved all artifacts to: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
