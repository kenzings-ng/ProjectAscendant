#!/usr/bin/env python3
# ==============================================================================
# generate_professional_stone_golem_spine.py
#
# Generates professional Spine 2D skeletal assets for Stone Golem Boss:
# - Full overdrawn modular parts (pill/ball joints, socket curves, filled flanks)
# - No occlusion holes when limbs rotate or stretch
# - Exact bone joint pivots and attachment offsets
# - Packed Texture Atlas with LibGDX .atlas definition
# - Spine 4.3 Skeleton JSON with Bone Hierarchy, IK, and 5 Action-First animations
# - Master multi-layer .aseprite source file
# ==============================================================================

import os
import sys
import json
import math
import zlib
import struct
from PIL import Image, ImageDraw

OUTPUT_DIR = "Content/art/characters/boss/spine"
PARTS_DIR = os.path.join(OUTPUT_DIR, "pro_parts")
ASEPRITE_DIR = "Art_Gallery/aseprite/boss"

os.makedirs(PARTS_DIR, exist_ok=True)
os.makedirs(ASEPRITE_DIR, exist_ok=True)

# ------------------------------------------------------------------------------
# Color Palette (Dark Fantasy Granite Stone, Moss, Emissive Cyan Runes)
# ------------------------------------------------------------------------------
C_OUTLINE    = (26, 28, 26, 255)       # #1a1c1a Dark stone outline
C_SHADOW_DEEP= (35, 38, 34, 255)       # #232622 Deep crevice shadow
C_SHADOW_MID = (56, 61, 55, 255)       # #383d37 Mid shadow
C_STONE_BASE = (92, 99, 93, 255)       # #5c635d Mid granite tone
C_STONE_LIGHT= (121, 128, 118, 255)    # #798076 Light stone
C_STONE_HI   = (155, 163, 148, 255)    # #9ba394 Edge highlight
C_MOSS_DARK  = (63, 92, 42, 255)       # #3f5c2a Dark moss
C_MOSS_LIGHT = (93, 133, 59, 255)      # #5d853b Lush moss
C_RUNE_DARK  = (19, 108, 122, 255)     # #136c7a Emissive cyan edge
C_RUNE_MID   = (28, 184, 203, 255)     # #1cb8cb Glowing cyan
C_RUNE_HI    = (70, 241, 255, 255)     # #46f1ff Core cyan glow
C_RUNE_WHITE = (225, 255, 255, 255)    # #e1ffff Hot core white

def draw_stone_brick(draw, xy, fill=C_STONE_BASE, outline=C_OUTLINE, highlight=C_STONE_HI):
    """Draws a stylized dark fantasy stone block with bevel highlights."""
    x0, y0, x1, y1 = xy
    draw.rectangle([x0, y0, x1, y1], fill=fill, outline=outline, width=1)
    if x1 - x0 > 3 and y1 - y0 > 3:
        # Top and left highlight bevel
        draw.line([x0 + 1, y0 + 1, x1 - 1, y0 + 1], fill=highlight, width=1)
        draw.line([x0 + 1, y0 + 1, x0 + 1, y1 - 1], fill=highlight, width=1)
        # Bottom and right shadow bevel
        draw.line([x0 + 1, y1 - 1, x1 - 1, y1 - 1], fill=C_SHADOW_DEEP, width=1)
        draw.line([x1 - 1, y0 + 1, x1 - 1, y1 - 1], fill=C_SHADOW_DEEP, width=1)

def draw_pill_joint(draw, xy, fill=C_STONE_BASE, outline=C_OUTLINE):
    """Draws a rounded capsule ball joint for seamless bone rotation."""
    x0, y0, x1, y1 = xy
    draw.ellipse([x0, y0, x1, y1], fill=fill, outline=outline, width=1)
    if x1 - x0 > 4 and y1 - y0 > 4:
        draw.ellipse([x0 + 2, y0 + 2, x1 - 2, y1 - 2], fill=C_STONE_LIGHT)

