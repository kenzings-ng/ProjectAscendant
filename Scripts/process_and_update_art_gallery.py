#!/usr/bin/env python3
"""
Comprehensive Art Processing & Gallery Generator for Project Ascendant.
- Generates female character paperdoll transparent sprites
- Slices individual item icons (weapons, shields, potions, jewelry, books, armor)
- Populates the Art_Gallery folder with categorized assets
- Builds the rich interactive HTML visualizer with item cards & lightbox
"""

import os
import sys
import shutil
import numpy as np
from PIL import Image

PROJECT_ROOT = "/mnt/Data/Projects/project-games/ProjectAscendant"
BRAIN_DIR = "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625"
GALLERY_DIR = os.path.join(PROJECT_ROOT, "Art_Gallery")

def remove_magenta_chromakey(im):
    """Remove magenta #FF00FF and suppress purple fringe."""
    arr = np.array(im).astype(np.float32)
    r, g, b = arr[:, :, 0], arr[:, :, 1], arr[:, :, 2]
    dist = np.sqrt((r - 255)**2 + (g - 0)**2 + (b - 255)**2)
    alpha = np.clip((dist - 85.0) / (135.0 - 85.0), 0.0, 1.0) * 255.0
    
    fringe_mask = (alpha > 0) & (alpha < 255)
    avg_col = (r[fringe_mask] + g[fringe_mask] + b[fringe_mask]) / 3.0
    r[fringe_mask] = np.minimum(r[fringe_mask], avg_col * 1.15)
    b[fringe_mask] = np.minimum(b[fringe_mask], avg_col * 1.15)
    
    rgba = np.zeros((arr.shape[0], arr.shape[1], 4), dtype=np.uint8)
    rgba[:, :, 0] = np.clip(r, 0, 255).astype(np.uint8)
    rgba[:, :, 1] = np.clip(g, 0, 255).astype(np.uint8)
    rgba[:, :, 2] = np.clip(b, 0, 255).astype(np.uint8)
    rgba[:, :, 3] = alpha.astype(np.uint8)
    return Image.fromarray(rgba, 'RGBA')

def crop_and_center_sprite(im, pad=20, target_size=(256, 256)):
    """Crop non-transparent content and center it inside a square canvas."""
    arr = np.array(im)
    if arr.shape[2] < 4:
        return im.resize(target_size, Image.Resampling.LANCZOS)
    alpha = arr[:, :, 3]
    rows = np.where(alpha > 15)[0]
    cols = np.where(alpha > 15)[1]
    if len(rows) == 0 or len(cols) == 0:
        return im.resize(target_size, Image.Resampling.LANCZOS)
    
    y0, y1 = int(rows.min()), int(rows.max())
    x0, x1 = int(cols.min()), int(cols.max())
    if x1 <= x0 or y1 <= y0:
        return im.resize(target_size, Image.Resampling.LANCZOS)
    
    cropped = im.crop((x0, y0, x1 + 1, y1 + 1))
    
    # Square canvas with padding
    w, h = cropped.size
    max_dim = max(w, h) + pad * 2
    canvas = Image.new("RGBA", (max_dim, max_dim), (0, 0, 0, 0))
    paste_x = (max_dim - w) // 2
    paste_y = (max_dim - h) // 2
    canvas.paste(cropped, (paste_x, paste_y), cropped)
    return canvas.resize(target_size, Image.Resampling.LANCZOS)

def process_female_characters():
    print("--- Processing Female Character Paperdoll ---")
    dest_dir = os.path.join(GALLERY_DIR, "02_Modular_Paperdoll_Female")
    os.makedirs(dest_dir, exist_ok=True)
    
    female_items = [
        ("female_starter_cloth_1790174718026.jpg", "01_Concept_Female_Starter_Cloth.jpg", "05_Sprite_Female_Starter_Cloth_Transparent.png"),
        ("female_iron_armor_1790174741541.jpg", "02_Concept_Female_Iron_Armor.jpg", "06_Sprite_Female_Iron_Armor_Transparent.png"),
        ("female_leather_ranger_1790174760920.jpg", "03_Concept_Female_Leather_Ranger.jpg", "07_Sprite_Female_Leather_Ranger_Transparent.png"),
        ("female_arcanist_robe_1790175022641.jpg", "04_Concept_Female_Arcanist_Robe.jpg", "08_Sprite_Female_Arcanist_Robe_Transparent.png"),
    ]
    
    for brain_file, concept_name, sprite_name in female_items:
        src = os.path.join(BRAIN_DIR, brain_file)
        if os.path.exists(src):
            # Save concept
            shutil.copy2(src, os.path.join(dest_dir, concept_name))
            # Save transparent sprite
            im = Image.open(src)
            clean_im = remove_magenta_chromakey(im)
            clean_im.save(os.path.join(dest_dir, sprite_name), format="PNG")
            # Also save into Content/art/characters/paperdoll
            game_content_dir = os.path.join(PROJECT_ROOT, "Content/art/characters/paperdoll")
            clean_im.save(os.path.join(game_content_dir, f"T_Hero_Female_{sprite_name[10:]}"), format="PNG")
            print(f"Processed: {concept_name} & {sprite_name}")

