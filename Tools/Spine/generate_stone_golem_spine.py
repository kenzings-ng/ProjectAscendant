#!/usr/bin/env python3
# ==============================================================================
# generate_stone_golem_spine.py
# Generates complete modular cut-out parts, packed Atlas, and Spine 4.3 JSON
# for Stone Golem Boss in Project Ascendant.
# ==============================================================================

import os
import sys
import json
import zlib
import struct
from PIL import Image, ImageDraw, ImageFilter

OUTPUT_DIR = "Content/art/characters/boss/spine"
PARTS_DIR = os.path.join(OUTPUT_DIR, "parts")
ASEPRITE_DIR = "Art_Gallery/aseprite/boss"
MASTER_IMAGE_PATH = "Art_Gallery/legacy/characters/01_Boss_Stone_Golem_Transparent.png"

os.makedirs(PARTS_DIR, exist_ok=True)
os.makedirs(ASEPRITE_DIR, exist_ok=True)

# ------------------------------------------------------------------------------
# 1. Modular Parts Definition & Extraction from Master Art
# Coordinates on 1024x1024 master image (box: left, upper, right, lower)
# ------------------------------------------------------------------------------
PARTS_CONFIG = {
    "shadow": {
        "box": (140, 720, 880, 990),
        "target_size": (180, 70),
        "type": "shadow"
    },
    "head": {
        "box": (420, 90, 640, 360),
        "target_size": (70, 85),
        "bone": "head",
        "parent": "torso",
        "pivot_rel": (35, 75)
    },
    "torso": {
        "box": (340, 300, 680, 600),
        "target_size": (110, 95),
        "bone": "torso",
        "parent": "pelvis",
        "pivot_rel": (55, 80)
    },
    "pelvis": {
        "box": (360, 560, 640, 710),
        "target_size": (90, 50),
        "bone": "pelvis",
        "parent": "root",
        "pivot_rel": (45, 25)
    },
    "shoulder_r": {
        "box": (140, 160, 370, 390),
        "target_size": (75, 75),
        "bone": "shoulder_r",
        "parent": "torso",
        "pivot_rel": (55, 35)
    },
    "arm_upper_r": {
        "box": (80, 350, 240, 500),
        "target_size": (50, 50),
        "bone": "arm_upper_r",
        "parent": "shoulder_r",
        "pivot_rel": (35, 15)
    },
    "arm_lower_r": {
        "box": (40, 460, 290, 680),
        "target_size": (75, 70),
        "bone": "arm_lower_r",
        "parent": "arm_upper_r",
        "pivot_rel": (50, 20)
    },
    "fist_r": {
        "box": (90, 610, 320, 790),
        "target_size": (70, 60),
        "bone": "fist_r",
        "parent": "arm_lower_r",
        "pivot_rel": (40, 15)
    },
    "shoulder_l": {
        "box": (600, 90, 760, 260),
        "target_size": (60, 60),
        "bone": "shoulder_l",
        "parent": "torso",
        "pivot_rel": (20, 45)
    },
    "arm_upper_l": {
        "box": (730, 120, 890, 330),
        "target_size": (60, 65),
        "bone": "arm_upper_l",
        "parent": "shoulder_l",
        "pivot_rel": (25, 45)
    },
    "arm_lower_l": {
        "box": (720, 260, 950, 480),
        "target_size": (70, 70),
        "bone": "arm_lower_l",
        "parent": "arm_upper_l",
        "pivot_rel": (25, 45)
    },
    "fist_l": {
        "box": (740, 90, 960, 300),
        "target_size": (70, 65),
        "bone": "fist_l",
        "parent": "arm_lower_l",
        "pivot_rel": (25, 45)
    },
    "thigh_r": {
        "box": (330, 580, 460, 760),
        "target_size": (50, 55),
        "bone": "thigh_r",
        "parent": "pelvis",
        "pivot_rel": (35, 15)
    },
    "calf_r": {
        "box": (280, 730, 450, 920),
        "target_size": (55, 60),
        "bone": "calf_r",
        "parent": "thigh_r",
        "pivot_rel": (35, 15)
    },
    "foot_r": {
        "box": (270, 850, 460, 935),
        "target_size": (60, 35),
        "bone": "foot_r",
        "parent": "calf_r",
        "pivot_rel": (40, 10)
    },
    "thigh_l": {
        "box": (510, 550, 670, 670),
        "target_size": (55, 50),
        "bone": "thigh_l",
        "parent": "pelvis",
        "pivot_rel": (20, 15)
    },
    "calf_l": {
        "box": (500, 610, 680, 740),
        "target_size": (60, 55),
        "bone": "calf_l",
        "parent": "thigh_l",
        "pivot_rel": (25, 15)
    },
    "foot_l": {
        "box": (570, 715, 735, 835),
        "target_size": (65, 45),
        "bone": "foot_l",
        "parent": "calf_l",
        "pivot_rel": (30, 10)
    }
}

