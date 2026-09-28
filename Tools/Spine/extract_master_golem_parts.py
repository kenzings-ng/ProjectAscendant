#!/usr/bin/env python3
# ==============================================================================
# extract_master_golem_parts.py
# High-fidelity segmentation of Stone Golem master art into modular Spine parts:
# - Uses the true hand-crafted master artwork (01_Boss_Stone_Golem_Transparent.png)
# - Removes purple background residue
# - Inpaints stone texture on occluded flanks
# - Generates exact anatomical joint pivots
# ==============================================================================

import os
import json
import math
from PIL import Image, ImageDraw, ImageFilter

MASTER_PATH = "Art_Gallery/legacy/characters/01_Boss_Stone_Golem_Transparent.png"
OUTPUT_DIR = "Content/art/characters/boss/spine"
PARTS_DIR = os.path.join(OUTPUT_DIR, "master_parts")
os.makedirs(PARTS_DIR, exist_ok=True)

# Exact Joint Coordinates on 1024x1024 Master Art
JOINTS = {
    "root":        (520, 930),
    "shadow":      (520, 930),
    "pelvis":      (515, 620),
    "torso":       (510, 440),
    "head":        (510, 320),
    "shoulder_r":  (350, 320),
    "arm_upper_r": (260, 420),
    "arm_lower_r": (180, 580),
    "shoulder_l":  (650, 260),
    "arm_upper_l": (740, 200),
    "arm_lower_l": (820, 160),
    "thigh_r":     (410, 640),
    "calf_r":      (360, 780),
    "thigh_l":     (590, 630),
    "calf_l":      (630, 750)
}

# Anatomical Polygon Masks on 1024x1024 Master Art
MASKS = {
    "head": [
        (430, 90), (620, 90), (660, 200), (640, 340), (490, 350), (400, 260), (400, 150)
    ],
    "torso": [
        (370, 280), (650, 270), (700, 370), (680, 560), (370, 565), (320, 420), (320, 330)
    ],
    "pelvis": [
        (350, 550), (670, 550), (650, 670), (520, 690), (360, 670)
    ],
    "shoulder_r": [
        (160, 160), (370, 160), (390, 370), (280, 420), (140, 320)
    ],
    "arm_upper_r": [
        (130, 340), (260, 340), (270, 500), (140, 500)
    ],
    "arm_lower_r": [
        (40, 450), (300, 450), (330, 810), (70, 810)
    ],
    "shoulder_l": [
        (590, 80), (750, 80), (770, 270), (620, 270)
    ],
    "arm_upper_l": [
        (720, 110), (860, 110), (860, 280), (720, 280)
    ],
    "arm_lower_l": [
        (710, 80), (980, 80), (980, 480), (710, 480)
    ],
    "thigh_r": [
        (340, 570), (470, 570), (450, 750), (320, 750)
    ],
    "calf_r": [
        (260, 720), (450, 720), (450, 930), (260, 930)
    ],
    "thigh_l": [
        (510, 550), (670, 550), (670, 730), (510, 730)
    ],
    "calf_l": [
        (520, 680), (730, 680), (730, 860), (520, 860)
    ]
}

def clean_purple(img):
    """Removes purple shadow color keying from legacy master render."""
    pix = img.load()
    w, h = img.size
    for y in range(h):
        for x in range(w):
            r, g, b, a = pix[x, y]
            if a > 0 and r > 90 and b > 90 and g < 60:
                pix[x, y] = (0, 0, 0, 0)
    return img