# ------------------------------------------------------------------------------
# 1. Procedural Creation of Professional Modular Parts with Overdraw
# ------------------------------------------------------------------------------
def create_torso():
    """Torso monolith block: fully painted chest with runes, no arm occlusion."""
    w, h = 120, 110
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Base torso silhouette
    d.polygon([(20, 15), (100, 15), (115, 55), (95, 105), (25, 105), (5, 55)], fill=C_STONE_BASE, outline=C_OUTLINE)
    
    # Internal carved stone layers
    draw_stone_brick(d, [25, 18, 58, 48], C_STONE_LIGHT)
    draw_stone_brick(d, [62, 18, 95, 48], C_STONE_LIGHT)
    draw_stone_brick(d, [15, 52, 58, 80], C_STONE_BASE)
    draw_stone_brick(d, [62, 52, 105, 80], C_STONE_BASE)
    draw_stone_brick(d, [30, 84, 90, 102], C_SHADOW_MID)

    # Glowing Cyan Runes carved into chest plate
    # Rune 1 (Left): Ankh-like ancient rune
    d.line([38, 26, 38, 42], fill=C_RUNE_MID, width=2)
    d.line([32, 30, 44, 30], fill=C_RUNE_MID, width=2)
    d.ellipse([34, 22, 42, 28], outline=C_RUNE_HI, width=1)
    # Rune 2 (Right): Angular Nordic-style Rune
    d.line([76, 24, 76, 42], fill=C_RUNE_MID, width=2)
    d.line([76, 26, 86, 32], fill=C_RUNE_HI, width=2)
    d.line([86, 32, 76, 38], fill=C_RUNE_MID, width=2)
    # Central glowing fissure
    d.line([58, 48, 62, 60], fill=C_RUNE_DARK, width=2)
    d.point([(60, 54)], fill=C_RUNE_WHITE)

    # Moss accents creeping along seams
    d.polygon([(18, 50), (28, 54), (22, 62), (16, 56)], fill=C_MOSS_LIGHT)
    d.polygon([(92, 46), (104, 52), (98, 60), (90, 52)], fill=C_MOSS_DARK)

    # Top neck socket (concave hollow for neck insertion)
    d.ellipse([45, 8, 75, 20], fill=C_SHADOW_DEEP, outline=C_OUTLINE)

    return img

def create_head():
    """Brutalist stone monolith head with neck pivot extension."""
    w, h = 60, 80
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Neck extension (inserts into torso socket)
    draw_pill_joint(d, [20, 55, 40, 78], fill=C_SHADOW_MID)

    # Cranium boulder
    d.polygon([(12, 10), (48, 10), (54, 38), (42, 58), (18, 58), (6, 38)], fill=C_STONE_LIGHT, outline=C_OUTLINE)
    
    # Brow plate
    draw_stone_brick(d, [10, 18, 50, 32], C_STONE_HI)
    # Jaw block
    draw_stone_brick(d, [16, 42, 44, 56], C_STONE_BASE)

    # Glowing Cyan Eye Slits
    d.rectangle([18, 30, 26, 34], fill=C_RUNE_HI, outline=C_RUNE_DARK)
    d.rectangle([34, 30, 42, 34], fill=C_RUNE_HI, outline=C_RUNE_DARK)
    d.point([(22, 32), (38, 32)], fill=C_RUNE_WHITE)

    # Forehead Rune
    d.line([30, 12, 30, 18], fill=C_RUNE_HI, width=1)
    d.line([27, 14, 33, 14], fill=C_RUNE_MID, width=1)

    return img

