# Itemization & Equipment System

> **Status**: Approved  
> **Author**: Lead Systems Designer & Lead Technical Architect  
> **Last Updated**: 2026-09-24  
> **Implements Pillar**: Meaningful Progression, True Skill Expression, High-Risk High-Reward Exploration  
> **Target Engine**: Unreal Engine 5.7 (Gameplay Ability System, `FFastArraySerializer`, DataTables)

---

## Overview

Hệ thống Vật Phẩm & Trang Bị (Itemization & Equipment System) là xương sống vận hành cơ chế phát triển sức mạnh nhân vật, vòng lặp phần thưởng (Loot Loop), và cân bằng kinh tế trong Project Ascendant (MMO 2.5D ARPG). Hệ thống kết nối trực tiếp giữa:
1. **Tiến trình cấp độ nhân vật Cấp 1–50** với chỉ số Item Level ($iLvl$).
2. **Cơ chế Thợ Rèn 3 Bậc (3-Tier Forge)**: Lò Rèn Tiền Trạm (Outpost), Lò Rèn Dã Ngoại (Field), và Lò Rèn Cấm Địa (Forbidden).
3. **Kinh tế Song Tiền Tệ (Dual-Currency)**: Vàng (`currency_gold`) kết hợp Tàn Trang Kỹ Năng (`item_skill_shard`).
4. **Kiến trúc đồng bộ mạng hiệu năng cao**: Sử dụng `FFastArraySerializer` của Unreal Engine 5 để tối ưu hóa băng thông delta-replication cho hàng ngàn người chơi trong thế giới mở.

Hệ thống được thiết kế theo triết lý **"Chất lượng đồ họa tối ưu - Biến thiên cơ chế vô tận"**: Giới hạn nghiêm ngặt ngân sách đồ họa ở **7 Weapon Families** và **9–12 Armor Bases**, trong khi chiều sâu gameplay được mở rộng qua hệ thống xúc xắc Affix ngẫu nhiên theo bậc (Procedural Affixes within Bounded Ranges) và cơ chế phủ hiệu ứng hình ảnh động (Palette LUT & Niagara Overlays).

---

## Player Fantasy

*"Bước ra khỏi trận tử chiến nghẹt thở với Lãnh Chúa Hỏa Long ở Cấm Địa Núi Lửa, trên tay bạn là thanh Trảm Kiếm Bậc Legendary vừa nhặt được. Dù mang cùng khung xương đại kiếm bạn từng dùng ở Cấp 15, thanh kiếm giờ đây khoác lên mình hình thái kim loại tôi luyện rực lửa của Lò Rèn Cấm Địa, bao quanh bởi luồng plasma tím-vàng bập bùng.*

*Khi mở bảng thuộc tính, bạn mỉm cười mãn nguyện: các dòng Affix đã lăn trúng chỉ số tối đa (God-roll) về Sát thương Phản đòn (Parry Damage) và Tốc độ Hồi Thể Lực. Bạn lập tức dùng số Tàn Trang Kỹ Năng tích lũy được để đục thêm ô khảm ngọc Prismatic thứ 3. Mỗi món trang bị trong hành trang không chỉ là những con số vô hồn—chúng là minh chứng sống cho kỹ năng sinh tồn và hành trình vượt qua hiểm nguy của chính bạn."*

---

## 1. Hệ Thống 5 Bậc Hiếm (5-Tier Rarity Matrix)

Mọi trang bị trong Project Ascendant được phân loại thành **5 Bậc Hiếm** chuẩn RPG. 

> 🛑 **QUY TẮC MỸ THUẬT CỐT LÕI (Art Budget Rule):**  
> **Độ hiếm (Rarity) KHÔNG sinh ra art vẽ tay riêng biệt!**  
> Khung xương lưới (Mesh / Sprite silhouette) chỉ thay đổi theo **Forge Tier (Bậc Lò Rèn: Tier 1/2/3)** và **Weapon Archetype**. Độ hiếm được thể hiện trực quan $100\%$ qua:
> - Màu khung viền UI & Nền thẻ trang bị (Rarity Border & Card Gradient).
> - Bảng màu chất liệu (Material Palette LUT Swap): Kim loại xỉn $\rightarrow$ Thép sáng $\rightarrow$ Thép rune xanh $\rightarrow$ Tinh thể tím $\rightarrow$ Hoàng kim.
> - Hào quang hạt Niagara In-Game (Particle Glow / Weapon Trail Overlay).

| Bậc Hiếm (Rarity) | Mã Màu Nhận Diện | Quy Tắc Affix (Dòng Thuộc Tính) | Lỗ Khảm Ngọc (Sockets) | Hiệu Ứng Trực Quan (In-Game VFX Overlay) | Hệ Số Giá Bán (Value Mult) |
| :--- | :---: | :--- | :---: | :--- | :---: |
| **Common (Thường)** | `#9CA3AF`<br>(Xám Slate) | **0 Affix**.<br>Chỉ mang chỉ số gốc (Implicit Base Stat) sạch theo $iLvl$. | 0 | Không hào quang, bề mặt kim loại mộc mạc. | $1.0\times$ |
| **Uncommon (Khá)** | `#22C55E`<br>(Xanh Lục) | **1–2 Affixes** (1 Prefix hoặc 1 Suffix).<br>Chỉ số ngẫu nhiên trong dải Tier 1–2. | 0 | Viền sáng xanh ngọc nhẹ trên giao diện, không phát hạt. | $2.5\times$ |
| **Rare (Hiếm)** | `#3B82F6`<br>(Xanh Lam) | **3 Affixes** (2 Prefixes + 1 Suffix).<br>Chỉ số lăn trong dải Tier 2–3. | 1 Lỗ thường<br>*(tại Field Forge)* | Đường viền phát quang lam ngọc, lưỡi vũ khí có ánh sáng mờ. | $6.0\times$ |
| **Epic (Sử Thi)** | `#A855F7`<br>(Tím Bí Ẩn) | **4 Affixes** (2 Prefixes + 2 Suffixes).<br>Mở khóa dòng nội tại nhóm (Family Trait). | 2 Lỗ thường<br>*(tại Field Forge)* | Khói ma thuật tím mỏng uốn lượn quanh thân vũ khí / cầu vai. | $15.0\times$ |
| **Legendary (Huyền Thoại)** | `#F59E0B`<br>(Hoàng Kim) | **4 Affixes (Tier Max)** + **1 Dòng Cải Biến Kỹ Năng Độc Quyền (Class Combat Perk)**. | 2 Lỗ thường + 1 Lỗ Prismatic *(tại Forbidden Forge)* | Hào quang rực lửa vàng kim, hạt tro bụi phát sáng, vệt chém (Trail VFX). | $40.0\times$ |

---

## 2. Cấu Trúc Chỉ Số & Cơ Chế Xúc Xắc Affix (Stat & Affix Engine)

Mỗi món trang bị được cấu thành từ 3 thành phần chỉ số rõ rệt:

```
[MÓN TRANG BỊ] = [1. Chỉ Số Gốc Ẩn (Implicit Stat)] 
               + [2. Tiền Tố (Prefixes - Thiên về Công / Sát Thương)] 
               + [3. Hậu Tố (Suffixes - Thiên về Thủ / Hồi Phục / Cơ Động)] 
               + [4. Dòng Độc Quyền Legendary (Combat Perk - Chỉ có ở Legendary)]
```