def generate_modular_parts():
    print(f"Loading master art from {MASTER_IMAGE_PATH}...")
    master = Image.open(MASTER_IMAGE_PATH).convert("RGBA")
    
    extracted_parts = {}

    for name, cfg in PARTS_CONFIG.items():
        target_w, target_h = cfg["target_size"]
        
        if name == "shadow":
            # Create a clean, smooth translucent dark ambient occlusion shadow ellipse
            part_img = Image.new("RGBA", (target_w, target_h), (0, 0, 0, 0))
            draw = ImageDraw.Draw(part_img)
            # Draw multi-layer dark ellipse for depth
            draw.ellipse([10, 10, target_w - 10, target_h - 10], fill=(12, 10, 20, 160))
            draw.ellipse([30, 18, target_w - 30, target_h - 18], fill=(8, 6, 15, 200))
        else:
            box = cfg["box"]
            cropped = master.crop(box)
            part_img = cropped.resize((target_w, target_h), Image.Resampling.NEAREST)
            
            # Clean purple background shadow residue from feet or edges
            pixels = part_img.load()
            for py in range(target_h):
                for px in range(target_w):
                    r, g, b, a = pixels[px, py]
                    # Detect purple legacy shadow (high R and B, low G)
                    if a > 0 and r > 90 and b > 90 and g < 60:
                        pixels[px, py] = (0, 0, 0, 0)
        
        # Save individual part PNG
        part_path = os.path.join(PARTS_DIR, f"{name}.png")
        part_img.save(part_path)
        extracted_parts[name] = {
            "img": part_img,
            "width": target_w,
            "height": target_h
        }
        print(f"  + Generated part: {name} ({target_w}x{target_h}) -> {part_path}")

    return extracted_parts

# ------------------------------------------------------------------------------
# 2. Pack Parts into 512x512 Texture Atlas & Write .atlas file
# ------------------------------------------------------------------------------
def pack_texture_atlas(extracted_parts):
    atlas_w = 512
    atlas_h = 512
    atlas_img = Image.new("RGBA", (atlas_w, atlas_h), (0, 0, 0, 0))
    
    atlas_lines = [
        "stone_golem.png",
        f"size: {atlas_w}, {atlas_h}",
        "format: RGBA8888",
        "filter: Nearest, Nearest",
        "repeat: none"
    ]
    
    # Simple row-based packer with 2px padding
    cur_x = 2
    cur_y = 2
    row_h = 0
    padding = 2
    
    part_regions = {}
    
    # Sort parts by height descending for better packing
    sorted_parts = sorted(extracted_parts.items(), key=lambda item: item[1]["height"], reverse=True)
    
    for name, data in sorted_parts:
        img = data["img"]
        w = data["width"]
        h = data["height"]
        
        if cur_x + w + padding > atlas_w:
            cur_x = 2
            cur_y += row_h + padding
            row_h = 0
            
        if cur_y + h + padding > atlas_h:
            raise RuntimeError(f"Texture atlas overflowed at part {name}! Increase atlas dimensions.")
            
        atlas_img.paste(img, (cur_x, cur_y), img)
        part_regions[name] = {
            "x": cur_x,
            "y": cur_y,
            "width": w,
            "height": h
        }
        
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
        
    print(f"✅ Exported Texture Atlas: {out_atlas_png}")
    print(f"✅ Exported LibGDX Atlas : {out_atlas_file}")
    return part_regions

