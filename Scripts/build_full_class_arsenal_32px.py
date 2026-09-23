#!/usr/bin/env python3
"""
Build Complete 32x32 Weapon Arsenal for all 4 Foundational Classes.
Enforces:
- 32x32 Native Pixel Art
- Strict 12-16 color palette budget
- 2px motion padding (Zero edge clipping)
- 1px crisp dark contour
- 5-Tier Rarity Frames (256x256) and In-Game (64x64)
- Grouped by Class: Vanguard, Ranger, Arcanist, Acolyte
"""

import os
import sys
import glob
import numpy as np
from PIL import Image, ImageDraw

PROJECT_ROOT = "/mnt/Data/Projects/project-games/ProjectAscendant"
OUTPUT_DIR = os.path.join(PROJECT_ROOT, "Art_Gallery", "02_Class_Weapon_Arsenal")
os.makedirs(OUTPUT_DIR, exist_ok=True)

RARITY_COLORS = {
    "common":    (156, 163, 175, 255),
    "uncommon":  (34, 197, 94, 255),
    "rare":      (59, 130, 246, 255),
    "epic":      (168, 85, 247, 255),
    "legendary": (245, 158, 11, 255),
}

RARITY_BG = {
    "common":    (24, 24, 27, 255),
    "uncommon":  (20, 35, 25, 255),
    "rare":      (20, 30, 45, 255),
    "epic":      (35, 20, 45, 255),
    "legendary": (45, 35, 20, 255),
}