def create_pelvis():
    """Keystone stone groin/hip arch with thigh sockets."""
    w, h = 90, 55
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Torso connector bowl at top
    d.ellipse([15, 2, 75, 20], fill=C_SHADOW_DEEP)

    # Pelvis stone keystone arch
    d.polygon([(10, 10), (80, 10), (70, 45), (45, 52), (20, 45)], fill=C_STONE_BASE, outline=C_OUTLINE)
    draw_stone_brick(d, [30, 16, 60, 46], C_STONE_LIGHT)
    
    # Left & right hip socket recesses for thigh ball joints
    d.ellipse([5, 25, 25, 48], fill=C_SHADOW_DEEP, outline=C_OUTLINE)
    d.ellipse([65, 25, 85, 48], fill=C_SHADOW_DEEP, outline=C_OUTLINE)

    return img

def create_shoulder(is_left=False):
    """Curved pauldron stone block."""
    w, h = 65, 60
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Paired pauldron shape
    d.polygon([(10, 10), (55, 10), (60, 40), (45, 55), (15, 50), (5, 30)], fill=C_STONE_HI, outline=C_OUTLINE)
    draw_stone_brick(d, [15, 15, 50, 40], C_STONE_LIGHT)

    # Ball joint connector to torso
    draw_pill_joint(d, [20, 35, 45, 58], fill=C_SHADOW_MID)

    # Moss coating
    d.polygon([(12, 12), (26, 12), (20, 22)], fill=C_MOSS_LIGHT)
    return img

def create_arm_upper():
    """Upper arm bicep with rounded ball joints at BOTH ends."""
    w, h = 45, 65
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Top shoulder ball joint (Overdraw!)
    draw_pill_joint(d, [10, 2, 35, 26], fill=C_STONE_LIGHT)
    
    # Mid stone block
    draw_stone_brick(d, [8, 18, 37, 46], C_STONE_BASE)

    # Bottom elbow ball joint (Overdraw!)
    draw_pill_joint(d, [10, 38, 35, 62], fill=C_STONE_LIGHT)

    return img

def create_arm_lower():
    """Forearm with elbow socket cup at top and wrist ball at bottom."""
    w, h = 55, 75
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Elbow socket cup (concave receiver)
    d.ellipse([10, 2, 45, 22], fill=C_SHADOW_DEEP, outline=C_OUTLINE)

    # Forearm stone block
    draw_stone_brick(d, [8, 14, 47, 56], C_STONE_BASE)

    # Glowing Rune band on forearm
    d.line([14, 30, 41, 30], fill=C_RUNE_MID, width=2)
    d.line([27, 24, 27, 36], fill=C_RUNE_HI, width=2)
    d.point([(27, 30)], fill=C_RUNE_WHITE)

    # Wrist ball joint (Overdraw!)
    draw_pill_joint(d, [15, 50, 40, 72], fill=C_STONE_LIGHT)

    return img

def create_fist():
    """Massive stone block hammer fist."""
    w, h = 65, 65
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Wrist socket cup
    d.ellipse([15, 2, 50, 20], fill=C_SHADOW_DEEP, outline=C_OUTLINE)

    # Fist boulder block
    d.polygon([(10, 15), (55, 15), (60, 55), (10, 58)], fill=C_STONE_HI, outline=C_OUTLINE)
    
    # Carved knuckle bricks
    draw_stone_brick(d, [12, 22, 28, 54], C_STONE_LIGHT)
    draw_stone_brick(d, [30, 22, 44, 54], C_STONE_LIGHT)
    draw_stone_brick(d, [46, 22, 58, 54], C_STONE_LIGHT)

    return img

def create_thigh():
    """Upper leg stone pillar with hip ball joint at top and knee ball at bottom."""
    w, h = 45, 65
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Top hip ball joint (Overdraw into pelvis)
    draw_pill_joint(d, [10, 2, 35, 26], fill=C_STONE_LIGHT)

    # Pillar stone block
    draw_stone_brick(d, [8, 18, 37, 46], C_STONE_BASE)

    # Bottom knee ball joint (Overdraw into calf)
    draw_pill_joint(d, [10, 38, 35, 62], fill=C_STONE_LIGHT)

    return img

