# Game Concept: Project Ascendant (Working Title)

*Created: 2026-09-14*  
*Status: In Review — Scope Adjusted to Open World Action MMO (v2)*  
*Lead Designer: Creative Director / Systems Designer*  
*Tech Stack: Unreal Engine 5.7 (GAS, Niagara, Server-Authoritative Dedicated Servers, World Partition)*

---

## 1. Elevator Pitch

> Một tựa game **2.5D Isometric Action RPG MMO Thế Giới Mở (Open World MMO)** hardcore lấy cảm hứng từ các thiên tiểu thuyết LitRPG sống động: người chơi dựa vào timing né đòn (I-frame) và phản xạ kỹ năng để chinh phục các Lãnh chúa vượt cấp trong một thế giới mở cạnh tranh thực tế (**Contested Combat / Open Tagging** — không khóa mục tiêu quái/boss). Nơi đây, ranh giới giữa luật chơi và quy chuẩn đạo đức xã hội hòa quyện làm một: PvP tự do ngoài hoang dã đi kèm **Hệ thống Danh dự & Truy nã (Karma / Wanted System)** khắc nghiệt; người chơi có thể độc hành, kết giao tổ đội săn Boss thế giới, hoặc trở thành những thợ săn tiền thưởng thanh trừng tội đồ máy chủ.

---

## 2. Core Identity

| Tiêu chí | Chi tiết |
| :--- | :--- |
| **Thể loại (Genre)** | 2.5D Isometric Action RPG / Open World MMO / LitRPG Living World |
| **Góc nhìn & Thẩm mỹ** | 2.5D Isometric (như *Hades*), phong cách 3D Stylized / HD-Pixelated với ánh sáng động |
| **Nền tảng (Platform)** | PC (Steam / Epic Games Store), hỗ trợ Controller & Keyboard/Mouse |
| **Động cơ (Engine)** | **Unreal Engine 5.7** (GAS, Niagara, Server-Authoritative Netcode, World Partition) |
| **Số lượng người chơi** | **MMO Thế Giới Mở** (Hàng trăm người chơi đồng thời trên cùng bản đồ thế giới mở phân vùng) |
| **Quy mô Tổ đội (Party)** | Tối đa **4 người chơi / nhóm** (Tối ưu hóa bố cục giao diện và 4 vai trò cốt lõi) |
| **Luật chiến đấu (Combat Rule)** | **Cạnh tranh thực tế (Contested / Open Tagging)**: Không có cơ chế khóa quyền sở hữu quái hay Boss. Mọi người chơi xa lạ đều có thể cùng lúc tấn công 1 mục tiêu. |
| **Quy tắc PvP** | **Open World PvP Tự Do**: Cho phép tấn công người chơi khác ở mọi nơi ngoài các Tòa Thành An Toàn (Citadel Safe Zones / Thành Trì), chịu sự chế tài nghiêm ngặt của Hệ thống Karma. |
| **Thời lượng phiên chơi** | 20–45 phút mỗi chuyến thám hiểm / săn Boss / săn tiền thưởng |
| **Đối tượng mục tiêu** | Người chơi yêu thích *Elden Ring, Hades, Albion Online, V Rising*, độc giả LitRPG (*Solo Leveling, Moonlight Sculptor*), chuộng tính cạnh tranh, xã hội sống động và biểu đạt kỹ năng cao |
| **Tựa game đối trọng** | *Albion Online* (thế giới mở cạnh tranh, không khóa mục tiêu), *Hades* (nhịp độ, camera), *Elden Ring* (tự do vượt cấp, thử thách) |

---

## 3. Core Fantasy & 4 Unique Hooks

### Core Fantasy
> **"Chinh phục điều không thể bằng kỹ năng tuyệt đỉnh trong một thế giới LitRPG chân thực và khắc nghiệt"**  
Một thế giới nơi cấp độ không thể che giấu sự non nớt, và số đông không thể khỏa lấp kỹ năng cá nhân thượng thừa. Một người chơi cấp 10 nắm vững cơ chế né I-frame có thể hạ sát Boss Lãnh chúa cấp 40, lập đội cùng lữ khách qua đường, hoặc trở thành lưỡi đao công lý săn lùng những kẻ tội đồ mang lệnh truy nã đỏ thẫm.