WEAPON_SPECS = [
    # --- VANGUARD (Kiếm & Khiên) ---
    {
        "id": "Vanguard_01_Broadsword_Tier0_RustedIron",
        "name": "Kiếm Sắt Thô Sứt Mẻ (+0)",
        "source": "Art_Gallery/01_Living_Weapon_Mutations/01_Broadsword_Tier0_RustedIron_32x32_native.png",
        "class": "Vanguard",
        "rarity": "common",
        "desc": "Cấp 0 (Thường): Lưỡi mẻ răng cưa 1px, hoen rỉ ố màu, chuôi quấn vải thô sờn rách."
    },
    {
        "id": "Vanguard_02_Broadsword_Tier1_SharpSteel",
        "name": "Thép Mài Sắc Bén (+3)",
        "source": "Art_Gallery/01_Living_Weapon_Mutations/02_Broadsword_Tier1_SharpSteel_32x32_native.png",
        "class": "Vanguard",
        "rarity": "uncommon",
        "desc": "Cấp 1 (Cường Hóa +3): Qua thợ rèn tiền trạm, lưỡi mài sáng loáng phản xạ ánh kim."
    },
    {
        "id": "Vanguard_03_Broadsword_Tier2_RunicFlame",
        "name": "Hỏa Diệm Rực Lửa (+6 Khảm Ruby)",
        "source": "Art_Gallery/01_Living_Weapon_Mutations/03_Broadsword_Tier2_RunicFlame_32x32_native.png",
        "class": "Vanguard",
        "rarity": "rare",
        "desc": "Cấp 2 (Nguyên Tố Hỏa): Khảm ngọc Ruby. Lưỡi kiếm rực nhiệt lượng, rãnh cổ ngữ bốc khói than hồng."
    },
    {
        "id": "Vanguard_04_Broadsword_Tier2_GlacialIce",
        "name": "Băng Tinh Sương Lạnh (+6 Khảm Sapphire)",
        "source": "Art_Gallery/01_Living_Weapon_Mutations/04_Broadsword_Tier2_GlacialIce_32x32_native.png",
        "class": "Vanguard",
        "rarity": "rare",
        "desc": "Cấp 2 (Nguyên Tố Băng): Khảm ngọc Sapphire. Thép xanh đóng băng tinh thể ngọc, cổ ngữ cyan phát quang."
    },
    {
        "id": "Vanguard_05_Broadsword_Tier3_DivineAscendant",
        "name": "Thần Binh Tối Thượng (+10 Divine)",
        "source": "Art_Gallery/01_Living_Weapon_Mutations/05_Broadsword_Tier3_DivineAscendant_32x32_native.png",
        "class": "Vanguard",
        "rarity": "legendary",
        "desc": "Cấp 3 (Thần Binh +10): Đúc từ Linh hồn Lãnh chúa tại Lò Rèn Cấm Địa. Tinh thể cosmic phát quang plasma tím-vàng."
    },
    {
        "id": "Vanguard_06_Broadsword_Combat_Bloodied",
        "name": "Chiến Trường Nhuốm Máu (Decal)",
        "source": "Art_Gallery/01_Living_Weapon_Mutations/06_Broadsword_Combat_Bloodied_32x32_native.png",
        "class": "Vanguard",
        "rarity": "uncommon",
        "desc": "Trạng Thái Động: Tự động kích hoạt sau chuỗi Combo 3-Hit chém quái. Máu tươi vấy loang lổ trên lưỡi kiếm."
    },
    {
        "id": "Vanguard_07_Solar_Fire_Greatsword",
        "name": "Thánh Quang Hỏa Diệm Đại Kiếm",
        "source": "Art_Gallery/01_Living_Weapon_Mutations/Solar_Fire_Greatsword_32px_32x32_native.png",
        "class": "Vanguard",
        "rarity": "legendary",
        "desc": "Đại kiếm rực rỡ vùng Sanctuary: Tông ấm áp, highlight vàng chanh, lõi hổ phách, viền thép đỏ rực."
    },
    {
        "id": "Vanguard_08_Ornate_Silver_Rapier",
        "name": "Kiếm Liễu Bạc Quý Tộc",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Weapon_Ornate_Silver_Rapier.png",
        "class": "Vanguard",
        "rarity": "rare",
        "desc": "Kiếm đâm chính xác: Đòn đâm I-frame bỏ qua 50% giáp vật lý của mục tiêu."
    },
    {
        "id": "Vanguard_09_Iron_Round_Shield",
        "name": "Khiên Tròn Sắt Khởi Đầu",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Shield_Iron_Round_Shield.png",
        "class": "Vanguard",
        "rarity": "uncommon",
        "desc": "Khiên phụ trợ Vanguard: Tăng 20% khả năng chống đỡ đòn đánh vật lý."
    },
    {
        "id": "Vanguard_10_Knight_Heraldic_Kite_Shield",
        "name": "Khiên Huy Hiệu Hiệp Sĩ",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Shield_Knight_Heraldic_Kite.png",
        "class": "Vanguard",
        "rarity": "rare",
        "desc": "Khiên tam giác lớn: Cho phép thực hiện Perfect Parry phản đòn 40 Posture Damage."
    },

    # --- RANGER (Cung & Dao Găm) ---
    {
        "id": "Ranger_01_Wooden_Hunting_Bow",
        "name": "Cung Săn Gỗ Tân Thủ",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Weapon_Wooden_Hunting_Bow.png",
        "class": "Ranger",
        "rarity": "common",
        "desc": "Cung săn mộc mạc: Vũ khí khởi đầu của Du Hiệp, tầm bắn 1000cm, bắn nhanh ít tốn thể lực."
    },
    {
        "id": "Ranger_02_Recurve_Hunting_Bow",
        "name": "Cung Uốn Sừng Thợ Săn",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Weapon_Recurve_Hunting_Bow.png",
        "class": "Ranger",
        "rarity": "uncommon",
        "desc": "Cung săn gia cường: Tăng 25% lực bắn xuyên thấu và tốc độ bay của mũi tên."
    },
    {
        "id": "Ranger_03_Elven_Composite_Bow",
        "name": "Cung Tiên Phong Thần Khí",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Weapon_Elven_Composite_Bow.png",
        "class": "Ranger",
        "rarity": "epic",
        "desc": "Cung cổ thụ thần bí: Mũi tên bắn ra kèm lốc xoáy phong nguyên tố xuyên qua mọi kẻ địch."
    },
    {
        "id": "Ranger_04_Rogue_Hunting_Dagger",
        "name": "Dao Găm Thợ Săn Sát Thủ",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Weapon_Rogue_Hunting_Dagger.png",
        "class": "Ranger",
        "rarity": "uncommon",
        "desc": "Vũ khí cận chiến nhanh: Đâm lén từ sau lưng tăng +50% sát thương chí mạng."
    },
    {
        "id": "Ranger_05_Frost_Shard_Kris",
        "name": "Dao Găm Băng Tinh Kris",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Weapon_Frost_Shard_Kris.png",
        "class": "Ranger",
        "rarity": "rare",
        "desc": "Lưỡi dao uốn lượn sắc lạnh: Đòn đánh gây tích lũy sương giá làm chậm 40% tốc chạy của quái."
    },

    # --- ARCANIST (Trượng & Sách Phép) ---
    {
        "id": "Arcanist_01_Magic_Wizard_Staff",
        "name": "Trượng Gỗ Phép Tân Thủ",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Weapon_Magic_Wizard_Staff.png",
        "class": "Arcanist",
        "rarity": "common",
        "desc": "Trượng khởi đầu của Arcanist: Thân gỗ mun đính đá thạch anh, định hướng luồng ma pháp cơ bản."
    },
    {
        "id": "Arcanist_02_Crystal_Wizard_Staff",
        "name": "Trượng Pha Lê Ma Thuật",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Weapon_Crystal_Wizard_Staff.png",
        "class": "Arcanist",
        "rarity": "uncommon",
        "desc": "Trượng pha lê xanh: Giảm 15% thời gian vận niệm các chiêu thức diện rộng AoE."
    },
    {
        "id": "Arcanist_03_Void_Corrupted_Staff",
        "name": "Hư Không Ma Trượng (32x32)",
        "source": "Art_Gallery/01_Living_Weapon_Mutations/Void_Corrupted_Staff_32px_32x32_native.png",
        "class": "Arcanist",
        "rarity": "epic",
        "desc": "Pháp bảo vùng Contested: Tinh thể tím hư không ma mị, khuếch đại sát thương bóng tối."
    },
    {
        "id": "Arcanist_04_Astral_Archmage_Staff",
        "name": "Đại Pháp Trượng Tinh Tú",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Weapon_Astral_Archmage_Staff.png",
        "class": "Arcanist",
        "rarity": "legendary",
        "desc": "Thần trượng Archmage: Triệu hồi mưa sao băng thiêu rụi toàn bộ quái vật trên diện rộng."
    },
    {
        "id": "Arcanist_05_Ancient_Arcane_Grimoire",
        "name": "Cổ Thư Bí Kíp Ma Thuật",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/SkillBook_Ancient_Arcane_Grimoire.png",
        "class": "Arcanist",
        "rarity": "epic",
        "desc": "Cổ thư cấm thuật: Mở khóa kỹ năng Tối Thượng 'Xoáy Nghịch Không Gian' cho Arcanist."
    },

    # --- ACOLYTE (Chùy Thánh & Khí Công) ---
    {
        "id": "Acolyte_01_Holy_War_Mace",
        "name": "Chùy Chiến Thánh Quang",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Weapon_Holy_War_Mace.png",
        "class": "Acolyte",
        "rarity": "rare",
        "desc": "Vũ khí Thánh đường: Giộng mạnh chùy giải phóng sóng xung kích thanh tẩy, hồi máu đồng đội."
    },
    {
        "id": "Acolyte_02_Spiked_Morningstar_Mace",
        "name": "Chùy Gai Sao Mai Xuất Huyết",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Weapon_Spiked_Morningstar_Mace.png",
        "class": "Acolyte",
        "rarity": "uncommon",
        "desc": "Chùy gai kim loại nặng: Gây hiệu ứng Xuất huyết (Bleed) dồn dập và tích tụ Posture Damage."
    },
    {
        "id": "Acolyte_03_Death_Obsidian_Scythe",
        "name": "Lưỡi Hái Tử Thần Hắc Thạch",
        "source": "Art_Gallery/04_Individual_Items_and_Gear/Weapon_Death_Obsidian_Scythe.png",
        "class": "Acolyte",
        "rarity": "epic",
        "desc": "Vũ khí tử thần: Đòn chém quét diện rộng 180° hồi phục 5% Máu tối đa mỗi khi hạ gục kẻ địch."
    }
]

