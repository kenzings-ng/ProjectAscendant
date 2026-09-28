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

# Generous overlapping polygons following stone contours with zero seams!
PARTS_POLYS = {
    # Head: jawline, crown, neck collar extending down to Y 365, X 380..680
    "head": [
        (380, 85), (660, 85), (680, 180), (670, 280), (630, 365), 
        (470, 365), (430, 355), (385, 260), (380, 150)
    ],
    # Torso: collar at Y 240, sides at X 270..720, bottom at Y 590
    "torso": [
        (280, 240), (660, 230), (715, 330), (700, 590), (340, 595), 
        (280, 430), (270, 330)
    ],
    # Pelvis: waist extending up to Y 510 (under torso), down to Y 710 (over thighs)
    "pelvis": [
        (320, 510), (700, 510), (690, 710), (490, 720), (320, 700)
    ],
    # Shoulder R: massive pauldron with vines, overlapping torso and head
    "shoulder_r": [
        (135, 140), (420, 140), (420, 440), (260, 440), (135, 320)
    ],
    # Arm Upper R: bicep under pauldron (Y 300..540, X 110..300)
    "arm_upper_r": [
        (110, 300), (300, 300), (300, 540), (110, 540)
    ],
    # Arm Lower R: forearm with rune & fist (Y 410..830, X 35..340)
    "arm_lower_r": [
        (35, 410), (340, 410), (340, 830), (35, 830)
    ],
    # Shoulder L: pauldron behind head (Y 70..300, X 560..800)
    "shoulder_l": [
        (560, 70), (800, 70), (800, 300), (560, 300)
    ],
    # Arm Upper L: behind torso/head (Y 90..320, X 680..900)
    "arm_upper_l": [
        (680, 90), (900, 90), (900, 320), (680, 320)
    ],
    # Arm Lower L: raised fist (Y 70..510, X 680..990)
    "arm_lower_l": [
        (680, 70), (990, 70), (990, 510), (680, 510)
    ],
    # Thigh R: under pelvis (Y 530..770, X 290..500)
    "thigh_r": [
        (290, 530), (500, 530), (500, 770), (290, 770)
    ],
    # Calf R: shin + foot + all toes (Y 660..965, X 250..500)
    "calf_r": [
        (250, 660), (500, 660), (500, 965), (250, 965)
    ],
    # Thigh L: under pelvis (Y 520..760, X 480..710)
    "thigh_l": [
        (480, 520), (710, 520), (710, 760), (480, 760)
    ],
    # Calf L: shin + foot + all toes (Y 650..860, X 490..760)
    "calf_l": [
        (490, 650), (760, 650), (760, 860), (490, 860)
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

extracted = {}
os.makedirs("Content/art/characters/boss/spine/clean_parts", exist_ok=True)

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
print("Updated clean_parts_recomposite.png with generous overlap!")