### 4 Unique Hooks (Điểm độc nhất tạo sự khác biệt)

1. **True Skill Expression & Contested Boss Posture (Chiến đấu mở không rào cản):**
   - Không có cơ chế "Miss" hay "Level-gate" ép cày cuốc.
   - **Contested Combat & Open Tagging**: Không có cơ chế khóa quái (No kill-lock).
   - **Thanh Posture Chung & Quyền Kết Liễu (Execution)**: Thanh Posture của Boss là thanh chung có Dynamic Scaling theo số người tham chiến. Người chơi tung đòn bẻ gãy thế đứng cuối cùng (Posture Break Finisher) nhận cửa sổ ưu tiên 1.5s tung đòn Execution rút 25% Max HP.
   - **Instanced Loot có Ngưỡng Đóng Góp (Contribution >= 5%)**: Máy chủ tính toán độc lập; người chơi hoặc tổ đội đạt ngưỡng nhận bảng rơi đồ cá nhân riêng biệt, loại bỏ hoàn toàn nạn đánh hôi (leeching).

2. **Hệ Thống Tổ Đội Hợp Tác 4 Người (4-Player Dynamic Party):**
   - **Chia sẻ EXP & Tiến độ**: EXP chia đều cho thành viên trong bán kính hỗ trợ (2500 cm), không bị phạt giảm EXP khi đi đông người.
   - **Chia Loot Văn Minh**: Kế thừa Instanced Loot độc lập per player—mỗi thành viên đạt ngưỡng nhận hòm đồ của riêng mình, không tranh chấp vật lý, không xúc xắc Need/Greed.
   - **Ý Chí Đồng Đội (Party Morale Buff)**: Nhóm 2 người (+5% EXP, +5% Move Speed ngoài combat), Nhóm 3 người (+10% EXP, +5% Move Speed), Nhóm 4 người (+15% EXP, +10% Move Speed, +5% Max Stamina).

3. **Luật Pháp Thế Giới & Hệ Thống Truy Nã (LitRPG Karma & Bounty System):**
   - **Chỉ số Karma (-100 đến +100)**: Tấn công người chơi vô tội biến tên thành Màu Xám (Aggressor, 120s); hạ sát người vô tội trừ -30 Karma.
   - **Lệnh Truy Nã Đỏ Thẫm (Wanted Tag)**: Karma < -50 kích hoạt trạng thái "Bị Truy Nã Toàn Server" (Red Name), vị trí vùng bị phát sóng công khai trên bản đồ.
   - **Vệ Binh Trục Xuất & Săn Tiền Thưởng (Bounty Hunting)**: NPC Vệ Binh tấn công kẻ bị truy nã ngay khi bước vào cổng trấn, từ chối mọi dịch vụ thợ rèn/thương nhân tiền trạm. Người chơi khác có quyền săn giết kẻ bị truy nã để nhận Vàng thưởng và tăng Karma danh dự.
   - **Phạt Nặng Kẻ Ác & Bảo Vệ Nạn Nhân**:
     * Kẻ Wanted khi bị tiêu diệt: Rơi 100% Vàng, 100% Tàn Trang, có tỷ lệ rơi trang bị đang mặc và bị tống giam vào Ngục Tối 15 phút.
     * Nạn nhân vô tội bị PK: Chỉ mất 25% Vàng (kẻ PK nhận phần này), bảo toàn 100% Tàn Trang & Trang Bị, không bị phạt kép ức chế.
   - **Gột Rửa Tội Ác (Atonement)**: Chuộc tội bằng cách sinh tồn ngoài hoang dã (+2 Karma/10 phút), làm nhiệm vụ chuộc tội tiêu diệt quái cấm địa, hoặc nộp phạt bồi thường tại Chợ Đen.

