# PROJECT ASCENDANT -- LOCKED ARCHITECTURAL & DESIGN DECISIONS (DECISIONS.md)

> **Status**: APPROVED & LOCKED  
> **Authority**: System Architect & Project Lead  
> **Last Updated**: 2026-09-28  
> **Scope**: Bắt buộc tuân thủ tuyệt đối trong toàn bộ quá trình phát triển (Giai đoạn 0 → 6). Mọi sửa đổi phải có sự chấp thuận trực tiếp từ Project Lead.

---

## 1. Thuật Ngữ & Phân Định Bậc Chức Nghiệp vs Độ Hiếm

- **Bậc Chức Nghiệp (Class Rank)**:
  - T1: Sơ cấp (Novice / Foundational)
  - T2: Trung cấp (Intermediate / Advanced)
  - T3: Cao cấp (Master / Specialist)
  - T4: Ẩn / Tối thượng (Apex / Mythic)
- **Quy tắc phân lập**: TUYỆT ĐỐI KHÔNG dùng chung chữ "Tier" cho Bậc Chức Nghiệp và Độ Hiếm Vật Phẩm.
  - Chức nghiệp dùng **Bậc (Rank)**: T1, T2, T3, T4.
  - Vật phẩm dùng **Độ Hiếm (Rarity)**.

---

## 2. Cây Chuyển Chức 4 Nhánh & Apex Class (16 Class Hoàn Chỉnh)

Cấu trúc cây chức nghiệp gồm 4 nhánh chính bắt nguồn từ 4 class T1 nền tảng, cùng 1 class Apex đa nhánh:

1. **Nhánh Hộ Vệ (Guard Line)**:
   - **T1**: `Vanguard` (Tiên Phong)
   - **T2**: `Templar` (Thánh Hiệp Sĩ) & `Berserker` (Cuồng Chiến Sĩ)
   - **T3**: `Dragon Knight` (Long Kỵ Sĩ)
2. **Nhánh Du Hiệp (Scout Line)**:
   - **T1**: `Ranger` (Xạ Thủ)
   - **T2**: `Shadowblade` (Ảnh Nhẫn) & `Swordmaster` (Kiếm Sư)
   - **T3**: `Phantom Stalker` (U Hồn Đoạt Mệnh)
3. **Nhánh Pháp Sư (Caster Line)**:
   - **T1**: `Arcanist` (Bí Thuật Sư)
   - **T2**: `Elementalist` (Nguyên Tố Sư) & `Chronomancer` (Thời Gian Pháp Sư)
   - **T3**: `Void Weaver` (Hư Không Dệt Mệnh)
4. **Nhánh Tín Đồ (Faith Line)**:
   - **T1**: `Acolyte` (Tập Sự)
   - **T2**: `Inquisitor` (Thẩm Phán Dị Giáo) & `Oracle` (Nhà Tiên Tri)
   - **T3**: `Seraph` (Thiên Sứ Lục Dực - Phương án 1: chân chạm đất, dùng chung Lower Body Rig, 50 asset mới)
5. **Class Ẩn Tối Thượng (Apex Class)**:
   - **T4**: `God Slayer` (Kẻ Diệt Thần - Yêu cầu hoàn thành kỳ ngộ và đạt T3 từ nhiều nhánh).

---

## 3. Cơ Chế Class Chính & Class Phụ (Dual-Class System)

- Mỗi nhân vật có đúng **1 Class Chính (Primary Class)** và **1 Class Phụ (Secondary Class)**:
  - **Class Chính**: Quyết định ngoại hình (idle stance, crest, tabard, paperdoll chính), vai trò tổ đội cốt lõi, vũ khí chính, và phần lớn Action Deck (3 kỹ năng chủ động + 2 passive).
  - **Class Phụ**: Bổ trợ lối chơi, cung cấp 1 kỹ năng chủ động + 1 passive cho Action Deck.
- **Ràng buộc cấp bậc**:
  - $\text{Bậc Class Phụ} \le \text{Bậc Class Chính}$.
- **Tiến trình & Hoán đổi**:
  - Khi đổi Class Phụ sang một class mới, class mới bắt đầu từ T1. Tiến trình của class cũ được lưu vĩnh viễn trong `class_progression_history` (JSONB), chọn lại không mất.
  - **Quy tắc A1 (Hoán đổi Chính ↔ Phụ)**: Người chơi được phép hoán đổi vị trí Chính ↔ Phụ (nếu class phụ đã đủ điều kiện làm chính) để nhận 100% Class EXP từ chiến đấu nhằm cày tiến trình nhanh hơn.

---

## 4. Hệ Thống Cấp Độ Song Song (Dual-Track Progression)

Hệ thống tiến trình phân tách thành 2 trục song song:
1. **Character Level (1–50)**:
   - Tích lũy qua mọi hoạt động (quái, boss, quest).
   - Tăng Base Stats (Max HP, Max Mana, Base Attack/Defense).
   - Mở khóa slot trang bị và ngưỡng sử dụng item.
2. **Class Level (1–20)**:
   - Mỗi class có thanh EXP riêng.
   - Khi chiến đấu: Class Chính nhận 70% Class EXP, Class Phụ nhận 30% Class EXP.
   - Thăng cấp Class Level mở khóa kỹ năng nội tại (passives) và các thẻ kỹ năng trong Action Deck.

---

## 5. Thang Độ Hiếm Phân Lập (Two Independent Rarity Scales)

