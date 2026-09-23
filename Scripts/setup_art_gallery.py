#!/usr/bin/env python3
"""
Populates the Art_Gallery directory with all game art assets,
categorized into subfolders, and generates a rich interactive HTML visualizer.
"""

import os
import shutil
import json

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
GALLERY_DIR = os.path.join(PROJECT_ROOT, "Art_Gallery")

ASSET_MAPPINGS = [
    # Category 1: Modular Paperdoll
    {
        "cat_folder": "01_Modular_Paperdoll",
        "cat_title": "Modular Paperdoll (Trang Bị Nhân Vật)",
        "items": [
            {
                "src": "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625/hero_starter_cloth_1790138979243.jpg",
                "dest": "01_Concept_Starter_Cloth.jpg",
                "title": "Nhân Vật Tân Thủ (Starter Linen Cloth)",
                "desc": "Trang phục sơ khai mặc định khi mới vào game: áo sơ mi vải thô mộc mạc và quần nâu sờn rách.",
                "badge": "Concept Art",
                "tags": ["Tier 1", "Starter", "Base Body"]
            },
            {
                "src": "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625/hero_iron_armor_1790139001271.jpg",
                "dest": "02_Concept_Vanguard_Iron_Armor.jpg",
                "title": "Bộ Giáp Sắt Vanguard (Vanguard Steel Plate)",
                "desc": "Giáp thép tấm kiên cố, cầu vai gia cố, đại kiếm thép và khiên tròn bảo vệ mặt trận.",
                "badge": "Concept Art",
                "tags": ["Tier 2", "Vanguard", "Heavy Armor"]
            },
            {
                "src": "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625/hero_leather_ranger_1790139031021.jpg",
                "dest": "03_Concept_Ranger_Leather_Scout.jpg",
                "title": "Áo Da Thợ Săn (Ranger Leather Scout)",
                "desc": "Bộ giáp da thú thuộc nhẹ nhàng, bao tay bảo hộ, cung săn tầm xa và dao găm gài hông.",
                "badge": "Concept Art",
                "tags": ["Tier 2", "Ranger", "Medium Armor"]
            },
            {
                "src": "/home/kenzings/.gemini/antigravity-cli/brain/5d6493a0-c1f8-4516-8f3d-342b9b826625/hero_arcanist_robe_1790139057554.jpg",
                "dest": "04_Concept_Arcanist_Mystic_Robe.jpg",
                "title": "Pháp Bào Thuật Sĩ (Arcanist Mystic Robe)",
                "desc": "Áo choàng nhung xanh thẫm thêu cổ ngữ ma thuật hoàng kim và trượng pha lê dẫn truyền mana.",
                "badge": "Concept Art",
                "tags": ["Tier 3", "Arcanist", "Cloth Robe"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/paperdoll/T_Hero_StarterCloth.png"),
                "dest": "05_Sprite_Starter_Cloth_Transparent.png",
                "title": "Sprite Trong Suốt: Đồ Vải Tân Thủ",
                "desc": "Đã khử phông tím Chroma-Key thành Alpha trong suốt (1024x1024 / 32-bit RGBA) sẵn sàng ghép lớp.",
                "badge": "In-Game Sprite",
                "tags": ["Transparent", "Layer 0", "Base Body"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/paperdoll/T_Hero_IronArmor.png"),
                "dest": "06_Sprite_Iron_Armor_Transparent.png",
                "title": "Sprite Trong Suốt: Giáp Sắt Vanguard",
                "desc": "Sprite tách nền hoàn hảo, căn chuẩn trọng tâm chân đáy để xếp chồng lớp trực tiếp trong Paper2D.",
                "badge": "In-Game Sprite",
                "tags": ["Transparent", "Layer 1", "Chest Armor"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/paperdoll/T_Hero_LeatherRanger.png"),
                "dest": "07_Sprite_Leather_Ranger_Transparent.png",
                "title": "Sprite Trong Suốt: Áo Da Ranger",
                "desc": "Sprite tách nền không viền tím, đồng bộ tỷ lệ cơ thể và góc nhìn isometric 45 độ.",
                "badge": "In-Game Sprite",
                "tags": ["Transparent", "Layer 1", "Chest Armor"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/paperdoll/T_Hero_ArcanistRobe.png"),
                "dest": "08_Sprite_Arcanist_Robe_Transparent.png",
                "title": "Sprite Trong Suốt: Pháp Bào Arcanist",
                "desc": "Sprite trong suốt với hiệu ứng rune phát sáng, khớp từng pixel khi gắn vào nhân vật.",
                "badge": "In-Game Sprite",
                "tags": ["Transparent", "Layer 1", "Chest Armor"]
            }
        ]
    },

    # Category 2: 4 Core Classes
    {
        "cat_folder": "02_Character_Classes",
        "cat_title": "4 Class Cơ Bản (Core Character Classes)",
        "items": [
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/vanguard_pixel_spritesheet.png"),
                "dest": "Vanguard_Pixel_Spritesheet.png",
                "title": "Spritesheet Chiến Binh (Vanguard)",
                "desc": "Tập hợp các hoạt ảnh 8 hướng: Đứng yên, Chạy bộ, Chém đòn 3-Hit Combo, Lướt né và Tử trận.",
                "badge": "Spritesheet",
                "tags": ["Vanguard", "8-Way", "Full Sheet"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/vanguard_idle_animation.gif"),
                "dest": "Vanguard_Idle_Loop.gif",
                "title": "Hoạt Ảnh Đứng Yên: Vanguard",
                "desc": "Vòng lặp animation Idle thở và cầm khiên phòng thủ nhịp nhàng 12 FPS.",
                "badge": "Animated GIF",
                "tags": ["Animation", "Idle", "Loop"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/vanguard/FB_Vanguard_Attack1.gif"),
                "dest": "Vanguard_Anim_Attack.gif",
                "title": "Đòn Đánh Chém 1: Vanguard",
                "desc": "Hoạt ảnh vung đại kiếm chém ngang uy lực với vệt kiếm sáng (Sword Trail) sắc nét.",
                "badge": "Animated GIF",
                "tags": ["Combat", "Attack", "PaperZD"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/vanguard/FB_Vanguard_Dash.gif"),
                "dest": "Vanguard_Anim_Dash.gif",
                "title": "Lướt Né Tránh (Dash I-Frame): Vanguard",
                "desc": "Hoạt ảnh lướt nhanh với bóng mờ ma thuật (Spectral Afterimage) bảo đảm an toàn I-frame 0.25s.",
                "badge": "Animated GIF",
                "tags": ["Combat", "I-Frame", "Dash"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/ranger_pixel_spritesheet.png"),
                "dest": "Ranger_Pixel_Spritesheet.png",
                "title": "Spritesheet Du Mục (Ranger)",
                "desc": "Hoạt ảnh cử động thợ săn viễn chiến: giương cung, bắn tên liên hoàn và lùi né phản kích.",
                "badge": "Spritesheet",
                "tags": ["Ranger", "Bow", "Full Sheet"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/arcanist_pixel_spritesheet.png"),
                "dest": "Arcanist_Pixel_Spritesheet.png",
                "title": "Spritesheet Thuật Sĩ (Arcanist)",
                "desc": "Hoạt ảnh niệm chú, xoay trượng ma thuật và chưởng đạn năng lượng arcane huyền bí.",
                "badge": "Spritesheet",
                "tags": ["Arcanist", "Magic", "Full Sheet"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/acolyte_pixel_spritesheet.png"),
                "dest": "Acolyte_Pixel_Spritesheet.png",
                "title": "Spritesheet Tu Sĩ (Acolyte)",
                "desc": "Hoạt ảnh tu sĩ cầm chùy thánh, cầu nguyện hồi phục và phát tán hào quang bảo hộ đồng đội.",
                "badge": "Spritesheet",
                "tags": ["Acolyte", "Holy", "Full Sheet"]
            }
        ]
    },

    # Category 3: Bosses & Monsters
    {
        "cat_folder": "03_Bosses_and_Monsters",
        "cat_title": "Boss Thế Giới & Quái Vật (Bosses & Enemies)",
        "items": [
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/stone_golem_boss_pixel_spritesheet.png"),
                "dest": "Stone_Golem_Boss_Spritesheet.png",
                "title": "Spritesheet Trùm: Người Đá Cổ Đại (Stone Golem)",
                "desc": "Spritesheet kích thước lớn của Boss Golem với các đòn Đập đất (Ground Slam), Quét đá và trạng thái Vỡ Thế Đứng (Posture Broken).",
                "badge": "Boss Spritesheet",
                "tags": ["Boss", "Stone Golem", "Heavy Enemy"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/boss/FB_Golem_Idle.gif"),
                "dest": "Golem_Anim_Idle.gif",
                "title": "Hoạt Ảnh Boss Đứng Yên: Stone Golem",
                "desc": "Người Đá tỏa sáng lõi năng lượng với nhịp thở đá rung chuyển mặt đất.",
                "badge": "Animated GIF",
                "tags": ["Boss", "Idle", "Animation"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/boss/FB_Golem_Slam.gif"),
                "dest": "Golem_Anim_Slam.gif",
                "title": "Đòn Nện Đất Hủy Diệt (Ground Slam)",
                "desc": "Golem giơ 2 cánh tay đá nện xuống đất kích hoạt vòng cảnh báo sóng chấn động.",
                "badge": "Animated GIF",
                "tags": ["Boss Attack", "Slam", "AoE"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/boss/FB_Golem_Stagger.gif"),
                "dest": "Golem_Anim_Stagger.gif",
                "title": "Trạng Thái Gục Ngã (Posture Broken / Stagger)",
                "desc": "Golem quỵ gối, nứt vỡ lớp giáp ngoài tạo cửa sổ 4.0s cho người chơi áp sát tung đòn Kết Liễu (Finisher).",
                "badge": "Animated GIF",
                "tags": ["Boss Stagger", "Execution Window"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/lich_necromancer_spritesheet.webp"),
                "dest": "Lich_Necromancer_Spritesheet.webp",
                "title": "Spritesheet Pháp Sư Tử Linh (Lich Necromancer)",
                "desc": "Thiết kế trùm phép thuật bóng tối với áo choàng rách bay bổng và linh hồn hư vô vây quanh.",
                "badge": "Spritesheet",
                "tags": ["Undead", "Necromancer", "Boss"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/characters/lich_idle_animation.gif"),
                "dest": "Lich_Idle_Loop.gif",
                "title": "Hoạt Ảnh Lơ Lửng: Lich Necromancer",
                "desc": "Chuyển động lơ lửng ma quái phát sáng hốc mắt xanh hư không.",
                "badge": "Animated GIF",
                "tags": ["Animation", "Loop", "Floating"]
            }
        ]
    },

    # Category 4: Environment & World
    {
        "cat_folder": "04_Environment_and_World",
        "cat_title": "Môi Trường & Thế Giới Mở (Environment & World)",
        "items": [
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/complete_map_showcase.jpg"),
                "dest": "Complete_Map_Showcase.jpg",
                "title": "Toàn Cảnh Bản Đồ Thế Giới Mở (Complete Map Showcase)",
                "desc": "Góc nhìn Isometric tổng quan từ Tiền Đồn Tân Thủ (Tier 1 Verdant Outpost), Bờ thành phế tích qua Vùng Tro Tàn (Tier 2 Ashen Keep).",
                "badge": "Environment Art",
                "tags": ["Isometric", "World Map", "HD-2D"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/ruins_concept.jpg"),
                "dest": "Ruins_Concept.jpg",
                "title": "Bản Vẽ Ý Niệm Phế Tích Cổ (Ancient Ruins Concept)",
                "desc": "Khái niệm kiến trúc gothic cổ kính, bia mộ đổ nát và ánh sáng thể tích mờ ảo giữa rừng rậm.",
                "badge": "Concept Art",
                "tags": ["Ruins", "Architecture", "Atmosphere"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/ruins_blockout_3d.gif"),
                "dest": "Ruins_Blockout_3D_Isometric.gif",
                "title": "Khối Dựng 3D Isometric Xoay Tròn (3D Blockout Demo)",
                "desc": "Animation xoay vòng 360 độ chứng minh chiều sâu hình học 3D thật kết hợp hài hòa với Pixel Art 2D.",
                "badge": "Animated GIF",
                "tags": ["3D Depth", "Blockout", "Isometric"]
            }
        ]
    },

    # Category 5: Items & Icons
    {
        "cat_folder": "05_Items_and_Icons",
        "cat_title": "Vật Phẩm & Biểu Tượng UI (Items & Icons)",
        "items": [
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/weapons/blacksmith_ward_stone.jpg"),
                "dest": "Blacksmith_Ward_Stone_Concept.jpg",
                "title": "Đá Hộ Mệnh Thợ Rèn (Blacksmith Ward Stone Concept)",
                "desc": "Bản vẽ chi tiết bảo vật bảo vệ trang bị không bị vỡ khi cường hóa lên cấp +7 ~ +10 tại Lò Rèn.",
                "badge": "Item Concept",
                "tags": ["Crafting", "Protection", "Forge"]
            },
            {
                "src": os.path.join(PROJECT_ROOT, "Content/art/icons/icon_blacksmith_ward_256x256.png"),
                "dest": "Icon_Blacksmith_Ward_256.png",
                "title": "Icon Vật Phẩm: Đá Hộ Mệnh (256x256 PNG)",
                "desc": "Biểu tượng giao diện sắc nét hiển thị trong túi đồ 30 ô và cửa sổ Lò Rèn Thợ Rèn.",
                "badge": "UI Icon",
                "tags": ["Inventory", "256x256", "Transparent"]
            }
        ]
    }
]

def copy_assets():
    print("=== Copying and categorizing game art assets ===")
    total_copied = 0
    for cat in ASSET_MAPPINGS:
        folder_path = os.path.join(GALLERY_DIR, cat["cat_folder"])
        os.makedirs(folder_path, exist_ok=True)
        for item in cat["items"]:
            src = item["src"]
            dest = os.path.join(folder_path, item["dest"])
            if os.path.exists(src):
                shutil.copy2(src, dest)
                total_copied += 1
                print(f"[{cat['cat_folder']}] Copied: {item['dest']}")
            else:
                print(f"[WARNING] Missing source file: {src}")
    print(f"\nSuccessfully populated {total_copied} assets into {GALLERY_DIR}!")

def generate_html_gallery():
    html_file = os.path.join(GALLERY_DIR, "index.html")
    
    tabs_html = ""
    sections_html = ""
    
    for idx, cat in enumerate(ASSET_MAPPINGS):
        active_class = "active" if idx == 0 else ""
        tab_id = f"tab_{idx}"
        
        tabs_html += f"""
        <button class="nav-tab {active_class}" onclick="switchTab('{tab_id}')">
            {cat['cat_title']}
            <span class="count-badge">{len(cat['items'])}</span>
        </button>
        """
        
        cards_html = ""
        for item in cat["items"]:
            rel_path = f"{cat['cat_folder']}/{item['dest']}"
            tags_html = "".join([f'<span class="tag">{t}</span>' for t in item.get("tags", [])])
            is_gif = item["dest"].endswith(".gif")
            badge_class = "badge-gif" if is_gif else "badge-default"
            
            cards_html += f"""
            <div class="art-card" onclick="openLightbox('{rel_path}', '{item['title']}', '{item['desc']}')">
                <div class="card-media">
                    <img src="{rel_path}" alt="{item['title']}" loading="lazy" />
                    <span class="card-badge {badge_class}">{item['badge']}</span>
                </div>
                <div class="card-info">
                    <h3 class="card-title">{item['title']}</h3>
                    <p class="card-desc">{item['desc']}</p>
                    <div class="card-tags">{tags_html}</div>
                </div>
            </div>
            """
            
        display_style = "block" if idx == 0 else "none"
        sections_html += f"""
        <section id="{tab_id}" class="tab-content" style="display: {display_style};">
            <div class="section-header">
                <h2>{cat['cat_title']}</h2>
                <p class="section-summary">Bao gồm {len(cat['items'])} tài nguyên đồ họa hoàn chỉnh. Bấm vào bất kỳ ảnh nào để xem chi tiết độ phân giải cao.</p>
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
    <title>Project Ascendant — Trung Tâm Xem Xét Art & Đồ Họa (Art Review Gallery)</title>
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
            max-width: 800px;
            margin-left: auto;
            margin-right: auto;
        }}

        /* Navigation Tabs */
        .tabs-bar {{
            display: flex;
            justify-content: center;
            gap: 12px;
            padding: 24px 20px;
            overflow-x: auto;
            background: #0E1014;
            border-bottom: 1px solid var(--border-card);
        }}

        .nav-tab {{
            background: #14171E;
            color: var(--text-muted);
            border: 1px solid var(--border-card);
            padding: 10px 20px;
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

        /* Card Grid */
        .art-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(320px, 1fr));
            gap: 24px;
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
            height: 280px;
            background: repeating-conic-gradient(#15171D 0% 25%, #1B1E26 0% 50%) 50% / 20px 20px;
            display: flex;
            align-items: center;
            justify-content: center;
            overflow: hidden;
        }}

        .card-media img {{
            max-width: 90%;
            max-height: 90%;
            object-fit: contain;
            image-rendering: pixelated;
            transition: transform 0.3s ease;
        }}

        .art-card:hover .card-media img {{
            transform: scale(1.05);
        }}

        .card-badge {{
            position: absolute;
            top: 12px;
            right: 12px;
            font-size: 0.7rem;
            font-weight: 700;
            padding: 4px 10px;
            border-radius: 6px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }}

        .badge-default {{
            background: rgba(19, 21, 26, 0.85);
            color: var(--border-gold);
            border: 1px solid var(--border-gold);
        }}

        .badge-gif {{
            background: rgba(30, 213, 198, 0.2);
            color: var(--cyan-accent);
            border: 1px solid var(--cyan-accent);
        }}

        .card-info {{
            padding: 18px;
            display: flex;
            flex-direction: column;
            flex-grow: 1;
        }}

        .card-title {{
            font-size: 1.05rem;
            font-weight: 700;
            color: #FFFFFF;
            margin-bottom: 6px;
        }}

        .card-desc {{
            color: var(--text-muted);
            font-size: 0.82rem;
            line-height: 1.4;
            margin-bottom: 14px;
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
            font-size: 0.72rem;
            padding: 3px 8px;
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
            background: rgba(5, 6, 8, 0.94);
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
            padding: 20px;
            max-height: 60vh;
        }}

        .lightbox-img-wrap img {{
            max-width: 100%;
            max-height: 55vh;
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
        <h1>Phòng Trưng Bày Đồ Họa & Art Thiết Kế</h1>
        <p class="lead-text">Thư viện tổng hợp toàn bộ các tác phẩm Art, Spritesheet 8 hướng, Hoạt ảnh PaperZD, Sprite trang bị Modular Paperdoll và Bối cảnh thế giới trước khi tiến hành ghép nối vào Unreal Engine.</p>
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

    with open(html_file, "w", encoding="utf-8") as f:
        f.write(full_html)
    print(f"Generated Interactive Art Gallery HTML: {html_file}")

def main():
    copy_assets()
    generate_html_gallery()

if __name__ == "__main__":
    main()