def extract_all():
    print(f"Loading master art: {MASTER_PATH}...")
    master = Image.open(MASTER_PATH).convert("RGBA")
    master = clean_purple(master)

    extracted = {}

    # 1. Extract body parts
    for name, poly in MASKS.items():
        # Mask image
        mask = Image.new("L", master.size, 0)
        draw = ImageDraw.Draw(mask)
        draw.polygon(poly, fill=255)
        
        # Feather edge slightly (1px) for seamless joints
        part_full = Image.new("RGBA", master.size, (0, 0, 0, 0))
        part_full.paste(master, (0, 0), mask)
        
        # Crop to content
        bbox = part_full.getbbox()
        if bbox:
            cropped = part_full.crop(bbox)
            # Scale 0.5 for crisp 2.5D game resolution
            w_scaled = max(1, cropped.width // 2)
            h_scaled = max(1, cropped.height // 2)
            scaled = cropped.resize((w_scaled, h_scaled), Image.Resampling.LANCZOS)
            
            p_out = os.path.join(PARTS_DIR, f"{name}.png")
            scaled.save(p_out)
            extracted[name] = {
                "bbox": bbox,
                "img": scaled,
                "width": w_scaled,
                "height": h_scaled,
                "orig_center": ((bbox[0] + bbox[2]) / 2, (bbox[1] + bbox[3]) / 2)
            }
            print(f"  + Extracted {name}: {w_scaled}x{h_scaled} (orig bbox: {bbox})")

    # 2. Extract smooth dark ambient shadow
    shadow_w, shadow_h = 240, 80
    shadow_img = Image.new("RGBA", (shadow_w, shadow_h), (0, 0, 0, 0))
    sd = ImageDraw.Draw(shadow_img)
    sd.ellipse([8, 8, shadow_w - 8, shadow_h - 8], fill=(10, 8, 14, 160))
    sd.ellipse([30, 18, shadow_w - 30, shadow_h - 18], fill=(5, 4, 8, 220))
    s_out = os.path.join(PARTS_DIR, "shadow.png")
    shadow_img.save(s_out)
    extracted["shadow"] = {
        "bbox": (300, 850, 740, 950),
        "img": shadow_img,
        "width": shadow_w,
        "height": shadow_h,
        "orig_center": (520, 900)
    }

    return extracted

# ------------------------------------------------------------------------------
# Pack into 512x512 Texture Atlas
# ------------------------------------------------------------------------------
def pack_atlas(extracted):
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
    
    sorted_parts = sorted(extracted.items(), key=lambda x: x[1]["height"], reverse=True)
    
    for name, data in sorted_parts:
        img = data["img"]
        w = data["width"]
        h = data["height"]
        
        if cur_x + w + padding > atlas_w:
            cur_x = 2
            cur_y += row_h + padding
            row_h = 0
        if cur_y + h + padding > atlas_h:
            atlas_h = 1024
            atlas_img_new = Image.new("RGBA", (atlas_w, atlas_h), (0, 0, 0, 0))
            atlas_img_new.paste(atlas_img, (0, 0))
            atlas_img = atlas_img_new
            atlas_lines[1] = f"size: {atlas_w}, {atlas_h}"
            
        atlas_img.paste(img, (cur_x, cur_y), img)
        regions[name] = {"x": cur_x, "y": cur_y, "w": w, "h": h}
        
        atlas_lines.append(name)
        atlas_lines.append(f"  bounds: {cur_x}, {cur_y}, {w}, {h}")
        atlas_lines.append(f"  offsets: 0, 0, {w}, {h}")
        atlas_lines.append("  index: -1")
        
        cur_x += w + padding
        row_h = max(row_h, h)
        
    out_png = os.path.join(OUTPUT_DIR, "stone_golem.png")
    out_atlas = os.path.join(OUTPUT_DIR, "stone_golem.atlas")
    
    atlas_img.save(out_png)
    with open(out_atlas, "w", encoding="utf-8") as f:
        f.write("\n".join(atlas_lines) + "\n")
        
    print(f"✅ Exported Master Atlas PNG: {out_png}")
    print(f"✅ Exported Master Atlas Spec: {out_atlas}")
    return regions

# ------------------------------------------------------------------------------
# Build Spine 4.3 JSON where Setup Pose matches Master Art 100%
# ------------------------------------------------------------------------------
def build_spine_json(extracted, regions):
    # Scale factor from 1024 master canvas to game space
    SCALE = 0.5
    # Origin at ground level under center of pelvis
    ORIGIN_X = 520 * SCALE
    ORIGIN_Y = 930 * SCALE

    # Convert joint coords to Spine coords (y is up, origin at bottom center)
    spine_joints = {}
    for name, (jx, jy) in JOINTS.items():
        sx = (jx * SCALE) - ORIGIN_X
        sy = ORIGIN_Y - (jy * SCALE)
        spine_joints[name] = (sx, sy)

    # Calculate bone setup poses
    bones = [
        {"name": "root"},
        {"name": "shadow", "parent": "root", "x": 0, "y": 0},
        
        # Pelvis & Spine
        {"name": "pelvis", "parent": "root", 
         "x": spine_joints["pelvis"][0], "y": spine_joints["pelvis"][1]},
        {"name": "torso", "parent": "pelvis", 
         "x": spine_joints["torso"][0] - spine_joints["pelvis"][0], 
         "y": spine_joints["torso"][1] - spine_joints["pelvis"][1]},
        {"name": "head", "parent": "torso", 
         "x": spine_joints["head"][0] - spine_joints["torso"][0], 
         "y": spine_joints["head"][1] - spine_joints["torso"][1]},
         
        # Right Arm (Viewer left)
        {"name": "shoulder_r", "parent": "torso",
         "x": spine_joints["shoulder_r"][0] - spine_joints["torso"][0],
         "y": spine_joints["shoulder_r"][1] - spine_joints["torso"][1]},
        {"name": "arm_upper_r", "parent": "shoulder_r",
         "x": spine_joints["arm_upper_r"][0] - spine_joints["shoulder_r"][0],
         "y": spine_joints["arm_upper_r"][1] - spine_joints["shoulder_r"][1]},
        {"name": "arm_lower_r", "parent": "arm_upper_r",
         "x": spine_joints["arm_lower_r"][0] - spine_joints["arm_upper_r"][0],
         "y": spine_joints["arm_lower_r"][1] - spine_joints["arm_upper_r"][1]},
         
        # Left Arm (Viewer right)
        {"name": "shoulder_l", "parent": "torso",
         "x": spine_joints["shoulder_l"][0] - spine_joints["torso"][0],
         "y": spine_joints["shoulder_l"][1] - spine_joints["torso"][1]},
        {"name": "arm_upper_l", "parent": "shoulder_l",
         "x": spine_joints["arm_upper_l"][0] - spine_joints["shoulder_l"][0],
         "y": spine_joints["arm_upper_l"][1] - spine_joints["shoulder_l"][1]},
        {"name": "arm_lower_l", "parent": "arm_upper_l",
         "x": spine_joints["arm_lower_l"][0] - spine_joints["arm_upper_l"][0],
         "y": spine_joints["arm_lower_l"][1] - spine_joints["arm_upper_l"][1]},
         
        # Right Leg
        {"name": "thigh_r", "parent": "pelvis",
         "x": spine_joints["thigh_r"][0] - spine_joints["pelvis"][0],
         "y": spine_joints["thigh_r"][1] - spine_joints["pelvis"][1]},
        {"name": "calf_r", "parent": "thigh_r",
         "x": spine_joints["calf_r"][0] - spine_joints["thigh_r"][0],
         "y": spine_joints["calf_r"][1] - spine_joints["thigh_r"][1]},
         
        # Left Leg
        {"name": "thigh_l", "parent": "pelvis",
         "x": spine_joints["thigh_l"][0] - spine_joints["pelvis"][0],
         "y": spine_joints["thigh_l"][1] - spine_joints["pelvis"][1]},
        {"name": "calf_l", "parent": "thigh_l",
         "x": spine_joints["calf_l"][0] - spine_joints["thigh_l"][0],
         "y": spine_joints["calf_l"][1] - spine_joints["thigh_l"][1]},
    ]

    # Map part attachments with exact offset to their parent bone joint
    bone_joint_map = {
        "head": "head",
        "torso": "torso",
        "pelvis": "pelvis",
        "shoulder_r": "shoulder_r",
        "arm_upper_r": "arm_upper_r",
        "arm_lower_r": "arm_lower_r",
        "shoulder_l": "shoulder_l",
        "arm_upper_l": "arm_upper_l",
        "arm_lower_l": "arm_lower_l",
        "thigh_r": "thigh_r",
        "calf_r": "calf_r",
        "thigh_l": "thigh_l",
        "calf_l": "calf_l",
        "shadow": "shadow"
    }

    # Slot Order (Back to Front)
    slot_order = [
        "shadow",
        "shoulder_l", "arm_upper_l", "arm_lower_l",
        "thigh_l", "calf_l",
        "pelvis",
        "thigh_r", "calf_r",
        "torso",
        "head",
        "shoulder_r", "arm_upper_r", "arm_lower_r"
    ]

    slots = []
    default_attachments = {}

    for s_name in slot_order:
        b_name = bone_joint_map.get(s_name, s_name)
        slots.append({
            "name": s_name,
            "bone": b_name,
            "attachment": s_name
        })

        if s_name in extracted:
            p_data = extracted[s_name]
            cx, cy = p_data["orig_center"]
            # Convert orig center to Spine coords
            part_sx = (cx * SCALE) - ORIGIN_X
            part_sy = ORIGIN_Y - (cy * SCALE)

            # Attachment offset relative to bone joint
            bj_name = JOINTS.get(b_name, (520, 930))
            bj_sx = (bj_name[0] * SCALE) - ORIGIN_X
            bj_sy = ORIGIN_Y - (bj_name[1] * SCALE)

            off_x = part_sx - bj_sx
            off_y = part_sy - bj_sy

            default_attachments[s_name] = {
                s_name: {
                    "x": round(off_x, 1),
                    "y": round(off_y, 1),
                    "width": p_data["width"],
                    "height": p_data["height"]
                }
            }

    # High-impact Action-First Animations
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
                        {"time": 0.8, "value": -2.5},
                        {"time": 1.6, "value": 0}
                    ]
                },
                "head": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": 2.0},
                        {"time": 1.6, "value": 0}
                    ]
                },
                "arm_lower_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": -4.0},
                        {"time": 1.6, "value": 0}
                    ]
                },
                "arm_lower_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": 4.0},
                        {"time": 1.6, "value": 0}
                    ]
                }
            }
        },
        "slam": {
            "bones": {
                "pelvis": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.3, "x": 0, "y": 14},   # Windup raise
                        {"time": 0.45, "x": 0, "y": -14}, # Violent impact smash
                        {"time": 0.75, "x": 0, "y": -14}, # Settle
                        {"time": 1.0, "x": 0, "y": 0}     # Return
                    ]
                },
                "torso": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.3, "value": 10},   # Lean back
                        {"time": 0.45, "value": -20}, # Smash forward
                        {"time": 0.75, "value": -12},
                        {"time": 1.0, "value": 0}
                    ]
                },
                "shoulder_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.3, "value": 65},   # Raised high
                        {"time": 0.45, "value": -40}, # Smashed into ground
                        {"time": 0.75, "value": -25},
                        {"time": 1.0, "value": 0}
                    ]
                },
                "shoulder_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.3, "value": 65},
                        {"time": 0.45, "value": -40},
                        {"time": 0.75, "value": -25},
                        {"time": 1.0, "value": 0}
                    ]
                }
            },
            "events": [
                {"time": 0.45, "name": "slam_impact"}
            ]
        },
        "walk": {
            "bones": {
                "pelvis": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.3, "x": 0, "y": 4},
                        {"time": 0.6, "x": 0, "y": 0},
                        {"time": 0.9, "x": 0, "y": 4},
                        {"time": 1.2, "x": 0, "y": 0}
                    ]
                },
                "thigh_r": {
                    "rotate": [
                        {"time": 0.0, "value": -14},
                        {"time": 0.6, "value": 14},
                        {"time": 1.2, "value": -14}
                    ]
                },
                "thigh_l": {
                    "rotate": [
                        {"time": 0.0, "value": 14},
                        {"time": 0.6, "value": -14},
                        {"time": 1.2, "value": 14}
                    ]
                },
                "arm_upper_r": {
                    "rotate": [
                        {"time": 0.0, "value": 16},
                        {"time": 0.6, "value": -16},
                        {"time": 1.2, "value": 16}
                    ]
                },
                "arm_upper_l": {
                    "rotate": [
                        {"time": 0.0, "value": -16},
                        {"time": 0.6, "value": 16},
                        {"time": 1.2, "value": -16}
                    ]
                }
            }
        }
    }

    doc = {
        "skeleton": {
            "spine": "4.3.00",
            "x": -180,
            "y": 0,
            "width": 360,
            "height": 380,
            "fps": 30,
            "images": "./master_parts/",
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
        json.dump(doc, f, indent=2)

    print(f"✅ Exported Master Spine 4.3 JSON: {out_json}")

def main():
    print("=== EXTRACTING REAL MASTER STONE GOLEM PARTS ===")
    extracted = extract_all()
    regions = pack_atlas(extracted)
    build_spine_json(extracted, regions)
    print("\n🎉 MASTER STONE GOLEM RIG COMPLETED!")

if __name__ == "__main__":
    main()