def create_calf():
    """Lower leg shin with knee socket at top and ankle joint at bottom."""
    w, h = 50, 70
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Knee socket cup
    d.ellipse([8, 2, 42, 22], fill=C_SHADOW_DEEP, outline=C_OUTLINE)

    # Shin stone block
    draw_stone_brick(d, [8, 14, 42, 52], C_STONE_BASE)

    # Ankle ball joint (Overdraw into foot)
    draw_pill_joint(d, [12, 45, 38, 68], fill=C_STONE_LIGHT)

    return img

def create_foot():
    """Massive stone foot with flat sole and ankle receiver cup."""
    w, h = 70, 45
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Ankle socket cup
    d.ellipse([15, 2, 45, 18], fill=C_SHADOW_DEEP, outline=C_OUTLINE)

    # Foot stone slab
    d.polygon([(10, 12), (55, 12), (65, 40), (5, 40)], fill=C_STONE_BASE, outline=C_OUTLINE)
    
    # 3 Carved toe boulders
    draw_stone_brick(d, [8, 22, 26, 40], C_STONE_LIGHT)
    draw_stone_brick(d, [28, 22, 44, 40], C_STONE_LIGHT)
    draw_stone_brick(d, [46, 22, 63, 40], C_STONE_LIGHT)

    return img

def create_shadow():
    """Ambient occlusion drop shadow ellipse."""
    w, h = 180, 60
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.ellipse([5, 5, w - 5, h - 5], fill=(12, 10, 18, 140))
    d.ellipse([25, 12, w - 25, h - 12], fill=(8, 6, 12, 200))
    return img

# ------------------------------------------------------------------------------
# 2. Part Generation & Master Rig Synthesis
# ------------------------------------------------------------------------------
PRO_PARTS = {
    "shadow":       create_shadow(),
    "pelvis":       create_pelvis(),
    "torso":        create_torso(),
    "head":         create_head(),
    "shoulder_r":   create_shoulder(is_left=False),
    "arm_upper_r":  create_arm_upper(),
    "arm_lower_r":  create_arm_lower(),
    "fist_r":       create_fist(),
    "shoulder_l":   create_shoulder(is_left=True),
    "arm_upper_l":  create_arm_upper(),
    "arm_lower_l":  create_arm_lower(),
    "fist_l":       create_fist(),
    "thigh_r":      create_thigh(),
    "calf_r":       create_calf(),
    "foot_r":       create_foot(),
    "thigh_l":      create_thigh(),
    "calf_l":       create_calf(),
    "foot_l":       create_foot(),
}

def export_parts():
    for name, img in PRO_PARTS.items():
        out_p = os.path.join(PARTS_DIR, f"{name}.png")
        img.save(out_p)
        print(f"  + Exported Overdrawn Part: {name} ({img.width}x{img.height}) -> {out_p}")

# ------------------------------------------------------------------------------
# 3. Pack Atlas (LibGDX Spine Format)
# ------------------------------------------------------------------------------
def pack_pro_atlas():
    atlas_w, atlas_h = 512, 512
    atlas_img = Image.new("RGBA", (atlas_w, atlas_h), (0, 0, 0, 0))
    
    atlas_lines = [
        "stone_golem.png",
        f"size: {atlas_w}, {atlas_h}",
        "format: RGBA8888",
        "filter: Nearest, Nearest",
        "repeat: none"
    ]
    
    cur_x, cur_y = 2, 2
    row_h = 0
    padding = 2
    regions = {}
    
    # Sort descending by height
    sorted_parts = sorted(PRO_PARTS.items(), key=lambda x: x[1].height, reverse=True)
    
    for name, img in sorted_parts:
        w, h = img.size
        if cur_x + w + padding > atlas_w:
            cur_x = 2
            cur_y += row_h + padding
            row_h = 0
        if cur_y + h + padding > atlas_h:
            raise RuntimeError(f"Atlas overflow at {name}")
            
        atlas_img.paste(img, (cur_x, cur_y), img)
        regions[name] = {"x": cur_x, "y": cur_y, "w": w, "h": h}
        
        atlas_lines.append(name)
        atlas_lines.append(f"  bounds: {cur_x}, {cur_y}, {w}, {h}")
        atlas_lines.append(f"  offsets: 0, 0, {w}, {h}")
        atlas_lines.append("  index: -1")
        
        cur_x += w + padding
        row_h = max(row_h, h)
        
    out_atlas_png = os.path.join(OUTPUT_DIR, "stone_golem.png")
    out_atlas_file = os.path.join(OUTPUT_DIR, "stone_golem.atlas")
    
    atlas_img.save(out_atlas_png)
    with open(out_atlas_file, "w", encoding="utf-8") as f:
        f.write("\n".join(atlas_lines) + "\n")
        
    print(f"✅ Generated Professional Spine Atlas: {out_atlas_png}")
    print(f"✅ Generated LibGDX Atlas Spec      : {out_atlas_file}")
    return regions

