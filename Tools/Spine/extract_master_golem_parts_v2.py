#!/usr/bin/env python3
import os
import json
import math
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

MASTER_PATH = "Art_Gallery/legacy/characters/01_Boss_Stone_Golem_Transparent.png"
OUTPUT_DIR = "Content/art/characters/boss/spine"
PARTS_DIR = os.path.join(OUTPUT_DIR, "master_parts")
os.makedirs(PARTS_DIR, exist_ok=True)

# Primary color palette from master art
OUTLINE_COL = (20, 18, 30, 255)
STONE_DARK  = (75, 78, 85, 255)
STONE_MID   = (128, 122, 105, 255)
STONE_LIGHT = (184, 180, 145, 255)
MOSS_GREEN  = (65, 110, 35, 255)

def clean_master(img):
    arr = np.array(img)
    # Magenta background chroma residue
    chroma = (arr[:, :, 3] > 0) & (arr[:, :, 0] > 110) & (arr[:, :, 2] > 110) & (arr[:, :, 1] < 45)
    arr[chroma] = [0, 0, 0, 0]
    return Image.fromarray(arr)

def draw_capsule_joint(draw, center_x, center_y, radius, start_angle=0, end_angle=360):
    """Draws a shaded stone ball-joint overdraw with dark outline and stone texture."""
    x0, y0 = center_x - radius, center_y - radius
    x1, y1 = center_x + radius, center_y + radius
    # Dark outline
    draw.ellipse([x0 - 2, y0 - 2, x1 + 2, y1 + 2], fill=OUTLINE_COL)
    # Stone dark body
    draw.ellipse([x0, y0, x1, y1], fill=STONE_DARK)
    # Stone mid shading
    r_mid = radius * 0.8
    draw.ellipse([center_x - r_mid, center_y - r_mid + 2, center_x + r_mid, center_y + r_mid], fill=STONE_MID)
    # Stone light highlight
    r_lit = radius * 0.45
    draw.ellipse([center_x - r_lit - 2, center_y - r_lit, center_x + r_lit - 2, center_y + r_lit], fill=STONE_LIGHT)

# Anatomical Masks (1024x1024)
MASKS = {
    "head": [
        (420, 85), (630, 85), (665, 180), (660, 280), (620, 350), 
        (490, 360), (450, 350), (410, 260), (405, 150)
    ],
    "torso": [
        (310, 280), (650, 250), (705, 340), (680, 565), (365, 570), 
        (310, 430), (290, 340)
    ],
    "pelvis": [
        (340, 530), (680, 530), (670, 680), (510, 695), (340, 675)
    ],
    "shoulder_r": [
        (135, 150), (385, 150), (400, 375), (280, 425), (135, 320)
    ],
    "arm_upper_r": [
        (120, 320), (280, 320), (280, 520), (120, 520)
    ],
    "arm_lower_r": [
        (35, 430), (325, 430), (330, 825), (45, 825)
    ],
    "shoulder_l": [
        (580, 75), (780, 75), (780, 280), (610, 280)
    ],
    "arm_upper_l": [
        (710, 100), (880, 100), (880, 300), (710, 300)
    ],
    "arm_lower_l": [
        (700, 75), (990, 75), (990, 490), (700, 490)
    ],
    "thigh_r": [
        (310, 550), (480, 550), (470, 755), (310, 755)
    ],
    "calf_r": [
        (260, 680), (480, 680), (480, 960), (260, 960)
    ],
    "thigh_l": [
        (495, 540), (685, 540), (685, 740), (495, 740)
    ],
    "calf_l": [
        (510, 665), (750, 665), (750, 845), (510, 845)
    ]
}