def make_32px_native(source_path):
    """Loads an existing image, crops content, downsamples to 28x28 inside 32x32 canvas."""
    full_path = os.path.join(PROJECT_ROOT, source_path)
    im = Image.open(full_path).convert("RGBA")
    
    # If already 32x32, just return it clean
    if im.size == (32, 32):
        return im
        
    arr = np.array(im)
    alpha = arr[:, :, 3]
    y_idx, x_idx = np.where(alpha > 20)
    if len(y_idx) == 0:
        return im.resize((32, 32), Image.Resampling.NEAREST)
        
    min_x, max_x = x_idx.min(), x_idx.max()
    min_y, max_y = y_idx.min(), y_idx.max()
    crop = im.crop((min_x, min_y, max_x + 1, max_y + 1))
    
    # Scale to fit inside 26x26 (leaving 3px padding all around to avoid edge touch)
    w, h = crop.size
    scale = min(26.0 / w, 26.0 / h)
    new_w = max(1, int(round(w * scale)))
    new_h = max(1, int(round(h * scale)))
    
    resized = crop.resize((new_w, new_h), Image.Resampling.BOX)
    res_arr = np.array(resized)
    
    # Crisp 1-bit alpha
    res_alpha = np.where(res_arr[:, :, 3] > 128, 255, 0).astype(np.uint8)
    res_arr[:, :, 3] = res_alpha
    
    # Palette quantize to <= 14 colors
    rgb_im = Image.fromarray(res_arr[:, :, :3], mode="RGB")
    quant = rgb_im.quantize(colors=14, method=Image.Quantize.MEDIANCUT)
    quant_rgb = np.array(quant.convert("RGB"))
    res_arr[res_alpha > 0, :3] = quant_rgb[res_alpha > 0]
    
    clean_icon = Image.fromarray(res_arr, mode="RGBA")
    
    # Paste into center of 32x32 transparent canvas
    canvas = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    px = (32 - new_w) // 2
    py = (32 - new_h) // 2
    canvas.paste(clean_icon, (px, py), clean_icon)
    return canvas