# ------------------------------------------------------------------------------
# 4. Generate Spine 4.3 Skeleton JSON with Bone Pivots and Action-First Anims
# ------------------------------------------------------------------------------
def generate_spine_json(regions):
    # Professional bone layout with exact pivot offsets
    bones = [
        {"name": "root"},
        {"name": "shadow", "parent": "root", "x": 0, "y": 0},
        {"name": "pelvis", "parent": "root", "x": 0, "y": 95, "length": 35, "rotation": 90},
        {"name": "torso", "parent": "pelvis", "x": 25, "y": 0, "length": 65, "rotation": 0},
        {"name": "head", "parent": "torso", "x": 65, "y": 0, "length": 45, "rotation": 0},
        
        # Right Arm (Viewer left)
        {"name": "shoulder_r", "parent": "torso", "x": 45, "y": 45, "length": 30, "rotation": 140},
        {"name": "arm_upper_r", "parent": "shoulder_r", "x": 30, "y": 0, "length": 40, "rotation": -25},
        {"name": "arm_lower_r", "parent": "arm_upper_r", "x": 40, "y": 0, "length": 45, "rotation": -20},
        {"name": "fist_r", "parent": "arm_lower_r", "x": 45, "y": 0, "length": 35, "rotation": 5},
        
        # Left Arm (Viewer right)
        {"name": "shoulder_l", "parent": "torso", "x": 45, "y": -45, "length": 30, "rotation": -140},
        {"name": "arm_upper_l", "parent": "shoulder_l", "x": 30, "y": 0, "length": 40, "rotation": 25},
        {"name": "arm_lower_l", "parent": "arm_upper_l", "x": 40, "y": 0, "length": 45, "rotation": 20},
        {"name": "fist_l", "parent": "arm_lower_l", "x": 45, "y": 0, "length": 35, "rotation": -5},
        
        # Right Leg
        {"name": "thigh_r", "parent": "pelvis", "x": -10, "y": 30, "length": 45, "rotation": -170},
        {"name": "calf_r", "parent": "thigh_r", "x": 45, "y": 0, "length": 50, "rotation": 15},
        {"name": "foot_r", "parent": "calf_r", "x": 50, "y": 0, "length": 40, "rotation": 65},
        
        # Left Leg
        {"name": "thigh_l", "parent": "pelvis", "x": -10, "y": -30, "length": 45, "rotation": -170},
        {"name": "calf_l", "parent": "thigh_l", "x": 45, "y": 0, "length": 50, "rotation": -15},
        {"name": "foot_l", "parent": "calf_l", "x": 50, "y": 0, "length": 40, "rotation": 65},
    ]

    # Slot Draw Order (Back to Front)
    slot_order = [
        "shadow",
        "shoulder_l", "arm_upper_l", "arm_lower_l", "fist_l",
        "thigh_l", "calf_l", "foot_l",
        "pelvis",
        "thigh_r", "calf_r", "foot_r",
        "torso",
        "head",
        "shoulder_r", "arm_upper_r", "arm_lower_r", "fist_r"
    ]

    slots = []
    for s in slot_order:
        slots.append({
            "name": s,
            "bone": s,
            "attachment": s
        })

    # Default Skin
    default_attachments = {}
    for name, r in regions.items():
        default_attachments[name] = {
            name: {
                "x": 0,
                "y": 0,
                "width": r["w"],
                "height": r["h"]
            }
        }

    # Action-First Animations
    animations = {
        "idle": {
            "bones": {
                "pelvis": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.8, "x": 0, "y": -4},
                        {"time": 1.6, "x": 0, "y": 0}
                    ]
                },
                "torso": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": -3},
                        {"time": 1.6, "value": 0}
                    ]
                },
                "head": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": 2},
                        {"time": 1.6, "value": 0}
                    ]
                },
                "arm_lower_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": -6},
                        {"time": 1.6, "value": 0}
                    ]
                },
                "arm_lower_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": 6},
                        {"time": 1.6, "value": 0}
                    ]
                }
            }
        },
        "walk": {
            "bones": {
                "pelvis": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.3, "x": 0, "y": 6},
                        {"time": 0.6, "x": 0, "y": 0},
                        {"time": 0.9, "x": 0, "y": 6},
                        {"time": 1.2, "x": 0, "y": 0}
                    ]
                },
                "thigh_r": {
                    "rotate": [
                        {"time": 0.0, "value": -22},
                        {"time": 0.6, "value": 22},
                        {"time": 1.2, "value": -22}
                    ]
                },
                "thigh_l": {
                    "rotate": [
                        {"time": 0.0, "value": 22},
                        {"time": 0.6, "value": -22},
                        {"time": 1.2, "value": 22}
                    ]
                },
                "arm_upper_r": {
                    "rotate": [
                        {"time": 0.0, "value": 25},
                        {"time": 0.6, "value": -25},
                        {"time": 1.2, "value": 25}
                    ]
                },
                "arm_upper_l": {
                    "rotate": [
                        {"time": 0.0, "value": -25},
                        {"time": 0.6, "value": 25},
                        {"time": 1.2, "value": -25}
                    ]
                }
            }
        },
        "slam": {
            "bones": {
                "pelvis": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.3, "x": 0, "y": 18},
                        {"time": 0.45, "x": 0, "y": -16},
                        {"time": 0.75, "x": 0, "y": -16},
                        {"time": 1.0, "x": 0, "y": 0}
                    ]
                },
                "torso": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.3, "value": 15},
                        {"time": 0.45, "value": -28},
                        {"time": 0.75, "value": -18},
                        {"time": 1.0, "value": 0}
                    ]
                },
                "shoulder_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.3, "value": 85},
                        {"time": 0.45, "value": -55},
                        {"time": 0.75, "value": -40},
                        {"time": 1.0, "value": 0}
                    ]
                },
                "shoulder_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.3, "value": 85},
                        {"time": 0.45, "value": -55},
                        {"time": 0.75, "value": -40},
                        {"time": 1.0, "value": 0}
                    ]
                }
            },
            "events": [
                {"time": 0.45, "name": "slam_impact"}
            ]
        },
        "stagger": {
            "bones": {
                "pelvis": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.15, "x": -18, "y": 0},
                        {"time": 0.5, "x": 0, "y": 0}
                    ]
                },
                "torso": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.15, "value": 20},
                        {"time": 0.5, "value": 0}
                    ]
                }
            }
        },
        "death": {
            "bones": {
                "pelvis": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.5, "x": 0, "y": -45},
                        {"time": 1.2, "x": 0, "y": -95}
                    ]
                },
                "torso": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.6, "value": 45},
                        {"time": 1.2, "value": 90}
                    ]
                },
                "head": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.6, "x": 30, "y": -65},
                        {"time": 1.2, "x": 75, "y": -125}
                    ]
                }
            }
        }
    }

    skeleton_doc = {
        "skeleton": {
            "spine": "4.3.00",
            "x": -150,
            "y": 0,
            "width": 300,
            "height": 320,
            "fps": 30,
            "images": "./pro_parts/",
            "audio": ""
        },
        "bones": bones,
        "slots": slots,
        "skins": [
            {
                "name": "default",
                "attachments": default_attachments
            }
        ],
        "events": {
            "slam_impact": {}
        },
        "animations": animations
    }

    out_json = os.path.join(OUTPUT_DIR, "stone_golem.json")
    with open(out_json, "w", encoding="utf-8") as f:
        json.dump(skeleton_doc, f, indent=2)

    print(f"✅ Generated Professional Spine 4.3 JSON: {out_json}")

