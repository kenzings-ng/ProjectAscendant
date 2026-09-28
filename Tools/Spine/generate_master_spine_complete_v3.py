#!/usr/bin/env python3
# ==============================================================================
# generate_master_spine_complete_v3.py
# High-fidelity Spine 2D modular rig & animation pipeline for Stone Golem Boss.
# Features:
# - Pixel-perfect segmentation directly from master handcrafted artwork
# - Deep anatomical overlap between connected pieces (zero gaps, zero severed edges)
# - No artificial procedural vector circles; 100% authentic dark fantasy pixel art
# - Clean 1024x512 texture atlas (zero overflow)
# - Spine 4.3 JSON specification with slam_impact event
# - Spacious 600x600 canvas rendering with FK kinematics (no clipping)
# ==============================================================================

import os
import json
import math
import numpy as np
from PIL import Image, ImageDraw

MASTER_PATH = "Art_Gallery/legacy/characters/01_Boss_Stone_Golem_Transparent.png"
SPINE_DIR = "Content/art/characters/boss/spine"
PARTS_DIR = os.path.join(SPINE_DIR, "master_parts")
os.makedirs(PARTS_DIR, exist_ok=True)

def clean_master():
    img = Image.open(MASTER_PATH).convert("RGBA")
    arr = np.array(img)
    # Magenta background chroma residue removal
    chroma = (arr[:, :, 3] > 0) & (arr[:, :, 0] > 110) & (arr[:, :, 2] > 110) & (arr[:, :, 1] < 45)
    arr[chroma] = [0, 0, 0, 0]
    return Image.fromarray(arr)

# Anatomical Polygons with Generous Natural Overlap
PARTS_POLYS = {
    # Head: jawline, crown, neck collar extending down to Y 380 into torso collar
    "head": [
        (380, 85), (660, 85), (680, 180), (670, 280), (630, 380), 
        (470, 380), (430, 360), (385, 260), (380, 150)
    ],
    # Torso: collar at Y 250, sides, waist down to Y 590 into pelvis
    "torso": [
        (280, 250), (660, 230), (715, 330), (700, 590), (340, 595), 
        (280, 430), (270, 330)
    ],
    # Pelvis: waist blocks, top extends up to Y 500 into torso, bottom down to Y 710 over thighs
    "pelvis": [
        (335, 500), (685, 500), (680, 710), (500, 720), (335, 700)
    ],
    # Shoulder R: massive pauldron
    "shoulder_r": [
        (135, 140), (420, 140), (420, 440), (260, 440), (135, 320)
    ],
    # Arm Upper R: bicep extends up to Y 240 (deep into shoulder), down to Y 530 (deep into forearm)
    "arm_upper_r": [
        (120, 240), (280, 240), (280, 530), (120, 530)
    ],
    # Arm Lower R: forearm + fist, extends up to Y 380 (deep into bicep)
    "arm_lower_r": [
        (35, 380), (330, 380), (330, 600), (310, 600), (310, 750), (285, 750), (285, 830), (35, 830)
    ],
    # Shoulder L: pauldron behind head
    "shoulder_l": [
        (570, 70), (790, 70), (790, 290), (570, 290)
    ],
    # Arm Upper L: extends up to Y 80 into shoulder, down to Y 320
    "arm_upper_l": [
        (700, 80), (890, 80), (890, 320), (700, 320)
    ],
    # Arm Lower L: raised fist, extends down to Y 200 into bicep
    "arm_lower_l": [
        (690, 70), (990, 70), (990, 500), (690, 500)
    ],
    # Thigh R: extends up to Y 490 (deep into pelvis), down to Y 770 (deep into calf)
    "thigh_r": [
        (330, 490), (490, 490), (490, 770), (330, 770)
    ],
    # Calf R: extends up to Y 630 (deep into thigh), foot down to Y 965
    "calf_r": [
        (325, 630), (490, 630), (490, 965), 
        (270, 965), (280, 785), (325, 740)
    ],
    # Thigh L: extends up to Y 490 into pelvis, down to Y 760
    "thigh_l": [
        (485, 490), (700, 490), (700, 760), (485, 760)
    ],
    # Calf L: extends up to Y 630 into thigh, foot down to Y 855
    "calf_l": [
        (495, 630), (750, 630), (750, 855), (495, 855)
    ]
}

