#!/usr/bin/env python3
# ==============================================================================
# generate_environment_assets.py: Production Environment & Foliage Generator
# Project: Project Ascendant (2.5D Isometric Hardcore ARPG MMO on UE 5.8)
# Conforms to: SPEC-ART-2026-09-23-V2 & Package M6 Technical Specifications
# ==============================================================================

import os
import sys
import struct
import zlib
import json
from PIL import Image

# 32-Color 4-Tone Ramp Palette (SPEC-ART-2026-09-23-V2 & Project Ascendant Master)
# Index mapping:
# 0: Transparent
# 1-6: Steel & Slate (Outline, Shadow, Mid, Light, Specular)
# 7-10: Gold & Warm Light
# 11-14: Leather & Earth/Wood
# 15-18: Skin/Flesh
# 19-22: Red / Poppies / Ember
# 23-26: Blue / Bellflower / Magic
# 27-30: Void / Deep Shadow
# 31: Foliage Deep / Canopy Shadow
# Extended Foliage ramp mapped into indexed palette slots:
PALETTE_32 = [
    (0, 0, 0, 0),         # 0: transparent
    (18, 18, 20, 255),    # 1: #121214 (Outline / Deep Slate Shadow)
    (30, 36, 44, 255),    # 2: #1E242C (Contour / Dark Stone)
    (74, 88, 104, 255),   # 3: #4A5868 (Stone Shadow)
    (148, 164, 180, 255), # 4: #94A4B4 (Stone Midtone)
    (220, 228, 236, 255), # 5: #DCE4EC (Stone Highlight / Quartz)
    (255, 255, 255, 255), # 6: #FFFFFF (Specular Glint / White Flora)
    (58, 36, 8, 255),     # 7: #3A2408 (Wood Deep / Earth Crevice)
    (140, 90, 20, 255),   # 8: #8C5A14 (Wood Shadow / Gold Ochre)
    (224, 168, 48, 255),  # 9: #E0A830 (Wood Light / Yellow Daisy)
    (255, 244, 176, 255), # 10: #FFF4B0 (Sunlit Flower Core)
    (44, 24, 8, 255),     # 11: #2C1808 (Bark Deep / Rich Loam)
    (92, 58, 30, 255),    # 12: #5C3A1E (Bark Shadow / Moist Earth)
    (154, 106, 64, 255),  # 13: #9A6A40 (Timber Midtone / Path Dirt)
    (208, 168, 120, 255), # 14: #D0A878 (Dry Earth / Weathered Planks)
    (14, 40, 24, 255),    # 15: #0E2818 (Foliage Deep Undergrowth)
    (27, 82, 38, 255),    # 16: #1B5226 (Foliage Shadow)
    (61, 139, 55, 255),   # 17: #3D8B37 (Grass / Leaf Midtone)
    (126, 200, 80, 255),  # 18: #7EC850 (Sunlit Foliage Highlight)
    (56, 0, 8, 255),      # 19: #380008 (Poppy Deep)
    (128, 8, 24, 255),    # 20: #800818 (Poppy Shadow)
    (208, 32, 32, 255),   # 21: #D02020 (Poppy Red Midtone)
    (255, 112, 96, 255),  # 22: #FF7060 (Poppy Highlight)
    (8, 16, 48, 255),     # 23: #081030 (Water Deep)
    (16, 40, 120, 255),   # 24: #102878 (Bellflower Shadow)
    (40, 96, 208, 255),   # 25: #2860D0 (Bellflower Midtone)
    (112, 176, 255, 255), # 26: #70B0FF (Bellflower Highlight)
    (200, 240, 128, 255), # 27: #C8F080 (Foliage Specular Glint)
    (180, 60, 20, 255),   # 28: #B43C14 (Torch Flame Core)
    (255, 180, 40, 255),  # 29: #FFB428 (Torch Flame Outer)
    (60, 80, 30, 255),    # 30: #3C501E (Moss Olive Midtone)
    (0, 255, 255, 255)    # 31: #00FFFF (Guide Sockets / Anchors)
]

# Color constants
C_TRANS = (0, 0, 0, 0)
C_OUTLINE = (18, 18, 20, 255)
C_STONE_DARK = (30, 36, 44, 255)
C_STONE_SHADOW = (74, 88, 104, 255)
C_STONE_MID = (148, 164, 180, 255)
C_STONE_LIGHT = (220, 228, 236, 255)
C_GLINT = (255, 255, 255, 255)

C_BARK_DEEP = (44, 24, 8, 255)
C_BARK_SHADOW = (92, 58, 30, 255)
C_TIMBER_MID = (154, 106, 64, 255)
C_TIMBER_LIGHT = (208, 168, 120, 255)

C_LEAF_DEEP = (14, 40, 24, 255)
C_LEAF_SHADOW = (27, 82, 38, 255)
C_LEAF_MID = (61, 139, 55, 255)
C_LEAF_LIGHT = (126, 200, 80, 255)
C_LEAF_GLINT = (200, 240, 128, 255)
C_MOSS = (60, 80, 30, 255)

C_DIRT_DEEP = (58, 36, 8, 255)
C_DIRT_SHADOW = (92, 58, 30, 255)
C_DIRT_MID = (154, 106, 64, 255)
C_DIRT_LIGHT = (208, 168, 120, 255)

C_POPPY_DARK = (128, 8, 24, 255)
C_POPPY_MID = (208, 32, 32, 255)
C_POPPY_LIGHT = (255, 112, 96, 255)

C_BELL_DARK = (16, 40, 120, 255)
C_BELL_MID = (40, 96, 208, 255)
C_BELL_LIGHT = (112, 176, 255, 255)

C_DAISY_DARK = (140, 90, 20, 255)
C_DAISY_MID = (224, 168, 48, 255)
C_DAISY_LIGHT = (255, 244, 176, 255)

C_FLAME_RED = (180, 60, 20, 255)
C_FLAME_ORANGE = (255, 180, 40, 255)
C_FLAME_YELLOW = (255, 244, 176, 255)

def draw_rect(img, x0, y0, x1, y1, color):
    w, h = img.size
    for y in range(max(0, y0), min(h, y1 + 1)):
        for x in range(max(0, x0), min(w, x1 + 1)):
            img.putpixel((x, y), color)

def draw_point(img, x, y, color):
    w, h = img.size
    if 0 <= x < w and 0 <= y < h:
        img.putpixel((x, y), color)