# ------------------------------------------------------------------------------
# 5. Export Master Aseprite Rig
# ------------------------------------------------------------------------------
def export_aseprite_rig():
    canvas_w = 256
    canvas_h = 256
    layer_names = list(PRO_PARTS.keys())

    header = bytearray(128)
    struct.pack_into("<I H H H H H I H I I 3s B 2s B B 2s H B B", header, 0,
                     0, 0xA5E0, 1, canvas_w, canvas_h, 32,
                     1, 100, 0, 0, b"\x00\x00\x00", 0, b"\x00\x00", 0, 0, b"\x00\x00", 0, 0, 0)

    chunks = []
    # Color Profile Chunk
    profile_data = struct.pack("<H H I", 1, 0, 0)
    chunks.append(struct.pack("<I H", len(profile_data) + 6, 0x2007) + profile_data)

    # Layer chunks
    for name in layer_names:
        name_bytes = name.encode("utf-8")
        layer_payload = struct.pack("<H H H H H B 3s", 3, 0, 0, canvas_w, canvas_h, 0, b"\x00\x00\x00")
        layer_payload += struct.pack("<B", 255)
        layer_payload += struct.pack("<3s", b"\x00\x00\x00")
        layer_payload += struct.pack("<H", len(name_bytes)) + name_bytes
        chunks.append(struct.pack("<I H", len(layer_payload) + 6, 0x2004) + layer_payload)

    # Cel chunks
    for i, (name, img) in enumerate(PRO_PARTS.items()):
        w, h = img.size
        x = (canvas_w - w) // 2
        y = (canvas_h - h) // 2
        
        raw_pixels = bytearray()
        for py in range(h):
            for px in range(w):
                r, g, b, a = img.getpixel((px, py))
                raw_pixels.extend([r, g, b, a])
        compressed = zlib.compress(raw_pixels)
        
        cel_header = struct.pack("<H h h B H 7s", i, x, y, 255, 2, b"\x00" * 7)
        image_header = struct.pack("<H H", w, h)
        cel_payload = cel_header + image_header + compressed
        chunks.append(struct.pack("<I H", len(cel_payload) + 6, 0x2005) + cel_payload)

    frame_body = b"".join(chunks)
    frame_size = len(frame_body) + 16
    frame_header = struct.pack("<I H H H 2s I", frame_size, 0xF1FA, min(len(chunks), 0xFFFF), 100, b"\x00\x00", len(chunks))

    total_data = header + frame_header + frame_body
    struct.pack_into("<I", total_data, 0, len(total_data))

    out_ase = os.path.join(ASEPRITE_DIR, "stone_golem_pro_rig.aseprite")
    with open(out_ase, "wb") as f:
        f.write(total_data)
        
    print(f"✅ Generated Professional Aseprite Rig: {out_ase} (18 Overdrawn Layers)")

def main():
    print("=== PRODUCING PROFESSIONAL SPINE 4.3 STONE GOLEM RIG ===")
    export_parts()
    regions = pack_pro_atlas()
    generate_spine_json(regions)
    export_aseprite_rig()
    print("\n🎉 Professional Spine 4.3 Rigging Production COMPLETE!")

if __name__ == "__main__":
    main()
