#!/usr/bin/env python3
"""
Living Equipment Mutation Matrix Generator
Implements Section 7 of SPEC-ART-2026-09-23-V2 & game-art-studio:
Demonstrates how 1 single weapon archetype (Broadsword 32x32) produces a rich matrix
of in-game variants based on enhancement tier (+0 to +10), elemental gem sockets,
and dynamic combat overlays (blood decals).
"""

import os
import sys
import numpy as np
from PIL import Image, ImageDraw

OUTPUT_DIR = "/mnt/Data/Projects/project-games/ProjectAscendant/Art_Gallery/08_Weapon_Mutation_Matrix"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# -------------------------------------------------------------
# Base Sword Matrix (32x32 Geometry)
# -------------------------------------------------------------
# Character codes:
# '.' : Transparent
# 'O' : Outer Contour
# 'H' : Highlight Edge
# 'M' : Midtone Blade
# 'S' : Core Shadow
# 'D' : Deep Crease / Runic Groove
# 'G' : Crossguard
# 'K' : Hilt / Grip
# 'P' : Pommel
# 'J' : Gem Socket Jewel

def generate_sword_variant(blade_colors, guard_color, hilt_color, gem_color, overlay=None):
    """
    Renders 32x32 sword with specific palette and optional combat overlay.
    blade_colors: (contour, highlight, midtone, shadow, deep_crease)
    """
    c_out, c_hi, c_mid, c_shd, c_crs = blade_colors
    img = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    
    # 45-degree diagonal from (5, 26) to (26, 5)
    for t in range(21):
        x = 5 + t
        y = 26 - t
        # Crossguard at t=5..7
        if 4 <= t <= 8:
            for d in range(-4, 5):
                img.putpixel((x - d, y - d), c_out)
        # Hilt & Pommel at t <= 3
        elif t <= 3:
            img.putpixel((x, y), c_out)
            img.putpixel((x+1, y), c_out)
            img.putpixel((x, y+1), c_out)
        # Blade contour at t >= 9
        else:
            for w in [-2, -1, 0, 1, 2]:
                px = x - w
                py = y - w
                if 0 <= px < 32 and 0 <= py < 32:
                    img.putpixel((px, py), c_out)
                    
    # Fill Blade Interior
    for t in range(9, 21):
        x = 5 + t
        y = 26 - t
        img.putpixel((x - 1, y - 1), c_hi)
        # Center fuller / rune line
        if t % 3 == 0 and c_crs != c_mid:
            img.putpixel((x, y), c_crs)
        else:
            img.putpixel((x, y), c_mid)
        img.putpixel((x + 1, y + 1), c_shd)
        if t < 20:
            img.putpixel((x + 2, y + 2), c_out)
            
    # Fill Crossguard
    for d in range(-3, 4):
        x = 5 + 6 - d
        y = 26 - 6 - d
        if 0 <= x < 32 and 0 <= y < 32:
            img.putpixel((x, y), guard_color)
    # Center Gem
    img.putpixel((11, 20), gem_color)
    img.putpixel((12, 19), gem_color)
    
    # Fill Hilt
    img.putpixel((7, 24), hilt_color[0])
    img.putpixel((8, 23), hilt_color[1])
    img.putpixel((6, 25), hilt_color[2])
    # Pommel
    img.putpixel((5, 26), guard_color)
    
    # Apply Combat Overlays
    if overlay == "blood":
        # Blood splatter decal on blade tip and edges
        blood_color = (185, 28, 28, 240)
        blood_dark  = (127, 29, 29, 240)
        img.putpixel((23, 8), blood_color)
        img.putpixel((24, 7), blood_color)
        img.putpixel((25, 6), blood_dark)
        img.putpixel((22, 9), blood_color)
        img.putpixel((19, 12), blood_color)
        img.putpixel((18, 13), blood_dark)
    elif overlay == "broken":
        # Chipped edge
        img.putpixel((17, 14), (0, 0, 0, 0)) # Chipped pixel
        img.putpixel((18, 13), (30, 30, 30, 255))
        
    return img

def apply_rarity_border(icon_img, rarity="common", border_width=4):
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