4. **Kinh Tế Dã Ngoại & Thợ Rèn Phân Tầng Theo Vùng (Zone-tiered Smithing):**
   - Lò rèn dã ngoại phân bố khắp 3 bậc hiểm nguy. Người chơi có thể lập tổ đội hộ tống hoặc lén lút độc hành qua bãi quái cạnh tranh khốc liệt để tiếp cận Lò Rèn Cấm Địa đúc Thần Binh từ Linh Hồn Boss.

---

## 4. Hệ Thống Chức Nghiệp trong Thế Giới MMO (Class Architecture)

> [!NOTE]
> **Đã thay thế bởi `advanced-classes.md` và `DECISIONS.md`**: Cấu trúc chức nghiệp chính thức gồm 16 Class phân bố qua 4 Nhánh (Guard, Scout, Caster, Faith) và 1 Apex Class (God Slayer) cùng cơ chế Dual-Class (Chính/Phụ). Chi tiết đầy đủ và ràng buộc tiến trình xem tại [`advanced-classes.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/gdd/advanced-classes.md) và [`DECISIONS.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/DECISIONS.md).

```
                     ┌───────────────────────────┐
                     │   1 APEX CLASS (BẬC T4)   │
                     │  (Thí Thần Giả - No-Hit)  │
                     └─────────────┬─────────────┘
                                   │
                     ┌─────────────┴─────────────┐
                     │   5 CLASS CAO CẤP (BẬC T3)│
                     │(Di chỉ Lãnh Chúa / Boss)  │
                     └─────────────┬─────────────┘
                                   │
                     ┌─────────────┴─────────────┐
                     │  6 CLASS TRUNG CẤP (BẬC T2│
                     │ (Kỳ ngộ dã ngoại / Tàn tích)│
                     └─────────────┬─────────────┘
                                   │
        ┌──────────────────────────┴──────────────────────────┐
        │              4 CLASS SƠ CẤP (BẬC T1)                │
        │  Vanguard    │   Ranger    │   Arcanist  │  Acolyte │
        │(4 Nhánh cốt lõi: Guard,     Scout,        Caster,    Faith)  │
        └─────────────────────────────────────────────────────┘
```

### Chi tiết 16 Chức Nghiệp:

#### BẬC T1: 4 Class Sơ Cấp (Foundational)
*Có sẵn ngay khi tạo nhân vật. Ưu thế: Cây kỹ năng đa dụng nhất, chi phí nâng cấp rẻ hơn 40%, dễ phối trang bị và mix skill, không bao giờ bị lỗi thời ở endgame.*

| STT | Tên Class | Vũ Khí Đặc Trưng | Phong Cách Chiến Đấu & Kỹ Năng Độc Đáo |
| :---: | :--- | :--- | :--- |
| **1** | **Tiên Phong (Vanguard)** | Kiếm 1 tay & Khiên Sắt Vuông (`1H.Blade`) | **Bậc thầy cận chiến & Đỡ đòn:** Sở hữu kỹ năng **Phản Đòn (Parry)** đòn đánh thường; thể lực (Stamina) dồi dào, khả năng càn lướt và tạo thế đứng vững chắc trước boss. |
| **2** | **Du Hiệp (Ranger)** | Cung Tên / Song Đao (`2H.Bow` / `Dual.Daggers`) | **Cơ động tầm xa & Bẫy:** Nhịp lướt né nhanh nhất game; có khả năng bắn tiễn xuyên thấu làm chậm quái, đặt bẫy chông và găm tên nổ vào điểm yếu của mục tiêu. |
| **3** | **Thuật Sĩ (Arcanist)** | Trượng Ma Pháp (`2H.Staff`) | **Kiểm soát diện rộng & Phép thuật:** Tầm đánh trung bình; các chiêu thức kéo dồn quái, đẩy lùi kẻ địch khi bị áp sát và dồn sát thương nguyên tố phép thuật bùng nổ. |
| **4** | **Tu Sĩ (Acolyte)** | Chùy Chiến / Pháp Bảo (`1H.Mace` / `Relic`) | **Hỗ trợ sinh tồn & Khí công:** Vừa cận chiến vừa tạo khiên ánh sáng; có kỹ năng thanh tẩy độc/chảy máu, hồi phục Stamina cho bản thân và đồng đội xung quanh trong thế giới mở / tổ đội. |

