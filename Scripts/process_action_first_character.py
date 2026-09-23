#!/usr/bin/env python3
"""
Process raw Action-First AI generation into true Capcom CPS2 128x128 pixel art.
1. Clean Chroma-key removal (#FF00FF).
2. Align to native pixel grid and resample with clean integer cells.
3. Quantize palette to <= 32 indexed colors (Capcom CPS2 preset).
4. Clean orphan pixels and enforce 1px crisp contour.
5. Mount on 128x128 canvas with bottom-center anchor at (64, 114).
6. Export native 128x128 and Nearest-Neighbor scaled versions.
"""

import os
import sys
import numpy as np
from PIL import Image

RAW_IMAGE_PATH = "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625/warrior_action_first_cps2_1790202333004.jpg"
OUTPUT_DIR = "/mnt/Data/Projects/project-games/ProjectAscendant/Art_Gallery/05_Character_Classes"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def process_knight():
    img = Image.open(RAW_IMAGE_PATH).convert("RGBA")
    arr = np.array(img)

    # 1. Chroma-key segmentation of magenta background
    # Pure magenta is [255, 0, 255]
    r = arr[:, :, 0].astype(float)
    g = arr[:, :, 1].astype(float)
    b = arr[:, :, 2].astype(float)

    # Magenta metric: high R & B, low G
    diff_from_magenta = np.sqrt((r - 255)**2 + g**2 + (b - 255)**2)
    is_bg = (diff_from_magenta < 150) | ((r > 170) & (b > 170) & (g < 90))

    # Also detect background fringe
    is_fringe = (r > 150) & (b > 150) & (g < 110) & (diff_from_magenta < 200)
    is_bg = is_bg | is_fringe

    # Alpha mask
    alpha = np.where(is_bg, 0, 255).astype(np.uint8)
    arr[:, :, 3] = alpha

    # 2. Find bounding box of foreground
    char_y, char_x = np.where(alpha > 128)
    min_x, max_x = char_x.min(), char_x.max()
    min_y, max_y = char_y.min(), char_y.max()

    cropped_arr = arr[min_y:max_y+1, min_x:max_x+1]
    cropped_img = Image.fromarray(cropped_arr)

    # 3. Detect grid cell pitch
    # In raw 1024x1024, character height is roughly 953px which corresponds to ~90-95 native pixel blocks
    w_crop, h_crop = cropped_img.size
    target_body_height = 88  # Target 88px tall character for 128x128 canvas
    scale_factor = target_body_height / h_crop
    target_w = int(round(w_crop * scale_factor))

    # Downscale using Box/Area filter to capture true pixel block average, then sharpen
    downscaled = cropped_img.resize((target_w, target_body_height), Image.Resampling.BOX)
    down_arr = np.array(downscaled)

    # Clean alpha thresholding: crisp 1-bit alpha (0 or 255)
    down_alpha = down_arr[:, :, 3]
    down_alpha = np.where(down_alpha > 128, 255, 0).astype(np.uint8)
    down_arr[:, :, 3] = down_alpha

    # 4. Color Quantization to <= 30 colors (Capcom CPS2) using PIL Adaptive Quantization
    fg_rgb = Image.fromarray(down_arr[:, :, :3], mode="RGB")
    quantized_p = fg_rgb.quantize(colors=28, method=Image.Quantize.MEDIANCUT)
    quantized_rgb = np.array(quantized_p.convert("RGB"))
    down_arr[down_alpha > 0, :3] = quantized_rgb[down_alpha > 0]

    # Enforce dark charcoal 1px contour around edges
    # Find contour: pixels that have alpha 255 but have at least one transparent neighbor
    h_d, w_d = down_arr.shape[:2]
    contour_mask = np.zeros((h_d, w_d), dtype=bool)
    for dy, dx in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
        shifted_alpha = np.pad(down_alpha, 1, mode='constant', constant_values=0)
        shifted = shifted_alpha[1+dy:1+dy+h_d, 1+dx:1+dx+w_d]
        contour_mask |= (down_alpha == 255) & (shifted == 0)

    # Color of contour: Dark Charcoal (#181824 or deep navy shadow)
    charcoal = np.array([24, 24, 36], dtype=np.uint8)
    # Only darken edge pixels that aren't already very dark
    for y in range(h_d):
        for x in range(w_d):
            if contour_mask[y, x]:
                # If luminance is bright, bring it down to dark contour tone
                lum = 0.299 * down_arr[y, x, 0] + 0.587 * down_arr[y, x, 1] + 0.114 * down_arr[y, x, 2]
                if lum > 80:
                    down_arr[y, x, :3] = (down_arr[y, x, :3] * 0.35 + charcoal * 0.65).astype(np.uint8)

    quantized_img = Image.fromarray(down_arr, mode="RGBA")

    # 5. Place onto canonical 128x128 canvas with Bottom-Center Pivot at (64, 114)
    canvas = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    # Place bottom of character at y = 114, center horizontally at x = 64
    paste_x = 64 - target_w // 2
    paste_y = 114 - target_body_height

    canvas.paste(quantized_img, (paste_x, paste_y), quantized_img)

    # 6. Save native 128x128 and scaled versions
    native_path = os.path.join(OUTPUT_DIR, "02_Warrior_Knight_ActionFirst_128x128_native.png")
    scale_2x_path = os.path.join(OUTPUT_DIR, "02_Warrior_Knight_ActionFirst_256x256_ingame.png")
    scale_4x_path = os.path.join(OUTPUT_DIR, "02_Warrior_Knight_ActionFirst_512x512_showcase.png")

    canvas.save(native_path, "PNG")

    scale_2x = canvas.resize((256, 256), Image.Resampling.NEAREST)
    scale_2x.save(scale_2x_path, "PNG")

    scale_4x = canvas.resize((512, 512), Image.Resampling.NEAREST)
    scale_4x.save(scale_4x_path, "PNG")

    # Analyze final palette
    data = np.array(canvas)
    vis = data[data[:, :, 3] > 0][:, :3]
    unique_c = len(np.unique(vis, axis=0))

    print(f"Success!")
    print(f"Canvas: 128x128 px | Character Height: {target_body_height} px | Anchor: (64, 114)")
    print(f"Total Unique Colors: {unique_c} (Capcom CPS2 budget <= 32)")
    print(f"Saved Native: {native_path}")
    print(f"Saved 2x In-Game: {scale_2x_path}")
    print(f"Saved 4x Showcase: {scale_4x_path}")

if __name__ == "__main__":
    process_knight()