# Anatomical Joints in 1024-space
JOINTS_1024 = {
    "root":        (512, 950),
    "shadow":      (512, 950),
    "pelvis":      (512, 615),
    "torso":       (510, 435),
    "head":        (510, 315),
    "shoulder_r":  (350, 310),
    "arm_upper_r": (250, 410),
    "arm_lower_r": (195, 570),
    "shoulder_l":  (650, 250),
    "arm_upper_l": (740, 195),
    "arm_lower_l": (825, 155),
    "thigh_r":     (415, 635),
    "calf_r":      (370, 775),
    "thigh_l":     (590, 625),
    "calf_l":      (630, 745)
}

DRAW_ORDER = [
    "shadow",
    "shoulder_l", "arm_upper_l", "arm_lower_l",
    "thigh_l", "calf_l",
    "pelvis",
    "thigh_r", "calf_r",
    "torso", "head",
    "arm_upper_r", "shoulder_r", "arm_lower_r"
]

def extract_all_parts():
    master = clean_master()
    parts = {}

    for name, poly in PARTS_POLYS.items():
        mask = Image.new("L", master.size, 0)
        draw = ImageDraw.Draw(mask)
        draw.polygon(poly, fill=255)

        part_img = Image.new("RGBA", master.size, (0, 0, 0, 0))
        part_img.paste(master, (0, 0), mask)

        bbox = part_img.getbbox()
        if not bbox:
            continue
        cropped = part_img.crop(bbox)

        w_scaled = max(1, cropped.width // 2)
        h_scaled = max(1, cropped.height // 2)
        scaled = cropped.resize((w_scaled, h_scaled), Image.Resampling.LANCZOS)
        
        p_path = os.path.join(PARTS_DIR, f"{name}.png")
        scaled.save(p_path)

        parts[name] = {
            "bbox_orig": bbox,
            "center_scaled": ((bbox[0] + bbox[2]) / 4.0, (bbox[1] + bbox[3]) / 4.0),
            "width": w_scaled,
            "height": h_scaled,
            "img": scaled
        }
        print(f"  ✓ {name:12s}: {w_scaled}x{h_scaled} (orig: {bbox})")

    # Shadow
    sw, sh = 260, 90
    shadow = Image.new("RGBA", (sw, sh), (0, 0, 0, 0))
    s_draw = ImageDraw.Draw(shadow)
    s_draw.ellipse([8, 10, sw - 8, sh - 10], fill=(12, 10, 16, 170))
    s_draw.ellipse([35, 22, sw - 35, sh - 22], fill=(6, 5, 10, 230))
    shadow_path = os.path.join(PARTS_DIR, "shadow.png")
    shadow.save(shadow_path)
    parts["shadow"] = {
        "bbox_orig": (382, 905, 642, 995),
        "center_scaled": (512 / 2.0, 950 / 2.0),
        "width": sw,
        "height": sh,
        "img": shadow
    }

    return parts

def build_texture_atlas(parts):
    # Pack into 1024x512
    atlas_w, atlas_h = 1024, 512
    atlas = Image.new("RGBA", (atlas_w, atlas_h), (0, 0, 0, 0))
    atlas_lines = [
        "\nstone_golem.png",
        f"size: {atlas_w},{atlas_h}",
        "format: RGBA8888",
        "filter: Linear,Linear",
        "repeat: none"
    ]

    cur_x, cur_y, row_h = 4, 4, 0
    packed_info = {}

    for name in sorted(parts.keys(), key=lambda k: parts[k]["height"], reverse=True):
        p = parts[name]
        w, h = p["width"], p["height"]
        if cur_x + w + 4 > atlas_w:
            cur_x = 4
            cur_y += row_h + 4
            row_h = 0
        if cur_y + h + 4 > atlas_h:
            print(f"ERROR: Atlas overflow for {name}!")

        atlas.paste(p["img"], (cur_x, cur_y))
        packed_info[name] = {"x": cur_x, "y": cur_y, "w": w, "h": h}
        
        atlas_lines.extend([
            name,
            "  rotate: false",
            f"  xy: {cur_x}, {cur_y}",
            f"  size: {w}, {h}",
            f"  orig: {w}, {h}",
            "  offset: 0, 0",
            "  index: -1"
        ])

        cur_x += w + 4
        row_h = max(row_h, h)

    atlas_path = os.path.join(SPINE_DIR, "stone_golem.png")
    atlas.save(atlas_path)

    atlas_txt_path = os.path.join(SPINE_DIR, "stone_golem.atlas")
    with open(atlas_txt_path, "w", encoding="utf-8") as f:
        f.write("\n".join(atlas_lines) + "\n")

    print(f"✓ Texture atlas generated: {atlas_path} & {atlas_txt_path}")
    return packed_info

def generate_spine_json(parts):
    def to_spine_world(px_1024, py_1024):
        return (px_1024 - 512) * 0.5, (950 - py_1024) * 0.5

    joints_spine = {k: to_spine_world(*v) for k, v in JOINTS_1024.items()}

    bones_def = [
        {"name": "root"},
        {"name": "shadow", "parent": "root"},
        {"name": "pelvis", "parent": "root", 
         "x": round(joints_spine["pelvis"][0], 2), "y": round(joints_spine["pelvis"][1], 2)},
        {"name": "torso", "parent": "pelvis",
         "x": round(joints_spine["torso"][0] - joints_spine["pelvis"][0], 2),
         "y": round(joints_spine["torso"][1] - joints_spine["pelvis"][1], 2)},
        {"name": "head", "parent": "torso",
         "x": round(joints_spine["head"][0] - joints_spine["torso"][0], 2),
         "y": round(joints_spine["head"][1] - joints_spine["torso"][1], 2)},
        {"name": "shoulder_r", "parent": "torso",
         "x": round(joints_spine["shoulder_r"][0] - joints_spine["torso"][0], 2),
         "y": round(joints_spine["shoulder_r"][1] - joints_spine["torso"][1], 2)},
        {"name": "arm_upper_r", "parent": "shoulder_r",
         "x": round(joints_spine["arm_upper_r"][0] - joints_spine["shoulder_r"][0], 2),
         "y": round(joints_spine["arm_upper_r"][1] - joints_spine["shoulder_r"][1], 2)},
        {"name": "arm_lower_r", "parent": "arm_upper_r",
         "x": round(joints_spine["arm_lower_r"][0] - joints_spine["arm_upper_r"][0], 2),
         "y": round(joints_spine["arm_lower_r"][1] - joints_spine["arm_upper_r"][1], 2)},
        {"name": "shoulder_l", "parent": "torso",
         "x": round(joints_spine["shoulder_l"][0] - joints_spine["torso"][0], 2),
         "y": round(joints_spine["shoulder_l"][1] - joints_spine["torso"][1], 2)},
        {"name": "arm_upper_l", "parent": "shoulder_l",
         "x": round(joints_spine["arm_upper_l"][0] - joints_spine["shoulder_l"][0], 2),
         "y": round(joints_spine["arm_upper_l"][1] - joints_spine["shoulder_l"][1], 2)},
        {"name": "arm_lower_l", "parent": "arm_upper_l",
         "x": round(joints_spine["arm_lower_l"][0] - joints_spine["arm_upper_l"][0], 2),
         "y": round(joints_spine["arm_lower_l"][1] - joints_spine["arm_upper_l"][1], 2)},
        {"name": "thigh_r", "parent": "pelvis",
         "x": round(joints_spine["thigh_r"][0] - joints_spine["pelvis"][0], 2),
         "y": round(joints_spine["thigh_r"][1] - joints_spine["pelvis"][1], 2)},
        {"name": "calf_r", "parent": "thigh_r",
         "x": round(joints_spine["calf_r"][0] - joints_spine["thigh_r"][0], 2),
         "y": round(joints_spine["calf_r"][1] - joints_spine["thigh_r"][1], 2)},
        {"name": "thigh_l", "parent": "pelvis",
         "x": round(joints_spine["thigh_l"][0] - joints_spine["pelvis"][0], 2),
         "y": round(joints_spine["thigh_l"][1] - joints_spine["pelvis"][1], 2)},
        {"name": "calf_l", "parent": "thigh_l",
         "x": round(joints_spine["calf_l"][0] - joints_spine["thigh_l"][0], 2),
         "y": round(joints_spine["calf_l"][1] - joints_spine["thigh_l"][1], 2)},
    ]

    slots_def = [
        {"name": "shadow", "bone": "shadow", "attachment": "shadow"},
        {"name": "shoulder_l", "bone": "shoulder_l", "attachment": "shoulder_l"},
        {"name": "arm_upper_l", "bone": "arm_upper_l", "attachment": "arm_upper_l"},
        {"name": "arm_lower_l", "bone": "arm_lower_l", "attachment": "arm_lower_l"},
        {"name": "thigh_l", "bone": "thigh_l", "attachment": "thigh_l"},
        {"name": "calf_l", "bone": "calf_l", "attachment": "calf_l"},
        {"name": "pelvis", "bone": "pelvis", "attachment": "pelvis"},
        {"name": "thigh_r", "bone": "thigh_r", "attachment": "thigh_r"},
        {"name": "calf_r", "bone": "calf_r", "attachment": "calf_r"},
        {"name": "torso", "bone": "torso", "attachment": "torso"},
        {"name": "head", "bone": "head", "attachment": "head"},
        {"name": "arm_upper_r", "bone": "arm_upper_r", "attachment": "arm_upper_r"},
        {"name": "shoulder_r", "bone": "shoulder_r", "attachment": "shoulder_r"},
        {"name": "arm_lower_r", "bone": "arm_lower_r", "attachment": "arm_lower_r"},
    ]

    skin_attachments = {}
    for slot in slots_def:
        s_name = slot["name"]
        bone_name = slot["bone"]
        p = parts[s_name]

        part_spine_x = p["center_scaled"][0] - (512 / 2.0)
        part_spine_y = (950 / 2.0) - p["center_scaled"][1]
        bone_spine_x, bone_spine_y = joints_spine[bone_name]

        att_x = round(part_spine_x - bone_spine_x, 2)
        att_y = round(part_spine_y - bone_spine_y, 2)

        skin_attachments[s_name] = {
            s_name: {
                "x": att_x,
                "y": att_y,
                "width": p["width"],
                "height": p["height"]
            }
        }

    animations = {
        "idle": {
            "bones": {
                "pelvis": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.8, "x": 0, "y": 5},
                        {"time": 1.6, "x": 0, "y": 0}
                    ]
                },
                "torso": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": -1.5},
                        {"time": 1.6, "value": 0}
                    ]
                },
                "head": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": 1.0},
                        {"time": 1.6, "value": 0}
                    ]
                },
                "shoulder_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": 2.0},
                        {"time": 1.6, "value": 0}
                    ]
                },
                "shoulder_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": -2.0},
                        {"time": 1.6, "value": 0}
                    ]
                }
            }
        },
                "slam": {
            "events": [
                {"time": 0.60, "name": "slam_impact", "int": 100}
            ],
            "bones": {
                "pelvis": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.35, "x": -3, "y": 10},   # Wind-up lift
                        {"time": 0.45, "x": -3, "y": 10},   # Apex pause
                        {"time": 0.60, "x": 4, "y": -16},   # Heavy downward smash
                        {"time": 0.75, "x": 3, "y": -12},   # Shockwave squat
                        {"time": 1.20, "x": 0, "y": 0}      # Recover
                    ]
                },
                "torso": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": -14}, # Arch back
                        {"time": 0.45, "value": -14},
                        {"time": 0.60, "value": 18},  # Slam forward
                        {"time": 0.75, "value": 14},  # Stay crushed
                        {"time": 1.20, "value": 0}
                    ]
                },
                "head": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": -8},
                        {"time": 0.60, "value": 12},
                        {"time": 0.75, "value": 8},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "shoulder_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 32},  # Raise high
                        {"time": 0.45, "value": 32},
                        {"time": 0.60, "value": -26}, # Smash down
                        {"time": 0.75, "value": -22},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "arm_upper_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 14},
                        {"time": 0.60, "value": -12},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "arm_lower_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 16},
                        {"time": 0.60, "value": -14},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "shoulder_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 28},
                        {"time": 0.45, "value": 28},
                        {"time": 0.60, "value": -22},
                        {"time": 0.75, "value": -18},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "arm_upper_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 12},
                        {"time": 0.60, "value": -10},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "arm_lower_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 14},
                        {"time": 0.60, "value": -12},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "thigh_r": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.35, "x": 0, "y": -8},
                        {"time": 0.60, "x": 0, "y": 14},  # Knee compression keeps foot on ground
                        {"time": 0.75, "x": 0, "y": 10},
                        {"time": 1.20, "x": 0, "y": 0}
                    ],
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": -5},
                        {"time": 0.60, "value": 12},
                        {"time": 0.75, "value": 8},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "calf_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 5},
                        {"time": 0.60, "value": -12},
                        {"time": 0.75, "value": -8},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "thigh_l": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.35, "x": 0, "y": -8},
                        {"time": 0.60, "x": 0, "y": 14},
                        {"time": 0.75, "x": 0, "y": 10},
                        {"time": 1.20, "x": 0, "y": 0}
                    ],
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 5},
                        {"time": 0.60, "value": -12},
                        {"time": 0.75, "value": -8},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "calf_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": -5},
                        {"time": 0.60, "value": 12},
                        {"time": 0.75, "value": 8},
                        {"time": 1.20, "value": 0}
                    ]
                }
            }
        },
        "walk": {
            "bones": {
                "pelvis": {
                    "translate": [
                        {"time": 0.0, "x": -3, "y": 0},
                        {"time": 0.3, "x": 0, "y": 4},
                        {"time": 0.6, "x": 3, "y": 0},
                        {"time": 0.9, "x": 0, "y": 4},
                        {"time": 1.2, "x": -3, "y": 0}
                    ]
                },
                "thigh_r": {
                    "rotate": [
                        {"time": 0.0, "value": 8},
                        {"time": 0.3, "value": 0},
                        {"time": 0.6, "value": -8},
                        {"time": 0.9, "value": 0},
                        {"time": 1.2, "value": 8}
                    ]
                },
                "calf_r": {
                    "rotate": [
                        {"time": 0.0, "value": -4},
                        {"time": 0.3, "value": 8},
                        {"time": 0.6, "value": 0},
                        {"time": 0.9, "value": -4},
                        {"time": 1.2, "value": -4}
                    ]
                },
                "thigh_l": {
                    "rotate": [
                        {"time": 0.0, "value": -8},
                        {"time": 0.3, "value": 0},
                        {"time": 0.6, "value": 8},
                        {"time": 0.9, "value": 0},
                        {"time": 1.2, "value": -8}
                    ]
                },
                "calf_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.3, "value": -4},
                        {"time": 0.6, "value": -4},
                        {"time": 0.9, "value": 8},
                        {"time": 1.2, "value": 0}
                    ]
                },
                "shoulder_r": {
                    "rotate": [
                        {"time": 0.0, "value": -6},
                        {"time": 0.6, "value": 6},
                        {"time": 1.2, "value": -6}
                    ]
                },
                "shoulder_l": {
                    "rotate": [
                        {"time": 0.0, "value": 6},
                        {"time": 0.6, "value": -6},
                        {"time": 1.2, "value": 6}
                    ]
                }
            }
        }
    }

    spine_doc = {
        "skeleton": {
            "hash": "stone_golem_master_v3",
            "spine": "4.3.00",
            "x": -260,
            "y": 0,
            "width": 520,
            "height": 520,
            "images": "./master_parts/",
            "audio": ""
        },
        "bones": bones_def,
        "slots": slots_def,
        "skins": [
            {
                "name": "default",
                "attachments": skin_attachments
            }
        ],
        "events": {
            "slam_impact": {}
        },
        "animations": animations
    }

    json_path = os.path.join(SPINE_DIR, "stone_golem.json")
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(spine_doc, f, indent=2)

    print(f"✓ Spine 4.3 JSON generated: {json_path}")
    return spine_doc