# ------------------------------------------------------------------------------
# 3. Generate Spine 4.3 Skeleton JSON with 5 Animations
# ------------------------------------------------------------------------------
def build_spine_skeleton_json(part_regions):
    bones = [
        {"name": "root"},
        {"name": "shadow", "parent": "root", "x": 0, "y": 0},
        {"name": "pelvis", "parent": "root", "x": 0, "y": 95},
        {"name": "torso", "parent": "pelvis", "x": 0, "y": 45},
        {"name": "head", "parent": "torso", "x": 0, "y": 75},
        
        # Right Arm (Viewer's left)
        {"name": "shoulder_r", "parent": "torso", "x": -55, "y": 50},
        {"name": "arm_upper_r", "parent": "shoulder_r", "x": -25, "y": -20},
        {"name": "arm_lower_r", "parent": "arm_upper_r", "x": -20, "y": -35},
        {"name": "fist_r", "parent": "arm_lower_r", "x": -10, "y": -35},
        
        # Left Arm (Viewer's right)
        {"name": "shoulder_l", "parent": "torso", "x": 55, "y": 50},
        {"name": "arm_upper_l", "parent": "shoulder_l", "x": 25, "y": -20},
        {"name": "arm_lower_l", "parent": "arm_upper_l", "x": 20, "y": -35},
        {"name": "fist_l", "parent": "arm_lower_l", "x": 10, "y": -35},
        
        # Right Leg
        {"name": "thigh_r", "parent": "pelvis", "x": -30, "y": -15},
        {"name": "calf_r", "parent": "thigh_r", "x": -10, "y": -35},
        {"name": "foot_r", "parent": "calf_r", "x": -5, "y": -35},
        
        # Left Leg
        {"name": "thigh_l", "parent": "pelvis", "x": 30, "y": -15},
        {"name": "calf_l", "parent": "thigh_l", "x": 10, "y": -35},
        {"name": "foot_l", "parent": "calf_l", "x": 5, "y": -35}
    ]
    
    # Layer order (back to front)
    slot_names = [
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
    for s_name in slot_names:
        slots.append({
            "name": s_name,
            "bone": s_name,
            "attachment": s_name
        })
        
    # Default skin attachments
    default_attachments = {}
    for name, r in part_regions.items():
        default_attachments[name] = {
            name: {
                "x": 0,
                "y": 0,
                "width": r["width"],
                "height": r["height"]
            }
        }
        
    # Animations
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
                        {"time": 0.8, "value": -2},
                        {"time": 1.6, "value": 0}
                    ]
                },
                "head": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": 1.5},
                        {"time": 1.6, "value": 0}
                    ]
                },
                "arm_lower_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": -4},
                        {"time": 1.6, "value": 0}
                    ]
                },
                "arm_lower_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.8, "value": 4},
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
                        {"time": 0.3, "x": 0, "y": 5},
                        {"time": 0.6, "x": 0, "y": 0},
                        {"time": 0.9, "x": 0, "y": 5},
                        {"time": 1.2, "x": 0, "y": 0}
                    ]
                },
                "thigh_r": {
                    "rotate": [
                        {"time": 0.0, "value": -15},
                        {"time": 0.6, "value": 15},
                        {"time": 1.2, "value": -15}
                    ]
                },
                "thigh_l": {
                    "rotate": [
                        {"time": 0.0, "value": 15},
                        {"time": 0.6, "value": -15},
                        {"time": 1.2, "value": 15}
                    ]
                },
                "arm_upper_r": {
                    "rotate": [
                        {"time": 0.0, "value": 18},
                        {"time": 0.6, "value": -18},
                        {"time": 1.2, "value": 18}
                    ]
                },
                "arm_upper_l": {
                    "rotate": [
                        {"time": 0.0, "value": -18},
                        {"time": 0.6, "value": 18},
                        {"time": 1.2, "value": -18}
                    ]
                }
            }
        },
        "slam": {
            "bones": {
                "pelvis": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.3, "x": 0, "y": 15},   # Windup jump/lift
                        {"time": 0.45, "x": 0, "y": -12}, # Slam downward impact
                        {"time": 0.75, "x": 0, "y": -12}, # Recovery hold
                        {"time": 1.0, "x": 0, "y": 0}     # Back to idle
                    ]
                },
                "torso": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.3, "value": 12},   # Lean back
                        {"time": 0.45, "value": -22}, # Forward impact smash
                        {"time": 0.75, "value": -15},
                        {"time": 1.0, "value": 0}
                    ]
                },
                "shoulder_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.3, "value": 75},   # Raised high overhead
                        {"time": 0.45, "value": -45}, # Smashed into ground
                        {"time": 0.75, "value": -35},
                        {"time": 1.0, "value": 0}
                    ]
                },
                "shoulder_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.3, "value": 75},   # Raised high overhead
                        {"time": 0.45, "value": -45}, # Smashed into ground
                        {"time": 0.75, "value": -35},
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
                        {"time": 0.15, "x": -15, "y": 0},
                        {"time": 0.5, "x": 0, "y": 0}
                    ]
                },
                "torso": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.15, "value": 15},
                        {"time": 0.5, "value": 0}
                    ]
                },
                "head": {
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
                        {"time": 0.5, "x": 0, "y": -40},
                        {"time": 1.2, "x": 0, "y": -90}
                    ]
                },
                "torso": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.6, "value": 35},
                        {"time": 1.2, "value": 90}
                    ]
                },
                "head": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.6, "x": 25, "y": -60},
                        {"time": 1.2, "x": 65, "y": -120}
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
            "images": "./parts/",
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
        
    print(f"✅ Exported Spine 4.3 Skeleton JSON: {out_json}")

