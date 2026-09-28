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

# DEEP OVERLAP POLYGONS
# Every child piece extends 40-80 pixels DEEPER into its parent socket!
DEEP_POLYS = {
    # Head: jawline, crown, neck extends down to Y 380 into torso collar
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
        (120, 240), (300, 240), (300, 530), (120, 530)
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
        (270, 965), (270, 785), (325, 785)
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

print("Deep overlap polygons defined.")
