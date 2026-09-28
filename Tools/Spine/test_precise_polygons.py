#!/usr/bin/env python3
import os
import numpy as np
from PIL import Image, ImageDraw

MASTER_PATH = "Art_Gallery/legacy/characters/01_Boss_Stone_Golem_Transparent.png"
master = Image.open(MASTER_PATH).convert("RGBA")

# Chroma key residue removal
arr = np.array(master)
chroma = (arr[:, :, 3] > 0) & (arr[:, :, 0] > 110) & (arr[:, :, 2] > 110) & (arr[:, :, 1] < 45)
arr[chroma] = [0, 0, 0, 0]
clean_master = Image.fromarray(arr)

PARTS_POLYS = {
    "head": [
        (380, 85), (660, 85), (680, 180), (670, 280), (630, 365), 
        (470, 365), (430, 355), (385, 260), (380, 150)
    ],
    "torso": [
        (280, 260), (660, 240), (715, 330), (700, 580), (340, 585), 
        (280, 430), (270, 330)
    ],
    "pelvis": [
        (335, 530), (685, 530), (680, 695), (500, 705), (335, 685)
    ],
    "shoulder_r": [
        (135, 140), (410, 140), (410, 430), (260, 430), (135, 320)
    ],
    "arm_upper_r": [
        (120, 320), (265, 320), (265, 510), (120, 510)
    ],
    "arm_lower_r": [
        (35, 410), (330, 410), (330, 600), (310, 600), (310, 750), (285, 750), (285, 830), (35, 830)
    ],
    "shoulder_l": [
        (570, 70), (790, 70), (790, 290), (570, 290)
    ],
    "arm_upper_l": [
        (700, 95), (890, 95), (890, 310), (700, 310)
    ],
    "arm_lower_l": [
        (690, 70), (990, 70), (990, 500), (690, 500)
    ],
    "thigh_r": [
        (330, 540), (490, 540), (490, 765), (330, 765)
    ],
    "calf_r": [
        (325, 660), (490, 660), (490, 965), 
        (270, 965), (270, 785), (325, 785)
    ],
    "thigh_l": [
        (485, 530), (700, 530), (700, 750), (485, 750)
    ],
    "calf_l": [
        (495, 655), (750, 655), (750, 855), (495, 855)
    ]
}

DRAW_ORDER = [
    "shoulder_l", "arm_upper_l", "arm_lower_l",
    "thigh_l", "calf_l",
    "pelvis",
    "thigh_r", "calf_r",
    "torso", "head",
    "arm_upper_r", "shoulder_r", "arm_lower_r"
]

os.makedirs("Content/art/characters/boss/spine/clean_parts", exist_ok=True)
extracted = {}

for name in DRAW_ORDER:
    poly = PARTS_POLYS[name]
    mask = Image.new("L", clean_master.size, 0)
    ImageDraw.Draw(mask).polygon(poly, fill=255)
    
    part = Image.new("RGBA", clean_master.size, (0, 0, 0, 0))
    part.paste(clean_master, (0, 0), mask)
    
    bbox = part.getbbox()
    if bbox:
        cropped = part.crop(bbox)
        w_scaled = max(1, cropped.width // 2)
        h_scaled = max(1, cropped.height // 2)
        scaled = cropped.resize((w_scaled, h_scaled), Image.Resampling.LANCZOS)
        scaled.save(f"Content/art/characters/boss/spine/clean_parts/{name}.png")
        extracted[name] = {"bbox": bbox, "scaled": scaled, "w": w_scaled, "h": h_scaled}

comp = Image.new("RGBA", (600, 600), (0, 0, 0, 0))
for name in DRAW_ORDER:
    p = extracted[name]
    orig_x0, orig_y0 = p["bbox"][0], p["bbox"][1]
    pos_x = 300 + (orig_x0 - 512) // 2
    pos_y = 520 + (orig_y0 - 950) // 2
    comp.paste(p["scaled"], (pos_x, pos_y), p["scaled"])

comp.save("Content/art/characters/boss/spine/clean_parts_recomposite.png")
print("Saved ultra-clean recomposite!")