#### BẬC T2: 6 Class Trung Cấp (Advanced)
*Mở khóa qua các chuỗi thử thách dã ngoại, săn quái tinh anh hoặc khám phá tàn tích.*

| STT | Tên Class | Vũ Khí Đặc Trưng | Phong Cách Chiến Đấu & Điều Kiện Mở Khóa |
| :---: | :--- | :--- | :--- |
| **5** | **Cuồng Chiến Sĩ (Berserker)** | Đại Đao 2 tay (`Weapon.2H.Heavy`) | **Liều mạng đổi sát thương:** Máu càng tụt thì tốc độ đánh và sát thương càng tăng; đòn đánh sở hữu *Super Armor* (không bị ngắt chiêu).<br>📍 *Cách mở:* Sống sót qua đấu trường quái tinh anh tại *Hẻm Núi Máu*. |
| **6** | **Thánh Hiệp Sĩ (Templar)** | Chiến Chùy & Đại Thuẫn (`Weapon.1H.Mace`) | **Tanker thuần túy & Phản kích:** Thu hút hận thù quái (Taunt) cực mạnh trong giao tranh đông người; chặn đòn đánh tích năng lượng ánh sáng nổ tung phản sát thương.<br>📍 *Cách mở:* Thanh tẩy lăng mộ thánh địa bị ô uế. |
| **7** | **Kiếm Sư (Swordmaster)** | Kiếm 1 tay (`Weapon.1H.Blade`, không khiên) | **Đoạt mệnh bằng kiếm đạo:** Bỏ khiên để tối ưu tốc độ vung kiếm và phản xạ, kiếm ý sắc bén tạo vết chém xuyên giáp.<br>📍 *Cách mở:* Giải mã tàn tích Kiếm Tông cổ xưa. |
| **8** | **Ảo Ảnh Thích Khách (Shadowblade)** | Song Đoản Kiếm (`Weapon.Dual.Daggers`) | **Ám sát & Tàng hình:** Lướt né để lại phân thân đánh lạc hướng quái; đòn đánh từ sau lưng (Backstab) x3 sát thương bạo kích.<br>📍 *Cách mở:* Tìm thấy căn cứ hội sát thủ ẩn sâu trong hang động đầm lầy. |
| **9** | **Nguyên Tố Sư (Elementalist)** | Trượng Phép / Cầu Phép (`Weapon.2H.Staff`) | **Phản ứng kết hợp:** Luân chuyển 3 nguyên tố Hỏa - Băng - Lôi. Băng làm đông cứng, Hỏa thiêu đốt, Lôi gây nổ giật sét lan sang đàn quái.<br>📍 *Cách mở:* Giải mã câu đố 4 trụ đá nguyên tố ở đền thờ cổ. |
| **10** | **Thẩm Phán (Inquisitor)** | Chiến Chùy 1 tay (`Weapon.1H.Mace`) | **Trừng phạt dị giáo:** Giáng chùy thiêng định tội, thanh trừng tà linh và phá Posture kẻ địch cực mạnh.<br>📍 *Cách mở:* Thanh tẩy tu viện hắc ám bị giáo sĩ tha hóa chiếm đóng. |

#### BẬC T3: 5 Class Cao Cấp (Master)
*Mở khóa sau khi đánh bại các Lãnh Chúa (Boss) hoặc giải mã bí cảnh cấp cao.*