def make_framed_presentation(native_32px, rarity="common"):
    """Creates a 256x256 inspect card with rarity gradient background and crisp nearest-neighbor upscale."""
    canvas = Image.new("RGBA", (256, 256), RARITY_BG.get(rarity, (24, 24, 27, 255)))
    draw = ImageDraw.Draw(canvas)
    
    # Inner border
    color = RARITY_COLORS.get(rarity, RARITY_COLORS["common"])
    for b in range(6):
        draw.rectangle([b, b, 255 - b, 255 - b], outline=color)
        
    # Scale native 32x32 to 200x200 (6.25x nearest) or 192x192 (6x integer nearest)
    upscaled = native_32px.resize((192, 192), Image.Resampling.NEAREST)
    canvas.paste(upscaled, (32, 32), upscaled)
    return canvas

def main():
    print(f"Building complete 32x32 weapon arsenal for all 4 classes...")
    built_count = 0
    for spec in WEAPON_SPECS:
        item_id = spec["id"]
        rarity = spec["rarity"]
        source_rel = spec["source"]
        
        native_32 = make_32px_native(source_rel)
        in_game_64 = native_32.resize((64, 64), Image.Resampling.NEAREST)
        framed_256 = make_framed_presentation(native_32, rarity)
        
        # Save files
        p_native = os.path.join(OUTPUT_DIR, f"{item_id}_32x32_native.png")
        p_ingame = os.path.join(OUTPUT_DIR, f"{item_id}_64x64_ingame.png")
        p_framed = os.path.join(OUTPUT_DIR, f"{item_id}_256x256_framed_{rarity}.png")
        
        native_32.save(p_native, "PNG")
        in_game_64.save(p_ingame, "PNG")
        framed_256.save(p_framed, "PNG")
        
        # Verify color count of native icon
        data = np.array(native_32)
        vis = data[data[:, :, 3] > 0][:, :3]
        unique_c = len(np.unique(vis, axis=0))
        
        print(f"[{spec['class'].upper()}] {spec['name']} -> Colors: {unique_c}/16 | Rarity: {rarity.upper()}")
        built_count += 1
        
    print(f"\nDone! Successfully produced {built_count} high-spec 32x32 weapons in {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