### 2.1 Chỉ Số Gốc (Implicit Base Stats)
- **Cố định theo Archetype & Item Level ($iLvl$):** Không bị random. Đảm bảo mọi cây kiếm cùng cấp độ đều có lượng sát thương gốc nền tảng đáng tin cậy.
- **Vũ khí:** Tấn Công Vật Lý (Physical Damage) hoặc Tấn Công Phép (Magic Damage) + Tốc Độ Đánh (Attack Speed cơ sở) + Lực Phá Vỡ Thế Đứng (Base Posture Damage).
- **Giáp (Armor):** Giáp Vật Lý (Physical Armor) + Kháng Ma Pháp (Magic Resistance) + Trọng lượng trang bị (Equipment Load).
- **Trang sức (Accessories):** Bể Mana Tối Đa (Max Mana) hoặc Bể Thể Lực Tối Đa (Max Stamina).

### 2.2 Cơ Chế Procedural Roll Trong Khoảng (Bounded Range Rolling)
Các dòng Affix **KHÔNG** cố định mà được xúc xắc ngẫu nhiên trong một khoảng giá trị được định lượng theo **Affix Tier (Bậc Thuộc Tính T1–T4)** dựa trên $iLvl$ của trang bị:

$$\text{AffixValue} = \text{RandomRound}(\text{MinVal}_{T}, \text{MaxVal}_{T})$$

#### Bảng Phân Bổ Tiền Tố (Prefix Pool — Thiên về Tấn Công):
| Mã Định Danh Prefix | Tên Hiển Thị | T1 ($iLvl$ 1–15) | T2 ($iLvl$ 16–30) | T3 ($iLvl$ 31–45) | T4 ($iLvl$ 46–50) |
| :--- | :--- | :---: | :---: | :---: | :---: |
| `prefix_phys_flat` | Sát Thương Sắt Thép | +3–6 DMG | +8–14 DMG | +18–26 DMG | +32–45 DMG |
| `prefix_phys_pct` | Tàn Bạo Tăng Cường | +4–7% Phys | +8–13% Phys | +15–22% Phys | +25–35% Phys |
| `prefix_elemental_fire` | Hỏa Diệm Tôi Luyện | +4–7 Hỏa | +9–16 Hỏa | +20–30 Hỏa | +35–50 Hỏa |
| `prefix_elemental_ice` | Sương Băng Giá Lạnh | +3–6 Băng | +8–14 Băng | +18–26 Băng | +30–44 Băng |
| `prefix_posture_dmg` | Trọng Lực Phá Khớp | +5–8% Posture | +10–16% Posture | +18–25% Posture | +28–40% Posture |
| `prefix_armor_flat` | Vảy Thép Kiên Cố | +8–15 Giáp | +20–35 Giáp | +45–70 Giáp | +85–120 Giáp |

#### Bảng Phân Bổ Hậu Tố (Suffix Pool — Thiên về Phòng Thủ & Cơ Động):
| Mã Định Danh Suffix | Tên Hiển Thị | T1 ($iLvl$ 1–15) | T2 ($iLvl$ 16–30) | T3 ($iLvl$ 31–45) | T4 ($iLvl$ 46–50) |
| :--- | :--- | :---: | :---: | :---: | :---: |
| `suffix_max_hp` | Sinh Lực Dồi Dào | +15–25 HP | +35–55 HP | +70–110 HP | +130–180 HP |
| `suffix_stamina_regen`| Tật Phong Hồi Thể | +5–8% StaRegen| +10–15% StaRegen| +18–25% StaRegen| +28–38% StaRegen|
| `suffix_parry_window` | Phản Xạ Thần Tốc | +0.01s Parry | +0.02s Parry | +0.03s Parry | +0.04s Parry |
| `suffix_move_speed` | Bước Chân Lữ Hành | +10–18 cm/s | +22–35 cm/s | +40–60 cm/s | +70–95 cm/s |
| `suffix_crit_chance` | Tử Huyệt Chuẩn Xác | +2.0–3.5% Crit | +4.0–6.5% Crit | +7.0–10.0% Crit | +11.0–15.0% Crit|
| `suffix_cooldown_red` | Dòng Chảy Ma Lực | +2–4% CDR | +5–8% CDR | +9–13% CDR | +14–20% CDR |

### 2.3 Dòng Đặc Quyền Huyền Thoại (Legendary Combat Perks)
Chỉ xuất hiện trên trang bị Bậc Legendary. Thay vì chỉ tăng số liệu thuần túy, dòng này can thiệp và biến đổi cơ chế chiêu thức của Gameplay Ability System (GAS):
- **Gươm Lãnh Chúa Hỏa Ngục (Vanguard):** Kỹ năng *Blade Arc* phóng ra một luồng sóng dung nham thiêu đốt mặt đất trong 3 giây.
- **Cung Phong Bão Cổ Thụ (Ranger):** Phát bắn *Piercing Shot* khi xuyên qua kẻ địch thứ 2 sẽ tự động tách thành 3 mũi tên phụ.
- **Pháp Trượng Hư Không Cấm Thuật (Arcanist):** Kỹ năng *Gravity Pull* tăng 40% bán kính hút và làm câm lặng quái nhỏ trong 1.5 giây.
- **Chiến Chùy Thái Dương (Acolyte):** Đòn giáng *Smite* tạo kết giới hồi 10 điểm Thể Lực mỗi giây cho mọi đồng minh đứng trong vòng tròn.

---

## 3. Bản Đồ Phân Bổ 12 Class Vào 7 Weapon Families

Để kiểm soát chặt chẽ ngân sách sản xuất đồ họa (tránh bùng nổ hàng trăm model/sprite vũ khí), toàn bộ **12 Chức Nghiệp** (4 Cơ Bản + 4 Hiếm + 3 Cao Cấp + 1 Ẩn) được phân bổ khoa học vào **7 Dòng Vũ Khí Cơ Sở (7 Weapon Families)**:

```mermaid
graph TD
    subgraph 12_Classes["12 CHỨC NGHIỆP TRONG GAME"]
        C1["Chiến Binh (Vanguard)"]
        C2["Du Hiệp (Ranger)"]
        C3["Thuật Sĩ (Arcanist)"]
        C4["Tu Sĩ (Acolyte)"]
        C5["Cuồng Nộ (Berserker)"]
        C6["Thích Khách (Shadowblade)"]
        C7["Nguyên Tố (Elementalist)"]
        C8["Thánh Hiệp Sĩ (Templar)"]
        C9["Hư Không Kiếm (Void Blade)"]
        C10["Thời Gian (Chronomancer)"]
        C11["Long Kỵ Sĩ (Dragon Knight)"]
        C12["Thần Thí Giả (God Slayer)"]
    end

    subgraph 7_Families["7 WEAPON FAMILIES (NGÂN SÁCH ART TỐI ƯU)"]
        F1["Family 1: 1H Blades (Kiếm 1 Tay)"]
        F2["Family 2: 2H Heavy (Đại Khí Hạng Nặng)"]
        F3["Family 3: Polearms (Thương Dài / Kích)"]
        F4["Family 4: Bows (Cung Tên Xạ Kích)"]
        F5["Family 5: Twin Daggers (Song Đoản Đao)"]
        F6["Family 6: Staves (Trượng Phép)"]
        F7["Family 7: Maces & Relics (Chùy & Pháp Bảo)"]
    end

    C1 --> F1
    C1 --> F2
    C2 --> F4
    C2 --> F5
    C3 --> F6
    C4 --> F7
    C5 --> F2
    C6 --> F5
    C7 --> F6
    C8 --> F1
    C8 --> F7
    C9 --> F1
    C10 --> F7
    C11 --> F3
    C12 --> F1
    C12 --> F3
```