def slice_individual_items():
    print("--- Slicing Individual Items ---")
    dest_dir = os.path.join(GALLERY_DIR, "03_Individual_Items_and_Gear")
    os.makedirs(dest_dir, exist_ok=True)
    
    # 1. Weapons & Shields
    src_wpn = os.path.join(BRAIN_DIR, "pixel_weapons_shields_1790175047575.jpg")
    if os.path.exists(src_wpn):
        shutil.copy2(src_wpn, os.path.join(dest_dir, "00_Sheet_Weapons_and_Shields.jpg"))
        clean_wpn = remove_magenta_chromakey(Image.open(src_wpn))
        # Coordinates for weapons (approx 1024x1024 grid)
        # Row 1: Broadsword (x: 80-280, y: 30-440), Bow (x: 430-610, y: 30-440), Staff (x: 740-920, y: 30-440)
        # Row 2: War Mace (x: 60-260, y: 520-860), Dagger (x: 310-470, y: 520-860), Round Shield (x: 500-740, y: 560-840), Kite Shield (x: 750-960, y: 540-860)
        weapon_boxes = [
            ("Weapon_Steel_Broadsword.png", (80, 40, 270, 440)),
            ("Weapon_Recurve_Hunting_Bow.png", (430, 40, 600, 440)),
            ("Weapon_Magic_Wizard_Staff.png", (750, 40, 920, 440)),
            ("Weapon_Holy_War_Mace.png", (70, 520, 250, 860)),
            ("Weapon_Rogue_Hunting_Dagger.png", (320, 530, 460, 860)),
            ("Shield_Iron_Round_Shield.png", (500, 560, 740, 840)),
            ("Shield_Knight_Heraldic_Kite.png", (745, 540, 960, 860)),
        ]
        for name, box in weapon_boxes:
            crop = clean_wpn.crop(box)
            icon = crop_and_center_sprite(crop, pad=16, target_size=(256, 256))
            icon.save(os.path.join(dest_dir, name), format="PNG")
            print(f"Extracted Weapon/Shield icon: {name}")

    # 2. Potions, Jewelry, Books, Runes
    src_pot = os.path.join(BRAIN_DIR, "pixel_potions_magic_items_1790175070607.jpg")
    if os.path.exists(src_pot):
        shutil.copy2(src_pot, os.path.join(dest_dir, "00_Sheet_Potions_Jewelry_Grimoire.jpg"))
        clean_pot = remove_magenta_chromakey(Image.open(src_pot))
        # Row 1: Health Potion (x: 60-310, y: 30-340), Mana Elixir (x: 370-630, y: 30-340), Stamina Draught (x: 710-910, y: 30-340)
        # Row 2: Gold Ring (x: 50-320, y: 370-650), Sapphire Amulet (x: 360-640, y: 350-650), Skill Grimoire (x: 680-960, y: 360-660)
        # Row 3: Blacksmith Ward Stone (x: 360-640, y: 680-960)
        item_boxes = [
            ("Potion_Health_Crimson_Flask.png", (60, 30, 310, 340)),
            ("Potion_Mana_Cyan_Phial.png", (370, 30, 630, 340)),
            ("Potion_Stamina_Green_Draught.png", (710, 30, 910, 340)),
            ("Jewelry_Engraved_Gold_Ring.png", (50, 370, 320, 650)),
            ("Jewelry_Sapphire_Pendant_Amulet.png", (360, 350, 640, 650)),
            ("SkillBook_Ancient_Arcane_Grimoire.png", (680, 360, 960, 660)),
            ("Crafting_Blacksmith_Ward_Stone.png", (360, 680, 640, 960)),
        ]
        for name, box in item_boxes:
            crop = clean_pot.crop(box)
            icon = crop_and_center_sprite(crop, pad=16, target_size=(256, 256))
            icon.save(os.path.join(dest_dir, name), format="PNG")
            print(f"Extracted Magic Item icon: {name}")

    # 3. Armor Pieces & Helmets
    src_arm = os.path.join(BRAIN_DIR, "pixel_armor_headwear_1790175122525.jpg")
    if os.path.exists(src_arm):
        shutil.copy2(src_arm, os.path.join(dest_dir, "00_Sheet_Armor_and_Helmets.jpg"))
        clean_arm = remove_magenta_chromakey(Image.open(src_arm))
        # Row 1: Knight Cuirass (x: 20-330, y: 20-320), Leather Vest (x: 360-640, y: 20-320), Arcanist Robe (x: 670-980, y: 20-320)
        # Row 2: Knight Helm Visor (x: 50-290, y: 350-640), Open Visor Helm (x: 370-620, y: 350-640), Ranger Hood Cowl (x: 670-970, y: 350-640)
        # Row 3: Steel Greaves (x: 40-300, y: 680-970), Alternate Greaves (x: 360-640, y: 680-970), Traveler Leather Boots (x: 670-970, y: 690-960)
        armor_boxes = [
            ("Armor_Steel_Knight_Cuirass.png", (20, 20, 330, 320)),
            ("Armor_Leather_Scout_Vest.png", (360, 20, 640, 320)),
            ("Armor_Arcanist_Scholar_Tunic.png", (670, 20, 980, 320)),
            ("Helmet_Steel_Knight_Closed.png", (50, 350, 290, 640)),
            ("Helmet_Steel_Knight_Open.png", (370, 350, 620, 640)),
            ("Helmet_Leather_Ranger_Hood.png", (670, 350, 970, 640)),
            ("Greaves_Steel_Knight_Legguards.png", (40, 680, 300, 970)),
            ("Boots_Leather_Traveler_Boots.png", (670, 690, 970, 960)),
        ]
        for name, box in armor_boxes:
            crop = clean_arm.crop(box)
            icon = crop_and_center_sprite(crop, pad=16, target_size=(256, 256))
            icon.save(os.path.join(dest_dir, name), format="PNG")
            print(f"Extracted Armor/Helmet icon: {name}")

