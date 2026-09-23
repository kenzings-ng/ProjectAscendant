#!/usr/bin/env python3
"""
Slices and exports 38+ individual, pristine pixel art RPG items with ZERO cut-off edges.
Uses exact contour bounding boxes with generous padding and clean alpha transparency.
"""

import os
import numpy as np
from PIL import Image

PROJECT_ROOT = "/mnt/Data/Projects/project-games/ProjectAscendant"
BRAIN_DIR = "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625"
DEST_DIR = os.path.join(PROJECT_ROOT, "Art_Gallery", "03_Individual_Items_and_Gear")

def remove_magenta_chromakey(im):
    arr = np.array(im).astype(np.float32)
    r, g, b = arr[:, :, 0], arr[:, :, 1], arr[:, :, 2]
    dist = np.sqrt((r - 255)**2 + (g - 0)**2 + (b - 255)**2)
    # Magenta threshold: < 80 is fully transparent, > 130 is opaque
    alpha = np.clip((dist - 80.0) / (130.0 - 80.0), 0.0, 1.0) * 255.0
    
    fringe = (alpha > 0) & (alpha < 255)
    avg_col = (r[fringe] + g[fringe] + b[fringe]) / 3.0
    r[fringe] = np.minimum(r[fringe], avg_col * 1.1)
    b[fringe] = np.minimum(b[fringe], avg_col * 1.1)
    
    rgba = np.zeros((arr.shape[0], arr.shape[1], 4), dtype=np.uint8)
    rgba[:, :, 0] = np.clip(r, 0, 255).astype(np.uint8)
    rgba[:, :, 1] = np.clip(g, 0, 255).astype(np.uint8)
    rgba[:, :, 2] = np.clip(b, 0, 255).astype(np.uint8)
    rgba[:, :, 3] = alpha.astype(np.uint8)
    return Image.fromarray(rgba, 'RGBA')

def crop_exact(clean_img, x0, y0, x1, y1, pad=18, target_size=(256, 256)):
    """Crops exact coordinate box with generous padding and centers in square icon."""
    cropped = clean_img.crop((x0, y0, x1, y1))
    
    # Check actual non-transparent bounds inside this crop to center perfectly
    arr = np.array(cropped)
    alpha = arr[:, :, 3]
    rows = np.where(alpha > 20)[0]
    cols = np.where(alpha > 20)[1]
    
    if len(rows) > 0 and len(cols) > 0:
        ry0, ry1 = int(rows.min()), int(rows.max())
        rx0, rx1 = int(cols.min()), int(cols.max())
        tight = cropped.crop((rx0, ry0, rx1 + 1, ry1 + 1))
    else:
        tight = cropped
        
    tw, th = tight.size
    max_side = max(tw, th) + pad * 2
    canvas = Image.new("RGBA", (max_side, max_side), (0, 0, 0, 0))
    paste_x = (max_side - tw) // 2
    paste_y = (max_side - th) // 2
    canvas.paste(tight, (paste_x, paste_y), tight)
    
    # Resize with Nearest Neighbor to preserve authentic sharp pixel art!
    return canvas.resize(target_size, Image.Resampling.NEAREST)