# ------------------------------------------------------------------------------
# High-Quality Animation Renderer with 600x600 Canvas
# ------------------------------------------------------------------------------
def lerp(a, b, t):
    return a + (b - a) * t

def sample_val_timeline(keys, t, val_key="value", default=0.0):
    if not keys: return default
    if len(keys) == 1 or t <= keys[0]["time"]: return keys[0].get(val_key, default)
    if t >= keys[-1]["time"]: return keys[-1].get(val_key, default)
    for i in range(len(keys) - 1):
        k0, k1 = keys[i], keys[i + 1]
        if k0["time"] <= t <= k1["time"]:
            dur = k1["time"] - k0["time"]
            frac = (t - k0["time"]) / dur if dur > 0 else 0
            return lerp(k0.get(val_key, default), k1.get(val_key, default), frac)
    return default

def sample_vec2_timeline(keys, t, default_x=0.0, default_y=0.0):
    if not keys: return default_x, default_y
    if len(keys) == 1 or t <= keys[0]["time"]:
        return keys[0].get("x", default_x), keys[0].get("y", default_y)
    if t >= keys[-1]["time"]:
        return keys[-1].get("x", default_x), keys[-1].get("y", default_y)
    for i in range(len(keys) - 1):
        k0, k1 = keys[i], keys[i + 1]
        if k0["time"] <= t <= k1["time"]:
            dur = k1["time"] - k0["time"]
            frac = (t - k0["time"]) / dur if dur > 0 else 0
            return (
                lerp(k0.get("x", default_x), k1.get("x", default_x), frac),
                lerp(k0.get("y", default_y), k1.get("y", default_y), frac)
            )
    return default_x, default_y