### Bảng Ánh Xạ Chi Tiết 12 Class & Weapon Archetypes:

| Dòng Vũ Khí (Weapon Family) | Archetype Kỹ Thuật | Đặc Điểm Hoạt Ảnh & Hitbox | Các Class Sử Dụng Được | Vũ Khí Phụ (Offhand) Tương Thích |
| :--- | :--- | :--- | :--- | :--- |
| **1. One-Handed Blades** | `Weapon.1H.Blade` | Chém ngang góc $120^\circ$, tầm 220cm, tốc độ 1.1 đòn/s. | **Vanguard, Templar, Void Blade, God Slayer** | Khiên Sắt, Khiên Phản Đòn, hoặc Bỏ trống (Song đấu). |
| **2. Two-Handed Heavy** | `Weapon.2H.Heavy` | Bổ dọc 180cm, quét nặng $160^\circ$, tầm 300cm, tốc độ 0.75 đòn/s, Hyper-Armor. | **Berserker, Vanguard (2H), Dragon Knight** | Khóa cứng 2 tay (`bIsTwoHanded = true`). Không mang khiên. |
| **3. Polearms & Halberds** | `Weapon.2H.Polearm`| Đâm thẳng tầm xa 400cm, quét vòng tròn $360^\circ$ trên không, phá Posture cực lớn. | **Dragon Knight, God Slayer** | Khóa cứng 2 tay. |
| **4. Ranged Bows** | `Weapon.2H.Bow` | Bắn đạn đạo tầm xa 1200–1600cm, sạc lực (Hold to Charge), tốc độ 0.9 phát/s. | **Ranger** | Khóa cứng 2 tay (Tự động nạp ống tên Quiver). |
| **5. Twin Light Blades** | `Weapon.Dual.Daggers`| Đâm chém liên hoàn cực nhanh 2.2 đòn/s, hitbox hẹp 150cm, dồn tích tụ Xuất huyết. | **Shadowblade, Ranger (Sub-set)** | Tay thuận: Đoản đao. Tay nghịch: Dao găm hoặc Phi tiêu. |
| **6. Magic Staves** | `Weapon.2H.Staff` | Phóng quả cầu ma pháp 900cm, giộng trượng kích hoạt AoE, tăng mạnh Spell Damage. | **Arcanist, Elementalist** | Cổ Thư Grimoire hoặc Cầu Phép (Orb) bổ trợ. |
| **7. Blunt Maces & Relics** | `Weapon.1H.Mace` / `Relic` | Đập nện 180cm, gây choáng nhẹ (Stagger), sóng xung kích Holy/Time/Ki. | **Acolyte, Templar, Chronomancer** | Đại Thuẫn (Tower Shield), Tràng Hạt Khí Công, Đồng Hồ Cát. |

---

## 4. Ngân Sách Mỹ Thuật (Art Asset Budget & Progression Rules)

Để tránh tình trạng "vẽ tràn lan không kiểm soát", studio quy định trần giới hạn tài nguyên đồ họa (Hard Caps):

```
TỔNG NGÂN SÁCH MỸ THUẬT TOÀN GAME:
├── VŨ KHÍ: 7 Families × 3 Forge Tiers = 21 Base Weapon Sprites
└── GIÁP TRỤ: 3 Armor Weights × 3 Forge Tiers = 9 Base Sets (+ 3 Class Tabards = 12 Base Sets)
```

### 4.1 Quy Chuẩn Tiến Hóa Ngoại Trang Theo Bậc Thợ Rèn (Forge-Tier Visual Evolution)
Trang bị chỉ thay đổi model/silhouette khi được rèn hoặc nâng cấp qua **3 Cấp Độ Lò Rèn Dã Ngoại**:

1. **Cấp 1: Tiền Trạm (Outpost Forge — $iLvl$ 1–20):**
   - *Phong cách:* Thép thô mộc, da thuộc xỉn màu, lưỡi kiếm mài phẳng cơ bản, vải bố thô, nẹp đinh tán đơn giản. Đậm chất trang bị dã chiến của lính đánh thuê.
2. **Cấp 2: Dã Ngoại (Wilderness Forge — $iLvl$ 21–35):**
   - *Phong cách:* Thép đen tôi luyện (Dark Iron), nẹp ngọc nguyên tố phát quang nhẹ, khớp giáp đa tầng linh hoạt, da thú cường lực có lông thú giữ ấm, hoa văn khắc sâu dọc thân kiếm.
3. **Cấp 3: Cấm Địa (Ancient Forbidden Forge — $iLvl$ 36–50):**
   - *Phong cách:* Tinh thể rèn từ Linh hồn Lãnh chúa (Boss Soul), kim loại hắc thạch vũ trụ, rãnh plasma phát sáng rực rỡ, gai nhọn phong ấn, tà áo choàng hư không uốn lượn.

### 4.2 Ngân Sách 9–12 Bộ Giáp Cơ Sở (Armor Base Matrix)

| Nhóm Giáp (Armor Weight) | Cấp 1: Tiền Trạm (Outpost) | Cấp 2: Dã Ngoại (Field) | Cấp 3: Cấm Địa (Forbidden) | Các Class Thích Ứng Chính |
| :--- | :--- | :--- | :--- | :--- |
| **Hạng Nặng (Heavy Plate)** | `Armor_Heavy_T1`<br>Giáp sắt đúc tấm thô, mũ nồi hở mặt. | `Armor_Heavy_T2`<br>Giáp thép tôi đen, giáp vai sư tử, mũ Greathelm đóng kín. | `Armor_Heavy_T3`<br>Giáp rồng hắc thạch, gai vai nham thạch, hào quang lửa. | Vanguard, Templar, Berserker, Dragon Knight |
| **Hạng Vừa (Medium Leather)**| `Armor_Medium_T1`<br>Áo giáp da bò thô, bao tay da nâu, mũ trùm thợ săn. | `Armor_Medium_T2`<br>Giáp da vảy dã thú khâu chỉ nổi, nẹp giáp cẳng chân thép. | `Armor_Medium_T3`<br>Áo da bóng ma viền lông chồn tuyết, tàn ảnh tàng hình. | Ranger, Shadowblade, God Slayer |
| **Hạng Nhẹ (Light Cloth)** | `Armor_Light_T1`<br>Áo sơ mi vải gai, quần vải thô mộc, thắt lưng dây thừng. | `Armor_Light_T2`<br>Pháp bào nhung xanh thẫm, thêu cổ ngữ lam ngọc, nẹp cổ cao. | `Armor_Light_T3`<br>Pháp phục vũ trụ viền vàng ròng, tà áo chuyển màu thiên hà. | Arcanist, Elementalist, Chronomancer, Acolyte |
| **Phụ Kiện Bổ Trợ (Tabards)**| *Tabard 1: Khăn Choàng Thánh Giá (Acolyte)* | *Tabard 2: Khăn Quàng Du Mục (Ranger)* | *Tabard 3: Cầu Vai Rồng (Dragon Knight)* | Gắn đè lên lớp Chest Armor (Paperdoll Overlay) |

---

## 5. Thang Cấp Độ Vật Phẩm & Mối Liên Kết Lò Rèn ($iLvl$ Scaling 1–50)

Hệ thống liên kết chặt chẽ giữa **Cấp độ người chơi (Level 1–50)**, **Item Level ($iLvl$)**, và **3 Bậc Lò Rèn Dã Ngoại**:

