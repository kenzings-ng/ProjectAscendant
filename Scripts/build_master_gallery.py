#!/usr/bin/env python3
"""
Master Art Gallery Builder (Clean, Unified, Anti-AI Handcrafted Architecture)
Builds Art_Gallery/index.html with 5 official categories:
1. 01_Core_Character_Classes: 4 Foundational Classes (Capcom CPS2 128x128 Action-First Pixel Art)
2. 02_Class_Weapon_Arsenal: 23 Weapons & Shields across 4 Classes (32x32 Native, 5-Tier Rarity Frames)
3. 03_Armor_Potions_and_Gems: Wearable Armors, Flasks, Cut Gemstones, Crafting Ores
4. 04_World_Bosses: Stone Golem & Lich Necromancer
5. 05_Environment_and_World: Isometric Ruins 3D GIF & Complete World Map
"""

import os
import sys

PROJECT_ROOT = "/mnt/Data/Projects/project-games/ProjectAscendant"
GALLERY_DIR = os.path.join(PROJECT_ROOT, "Art_Gallery")
OUTPUT_HTML = os.path.join(GALLERY_DIR, "index.html")

def generate_gallery():
    categories = [
        # --- TAB 1: 4 CORE CHARACTER CLASSES ---
        {
            "id": "tab_classes",
            "title": "🧙 4 Chức Nghiệp Nền Tảng (Core Classes)",
            "folder": "01_Core_Character_Classes",
            "badge": "Chuẩn Capcom CPS2 128x128",
            "summary": "Bộ 4 Chức Nghiệp Khởi Đầu (Vanguard, Ranger, Arcanist, Acolyte) được thiết kế theo đúng triết lý Action-First Posing và quy chuẩn Anti-AI của game-art-studio. Canvas chuẩn 128x128 px, chiều cao cơ thể 86-90px (Chibi Heroic 3.5 đầu), điểm neo chân (64, 114) gắn vừa khít CapsuleComponent UE5.7. Bảng màu khóa cứng <= 32 màu indexed palette với chuyển nhiệt độ màu Hue-shifting (Ánh nắng vàng chanh -> bóng đổ xanh tím Navy), 0 mixels, viền ngoài than chì 1px sắc lẹm.",
            "items": [
                # Vanguard
                ("01_Class_Vanguard_Knight_ActionFirst_512x512_showcase.png", "Chiến Binh (Vanguard Knight) — 4x Showcase", "Bậc thầy cận chiến & phản đòn: Tư thế tấn chữ S, gối chùng chịu 80% lực, lùi kiếm nén lực chờ vung đòn. Giáp thép tôi bóng bẩy với mảng phẳng nghỉ mắt 70%, rãnh bóng tím lạnh.", "CPS2 26 Màu", ["Vanguard", "Showcase 4x", "Action-First", "Handcrafted", "88px Body", "QA Passed"]),
                ("01_Class_Vanguard_Knight_ActionFirst_256x256_ingame.png", "Chiến Binh (Vanguard Knight) — 2x In-Game", "Khung hình hiển thị 2x Nearest-Neighbor cho game viewport. Lưới pixel 1:1 đanh gọn.", "In-Game 2x", ["Vanguard", "2x Scale", "Pixelated"]),
                ("01_Class_Vanguard_Knight_ActionFirst_128x128_native.png", "Chiến Binh (Vanguard Knight) — 128x128 Native", "File sprite gốc 128x128 pixel native, tâm chân (64, 114) chuẩn UE5 Paper2D.", "Native 128px", ["Vanguard", "Native 128x128", "Pivot 64,114"]),

                # Ranger
                ("02_Class_Ranger_Hunter_ActionFirst_512x512_showcase.png", "Du Hiệp (Ranger Hunter) — 4x Showcase", "Bóng ma tật phong & xạ kích: Tư thế hạ thấp trọng tâm, tấn trước dứt khoát, kéo căng toàn lực cung săn uốn sừng, mũi tên lắp sẵn sàng nhả tiễn. Áo da rừng xanh thuộc chỉ nổi.", "CPS2 25 Màu", ["Ranger", "Showcase 4x", "Action-First", "Handcrafted", "86px Body", "QA Passed"]),
                ("02_Class_Ranger_Hunter_ActionFirst_256x256_ingame.png", "Du Hiệp (Ranger Hunter) — 2x In-Game", "Sprite 2x cho gameplay. Dây cung và mũi tên mảnh 1px rõ ràng trên nền tối.", "In-Game 2x", ["Ranger", "2x Scale", "Pixelated"]),
                ("02_Class_Ranger_Hunter_ActionFirst_128x128_native.png", "Du Hiệp (Ranger Hunter) — 128x128 Native", "File gốc 128x128 px, tỷ lệ giải phẫu thon thả linh hoạt của xạ thủ.", "Native 128px", ["Ranger", "Native 128x128", "Pivot 64,114"]),

                # Arcanist
                ("03_Class_Arcanist_Mage_ActionFirst_512x512_showcase.png", "Thuật Sĩ (Arcanist Mage) — 4x Showcase", "Bậc thầy kiểm soát & bùng nổ ma pháp: Thân uốn cong chữ S, tay trái vươn mở xoáy nghịch không gian, tay phải giương cao trượng pha lê phóng luồng sét cyan-tím. Pháp bào lam uốn lượn đón gió ma lực.", "CPS2 23 Màu", ["Arcanist", "Showcase 4x", "Action-First", "Handcrafted", "90px Body", "QA Passed"]),
                ("03_Class_Arcanist_Mage_ActionFirst_256x256_ingame.png", "Thuật Sĩ (Arcanist Mage) — 2x In-Game", "Sprite 2x ingame. Tà áo choàng và hiệu ứng vòng xoáy ma thuật sắc sảo.", "In-Game 2x", ["Arcanist", "2x Scale", "Pixelated"]),
                ("03_Class_Arcanist_Mage_ActionFirst_128x128_native.png", "Thuật Sĩ (Arcanist Mage) — 128x128 Native", "File gốc 128x128 px, hiệu ứng hào quang được vẽ tay chuẩn từng điểm ảnh.", "Native 128px", ["Arcanist", "Native 128x128", "Pivot 64,114"]),

                # Acolyte
                ("04_Class_Acolyte_Cleric_ActionFirst_512x512_showcase.png", "Tu Sĩ (Acolyte Cleric) — 4x Showcase", "Điểm tựa sinh tồn & hộ thể kim cương: Tư thế hai tay cầm chắc chùy thánh hoàng kim giộng mạnh xuống đất, giải phóng vầng hào quang thánh quang bùng nổ. Giáp xích thép và áo choàng mặt trời kiên cố.", "CPS2 24 Màu", ["Acolyte", "Showcase 4x", "Action-First", "Handcrafted", "86px Body", "QA Passed"]),
                ("04_Class_Acolyte_Cleric_ActionFirst_256x256_ingame.png", "Tu Sĩ (Acolyte Cleric) — 2x In-Game", "Sprite 2x ingame. Hiệu ứng sóng chấn động tỏa hào quang vàng chói lọi.", "In-Game 2x", ["Acolyte", "2x Scale", "Pixelated"]),
                ("04_Class_Acolyte_Cleric_ActionFirst_128x128_native.png", "Tu Sĩ (Acolyte Cleric) — 128x128 Native", "File gốc 128x128 px, trọng tâm vững chãi của đấu sĩ hộ thể tuyến đầu.", "Native 128px", ["Acolyte", "Native 128x128", "Pivot 64,114"]),
            ]
        },

        # --- TAB 2: CLASS WEAPON ARSENAL ---
        {
            "id": "tab_weapons",
            "title": "⚔️ Kho Binh Khí Toàn Diện (Weapon Arsenal)",
            "folder": "02_Class_Weapon_Arsenal",
            "badge": "23 Mẫu 32x32 Đóng Khung 5-Tier",
            "summary": "Toàn bộ kho vũ khí của 4 Chức nghiệp nền tảng được quy chuẩn hóa 100% về kích thước 32x32 Native Pixel Art. Khắc phục triệt để viền chạm mép (đệm an toàn 2-3px), ngân sách màu cực kỳ tiết kiệm (7-14 màu/icon), không có mixels, viền ngoài than chì 1px. Mỗi vũ khí được xuất xưởng 3 định dạng: Native 32x32, In-Game 64x64, và Inspect Card 256x256 với khung viền 5 cấp độ hiếm (Common, Uncommon, Rare, Epic, Legendary).",
            "items": [
                # Vanguard
                ("Vanguard_01_Broadsword_Tier0_RustedIron_256x256_framed_common.png", "Kiếm Sắt Thô Sứt Mẻ (+0)", "Cấp 0 (Thường): Lưỡi mẻ răng cưa 1px, hoen rỉ ố màu, chuôi quấn vải thô sờn rách.", "Vanguard (Cơ Bản)", ["Vanguard", "Sword", "Common", "Tier 0"]),
                ("Vanguard_02_Broadsword_Tier1_SharpSteel_256x256_framed_uncommon.png", "Thép Mài Sắc Bén (+3)", "Cấp 1 (Cường Hóa +3): Qua thợ rèn tiền trạm, lưỡi mài phẳng phản xạ ánh kim.", "Vanguard (+3)", ["Vanguard", "Sword", "Uncommon", "Tier 1"]),
                ("Vanguard_03_Broadsword_Tier2_RunicFlame_256x256_framed_rare.png", "Hỏa Diệm Rực Lửa (+6 Khảm Ruby)", "Cấp 2 (Nguyên Tố Hỏa): Khảm ngọc Ruby. Lưỡi kiếm rực nhiệt lượng, cổ ngữ bốc khói.", "Vanguard (+6 Hỏa)", ["Vanguard", "Sword", "Rare", "Ruby"]),
                ("Vanguard_04_Broadsword_Tier2_GlacialIce_256x256_framed_rare.png", "Băng Tinh Sương Lạnh (+6 Khảm Sapphire)", "Cấp 2 (Nguyên Tố Băng): Khảm ngọc Sapphire. Thép xanh đóng băng, cổ ngữ cyan phát quang.", "Vanguard (+6 Băng)", ["Vanguard", "Sword", "Rare", "Sapphire"]),
                ("Vanguard_05_Broadsword_Tier3_DivineAscendant_256x256_framed_legendary.png", "Thần Binh Tối Thượng (+10 Divine)", "Cấp 3 (Thần Binh +10): Đúc từ Linh hồn Lãnh chúa tại Lò Rèn Cấm Địa. Tinh thể cosmic phát quang.", "Vanguard (+10 Divine)", ["Vanguard", "Sword", "Legendary", "Tier 3"]),
                ("Vanguard_06_Broadsword_Combat_Bloodied_256x256_framed_uncommon.png", "Chiến Trường Nhuốm Máu (Decal)", "Trạng Thái Động: Kích hoạt sau Combo 3-Hit chém quái. Máu tươi vấy loang lổ trên lưỡi kiếm.", "Vanguard (Tác Chiến)", ["Vanguard", "Decal", "Blood", "Dynamic"]),
                ("Vanguard_07_Solar_Fire_Greatsword_256x256_framed_legendary.png", "Thánh Quang Hỏa Diệm Đại Kiếm", "Đại kiếm rực rỡ vùng Sanctuary: Tông ấm áp, highlight vàng chanh, lõi hổ phách.", "Vanguard (Đại Kiếm)", ["Vanguard", "Greatsword", "Legendary", "Fire"]),
                ("Vanguard_08_Ornate_Silver_Rapier_256x256_framed_rare.png", "Kiếm Liễu Bạc Quý Tộc", "Kiếm đâm chính xác: Đòn đâm I-frame bỏ qua 50% giáp vật lý của mục tiêu.", "Vanguard (Kiếm Liễu)", ["Vanguard", "Rapier", "Rare", "Pierce"]),
                ("Vanguard_09_Iron_Round_Shield_256x256_framed_uncommon.png", "Khiên Tròn Sắt Khởi Đầu", "Khiên phụ trợ Vanguard: Tăng 20% khả năng chống đỡ đòn đánh vật lý.", "Vanguard (Khiên Tròn)", ["Vanguard", "Shield", "Uncommon", "Block"]),
                ("Vanguard_10_Knight_Heraldic_Kite_Shield_256x256_framed_rare.png", "Khiên Huy Hiệu Hiệp Sĩ", "Khiên tam giác lớn: Cho phép thực hiện Perfect Parry phản đòn 40 Posture Damage.", "Vanguard (Đại Thuẫn)", ["Vanguard", "Shield", "Rare", "Parry"]),

                # Ranger
                ("Ranger_01_Wooden_Hunting_Bow_256x256_framed_common.png", "Cung Săn Gỗ Tân Thủ", "Cung săn mộc mạc: Vũ khí khởi đầu của Du Hiệp, tầm bắn 1000cm, bắn nhanh ít tốn thể lực.", "Ranger (Cung Sơ Cấp)", ["Ranger", "Bow", "Common", "Tier 0"]),
                ("Ranger_02_Recurve_Hunting_Bow_256x256_framed_uncommon.png", "Cung Uốn Sừng Thợ Săn", "Cung săn gia cường: Tăng 25% lực bắn xuyên thấu và tốc độ bay của mũi tên.", "Ranger (Cung Gia Cường)", ["Ranger", "Bow", "Uncommon", "Tier 1"]),
                ("Ranger_03_Elven_Composite_Bow_256x256_framed_epic.png", "Cung Tiên Phong Thần Khí", "Cung cổ thụ thần bí: Mũi tên bắn ra kèm lốc xoáy phong nguyên tố xuyên quái.", "Ranger (Cung Thần)", ["Ranger", "Bow", "Epic", "Tier 2"]),
                ("Ranger_04_Rogue_Hunting_Dagger_256x256_framed_uncommon.png", "Dao Găm Thợ Săn Sát Thủ", "Vũ khí cận chiến nhanh: Đâm lén từ sau lưng tăng +50% sát thương chí mạng.", "Ranger (Dao Găm)", ["Ranger", "Dagger", "Uncommon", "Crit"]),
                ("Ranger_05_Frost_Shard_Kris_256x256_framed_rare.png", "Dao Găm Băng Tinh Kris", "Lưỡi dao uốn lượn sắc lạnh: Đòn đánh gây tích lũy sương giá làm chậm 40% tốc chạy.", "Ranger (Dao Băng)", ["Ranger", "Dagger", "Rare", "Frost"]),

                # Arcanist
                ("Arcanist_01_Magic_Wizard_Staff_256x256_framed_common.png", "Trượng Gỗ Phép Tân Thủ", "Trượng khởi đầu của Arcanist: Thân gỗ mun đính đá thạch anh, định hướng ma pháp.", "Arcanist (Trượng Gỗ)", ["Arcanist", "Staff", "Common", "Tier 0"]),
                ("Arcanist_02_Crystal_Wizard_Staff_256x256_framed_uncommon.png", "Trượng Pha Lê Ma Thuật", "Trượng pha lê xanh: Giảm 15% thời gian vận niệm các chiêu thức diện rộng AoE.", "Arcanist (Trượng Pha Lê)", ["Arcanist", "Staff", "Uncommon", "Tier 1"]),
                ("Arcanist_03_Void_Corrupted_Staff_256x256_framed_epic.png", "Hư Không Ma Trượng (32x32)", "Pháp bảo vùng Contested: Tinh thể tím hư không ma mị, khuếch đại sát thương bóng tối.", "Arcanist (Hư Không)", ["Arcanist", "Staff", "Epic", "Void"]),
                ("Arcanist_04_Astral_Archmage_Staff_256x256_framed_legendary.png", "Đại Pháp Trượng Tinh Tú", "Thần trượng Archmage: Triệu hồi mưa sao băng thiêu rụi toàn bộ quái vật trên diện rộng.", "Arcanist (Thần Trượng)", ["Arcanist", "Staff", "Legendary", "Tier 3"]),
                ("Arcanist_05_Ancient_Arcane_Grimoire_256x256_framed_epic.png", "Cổ Thư Bí Kíp Ma Thuật", "Cổ thư cấm thuật: Mở khóa kỹ năng Tối Thượng 'Xoáy Nghịch Không Gian' cho Arcanist.", "Arcanist (Cổ Thư)", ["Arcanist", "Grimoire", "Epic", "SkillBook"]),

                # Acolyte
                ("Acolyte_01_Holy_War_Mace_256x256_framed_rare.png", "Chùy Chiến Thánh Quang", "Vũ khí Thánh đường: Giộng mạnh chùy giải phóng sóng xung kích thanh tẩy, hồi máu.", "Acolyte (Chùy Thánh)", ["Acolyte", "Mace", "Rare", "Holy"]),
                ("Acolyte_02_Spiked_Morningstar_Mace_256x256_framed_uncommon.png", "Chùy Gai Sao Mai Xuất Huyết", "Chùy gai kim loại nặng: Gây hiệu ứng Xuất huyết (Bleed) dồn dập và tích tụ Posture.", "Acolyte (Chùy Gai)", ["Acolyte", "Mace", "Uncommon", "Bleed"]),
                ("Acolyte_03_Death_Obsidian_Scythe_256x256_framed_epic.png", "Lưỡi Hái Tử Thần Hắc Thạch", "Vũ khí tử thần: Đòn chém quét diện rộng 180° hồi phục 5% Máu tối đa mỗi khi hạ quái.", "Acolyte (Lưỡi Hái)", ["Acolyte", "Scythe", "Epic", "Lifesteal"]),
            ]
        },

        # --- TAB 3: ARMOR, POTIONS, AND GEMS ---
        {
            "id": "tab_gear",
            "title": "🛡️ Giáp, Dược Phẩm & Đá Quý (Gear & Supplies)",
            "folder": "03_Armor_Potions_and_Gems",
            "badge": "Trang Phục & Tiếp Tế",
            "summary": "Bộ sưu tập trang bị phòng hộ (Mũ giáp sắt/da, Giáp thân hiệp sĩ/thợ săn/học giả, Xà cạp, Ủng da), bình dược phẩm tiếp tế hồi phục tức thì gán phím tắt nhanh 1-4, đá quý khảm rèn trang bị, và nguyên liệu chế tác khoáng sản.",
            "items": [
                # Armors
                ("Armor_Steel_Knight_Cuirass.png", "Giáp Ngực Thép Hiệp Sĩ", "Trang bị thân trên Vanguard: Cung cấp +35 Giáp vật lý, giảm 20% sát thương Posture.", "Giáp Thân", ["Armor", "Vanguard", "Heavy"]),
                ("Armor_Leather_Scout_Vest.png", "Áo Giáp Da Thợ Săn", "Trang bị thân Ranger: Tăng độ linh hoạt, giảm chi phí Thể lực của thao tác Lướt.", "Giáp Thân", ["Armor", "Ranger", "Medium"]),
                ("Armor_Arcanist_Scholar_Tunic.png", "Áo Choàng Học Giả", "Trang bị Arcanist: Kháng ma thuật nguyên tố cao và tăng 25 điểm Mana dự trữ.", "Giáp Thân", ["Armor", "Arcanist", "Cloth"]),
                ("Helmet_Steel_Knight_Closed.png", "Mũ Trùm Kín Thép", "Mũ sắt kín mặt: Bảo vệ đầu tuyệt đối, chống lại đòn đánh chí mạng của quái vật.", "Mũ Nón", ["Helmet", "Vanguard", "Heavy"]),
                ("Helmet_Steel_Knight_Open.png", "Mũ Chiến Binh Kính Lật", "Mũ giáp kính hở: Cân bằng giữa khả năng phòng thủ và tầm nhìn quan sát chiến trường.", "Mũ Nón", ["Helmet", "Vanguard", "Medium"]),
                ("Helmet_Leather_Ranger_Hood.png", "Mũ Trùm Da Du Mục", "Mũ trùm da thợ săn: Giúp ngụy trang, giảm bán kính phát hiện của quái dã ngoại.", "Mũ Nón", ["Helmet", "Ranger", "Light"]),
                ("Greaves_Steel_Knight_Legguards.png", "Xà Cạp Thép Hiệp Sĩ", "Giáp chân kim loại: Tăng khả năng trụ vững, giảm 30% hiệu ứng đẩy lùi (Knockback).", "Giáp Chân", ["Greaves", "Legs", "Heavy"]),
                ("Boots_Leather_Traveler_Boots.png", "Ủng Da Lữ Hành", "Ủng da dã ngoại: Tăng +40 tốc độ di chuyển cơ bản ngoài khu vực an toàn.", "Giày Ủng", ["Boots", "Speed", "Utility"]),

                # Potions
                ("Potion_Health_Crimson_Flask.png", "Bình Dược Hồi Máu (Health Flask)", "Dược phẩm khẩn cấp: Hồi phục tức thì 100 HP trong 0.8 giây, gán phím nóng [1].", "Dược Phẩm", ["Potion", "Healing", "Consumable"]),
                ("Potion_Mana_Cyan_Phial.png", "Bình Dược Hồi Mana (Mana Phial)", "Dược phẩm ma pháp: Hồi phục 80 Mana, xóa bỏ trạng thái Câm lặng.", "Dược Phẩm", ["Potion", "Mana", "Consumable"]),
                ("Potion_Stamina_Green_Draught.png", "Dược Thảo Thể Lực (Stamina Draught)", "Dược liệu tăng lực: Giảm 50% chi phí thể lực lướt né trong 10 giây.", "Dược Phẩm", ["Potion", "Stamina", "Consumable"]),

                # Jewelry & Crafting
                ("Jewelry_Engraved_Gold_Ring.png", "Nhẫn Vàng Cổ Ngữ", "Nhẫn trang bị Slot 1: Tăng 15% sát thương chí mạng và gia tăng thời gian I-frame.", "Trang Sức", ["Ring", "Jewelry", "Tier 3"]),
                ("Jewelry_Sapphire_Pendant_Amulet.png", "Dây Chuyền Lam Ngọc", "Dây chuyền cổ: Tăng 30 Mana tối đa và tạo khiên hấp thụ sát thương ma thuật.", "Trang Sức", ["Amulet", "Jewelry", "Tier 3"]),
                ("Crafting_Blacksmith_Ward_Stone.png", "Đá Bảo Hộ Cổ Xưa", "Đá hộ mệnh thợ rèn: Bảo vệ trang bị không bị tụt cấp khi rèn thất bại mốc +7..+10.", "Đá Phù Trợ", ["Crafting", "Ward", "Tier 4"]),

                # Materials & Gems
                ("Material_Rough_Iron_Ore.png", "Quặng Sắt Thô", "Nguyên liệu chế tạo: Dùng để đúc phôi kiếm và giáp sắt cơ bản tại Thợ rèn.", "Nguyên Liệu", ["Ore", "Crafting", "Tier 1"]),
                ("Material_Gold_Ingot_Bar.png", "Thỏi Vàng Đúc", "Kim loại quý: Tiền tệ giao dịch cấp cao và nguyên liệu dát vàng trang bị Thần Thánh.", "Nguyên Liệu", ["Ingot", "Gold", "Tier 3"]),
                ("Material_Clay_Terracotta_Brick.png", "Gạch Nung Cổ Xưa", "Vật liệu kiến trúc: Xây dựng và nâng cấp lò than tại Trạm nghỉ Sanctuary.", "Nguyên Liệu", ["Material", "Sanctuary", "Tier 1"]),
                ("Gemstone_Faceted_Red_Ruby.png", "Hồng Ngọc Đa Diện", "Đá quý khảm trang bị: Khảm vào vũ khí để thêm thuộc tính Sát thương Hỏa Diệm.", "Đá Quý", ["Gemstone", "Ruby", "Tier 3"]),
                ("Gemstone_Radiant_Blue_Sapphire.png", "Lam Ngọc Rực Sáng", "Đá quý khảm trang bị: Khảm vào giáp/vũ khí tăng Mana và sát thương Băng.", "Đá Quý", ["Gemstone", "Sapphire", "Tier 3"]),
                ("Gemstone_Cut_Green_Emerald.png", "Ngọc Lục Bảo Giác Cắt", "Đá quý khảm trang bị: Gia tăng tốc độ hồi phục Thể lực và tốc chạy.", "Đá Quý", ["Gemstone", "Emerald", "Tier 3"]),
                ("Gemstone_Violet_Amethyst_Cluster.png", "Tinh Thể Thạch Anh Tím", "Đá quý huyền bí: Khảm vào trang bị kháng hiệu ứng Khống chế và Sát khí.", "Đá Quý", ["Gemstone", "Amethyst", "Tier 3"]),
                ("Loot_Curved_Monster_Fang.png", "Nanh Vuốt Quái Vật", "Chiến lợi phẩm săn bắn: Nguyên liệu tôi luyện độc dược và rèn dao găm sát thủ.", "Chiến Lợi Phẩm", ["MonsterLoot", "Fang", "Tier 2"]),
                ("Loot_Glowing_Stone_Golem_Core.png", "Lõi Golem Đá Phát Quang", "Chiến lợi phẩm Lãnh chúa: Rơi từ Boss Stone Golem, dùng rèn Giáp Bất Tử Tier 4.", "Linh Hồn Boss", ["BossLoot", "Core", "Tier 4"]),
            ]
        },

        # --- TAB 4: WORLD BOSSES ---
        {
            "id": "tab_bosses",
            "title": "👹 Lãnh Chúa Thế Giới (World Bosses)",
            "folder": "04_Bosses_and_Monsters",
            "badge": "Lãnh Chúa & Quái Vật",
            "summary": "Thiết kế các Lãnh Chúa thế giới mở: Stone Golem (Boss hộ vệ tàn tích cổ) và Lich Necromancer (Lãnh chúa vong hồn). Đi kèm cơ chế phá hủy từng bộ phận (Part Breaking) bẻ sừng, chặt lõi rơi nguyên liệu rèn Thần Binh Bậc 4-5.",
            "items": [
                ("01_Boss_Stone_Golem_Handcrafted.jpg", "Boss Stone Golem (Concept Hoang Phế)", "Lãnh chúa đá cổ thụ phủ rêu phong, vết nứt ngực rực sáng tinh thể lam ngọc.", "Concept Art", ["Boss", "Stone Golem", "Sanctuary"]),
                ("01_Boss_Stone_Golem_Transparent.png", "Boss Stone Golem (Sprite Tách Nền)", "Sprite Lãnh chúa đá khổng lồ tách nền trong suốt, sẵn sàng nhập vào Unreal Engine.", "In-Game Sprite", ["Transparent", "Sprite", "Paper2D"]),
                ("02_Boss_Lich_Necromancer_Handcrafted.jpg", "Boss Lich Necromancer (Concept Âm Giới)", "Pháp sư vong hồn trôi nổi trong không trung, áo choàng rách ma mị và trượng đầu lâu.", "Concept Art", ["Boss", "Lich", "Contested Zone"]),
                ("02_Boss_Lich_Necromancer_Transparent.png", "Boss Lich Necromancer (Sprite Tách Nền)", "Sprite Lich tách nền trong suốt, hiệu ứng khói tím linh hồn bao quanh.", "In-Game Sprite", ["Transparent", "Sprite", "Paper2D"]),
            ]
        },

        # --- TAB 5: ENVIRONMENT & WORLD ---
        {
            "id": "tab_world",
            "title": "🗺️ Thế Giới & Môi Trường (World & Environment)",
            "folder": "05_Environment_and_World",
            "badge": "Môi Trường 2.5D Isometric",
            "summary": "Thiết kế không gian thế giới mở không rào chắn cấp độ: Bản đồ địa hình tổng quan phân 3 vùng (Sanctuary, Wilderness, Contested Zone), phối cảnh Tàn tích cổ đại, và mô hình Blockout 3D Isometric xoay 360 độ phục vụ kiểm tra va chạm CapsuleComponent.",
            "items": [
                ("Complete_Map_Showcase.jpg", "Bản Đồ Thế Giới Mở Tổng Quan", "Phối cảnh phân vùng thế giới: Vùng an toàn Sanctuary, Rừng rậm hoang dã, và Vùng cấm địa.", "World Map", ["Overworld", "Sanctuary", "Wilderness", "Contested"]),
                ("Ruins_Concept.jpg", "Bản Vẽ Ý Niệm Tàn Tích Cổ Đại", "Kiến trúc cột đá Hy Lạp hoang phế, bậc thềm rêu phong và ánh nắng vàng chiếu rọi.", "Concept Art", ["Ruins", "Architecture", "Environment"]),
                ("Ruins_Blockout_3D_Isometric.gif", "Mô Hình 3D Isometric Blockout (GIF 360°)", "Mô phỏng hình học không gian 3D góc nhìn 2.5D Dimetric 2:1 (-45° pitch, 45° yaw).", "3D Blockout", ["Isometric", "3D GIF", "Collision Geometry"]),
            ]
        }
    ]

    total_assets = sum(len(c["items"]) for c in categories)

    html = f"""<!DOCTYPE html>
<html lang="vi" class="dark">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Project Ascendant — Master Art Gallery (Anti-AI Handcrafted Standard)</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <script>
        tailwind.config = {{
            darkMode: 'class',
            theme: {{
                extend: {{
                    colors: {{
                        primary: '#6366F1',
                        accent: '#F59E0B',
                        darkbg: '#090D16',
                        cardbg: '#111827',
                        bordercolor: '#1F2937'
                    }}
                }}
            }}
        }}
    </script>
    <style>
        .pixelated {{
            image-rendering: -moz-crisp-edges;
            image-rendering: -webkit-crisp-edges;
            image-rendering: pixelated;
            image-rendering: crisp-edges;
        }}
        .tab-btn.active {{
            background: linear-gradient(135deg, #4F46E5 0%, #7C3AED 100%);
            color: #FFFFFF;
            border-color: #818CF8;
            box-shadow: 0 4px 15px rgba(79, 70, 229, 0.4);
        }}
    </style>
</head>
<body class="bg-darkbg text-slate-200 min-h-screen flex flex-col font-sans selection:bg-indigo-500 selection:text-white">

    <!-- Header Banner -->
    <header class="border-b border-bordercolor bg-slate-900/80 backdrop-blur sticky top-0 z-40">
        <div class="max-w-7xl mx-auto px-4 py-4 flex flex-col sm:flex-row items-center justify-between gap-4">
            <div class="flex items-center gap-3">
                <div class="w-10 h-10 rounded-lg bg-gradient-to-tr from-indigo-600 to-amber-500 flex items-center justify-center font-bold text-white shadow-lg shadow-indigo-500/30">
                    PA
                </div>
                <div>
                    <h1 class="text-xl font-extrabold tracking-tight text-white flex items-center gap-2">
                        Project Ascendant <span class="text-xs px-2 py-0.5 rounded bg-indigo-500/20 text-indigo-400 border border-indigo-500/30">Anti-AI Handcrafted</span>
                    </h1>
                    <p class="text-xs text-slate-400">Capcom CPS2 128x128 Action-First & 32x32 HD-2D Weapon Arsenal</p>
                </div>
            </div>
            
            <div class="flex items-center gap-3">
                <div class="flex items-center gap-2 px-3 py-1.5 rounded-lg bg-slate-800 border border-slate-700 text-xs">
                    <span class="w-2.5 h-2.5 rounded-full bg-emerald-500 animate-pulse"></span>
                    <span class="font-medium text-slate-300">Tổng Số: <strong class="text-emerald-400">{total_assets} Assets</strong></span>
                </div>
                <div class="flex items-center gap-2 px-3 py-1.5 rounded-lg bg-slate-800 border border-slate-700 text-xs">
                    <span class="text-amber-400 font-bold">5 Danh Mục</span>
                </div>
            </div>
        </div>

        <!-- Navigation Tabs -->
        <div class="max-w-7xl mx-auto px-4 mt-2 pb-3 overflow-x-auto">
            <div class="flex gap-2 min-w-max">
                <button onclick="switchTab('all')" class="tab-btn active px-4 py-2 rounded-lg text-xs font-semibold border border-slate-700 bg-slate-800 hover:bg-slate-700 transition">
                    ⭐ Tất Cả ({total_assets})
                </button>
    """

    for cat in categories:
        count = len(cat["items"])
        html += f"""
                <button onclick="switchTab('{cat['id']}')" class="tab-btn px-4 py-2 rounded-lg text-xs font-semibold border border-slate-700 bg-slate-800 hover:bg-slate-700 transition flex items-center gap-1.5">
                    {cat['title']} <span class="px-1.5 py-0.2 rounded-full bg-slate-900/60 text-[10px] text-slate-300 font-mono">{count}</span>
                </button>
        """

    html += """
            </div>
        </div>
    </header>

    <!-- Main Content -->
    <main class="max-w-7xl mx-auto px-4 py-8 flex-1 space-y-12">
    """

    for cat in categories:
        html += f"""
        <section id="{cat['id']}" class="category-section space-y-4">
            <div class="border-b border-slate-800 pb-3 flex flex-col md:flex-row md:items-end justify-between gap-2">
                <div>
                    <div class="flex items-center gap-2">
                        <h2 class="text-lg font-bold text-white tracking-wide">{cat['title']}</h2>
                        <span class="text-xs px-2 py-0.5 rounded bg-indigo-900/40 text-indigo-300 border border-indigo-700/40 font-mono">{cat['badge']}</span>
                    </div>
                    <p class="text-xs text-slate-400 mt-1 max-w-4xl leading-relaxed">{cat['summary']}</p>
                </div>
                <div class="text-xs font-mono text-slate-500 whitespace-nowrap">
                    Thư mục: <code class="text-indigo-400">{cat['folder']}/</code>
                </div>
            </div>

            <div class="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-5 gap-4">
        """

        for filename, title, desc, tag, badges in cat["items"]:
            file_rel = f"{cat['folder']}/{filename}"
            badge_html = "".join([f'<span class="px-1.5 py-0.5 text-[9px] font-mono rounded bg-slate-800 border border-slate-700 text-slate-300">{b}</span>' for b in badges[:3]])
            is_pixel = "pixelated" if (filename.endswith(".png") and ("32x32" in filename or "native" in filename or "ingame" in filename or "showcase" in filename or "Armor" in filename or "Potion" in filename or "Gemstone" in filename or "Material" in filename or "Loot" in filename or "Shield" in filename or "Helmet" in filename or "Boots" in filename or "Greaves" in filename or "Jewelry" in filename)) else ""
            bg_style = "background: radial-gradient(circle at center, #1E293B 0%, #0F172A 100%);"

            html += f"""
                <div class="bg-cardbg border border-bordercolor rounded-xl overflow-hidden shadow-lg hover:border-indigo-500/50 hover:shadow-indigo-500/10 transition group flex flex-col cursor-pointer" onclick="openModal('{file_rel}', '{title}', '{desc}', '{tag}')">
                    <div class="aspect-square flex items-center justify-center p-3 relative overflow-hidden border-b border-bordercolor/60" style="{bg_style}">
                        <img src="{file_rel}" alt="{title}" class="max-w-full max-h-full object-contain transition-transform duration-200 group-hover:scale-105 {is_pixel}" loading="lazy">
                        <div class="absolute top-2 right-2">
                            <span class="px-1.5 py-0.5 rounded text-[10px] font-bold bg-slate-900/80 text-amber-300 border border-amber-500/30 backdrop-blur">
                                {tag}
                            </span>
                        </div>
                    </div>
                    <div class="p-3 flex-1 flex flex-col justify-between space-y-2">
                        <div>
                            <h3 class="font-bold text-xs text-white group-hover:text-indigo-400 transition line-clamp-1">{title}</h3>
                            <p class="text-[11px] text-slate-400 mt-1 line-clamp-2 leading-relaxed">{desc}</p>
                        </div>
                        <div class="flex flex-wrap gap-1 pt-1 border-t border-slate-800/80">
                            {badge_html}
                        </div>
                    </div>
                </div>
            """

        html += """
            </div>
        </section>
        """

    html += """
    </main>

    <!-- Modal Lightbox -->
    <div id="inspectModal" class="fixed inset-0 z-50 bg-black/80 backdrop-blur-sm hidden flex items-center justify-center p-4" onclick="closeModal(event)">
        <div class="bg-slate-900 border border-slate-700 rounded-2xl max-w-2xl w-full p-6 space-y-4 shadow-2xl relative" onclick="event.stopPropagation()">
            <div class="flex items-center justify-between border-b border-slate-800 pb-3">
                <div class="flex items-center gap-2">
                    <span id="modalTag" class="px-2 py-0.5 rounded text-xs font-bold bg-amber-500/20 text-amber-400 border border-amber-500/30"></span>
                    <h3 id="modalTitle" class="text-base font-bold text-white"></h3>
                </div>
                <button onclick="closeModalDirect()" class="text-slate-400 hover:text-white text-lg font-bold px-2 py-1">&times;</button>
            </div>

            <div class="aspect-square max-h-[380px] w-full flex items-center justify-center bg-slate-950/80 rounded-xl border border-slate-800 p-4 overflow-hidden relative">
                <img id="modalImg" src="" alt="" class="max-w-full max-h-full object-contain pixelated shadow-lg">
            </div>

            <div class="space-y-2">
                <p id="modalDesc" class="text-xs text-slate-300 leading-relaxed"></p>
                <div class="flex items-center justify-between text-[11px] text-slate-400 pt-2 border-t border-slate-800">
                    <div>Đường dẫn: <code id="modalPath" class="text-indigo-400 font-mono"></code></div>
                    <button onclick="copyPath()" class="px-2 py-1 rounded bg-slate-800 hover:bg-slate-700 text-slate-200 border border-slate-700">Sao chép path</button>
                </div>
            </div>
        </div>
    </div>

    <!-- Script for Tab Switching & Modal -->
    <script>
        function switchTab(tabId) {
            document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
            event.target.closest('.tab-btn').classList.add('active');

            const sections = document.querySelectorAll('.category-section');
            if (tabId === 'all') {
                sections.forEach(sec => sec.classList.remove('hidden'));
            } else {
                sections.forEach(sec => {
                    if (sec.id === tabId) {
                        sec.classList.remove('hidden');
                    } else {
                        sec.classList.add('hidden');
                    }
                });
            }
        }

        let currentPath = '';
        function openModal(path, title, desc, tag) {
            currentPath = path;
            document.getElementById('modalImg').src = path;
            document.getElementById('modalTitle').textContent = title;
            document.getElementById('modalDesc').textContent = desc;
            document.getElementById('modalTag').textContent = tag;
            document.getElementById('modalPath').textContent = path;
            document.getElementById('inspectModal').classList.remove('hidden');
        }

        function closeModal(e) {
            if (e.target.id === 'inspectModal') {
                closeModalDirect();
            }
        }

        function closeModalDirect() {
            document.getElementById('inspectModal').classList.add('hidden');
        }

        function copyPath() {
            navigator.clipboard.writeText('/mnt/Data/Projects/project-games/ProjectAscendant/Art_Gallery/' + currentPath);
            alert('Đã sao chép đường dẫn tuyệt đối vào clipboard!');
        }

        document.addEventListener('keydown', (e) => {
            if (e.key === 'Escape') closeModalDirect();
        });
    </script>
</body>
</html>
    """

    with open(OUTPUT_HTML, "w", encoding="utf-8") as f:
        f.write(html)

    print(f"Generated clean Master Art Gallery HTML ({total_assets} items) at: {OUTPUT_HTML}")

if __name__ == "__main__":
    generate_gallery()