def draw_circle(img, cx, cy, r, color):
    w, h = img.size
    for y in range(max(0, cy - r), min(h, cy + r + 1)):
        for x in range(max(0, cx - r), min(w, cx + r + 1)):
            if (x - cx) ** 2 + (y - cy) ** 2 <= r ** 2:
                img.putpixel((x, y), color)

# ==============================================================================
# FOLIAGE GENERATORS (Grass, Bushes, Flowers)
# ==============================================================================

def create_grass_tuft():
    # 16x16 small wild grass tuft
    img = Image.new("RGBA", (16, 16), C_TRANS)
    # Roots / Base contour
    draw_rect(img, 6, 13, 10, 14, C_OUTLINE)
    draw_rect(img, 7, 12, 9, 13, C_LEAF_DEEP)
    # Blade 1 (Left leaning)
    for i, (x, y) in enumerate([(6, 11), (5, 9), (4, 7), (3, 5), (2, 4)]):
        draw_point(img, x, y, C_LEAF_SHADOW)
        draw_point(img, x + 1, y, C_LEAF_MID)
    draw_point(img, 2, 4, C_LEAF_LIGHT)
    # Blade 2 (Center tall)
    for y in range(4, 12):
        draw_point(img, 8, y, C_LEAF_MID)
        draw_point(img, 7, y, C_LEAF_LIGHT if y < 8 else C_LEAF_MID)
    draw_point(img, 7, 3, C_LEAF_LIGHT)
    # Blade 3 (Right leaning)
    for i, (x, y) in enumerate([(9, 11), (10, 9), (11, 7), (12, 6)]):
        draw_point(img, x, y, C_LEAF_MID)
        draw_point(img, x - 1, y, C_LEAF_LIGHT)
    draw_point(img, 13, 5, C_LEAF_LIGHT)
    return img