def update_classes_and_bosses():
    print("--- Updating Core Classes & Bosses ---")
    cls_dir = os.path.join(GALLERY_DIR, "04_Character_Classes_Redrawn")
    boss_dir = os.path.join(GALLERY_DIR, "05_Bosses_and_Monsters_Redrawn")
    os.makedirs(cls_dir, exist_ok=True)
    os.makedirs(boss_dir, exist_ok=True)
    
    # Core Classes Showcase
    src_cls = os.path.join(BRAIN_DIR, "core_classes_showcase_1790175151339.jpg")
    if os.path.exists(src_cls):
        shutil.copy2(src_cls, os.path.join(cls_dir, "01_Core_Classes_Handcrafted_Showcase.jpg"))
        clean_cls = remove_magenta_chromakey(Image.open(src_cls))
        clean_cls.save(os.path.join(cls_dir, "01_Core_Classes_Handcrafted_Transparent.png"), format="PNG")
        print("Updated Core Classes Showcase")
        
    # Stone Golem
    src_golem = os.path.join(BRAIN_DIR, "boss_stone_golem_pixel_1790175181226.jpg")
    if os.path.exists(src_golem):
        shutil.copy2(src_golem, os.path.join(boss_dir, "01_Boss_Stone_Golem_Handcrafted.jpg"))
        clean_golem = remove_magenta_chromakey(Image.open(src_golem))
        clean_golem.save(os.path.join(boss_dir, "01_Boss_Stone_Golem_Transparent.png"), format="PNG")
        print("Updated Stone Golem Boss Art")
        
    # Lich Necromancer
    src_lich = os.path.join(BRAIN_DIR, "boss_lich_pixel_1790175226360.jpg")
    if os.path.exists(src_lich):
        shutil.copy2(src_lich, os.path.join(boss_dir, "02_Boss_Lich_Necromancer_Handcrafted.jpg"))
        clean_lich = remove_magenta_chromakey(Image.open(src_lich))
        clean_lich.save(os.path.join(boss_dir, "02_Boss_Lich_Necromancer_Transparent.png"), format="PNG")
        print("Updated Lich Necromancer Boss Art")

def reorganize_folders():
    # Rename 01 to 01_Modular_Paperdoll_Male if not already
    old_p1 = os.path.join(GALLERY_DIR, "01_Modular_Paperdoll")
    new_p1 = os.path.join(GALLERY_DIR, "01_Modular_Paperdoll_Male")
    if os.path.exists(old_p1) and not os.path.exists(new_p1):
        os.rename(old_p1, new_p1)
        print("Renamed 01_Modular_Paperdoll -> 01_Modular_Paperdoll_Male")
        
    old_env = os.path.join(GALLERY_DIR, "04_Environment_and_World")
    new_env = os.path.join(GALLERY_DIR, "06_Environment_and_World")
    if os.path.exists(old_env) and not os.path.exists(new_env):
        os.rename(old_env, new_env)