```mermaid
gantt
    title TIẾN TRÌNH ITEM LEVEL & BẬC THỢ RÈN (LEVEL 1-50)
    dateFormat  X
    axisFormat %s
    section Thợ Rèn
    Tier 1 Outpost Forge (An Toàn)       :0, 20
    Tier 2 Field Forge (Nguy Hiểm Cấp 20+) :20, 35
    Tier 3 Forbidden Forge (Cấm Địa Cấp 35+) :35, 50
    section Cấp Độ & Độ Hiếm
    iLvl 1-20 (Common & Rare)             :0, 20
    iLvl 21-35 (Mở Khóa Epic & Legendary) :20, 35
    iLvl 36-50 (Đỉnh Phong Thần Binh)    :35, 50
```

### 5.1 Công Thức Tính Chỉ Số Gốc Theo $iLvl$
Chỉ số gốc tăng tuyến tính theo cấp độ vật phẩm, bảo đảm không bị lạm phát số quá đà (Flat Math Curve):

$$\text{BaseDamage}(iLvl) = \text{ArchetypeDamage}_{\text{Base}} \times \left(1.0 + 0.075 \times (iLvl - 1)\right)$$

$$\text{BaseArmor}(iLvl) = \text{ArchetypeArmor}_{\text{Base}} \times \left(1.0 + 0.080 \times (iLvl - 1)\right)$$

- *Ví dụ:* Một thanh Trường Kiếm (Broadsword 1H) có DMG cơ sở tại Cấp 1 là **20**.
  - Tại $iLvl$ 10 (Tier 1 Outpost): $\text{DMG} = 20 \times (1 + 0.075 \times 9) = \mathbf{33.5}$
  - Tại $iLvl$ 30 (Tier 2 Field): $\text{DMG} = 20 \times (1 + 0.075 \times 29) = \mathbf{63.5}$
  - Tại $iLvl$ 50 (Tier 3 Forbidden): $\text{DMG} = 20 \times (1 + 0.075 \times 49) = \mathbf{93.5}$

### 5.2 Bảng Phân Bổ Cấp Rèn & Dịch Vụ 3 Bậc Lò Rèn

| Bậc Lò Rèn (Forge Tier) | Vị Trí Phân Bố | Ngưỡng $iLvl$ Trang Bị | Giới Hạn Cường Hóa (+N) | Quy Tắc Khảm Ngọc (Gem Sockets) | Tỷ Lệ Rớt Độ Hiếm Tự Nhiên |
| :---: | :--- | :---: | :---: | :--- | :--- |
| **Tier 1: Outpost** | Thành trấn an toàn, Tiền trạm bìa rừng | **$iLvl$ 1–20** | Tối đa **+3** (Tỷ lệ 100%) | Không hỗ trợ đục lỗ. Chỉ sửa chữa & rã sách. | Common: 70%<br>Uncommon: 25%<br>Rare: 5% |
| **Tier 2: Field** | Rừng sâu, Đầm lầy quái Cấp 20+ | **$iLvl$ 21–35** | Tối đa **+6** (Tỷ lệ 70% $\rightarrow$ 50%) | Mở đục tối đa **2 Lỗ Thường** trên đồ Rare/Epic. | Common: 35%<br>Uncommon: 40%<br>Rare: 20%<br>Epic: 5% |
| **Tier 3: Forbidden** | Hang Lãnh Chúa, Núi Lửa Cấp 35+ | **$iLvl$ 36–50** | Đỉnh phong **+10** (Tỷ lệ 40% $\rightarrow$ 15%) | Đục **Lỗ Prismatic thứ 3** & Đúc Boss Soul Forging. | Uncommon: 30%<br>Rare: 45%<br>Epic: 20%<br>Legendary: 5% |

---

## 6. Danh Mục Vị Trí Trang Bị Đầy Đủ (Comprehensive Slot List)

Nhân vật trong Project Ascendant quản lý trang bị qua 4 phân vùng lưu trữ chuyên biệt:

```
TỔNG THỂ KHO VẬT PHẨM & TRANG BỊ
├── 1. Khung Trang Bị Nhân Vật (Paperdoll Slots): 9 Slots
├── 2. Khay Phím Tắt Tiêu Hao (Quickbar Slots): 4 Slots [1] [2] [3] [4]
├── 3. Túi Đồ Dã Ngoại (Backpack Inventory Grid): 30 đến 60 Ô (FastArray)
└── 4. Túi Khoáng Sản & Nguyên Liệu (Virtual Crafting Satchel): Không Giới Hạn Ô, Max 999/Slot
```

### 6.1 Khung Trang Bị Nhân Vật (9 Paperdoll Slots)
Các slot trang bị tác động trực tiếp lên chỉ số GAS và ngoại hình nhân vật theo thời gian thực (`OnPaperdollVisualChanged`):

1. **`Slot_MainHand` (Vũ Khí Chính):**
   - Chấp nhận: Mọi vũ khí 1 tay hoặc 2 tay thuộc class cho phép.
   - *Logic:* Nếu trang bị vũ khí 2 tay (`bIsTwoHanded = true`), `Slot_OffHand` tự động bị khóa và mờ đi (Disabled).
2. **`Slot_OffHand` (Vũ Khí Phụ / Khiên):**
   - Chấp nhận: Khiên sắt, Khiên phản đòn, Dao găm phụ, Cổ thư ma thuật, Tràng hạt khí công.
3. **`Slot_Helmet` (Mũ Giáp / Mũ Trùm):**
   - Hiển thị trên Paperdoll đầu nhân vật (Layer Z-Order: 6).
4. **`Slot_Chest` (Giáp Thân / Áo Choàng):**
   - Cốt lõi xác định hình bóng cơ thể nhân vật (Layer Z-Order: 2).
5. **`Slot_Legs` (Xà Cạp / Quần Chiến):**
   - Hiển thị phần thân dưới (Layer Z-Order: 1).
6. **`Slot_Boots` (Ủng Da / Giày Thép):**
   - Hiển thị bàn chân (Layer Z-Order: 3), cung cấp chỉ số Tốc độ chạy.
7. **`Slot_Amulet` (Dây Chuyền Cổ):**
   - Trang sức không hiển thị model ngoài thế giới; cung cấp chỉ số Kháng nguyên tố và Bể Mana.
8. **`Slot_Ring_1` (Nhẫn Thuận):**
   - Tăng sát thương chí mạng, I-frame lướt né.
9. **`Slot_Ring_2` (Nhẫn Nghịch):**
   - Tăng tốc độ hồi phục Thể lực và giảm tiêu hao Mana.

### 6.2 Khay Phím Tắt Nhanh (Quickbar Slots 1–4)
- Gán phím cứng `[1]`, `[2]`, `[3]`, `[4]` trên bàn phím.
- Chỉ chấp nhận vật phẩm thuộc nhóm **Tiêu Hao (Consumables)**:
  - Phím [1]: Mặc định ưu tiên Bình Dược Hồi Máu (`Potion_Health`).
  - Phím [2]: Mặc định ưu tiên Bình Dược Hồi Mana (`Potion_Mana`).
  - Phím [3]: Bình Thể Lực Tăng Lực (`Potion_Stamina`).
  - Phím [4]: Thánh Thủy / Thuốc Giải Độc / Buff Tấn Công.
- Thời gian thi triển: $0.8\text{s}$, giảm 30% tốc độ chạy trong khi uống.