def create_grass_patch():
    # 32x32 lush grass patch
    img = Image.new("RGBA", (32, 32), C_TRANS)
    # Base ground shade
    draw_circle(img, 16, 26, 9, C_LEAF_DEEP)
    draw_circle(img, 16, 25, 7, C_LEAF_SHADOW)
    # Clustered grass blades with 10 o'clock highlights
    blades = [
        # (root_x, root_y, points)
        [(11, 26), [(10, 24), (9, 21), (7, 18), (6, 15), (5, 13)]],
        [(14, 26), [(13, 23), (13, 19), (12, 16), (12, 12), (11, 9)]],
        [(16, 27), [(16, 23), (16, 18), (15, 14), (15, 10), (14, 7)]],
        [(19, 26), [(19, 23), (20, 19), (21, 15), (22, 12), (23, 10)]],
        [(22, 27), [(23, 24), (24, 21), (26, 18), (27, 16)]],
        [(9, 27),  [(8, 24), (7, 22), (6, 20)]],
        [(24, 27), [(25, 25), (26, 23), (27, 21)]]
    ]
    for root, pts in blades:
        draw_point(img, root[0], root[1], C_OUTLINE)
        for idx, (px, py) in enumerate(pts):
            draw_point(img, px, py, C_LEAF_LIGHT if idx > len(pts)//2 else C_LEAF_MID)
            draw_point(img, px + 1, py, C_LEAF_SHADOW)
    # Specular blade tips
    draw_point(img, 11, 9, C_LEAF_GLINT)
    draw_point(img, 14, 7, C_LEAF_GLINT)
    return img

def create_tall_grass():
    # 32x32 tall wind-blown grass
    img = Image.new("RGBA", (32, 32), C_TRANS)
    draw_rect(img, 10, 27, 22, 30, C_LEAF_DEEP)
    # Long swaying arches towards right
    curves = [
        [(12, 28), (11, 23), (11, 17), (13, 11), (16, 7), (20, 5), (24, 4)],
        [(15, 29), (14, 24), (15, 18), (17, 13), (21, 9), (26, 7), (30, 6)],
        [(18, 29), (17, 25), (18, 20), (20, 15), (24, 12), (28, 10)],
        [(8, 28), (7, 24), (7, 19), (9, 15), (12, 12), (15, 10)]
    ]
    for pts in curves:
        for i, (x, y) in enumerate(pts):
            draw_point(img, x, y, C_LEAF_LIGHT if i >= len(pts)-2 else C_LEAF_MID)
            draw_point(img, x, y + 1, C_LEAF_SHADOW)
    return img

def create_flower_poppies():
    # 32x32 grass patch with red poppies
    img = create_grass_patch()
    # Poppy heads
    poppies = [(12, 14), (18, 11), (22, 16)]
    for px, py in poppies:
        # Poppy blossom
        draw_circle(img, px, py, 3, C_OUTLINE)
        draw_circle(img, px, py, 2, C_POPPY_MID)
        draw_point(img, px - 1, py - 1, C_POPPY_LIGHT) # 10 o'clock key light
        draw_point(img, px + 1, py + 1, C_POPPY_DARK)
        draw_point(img, px, py, C_OUTLINE) # dark flower center
        draw_point(img, px, py - 1, C_DAISY_LIGHT) # stamen speck
    return img

def create_flower_bellflowers():
    # 32x32 grass patch with blue bellflowers
    img = create_grass_patch()
    bells = [(11, 13), (16, 9), (21, 14)]
    for bx, by in bells:
        draw_circle(img, bx, by, 3, C_OUTLINE)
        draw_circle(img, bx, by, 2, C_BELL_MID)
        draw_point(img, bx - 1, by - 1, C_BELL_LIGHT)
        draw_point(img, bx + 1, by + 1, C_BELL_DARK)
        draw_point(img, bx, by, C_GLINT)
    return img

def create_flower_daisies():
    # 32x32 grass patch with golden sun-daisies
    img = create_grass_patch()
    daisies = [(10, 15), (17, 12), (23, 17)]
    for dx, dy in daisies:
        draw_circle(img, dx, dy, 3, C_OUTLINE)
        draw_circle(img, dx, dy, 2, C_GLINT) # white petals
        draw_point(img, dx, dy, C_DAISY_MID) # yellow center
        draw_point(img, dx, dy - 1, C_DAISY_LIGHT)
    return img

def create_bush_small():
    # 32x32 small berry shrub
    img = Image.new("RGBA", (32, 32), C_TRANS)
    # Ground shadow
    draw_circle(img, 16, 27, 11, C_LEAF_DEEP)
    # Foliage clusters
    clusters = [(12, 20, 7), (20, 20, 7), (16, 15, 8)]
    for cx, cy, r in clusters:
        draw_circle(img, cx, cy, r, C_OUTLINE)
    for cx, cy, r in clusters:
        draw_circle(img, cx, cy, r - 1, C_LEAF_SHADOW)
    for cx, cy, r in clusters:
        draw_circle(img, cx - 1, cy - 1, r - 2, C_LEAF_MID)
        draw_circle(img, cx - 2, cy - 2, r - 4, C_LEAF_LIGHT)
    # Red berries
    berries = [(12, 16), (19, 14), (15, 21), (21, 21), (10, 21)]
    for bx, by in berries:
        draw_circle(img, bx, by, 1, C_POPPY_MID)
        draw_point(img, bx - 1, by - 1, C_POPPY_LIGHT)
    return img

def create_bush_medium():
    # 48x48 dense foliage bush
    img = Image.new("RGBA", (48, 48), C_TRANS)
    # Ground shadow
    draw_circle(img, 24, 40, 16, C_LEAF_DEEP)
    # Main leafy mounds
    clusters = [
        (16, 30, 11), (32, 30, 11),
        (24, 22, 13), (16, 20, 9), (32, 20, 9),
        (24, 15, 10)
    ]
    for cx, cy, r in clusters:
        draw_circle(img, cx, cy, r, C_OUTLINE)
    for cx, cy, r in clusters:
        draw_circle(img, cx, cy, r - 1, C_LEAF_SHADOW)
    for cx, cy, r in clusters:
        draw_circle(img, cx - 2, cy - 2, r - 3, C_LEAF_MID)
        draw_circle(img, cx - 4, cy - 4, r - 5, C_LEAF_LIGHT)
        draw_point(img, cx - 5, cy - 5, C_LEAF_GLINT)
    return img

def create_bush_bramble():
    # 48x48 thorny bramble hedge
    img = Image.new("RGBA", (48, 48), C_TRANS)
    draw_circle(img, 24, 41, 15, C_BARK_DEEP)
    # Intertwined dark branches
    branches = [
        [(12, 42), (16, 34), (20, 26), (18, 18), (14, 14)],
        [(36, 42), (32, 35), (28, 27), (30, 19), (34, 15)],
        [(24, 43), (24, 32), (22, 22), (25, 14)]
    ]
    for b in branches:
        for i in range(len(b) - 1):
            x0, y0 = b[i]
            x1, y1 = b[i+1]
            draw_circle(img, (x0+x1)//2, (y0+y1)//2, 2, C_BARK_DEEP)
            draw_point(img, x0, y0, C_BARK_SHADOW)
            draw_point(img, x1, y1, C_TIMBER_MID)
    # Tangled foliage clumps on top
    clumps = [(15, 22, 7), (32, 24, 8), (24, 18, 9), (21, 30, 6), (28, 31, 6)]
    for cx, cy, r in clumps:
        draw_circle(img, cx, cy, r, C_OUTLINE)
        draw_circle(img, cx, cy, r - 1, C_LEAF_SHADOW)
        draw_circle(img, cx - 1, cy - 1, r - 2, C_LEAF_MID)
        draw_circle(img, cx - 2, cy - 2, r - 4, C_LEAF_LIGHT)
    # Thorns
    for tx, ty in [(17, 29), (30, 29), (23, 25), (14, 18), (33, 19)]:
        draw_point(img, tx, ty, C_DAISY_LIGHT)
    return img

# ==============================================================================
# TREE GENERATORS (Sapling, Oak, Pine)
# ==============================================================================

def create_tree_sapling():
    # 32x48 small sapling
    img = Image.new("RGBA", (32, 48), C_TRANS)
    # Ground roots at Y=44
    draw_circle(img, 16, 44, 5, C_BARK_DEEP)
    draw_rect(img, 14, 44, 18, 45, C_BARK_DEEP)
    # Slender trunk
    draw_rect(img, 15, 24, 17, 43, C_OUTLINE)
    draw_rect(img, 15, 24, 16, 43, C_BARK_SHADOW)
    draw_rect(img, 15, 24, 15, 43, C_TIMBER_MID) # 10 o'clock highlight
    # Foliage crown (Y=8..28)
    crowns = [(16, 20, 9), (13, 14, 8), (19, 14, 8), (16, 9, 7)]
    for cx, cy, r in crowns:
        draw_circle(img, cx, cy, r, C_OUTLINE)
    for cx, cy, r in crowns:
        draw_circle(img, cx, cy, r - 1, C_LEAF_SHADOW)
    for cx, cy, r in crowns:
        draw_circle(img, cx - 2, cy - 2, r - 3, C_LEAF_MID)
        draw_circle(img, cx - 3, cy - 3, r - 5, C_LEAF_LIGHT)
        draw_point(img, cx - 4, cy - 4, C_LEAF_GLINT)
    return img

def create_tree_oak():
    # 64x96 mature broadleaf oak tree
    img = Image.new("RGBA", (64, 96), C_TRANS)
    # Root base (pivot at 32, 90)
    draw_circle(img, 32, 90, 14, C_BARK_DEEP)
    # Heavy gnarled trunk (Y=52..90)
    draw_rect(img, 27, 52, 37, 90, C_OUTLINE)
    draw_rect(img, 28, 53, 36, 89, C_BARK_SHADOW)
    draw_rect(img, 29, 53, 32, 89, C_TIMBER_MID)
    draw_rect(img, 29, 53, 30, 89, C_TIMBER_LIGHT) # 10 o'clock key light on trunk
    # Roots flair out
    draw_rect(img, 22, 86, 27, 91, C_BARK_SHADOW)
    draw_rect(img, 37, 86, 42, 91, C_BARK_DEEP)
    # Massive layered leafy canopy (Y=8..60)
    canopy_mounds = [
        (22, 48, 14), (42, 48, 14),
        (16, 36, 15), (48, 36, 15),
        (26, 32, 17), (38, 32, 17),
        (20, 20, 15), (44, 20, 15),
        (32, 18, 18), (32, 10, 12)
    ]
    # 1. Dark silhouettes
    for cx, cy, r in canopy_mounds:
        draw_circle(img, cx, cy, r, C_OUTLINE)
    # 2. Shadow undercoat
    for cx, cy, r in canopy_mounds:
        draw_circle(img, cx, cy, r - 1, C_LEAF_SHADOW)
    # 3. Leaf clusters midtones & 10 o'clock highlights
    for cx, cy, r in canopy_mounds:
        draw_circle(img, cx - 2, cy - 3, r - 3, C_LEAF_MID)
        draw_circle(img, cx - 4, cy - 5, r - 6, C_LEAF_LIGHT)
        draw_circle(img, cx - 5, cy - 6, max(1, r - 9), C_LEAF_GLINT)
    return img

def create_tree_pine():
    # 48x96 evergreen conifer pine tree
    img = Image.new("RGBA", (48, 96), C_TRANS)
    # Base roots (pivot at 24, 90)
    draw_circle(img, 24, 90, 8, C_BARK_DEEP)
    draw_rect(img, 22, 60, 26, 90, C_OUTLINE)
    draw_rect(img, 23, 60, 25, 89, C_BARK_SHADOW)
    draw_rect(img, 23, 60, 23, 89, C_TIMBER_MID)
    # Conical needle tiers (4 tiers from bottom to top)
    tiers = [
        # (center_y, width, height)
        (65, 40, 18),
        (50, 34, 17),
        (36, 26, 16),
        (22, 18, 15),
        (10, 10, 12)
    ]
    for cy, tw, th in tiers:
        # Draw needle triangle
        for y_offset in range(th):
            y = cy - y_offset
            cur_w = int((tw / 2) * (1.0 - (y_offset / th)))
            draw_rect(img, 24 - cur_w, y, 24 + cur_w, y, C_OUTLINE)
            draw_rect(img, 24 - cur_w + 1, y, 24 + cur_w - 1, y, C_LEAF_SHADOW)
            draw_rect(img, 24 - cur_w + 1, y, 24, y, C_LEAF_MID)
            draw_rect(img, 24 - cur_w + 1, y, 24 - cur_w // 2, y, C_LEAF_LIGHT)
    # Spire tip
    draw_point(img, 24, 5, C_LEAF_LIGHT)
    draw_point(img, 24, 4, C_LEAF_GLINT)
    return img

# ==============================================================================
# ROCK GENERATORS (Pebbles, Mossy Boulder, Crag)
# ==============================================================================

def create_rock_pebbles():
    # 16x16 scattered pebbles
    img = Image.new("RGBA", (16, 16), C_TRANS)
    pebbles = [(4, 11, 3, 2), (10, 8, 4, 3), (12, 13, 2, 2)]
    for px, py, rw, rh in pebbles:
        draw_rect(img, px - rw//2 - 1, py - rh//2 - 1, px + rw//2 + 1, py + rh//2 + 1, C_OUTLINE)
        draw_rect(img, px - rw//2, py - rh//2, px + rw//2, py + rh//2, C_STONE_SHADOW)
        draw_rect(img, px - rw//2, py - rh//2, px, py, C_STONE_MID)
        draw_point(img, px - rw//2, py - rh//2, C_STONE_LIGHT)
    return img

def create_rock_boulder():
    # 32x32 granite boulder with mossy top
    img = Image.new("RGBA", (32, 32), C_TRANS)
    # Ground shadow
    draw_circle(img, 16, 26, 11, C_STONE_DARK)
    # Main boulder body (faceted granite)
    draw_circle(img, 16, 20, 11, C_OUTLINE)
    draw_circle(img, 16, 20, 10, C_STONE_SHADOW)
    # Planar facet shading
    draw_circle(img, 14, 18, 8, C_STONE_MID)
    draw_circle(img, 12, 16, 5, C_STONE_LIGHT)
    draw_point(img, 10, 14, C_GLINT)
    # Moss patch on top
    for mx, my in [(14, 10), (15, 10), (16, 11), (17, 11), (13, 11), (12, 12), (18, 12)]:
        draw_point(img, mx, my, C_MOSS)
    draw_point(img, 14, 9, C_LEAF_LIGHT)
    return img

def create_rock_crag():
    # 48x48 jagged rock outcrop formation
    img = Image.new("RGBA", (48, 48), C_TRANS)
    # Shadow base
    draw_rect(img, 8, 40, 40, 44, C_STONE_DARK)
    # Left jagged pillar
    draw_rect(img, 10, 22, 24, 41, C_OUTLINE)
    draw_rect(img, 11, 23, 23, 40, C_STONE_SHADOW)
    draw_rect(img, 11, 23, 17, 38, C_STONE_MID)
    draw_rect(img, 11, 23, 14, 30, C_STONE_LIGHT)
    draw_point(img, 11, 22, C_GLINT)
    # Right massive spire (reaches up to Y=10)
    draw_rect(img, 20, 10, 38, 41, C_OUTLINE)
    draw_rect(img, 21, 11, 37, 40, C_STONE_SHADOW)
    draw_rect(img, 21, 11, 30, 39, C_STONE_MID)
    draw_rect(img, 21, 11, 25, 25, C_STONE_LIGHT)
    draw_point(img, 21, 10, C_GLINT)
    # Crevices and stratified fracture lines
    for y in [18, 26, 34]:
        draw_rect(img, 16, y, 32, y, C_OUTLINE)
        draw_rect(img, 16, y + 1, 30, y + 1, C_STONE_LIGHT)
    return img

# ==============================================================================
# PROP GENERATORS (Crate, Barrel, Signpost, Brazier)
# ==============================================================================

def create_prop_crate():
    # 32x32 sturdy wooden cargo crate
    img = Image.new("RGBA", (32, 32), C_TRANS)
    # Cast shadow
    draw_rect(img, 4, 27, 28, 30, C_STONE_DARK)
    # Crate wooden body (X=5..26, Y=8..27)
    draw_rect(img, 5, 8, 26, 27, C_OUTLINE)
    draw_rect(img, 6, 9, 25, 26, C_TIMBER_MID)
    # Wood planks (horizontal lines)
    for y in [14, 20]:
        draw_rect(img, 6, y, 25, y, C_BARK_DEEP)
    # Diagonal brace
    for i in range(16):
        draw_point(img, 7 + i, 10 + i, C_BARK_SHADOW)
        draw_point(img, 8 + i, 10 + i, C_TIMBER_LIGHT)
    # Metal corner brackets
    corners = [(6, 9), (23, 9), (6, 24), (23, 24)]
    for cx, cy in corners:
        draw_rect(img, cx, cy, cx + 2, cy + 2, C_STONE_SHADOW)
        draw_point(img, cx, cy, C_STONE_LIGHT)
    # 10 o'clock key light on top and left bevel
    draw_rect(img, 6, 9, 25, 9, C_TIMBER_LIGHT)
    draw_rect(img, 6, 9, 6, 26, C_TIMBER_LIGHT)
    return img

def create_prop_barrel():
    # 32x32 oak barrel with iron hoops
    img = Image.new("RGBA", (32, 32), C_TRANS)
    # Cast shadow
    draw_rect(img, 7, 27, 25, 30, C_STONE_DARK)
    # Barrel silhouette (tapered top/bottom, bulging center)
    for y in range(8, 28):
        # Calculate bulge width
        factor = 1.0 - abs((y - 18) / 10.0) ** 2 * 0.3
        bw = int(9 * factor)
        draw_rect(img, 16 - bw - 1, y, 16 + bw + 1, y, C_OUTLINE)
        draw_rect(img, 16 - bw, y, 16 + bw, y, C_TIMBER_MID)
        # 10 o'clock highlight
        draw_rect(img, 16 - bw, y, 16 - bw + 2, y, C_TIMBER_LIGHT)
        # Shadow side
        draw_rect(img, 16 + bw - 2, y, 16 + bw, y, C_BARK_SHADOW)
    # Iron hoops at Y=11, 16, 21, 25
    for y in [11, 16, 21, 25]:
        factor = 1.0 - abs((y - 18) / 10.0) ** 2 * 0.3
        bw = int(9 * factor)
        draw_rect(img, 16 - bw, y, 16 + bw, y, C_STONE_DARK)
        draw_rect(img, 16 - bw, y, 16 - bw + 3, y, C_STONE_LIGHT)
        draw_point(img, 16 - bw + 1, y, C_GLINT)
    # Top lid
    draw_circle(img, 16, 9, 5, C_BARK_SHADOW)
    return img

def create_prop_signpost():
    # 32x32 wooden signpost
    img = Image.new("RGBA", (32, 32), C_TRANS)
    # Ground mount
    draw_circle(img, 16, 28, 4, C_STONE_DARK)
    # Central wooden post (X=15..17, Y=6..28)
    draw_rect(img, 14, 6, 18, 28, C_OUTLINE)
    draw_rect(img, 15, 6, 17, 28, C_BARK_SHADOW)
    draw_rect(img, 15, 6, 15, 28, C_TIMBER_LIGHT)
    # Pointed wooden sign pointing right (Y=9..15, X=8..27)
    draw_rect(img, 8, 9, 24, 15, C_OUTLINE)
    draw_rect(img, 9, 10, 23, 14, C_TIMBER_MID)
    draw_rect(img, 9, 10, 23, 10, C_TIMBER_LIGHT) # top bevel light
    # Sign arrow point
    draw_point(img, 25, 11, C_OUTLINE)
    draw_point(img, 26, 12, C_OUTLINE)
    draw_point(img, 25, 13, C_OUTLINE)
    draw_point(img, 24, 12, C_TIMBER_LIGHT)
    # Iron nail fixing
    draw_point(img, 16, 12, C_STONE_LIGHT)
    return img

def create_prop_brazier_frames():
    # 32x32 iron brazier with animated flame (4 frames)
    frames = []
    flame_offsets = [
        [(16, 11), (15, 9), (17, 8), (16, 6)],
        [(16, 11), (17, 9), (15, 7), (17, 5)],
        [(16, 11), (16, 8), (17, 7), (16, 5)],
        [(16, 11), (15, 9), (16, 7), (15, 6)]
    ]
    for f_idx in range(4):
        img = Image.new("RGBA", (32, 32), C_TRANS)
        # Shadow
        draw_circle(img, 16, 27, 8, C_STONE_DARK)
        # Iron Tripod Stand
        draw_rect(img, 11, 23, 13, 28, C_OUTLINE)
        draw_rect(img, 19, 23, 21, 28, C_OUTLINE)
        draw_rect(img, 15, 18, 17, 25, C_STONE_SHADOW)
        # Iron Bowl Basin (Y=14..18, X=9..23)
        draw_rect(img, 9, 14, 23, 19, C_OUTLINE)
        draw_rect(img, 10, 15, 22, 18, C_STONE_DARK)
        draw_rect(img, 10, 15, 13, 18, C_STONE_LIGHT)
        # Hot coals in basin
        draw_rect(img, 11, 15, 21, 16, C_FLAME_RED)
        # Flickering Flame
        fo = flame_offsets[f_idx]
        draw_circle(img, fo[0][0], fo[0][1], 5, C_FLAME_RED)
        draw_circle(img, fo[1][0], fo[1][1], 3, C_FLAME_ORANGE)
        draw_circle(img, fo[2][0], fo[2][1], 2, C_FLAME_YELLOW)
        draw_point(img, fo[3][0], fo[3][1], C_GLINT)
        frames.append(img)
    return frames

# ==============================================================================
# TERRAIN TILESET GENERATORS (32x32 Isometric/2.5D Modular Tiles)
# ==============================================================================

def create_tile_dirt():
    img = Image.new("RGBA", (32, 32), C_DIRT_MID)
    # Soil texture speckles
    speckles = [
        (4, 5), (12, 8), (22, 4), (18, 14), (8, 20), (26, 18), (14, 26), (24, 28), (2, 28),
        (7, 12), (15, 3), (28, 10), (11, 17), (20, 22), (5, 24), (29, 25)
    ]
    for i, (sx, sy) in enumerate(speckles):
        img.putpixel((sx, sy), C_DIRT_DEEP if i % 2 == 0 else C_DIRT_LIGHT)
    return img

def create_tile_grass():
    img = Image.new("RGBA", (32, 32), C_LEAF_MID)
    # Subtle grass blade texture
    blades = [
        (5, 6), (11, 4), (20, 7), (27, 5), (14, 12), (8, 16), (22, 15), (29, 18),
        (4, 22), (12, 24), (19, 21), (25, 26), (7, 28), (16, 29), (28, 29)
    ]
    for i, (bx, by) in enumerate(blades):
        img.putpixel((bx, by), C_LEAF_LIGHT if i % 2 == 0 else C_LEAF_SHADOW)
    return img

def create_tile_stone_path():
    img = Image.new("RGBA", (32, 32), C_DIRT_SHADOW)
    # Cobblestones with mortar grooves
    stones = [
        (2, 2, 14, 10), (16, 2, 29, 10),
        (2, 12, 9, 20), (11, 12, 21, 20), (23, 12, 30, 20),
        (2, 22, 15, 30), (17, 22, 30, 30)
    ]
    for x0, y0, x1, y1 in stones:
        draw_rect(img, x0, y0, x1, y1, C_OUTLINE)
        draw_rect(img, x0 + 1, y0 + 1, x1 - 1, y1 - 1, C_STONE_MID)
        draw_rect(img, x0 + 1, y0 + 1, x1 - 2, y0 + 1, C_STONE_LIGHT) # 10 o'clock bevel
        draw_rect(img, x0 + 1, y0 + 1, x0 + 1, y1 - 2, C_STONE_LIGHT)
        draw_rect(img, x1 - 1, y0 + 1, x1 - 1, y1 - 1, C_STONE_SHADOW)
    return img

def create_tile_cliff_ledge():
    img = Image.new("RGBA", (32, 32), C_STONE_SHADOW)
    # Top rock ledge rim (Y=0..8)
    draw_rect(img, 0, 0, 31, 6, C_STONE_LIGHT)
    draw_rect(img, 0, 7, 31, 7, C_OUTLINE)
    # Vertical striated cliff face (Y=8..31)
    draw_rect(img, 0, 8, 31, 31, C_STONE_SHADOW)
    for x in [6, 14, 22, 28]:
        draw_rect(img, x, 8, x + 1, 31, C_STONE_DARK)
        draw_rect(img, x + 2, 8, x + 2, 31, C_STONE_LIGHT)
    return img

def create_tile_grass_edge_n():
    # Grass on top, transitioning into dirt at bottom
    img = create_tile_dirt()
    # Scalloped grass fringe
    draw_rect(img, 0, 0, 31, 14, C_LEAF_MID)
    for x in range(32):
        fringe = 14 + int(3 * (1.0 + (x % 4 == 0) - (x % 3 == 0)))
        draw_rect(img, x, 14, x, fringe, C_LEAF_MID)
        draw_point(img, x, fringe, C_LEAF_SHADOW)
        draw_point(img, x, fringe + 1, C_OUTLINE)
    return img

def create_tile_grass_edge_s():
    img = create_tile_dirt()
    draw_rect(img, 0, 16, 31, 31, C_LEAF_MID)
    for x in range(32):
        fringe = 16 - int(3 * (1.0 + (x % 5 == 0) - (x % 2 == 0)))
        draw_rect(img, x, fringe, x, 16, C_LEAF_MID)
        draw_point(img, x, fringe, C_LEAF_LIGHT)
    return img

def create_tile_grass_edge_w():
    img = create_tile_dirt()
    draw_rect(img, 0, 0, 14, 31, C_LEAF_MID)
    for y in range(32):
        fringe = 14 + int(3 * (1.0 + (y % 4 == 0)))
        draw_rect(img, 14, y, fringe, y, C_LEAF_MID)
        draw_point(img, fringe, y, C_LEAF_SHADOW)
    return img

def create_tile_grass_edge_e():
    img = create_tile_dirt()
    draw_rect(img, 16, 0, 31, 31, C_LEAF_MID)
    for y in range(32):
        fringe = 16 - int(3 * (1.0 + (y % 4 == 0)))
        draw_rect(img, fringe, y, 16, y, C_LEAF_MID)
        draw_point(img, fringe, y, C_LEAF_LIGHT)
    return img

def create_tile_grass_corner_nw():
    img = create_tile_dirt()
    draw_rect(img, 0, 0, 16, 16, C_LEAF_MID)
    return img

def create_tile_grass_corner_ne():
    img = create_tile_dirt()
    draw_rect(img, 16, 0, 31, 16, C_LEAF_MID)
    return img

def create_tile_grass_corner_sw():
    img = create_tile_dirt()
    draw_rect(img, 0, 16, 16, 31, C_LEAF_MID)
    return img

def create_tile_grass_corner_se():
    img = create_tile_dirt()
    draw_rect(img, 16, 16, 31, 31, C_LEAF_MID)
    return img

def create_tile_grass_inner_nw():
    img = create_tile_grass()
    draw_rect(img, 0, 0, 10, 10, C_DIRT_MID)
    return img

def create_tile_grass_inner_ne():
    img = create_tile_grass()
    draw_rect(img, 21, 0, 31, 10, C_DIRT_MID)
    return img

def create_tile_grass_inner_sw():
    img = create_tile_grass()
    draw_rect(img, 0, 21, 10, 31, C_DIRT_MID)
    return img

def create_tile_grass_inner_se():
    img = create_tile_grass()
    draw_rect(img, 21, 21, 31, 31, C_DIRT_MID)
    return img

# ==============================================================================
# ASEPRITE BINARY PACKAGING HELPER (0xA5E0)
# ==============================================================================

def write_aseprite_file(filepath, width, height, frames_images, tag_name="Default"):
    num_frames = len(frames_images)
    pal_data = bytearray()
    pal_data += struct.pack("<III 8s", len(PALETTE_32), 0, len(PALETTE_32) - 1, b"\x00" * 8)
    for c in PALETTE_32:
        pal_data += struct.pack("<HBBBB", 0, c[0], c[1], c[2], c[3])
    pal_chunk = struct.pack("<IH", len(pal_data) + 6, 0x2019) + pal_data

    layer_b = b"Layer_Main"
    l_body = struct.pack("<HHHHHHB 3s", 3, 0, 0, 0, 0, 0, 255, b"\x00" * 3) + struct.pack("<H", len(layer_b)) + layer_b
    layer_chunk = struct.pack("<IH", len(l_body) + 6, 0x2004) + l_body

    tag_data = struct.pack("<H 8s", 1, b"\x00" * 8)
    name_b = tag_name.encode("utf-8")
    tag_data += struct.pack("<HHB 8s 3s B", 0, num_frames - 1, 0, b"\x00" * 8, b"\x00" * 3, 0) + struct.pack("<H", len(name_b)) + name_b
    tag_chunk = struct.pack("<IH", len(tag_data) + 6, 0x2018) + tag_data

    frames_bytes = bytearray()
    for f_idx, f_img in enumerate(frames_images):
        dur = 100
        f_chunks = bytearray()
        if f_idx == 0:
            f_chunks += pal_chunk
            f_chunks += layer_chunk
            f_chunks += tag_chunk

        raw_px = f_img.tobytes()
        comp_px = zlib.compress(raw_px)
        cel_hdr = struct.pack("<HhhBHh 5s", 0, 0, 0, 255, 2, 0, b"\x00" * 5)
        cel_b = cel_hdr + struct.pack("<HH", width, height) + comp_px
        f_chunks += struct.pack("<IH", len(cel_b) + 6, 0x2005) + cel_b

        total_chunks = 1 + (3 if f_idx == 0 else 0)
        f_header = struct.pack("<IHHH 2s I", len(f_chunks) + 16, 0xF1FA, total_chunks, dur, b"\x00" * 2, total_chunks)
        frames_bytes += f_header + f_chunks

    total_size = 128 + len(frames_bytes)
    header = bytearray(128)
    struct.pack_into("<IHHHHHIHI I B 3s H BB hh HH 84s", header, 0,
        total_size, 0xA5E0, num_frames, width, height, 32, 1, 100, 0, 0, 0, b"\x00" * 3, len(PALETTE_32), 1, 1, 0, 0, 16, 16, b"\x00" * 84)

    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    with open(filepath, "wb") as f:
        f.write(header)
        f.write(frames_bytes)
    print(f"  [Aseprite] Xuất xưởng thành công: {filepath} ({total_size} bytes)")

# ==============================================================================
# MAIN EXECUTION & CATALOG
# ==============================================================================

def main():
    base_out = "Content/Art/Environment"
    src_dir = f"{base_out}/Source"
    spr_dir = f"{base_out}/Sprites"
    fol_dir = f"{spr_dir}/Foliage"
    tree_dir = f"{spr_dir}/Trees"
    prop_dir = f"{spr_dir}/Props"
    tile_dir = f"{spr_dir}/Tiles"
    meta_dir = f"{base_out}/metadata"

    for d in [src_dir, spr_dir, fol_dir, tree_dir, prop_dir, tile_dir, meta_dir]:
        os.makedirs(d, exist_ok=True)

    print("================================================================================")
    print("ProjectAscendant — Package M6: Environment & Vegetation Production Foundation")
    print("================================================================================")

    # 1. Author Foliage Assets
    print("\n[1/5] Khởi tạo tài nguyên Foliage (Cỏ, Hoa, Bụi cây)...")
    foliage_assets = {
        "FOL_Grass_Tuft_01": (create_grass_tuft(), (8, 14), "Foliage/Grass"),
        "FOL_Grass_Patch_02": (create_grass_patch(), (16, 28), "Foliage/Grass"),
        "FOL_Grass_Tall_03": (create_tall_grass(), (16, 29), "Foliage/Grass"),
        "FOL_Flower_Poppies_01": (create_flower_poppies(), (16, 28), "Foliage/Flowers"),
        "FOL_Flower_Bellflowers_02": (create_flower_bellflowers(), (16, 28), "Foliage/Flowers"),
        "FOL_Flower_Daisies_03": (create_flower_daisies(), (16, 28), "Foliage/Flowers"),
        "FOL_Bush_Small_01": (create_bush_small(), (16, 28), "Foliage/Bushes"),
        "FOL_Bush_Medium_02": (create_bush_medium(), (24, 42), "Foliage/Bushes"),
        "FOL_Bush_Bramble_03": (create_bush_bramble(), (24, 43), "Foliage/Bushes"),
    }
    for name, (img, pivot, cat) in foliage_assets.items():
        out_p = f"{fol_dir}/{name}.png"
        img.save(out_p)
        print(f"  + {name}.png ({img.size[0]}x{img.size[1]}) [Pivot: {pivot}]")

    # 2. Author Tree Assets
    print("\n[2/5] Khởi tạo tài nguyên Cây cối (Trees)...")
    tree_assets = {
        "TREE_Sapling_01": (create_tree_sapling(), (16, 45), "Trees/Woodland"),
        "TREE_Oak_Canopy_02": (create_tree_oak(), (32, 90), "Trees/Woodland"),
        "TREE_Pine_Evergreen_03": (create_tree_pine(), (24, 90), "Trees/Conifer"),
    }
    for name, (img, pivot, cat) in tree_assets.items():
        out_p = f"{tree_dir}/{name}.png"
        img.save(out_p)
        print(f"  + {name}.png ({img.size[0]}x{img.size[1]}) [Pivot: {pivot}]")

    # 3. Author Rocks & Props
    print("\n[3/5] Khởi tạo tài nguyên Đá & Đạo cụ (Rocks & Props)...")
    prop_assets = {
        "ROCK_Pebbles_01": (create_rock_pebbles(), (8, 14), "Rocks"),
        "ROCK_Boulder_Mossy_02": (create_rock_boulder(), (16, 27), "Rocks"),
        "ROCK_Formation_Crag_03": (create_rock_crag(), (24, 42), "Rocks"),
        "PROP_Crate_Oak_01": (create_prop_crate(), (16, 28), "Props/Containers"),
        "PROP_Barrel_Keg_02": (create_prop_barrel(), (16, 28), "Props/Containers"),
        "PROP_Signpost_Waymarker_03": (create_prop_signpost(), (16, 29), "Props/Waymarkers"),
    }
    for name, (img, pivot, cat) in prop_assets.items():
        out_p = f"{prop_dir}/{name}.png"
        img.save(out_p)
        print(f"  + {name}.png ({img.size[0]}x{img.size[1]}) [Pivot: {pivot}]")

    # Animated Brazier
    brazier_frames = create_prop_brazier_frames()
    brazier_sheet = Image.new("RGBA", (128, 32), C_TRANS)
    for idx, f_img in enumerate(brazier_frames):
        brazier_sheet.paste(f_img, (idx * 32, 0))
    brazier_sheet.save(f"{prop_dir}/PROP_Brazier_Torch_04_sheet.png")
    brazier_frames[0].save(
        f"{prop_dir}/PROP_Brazier_Torch_04.gif",
        save_all=True,
        append_images=brazier_frames[1:],
        duration=100,
        loop=0,
        disposal=2
    )
    print(f"  + PROP_Brazier_Torch_04 (32x32, 4 frames anim) [Pivot: (16, 28)]")

    # 4. Author Terrain Tileset (16 tiles 32x32)
    print("\n[4/5] Khởi tạo bộ gạch địa hình (Terrain Tileset 32x32)...")
    tiles = {
        "TILE_Dirt_Base": create_tile_dirt(),
        "TILE_Grass_Base": create_tile_grass(),
        "TILE_Stone_Cobble_Path": create_tile_stone_path(),
        "TILE_Cliff_Ledge_Wall": create_tile_cliff_ledge(),
        "TILE_Grass_Edge_N": create_tile_grass_edge_n(),
        "TILE_Grass_Edge_S": create_tile_grass_edge_s(),
        "TILE_Grass_Edge_W": create_tile_grass_edge_w(),
        "TILE_Grass_Edge_E": create_tile_grass_edge_e(),
        "TILE_Grass_Corner_NW": create_tile_grass_corner_nw(),
        "TILE_Grass_Corner_NE": create_tile_grass_corner_ne(),
        "TILE_Grass_Corner_SW": create_tile_grass_corner_sw(),
        "TILE_Grass_Corner_SE": create_tile_grass_corner_se(),
        "TILE_Grass_Inner_NW": create_tile_grass_inner_nw(),
        "TILE_Grass_Inner_NE": create_tile_grass_inner_ne(),
        "TILE_Grass_Inner_SW": create_tile_grass_inner_sw(),
        "TILE_Grass_Inner_SE": create_tile_grass_inner_se(),
    }
    for name, img in tiles.items():
        img.save(f"{tile_dir}/{name}.png")
        print(f"  + {name}.png (32x32)")

    # Composite Tileset Atlas (4x4 tiles = 128x128 px)
    atlas = Image.new("RGBA", (128, 128), C_TRANS)
    tile_list = list(tiles.values())
    for idx, t_img in enumerate(tile_list):
        gx = (idx % 4) * 32
        gy = (idx // 4) * 32
        atlas.paste(t_img, (gx, gy))
    atlas.save(f"{spr_dir}/environment_terrain_tileset.png")
    print(f"  [Atlas] Tạo thành công tileset atlas: {spr_dir}/environment_terrain_tileset.png (128x128)")

    # Composite Category Sheets
    # Foliage Sheet (9 assets horizontal = 288x48)
    fol_sheet = Image.new("RGBA", (9 * 48, 48), C_TRANS)
    for i, (name, (f_img, piv, cat)) in enumerate(foliage_assets.items()):
        # Center horizontally and anchor bottom at Y=44
        ox = i * 48 + (48 - f_img.size[0]) // 2
        oy = 44 - f_img.size[1] + (f_img.size[1] - piv[1])
        fol_sheet.paste(f_img, (ox, max(0, oy)))
    fol_sheet.save(f"{spr_dir}/environment_foliage_sheet.png")

    # Trees Sheet (3 trees horizontal = 192x96)
    tree_sheet = Image.new("RGBA", (192, 96), C_TRANS)
    for i, (name, (t_img, piv, cat)) in enumerate(tree_assets.items()):
        ox = i * 64 + (64 - t_img.size[0]) // 2
        oy = 96 - t_img.size[1]
        tree_sheet.paste(t_img, (ox, oy))
    tree_sheet.save(f"{spr_dir}/environment_trees_sheet.png")

    # Props Sheet (6 props + 1 brazier = 7 * 48 = 336x48)
    props_sheet = Image.new("RGBA", (7 * 48, 48), C_TRANS)
    all_props = list(prop_assets.items()) + [("PROP_Brazier_Torch_04", (brazier_frames[0], (16, 28), "Props/Light"))]
    for i, (name, (p_img, piv, cat)) in enumerate(all_props):
        ox = i * 48 + (48 - p_img.size[0]) // 2
        oy = 44 - p_img.size[1] + (p_img.size[1] - piv[1])
        props_sheet.paste(p_img, (ox, max(0, oy)))
    props_sheet.save(f"{spr_dir}/environment_props_sheet.png")

    # 5. Author Master Aseprite Binaries
    print("\n[5/5] Xuất xưởng các tệp nguồn master .aseprite...")
    write_aseprite_file(f"{src_dir}/environment_foliage.aseprite", 48, 48, [v[0] if v[0].size == (48, 48) else v[0].resize((48, 48), Image.NEAREST) for v in foliage_assets.values()], "Foliage")
    write_aseprite_file(f"{src_dir}/environment_trees.aseprite", 64, 96, [v[0] if v[0].size == (64, 96) else v[0].resize((64, 96), Image.NEAREST) for v in tree_assets.values()], "Trees")
    write_aseprite_file(f"{src_dir}/environment_props.aseprite", 32, 32, brazier_frames, "Brazier_Flame")
    write_aseprite_file(f"{src_dir}/environment_terrain_tileset.aseprite", 128, 128, [atlas], "TerrainAtlas")

    # 6. Generate Complete Metadata JSON
    metadata = {
        "package": "Package M6 — Environment & Vegetation Production Foundation",
        "conforms_to": "SPEC-ART-2026-09-23-V2",
        "lighting": "10_oclock_directional",
        "color_ramp": "4_tone_steep_contrast",
        "alpha_mode": "strictly_binary",
        "sorting_anchor": "bottom_center_pivot",
        "foliage": {name: {"dimensions": list(img.size), "pivot": list(piv), "category": cat} for name, (img, piv, cat) in foliage_assets.items()},
        "trees": {name: {"dimensions": list(img.size), "pivot": list(piv), "category": cat} for name, (img, piv, cat) in tree_assets.items()},
        "rocks_and_props": {name: {"dimensions": list(img.size), "pivot": list(piv), "category": cat} for name, (img, piv, cat) in prop_assets.items()},
        "animated_props": {
            "PROP_Brazier_Torch_04": {"dimensions": [32, 32], "pivot": [16, 28], "frames": 4, "duration_ms": 100}
        },
        "tileset": {
            "grid_size": [32, 32],
            "atlas_dimensions": [128, 128],
            "tile_count": len(tiles),
            "tiles": list(tiles.keys())
        }
    }
    meta_path = f"{meta_dir}/environment_metadata.json"
    with open(meta_path, "w", encoding="utf-8") as fp:
        json.dump(metadata, fp, indent=2)
    print(f"\n[Metadata] Ghi dữ liệu thành công: {meta_path}")
    print("\n✅ Hoàn thành toàn bộ quy trình sản xuất Environment Production Foundation!")

if __name__ == "__main__":
    main()