def generate_master_gallery_html():
    print("--- Generating Master HTML Gallery ---")
    html_path = os.path.join(GALLERY_DIR, "index.html")
    
    categories = [
        {
            "id": "tab_paperdoll_female",
            "title": "👩 Nhân Vật Nữ (Female Paperdoll)",
            "folder": "02_Modular_Paperdoll_Female",
            "summary": "Bộ thiết kế nhân vật nữ hoàn chỉnh theo phong cách Pixel Art Stardew Valley & Octopath Traveler: Quần áo vải thô tân thủ, Giáp sắt Vanguard, Đồ da thợ săn và Pháp bào ma thuật.",
            "items": [
                ("01_Concept_Female_Starter_Cloth.jpg", "Nhân Vật Nữ Tân Thủ (Starter Cloth)", "Áo vải thô mộc mạc và quần nâu sờn rách, tóc tết nâu năng động.", "Concept Art", ["Starter", "Female", "Cloth"]),
                ("02_Concept_Female_Iron_Armor.jpg", "Nữ Chiến Binh Giáp Sắt (Vanguard Plate)", "Giáp ngực thép sáng bóng, cầu vai kiên cố, đại kiếm và khiên tròn.", "Concept Art", ["Vanguard", "Heavy Armor", "Sword/Shield"]),
                ("03_Concept_Female_Leather_Ranger.jpg", "Nữ Thợ Săn Áo Da (Ranger Scout)", "Áo da xanh rừng thuộc khâu chỉ nổi, bao tay da, cung săn uốn cong.", "Concept Art", ["Ranger", "Medium Armor", "Bow"]),
                ("04_Concept_Female_Arcanist_Robe.jpg", "Nữ Thuật Sĩ Pháp Bào (Arcanist Robe)", "Pháp bào nhung xanh thẫm thêu cổ ngữ vàng kim, trượng ngọc pha lê.", "Concept Art", ["Arcanist", "Cloth Robe", "Staff"]),
                ("05_Sprite_Female_Starter_Cloth_Transparent.png", "Sprite Nữ Trong Suốt: Đồ Vải Tân Thủ", "Sprite 32-bit RGBA tách nền sạch, căn chuẩn chân sẵn sàng ghép vào game.", "In-Game Sprite", ["Transparent", "Layer 0"]),
                ("06_Sprite_Female_Iron_Armor_Transparent.png", "Sprite Nữ Trong Suốt: Giáp Sắt", "Sprite giáp ngực thép và khiên tách nền trong suốt, khớp tỷ lệ 100%.", "In-Game Sprite", ["Transparent", "Layer 1"]),
                ("07_Sprite_Female_Leather_Ranger_Transparent.png", "Sprite Nữ Trong Suốt: Áo Da Thợ Săn", "Sprite áo da thợ săn tách nền không viền tím.", "In-Game Sprite", ["Transparent", "Layer 1"]),
                ("08_Sprite_Female_Arcanist_Robe_Transparent.png", "Sprite Nữ Trong Suốt: Pháp Bào", "Sprite pháp bào xanh thẫm phát sáng ngọc pha lê.", "In-Game Sprite", ["Transparent", "Layer 1"]),
            ]
        },
        {
            "id": "tab_paperdoll_male",
            "title": "👨 Nhân Vật Nam (Male Paperdoll)",
            "folder": "01_Modular_Paperdoll_Male",
            "summary": "Bộ thiết kế nhân vật nam cơ sở với trang phục vải thô sơ khai và các bộ giáp đại diện cho từng Class.",
            "items": [
                ("01_Concept_Starter_Cloth.jpg", "Nhân Vật Nam Tân Thủ (Starter Cloth)", "Áo sơ mi vải thô và quần nâu tân thủ.", "Concept Art", ["Starter", "Male", "Cloth"]),
                ("02_Concept_Vanguard_Iron_Armor.jpg", "Chiến Binh Giáp Sắt (Vanguard Iron)", "Giáp thép tấm đầy đủ, khiên tròn và thanh đại kiếm.", "Concept Art", ["Vanguard", "Heavy Armor"]),
                ("03_Concept_Ranger_Leather_Scout.jpg", "Du Mục Áo Da (Ranger Scout)", "Bộ giáp da thợ săn, cung săn và dao găm phụ.", "Concept Art", ["Ranger", "Medium Armor"]),
                ("04_Concept_Arcanist_Mystic_Robe.jpg", "Thuật Sĩ Pháp Bào (Arcanist Robe)", "Áo choàng nhung xanh thẫm và trượng ma thuật.", "Concept Art", ["Arcanist", "Cloth Robe"]),
                ("05_Sprite_Starter_Cloth_Transparent.png", "Sprite Nam Trong Suốt: Đồ Vải", "Tách nền Chroma-Key sẵn sàng cho hệ thống Modular Paperdoll C++.", "In-Game Sprite", ["Transparent", "Layer 0"]),
                ("06_Sprite_Iron_Armor_Transparent.png", "Sprite Nam Trong Suốt: Giáp Sắt", "Sprite giáp sắt thân căn chuẩn trọng tâm.", "In-Game Sprite", ["Transparent", "Layer 1"]),
                ("07_Sprite_Leather_Ranger_Transparent.png", "Sprite Nam Trong Suốt: Áo Da", "Sprite áo da thợ săn tách nền.", "In-Game Sprite", ["Transparent", "Layer 1"]),
                ("08_Sprite_Arcanist_Robe_Transparent.png", "Sprite Nam Trong Suốt: Pháp Bào", "Sprite pháp bào thuật sĩ trong suốt.", "In-Game Sprite", ["Transparent", "Layer 1"]),
            ]
        },
        {
            "id": "tab_items",
            "title": "⚔️ Vật Phẩm & Trang Bị Riêng Lẻ (Individual Items)",
            "folder": "03_Individual_Items_and_Gear",
            "summary": "Kho tài nguyên icon vật phẩm riêng lẻ vẽ tay theo phong cách Stardew Valley & Diablo 2: Vũ khí, Khiên, Bình dược phẩm, Trang sức ma pháp, Sách bí kíp và Bộ giáp.",
            "items": [
                ("Weapon_Steel_Broadsword.png", "Đại Kiếm Thép (Steel Broadsword)", "Vũ khí chính Vanguard: +15 Sát thương vật lý, đòn chém 3-Hit Combo sắc bén.", "Vũ Khí", ["Mainhand", "Vanguard", "Tier 2"]),
                ("Weapon_Recurve_Hunting_Bow.png", "Cung Săn Uốn Cong (Recurve Bow)", "Vũ khí viễn chiến Ranger: Tầm bắn xa 1800cm, bắn Snap Shot sau khi lướt né.", "Vũ Khí", ["Mainhand", "Ranger", "Tier 2"]),
                ("Weapon_Magic_Wizard_Staff.png", "Trượng Pha Lê Huyền Bí (Wizard Staff)", "Trượng phép Arcanist: Bắn đạn Arcane Orb xuyên thấu kẻ thù.", "Vũ Khí", ["Mainhand", "Arcanist", "Tier 3"]),
                ("Weapon_Holy_War_Mace.png", "Chùy Thánh Chiến (Holy War Mace)", "Vũ khí nghi lễ Acolyte: Tăng sát thương phá thế đứng Posture +30%.", "Vũ Khí", ["Mainhand", "Acolyte", "Tier 2"]),
                ("Weapon_Rogue_Hunting_Dagger.png", "Dao Găm Thợ Săn (Hunting Dagger)", "Vũ khí phụ Offhand: Kích hoạt đòn đâm chí mạng từ sau lưng (Backstab).", "Vũ Khí", ["Offhand", "Ranger", "Tier 2"]),
                ("Shield_Iron_Round_Shield.png", "Khiên Tròn Sắt (Iron Round Shield)", "Trang bị Offhand: Giảm 60% sát thương Posture khi đỡ đòn (Block).", "Khiên", ["Offhand", "Vanguard", "Shield"]),
                ("Shield_Knight_Heraldic_Kite.png", "Khiên Hiệp Sĩ Gia Huy (Kite Shield)", "Khiên lớn hình diều viền vàng gia tăng thời gian căn Perfect Parry.", "Khiên", ["Offhand", "Knight", "Rare"]),
                ("Potion_Health_Crimson_Flask.png", "Bình Máu Đỏ (Crimson Health Potion)", "Hồi phục 150 HP trong 0.8s. Giảm 30% tốc độ di chuyển trong lúc uống.", "Tiêu Hao", ["Consumable", "Quickbar 1", "Stack 20"]),
                ("Potion_Mana_Cyan_Phial.png", "Bình Mana Lam (Cyan Mana Phial)", "Hồi phục 120 điểm Mana năng lượng ma thuật.", "Tiêu Hao", ["Consumable", "Quickbar 2", "Stack 20"]),
                ("Potion_Stamina_Green_Draught.png", "Dược Thể Lực Lục (Stamina Draught)", "Lập tức xóa bỏ trạng thái Kiệt Sức và hồi phục 100% Stamina.", "Tiêu Hao", ["Consumable", "Quickbar 3", "Stack 20"]),
                ("Jewelry_Engraved_Gold_Ring.png", "Nhẫn Vàng Cổ Xưa (Engraved Gold Ring)", "Gia tăng tỷ lệ bạo kích Critical Strike +5% và giảm tiêu hao thể lực.", "Trang Sức", ["Ring Slot", "Tier 3", "Accessory"]),
                ("Jewelry_Sapphire_Pendant_Amulet.png", "Dây Chuyền Lam Ngọc (Sapphire Amulet)", "Tăng lượng Mana tối đa +50 và hồi phục 5 Mana mỗi giây.", "Trang Sức", ["Amulet Slot", "Tier 3", "Accessory"]),
                ("SkillBook_Ancient_Arcane_Grimoire.png", "Sách Bí Kíp: Grimoire Ma Thuật", "Học kỹ năng mới vào Action Deck. Cấm đọc trong lúc đang giao tranh.", "Sách Kỹ Năng", ["Skill Book", "Grimoire", "Non-stackable"]),
                ("Crafting_Blacksmith_Ward_Stone.png", "Đá Hộ Mệnh Thợ Rèn (Ward Stone)", "Bảo vệ vật phẩm không bị vỡ khi cường hóa lên +7 ~ +10 tại Lò Rèn.", "Nguyên Liệu", ["Crafting", "Protection", "Forge"]),
                ("Armor_Steel_Knight_Cuirass.png", "Giáp Ngực Thép Tấm (Steel Cuirass)", "Body Armor hạng nặng: +40 Giáp phòng ngự, chống đòn chém vật lý.", "Giáp Thân", ["Body Armor", "Heavy"]),
                ("Armor_Leather_Scout_Vest.png", "Áo Da Thợ Săn (Leather Scout Vest)", "Body Armor hạng trung: +25 Giáp, không làm giảm tốc độ di chuyển.", "Giáp Thân", ["Body Armor", "Medium"]),
                ("Armor_Arcanist_Scholar_Tunic.png", "Áo Thêu Học Giả (Arcanist Tunic)", "Body Armor vải phép: +18 Giáp, +45 Kháng nguyên tố hắc ám.", "Giáp Thân", ["Body Armor", "Light"]),
                ("Helmet_Steel_Knight_Closed.png", "Mũ Giáp Hiệp Sĩ (Steel Greathelm)", "Mũ sắt bảo vệ phần đầu: +20 Giáp, chống choáng Stun.", "Mũ Nón", ["Helmet", "Heavy"]),
                ("Helmet_Leather_Ranger_Hood.png", "Mũ Trùm Da Du Mục (Ranger Hood)", "Mũ trùm đầu thợ săn ngụy trang trong rừng rậm.", "Mũ Nón", ["Helmet", "Medium"]),
                ("Greaves_Steel_Knight_Legguards.png", "Xà Cạp Thép (Steel Greaves)", "Giáp bảo vệ cẳng chân và đầu gối trong giao tranh cận chiến.", "Trang Bị", ["Greaves", "Steel"]),
                ("Boots_Leather_Traveler_Boots.png", "Ủng Da Lữ Khách (Traveler Boots)", "Ủng da mềm giúp di chuyển êm ái trên địa hình bùn đá.", "Trang Bị", ["Boots", "Leather"]),
            ]
        },
        {
            "id": "tab_classes_redrawn",
            "title": "🛡️ 4 Class Phong Cách Tự Nhiên (Core Classes Redrawn)",
            "folder": "04_Character_Classes_Redrawn",
            "summary": "Tác phẩm vẽ lại 4 Class Chiến Binh, Thợ Săn, Thuật Sĩ và Tu Sĩ với nét vẽ Pixel sinh động, nét biểu cảm ấm áp tự nhiên giống Stardew Valley.",
            "items": [
                ("01_Core_Classes_Handcrafted_Showcase.jpg", "Bộ 4 Chức Nghiệp: Vanguard, Ranger, Arcanist, Acolyte", "Chiến Binh khiên sư tử, Nữ Thợ Săn mũ trùm xanh, Thuật Sĩ già thông thái và Nữ Tu Sĩ thuần khiết đứng trên gạch đá isometric.", "Tranh Tổng Quan", ["4 Classes", "Isometric", "Handcrafted"]),
                ("01_Core_Classes_Handcrafted_Transparent.png", "Bộ 4 Chức Nghiệp (Tách Nền Trong Suốt)", "Phiên bản trong suốt phục vụ làm banner, chọn nhân vật Character Select.", "Transparent", ["Banner", "UI Ready"]),
            ]
        },
        {
            "id": "tab_bosses_redrawn",
            "title": "👾 Boss & Quái Vật Vẽ Tay (Bosses Redrawn)",
            "folder": "05_Bosses_and_Monsters_Redrawn",
            "summary": "Thiết kế trùm thế giới Người Đá Cổ Đại và Pháp Sư Tử Linh với hình khối rõ ràng, vân rêu đá, ngọn lửa linh hồn ma quái.",
            "items": [
                ("01_Boss_Stone_Golem_Handcrafted.jpg", "Trùm Người Đá Cổ Đại (Ancient Stone Golem)", "Khối cự thạch phủ rêu phong, cổ ngữ ma thuật xanh lam rực sáng, dáng đứng dũng mãnh.", "Boss Concept", ["Stone Golem", "World Boss"]),
                ("01_Boss_Stone_Golem_Transparent.png", "Trùm Người Đá (Tách Nền Trong Suốt)", "Sprite trong suốt khổng lồ sẵn sàng đưa vào làm Boss Arena.", "In-Game Sprite", ["Transparent", "Boss"]),
                ("02_Boss_Lich_Necromancer_Handcrafted.jpg", "Pháp Sư Tử Linh (Lich Necromancer)", "Tử thi hoàng gia lơ lửng, hốc mắt rực lửa lục bảo, tay nắm linh hồn tím và trượng đầu lâu.", "Boss Concept", ["Lich", "Undead Boss"]),
                ("02_Boss_Lich_Necromancer_Transparent.png", "Pháp Sư Tử Linh (Tách Nền Trong Suốt)", "Sprite tách nền ma quái trong suốt cho vùng đầm lầy Ashen Keep.", "In-Game Sprite", ["Transparent", "Boss"]),
            ]
        },
        {
            "id": "tab_environment",
            "title": "🏰 Bản Đồ & Môi Trường Thế Giới",
            "folder": "06_Environment_and_World",
            "summary": "Bối cảnh thế giới mở 2.5D Isometric: Tiền Đồn Tân Thủ, Phế Tích Cổ Gothic và Khối Dựng 3D.",
            "items": [
                ("Complete_Map_Showcase.jpg", "Toàn Cảnh Bản Đồ Thế Giới (Complete Map)", "Toàn cảnh từ Tiền Đồn Tân Thủ (Verdant Bastion) qua Vùng Tro Tàn (Ashen Keep).", "Environment", ["World Map", "Isometric"]),
                ("Ruins_Concept.jpg", "Phế Tích Cổ Gothic (Ancient Ruins Concept)", "Khái niệm kiến trúc gothic đổ nát và ánh sáng sương mù thể tích u tối.", "Concept Art", ["Ruins", "Gothic"]),
                ("Ruins_Blockout_3D_Isometric.gif", "Mô Hình 3D Isometric Xoay Tròn (3D Blockout)", "Animation xoay 360 độ chứng minh sự hòa hợp giữa hình học 3D và Pixel Art 2D.", "Animated GIF", ["3D Depth", "Blockout"]),
            ]
        }
    ]
    
    tabs_html = ""
    sections_html = ""
    
    for idx, cat in enumerate(categories):
        active_class = "active" if idx == 0 else ""
        tab_id = cat["id"]
        
        tabs_html += f"""
        <button class="nav-tab {active_class}" onclick="switchTab('{tab_id}')">
            {cat['title']}
            <span class="count-badge">{len(cat['items'])}</span>
        </button>
        """
        
        cards_html = ""
        for item_data in cat["items"]:
            file_name, title, desc, badge, tags = item_data
            rel_path = f"{cat['folder']}/{file_name}"
            tags_html = "".join([f'<span class="tag">{t}</span>' for t in tags])
            is_icon = "Weapon_" in file_name or "Potion_" in file_name or "Jewelry_" in file_name or "Armor_" in file_name or "Shield_" in file_name or "Helmet_" in file_name
            media_class = "icon-media" if is_icon else ""
            
            cards_html += f"""
            <div class="art-card" onclick="openLightbox('{rel_path}', '{title}', '{desc}')">
                <div class="card-media {media_class}">
                    <img src="{rel_path}" alt="{title}" loading="lazy" />
                    <span class="card-badge">{badge}</span>
                </div>
                <div class="card-info">
                    <h3 class="card-title">{title}</h3>
                    <p class="card-desc">{desc}</p>
                    <div class="card-tags">{tags_html}</div>
                </div>
            </div>
            """
            
        display_style = "block" if idx == 0 else "none"
        sections_html += f"""
        <section id="{tab_id}" class="tab-content" style="display: {display_style};">
            <div class="section-header">
                <h2>{cat['title']}</h2>
                <p class="section-summary">{cat['summary']}</p>
            </div>
            <div class="art-grid">
                {cards_html}
            </div>
        </section>
        """

    full_html = f"""<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Project Ascendant — Trung Tâm Xem Xét Art & Trang Bị (Master Art Review)</title>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Cinzel:wght@600;700;800&family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <style>
        :root {{
            --bg-base: #0B0C0E;
            --bg-card: #13151A;
            --bg-card-hover: #1A1D24;
            --border-card: #262A34;
            --border-gold: #E6A122;
            --gold-glow: rgba(230, 161, 34, 0.25);
            --cyan-accent: #1ED5C6;
            --text-main: #E8ECEB;
            --text-muted: #8E95A5;
            --crimson: #9E1A1A;
        }}

        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}

        body {{
            background-color: var(--bg-base);
            color: var(--text-main);
            font-family: 'Inter', sans-serif;
            line-height: 1.5;
            padding-bottom: 80px;
        }}

        header {{
            background: linear-gradient(180deg, #161920 0%, #0B0C0E 100%);
            border-bottom: 1px solid var(--border-card);
            padding: 36px 40px;
            text-align: center;
            position: sticky;
            top: 0;
            z-index: 100;
            backdrop-filter: blur(12px);
        }}

        .brand-subtitle {{
            font-family: 'Cinzel', serif;
            color: var(--border-gold);
            font-size: 0.85rem;
            letter-spacing: 4px;
            text-transform: uppercase;
            margin-bottom: 6px;
        }}

        h1 {{
            font-family: 'Cinzel', serif;
            font-size: 2.2rem;
            font-weight: 800;
            letter-spacing: 1px;
            color: #FFFFFF;
            text-shadow: 0 0 20px var(--gold-glow);
        }}

        .lead-text {{
            color: var(--text-muted);
            font-size: 0.95rem;
            margin-top: 8px;
            max-width: 850px;
            margin-left: auto;
            margin-right: auto;
        }}

        .tabs-bar {{
            display: flex;
            justify-content: center;
            gap: 12px;
            padding: 20px;
            overflow-x: auto;
            background: #0E1014;
            border-bottom: 1px solid var(--border-card);
        }}

        .nav-tab {{
            background: #14171E;
            color: var(--text-muted);
            border: 1px solid var(--border-card);
            padding: 10px 18px;
            border-radius: 8px;
            cursor: pointer;
            font-size: 0.9rem;
            font-weight: 600;
            transition: all 0.2s ease;
            display: flex;
            align-items: center;
            gap: 8px;
            white-space: nowrap;
        }}

        .nav-tab:hover {{
            color: #FFFFFF;
            border-color: var(--border-gold);
            background: #1C202B;
        }}

        .nav-tab.active {{
            background: var(--border-gold);
            color: #0B0C0E;
            border-color: var(--border-gold);
            box-shadow: 0 0 15px var(--gold-glow);
        }}

        .count-badge {{
            background: rgba(0, 0, 0, 0.35);
            padding: 2px 7px;
            border-radius: 12px;
            font-size: 0.75rem;
        }}

        .container {{
            max-width: 1440px;
            margin: 0 auto;
            padding: 30px 24px;
        }}

        .section-header {{
            margin-bottom: 28px;
            border-left: 4px solid var(--border-gold);
            padding-left: 16px;
        }}

        .section-header h2 {{
            font-family: 'Cinzel', serif;
            font-size: 1.5rem;
            color: #FFF;
        }}

        .section-summary {{
            color: var(--text-muted);
            font-size: 0.9rem;
            margin-top: 4px;
        }}

        .art-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
            gap: 22px;
        }}

        .art-card {{
            background: var(--bg-card);
            border: 1px solid var(--border-card);
            border-radius: 12px;
            overflow: hidden;
            cursor: pointer;
            transition: all 0.25s cubic-bezier(0.16, 1, 0.3, 1);
            display: flex;
            flex-direction: column;
        }}

        .art-card:hover {{
            transform: translateY(-4px);
            border-color: var(--border-gold);
            box-shadow: 0 12px 28px rgba(0, 0, 0, 0.6), 0 0 15px var(--gold-glow);
            background: var(--bg-card-hover);
        }}

        .card-media {{
            position: relative;
            width: 100%;
            height: 260px;
            background: repeating-conic-gradient(#15171D 0% 25%, #1B1E26 0% 50%) 50% / 20px 20px;
            display: flex;
            align-items: center;
            justify-content: center;
            overflow: hidden;
            padding: 12px;
        }}

        .card-media.icon-media {{
            height: 200px;
            background: #111317;
        }}

        .card-media img {{
            max-width: 90%;
            max-height: 90%;
            object-fit: contain;
            image-rendering: pixelated;
            transition: transform 0.3s ease;
        }}

        .art-card:hover .card-media img {{
            transform: scale(1.08);
        }}

        .card-badge {{
            position: absolute;
            top: 10px;
            right: 10px;
            font-size: 0.68rem;
            font-weight: 700;
            padding: 3px 8px;
            border-radius: 6px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            background: rgba(19, 21, 26, 0.9);
            color: var(--border-gold);
            border: 1px solid var(--border-gold);
        }}

        .card-info {{
            padding: 16px;
            display: flex;
            flex-direction: column;
            flex-grow: 1;
        }}

        .card-title {{
            font-size: 1rem;
            font-weight: 700;
            color: #FFFFFF;
            margin-bottom: 6px;
        }}

        .card-desc {{
            color: var(--text-muted);
            font-size: 0.8rem;
            line-height: 1.4;
            margin-bottom: 12px;
            flex-grow: 1;
        }}

        .card-tags {{
            display: flex;
            flex-wrap: wrap;
            gap: 6px;
        }}

        .tag {{
            background: #1D212A;
            color: #A0A8B8;
            font-size: 0.7rem;
            padding: 2px 7px;
            border-radius: 4px;
            border: 1px solid #2C3240;
        }}

        /* Lightbox Modal */
        .lightbox {{
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(5, 6, 8, 0.95);
            z-index: 1000;
            justify-content: center;
            align-items: center;
            padding: 30px;
            backdrop-filter: blur(8px);
        }}

        .lightbox.active {{
            display: flex;
        }}

        .lightbox-box {{
            max-width: 900px;
            width: 100%;
            background: #14171E;
            border: 1px solid var(--border-gold);
            border-radius: 12px;
            overflow: hidden;
            box-shadow: 0 0 35px var(--gold-glow);
            display: flex;
            flex-direction: column;
            animation: modalIn 0.2s ease-out;
        }}

        @keyframes modalIn {{
            from {{ transform: scale(0.95); opacity: 0; }}
            to {{ transform: scale(1); opacity: 1; }}
        }}

        .lightbox-img-wrap {{
            background: repeating-conic-gradient(#15171D 0% 25%, #1B1E26 0% 50%) 50% / 24px 24px;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 24px;
            max-height: 65vh;
        }}

        .lightbox-img-wrap img {{
            max-width: 100%;
            max-height: 60vh;
            object-fit: contain;
            image-rendering: pixelated;
        }}

        .lightbox-details {{
            padding: 24px;
            border-top: 1px solid var(--border-card);
            position: relative;
        }}

        .lightbox-title {{
            font-family: 'Cinzel', serif;
            color: #FFF;
            font-size: 1.3rem;
            margin-bottom: 6px;
        }}

        .lightbox-desc {{
            color: var(--text-muted);
            font-size: 0.9rem;
        }}

        .lightbox-close {{
            position: absolute;
            top: 20px;
            right: 20px;
            background: #232733;
            border: none;
            color: #FFF;
            width: 36px;
            height: 36px;
            border-radius: 50%;
            cursor: pointer;
            font-size: 1.2rem;
            display: flex;
            align-items: center;
            justify-content: center;
            transition: all 0.2s;
        }}

        .lightbox-close:hover {{
            background: var(--crimson);
        }}
    </style>
</head>
<body>
    <header>
        <div class="brand-subtitle">Project Ascendant &bull; 2.5D Isometric HD-2D Dark Fantasy</div>
        <h1>Trung Tâm Trưng Bày & Kiểm Tra Đồ Họa Master Art</h1>
        <p class="lead-text">Tập hợp toàn bộ các tác phẩm Art vẽ tay theo phong cách Stardew Valley & Octopath Traveler: Nhân vật Nữ/Nam đa tầng, Kho vũ khí & vật phẩm riêng lẻ, 4 Class cơ bản, Trùm thế giới và Bối cảnh game.</p>
    </header>

    <div class="tabs-bar">
        {tabs_html}
    </div>

    <main class="container">
        {sections_html}
    </main>

    <div id="lightbox" class="lightbox" onclick="closeLightbox(event)">
        <div class="lightbox-box" onclick="event.stopPropagation()">
            <div class="lightbox-img-wrap">
                <img id="lightbox-img" src="" alt="Zoom Preview" />
            </div>
            <div class="lightbox-details">
                <button class="lightbox-close" onclick="closeLightbox(event)">&times;</button>
                <h3 id="lightbox-title" class="lightbox-title"></h3>
                <p id="lightbox-desc" class="lightbox-desc"></p>
            </div>
        </div>
    </div>

    <script>
        function switchTab(tabId) {{
            document.querySelectorAll('.tab-content').forEach(el => el.style.display = 'none');
            document.querySelectorAll('.nav-tab').forEach(el => el.classList.remove('active'));
            
            const target = document.getElementById(tabId);
            if (target) target.style.display = 'block';
            
            const activeBtn = Array.from(document.querySelectorAll('.nav-tab')).find(b => b.getAttribute('onclick').includes(tabId));
            if (activeBtn) activeBtn.classList.add('active');
        }}

        function openLightbox(src, title, desc) {{
            const lb = document.getElementById('lightbox');
            document.getElementById('lightbox-img').src = src;
            document.getElementById('lightbox-title').innerText = title;
            document.getElementById('lightbox-desc').innerText = desc;
            lb.classList.add('active');
        }}

        function closeLightbox(e) {{
            document.getElementById('lightbox').classList.remove('active');
        }}

        document.addEventListener('keydown', (e) => {{
            if (e.key === 'Escape') closeLightbox();
        }});
    </script>
</body>
</html>
"""
    with open(html_path, "w", encoding="utf-8") as f:
        f.write(full_html)
    print(f"Generated Master HTML Gallery: {html_path}")

def main():
    reorganize_folders()
    process_female_characters()
    slice_individual_items()
    update_classes_and_bosses()
    generate_master_gallery_html()
    print("\n=== ALL ART ASSETS SUCCESSFULLY PROCESSED AND INTEGRATED! ===")

if __name__ == "__main__":
    main()