### 6.3 Túi Đồ Dã Ngoại (Backpack Grid: 30–60 Ô)
- Sử dụng cấu trúc lưới ô vuông chuẩn MMO. Khởi đầu 30 ô, nâng cấp mở rộng qua 3 bậc tại Thợ Rèn (30 $\rightarrow$ 40 $\rightarrow$ 50 $\rightarrow$ 60 ô).
- Lưu trữ: Vũ khí dự phòng, giáp nhặt được, trang sức, sách kỹ năng, phù chú.

### 6.4 Túi Nguyên Liệu Riêng Biệt (Crafting Satchel)
- Để tránh chiếm dụng ô đồ dã ngoại của trang bị, toàn bộ vật phẩm thuộc nhóm `Materials`, `Ores`, `Gemstones` và `BossTrophies` được chuyển tự động vào ngăn túi nguyên liệu ảo chuyên biệt.
- Xếp chồng tối đa **999 cái/ô**, không bao giờ làm nghẽn túi đồ chính khi đi săn.

---

## 7. Kinh Tế Song Tiền Tệ & Vòng Lặp Tiêu Thụ (Dual-Currency Economy)

Nền kinh tế trang bị xoay quanh 2 loại tiền tệ cốt lõi nhằm chống lạm phát và khuyến khích tái chế tài nguyên:

```
LUỒNG TIỀN TỆ & TIÊU THỤ TRONG GAME:
├── VÀNG (currency_gold):
│   ├── Nguồn thu: Giết quái, bán đồ thừa, mở rương thế giới.
│   └── Tiêu thụ: Sửa chữa độ bền trang bị, mua bình dược phẩm, phí cường hóa cơ bản.
│
└── TÀN TRANG KỸ NĂNG (item_skill_shard):
    ├── Nguồn thu: Phân rã Sách Kỹ Năng thừa (Salvaging Skill Books) tại Thợ Rèn.
    └── Tiêu thụ: Tẩy luyện lại dòng Affix (Reforging), nâng cấp Skill Cấp 1-5, đục lỗ khảm ngọc.
```

### 7.1 Quy Định Tỷ Giá Phân Rã Sách Thành Tàn Trang
Khi nhặt được các cuốn Sách Kỹ Năng không thuộc class của mình hoặc đã học tối đa, người chơi đem tới Thợ Rèn để phân rã:
- **Sách Normal (Class cơ bản):** Phân rã nhận **1 Tàn Trang** (`item_skill_shard`).
- **Sách Rare (Class hiếm):** Phân rã nhận **3 Tàn Trang**.
- **Sách Epic (Class cao cấp):** Phân rã nhận **8 Tàn Trang**.
- **Sách Mythic (Class ẩn God Slayer):** Phân rã nhận **25 Tàn Trang**.

### 7.2 Chi Phí Dịch Vụ Rèn Đúc & Tẩy Dòng (Reforge Sink)

| Dịch Vụ Thợ Rèn | Tiêu Hao Vàng (`currency_gold`) | Tiêu Hao Tàn Trang (`item_skill_shard`) | Nguyên Liệu Khoáng Sản Kèm Theo |
| :--- | :---: | :---: | :--- |
| **Cường hóa +1 đến +3** | 300 – 800 Vàng | 0 Shards | 5 Quặng Đồng / Sắt Thô |
| **Cường hóa +4 đến +6** | 1,500 – 3,500 Vàng | 2 – 4 Shards | 5 Quặng Sắt Đen + 2 Tinh Thể |
| **Cường hóa +7 đến +9** | 5,000 – 12,000 Vàng | 8 – 15 Shards | 5 Quặng Hư Không + Đá Bảo Hộ |
| **Cường hóa +10 (Đỉnh phong)**| 25,000 Vàng | 30 Shards | 10 Quặng Hư Không + 1 Linh Hồn Boss |
| **Tẩy lại 1 dòng Affix (Reroll)**| 2,000 Vàng | 5 Shards | 1 Viên Ngọc cùng nguyên tố |
| **Đục Lỗ Ngọc thứ 1 / 2** | 1,000 / 3,000 Vàng | 3 / 8 Shards | 3 Thỏi Vàng Đúc |
| **Đục Lỗ Prismatic thứ 3** | 15,000 Vàng | 20 Shards | 5 Vảy Đuôi Boss + 1 Lõi Golem |

---

## 8. Kiến Trúc Kỹ Thuật Unreal Engine 5 (`FFastArraySerializer` & GAS)

Nhằm đáp ứng yêu cầu đồng bộ mạng mượt mà cho tựa game MMO hàng ngàn người chơi cùng lúc mà không làm nghẽn CPU server, hệ thống kho đồ sử dụng kiến trúc **Fast Array Replication** của Unreal Engine.

### 8.1 Cấu Trúc Dữ Liệu `FPASavedItemInstance`

```cpp
// Source: ProjectAscendant/Source/ProjectAscendant/Public/Inventory/PASerializedItem.h
#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GameplayTagContainer.h"
#include "PASerializedItem.generated.h"

class UPAItemDefinition;

/**
 * Cấu trúc đại diện cho 1 thực thể trang bị/vật phẩm cụ thể trong túi đồ.
 * Kế thừa từ FFastArraySerializerItem để hỗ trợ delta-serialization cực nhanh.
 */
USTRUCT(BlueprintType)
struct FPASavedItemInstance : public FFastArraySerializerItem
{
    GENERATED_BODY()

    UPROPERTY()
    FGuid ItemInstanceID;                    // Mã GUID duy nhất của item

    UPROPERTY()
    TObjectPtr<const UPAItemDefinition> ItemDef; // Con trỏ tới DataAsset gốc

    UPROPERTY()
    int32 SlotIndex = -1;                    // Vị trí trong kho (0..59) hoặc Slot trang bị

    UPROPERTY()
    int32 Quantity = 1;                      // Số lượng chồng (Stack count, max 999)

    UPROPERTY()
    int32 ItemLevel = 1;                     // Cấp độ vật phẩm (iLvl 1..50)

    UPROPERTY()
    int32 EnhancementLevel = 0;              // Mốc cường hóa (+0..+10)

    UPROPERTY()
    int32 Durability = 100;                  // Độ bền hiện tại (0..100)

    UPROPERTY()
    FGameplayTag RarityTag;                  // Rarity.Common -> Rarity.Legendary

    UPROPERTY()
    FGameplayTag ForgeTierTag;               // ForgeTier.Outpost -> ForgeTier.Forbidden

    UPROPERTY()
    TArray<FGameplayTag> Sockets;            // Danh sách ngọc đã khảm vào lỗ

    UPROPERTY()
    TArray<FGameplayTag> RolledAffixTags;    // Danh sách các dòng Affix đã roll

    UPROPERTY()
    TArray<float> RolledAffixValues;         // Giá trị số thực tương ứng của Affix

    UPROPERTY()
    bool bIsLocked = false;                  // Khóa chống bán/phân rã nhầm

    // Callback khi client nhận được thay đổi từ server
    void PreReplicatedRemove(const struct FPAInventoryFastArray& InArraySerializer);
    void PostReplicatedAdd(const struct FPAInventoryFastArray& InArraySerializer);
    void PostReplicatedChange(const struct FPAInventoryFastArray& InArraySerializer);
};

/**
 * Mảng chứa toàn bộ kho đồ, quản lý replication qua FastArraySerializer.
 */
USTRUCT(BlueprintType)
struct FPAInventoryFastArray : public FFastArraySerializer
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FPASavedItemInstance> Items;

    UPROPERTY(NotReplicated)
    TObjectPtr<UActorComponent> OwnerComponent;

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FPASavedItemInstance, FPAInventoryFastArray>(Items, DeltaParms, *this);
    }
};

template<>
struct TStructOpsTypeTraits<FPAInventoryFastArray> : public TStructOpsTypeTraitsBase2<FPAInventoryFastArray>
{
    enum { WithNetDeltaSerializer = true };
};
```