| STT | Tên Class | Vũ Khí Đặc Trưng | Phong Cách Chiến Đấu & Điều Kiện Mở Khóa |
| :---: | :--- | :--- | :--- |
| **11** | **Long Kỵ Sĩ (Dragon Knight)** | Thương Dài & Đại Khí (`2H.Polearm` / `2H.Heavy`) | **Không chiến & Hỏa long:** Kỹ năng phóng vút lên không trung rồi giáng xuống gây sát thương phá thế (Posture) cực mạnh; phun lửa diện rộng.<br>📍 *Cách mở:* Hạ gục và chặt đứt sừng của *Hỏa Long Lãnh Chúa*. |
| **12** | **Hư Không Kiếm Sư (Void Blade)** | Kiếm Hư Không (`Weapon.1H.Blade`) | **Chém xuyên không gian:** Cú lướt biến thành dịch chuyển tức thời; đòn đánh để lại các vết cắt không thời gian nổ sau 1 giây (*Judgement Cut*).<br>📍 *Cách mở:* Đánh bại *Lãnh Chúa Khe Nứt Hư Không*. |
| **13** | **Bóng Ma (Phantom Stalker)** | Song Đoản Kiếm (`Weapon.Dual.Daggers`) | **Ám tiễn & Tử khí:** Thoắt ẩn thoắt hiện trong sương mù độc, dồn sát thương bạo kích từ hư vô.<br>📍 *Cách mở:* Đánh bại *Lãnh Chúa Đầm Lầy Tử Khí*. |
| **14** | **Thời Gian Ma Đạo (Chronomancer)** | Pháp Bảo Thời Gian (`1H.Mace` / `Relic`) | **Thao túng dòng thời gian:** Tạo bong bóng làm chậm chuyển động của quái vật; kỹ năng "Đảo ngược" đưa lượng máu và vị trí bản thân quay lại 3 giây trước.<br>📍 *Cách mở:* Vượt qua thử thách *Tháp Đồng Hồ Vĩnh Hằng*. |
| **15** | **Thiên Sứ (Seraph)** | Pháp Bảo Thánh Quang (`1H.Mace` / `Relic`) | **Hộ trì ánh sáng & Cứu rỗi:** Ban phước lành hồi sinh, thánh thuẫn bảo hộ và ánh sáng thiêu đốt tà ma.<br>📍 *Cách mở:* Đánh bại *Lãnh Chúa Đền Thánh Ánh Sáng*. |

#### BẬC T4: 1 Class Ẩn / Tối Thượng (Apex)
*Tấm huy chương danh dự tối thượng dành riêng cho người chơi có kỹ năng thượng thừa.*

| STT | Tên Class | Vũ Khí Đặc Trưng | Phong Cách Chiến Đấu & Điều Kiện Mở Khóa |
| :---: | :--- | :--- | :--- |
| **16** | **Thí Thần Giả (God Slayer)** | Thần Binh Tự Biến Hình (Morphing Relic) | **Tước đoạt sức mạnh thần thánh:** <br>• Khi thực hiện **Né Hoàn Hảo (Perfect Dodge)**: Kích hoạt hiệu ứng "Ngưng Đọng Thời Gian" 1 giây để phản kích.<br>• Có khả năng sao chép 1 chiêu thức đặc trưng của Boss vừa bị hạ gục.<br>📍 *Cách mở:* **Solo hạ gục một Boss Lãnh chúa vượt ít nhất 15 cấp mà KHÔNG dính bất kỳ một đòn đánh nào (No-Hit Challenge).** |

---

## 5. Hệ Thống Trang Bị & Vòng Lặp Kinh Tế NPC

### Thang Độ Hiếm Trang Bị (5 Bậc Rarity)
*(Tuân thủ `itemization.md` và `DECISIONS.md` Mục 5)*
1. **Common (Trắng):** Chỉ số cơ bản, không có dòng bổ trợ.
2. **Uncommon (Xanh lá):** 1 dòng thuộc tính ngẫu nhiên.
3. **Rare (Xanh lam):** 1–2 dòng thuộc tính ngẫu nhiên (Tốc đánh, giảm tiêu hao thể lực).
4. **Epic (Tím):** 2–3 dòng thuộc tính ngẫu nhiên + tăng cường chỉ số cốt lõi.
5. **Legendary (Cam):** 3 dòng ngẫu nhiên + 1 hiệu ứng đặc thù (Unique Affix).