# -------------------------------------------------------------
# 6 Living Mutation Variants
# -------------------------------------------------------------
MUTATION_CONFIGS = [
    (
        "01_Broadsword_Tier0_RustedIron",
        # c_out, c_hi, c_mid, c_shd, c_crs
        ((40, 30, 25, 255), (148, 130, 115, 255), (100, 85, 75, 255), (60, 50, 45, 255), (45, 35, 30, 255)),
        (80, 70, 60, 255),
        ((70, 50, 35, 255), (90, 65, 45, 255), (50, 35, 25, 255)),
        (50, 45, 40, 255),
        "broken",
        "common",
        "Level 0: Kiếm Sắt Thô Sứt Mẻ (+0) — Vũ khí khởi đầu chưa qua mài giũa, lưỡi mẻ răng cưa, hoen rỉ."
    ),
    (
        "02_Broadsword_Tier1_SharpSteel",
        ((15, 23, 42, 255), (241, 245, 249, 255), (148, 163, 184, 255), (71, 85, 105, 255), (51, 65, 85, 255)),
        (100, 116, 139, 255),
        ((120, 53, 15, 255), (180, 83, 9, 255), (69, 26, 3, 255)),
        (100, 116, 139, 255),
        None,
        "uncommon",
        "Level 1: Thép Mài Sắc Bén (+3) — Qua lò rèn tiền trạm, lưỡi mài sáng loáng phản xạ ánh kim, chuôi da mới."
    ),
    (
        "03_Broadsword_Tier2_RunicFlame",
        ((35, 12, 5, 255), (254, 240, 138, 255), (245, 158, 11, 255), (220, 38, 38, 255), (255, 255, 0, 255)),
        (230, 161, 34, 255),
        ((120, 53, 15, 255), (180, 83, 9, 255), (69, 26, 3, 255)),
        (220, 38, 38, 255),
        None,
        "rare",
        "Level 2: Hỏa Diệm Rực Lửa (+6 Khảm Ruby) — Lưỡi kiếm rực nhiệt lượng, rãnh cổ ngữ khắc dọc thân bốc khói than hồng."
    ),
    (
        "04_Broadsword_Tier2_GlacialIce",
        ((11, 19, 43, 255), (224, 251, 252, 255), (58, 134, 255, 255), (29, 53, 87, 255), (0, 245, 212, 255)),
        (71, 85, 105, 255),
        ((30, 41, 59, 255), (71, 85, 105, 255), (15, 23, 42, 255)),
        (0, 245, 212, 255),
        None,
        "rare",
        "Level 2: Băng Tinh Sương Lạnh (+6 Khảm Sapphire) — Thép xanh cobalt đóng băng tinh thể ngọc, cổ ngữ cyan phát quang."
    ),
    (
        "05_Broadsword_Tier3_DivineAscendant",
        ((25, 0, 50, 255), (241, 147, 255, 255), (140, 56, 222, 255), (93, 38, 155, 255), (254, 240, 138, 255)),
        (234, 179, 8, 255),
        ((69, 10, 10, 255), (153, 27, 27, 255), (40, 5, 5, 255)),
        (254, 240, 138, 255),
        None,
        "legendary",
        "Level 3: Thần Binh Tối Thượng (+10 Divine) — Rèn từ Linh hồn Lãnh chúa tại Lò Rèn Cấm Địa. Tinh thể vũ trụ phát quang plasma."
    ),
    (
        "06_Broadsword_Combat_Bloodied",
        ((15, 23, 42, 255), (241, 245, 249, 255), (148, 163, 184, 255), (71, 85, 105, 255), (51, 65, 85, 255)),
        (100, 116, 139, 255),
        ((120, 53, 15, 255), (180, 83, 9, 255), (69, 26, 3, 255)),
        (100, 116, 139, 255),
        "blood",
        "uncommon",
        "Action State: Chiến Trường Nhuốm Máu — Kích hoạt sau Combo 3-Hit tiêu diệt quái. Vết máu tươi loang lổ trên lưỡi kiếm."
    ),
]

def main():
    print("🚀 Generating Living Equipment Mutation Matrix (32x32)...")
    for name, blade, guard, hilt, gem, overlay, rarity, desc in MUTATION_CONFIGS:
        img_32 = generate_sword_variant(blade, guard, hilt, gem, overlay=overlay)
        
        # Save 32x32 native
        p32 = os.path.join(OUTPUT_DIR, f"{name}_32x32_native.png")
        img_32.save(p32, "PNG")
        
        # Save 64x64 in-game
        img_64 = img_32.resize((64, 64), Image.NEAREST)
        p64 = os.path.join(OUTPUT_DIR, f"{name}_64x64_ingame.png")
        img_64.save(p64, "PNG")
        
        # Save 256x256 framed UI
        img_256 = img_32.resize((256, 256), Image.NEAREST)
        img_256_framed = apply_rarity_border(img_256, rarity=rarity, border_width=4)
        p256 = os.path.join(OUTPUT_DIR, f"{name}_256x256_framed_{rarity}.png")
        img_256_framed.save(p256, "PNG")
        
        print(f"  ✅ Generated: {name} [{rarity.upper()}]")
        
    print(f"\nAll 6 mutation variants saved to: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