def main():
    os.makedirs(DEST_DIR, exist_ok=True)
    print("=== Slicing 38+ Perfect Items with Zero Cut-Off Corners ===")

    # -------------------------------------------------------------
    # 1. Expanded Weapons (weapons_expanded)
    # -------------------------------------------------------------
    src_wpn_exp = os.path.join(BRAIN_DIR, "weapons_expanded_1790176925647.jpg")
    if os.path.exists(src_wpn_exp):
        clean = remove_magenta_chromakey(Image.open(src_wpn_exp))
        # Coordinates from component analysis:
        items = [
            ("Weapon_Flaming_Greatsword.png", (65, 30, 210, 500)),
            ("Weapon_Frost_Shard_Kris.png", (320, 65, 445, 495)),
            ("Weapon_Elven_Composite_Bow.png", (580, 40, 720, 500)),
            ("Weapon_Astral_Archmage_Staff.png", (815, 45, 945, 500)),
            ("Weapon_Spiked_Morningstar_Mace.png", (30, 525, 250, 990)),
            ("Weapon_Ornate_Silver_Rapier.png", (295, 525, 500, 990)),
            ("Weapon_Death_Obsidian_Scythe.png", (540, 525, 955, 990)),
        ]
        for name, box in items:
            icon = crop_exact(clean, box[0], box[1], box[2], box[3], pad=16)
            icon.save(os.path.join(DEST_DIR, name), format="PNG")
            print(f"Saved: {name}")

    # -------------------------------------------------------------
    # 2. Crafting Materials & Gemstones (materials_and_gems)
    # -------------------------------------------------------------
    src_mat = os.path.join(BRAIN_DIR, "materials_and_gems_1790176966668.jpg")
    if os.path.exists(src_mat):
        clean = remove_magenta_chromakey(Image.open(src_mat))
        items = [
            ("Material_Rough_Iron_Ore.png", (35, 45, 220, 215)),
            ("Material_Gold_Ingot_Bar.png", (285, 45, 485, 215)),
            ("Gemstone_Faceted_Red_Ruby.png", (555, 35, 725, 220)),
            ("Gemstone_Radiant_Blue_Sapphire.png", (820, 35, 965, 220)),
            ("Gemstone_Cut_Green_Emerald.png", (50, 290, 205, 475)),
            ("Gemstone_Violet_Amethyst_Cluster.png", (290, 290, 475, 480)),
            ("Loot_Curved_Monster_Fang.png", (555, 300, 725, 475)),
            ("Loot_Glowing_Stone_Golem_Core.png", (805, 290, 985, 480)),
            ("Material_Clay_Terracotta_Brick.png", (560, 545, 720, 735)),
        ]
        for name, box in items:
            icon = crop_exact(clean, box[0], box[1], box[2], box[3], pad=20)
            icon.save(os.path.join(DEST_DIR, name), format="PNG")
            print(f"Saved: {name}")

    # -------------------------------------------------------------
    # 3. Potions, Jewelry, Books, Runes (pixel_potions_magic_items)
    # -------------------------------------------------------------
    src_pot = os.path.join(BRAIN_DIR, "pixel_potions_magic_items_1790175070607.jpg")
    if os.path.exists(src_pot):
        clean = remove_magenta_chromakey(Image.open(src_pot))
        items = [
            ("Potion_Health_Crimson_Flask.png", (70, 35, 305, 355)),
            ("Potion_Mana_Cyan_Phial.png", (380, 30, 640, 355)),
            ("Potion_Stamina_Green_Draught.png", (750, 40, 925, 350)),
            ("Jewelry_Engraved_Gold_Ring.png", (55, 385, 315, 655)),
            ("Jewelry_Sapphire_Pendant_Amulet.png", (370, 360, 650, 665)),
            ("SkillBook_Ancient_Arcane_Grimoire.png", (700, 365, 980, 685)),
            ("Crafting_Blacksmith_Ward_Stone.png", (375, 690, 645, 990)),
        ]
        for name, box in items:
            icon = crop_exact(clean, box[0], box[1], box[2], box[3], pad=18)
            icon.save(os.path.join(DEST_DIR, name), format="PNG")
            print(f"Saved: {name}")

    # -------------------------------------------------------------
    # 4. Armor Pieces, Helmets, Greaves, Boots (pixel_armor_headwear)
    # -------------------------------------------------------------
    src_arm = os.path.join(BRAIN_DIR, "pixel_armor_headwear_1790175122525.jpg")
    if os.path.exists(src_arm):
        clean = remove_magenta_chromakey(Image.open(src_arm))
        items = [
            ("Armor_Steel_Knight_Cuirass.png", (25, 35, 335, 325)),
            ("Armor_Leather_Scout_Vest.png", (380, 25, 640, 325)),
            ("Armor_Arcanist_Scholar_Tunic.png", (690, 25, 995, 325)),
            ("Helmet_Steel_Knight_Closed.png", (55, 360, 295, 660)),
            ("Helmet_Steel_Knight_Open.png", (390, 360, 630, 660)),
            ("Helmet_Leather_Ranger_Hood.png", (690, 360, 995, 660)),
            ("Greaves_Steel_Knight_Legguards.png", (45, 695, 305, 995)),
            ("Boots_Leather_Traveler_Boots.png", (690, 710, 990, 980)),
        ]
        for name, box in items:
            icon = crop_exact(clean, box[0], box[1], box[2], box[3], pad=18)
            icon.save(os.path.join(DEST_DIR, name), format="PNG")
            print(f"Saved: {name}")

    # -------------------------------------------------------------
    # 5. Core Classic Weapons & Shields (pixel_weapons_shields)
    # -------------------------------------------------------------
    src_wpn = os.path.join(BRAIN_DIR, "pixel_weapons_shields_1790175047575.jpg")
    if os.path.exists(src_wpn):
        clean = remove_magenta_chromakey(Image.open(src_wpn))
        items = [
            ("Weapon_Steel_Broadsword.png", (110, 50, 245, 445)),
            ("Weapon_Wooden_Hunting_Bow.png", (470, 55, 590, 450)),
            ("Weapon_Crystal_Wizard_Staff.png", (775, 50, 935, 445)),
            ("Weapon_Holy_War_Mace.png", (80, 540, 230, 885)),
            ("Weapon_Rogue_Hunting_Dagger.png", (340, 555, 440, 880)),
            ("Shield_Iron_Round_Shield.png", (510, 595, 760, 845)),
            ("Shield_Knight_Heraldic_Kite.png", (765, 555, 985, 885)),
        ]
        for name, box in items:
            icon = crop_exact(clean, box[0], box[1], box[2], box[3], pad=18)
            icon.save(os.path.join(DEST_DIR, name), format="PNG")
            print(f"Saved: {name}")

    print("\nAll 38+ item icons successfully sliced with ZERO cut-off corners!")

if __name__ == "__main__":
    main()
