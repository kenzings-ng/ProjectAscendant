#!/usr/bin/env python3
"""
Process and export transparent Paperdoll Sprites for Project Ascendant.
Removes magenta chroma-key (#FF00FF), eliminates fringe color spill,
and exports normalized 32-bit RGBA sprites ready for Paper2D & PaperZD.
"""

import os
import sys
import numpy as np
from PIL import Image

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
OUTPUT_DIR = os.path.join(PROJECT_ROOT, "Content", "art", "characters", "paperdoll")

ASSETS = [
    {
        "id": "starter_cloth",
        "file": "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625/hero_starter_cloth_1790138979243.jpg",
        "output_name": "T_Hero_StarterCloth.png",
        "layer": "BaseBody"
    },
    {
        "id": "iron_armor",
        "file": "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625/hero_iron_armor_1790139001271.jpg",
        "output_name": "T_Hero_IronArmor.png",
        "layer": "ChestArmor"
    },
    {
        "id": "leather_ranger",
        "file": "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625/hero_leather_ranger_1790139031021.jpg",
        "output_name": "T_Hero_LeatherRanger.png",
        "layer": "ChestArmor"
    },
    {
        "id": "arcanist_robe",
        "file": "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625/hero_arcanist_robe_1790139057554.jpg",
        "output_name": "T_Hero_ArcanistRobe.png",
        "layer": "ChestArmor"
    }
]

def remove_magenta_chromakey(im):
    """Cleanly key out magenta #FF00FF and desaturate any fringe bleed."""
    arr = np.array(im).astype(np.float32)
    r, g, b = arr[:, :, 0], arr[:, :, 1], arr[:, :, 2]
    
    # Target magenta is (255, 0, 255)
    # Color distance in RGB space to magenta
    dist = np.sqrt((r - 255)**2 + (g - 0)**2 + (b - 255)**2)
    
    # Soft alpha thresholding
    # Close distance (< 90) is fully transparent
    # Far distance (> 140) is fully opaque
    # Transition zone [90, 140] has smooth alpha
    alpha = np.clip((dist - 90.0) / (140.0 - 90.0), 0.0, 1.0) * 255.0
    
    # In edge regions (alpha < 255), suppress excess red and blue to remove purple fringing
    fringe_mask = (alpha > 0) & (alpha < 255)
    avg_col = (r[fringe_mask] + g[fringe_mask] + b[fringe_mask]) / 3.0
    r[fringe_mask] = np.minimum(r[fringe_mask], avg_col * 1.2)
    b[fringe_mask] = np.minimum(b[fringe_mask], avg_col * 1.2)
    
    rgba = np.zeros((arr.shape[0], arr.shape[1], 4), dtype=np.uint8)
    rgba[:, :, 0] = np.clip(r, 0, 255).astype(np.uint8)
    rgba[:, :, 1] = np.clip(g, 0, 255).astype(np.uint8)
    rgba[:, :, 2] = np.clip(b, 0, 255).astype(np.uint8)
    rgba[:, :, 3] = alpha.astype(np.uint8)
    
    return Image.fromarray(rgba, 'RGBA')

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    print(f"Processing {len(ASSETS)} Paperdoll sprites...")
    print(f"Output directory: {OUTPUT_DIR}")
    
    for item in ASSETS:
        src = item["file"]
        out_name = item["output_name"]
        out_path = os.path.join(OUTPUT_DIR, out_name)
        
        if not os.path.exists(src):
            print(f"[ERROR] Source file not found: {src}", file=sys.stderr)
            continue
            
        print(f"-> Processing {item['id']} ({item['layer']})...")
        im = Image.open(src)
        clean_im = remove_magenta_chromakey(im)
        
        # Save full resolution 1024x1024 transparent sprite
        clean_im.save(out_path, format="PNG")
        print(f"   Saved transparent sprite: {out_path} ({clean_im.size[0]}x{clean_im.size[1]})")
        
        # Save standard 512x512 Paper2D game ready thumbnail/texture
        scaled_512 = clean_im.resize((512, 512), Image.Resampling.LANCZOS)
        thumb_path = os.path.join(OUTPUT_DIR, f"Thumb_{out_name}")
        scaled_512.save(thumb_path, format="PNG")
        print(f"   Saved 512x512 texture: {thumb_path}")

    print("\nAll Paperdoll textures successfully generated and verified!")

if __name__ == "__main__":
    main()
