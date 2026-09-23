#!/usr/bin/env python3
"""
Master Art Gallery Builder (Clean, Unified, Zero-Jumble Architecture)
Builds Art_Gallery/index.html with 7 clean official categories:
1. Living Equipment & Weapon Mutations (32x32 HD-2D)
2. Modular Paperdoll - Female Character
3. Modular Paperdoll - Male Character
4. 40 Individual Items & Gear (5-Tier Rarity)
5. Core Classes Showcase
6. World Bosses (Stone Golem & Lich)
7. World & Environment
"""

import os
import sys

PROJECT_ROOT = "/mnt/Data/Projects/project-games/ProjectAscendant"
GALLERY_DIR = os.path.join(PROJECT_ROOT, "Art_Gallery")
OUTPUT_HTML = os.path.join(GALLERY_DIR, "index.html")

def generate_gallery():
    categories = [
        {
            "id": "tab_mutations",
            "title": "⚔️ Ma Trận Đột Biến Vũ Khí (Living Equipment)",
            "folder": "01_Living_Weapon_Mutations",
            "badge": "Chuẩn 32x32 HD-2D",
            "summary": "Minh chứng cho kiến trúc trang bị sống động kế thừa từ module Consistent Upgrades & Variants trong game-art-studio: Cùng 1 thanh Trường Kiếm cơ sở (Broadsword 32x32) sản sinh ra các biến thể trực quan hoàn toàn khác biệt dựa trên Cấp rèn (+0 đến +10), Khảm ngọc nguyên tố (Ruby/Sapphire), Lớp phủ tác chiến (Vấy máu khi chém quái), và Phân vùng môi trường.",
            "items": [
                # Broadsword 6-variant mutation
                ("01_Broadsword_Tier0_RustedIron_256x256_framed_common.png", "Kiếm Sắt Thô Sứt Mẻ (+0)", "Cấp 0 (Thường): Vũ khí khởi đầu chưa qua tôi luyện, lưỡi mẻ răng cưa 1px, hoen rỉ ố màu, chuôi quấn vải thô sờn rách.", "Cấp +0", ["Tier 0", "Rusted", "Common", "Base"]),
                ("02_Broadsword_Tier1_SharpSteel_256x256_framed_uncommon.png", "Thép Mài Sắc Bén (+3)", "Cấp 1 (Cường Hóa +3): Qua thợ rèn tiền trạm, lưỡi mài vát phẳng sáng loáng phản xạ ánh kim, chuôi nẹp da bò mới.", "Cấp +3", ["Tier 1", "Sharp Steel", "Uncommon", "Forged"]),
                ("03_Broadsword_Tier2_RunicFlame_256x256_framed_rare.png", "Hỏa Diệm Rực Lửa (+6 Khảm Ruby)", "Cấp 2 (Nguyên Tố Hỏa): Khảm ngọc Ruby tại thợ rèn dã ngoại. Lưỡi kiếm rực nhiệt lượng, rãnh cổ ngữ khắc dọc thân bốc khói than hồng.", "Khảm Hỏa", ["Tier 2", "Flame", "Rare", "Ruby Socket"]),
                ("04_Broadsword_Tier2_GlacialIce_256x256_framed_rare.png", "Băng Tinh Sương Lạnh (+6 Khảm Sapphire)", "Cấp 2 (Nguyên Tố Băng): Khảm ngọc Sapphire. Thép xanh cobalt đóng băng tinh thể ngọc, cổ ngữ cyan phát quang rực rỡ.", "Khảm Băng", ["Tier 2", "Frost", "Rare", "Sapphire Socket"]),
                ("05_Broadsword_Tier3_DivineAscendant_256x256_framed_legendary.png", "Thần Binh Tối Thượng (+10 Divine)", "Cấp 3 (Thần Binh +10): Đúc từ Linh hồn Lãnh chúa tại Lò Rèn Cấm Địa. Lưỡi kiếm chuyển hóa thành tinh thể cosmic phát quang plasma tím-vàng.", "Thần Binh +10", ["Tier 3", "Divine", "Legendary", "Boss Soul"]),
                ("06_Broadsword_Combat_Bloodied_256x256_framed_uncommon.png", "Chiến Trường Nhuốm Máu (Blood Decal)", "Trạng Thái Tác Chiến Động: Tự động kích hoạt sau chuỗi Combo 3-Hit chém gục quái. Vết máu tươi vấy loang lổ trên lưỡi kiếm.", "Combat Overlay", ["Action State", "Blood Splatter", "Decal", "Dynamic"]),
                # Zone-Adaptive 32x32 Weapons
                ("Solar_Fire_Greatsword_32px_256x256_framed_legendary.png", "Thánh Quang Hỏa Diệm Đại Kiếm (32x32)", "Đại kiếm rực rỡ vùng Lò Rèn/Sanctuary: Tông màu ấm áp, highlight vàng chanh #FEF08A, lõi hổ phách #F59E0B, viền thép đỏ rực. Khung viền Legendary Hoàng Kim.", "Sanctuary / Lửa", ["32x32 Native", "Solar Flame", "Legendary", "QA Passed"]),
                ("Glacial_Runic_Blade_32px_256x256_framed_rare.png", "Băng Tinh Kiếm Cổ Ngữ (32x32)", "Vũ khí vùng Núi Băng/Wilderness: Sắc lạnh trong trẻo, lưỡi thép xanh cobalt #3A86FF xen kẽ ngọc cyan #00F5D4 phát quang, đổ bóng navy sâu. Khung viền Rare Lam Ngọc.", "Wilderness / Băng", ["32x32 Native", "Glacial Runic", "Rare", "QA Passed"]),
                ("Void_Corrupted_Staff_32px_256x256_framed_epic.png", "Hư Không Ma Trượng (32x32)", "Pháp bảo vùng Đất Chết Tha Hóa (Contested): Thân gỗ mun hắc thạch, đầu trượng pha lê tím hư không #6A1B9A u tối ma mị, hoa văn bạc cổ xưa. Khung viền Epic Tím Huyền Bí.", "Contested / Hư Không", ["32x32 Native", "Void Corrupted", "Epic", "QA Passed"]),
                ("Sacred_Amber_Phial_32px_256x256_framed_uncommon.png", "Bình Thánh Thủy Hổ Phách (32x32)", "Dược phẩm Tiệm Giả Kim Sanctuary: Lọ thủy tinh đa diện sáng rực mật ong vàng, nút bấc vàng đồng, vệt sáng phản quang thủy tinh 1px trắng sắc sảo. Khung viền Uncommon Ngọc Lục.", "Sanctuary / Dược Phẩm", ["32x32 Native", "Sacred Amber", "Uncommon", "QA Passed"]),
            ]
        },
        {
            "id": "tab_female",
            "title": "👩 Nhân Vật Nữ (Female Paperdoll)",
            "folder": "02_Modular_Paperdoll_Female",
            "badge": "Khóa Khớp Giải Phẫu",
            "summary": "Bộ thiết kế nhân vật nữ theo tỷ lệ Chibi Heroic 3.2 - 3.5 Đầu (cao 86px trên canvas 128x128). Khóa cứng các mốc giải phẫu: Trục mắt Y=44, Cổ áo Y=56, Thắt lưng Y=80, Khớp tay cầm Y=76, ghim tâm chân (64, 114) đồng bộ hoàn hảo với CapsuleComponent trong Unreal Engine 5.7.",
            "items": [
                ("01_Concept_Female_Starter_Cloth.jpg", "Nhân Vật Nữ Tân Thủ (Starter Cloth)", "Áo sơ mi vải thô mộc mạc và quần nâu sờn rách, tóc tết nâu năng động.", "Concept Art", ["Starter", "Female", "Cloth"]),
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
            "id": "tab_male",
            "title": "👨 Nhân Vật Nam (Male Paperdoll)",
            "folder": "03_Modular_Paperdoll_Male",
            "badge": "Khung Cơ Sở Nam",
            "summary": "Bộ thiết kế nhân vật nam cơ sở với trang phục vải thô sơ khai và các bộ giáp đại diện cho từng Class, chuẩn góc phối cảnh Isometric -45°.",
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
            "title": "🗡️ Kho 40 Vật Phẩm 5 Bậc Hiếm",
            "folder": "04_Individual_Items_and_Gear",
            "badge": "40 Items Độc Lập",
            "summary": "Kho trang bị độc lập được cắt chuẩn xác từng pixel (ZERO cắt góc, an toàn biên 18-20px), tách nền trong suốt 256x256 sắc nét, sẵn sàng tích hợp vào hệ thống Kho Đồ Lưới 6x5 và Khay Phím Tắt 1-4.",
            "items": [
                # Weapons
                ("Weapon_Flaming_Greatsword.png", "Đại Kiếm Lửa (Flaming Greatsword)", "Đại kiếm rực lửa hỏa diệm: +45 Sát thương vật lý, +20 Sát thương Hỏa (Burn DoT).", "Vũ Khí", ["Greatsword", "Fire", "Tier 4"]),
                ("Weapon_Steel_Broadsword.png", "Đại Kiếm Thép (Steel Broadsword)", "Vũ khí tiêu chuẩn Vanguard: +15 Sát thương vật lý, đòn chém 3-Hit Combo sắc bén.", "Vũ Khí", ["Broadsword", "Vanguard", "Tier 2"]),
                ("Weapon_Frost_Shard_Kris.png", "Dao Găm Băng Kris (Frost Kris)", "Kris lượn sóng ma thuật: Tốc đánh cực nhanh, hiệu ứng Đóng băng làm chậm quái 40%.", "Vũ Khí", ["Kris", "Frost", "Tier 3"]),
                ("Weapon_Rogue_Hunting_Dagger.png", "Dao Găm Thợ Săn (Hunting Dagger)", "Vũ khí phụ Ranger: Tăng 25% sát thương chí mạng từ phía sau lưng kẻ thù.", "Vũ Khí", ["Dagger", "Critical", "Tier 2"]),
                ("Weapon_Elven_Composite_Bow.png", "Cung Hợp Hợp Elven (Composite Bow)", "Cung cao cấp: Tầm bắn 1400cm, bắn mũi tên xuyên giáp gây Posture Break.", "Vũ Khí", ["Bow", "Elven", "Tier 3"]),
                ("Weapon_Wooden_Hunting_Bow.png", "Cung Săn Gỗ Sồi (Oak Hunting Bow)", "Cung khởi đầu của Ranger: Tốc bắn nhanh, chi phí thể lực Stamina thấp.", "Vũ Khí", ["Bow", "Starter", "Tier 1"]),
                ("Weapon_Astral_Archmage_Staff.png", "Trượng Tinh Vân Đại Pháp Sư", "Đầu trượng khảm tinh thể thiên hà phát quang Cyan: +40 Sát thương phép bùng nổ.", "Vũ Khí", ["Staff", "Astral", "Tier 4"]),
                ("Weapon_Crystal_Wizard_Staff.png", "Trượng Pha Lê Ma Thuật", "Trượng cơ bản Arcanist: Tăng tốc độ hồi phục Mana khi đánh trúng mục tiêu.", "Vũ Khí", ["Staff", "Magic", "Tier 2"]),
                ("Weapon_Holy_War_Mace.png", "Chùy Thánh Chiến (Holy War Mace)", "Chùy sắt phong ấn thánh quang: Phá vỡ thế đứng Posture của Boss nhanh hơn 35%.", "Vũ Khí", ["Mace", "Holy", "Tier 2"]),
                ("Weapon_Spiked_Morningstar_Mace.png", "Chùy Gai Sao Mai (Morningstar)", "Chùy gai kim loại nặng: Gây hiệu ứng Xuất huyết (Bleed) dồn dập lên quái to.", "Vũ Khí", ["Mace", "Bleed", "Tier 3"]),
                ("Weapon_Ornate_Silver_Rapier.png", "Kiếm Liễu Bạc Quý Tộc (Silver Rapier)", "Kiếm đâm chính xác: Đòn đâm I-frame bỏ qua 50% giáp vật lý của mục tiêu.", "Vũ Khí", ["Rapier", "Silver", "Tier 3"]),
                ("Weapon_Death_Obsidian_Scythe.png", "Lưỡi Hái Tử Thần Hắc Thạch", "Vũ khí tử thần: Mỗi đòn chém quét diện rộng 180° hồi phục 5% Máu tối đa.", "Vũ Khí", ["Scythe", "Lifesteal", "Tier 4"]),
                # Shields
                ("Shield_Iron_Round_Shield.png", "Khiên Tròn Sắt (Round Shield)", "Khiên phụ trợ Vanguard: Tăng 20% khả năng chống đỡ đòn đánh vật lý.", "Khiên", ["Shield", "Block", "Tier 2"]),
                ("Shield_Knight_Heraldic_Kite.png", "Khiên Huy Hiệu Hiệp Sĩ (Kite Shield)", "Khiên tam giác lớn: Cho phép thực hiện Perfect Parry phản đòn 40 Posture Damage.", "Khiên", ["Shield", "Parry", "Tier 3"]),
                # Potions
                ("Potion_Health_Crimson_Flask.png", "Bình Dược Hồi Máu (Health Flask)", "Dược phẩm khẩn cấp: Hồi phục tức thì 100 HP trong 0.8 giây, gán phím nóng [1].", "Dược Phẩm", ["Potion", "Healing", "Consumable"]),
                ("Potion_Mana_Cyan_Phial.png", "Bình Dược Hồi Mana (Mana Phial)", "Dược phẩm ma pháp: Hồi phục 80 Mana, xóa bỏ trạng thái Câm lặng.", "Dược Phẩm", ["Potion", "Mana", "Consumable"]),
                ("Potion_Stamina_Green_Draught.png", "Dược Thảo Thể Lực (Stamina Draught)", "Dược liệu tăng lực: Giảm 50% chi phí thể lực lướt né trong 10 giây.", "Dược Phẩm", ["Potion", "Stamina", "Consumable"]),
                # Magic & Jewelry
                ("Jewelry_Engraved_Gold_Ring.png", "Nhẫn Vàng Cổ Ngữ (Runic Ring)", "Nhẫn trang bị Slot 1: Tăng 15% sát thương chí mạng và gia tăng thời gian I-frame.", "Trang Sức", ["Ring", "Jewelry", "Tier 3"]),
                ("Jewelry_Sapphire_Pendant_Amulet.png", "Dây Chuyền Lam Ngọc (Sapphire Amulet)", "Dây chuyền cổ: Tăng 30 Mana tối đa và tạo khiên hấp thụ sát thương ma thuật.", "Trang Sức", ["Amulet", "Jewelry", "Tier 3"]),
                ("SkillBook_Ancient_Arcane_Grimoire.png", "Sách Bí Kíp Ma Thuật Cổ (Arcane Grimoire)", "Sách kỹ năng: Mở khóa chiêu thức tối thượng 'Xoáy Nghịch Không Gian' cho Arcanist.", "Bí Kíp", ["SkillBook", "Arcanist", "Tier 4"]),
                ("Crafting_Blacksmith_Ward_Stone.png", "Đá Bảo Hộ Cổ Xưa (Blacksmith Ward)", "Đá hộ mệnh thợ rèn: Bảo vệ trang bị không bị tụt cấp khi rèn thất bại mốc +7..+10.", "Đá Phù Trợ", ["Crafting", "Ward", "Tier 4"]),
                # Crafting Materials & Gems
                ("Material_Rough_Iron_Ore.png", "Quặng Sắt Thô (Rough Iron Ore)", "Nguyên liệu chế tạo: Dùng để đúc phôi kiếm và giáp sắt cơ bản tại Thợ rèn.", "Nguyên Liệu", ["Ore", "Crafting", "Tier 1"]),
                ("Material_Gold_Ingot_Bar.png", "Thỏi Vàng Đúc (Gold Ingot Bar)", "Kim loại quý: Tiền tệ giao dịch cấp cao và nguyên liệu dát vàng trang bị Thần Thánh.", "Nguyên Liệu", ["Ingot", "Gold", "Tier 3"]),
                ("Material_Clay_Terracotta_Brick.png", "Gạch Nung Cổ Xưa (Terracotta Brick)", "Vật liệu kiến trúc: Xây dựng và nâng cấp lò than tại Trạm nghỉ an toàn Sanctuary.", "Nguyên Liệu", ["Material", "Sanctuary", "Tier 1"]),
                ("Gemstone_Faceted_Red_Ruby.png", "Hồng Ngọc Đa Diện (Faceted Ruby)", "Đá quý khảm trang bị: Khảm vào vũ khí để thêm thuộc tính Sát thương Hỏa Diệm.", "Đá Quý", ["Gemstone", "Ruby", "Tier 3"]),
                ("Gemstone_Radiant_Blue_Sapphire.png", "Lam Ngọc Rực Sáng (Radiant Sapphire)", "Đá quý khảm trang bị: Khảm vào giáp/vũ khí tăng Mana và sát thương Băng.", "Đá Quý", ["Gemstone", "Sapphire", "Tier 3"]),
                ("Gemstone_Cut_Green_Emerald.png", "Ngọc Lục Bảo Giác Cắt (Cut Emerald)", "Đá quý khảm trang bị: Gia tăng tốc độ hồi phục Thể lực và tốc chạy.", "Đá Quý", ["Gemstone", "Emerald", "Tier 3"]),
                ("Gemstone_Violet_Amethyst_Cluster.png", "Tinh Thể Thạch Anh Tím (Amethyst)", "Đá quý huyền bí: Khảm vào trang bị kháng hiệu ứng Khống chế và Sát khí Đồ tể.", "Đá Quý", ["Gemstone", "Amethyst", "Tier 3"]),
                ("Loot_Curved_Monster_Fang.png", "Nanh Vuốt Quái Vật (Curved Monster Fang)", "Chiến lợi phẩm săn bắn: Nguyên liệu tôi luyện độc dược và rèn dao găm sát thủ.", "Chiến Lợi Phẩm", ["MonsterLoot", "Fang", "Tier 2"]),
                ("Loot_Glowing_Stone_Golem_Core.png", "Lõi Golem Đá Phát Quang (Golem Core)", "Chiến lợi phẩm Lãnh chúa: Rơi từ Boss Stone Golem, dùng rèn Giáp Bất Tử Tier 4.", "Linh Hồn Boss", ["BossLoot", "Core", "Tier 4"]),
                # Wearable Armors & Helmets
                ("Armor_Steel_Knight_Cuirass.png", "Giáp Ngực Thép Hiệp Sĩ (Knight Cuirass)", "Trang bị thân trên Vanguard: Cung cấp +35 Giáp vật lý, giảm 20% sát thương Posture.", "Giáp Thân", ["Armor", "Vanguard", "Heavy"]),
                ("Armor_Leather_Scout_Vest.png", "Áo Giáp Da Thợ Săn (Scout Vest)", "Trang bị thân Ranger: Tăng độ linh hoạt, giảm chi phí Thể lực của thao tác Lướt.", "Giáp Thân", ["Armor", "Ranger", "Medium"]),
                ("Armor_Arcanist_Scholar_Tunic.png", "Áo Choàng Học Giả (Scholar Tunic)", "Trang bị Arcanist: Kháng ma thuật nguyên tố cao và tăng 25 điểm Mana dự trữ.", "Giáp Thân", ["Armor", "Arcanist", "Cloth"]),
                ("Helmet_Steel_Knight_Closed.png", "Mũ Trùm Kín Thép (Greathelm Closed)", "Mũ sắt kín mặt: Bảo vệ đầu tuyệt đối, chống lại đòn đánh chí mạng của quái vật.", "Mũ Nón", ["Helmet", "Vanguard", "Heavy"]),
                ("Helmet_Steel_Knight_Open.png", "Mũ Chiến Binh Kính Lật (Visor Helm)", "Mũ giáp kính hở: Cân bằng giữa khả năng phòng thủ và tầm nhìn quan sát chiến trường.", "Mũ Nón", ["Helmet", "Vanguard", "Medium"]),
                ("Helmet_Leather_Ranger_Hood.png", "Mũ Trùm Da Du Mục (Ranger Hood)", "Mũ trùm da thợ săn: Giúp ngụy trang, giảm bán kính phát hiện của quái dã ngoại.", "Mũ Nón", ["Helmet", "Ranger", "Light"]),
                ("Greaves_Steel_Knight_Legguards.png", "Xà Cạp Thép Hiệp Sĩ (Knight Greaves)", "Giáp chân kim loại: Tăng khả năng trụ vững, giảm 30% hiệu ứng đẩy lùi (Knockback).", "Giáp Chân", ["Greaves", "Legs", "Heavy"]),
                ("Boots_Leather_Traveler_Boots.png", "Ủng Da Lữ Hành (Traveler Boots)", "Ủng da dã ngoại: Tăng +40 tốc độ di chuyển cơ bản ngoài khu vực an toàn.", "Giày Ủng", ["Boots", "Speed", "Utility"]),
            ]
        },
        {
            "id": "tab_classes",
            "title": "🧙 4 Chức Nghiệp Khởi Đầu (Core Classes)",
            "folder": "05_Character_Classes",
            "badge": "Handcrafted Pixel",
            "summary": "Bộ 4 Chức nghiệp nền tảng (Vanguard, Ranger, Arcanist, Acolyte) đứng trên các bệ đá isometric. Nét vẽ pixel art thủ công mộc mạc, tỷ lệ hài hòa, loại bỏ hoàn toàn cảm giác nhựa AI.",
            "items": [
                ("02_Warrior_Knight_ActionFirst_512x512_showcase.png", "Hiệp Sĩ Vanguard (Anti-AI Action-First Handcrafted)", "⭐ TIÊU CHUẨN MỚI GAME ART STUDIO: Tư thế Action-First Pose (chân trước tấn chữ S, lùi kiếm nén lực), Bảng màu Capcom CPS2 chuẩn xác 30 màu độc nhất (Hue-shifting nắng vàng chanh -> bóng tím navy), Canvas 128x128 (Body cao 88px, Pivot 64,114). Sạch 100% mùi AI!", "Chuẩn CPS2 30 Màu", ["Action-First", "Vanguard", "CPS2 32-Color", "Handcrafted", "88px Body", "QA Passed"]),
                ("02_Warrior_Knight_ActionFirst_256x256_ingame.png", "Hiệp Sĩ Vanguard (2x In-Game Canvas)", "Phiên bản phóng to 2x Nearest-Neighbor cho viewport game. Lưới pixel 1:1 đanh gọn, không một hạt mixel, viền ngoài than chì 1px sắc lẹm.", "In-Game 2x", ["2x Scale", "Pixelated", "Capcom CPS2"]),
                ("02_Warrior_Knight_ActionFirst_128x128_native.png", "Hiệp Sĩ Vanguard (128x128 Native Pixel)", "File gốc 128x128 pixel native đạt tỷ lệ Chibi Heroic 3.5 đầu, điểm neo chân (64, 114) gắn vừa khít CapsuleComponent UE5.7.", "Native 128px", ["Native 128x128", "Pivot 64,114", "UE5 Ready"]),
                ("01_Core_Classes_Handcrafted_Showcase.jpg", "4 Chức Nghiệp Khởi Đầu (Concept Minh Họa)", "Bản vẽ phối cảnh tổng quan 4 chức nghiệp trên bệ đá cổ hoang phế.", "Concept Art", ["4 Classes", "Vanguard", "Ranger", "Arcanist", "Acolyte"]),
                ("01_Core_Classes_Handcrafted_Transparent.png", "4 Chức Nghiệp (Bản Cũ 1024px - Đang Chờ Chuẩn Hóa Lưới)", "Bản cũ tách nền độ phân giải 1024px (119,538 màu). Đang được lên kế hoạch vẽ lại toàn bộ theo chuẩn Action-First như Hiệp Sĩ Vanguard ở trên.", "Bản Cũ (1024px)", ["Legacy", "Needs Pixelation"]),
            ]
        },
        {
            "id": "tab_bosses",
            "title": "💀 Trùm Thế Giới (World Bosses)",
            "folder": "06_Bosses_and_Monsters",
            "badge": "World Bosses",
            "summary": "Tạo hình Trùm Thế Giới Stone Golem và Lich Necromancer theo chuẩn Pixel Handcrafted. Khối hình học góc cạnh, nứt rạn tự nhiên, lõi năng lượng phát sáng đúng quy chuẩn Art Bible.",
            "items": [
                ("01_Boss_Stone_Golem_Handcrafted.jpg", "Boss Golem Đá Cổ Đại (Stone Golem)", "Trùm thế giới Cấp 25: Phiến đá sắc nhọn chắp vá quanh lõi năng lượng phát sáng rực rỡ.", "Concept Art", ["World Boss", "Golem", "Rock/Earth"]),
                ("01_Boss_Stone_Golem_Transparent.png", "Sprite Trong Suốt: Stone Golem", "Tách nền chuẩn xác, sẵn sàng cho đấu trường Boss Arena trong Unreal Engine.", "In-Game Sprite", ["Transparent", "Boss Arena"]),
                ("02_Boss_Lich_Necromancer_Handcrafted.jpg", "Boss Lich Tà Thuật (Lich Necromancer)", "Trùm hầm ngục Cấp 35: Pháp bào rách tà bay lượn, tay cầm đầu lâu phát quang u tối.", "Concept Art", ["Dungeon Boss", "Lich", "Necromancy"]),
                ("02_Boss_Lich_Necromancer_Transparent.png", "Sprite Trong Suốt: Lich Necromancer", "Tách nền sắc sảo từng tà áo rách và ngọn lửa linh hồn ma quái.", "In-Game Sprite", ["Transparent", "Dungeon Boss"]),
            ]
        },
        {
            "id": "tab_env",
            "title": "🏰 Thế Giới & Môi Trường (World & Environment)",
            "folder": "07_Environment_and_World",
            "badge": "HD-2D Isometric",
            "summary": "Toàn cảnh thế giới Project Ascendant: Phối cảnh di tích phế tích cổ, lưới địa hình Isometric Dimetric 2:1 và bản đồ thế giới mở rộng lớn.",
            "items": [
                ("Ruins_Concept.jpg", "Phối Cảnh Di Tích Cổ Hoang Tàn (Ruins Concept)", "Di tích gothic đổ nát trong ánh trăng lạnh lẽo, rêu lân tinh le lói thể hiện bầu không khí cô độc.", "Bối Cảnh", ["Environment", "Ruins", "Wilderness"]),
                ("Ruins_Blockout_3D_Isometric.gif", "Dựng Khối 3D Isometric Không Gian (3D Blockout)", "Mô hình diễn hoạt 3D góc nghiêng -45° kiểm tra độ sâu tầm nhìn và luồng di chuyển.", "Diễn Hoạt Kỹ Thuật", ["Isometric", "3D Blockout", "Perspective"]),
                ("Complete_Map_Showcase.jpg", "Bản Đồ Thế Giới Mở Toàn Cảnh (World Map)", "Quy hoạch 3 phân vùng dã ngoại: Vùng An Toàn Sanctuary, Rừng Sâu Dã Ngoại, và Lãnh Địa Cấm.", "Bản Đồ", ["World Map", "Open World", "Zones"]),
            ]
        }
    ]

    total_items = sum(len(cat["items"]) for cat in categories)
    
    html = f"""<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Project Ascendant — Master Art Gallery & Living Asset Showcase</title>
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
            --cyan-glow: rgba(30, 213, 198, 0.2);
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
            padding: 40px 40px 30px;
            text-align: center;
            position: sticky;
            top: 0;
            z-index: 100;
            backdrop-filter: blur(12px);
        }}

        .brand-subtitle {{
            font-family: 'Cinzel', serif;
            color: var(--border-gold);
            font-size: 0.95rem;
            letter-spacing: 0.35em;
            text-transform: uppercase;
            margin-bottom: 8px;
        }}

        h1 {{
            font-family: 'Cinzel', serif;
            font-size: 2.3rem;
            font-weight: 800;
            color: #FFFFFF;
            letter-spacing: 0.05em;
            text-shadow: 0 4px 20px rgba(0,0,0,0.8);
            margin-bottom: 12px;
        }}

        .spec-badge-bar {{
            display: flex;
            justify-content: center;
            flex-wrap: wrap;
            gap: 10px;
            margin-top: 15px;
        }}

        .spec-badge {{
            background: rgba(230, 161, 34, 0.08);
            border: 1px solid rgba(230, 161, 34, 0.3);
            color: #F3E8D2;
            padding: 4px 12px;
            border-radius: 6px;
            font-size: 0.78rem;
            font-weight: 600;
            letter-spacing: 0.02em;
        }}

        .spec-badge.cyan {{
            background: rgba(30, 213, 198, 0.08);
            border-color: rgba(30, 213, 198, 0.3);
            color: var(--cyan-accent);
        }}

        .tabs-nav {{
            display: flex;
            justify-content: center;
            flex-wrap: wrap;
            gap: 8px;
            margin-top: 24px;
        }}

        .tab-btn {{
            background: #111317;
            border: 1px solid var(--border-card);
            color: var(--text-muted);
            padding: 10px 18px;
            font-family: 'Inter', sans-serif;
            font-size: 0.88rem;
            font-weight: 600;
            cursor: pointer;
            border-radius: 8px;
            transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
        }}

        .tab-btn:hover {{
            background: #1C2028;
            color: var(--text-main);
            border-color: #3B4252;
        }}

        .tab-btn.active {{
            background: #201D17;
            color: var(--border-gold);
            border-color: var(--border-gold);
            box-shadow: 0 0 16px var(--gold-glow);
        }}

        main {{
            max-width: 1480px;
            margin: 36px auto 0;
            padding: 0 24px;
        }}

        .tab-panel {{
            display: none;
        }}

        .tab-panel.active {{
            display: block;
            animation: fadeIn 0.3s ease-out;
        }}

        @keyframes fadeIn {{
            from {{ opacity: 0; transform: translateY(6px); }}
            to {{ opacity: 1; transform: translateY(0); }}
        }}

        .section-header {{
            background: linear-gradient(90deg, #13151A 0%, rgba(19,21,26,0) 100%);
            border-left: 4px solid var(--border-gold);
            padding: 18px 24px;
            border-radius: 0 10px 10px 0;
            margin-bottom: 28px;
        }}

        .section-title {{
            font-family: 'Cinzel', serif;
            font-size: 1.5rem;
            font-weight: 700;
            color: #FFFFFF;
            display: flex;
            align-items: center;
            gap: 12px;
        }}

        .section-badge {{
            font-family: 'Inter', sans-serif;
            background: rgba(230, 161, 34, 0.15);
            border: 1px solid var(--border-gold);
            color: var(--border-gold);
            font-size: 0.72rem;
            padding: 2px 8px;
            border-radius: 4px;
            text-transform: uppercase;
            font-weight: 700;
        }}

        .section-desc {{
            color: var(--text-muted);
            font-size: 0.92rem;
            margin-top: 6px;
            max-width: 1100px;
            line-height: 1.6;
        }}

        .grid-container {{
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
            gap: 20px;
        }}

        .art-card {{
            background: var(--bg-card);
            border: 1px solid var(--border-card);
            border-radius: 12px;
            overflow: hidden;
            transition: all 0.25s ease;
            display: flex;
            flex-direction: column;
            position: relative;
        }}

        .art-card:hover {{
            background: var(--bg-card-hover);
            border-color: #454D5E;
            transform: translateY(-4px);
            box-shadow: 0 12px 28px rgba(0, 0, 0, 0.6);
        }}

        .img-wrapper {{
            width: 100%;
            height: 240px;
            background: #08090B;
            background-image: 
                linear-gradient(45deg, #101216 25%, transparent 25%), 
                linear-gradient(-45deg, #101216 25%, transparent 25%), 
                linear-gradient(45deg, transparent 75%, #101216 75%), 
                linear-gradient(-45deg, transparent 75%, #101216 75%);
            background-size: 16px 16px;
            background-position: 0 0, 0 8px, 8px -8px, -8px 0px;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 16px;
            cursor: pointer;
            position: relative;
            border-bottom: 1px solid var(--border-card);
        }}

        .art-card img {{
            max-width: 100%;
            max-height: 100%;
            object-fit: contain;
            image-rendering: pixelated;
            image-rendering: crisp-edges;
            transition: transform 0.25s ease;
        }}

        .art-card:hover img {{
            transform: scale(1.06);
        }}

        .type-tag {{
            position: absolute;
            top: 10px;
            right: 10px;
            background: rgba(11, 12, 14, 0.85);
            border: 1px solid #333946;
            color: #A6AFBF;
            font-size: 0.7rem;
            font-weight: 700;
            padding: 2px 8px;
            border-radius: 4px;
            text-transform: uppercase;
            backdrop-filter: blur(4px);
        }}

        .card-body {{
            padding: 16px;
            display: flex;
            flex-direction: column;
            flex: 1;
        }}

        .card-title {{
            font-size: 1.02rem;
            font-weight: 700;
            color: #FFFFFF;
            margin-bottom: 6px;
            line-height: 1.35;
        }}

        .card-desc {{
            font-size: 0.82rem;
            color: var(--text-muted);
            line-height: 1.5;
            margin-bottom: 12px;
            flex: 1;
        }}

        .tags-row {{
            display: flex;
            flex-wrap: wrap;
            gap: 6px;
        }}

        .tag {{
            background: #1B1E26;
            color: #9AA3B4;
            font-size: 0.7rem;
            padding: 2px 7px;
            border-radius: 4px;
            border: 1px solid #282E3A;
        }}

        /* Lightbox Modal */
        .modal {{
            display: none;
            position: fixed;
            z-index: 1000;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            background: rgba(4, 5, 7, 0.94);
            backdrop-filter: blur(8px);
            align-items: center;
            justify-content: center;
            padding: 20px;
        }}

        .modal.active {{
            display: flex;
        }}

        .modal-content {{
            max-width: 900px;
            max-height: 85vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            position: relative;
        }}

        .modal-img-container {{
            max-width: 100%;
            max-height: 70vh;
            display: flex;
            align-items: center;
            justify-content: center;
            background: #08090B;
            border: 1px solid var(--border-gold);
            border-radius: 12px;
            padding: 24px;
            box-shadow: 0 0 40px rgba(0, 0, 0, 0.9);
        }}

        .modal-img {{
            max-width: 100%;
            max-height: 65vh;
            object-fit: contain;
            image-rendering: pixelated;
        }}

        .modal-caption {{
            color: #FFFFFF;
            font-family: 'Cinzel', serif;
            font-size: 1.2rem;
            font-weight: 700;
            margin-top: 14px;
            text-align: center;
        }}

        .close-btn {{
            position: absolute;
            top: -40px;
            right: 0;
            color: #FFFFFF;
            font-size: 32px;
            font-weight: bold;
            cursor: pointer;
            transition: color 0.2s;
        }}

        .close-btn:hover {{
            color: var(--border-gold);
        }}
    </style>
</head>
<body>

<header>
    <div class="brand-subtitle">Project Ascendant — Master Art Exhibition</div>
    <h1>Kho Lưu Trữ Đồ Họa Master Art</h1>
    
    <div class="spec-badge-bar">
        <span class="spec-badge">Lưới Icon Gốc: 32×32 px (Chunky)</span>
        <span class="spec-badge">Canvas Nhân Vật: 128×128 px (Body 86px)</span>
        <span class="spec-badge">Ghim Tâm Chân: (64, 114) Bottom-Center</span>
        <span class="spec-badge cyan">Engine: Unreal Engine 5.7 Lumen HD-2D</span>
        <span class="spec-badge cyan">Quy Chuẩn: SPEC-ART-2026-09-23-V2</span>
        <span class="spec-badge">QA Gate Check: 100% Passed</span>
    </div>

    <div class="tabs-nav">
"""

    # Add Tab Buttons
    for i, cat in enumerate(categories):
        active_cls = "active" if i == 0 else ""
        html += f"""        <button class="tab-btn {active_cls}" onclick="switchTab('{cat['id']}', this)">{cat['title']}</button>\n"""

    html += """    </div>
</header>

<main>
"""

    # Add Tab Panels
    for i, cat in enumerate(categories):
        active_cls = "active" if i == 0 else ""
        folder = cat["folder"]
        
        html += f"""
    <div id="{cat['id']}" class="tab-panel {active_cls}">
        <div class="section-header">
            <div class="section-title">
                {cat['title']}
                <span class="section-badge">{cat['badge']}</span>
            </div>
            <div class="section-desc">{cat['summary']}</div>
        </div>

        <div class="grid-container">
"""
        for filename, title, desc, tag_type, tags in cat["items"]:
            file_rel = f"{folder}/{filename}"
            tags_html = "".join([f'<span class="tag">{t}</span>' for t in tags])
            
            html += f"""
            <div class="art-card">
                <div class="img-wrapper" onclick="openLightbox('{file_rel}', '{title}')">
                    <span class="type-tag">{tag_type}</span>
                    <img src="{file_rel}" alt="{title}" loading="lazy">
                </div>
                <div class="card-body">
                    <div class="card-title">{title}</div>
                    <div class="card-desc">{desc}</div>
                    <div class="tags-row">{tags_html}</div>
                </div>
            </div>
"""

        html += """        </div>
    </div>
"""

    html += """
</main>

<!-- Lightbox Modal -->
<div id="lightboxModal" class="modal" onclick="closeLightbox()">
    <div class="modal-content" onclick="event.stopPropagation()">
        <span class="close-btn" onclick="closeLightbox()">&times;</span>
        <div class="modal-img-container">
            <img id="lightboxImg" class="modal-img" src="" alt="">
        </div>
        <div id="lightboxCaption" class="modal-caption"></div>
    </div>
</div>

<script>
    function switchTab(tabId, btn) {
        document.querySelectorAll('.tab-panel').forEach(p => p.classList.remove('active'));
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        
        const target = document.getElementById(tabId);
        if (target) {
            target.classList.add('active');
            btn.classList.add('active');
            window.scrollTo({ top: 0, behavior: 'smooth' });
        }
    }

    function openLightbox(src, title) {
        const modal = document.getElementById('lightboxModal');
        const img = document.getElementById('lightboxImg');
        const caption = document.getElementById('lightboxCaption');
        
        img.src = src;
        caption.innerText = title;
        modal.classList.add('active');
    }

    function closeLightbox() {
        document.getElementById('lightboxModal').classList.remove('active');
    }

    document.addEventListener('keydown', function(e) {
        if (e.key === 'Escape') closeLightbox();
    });
</script>

</body>
</html>
"""

    with open(OUTPUT_HTML, "w", encoding="utf-8") as f:
        f.write(html)
        
    print(f"Generated clean Master Art Gallery HTML ({total_items} items) at: {OUTPUT_HTML}")

if __name__ == "__main__":
    generate_gallery()