# Anatomical Joints / Pivots in 1024x1024 Master Art coordinates
JOINTS = {
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

def extract_and_overdraw_parts():
    print(f"Loading master art: {MASTER_PATH}...")
    master = Image.open(MASTER_PATH).convert("RGBA")
    master = clean_master(master)

    parts = {}

    for name, poly in MASKS.items():
        # Mask image
        mask = Image.new("L", master.size, 0)
        draw = ImageDraw.Draw(mask)
        draw.polygon(poly, fill=255)

        part_img = Image.new("RGBA", master.size, (0, 0, 0, 0))
        part_img.paste(master, (0, 0), mask)
        
        # Add anatomical joint caps / overdraws
        od_draw = ImageDraw.Draw(part_img)
        
        if name == "head":
            # Neck ball joint tucking into torso collar
            draw_capsule_joint(od_draw, 510, 340, radius=32)
            # Re-paste face over the neck so face is sharp
            face_mask = Image.new("L", master.size, 0)
            ImageDraw.Draw(face_mask).polygon([(420, 85), (630, 85), (665, 280), (620, 335), (410, 260)], fill=255)
            part_img.paste(master, (0, 0), face_mask)

        elif name == "arm_upper_r":
            # Shoulder ball joint (top)
            draw_capsule_joint(od_draw, 255, 360, radius=38)
            # Elbow socket (bottom)
            draw_capsule_joint(od_draw, 205, 480, radius=35)
            # Re-paste mid bicep
            bicep_mask = Image.new("L", master.size, 0)
            ImageDraw.Draw(bicep_mask).rectangle([130, 370, 270, 470], fill=255)
            part_img.paste(master, (0, 0), bicep_mask)

        elif name == "arm_lower_r":
            # Elbow ball joint (top)
            draw_capsule_joint(od_draw, 195, 500, radius=42)
            # Re-paste forearm & fist
            arm_mask = Image.new("L", master.size, 0)
            ImageDraw.Draw(arm_mask).rectangle([35, 520, 325, 825], fill=255)
            part_img.paste(master, (0, 0), arm_mask)

        elif name == "arm_upper_l":
            # Shoulder ball joint
            draw_capsule_joint(od_draw, 745, 175, radius=36)
            # Elbow socket
            draw_capsule_joint(od_draw, 815, 235, radius=34)

        elif name == "arm_lower_l":
            # Elbow ball joint
            draw_capsule_joint(od_draw, 810, 230, radius=38)
            # Re-paste raised forearm
            fa_mask = Image.new("L", master.size, 0)
            ImageDraw.Draw(fa_mask).rectangle([720, 75, 990, 490], fill=255)
            part_img.paste(master, (0, 0), fa_mask)

        elif name == "thigh_r":
            # Hip ball joint under pelvis
            draw_capsule_joint(od_draw, 415, 600, radius=42)
            # Knee joint over shin
            draw_capsule_joint(od_draw, 380, 725, radius=40)
            # Inpaint stone block on left flank where fist previously occluded
            od_draw.rectangle([305, 620, 345, 735], fill=STONE_DARK)
            od_draw.rectangle([315, 630, 345, 725], fill=STONE_MID)
            od_draw.line([(305, 620), (305, 735)], fill=OUTLINE_COL, width=3)
            # Re-paste main thigh
            thigh_mask = Image.new("L", master.size, 0)
            ImageDraw.Draw(thigh_mask).rectangle([345, 590, 475, 730], fill=255)
            part_img.paste(master, (0, 0), thigh_mask)

        elif name == "calf_r":
            # Knee ball joint (top)
            draw_capsule_joint(od_draw, 375, 735, radius=44)
            # Inpaint left shin flank where fist previously occluded
            od_draw.rectangle([270, 740, 310, 810], fill=STONE_DARK)
            od_draw.rectangle([280, 745, 310, 805], fill=STONE_MID)
            od_draw.line([(270, 740), (270, 810)], fill=OUTLINE_COL, width=3)
            # Re-paste shin and complete foot
            calf_mask = Image.new("L", master.size, 0)
            ImageDraw.Draw(calf_mask).rectangle([300, 745, 480, 960], fill=255)
            part_img.paste(master, (0, 0), calf_mask)

        elif name == "thigh_l":
            # Hip ball joint
            draw_capsule_joint(od_draw, 585, 595, radius=40)
            # Knee joint
            draw_capsule_joint(od_draw, 625, 715, radius=38)

        elif name == "calf_l":
            # Knee ball joint
            draw_capsule_joint(od_draw, 625, 710, radius=42)
            # Re-paste shin and foot
            cl_mask = Image.new("L", master.size, 0)
            ImageDraw.Draw(cl_mask).rectangle([510, 725, 750, 845], fill=255)
            part_img.paste(master, (0, 0), cl_mask)

        elif name == "pelvis":
            # Rounded top ridge extending under chest
            od_draw.rectangle([370, 535, 650, 565], fill=STONE_DARK)
            od_draw.rectangle([385, 540, 635, 560], fill=STONE_MID)
            od_draw.line([(370, 535), (650, 535)], fill=OUTLINE_COL, width=3)
            # Re-paste pelvis center
            pel_mask = Image.new("L", master.size, 0)
            ImageDraw.Draw(pel_mask).rectangle([340, 555, 675, 680], fill=255)
            part_img.paste(master, (0, 0), pel_mask)

        bbox = part_img.getbbox()
        if not bbox:
            print(f"Warning: {name} has empty bbox!")
            continue

        cropped = part_img.crop(bbox)
        # Scale by 0.5 for game resolution
        w_scaled = max(1, cropped.width // 2)
        h_scaled = max(1, cropped.height // 2)
        scaled = cropped.resize((w_scaled, h_scaled), Image.Resampling.LANCZOS)
        
        p_path = os.path.join(PARTS_DIR, f"{name}.png")
        scaled.save(p_path)

        parts[name] = {
            "orig_bbox": bbox,
            "orig_center": ((bbox[0] + bbox[2]) / 2, (bbox[1] + bbox[3]) / 2),
            "width": w_scaled,
            "height": h_scaled,
            "img": scaled
        }
        print(f"  ✓ Processed {name}: {w_scaled}x{h_scaled} (orig bbox: {bbox})")

    # Shadow
    sw, sh = 260, 90
    shadow = Image.new("RGBA", (sw, sh), (0, 0, 0, 0))
    s_draw = ImageDraw.Draw(shadow)
    s_draw.ellipse([8, 10, sw - 8, sh - 10], fill=(12, 10, 16, 170))
    s_draw.ellipse([35, 22, sw - 35, sh - 22], fill=(6, 5, 10, 230))
    shadow_path = os.path.join(PARTS_DIR, "shadow.png")
    shadow.save(shadow_path)
    parts["shadow"] = {
        "orig_bbox": (382, 905, 642, 995),
        "orig_center": (512, 950),
        "width": sw,
        "height": sh,
        "img": shadow
    }

    return parts

print("Extraction logic v2 ready.")