### Bộ ba NPC Dã ngoại & Trung tâm:
* **Thương Nhân (Merchant):** Cung cấp trang bị tiêu chuẩn (Base gear) và vật phẩm tiêu hao tức thời.
* **Thợ Rèn (Blacksmith - Phân cấp theo vùng):**
  - Vùng sơ cấp (Tier 1): Rèn và cường hóa đồ Common -> Rare.
  - Vùng trung cấp (Tier 2): Mở khóa nâng cấp đồ Epic và đục lỗ khảm Ngọc (Gem Sockets).
  - Vùng cấm địa (Tier 3): Nơi duy nhất đúc Thần Binh Legendary từ Linh Hồn Boss nếu người chơi mang đủ nguyên vật liệu sống sót tới nơi.
* **Dược Sư (Apothecary):**
  - Chế tạo Dầu tẩm vũ khí (Weapon Oils) để gia tăng sát thương phá thế lên từng loại quái vật.
  - Chế thuốc bổ trợ Stamina và dung dịch kháng hiệu ứng rút máu/độc của Boss.

---

## 6. Phân Cấp Quái Vật & Cơ Chế Chiến Đấu

| Phân cấp | Đặc trưng chiến đấu | Quy tắc phần thưởng |
| :--- | :--- | :--- |
| **Quái Thường (Minion)** | Đòn đánh có thời gian chuẩn bị (Wind-up frames) rõ ràng, phục vụ luyện tập timing. | Nguyên liệu thô cơ bản, tiền tệ. |
| **Thủ Lĩnh (Elite)** | Có hào quang cường hóa đàn em, miễn nhiễm choáng nhẹ, tấn công có nhịp trễ (delayed attacks). | Phôi trang bị Rare, ngọc khảm thô. |
| **Lãnh Chúa (Boss)** | Chiến đấu nhiều giai đoạn (Multi-phase), vùng cảnh báo chiêu (AoE telegraphs), có cơ chế nộ (Enrage). | Linh hồn Lãnh chúa (Boss Soul), phôi đúc Epic / Legendary. |
| **Cổ Đại / Thần Thoại (World Boss)** | Đấu trường động, kỹ năng toàn bản đồ, đòi hỏi phối hợp phân chia vai trò trong Co-op. | Nguyên liệu chế tác tối thượng, chứng nhận mở khóa Class Ẩn (God Slayer). |

---

## 7. Kiến Trúc Kỹ Thuật (Unreal Engine 5)

* **Combat Core:** Tận dụng triệt để **Gameplay Ability System (GAS)**:
  - `GameplayAbility`: Mỗi chiêu thức, cú lướt, đòn đánh parry là một Gameplay Ability độc lập.
  - `GameplayTag`: Kiểm soát trạng thái I-frame (`State.Invulnerable`), Trạng thái choáng (`State.Staggered`).
  - `AttributeSet`: Quản lý Máu, Thể lực (Stamina), Thế đứng (Posture), Tốc độ di chuyển.
* **Multiplayer Netcode:**
  - Client-side prediction cho động tác né và xuất chiêu để triệt tiêu cảm giác trễ (delay) khi ping cao.
  - Server Authoritative cho việc phán đoán trúng đòn (Hit registration) và tính toán sát thương.
* **Camera & Rendering:**
  - Camera Isometric cố định góc 45 độ, xoay mượt theo nhân vật.
  - Niagara Particle System xử lý hiệu ứng vệt kiếm, vùng báo động đỏ (Telegraph decaling), và bụi sáng thần bí.

---

## 8. Lộ Trình Triển Khai Tiếp Theo (Next Steps)

1. [ ] **Giai đoạn 1 (Engine Setup):** Cấu hình dự án Unreal Engine 5, tích hợp Gameplay Ability System và thiết lập camera 2.5D Isometric.
2. [ ] **Giai đoạn 2 (Combat Prototype):** Dựng cơ chế di chuyển 8 hướng, Dash I-frame, hệ thống Stagger và 1 Boss thử nghiệm với đòn đánh mẫu.
3. [ ] **Giai đoạn 3 (Class & Ability Pipeline):** Tạo bộ khung 4 Class cơ bản đầu tiên và hệ thống chuyển đổi vũ khí.
4. [ ] **Giai đoạn 4 (Multiplayer & Crafting Loop):** Tích hợp Co-op qua Steam/LAN và hệ thống tương tác Thợ rèn/Thương nhân.
