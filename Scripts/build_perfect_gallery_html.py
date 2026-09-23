#!/usr/bin/env python3
"""
Builds the complete, rich, master HTML gallery with all 38+ individual items,
female/male paperdoll, handcrafted classes & bosses.
"""

import os
import glob

PROJECT_ROOT = "/mnt/Data/Projects/project-games/ProjectAscendant"
GALLERY_DIR = os.path.join(PROJECT_ROOT, "Art_Gallery")

def build_html():
    html_path = os.path.join(GALLERY_DIR, "index.html")
    
    # -------------------------------------------------------------
    # Category Definitions
    # -------------------------------------------------------------
    categories = [
        {
            "id": "tab_female",
            "title": "👩 Nhân Vật Nữ (Female Paperdoll)",
            "folder": "02_Modular_Paperdoll_Female",
            "summary": "Bộ thiết kế nhân vật nữ theo phong cách Stardew Valley & Octopath Traveler: Quần áo vải thô tân thủ, Giáp sắt Vanguard, Đồ da thợ săn và Pháp bào ma thuật (cả bản Concept và Sprite trong suốt).",
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
            "title": "⚔️ Kho Items Riêng Lẻ (38+ Individual Items)",
            "folder": "03_Individual_Items_and_Gear",
            "summary": "Toàn bộ 38+ icon vật phẩm vẽ tay phong cách Stardew Valley & Diablo 2 đã được cắt chuẩn từng pixel (ZERO mất góc), tách nền trong suốt 256x256 sắc nét.",
            "items": [
                # Weapons
                ("Weapon_Flaming_Greatsword.png", "Đại Kiếm Lửa (Flaming Greatsword)", "Đại kiếm rực lửa hỏa diệm: +45 Sát thương vật lý, +20 Sát thương Hỏa (Burn DoT).", "Vũ Khí", ["Greatsword", "Fire", "Tier 4"]),
                ("Weapon_Steel_Broadsword.png", "Đại Kiếm Thép (Steel Broadsword)", "Vũ khí tiêu chuẩn Vanguard: +15 Sát thương vật lý, đòn chém 3-Hit Combo sắc bén.", "Vũ Khí", ["Broadsword", "Vanguard", "Tier 2"]),
                ("Weapon_Frost_Shard_Kris.png", "Dao Găm Băng Tuyết (Frost Kris)", "Dao găm pha lê băng: +18 Sát thương, làm chậm tốc độ đánh của kẻ địch 25%.", "Vũ Khí", ["Dagger", "Ice", "Tier 3"]),
                ("Weapon_Rogue_Hunting_Dagger.png", "Dao Găm Thợ Săn (Hunting Dagger)", "Vũ khí phụ Offhand: Kích hoạt đòn đâm chí mạng từ sau lưng (Backstab +50%).", "Vũ Khí", ["Dagger", "Rogue", "Tier 2"]),
                ("Weapon_Elven_Composite_Bow.png", "Cung Yêu Tinh Khắc Vàng (Elven Bow)", "Cung săn viễn cổ: Tầm bắn xa 2200cm, bắn xuyên giáp 15%.", "Vũ Khí", ["Bow", "Elven", "Tier 4"]),
                ("Weapon_Wooden_Hunting_Bow.png", "Cung Săn Uốn Cong (Hunting Bow)", "Vũ khí viễn chiến Ranger: Tầm bắn 1800cm, bắn Snap Shot sau khi lướt né.", "Vũ Khí", ["Bow", "Ranger", "Tier 2"]),
                ("Weapon_Astral_Archmage_Staff.png", "Trượng Tinh Vân Cổ Đại (Astral Staff)", "Trượng tối thượng Arcanist gắn quả cầu tinh vân vũ trụ: +60 Sát thương ma pháp.", "Vũ Khí", ["Staff", "Astral", "Tier 5"]),
                ("Weapon_Crystal_Wizard_Staff.png", "Trượng Pha Lê Ma Thuật (Wizard Staff)", "Trượng phép học giả: Bắn đạn Arcane Orb giải tỏa giao tranh.", "Vũ Khí", ["Staff", "Arcanist", "Tier 3"]),
                ("Weapon_Holy_War_Mace.png", "Chùy Thánh Chiến (Holy War Mace)", "Vũ khí nghi lễ Acolyte: Tăng sát thương phá thế đứng Posture +30%.", "Vũ Khí", ["Mace", "Acolyte", "Tier 2"]),
                ("Weapon_Spiked_Morningstar_Mace.png", "Chùy Gai Thiết Kích (Morningstar)", "Chùy sắt đính gai nhọn: Phá giáp nặng và gây hiệu ứng Chảy máu.", "Vũ Khí", ["Mace", "Heavy", "Tier 3"]),
                ("Weapon_Ornate_Silver_Rapier.png", "Liễu Kiếm Bạc Quý Tộc (Silver Rapier)", "Thanh kiếm đâm tốc độ cao: Tăng tỷ lệ đòn đánh chính xác và tốc độ tấn công.", "Vũ Khí", ["Rapier", "Agility", "Tier 3"]),
                ("Weapon_Death_Obsidian_Scythe.png", "Lưỡi Hái Tử Thần (Obsidian Scythe)", "Lưỡi hái hắc diện thạch: Đòn quét diện rộng thu hoạch linh hồn kẻ tử trận.", "Vũ Khí", ["Scythe", "Dark", "Tier 4"]),
                # Shields
                ("Shield_Iron_Round_Shield.png", "Khiên Tròn Sắt (Iron Round Shield)", "Trang bị Offhand: Giảm 60% sát thương Posture khi đỡ đòn (Block).", "Khiên", ["Shield", "Block", "Vanguard"]),
                ("Shield_Knight_Heraldic_Kite.png", "Khiên Hiệp Sĩ Gia Huy (Kite Shield)", "Khiên lớn hình diều viền vàng gia tăng cửa sổ căn Perfect Parry.", "Khiên", ["Shield", "Parry", "Knight"]),
                # Potions
                ("Potion_Health_Crimson_Flask.png", "Bình Máu Đỏ (Crimson Health Potion)", "Hồi phục 150 HP trong 0.8s. Giảm 30% tốc độ di chuyển trong lúc uống.", "Tiêu Hao", ["Consumable", "Quickbar 1", "Health"]),
                ("Potion_Mana_Cyan_Phial.png", "Bình Mana Lam (Cyan Mana Phial)", "Hồi phục 120 điểm Mana năng lượng ma thuật.", "Tiêu Hao", ["Consumable", "Quickbar 2", "Mana"]),
                ("Potion_Stamina_Green_Draught.png", "Dược Thể Lực Lục (Stamina Draught)", "Lập tức xóa bỏ trạng thái Kiệt Sức và hồi phục 100% Stamina.", "Tiêu Hao", ["Consumable", "Quickbar 3", "Stamina"]),
                # Jewelry & Magic
                ("Jewelry_Engraved_Gold_Ring.png", "Nhẫn Vàng Cổ Xưa (Engraved Gold Ring)", "Gia tăng tỷ lệ bạo kích Critical Strike +5% và giảm tiêu hao thể lực.", "Trang Sức", ["Ring", "Accessory", "Gold"]),
                ("Jewelry_Sapphire_Pendant_Amulet.png", "Dây Chuyền Lam Ngọc (Sapphire Amulet)", "Tăng lượng Mana tối đa +50 và hồi phục 5 Mana mỗi giây.", "Trang Sức", ["Amulet", "Accessory", "Sapphire"]),
                ("SkillBook_Ancient_Arcane_Grimoire.png", "Sách Bí Kíp: Grimoire Ma Thuật", "Học kỹ năng mới vào Action Deck. Cấm đọc trong lúc đang giao tranh.", "Sách Kỹ Năng", ["Skill Book", "Grimoire", "Magic"]),
                ("Crafting_Blacksmith_Ward_Stone.png", "Đá Hộ Mệnh Thợ Rèn (Ward Stone)", "Bảo vệ vật phẩm không bị vỡ khi cường hóa lên +7 ~ +10 tại Lò Rèn.", "Nguyên Liệu", ["Crafting", "Forge", "Ward Stone"]),
                # Materials & Gems
                ("Material_Rough_Iron_Ore.png", "Quặng Sắt Thô (Rough Iron Ore)", "Nguyên liệu đào khoáng cơ bản dùng để tôi luyện vũ khí và giáp tại Thợ Rèn.", "Khoáng Sản", ["Ore", "Mining", "Material"]),
                ("Material_Gold_Ingot_Bar.png", "Thỏi Vàng Đúc (Gold Ingot)", "Thỏi kim loại quý dùng để chế tác trang sức và giao thương giá trị cao.", "Kim Loại Quý", ["Ingot", "Gold", "Economy"]),
                ("Gemstone_Faceted_Red_Ruby.png", "Hồng Ngọc Cắt Giác (Faceted Ruby)", "Ngọc quý khảm vào vũ khí để gia tăng sát thương Hỏa và chỉ số Tấn công.", "Đá Quý", ["Gemstone", "Ruby", "Socket"]),
                ("Gemstone_Radiant_Blue_Sapphire.png", "Lam Ngọc Tỏa Sáng (Radiant Sapphire)", "Ngọc quý khảm vào trang bị để tăng lượng Mana và Kháng phép thuật.", "Đá Quý", ["Gemstone", "Sapphire", "Socket"]),
                ("Gemstone_Cut_Green_Emerald.png", "Lục Bảo Cắt Cạnh (Cut Emerald)", "Ngọc quý gia tăng tốc độ hồi phục Stamina và né tránh đòn.", "Đá Quý", ["Gemstone", "Emerald", "Socket"]),
                ("Gemstone_Violet_Amethyst_Cluster.png", "Cụm Thạch Anh Tím (Amethyst Cluster)", "Pha lê tím huyền bí tăng sát thương đòn đánh chí mạng (Crit Damage).", "Đá Quý", ["Gemstone", "Amethyst", "Socket"]),
                ("Loot_Curved_Monster_Fang.png", "Nanh Quái Thú Uốn Cong (Monster Fang)", "Chiến lợi phẩm rơi từ quái vật hoang dã dùng chế thuốc độc và mũi tên.", "Chiến Lợi Phẩm", ["Monster Loot", "Crafting"]),
                ("Loot_Glowing_Stone_Golem_Core.png", "Lõi Năng Lượng Golem (Golem Core)", "Chiến lợi phẩm Boss Người Đá rơi ra dùng để rèn Trang Bị Huyền Thoại.", "Boss Loot", ["Boss Soul", "Golem", "Legendary"]),
                ("Material_Clay_Terracotta_Brick.png", "Đá Nung Bùa Chú (Terracotta Brick)", "Vật liệu chịu nhiệt cao dùng để nâng cấp bàn rèn của NPC Blacksmith.", "Nguyên Liệu", ["Forge", "Crafting"]),
                # Armor & Helmets
                ("Armor_Steel_Knight_Cuirass.png", "Giáp Ngực Thép Tấm (Steel Cuirass)", "Body Armor hạng nặng: +40 Giáp phòng ngự, chống đòn chém vật lý.", "Giáp Thân", ["Body Armor", "Heavy"]),
                ("Armor_Leather_Scout_Vest.png", "Áo Da Thợ Săn (Leather Scout Vest)", "Body Armor hạng trung: +25 Giáp, không làm giảm tốc độ di chuyển.", "Giáp Thân", ["Body Armor", "Medium"]),
                ("Armor_Arcanist_Scholar_Tunic.png", "Áo Thêu Học Giả (Arcanist Tunic)", "Body Armor vải phép: +18 Giáp, +45 Kháng nguyên tố hắc ám.", "Giáp Thân", ["Body Armor", "Light"]),
                ("Helmet_Steel_Knight_Closed.png", "Mũ Giáp Hiệp Sĩ (Steel Greathelm)", "Mũ sắt bảo vệ phần đầu: +20 Giáp, chống choáng Stun.", "Mũ Nón", ["Helmet", "Heavy"]),
                ("Helmet_Steel_Knight_Open.png", "Mũ Sắt Mở Kính (Open Visor Helm)", "Mũ giáp tăng tầm nhìn và góc quan sát trong trận địa.", "Mũ Nón", ["Helmet", "Steel"]),
                ("Helmet_Leather_Ranger_Hood.png", "Mũ Trùm Da Du Mục (Ranger Hood)", "Mũ trùm đầu thợ săn ngụy trang trong rừng rậm.", "Mũ Nón", ["Helmet", "Medium"]),
                ("Greaves_Steel_Knight_Legguards.png", "Xà Cạp Thép (Steel Greaves)", "Giáp bảo vệ cẳng chân và đầu gối trong giao tranh cận chiến.", "Trang Bị", ["Greaves", "Steel"]),
                ("Boots_Leather_Traveler_Boots.png", "Ủng Da Lữ Khách (Traveler Boots)", "Ủng da mềm giúp di chuyển êm ái trên địa hình bùn đá.", "Trang Bị", ["Boots", "Leather"]),
            ]
        },
        {
            "id": "tab_classes",
            "title": "🛡️ 4 Class Vẽ Nét Hữu Cơ (Core Classes)",
            "folder": "04_Character_Classes",
            "summary": "Tác phẩm vẽ lại 4 Class Chiến Binh, Thợ Săn, Thuật Sĩ và Tu Sĩ với nét vẽ Pixel sinh động, nét biểu cảm ấm áp tự nhiên giống Stardew Valley.",
            "items": [
                ("01_Core_Classes_Handcrafted_Showcase.jpg", "Bộ 4 Chức Nghiệp: Vanguard, Ranger, Arcanist, Acolyte", "Chiến Binh khiên sư tử, Nữ Thợ Săn mũ trùm xanh, Thuật Sĩ già thông thái và Nữ Tu Sĩ thuần khiết đứng trên gạch đá isometric.", "Tranh Tổng Quan", ["4 Classes", "Isometric", "Handcrafted"]),
                ("01_Core_Classes_Handcrafted_Transparent.png", "Bộ 4 Chức Nghiệp (Tách Nền Trong Suốt)", "Phiên bản trong suốt phục vụ làm banner, chọn nhân vật Character Select.", "Transparent", ["Banner", "UI Ready"]),
            ]
        },
        {
            "id": "tab_bosses",
            "title": "👾 Boss & Quái Vật (Bosses)",
            "folder": "05_Bosses_and_Monsters",
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
            is_icon = cat["id"] == "tab_items"
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
    <title>Project Ascendant — Trung Tâm Trưng Bày Đồ Họa Master Art</title>
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
        <h1>Trung Tâm Trưng Bày Đồ Họa Master Art</h1>
        <p class="lead-text">Tập hợp toàn bộ 60+ tác phẩm Art vẽ tay chuẩn Stardew Valley & Octopath Traveler: Nhân vật Nữ/Nam đa tầng, Kho 38+ vật phẩm riêng lẻ không mất góc, 4 Class cơ bản, Trùm thế giới và Bối cảnh game.</p>
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
    print(f"Generated perfect gallery HTML: {html_path}")

if __name__ == "__main__":
    build_html()