Tuyệt đối tuân thủ 2 thang độ hiếm riêng biệt:

1. **Thang Độ Hiếm Trang Bị (5 Bậc)** - Theo `itemization.md`:
   - `Common` (Trắng)
   - `Uncommon` (Xanh lá)
   - `Rare` (Xanh dương)
   - `Epic` (Tím)
   - `Legendary` (Cam)
   *(Giữ nguyên số lượng affix, màu sắc và thuộc tính đã quy định trong GDD)*.

2. **Thang Độ Hiếm Kỹ Năng & Quyển Trục (4 Bậc)** - Theo `skill-progression-system.md`:
   - `Normal` (Trắng - Sách T1)
   - `Rare` (Xanh dương - Quyển Trục T2)
   - `Epic` (Tím - Quyển Trục T3)
   - `Mythic` (Đỏ ánh kim - Quyển Trục T4)
   *(KHÔNG thêm Legendary hay Divine vào thang kỹ năng/quyển trục)*.

3. **Tiền Tệ Phân Rã Quyển Trục**:
   - Không tạo tiền tệ mới "Ash Shards".
   - Mọi quyển trục dư thừa/không dùng phân rã thành **Tàn Trang (Skill Shards - `item_skill_shard`)** hiện có trong GDD để hợp thành hoặc trao đổi.

---

## 6. Quy Tắc Kinh Tế & Giao Dịch Quyển Trục

- **Giai đoạn MVP / Pre-Production / Alpha**:
  - Không cho phép giao dịch P2P trực tiếp đối với Quyển Trục Thăng Chức.
  - Bảo vệ cảm giác thành tựu tiến trình và chống botting nông trại sơ khai.
  - Người chơi phân rã quyển trục không dùng thành Tàn Trang (`item_skill_shard`) và đổi lấy quyển trục mong muốn tại NPC Học Giả (Scholar).
- **Giai đoạn Post-MVP / Commercial**:
  - Kích hoạt giao dịch Quyển Trục thông qua Nhà Đấu Giá / Sàn Giao Dịch (Auction House) khi cơ chế kiểm soát lạm phát và chống gian lận kinh tế hoàn thiện.

---

## 7. Quy Chuẩn Vũ Khí & Trang Bị Theo Class

Thống nhất dứt điểm mọi mâu thuẫn giữa các file GDD cũ:
- **Vanguard**: Khiên Sắt Vuông (`Item.Shield.Square`) + Kiếm 1 tay (`Weapon.1H.Blade`).
- **Templar**: Đại Thuẫn / Khiên Tháp (`Item.Shield.Tower`) + Chùy 1 tay (`Weapon.1H.Mace`).
- **Inquisitor**: Chùy 1 tay (`Weapon.1H.Mace`). Bỏ hoàn toàn roi xích.
- **Berserker**: Vũ khí 2 tay hạng nặng (`Weapon.2H.Heavy` - Đại Đao).
- **Swordmaster**: Kiếm 1 tay (`Weapon.1H.Blade`), không dùng khiên. Tay trái tự do (Paperdoll tách biệt `Layer_Arms/Hands` khỏi `Layer_Equipment_Offhand`).

---

## 8. Quy Chuẩn GameplayTags

- Cấu trúc Tag Class chuẩn:
  `Class.Line.<Nhánh>.<Class>`  
  *(Ví dụ: `Class.Line.Guard.Vanguard`, `Class.Line.Guard.DragonKnight`, `Class.Line.Scout.Swordmaster`)*.
- Điều kiện chuyển chức không hardcode trong Tag mà nằm trong Data Asset của quyển trục (`AllowedSourceClassTags`, `RequiredMinRank`).
- Bỏ cụm tag phân nhánh cấp độ thô sơ cũ.

---

## 9. Cơ Sở Dữ Liệu & Giao Dịch Chống Dupe (Database Schema & Anti-Dupe)

- **Bảng `items` Độc Lập**:
  Mọi vật phẩm (kể cả trong hòm đồ chung `SHARED_STASH`) được lưu trữ ở bảng `items` riêng biệt:
  ```sql
  CONSTRAINT uk_owner_slot UNIQUE (owner_type, owner_id, slot_type, slot_index) DEFERRABLE INITIALLY DEFERRED
  ```
- **Giao Dịch Thăng Chức Nguyên Tử (Atomic Promotion Transaction)**:
  Dedicated Server bắt buộc thực hiện khóa dòng bằng `SELECT ... FOR UPDATE`, đối soát `item_def_id` đúng là quyển trục và người chơi đủ điều kiện. Nếu sai lập tức `ROLLBACK`. Chỉ `COMMIT` khi cập nhật slot và class đồng thời thành công.

---

## 10. Tiêu Chuẩn Art Gate & Ngoại Lệ Boss Spine

- Phong cách chủ đạo: **Action-First HD-2D**, kết hợp Paper2D/PaperZD cho nhân vật và Spine 4.3 cho boss.
- **Ngoại Lệ Art Gate**: Boss Stone Golem dùng Spine 4.3 được chấp nhận có rotation artifacts (nội suy xoay khớp xương), không bị cổng `pixel-review` đánh rớt.
- **Vanguard Art Direction**: Chọn Phương án A (Iron Bastion) - Giáp trụ hiệp sĩ gothic nặng, khiên sắt vuông, phong thái kiên cường vững chãi.