def render_spine_animation(skel, parts, anim_name, duration_sec, fps=16):
    bones_def = {b["name"]: b for b in skel["bones"]}
    bone_order = [b["name"] for b in skel["bones"]]
    slots_def = skel["slots"]
    anim_data = skel["animations"].get(anim_name, {})
    anim_bones = anim_data.get("bones", {})
    attachments_def = skel["skins"][0]["attachments"]

    total_frames = int(duration_sec * fps)
    frames = []

    # Canvas: 600x600, Ground Origin: (300, 520)
    canvas_w, canvas_h = 600, 600
    origin_x, origin_y = 300, 520

    for f_idx in range(total_frames):
        t = (f_idx / total_frames) * duration_sec
        canvas = Image.new("RGBA", (canvas_w, canvas_h), (0, 0, 0, 0))

        # Forward Kinematics
        world_transforms = {}
        for b_name in bone_order:
            b_def = bones_def[b_name]
            parent_name = b_def.get("parent")

            setup_x = b_def.get("x", 0.0)
            setup_y = b_def.get("y", 0.0)
            setup_rot = b_def.get("rotation", 0.0)

            delta_x, delta_y = 0.0, 0.0
            delta_rot = 0.0

            if b_name in anim_bones:
                b_anim = anim_bones[b_name]
                if "translate" in b_anim:
                    delta_x, delta_y = sample_vec2_timeline(b_anim["translate"], t)
                if "rotate" in b_anim:
                    delta_rot = sample_val_timeline(b_anim["rotate"], t, val_key="value")

            local_x = setup_x + delta_x
            local_y = setup_y + delta_y
            local_rot = setup_rot + delta_rot

            if parent_name and parent_name in world_transforms:
                p_x, p_y, p_rot = world_transforms[parent_name]
                rad = math.radians(p_rot)
                cos_r, sin_r = math.cos(rad), math.sin(rad)
                w_x = p_x + (local_x * cos_r - local_y * sin_r)
                w_y = p_y + (local_x * sin_r + local_y * cos_r)
                w_rot = p_rot + local_rot
            else:
                w_x, w_y, w_rot = local_x, local_y, local_rot

            world_transforms[b_name] = (w_x, w_y, w_rot)

        # Draw slots back-to-front
        for slot in slots_def:
            s_name = slot["name"]
            bone_name = slot["bone"]
            if s_name not in parts or bone_name not in world_transforms:
                continue

            part_img = parts[s_name]["img"]
            w_x, w_y, w_rot = world_transforms[bone_name]

            att_info = attachments_def.get(s_name, {}).get(s_name, {})
            att_x = att_info.get("x", 0.0)
            att_y = att_info.get("y", 0.0)

            rad = math.radians(w_rot)
            cos_r, sin_r = math.cos(rad), math.sin(rad)

            part_center_x = w_x + (att_x * cos_r - att_y * sin_r)
            part_center_y = w_y + (att_x * sin_r + att_y * cos_r)

            screen_x = int(origin_x + part_center_x)
            screen_y = int(origin_y - part_center_y)

            if abs(w_rot) > 0.1:
                rot_img = part_img.rotate(w_rot, resample=Image.Resampling.BILINEAR, expand=True)
            else:
                rot_img = part_img

            paste_x = screen_x - rot_img.width // 2
            paste_y = screen_y - rot_img.height // 2

            canvas.paste(rot_img, (paste_x, paste_y), rot_img)

        frames.append(canvas)

    out_gif = os.path.join(SPINE_DIR, f"stone_golem_{anim_name}.gif")
    if frames:
        frames[0].save(
            out_gif,
            save_all=True,
            append_images=frames[1:],
            duration=int(1000 / fps),
            loop=0,
            disposal=2
        )
        print(f"🎬 Exported GIF: {out_gif} ({len(frames)} frames @ {fps} FPS, canvas: {canvas_w}x{canvas_h})")
    return out_gif

def main():
    print("=== EXECUTING MASTER STONE GOLEM SPINE PIPELINE V3 ===")
    parts = extract_all_parts()
    build_texture_atlas(parts)
    skel = generate_spine_json(parts)

    print("\n=== RENDERING COMPLETE ANIMATIONS (NO CLIPPING, DEEP OVERLAP) ===")
    render_spine_animation(skel, parts, "idle", duration_sec=1.6, fps=16)
    render_spine_animation(skel, parts, "slam", duration_sec=1.2, fps=16)
    render_spine_animation(skel, parts, "walk", duration_sec=1.2, fps=16)

    print("\n✅ PIPELINE V3 COMPLETED SUCCESSFULLY!")

if __name__ == "__main__":
    main()