### 8.2 Tích Hợp Gameplay Ability System (GAS)
Khi người chơi mặc một món trang bị vào khung Paperdoll:
1. `UPAPaperdollComponent` phát thanh sự kiện `OnItemEquipped(ItemInstance)`.
2. Hệ thống đọc toàn bộ `RolledAffixTags` và `RolledAffixValues` từ món đồ.
3. Cấp phát động một `UGameplayEffect` tạm thời mang các Modifier tương ứng:
   - Thuộc tính `BaseDamage`: Áp dụng Modifier `Additive` từ Affixes và `Multiply` từ mốc Cường hóa $+N$ ($+5\%$ mỗi cấp).
   - Thuộc tính `PostureDamage`: Áp dụng Modifier từ dòng Affix và Ngọc khảm Ruby.
   - Thẻ Tag Kỹ Năng Độc Quyền (Legendary Perk): Cấp trực tiếp Ability Class (`FGameplayAbilitySpec`) vào `UAbilitySystemComponent` của nhân vật. Khi tháo trang bị ra, hệ thống tự động gỡ bỏ Effect và Ability Spec tương ứng mà không làm gián đoạn trạng thái trận đánh.

---

## 9. Phụ Lục Kỹ Thuật & Cập Nhật Thực Thể (Registry Entities)

Các thực thể mới được đăng ký chính thức vào [`design/registry/entities.yaml`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/registry/entities.yaml):

```yaml
  # ─── ITEMIZATION & WEAPON ARCHETYPES ─────────────────────────────
  - name: weapon_family_1h_blades
    display_name: "Dòng Kiếm 1 Tay (1H Blades)"
    status: active
    source: design/gdd/itemization.md
    referenced_by:
      - design/gdd/itemization.md
      - design/gdd/foundational-classes.md
      - design/gdd/advanced-classes.md
    attributes:
      classes: ["Vanguard", "Templar", "Void Blade", "God Slayer"]
      base_speed: 1.10
      hitbox_arc: 120

  - name: weapon_family_2h_heavy
    display_name: "Dòng Đại Khí Hạng Nặng (2H Heavy)"
    status: active
    source: design/gdd/itemization.md
    referenced_by:
      - design/gdd/itemization.md
      - design/gdd/foundational-classes.md
      - design/gdd/advanced-classes.md
    attributes:
      classes: ["Berserker", "Vanguard", "Dragon Knight"]
      base_speed: 0.75
      hitbox_arc: 160
      hyper_armor: true

  - name: weapon_family_polearms
    display_name: "Dòng Thương & Kích (Polearms)"
    status: active
    source: design/gdd/itemization.md
    referenced_by:
      - design/gdd/itemization.md
      - design/gdd/advanced-classes.md
      - design/gdd/hidden-classes.md
    attributes:
      classes: ["Dragon Knight", "God Slayer"]
      base_speed: 0.85
      range_cm: 400

  - name: weapon_family_bows
    display_name: "Dòng Cung Xạ Kích (Ranged Bows)"
    status: active
    source: design/gdd/itemization.md
    referenced_by:
      - design/gdd/itemization.md
      - design/gdd/foundational-classes.md
    attributes:
      classes: ["Ranger"]
      base_speed: 0.90
      range_cm: 1400

  - name: weapon_family_twin_daggers
    display_name: "Dòng Song Đoản Kiếm (Twin Daggers)"
    status: active
    source: design/gdd/itemization.md
    referenced_by:
      - design/gdd/itemization.md
      - design/gdd/foundational-classes.md
      - design/gdd/advanced-classes.md
    attributes:
      classes: ["Shadowblade", "Ranger"]
      base_speed: 2.20
      crit_multiplier: 1.50

  - name: weapon_family_staves
    display_name: "Dòng Trượng Ma Thuật (Magic Staves)"
    status: active
    source: design/gdd/itemization.md
    referenced_by:
      - design/gdd/itemization.md
      - design/gdd/foundational-classes.md
      - design/gdd/advanced-classes.md
    attributes:
      classes: ["Arcanist", "Elementalist"]
      base_speed: 0.95
      magic_scaling: true

  - name: weapon_family_maces_relics
    display_name: "Dòng Chùy & Pháp Bảo (Maces & Relics)"
    status: active
    source: design/gdd/itemization.md
    referenced_by:
      - design/gdd/itemization.md
      - design/gdd/foundational-classes.md
      - design/gdd/advanced-classes.md
    attributes:
      classes: ["Acolyte", "Templar", "Chronomancer"]
      base_speed: 1.00
      posture_stagger_bonus: 0.25
```

---

## 10. Quy Chuẩn Nhận Diện Thị Giác (Visual Identification Rules)