# ------------------------------------------------------------------------------
# 4. Generate Multi-Layer Aseprite File for Stone Golem Modular Rig
# ------------------------------------------------------------------------------
def build_aseprite_file(extracted_parts):
    """
    Constructs a binary .aseprite file containing all modular parts as individual layers
    at canvas 256x256.
    """
    canvas_w = 256
    canvas_h = 256
    layer_names = list(extracted_parts.keys())
    
    frames_count = 1
    filesize_placeholder = 0
    magic = 0xA5E0
    depth = 32 # 32-bit RGBA
    flags = 1
    speed = 100
    
    header = bytearray(128)
    struct.pack_into("<I H H H H H I H I I 3s B 2s B B 2s H B B", header, 0,
                     filesize_placeholder, magic, frames_count, canvas_w, canvas_h, depth,
                     flags, speed, 0, 0, b"\x00\x00\x00", 0, b"\x00\x00", 0, 0, b"\x00\x00", 0, 0, 0)
    
    # Chunks for Frame 0:
    # 1. Color Profile Chunk (0x2007)
    # 2. Layer Chunks (0x2004) for each layer
    # 3. Cel Chunks (0x2005) for each layer
    chunks = []
    
    # Color Profile Chunk
    profile_data = struct.pack("<H H I", 1, 0, 0) # sRGB
    chunks.append(struct.pack("<I H", len(profile_data) + 6, 0x2007) + profile_data)
    
    # Layer chunks
    for i, name in enumerate(layer_names):
        name_bytes = name.encode("utf-8")
        layer_flags = 1 | 2 # visible + editable
        layer_type = 0 # normal image layer
        child_level = 0
        blend_mode = 0 # normal
        opacity = 255
        
        layer_payload = struct.pack("<H H H H H B 3s", layer_flags, layer_type, child_level, canvas_w, canvas_h, blend_mode, b"\x00\x00\x00")
        layer_payload += struct.pack("<B", opacity)
        layer_payload += struct.pack("<3s", b"\x00\x00\x00")
        layer_payload += struct.pack("<H", len(name_bytes)) + name_bytes
        
        chunks.append(struct.pack("<I H", len(layer_payload) + 6, 0x2004) + layer_payload)
        
    # Cel chunks
    for i, name in enumerate(layer_names):
        data = extracted_parts[name]
        img = data["img"]
        w = data["width"]
        h = data["height"]
        
        # Center in canvas
        x = (canvas_w - w) // 2
        y = (canvas_h - h) // 2
        
        raw_pixels = bytearray()
        for py in range(h):
            for px in range(w):
                r, g, b, a = img.getpixel((px, py))
                raw_pixels.extend([r, g, b, a])
                
        compressed = zlib.compress(raw_pixels)
        
        cel_header = struct.pack("<H h h B H 7s", i, x, y, 255, 2, b"\x00" * 7) # cel type 2 = compressed image
        image_header = struct.pack("<H H", w, h)
        cel_payload = cel_header + image_header + compressed
        
        chunks.append(struct.pack("<I H", len(cel_payload) + 6, 0x2005) + cel_payload)
        
    frame_body = b"".join(chunks)
    frame_size = len(frame_body) + 16
    frame_header = struct.pack("<I H H H 2s I", frame_size, 0xF1FA, min(len(chunks), 0xFFFF), 100, b"\x00\x00", len(chunks))
    
    total_data = header + frame_header + frame_body
    struct.pack_into("<I", total_data, 0, len(total_data))
    
    out_aseprite = os.path.join(ASEPRITE_DIR, "stone_golem_modular_rig.aseprite")
    with open(out_aseprite, "wb") as f:
        f.write(total_data)
        
    print(f"✅ Exported Aseprite Source Rig: {out_aseprite} ({len(layer_names)} layers)")

def main():
    print("=== Generating Stone Golem Spine 4.3 & Aseprite Assets ===")
    parts = generate_modular_parts()
    regions = pack_texture_atlas(parts)
    build_spine_skeleton_json(regions)
    build_aseprite_file(parts)
    print("\n🎉 Stone Golem Boss Spine 4.3 Asset Production COMPLETE!")

if __name__ == "__main__":
    main()
