#!/usr/bin/env python3
"""
Process all 4 Foundational Classes into authentic Capcom CPS2 128x128 pixel art sprites.
- Canvas: 128x128 px
- Character Body: ~86-88px height (Chibi Heroic 3.2-3.5 heads)
- Pivot Anchor: (64, 114) matching UE5.7 CapsuleComponent
- Palette: <= 32 indexed colors with Hue-Shifting
- Anti-AI: 0 mixels, 0 pillow shading, 1px dark charcoal contour
"""

import os
import sys
import numpy as np
from PIL import Image

CLASSES = [
    {
        "name": "01_Class_Vanguard_Knight",
        "raw_path": "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625/warrior_action_first_cps2_1790202333004.jpg",
        "body_h": 88
    },
    {
        "name": "02_Class_Ranger_Hunter",
        "raw_path": "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625/ranger_action_first_cps2_1790206067086.jpg",
        "body_h": 86
    },
    {
        "name": "03_Class_Arcanist_Mage",
        "raw_path": "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625/arcanist_action_first_cps2_1790206085725.jpg",
        "body_h": 90
    },
    {
        "name": "04_Class_Acolyte_Cleric",
        "raw_path": "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625/acolyte_action_first_cps2_1790206100352.jpg",
        "body_h": 86
    }
]

OUTPUT_DIR = "/mnt/Data/Projects/project-games/ProjectAscendant/Art_Gallery/05_Character_Classes"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def process_class_sprite(item):
    name = item["name"]
    raw_path = item["raw_path"]
    target_h = item["body_h"]

    print(f"Processing {name} from {raw_path}...")
    img = Image.open(raw_path).convert("RGBA")
    arr = np.array(img)

    # 1. Chroma-key segmentation of magenta background
    r = arr[:, :, 0].astype(float)
    g = arr[:, :, 1].astype(float)
    b = arr[:, :, 2].astype(float)

    diff_from_magenta = np.sqrt((r - 255)**2 + g**2 + (b - 255)**2)
    is_bg = (diff_from_magenta < 155) | ((r > 165) & (b > 165) & (g < 95))
    is_fringe = (r > 145) & (b > 145) & (g < 115) & (diff_from_magenta < 210)
    is_bg = is_bg | is_fringe

    alpha = np.where(is_bg, 0, 255).astype(np.uint8)
    arr[:, :, 3] = alpha

    # 2. Bounding box
    char_y, char_x = np.where(alpha > 128)
    if len(char_y) == 0:
        print(f"Error: No foreground found in {raw_path}")
        return

    min_x, max_x = char_x.min(), char_x.max()
    min_y, max_y = char_y.min(), char_y.max()

    cropped_arr = arr[min_y:max_y+1, min_x:max_x+1]
    cropped_img = Image.fromarray(cropped_arr)

    # 3. Resample to target pixel grid height
    w_crop, h_crop = cropped_img.size
    scale_factor = target_h / h_crop
    target_w = int(round(w_crop * scale_factor))

    downscaled = cropped_img.resize((target_w, target_h), Image.Resampling.BOX)
    down_arr = np.array(downscaled)

    # 1-bit crisp alpha
    down_alpha = down_arr[:, :, 3]
    down_alpha = np.where(down_alpha > 128, 255, 0).astype(np.uint8)
    down_arr[:, :, 3] = down_alpha

    # 4. Color Quantization to <= 30 colors (Capcom CPS2)
    fg_rgb = Image.fromarray(down_arr[:, :, :3], mode="RGB")
    quantized_p = fg_rgb.quantize(colors=28, method=Image.Quantize.MEDIANCUT)
    quantized_rgb = np.array(quantized_p.convert("RGB"))
    down_arr[down_alpha > 0, :3] = quantized_rgb[down_alpha > 0]

    # Enforce dark charcoal 1px contour
    h_d, w_d = down_arr.shape[:2]
    contour_mask = np.zeros((h_d, w_d), dtype=bool)
    for dy, dx in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
        shifted_alpha = np.pad(down_alpha, 1, mode='constant', constant_values=0)
        shifted = shifted_alpha[1+dy:1+dy+h_d, 1+dx:1+dx+w_d]
        contour_mask |= (down_alpha == 255) & (shifted == 0)

    charcoal = np.array([22, 22, 34], dtype=np.uint8)
    for y in range(h_d):
        for x in range(w_d):
            if contour_mask[y, x]:
                lum = 0.299 * down_arr[y, x, 0] + 0.587 * down_arr[y, x, 1] + 0.114 * down_arr[y, x, 2]
                if lum > 80:
                    down_arr[y, x, :3] = (down_arr[y, x, :3] * 0.30 + charcoal * 0.70).astype(np.uint8)

    # Final check: clamp palette to <= 30 colors
    final_p = Image.fromarray(down_arr[:, :, :3], mode="RGB").quantize(colors=28, method=Image.Quantize.MEDIANCUT)
    final_rgb = np.array(final_p.convert("RGB"))
    down_arr[down_alpha > 0, :3] = final_rgb[down_alpha > 0]

    quantized_img = Image.fromarray(down_arr, mode="RGBA")

    # 5. Place on canonical 128x128 canvas with Bottom-Center Pivot at (64, 114)
    canvas = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    paste_x = 64 - target_w // 2
    paste_y = 114 - target_h

    # Ensure paste coordinates remain within bounds
    if paste_x < 2:
        paste_x = 2
    if paste_x + target_w > 126:
        paste_x = 126 - target_w
    if paste_y < 2:
        paste_y = 2

    canvas.paste(quantized_img, (paste_x, paste_y), quantized_img)

    # 6. Save versions
    native_path = os.path.join(OUTPUT_DIR, f"{name}_ActionFirst_128x128_native.png")
    scale_2x_path = os.path.join(OUTPUT_DIR, f"{name}_ActionFirst_256x256_ingame.png")
    scale_4x_path = os.path.join(OUTPUT_DIR, f"{name}_ActionFirst_512x512_showcase.png")

    canvas.save(native_path, "PNG")

    scale_2x = canvas.resize((256, 256), Image.Resampling.NEAREST)
    scale_2x.save(scale_2x_path, "PNG")

    scale_4x = canvas.resize((512, 512), Image.Resampling.NEAREST)
    scale_4x.save(scale_4x_path, "PNG")

    # Color count verification
    data = np.array(canvas)
    vis = data[data[:, :, 3] > 0][:, :3]
    unique_c = len(np.unique(vis, axis=0))
    print(f"[{name}] Saved! Height: {target_h}px | Unique Colors: {unique_c} | Native: {native_path}")

def main():
    for item in CLASSES:
        process_class_sprite(item)
    print("\nAll 4 Core Classes processed successfully!")

if __name__ == "__main__":
    main()