> **Phụ trách**: Lead Technical Art Director  
> **Tham chiếu**: [`design/art/art-bible.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/art/art-bible.md) (Mục 1, 2, 4) & [`references/anti-ai-craft-guide.md`](file:///home/kenzings/.gemini/config/skills/game-art-studio/references/anti-ai-craft-guide.md)  
> **Mục tiêu**: Thiết lập hệ thống mã màu, hiệu ứng hạt Niagara, và cơ chế chuyển đổi Palette LUT cho 5 Bậc Hiếm (Common $\rightarrow$ Legendary) đảm bảo tính thẩm mỹ HD-2D, chống nhầm lẫn với cơ chế chiến đấu, tối ưu hóa hiệu năng MMO, và hỗ trợ người chơi mù màu (Colorblind Accessibility).

---

### 10.1 Biên Bản Đệ Trình & Phân Xử Xung Đột (Creative Director Conflict-Resolution Protocol)

| Vấn Đề Xung Đột Hệ Thống (Cross-System Conflict) | Hệ Thống Bị Ảnh Hưởng | Rủi Ro Gameplay Trong Thực Tế | Phán Quyết Của Ban Giám Đốc Sáng Tạo (Creative Director Ruling) |
| :--- | :--- | :--- | :--- |
| **Xung đột tín hiệu Đỏ Thẫm giữa Rarity và Wanted PK System** | • `itemization.md` (Độ hiếm trang bị)<br>• `pvp-wanted-system.md` (Đánh dấu đồ tể)<br>• `combat-system.md` (Decal đòn đánh Boss) | Trong combat nhịp độ cao (Isometric $-45^\circ$, 12–14m), nếu trang bị bậc cao dùng màu Đỏ (như Immortal/Mythic ở các game ARPG truyền thống), người chơi sẽ **nhầm lẫn cột sáng loot với hào quang sát khí của kẻ PK (Red-Name PK)** hoặc **vùng báo động đòn quét tử thần của Boss (Telegraph Decal)** $\rightarrow$ Gây hoảng loạn né nhầm hoặc target sai mục tiêu. | 🛑 **QUYẾT ĐỊNH CÁCH LY MÀU ĐỎ (STRICT COLOR QUARANTINE):**<br>1. **Màu Đỏ Thẫm (`#9E1A1A` / `#EF4444`) được phong tỏa độc quyền** cho tín hiệu: Sát khí PK Wanted, Vạch máu quái vật, và Decal cảnh báo đòn Boss.<br>2. **Hệ thống Itemization TUYỆT ĐỐI KHÔNG DÙNG MÀU ĐỎ** cho bất kỳ bậc hiếm nào. Bậc cao nhất (Legendary) được chốt cứng ở sắc **Hoàng Kim Hổ Phách (`#F59E0B` / `#E6A122`)** kết hợp lõi trắng sáng. |

---

### 10.2 Bảng Mã Màu 5 Bậc Hiếm & Giải Pháp Tương Phản 3 Vùng Thế Giới

Để các vật phẩm rơi ngoài thế giới (Loot Drops) và icon giao diện không bị "chìm màu" hay hòa lẫn vào bối cảnh của 3 đại địa khu (Verdant Frontier, Ashen Wilderness, Forbidden Sanctum):

```
BẢNG MÃ MÀU CHUẨN HÓA CHO 5 BẬC HIẾM:
[Common]    #E8ECEB (Sacred Bone White) / #9CA3AF (Slate) ── Tương phản cao trên đá xỉn
[Uncommon]  #10B981 (Electric Emerald)                    ── Tươi sáng, rực rỡ hơn cỏ xanh
[Rare]      #3B82F6 (Cobalt Sapphire)                     ── Sắc xanh dương đậm, tách biệt tuyệt đối
[Epic]      #A855F7 (Astral Violet)                       ── Tím phát quang ma mị
[Legendary] #F59E0B (Solar Amber Gold)                    ── Vàng hổ phách viền plasma cyan
```

#### Ma Trận Xử Lý Tương Phản Đa Bối Cảnh (Multi-Biome Contrast Matrix):

| Bậc Hiếm | Mã Hex Chuẩn | Thử Thách Bối Cảnh (Biome Stress Test) | Giải Pháp Xử Lý Đồ Họa Của Studio (Art Direction Fix) |
| :--- | :---: | :--- | :--- |
| **Common** | `#E8ECEB` | **Ashen Wilderness:** Bụi tro xám và đá đen dễ nuốt chửng màu xám thường `#9CA3AF`. | Không dùng màu xám đục. Dùng **Trắng Xương Khô (`#E8ECEB`)** có độ sáng Value $V \ge 90\%$ kết hợp viền ngoài than chì 1px `#121316`. |
| **Uncommon** | `#10B981` | **Verdant Frontier:** Cỏ cây xanh tươi làm chìm màu xanh lá cây tiêu chuẩn `#22C55E`. | Nâng quang phổ sang **Ngọc Lục Bảo Điện Tử (Electric Mint `#10B981`)** có độ bão hòa cao, pha thêm hạt lân tinh trắng ở tâm icon. |
| **Rare** | `#3B82F6` | An toàn trên cả 3 bối cảnh. | Duy trì sắc xanh **Cobalt Sapphire**, viền đổ bóng xanh navy đậm `#1E3A8A`. |
| **Epic** | `#A855F7` | An toàn trên nền xanh và xám; cần chú ý nền đá tím tha hóa. | Dùng màu **Tím Thạch Anh Sáng (`#A855F7`)**, bổ sung viền phát quang ánh bạc (Silver Specular Edge). |
| **Legendary** | `#F59E0B` | **Forbidden Sanctum:** Dòng dung nham cam-đỏ và đá núi lửa dễ làm lẫn cột sáng vàng cam. | **Hiệu Ứng Viền Lạnh Nghịch Sắc (Chromatic Inversion Rim):** Lõi cột sáng là Hoàng Kim `#F59E0B`, nhưng **viền ngoài (Outer Rim) được bọc luồng hào quang Xanh Thiên Thanh / Tím Cyan (`#818CF8`)** $\rightarrow$ Cắt ngọt qua nền nham thạch nóng bỏng. |

---

### 10.3 Kiểm Soát Hiệu Năng Hạt Niagara Trong Trận Đánh World Boss (Crowded Loot Drop)

Khi 20–50 người chơi hạ gục một World Boss, lượng vật phẩm rơi ra đất cùng lúc có thể đạt từ **150 đến 250 items**. Nếu mỗi item đều sở hữu hệ thống hạt Niagara độc lập kèm Point Light và va chạm vật lý, GPU sẽ bị nghẽn Overdraw dẫn đến tụt tụt khung hình thảm hại.

#### Kiến Trúc Cắt Giảm Ngân Sách Hạt Theo Phân Cấp (Particle Budget Hierarchy):

```
HỆ THỐNG CỘT SÁNG & HẠT RƠI NGOÀI THẾ GIỚI:
├── Common & Uncommon : 0 Hạt Niagara. Decal phẳng 2D dưới đất (Static Unlit Shader).
├── Rare              : 0 Hạt Niagara. Vòng tròn sóng xung kích 2D nhấp nháy mờ (Material Panner).
├── Epic              : Mesh Cột Sáng (Unlit Cylinder) + Tối đa 12 hạt khói tím (GPU Sprites, No Light).
└── Legendary         : Mesh Cột Sáng Đa Tầng + 16 hạt tàn tro vàng kim + 1 Point Light CỤC BỘ (Chỉ sáng cho người sở hữu).
```

1. **Thay thế Particle Beam bằng Emissive Cylinder Mesh:** Cột sáng bốc lên trời của đồ Epic và Legendary **KHÔNG PHẢI** là hạt Niagara bắn liên tục, mà là một **khối trụ 3D (Cylinder Static Mesh)** áp vật liệu cuộn UV phát sáng (Emissive Scrolling Shader). Chi phí render gần như bằng 0.
2. **Cơ Chế Phân Luồng Loot Riêng Tư (Instanced Client-Side Culling):**
   - Server quản lý quyền nhặt đồ.
   - Client của người chơi **CHỈ RENDER** cột sáng và hiệu ứng phát quang cho những món đồ thuộc về chính người chơi đó (Private Instanced Loot).
   - Với những món đồ tự do nhặt chung (Contested FFA Loot), hệ thống gộp toàn bộ hạt vào một `UNiagaraDataChannel` duy nhất chạy chung cho toàn màn chơi.
3. **Cắt Giảm Khoảng Cách (Distance-Based LOD):**
   - Cự ly $> 25\text{m}$: Tắt toàn bộ hạt lơ lửng, chỉ giữ lại icon 2D trên minimap.
   - Cự ly $12\text{m} - 25\text{m}$: Giảm 70% mật độ hạt tàn tro.

---

### 10.4 Thiết Kế Đa Kênh Cho Người Mù Màu (Colorblind Accessibility Framework)

Tuân thủ nghiêm ngặt nguyên tắc của Art Bible: *"Màu sắc không bao giờ đứng đơn độc trong cơ chế gameplay cốt lõi."* Người chơi mắc các chứng mù màu (Protanopia, Deuteranopia, Tritanopia) hoặc nhìn màn hình đen trắng đều nhận diện được độ hiếm nhờ **4 kênh hỗ trợ độc lập**:

```
ĐA KÊNH NHẬN DIỆN VẬT PHẨM:
[Kênh 1: Màu Sắc]    ──> [Kênh 2: Hình Học Pip] ──> [Kênh 3: Ký Hiệu La Mã] ──> [Kênh 4: Âm Thanh Stinger]
```

1. **Hệ Thống Ký Hiệu Hình Học Bất Biến (Geometric Pip Badges):**
   Mỗi thẻ trang bị trên UI và biểu tượng nổi trên đầu vật phẩm rơi dưới đất đều gắn một biểu tượng hình học riêng biệt:
   - **Common (Tier 1):** Hình Tròn Đơn `●` *(Circle)*.
   - **Uncommon (Tier 2):** Hình Quả Trám Đôi `◆` *(Diamond)*.
   - **Rare (Tier 3):** Hình Tam Giác Hướng Lên `▲` *(Chevron Triangle)*.
   - **Epic (Tier 4):** Hình Đa Giác Ngũ Giác `⬟` *(Faceted Pentagon)*.
   - **Legendary (Tier 5):** Vương Miện / Ngôi Sao Thái Dương `★` *(Sunburst Crown)*.
2. **Tiền Tố Chữ Số La Mã Bắt Buộc (Roman Numeral Tag):**
   Tên vật phẩm và Tooltip luôn hiển thị kèm tiền tố cấp bậc: `[I] Kiếm Sắt`, `[II] Thép Mài`, `[III] Băng Tinh`, `[IV] Hư Không Trượng`, `[V] Thần Binh Tối Thượng`.
3. **Âm Thanh Rơi Đồ Phân Tầng (Acoustic Stinger Hierarchy):**
   Mỗi khi trang bị chạm đất, hệ thống phát ra một âm thanh vang đặc trưng (Audio Cue) với tần số cao dần:
   - Common/Uncommon: Tiếng kim loại va chạm đá khô khốc (Wood/Metal click, $250\text{Hz}$).
   - Rare: Hợp âm đơn thanh thoát ($520\text{Hz}$, Sound of Glass/Steel).
   - Epic: Hợp âm vang dội kéo dài $1.5\text{s}$ ($880\text{Hz}$ có tiếng chuông ngân ma thuật).
   - Legendary: Tiếng chuông thánh ca bùng nổ kết hợp tiếng rít thiên thạch xé gió ($1500\text{Hz}$ dội vang toàn đấu trường).

---

### 10.5 Đề Xuất 3 Phương Án Áp Dụng Palette LUT Swap Lên Sprite Nhân Vật

Về câu hỏi: *"Cơ chế LUT Swap áp dụng lên sprite: chỉ đổi màu viền/icon UI, hay đổi cả tint trên sprite equipped ngoài world?"*

Dưới đây là 3 phương án được Art Director phân tích kèm ưu/nhược điểm:

```
SO SÁNH 3 PHƯƠNG ÁN ÁP DỤNG BẢNG MÀU LÊN THỰC THỂ:
├── Phương án 1: Tách Biệt Tuyệt Đối (UI Only) ── An toàn kỹ thuật, bảo toàn silhouette class.
├── Phương án 2: Kép Lưỡng Tầng (Dual-Layer: UI + Weapon Edge Emissive) ── ⭐ ĐỀ XUẤT CHÍNH THỨC.
└── Phương án 3: Phủ Toàn Thân (Global Sprite Tint) ── Phô trương nhưng phá hủy mỹ thuật.
```

#### Bảng Đánh Giá Chi Tiết 3 Phương Án:

| Tiêu Chí Đánh Giá | Phương Án 1: Tách Biệt Tuyệt Đối (UI Only) | Phương Án 2: Kép Lưỡng Tầng (Dual-Layer: UI + Weapon Emissive Edge)<br>⭐ *(Khuyên Dùng)* | Phương Án 3: Phủ Toàn Thân (Global Sprite Tint) |
| :--- | :--- | :--- | :--- |
| **Mô tả kỹ thuật** | • Độ hiếm **chỉ đổi màu viền, card và icon UI**.<br>• Sprite nhân vật và vũ khí ngoài world **giữ nguyên 100% màu vẽ tay theo Forge Tier** (Outpost/Field/Forbidden). | • UI đổi đầy đủ màu viền và card.<br>• **Giáp nhân vật giữ nguyên** theo Forge Tier (bảo vệ nhận diện Class).<br>• **Riêng lưỡi vũ khí (Weapon Blade)** được shader tráo dải màu rãnh sáng (Emissive Runic Fuller) và phát vệt chém (Trail VFX) theo Rarity. | • Toàn bộ sprite nhân vật (cả giáp lẫn người) bị áp một lớp color-tint toàn thân (Xanh lục / Xanh lam / Tím / Vàng) tương ứng với món đồ cao nhất đang mặc. |
| **Ưu điểm** | • Tuyệt đối không làm biến dạng nét vẽ pixel thủ công.<br>• Chi phí shader rẻ nhất, không cần viết custom material cho PaperZD flipbooks.<br>• 0% nguy cơ nhầm lẫn màu với cơ chế Wanted PK. | • **Cân bằng hoàn hảo**: Người chơi thấy vũ khí của mình rực sáng hoàng kim ngoài thế giới để "khoe đồ", trong khi giáp trụ vẫn giữ đúng chất lính đánh thuê u tối.<br>• Nhận diện class từ xa không bị suy giảm.<br>• Không bao giờ nhầm với hào quang đỏ thẫm của Wanted PK. | • Rất nổi bật, nhìn lướt qua từ xa là biết ngay người chơi "đầy mình đồ Legendary". |
| **Nhược điểm** | • Người chơi nhặt được kiếm Legendary đắt giá nhưng cầm ngoài thế giới nhìn vẫn giống cây kiếm Common (nếu cùng Forge Tier), gây giảm hưng phấn sở hữu. | • Cần thiết lập Material Instance động trên PaperZD Flipbook Component của lớp Weapon Sprite (thêm 1 texture mask rãnh lưỡi kiếm). | • **Phá hủy ngôn ngữ mỹ thuật**: Nhân vật trông như "kẹo dẻo đổ màu", làm mất hết chất retro pixel art chân thực.<br>• Xung đột nghiêm trọng với màu phe phái và màu cờ PvP. |
| **Đánh giá của Art Director** | Khả thi cho bản Prototype ban đầu. | 🏆 **KHUYẾN NGHỊ LỰA CHỌN CHO BẢN CHÍNH THỨC (PRODUCTION)**. | ❌ **BÁC BỎ HOÀN TOÀN (REJECTED)**. |

---

### 10.6 Kết Luận & Quyết Định Triển Khai (Actionable Directive)

1. **Chốt Bảng Màu Rarity:** Tuyệt đối cấm sử dụng sắc đỏ. Cố định 5 mã màu: `#E8ECEB` (Trắng Xương), `#10B981` (Ngọc Lục), `#3B82F6` (Lam Ngọc), `#A855F7` (Tím Huyền), và `#F59E0B` (Hoàng Kim viền Cyan).
2. **Chốt Kiến Trúc Niagara:** Toàn bộ cột sáng loot drop ngoài world dùng Emissive Cylinder Mesh + Client-Side Instanced Culling. Zero hạt Niagara cho đồ Common/Uncommon để bảo vệ FPS trong trận đánh World Boss 50 người.
3. **Chốt Áp Dụng Phương Án 2 (Dual-Layer):** Lớp Giáp Paperdoll giữ nguyên màu theo Forge Tier; chỉ áp dụng tráo màu rãnh sáng và vệt chém phát quang lên lớp Vũ Khí (Weapon Layer).
4. **Chốt Bộ Ký Hiệu Hình Học:** Tích hợp bộ biểu tượng hình học (`●`, `◆`, `▲`, `⬟`, `★`) và tiền tố La Mã `[I]` đến `[V]` vào CommonUI Tooltip và Billboard Loot Indicator.

