# PROJECT ASCENDANT -- LOCKED ARCHITECTURAL & DESIGN DECISIONS (DECISIONS.md)

> **Status**: APPROVED & LOCKED  
> **Authority**: System Architect & Project Lead  
> **Last Updated**: 2026-10-10 (bổ sung §13)  
> **Scope**: Bắt buộc tuân thủ tuyệt đối trong toàn bộ quá trình phát triển (Giai đoạn 0 → 7, theo ROADMAP; cập nhật 2026-10-10 §13). Mọi sửa đổi phải có sự chấp thuận trực tiếp từ Project Lead.

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

## 2. Cây Chuyển Chức 4 Nhánh (15 Class) & Apex Class (Tổng Cộng 16 Class)

Cấu trúc cây chức nghiệp chuẩn đã được duyệt gồm 15 class thuộc 4 nhánh chính bắt nguồn từ 4 class T1 nền tảng, cùng 1 class Apex đa nhánh (tổng cộng 16 class):

1. **Nhánh Hộ Vệ (Guard Line - 6 Class)**:
   - **T1**: `Vanguard` (Tiên Phong)
   - **T2**: `Templar` (Thánh Hiệp Sĩ), `Berserker` (Cuồng Chiến Sĩ), `Swordmaster` (Kiếm Sư)
   - **T3**: `Dragon Knight` (Long Kỵ Sĩ), `Void Blade` (Hư Không Kiếm)
   - *Lộ trình nhánh Kiếm*: `Swordmaster (T2) → Void Blade (T3)`.
2. **Nhánh Du Hiệp (Scout Line - 3 Class)**:
   - **T1**: `Ranger` (Xạ Thủ)
   - **T2**: `Shadowblade` (Ảnh Nhẫn)
   - **T3**: `Phantom Stalker` (U Hồn Đoạt Mệnh)
3. **Nhánh Pháp Sư (Caster Line - 3 Class)**:
   - **T1**: `Arcanist` (Bí Thuật Sư)
   - **T2**: `Elementalist` (Nguyên Tố Sư)
   - **T3**: `Chronomancer` (Thời Gian Pháp Sư)
4. **Nhánh Tín Đồ (Faith Line - 3 Class)**:
   - **T1**: `Acolyte` (Tập Sự)
   - **T2**: `Inquisitor` (Thẩm Phán Dị Giáo)
   - **T3**: `Seraph` (Thiên Sứ Lục Dực - Phương án 1: chân chạm đất, dùng chung Lower Body Rig, 50 asset mới)
5. **Class Ẩn Tối Thượng (Apex Class - 1 Class Đa Nhánh)**:
   - **T4**: `God Slayer` (Kẻ Diệt Thần - Yêu cầu hoàn thành kỳ ngộ và đạt T3 từ nhiều nhánh). Tag chuẩn: `Class.Line.Apex.GodSlayer`.

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
  - Không cho phép giao dịch P2P trực tiếp đối với Quyển Trục Thăng Chức (giao dịch giữa người chơi để sau MVP).
  - MVP chỉ bán quyển trục cho NPC Thương nhân lấy vàng.
  - Chợ Đen Cấm Địa được bán lại quyển trục (xoay vòng hàng hiếm hoặc mua lại).
  - Phân rã quyển trục thành Tàn Trang (`item_skill_shard`) theo tỷ lệ trong `skill-progression-system.md`.
- **Giai đoạn Post-MVP / Commercial**:
  - Giao dịch giữa người chơi / Nhà Đấu Giá (Auction House) để sau MVP khi cơ chế kiểm soát lạm phát và chống gian lận kinh tế hoàn thiện.

---

## 7. Quy Chuẩn Vũ Khí & Trang Bị Theo 16 Class

Thống nhất dứt điểm mọi phân bổ weapon family cho toàn bộ 16 Class từ `character-visual-system.md` và `itemization.md`:

1. **Nhánh Hộ Vệ (Guard Line)**:
   - **Vanguard (T1)**: Khiên Sắt Vuông (`Item.Shield.Square`) + Kiếm 1 tay (`Weapon.1H.Blade`).
   - **Templar (T2)**: Đại Thuẫn / Khiên Tháp (`Item.Shield.Tower`) + Chùy 1 tay (`Weapon.1H.Mace`).
   - **Berserker (T2)**: Vũ khí 2 tay hạng nặng (`Weapon.2H.Heavy` - Đại Đao).
   - **Swordmaster (T2)**: Kiếm 1 tay (`Weapon.1H.Blade`), không dùng khiên. Tay trái tự do (Paperdoll tách biệt `Layer_Arms/Hands` khỏi `Layer_Equipment_Offhand`).
   - **Dragon Knight (T3)**: Vũ khí cán dài (`Weapon.2H.Polearm` - Chiến Kích). *(Lưu ý: `itemization.md` dòng 200 còn ghi thêm `Weapon.2H.Heavy`, đang trình duyệt xác nhận)*.
   - **Void Blade (T3)**: Kiếm đơn Hư Không (`Weapon.1H.Blade`).
2. **Nhánh Du Hiệp (Scout Line)**:
   - **Ranger (T1)**: Cung 2 tay (`Weapon.2H.Bow`). *(Sub-set: Đoản đao `Weapon.Dual.Daggers` theo itemization.md)*.
   - **Shadowblade (T2)**: Song đao / Dao găm (`Weapon.Dual.Daggers`).
   - **Phantom Stalker (T3)**: Song đao ám khí (`Weapon.Dual.Daggers`). *(Lưu ý: `character-visual-system.md` ghi `Dual.Daggers / Cung ám khí`; `itemization.md:198-205` đang thiếu, đang trình duyệt)*.
3. **Nhánh Pháp Sư (Caster Line)**:
   - **Arcanist (T1)**: Pháp trượng (`Weapon.2H.Staff`).
   - **Elementalist (T2)**: Pháp trượng (`Weapon.2H.Staff` / Orbs).
   - **Chronomancer (T3)**: Thánh tích thời gian (`Weapon.1H.Mace` / `Relic` - Đồng Hồ Cát).
4. **Nhánh Tín Đồ (Faith Line)**:
   - **Acolyte (T1)**: Chùy chuông (`Weapon.1H.Mace` / `Relic`).
   - **Inquisitor (T2)**: Chùy 1 tay (`Weapon.1H.Mace`). Bỏ hoàn toàn roi xích.
   - **Seraph (T3)**: Chùy thánh / Thánh tích (`Weapon.1H.Mace` / `Relic`).
5. **Nhánh Apex (Tối Cao)**:
   - **God Slayer (T4)**: Kiếm 1 tay (`Weapon.1H.Blade`) và Vũ khí cán dài (`Weapon.2H.Polearm`).

---

## 8. Quy Chuẩn GameplayTags

- Cấu trúc Tag Class chuẩn:
  `Class.Line.<Nhánh>.<Class>`  
  *(Ví dụ: `Class.Line.Guard.Vanguard`, `Class.Line.Guard.DragonKnight`, `Class.Line.Guard.Swordmaster`, `Class.Line.Guard.VoidBlade`)*.
- **Quy chuẩn riêng cho Apex Class**: `Class.Line.Apex.GodSlayer`.
- Điều kiện chuyển chức không hardcode trong Tag mà nằm trong Data Asset của quyển trục (`AllowedSourceClassTags`, `RequiredMinRank`).
- Bỏ hoàn toàn các tag cũ định dạng cấp độ hoặc thiếu nhánh (`Class.TierX.*`, `Class.RankX.*`, `Class.Vanguard`). Tag của Void Blade là `Class.Line.Guard.VoidBlade`.

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

## 10. Tiêu Chuẩn Art Gate, Quy Chuẩn 5 Hướng & Nguồn Art Hybrid

- Phong cách chủ đạo: **Action-First HD-2D**, kết hợp Paper2D/PaperZD cho nhân vật và Spine 4.3 cho boss.
- **Nguồn Art Hybrid (Hybrid Asset Pipeline)**:
  - Nhân vật: Thiết kế và sản xuất riêng bằng AI + Aseprite.
  - Môi trường: Có thể sử dụng asset pack bên ngoài, nhưng bắt buộc phải qua kiểm tra bảng màu (palette check) và bản quyền (license check hợp lệ).
- **Quy Chuẩn Hướng Nhìn Nhân Vật (5-Direction Isometric System)**:
  - Nhân vật sử dụng **5 hướng nhìn** vẽ tay gốc: Nam (`S`), Đông Nam (`SE`), Đông (`E`), Đông Bắc (`NE`), Bắc (`N`).
  - 3 hướng phía Tây (Tây Nam `SW`, Tây `W`, Tây Bắc `NW`) được tạo bằng cách **lật ngang (horizontal flip)** từ các hướng phía Đông tương ứng.
  - Chấp nhận vật phẩm cầm tay (vũ khí/khiên) đổi tay khi lật hình (mirror flip trade-off) để tối ưu ngân sách vẽ và dung lượng bộ nhớ.
- **Ngoại Lệ Art Gate**: Boss Stone Golem dùng Spine 4.3 được chấp nhận có rotation artifacts (nội suy xoay khớp xương), không bị cổng `pixel-review` đánh rớt.
- **Vanguard Art Direction**: Chọn Phương án A (Iron Bastion) - Giáp trụ hiệp sĩ gothic nặng, khiên sắt vuông, phong thái kiên cường vững chãi.

---

## 11. Kiến Trúc Netcode & An Toàn Đồng Bộ Mạng (100% Server-Authoritative)

- **Nguyên tắc cốt lõi (ADR-0001)**: Mọi logic trò chơi là **100% Server-Authoritative**. Dedicated Server là thẩm quyền duy nhất (Single Source of Truth) quyết định máu, sát thương, hiệu ứng khống chế, vị trí thực tế, nhặt đồ và thăng chức. Client chỉ chạy mô phỏng dự đoán (Client-side Prediction) và hiển thị kết quả.
- **Iris Network Replication**: Bắt buộc bật Iris Replication (`net.Iris.UseIrisReplication=1`) cho Unreal Engine 5.8+, tối ưu băng thông cho kiến trúc MMO/Co-op diện rộng.
- **Gameplay Ability System (GAS) Replication Mode**:
  - Nhân vật người chơi (Player Characters / Pawns): Sử dụng `EGameplayEffectReplicationMode::Mixed` (Replicate GameplayEffects tới Owner Client để hiển thị UI/HUD lập tức; chỉ Replicate GameplayTags và GameplayCues tới các Simulated Proxies khác để tối ưu đường truyền).
  - Quái vật / Boss / NPC: Sử dụng `EGameplayEffectReplicationMode::Minimal` (Chỉ replicate GameplayTags và GameplayCues).
- **Kiểm thử tự động mạng**: Mọi tính năng gameplay/combat đều phải có bài test headless replication xác thực tính nhất quán giữa Server và ít nhất 2 Clients.

---

## 12. Quyết Định Bổ Sung Sau Rà Soát Plan ↔ Code (2026-10-09)

> **Nguồn**: lựa chọn của chủ dự án, lưu lúc 2026-10-09 11:49 UTC trên trang "Ascendant Review Decisions", sau báo cáo `production/qa/review-plan-vs-code-2026-10-09.md`. Kế hoạch thực thi: `production/plans/execution-owner-decisions-2026-10-09.md` (thêm vào bằng một PR riêng).

- **ROADMAP**: Khôi phục `production/ROADMAP.md` theo bản ở commit `493a442`.
- **Talent Tree, Skill Point, Respec của talent**: **Gỡ bỏ** khỏi code và story. Đây là hệ thống không có trong GDD, nên không được phát triển tiếp. Tiến trình kỹ năng vẫn theo Skill Book như `skill-progression-system.md` mô tả. Tính năng đổi bộ kỹ năng (respec) mà GDD đã định nghĩa (`skill-progression-system.md:210`, `foundational-classes.md:445`) **được giữ nguyên**.
- **Thông số Dash / Combo / Finisher**: Giá trị đang có trong code runtime (`PAGameplayAbility_Dash`, `PAGameplayAbility_MeleeAttack`, `PAGameplayAbility_Finisher`) là **chuẩn**. GDD, `control-manifest.md`, `tr-registry.yaml` và story phải sửa cho khớp code. Cảm giác chơi với các giá trị này **vẫn cần chủ dự án duyệt** (điểm bắt buộc dừng và hỏi theo CLAUDE.md) trước khi coi là chốt.
- **Epic Class Chính / Phụ & Thăng Chức** (§3, §4): lập sau khi chốt ROADMAP, xếp vào giai đoạn phù hợp. ROADMAP bản `493a442` đã có sẵn giai đoạn cho hệ thống này: **Giai đoạn 2B — Hệ thống Class Chính / Phụ & Quyển trục**.
- **`item_skill_shard` (Tàn Trang)**: là **tiền tệ thứ hai** theo `merchant-economy.md` (giới hạn 99.999). Mọi chỗ còn dùng "Ash Shards" đều đổi sang `item_skill_shard`, đúng với §5. `inventory-system.md:49,62` (đang xếp `item_skill_shard` vào nhóm Nguyên Liệu) phải sửa cho khớp.

---

## 13. Quyết Định Bổ Sung Của Chủ Dự Án (2026-10-10)

> **Nguồn**:
> - Lựa chọn của chủ dự án, lưu lúc 2026-10-10 11:11 UTC trên trang "Ascendant Owner Approvals" (26 câu).
> - Ba câu trả lời trong terminal cùng ngày.
> - Kế hoạch thực thi: `production/plans/playable-slice-and-approvals-2026-10-10.md` (PR #28, đã merge). Các file chịu ảnh hưởng của những quyết định dưới đây được sửa ở các PR Y1, Y3… của kế hoạch đó.

**Cảm giác chơi**
- Thông số dash/combo/finisher trong code (§12): duyệt **sau khi chơi thử**. Chưa coi là chốt cho tới khi chủ dự án chơi thử và xác nhận.
- Cửa sổ Dash Cancel / Dash Attack: lấy mốc cũ co theo tỉ lệ 0.35/0.45, ra khoảng 0.27–0.35s. Chốt khi triển khai.
- Dash của Ranger: **0.30s**.
- Input buffer: chưa chốt, vẫn giữ nhãn "chưa triển khai". Hai giá trị đang lệch nhau: 0.15s (`combat-system.md`) và 250ms (manifest/tr-registry/accessibility). Sẽ chốt khi làm tính năng này.
- FOV camera: **90°**.

**Thông số thiết kế**
- Bán kính tương tác với merchant/forge: **300cm** (`merchant-economy.md:24`).
- "điểm kỹ năng" trong dữ liệu auto-save (`zone-system.md:53`) nghĩa là **cấp kỹ năng trong Grimoire**. Không phải Skill Point, vốn đã bị gỡ ở §12.
- Dragon Knight chỉ dùng **`Weapon.2H.Polearm`**, không dùng 2H Heavy. Câu "đang trình duyệt" ở §7 coi như đã đóng.
- Chết ở PvE: rơi **100% Tàn Trang** vào Ash Remnant (`zone-system.md:88`). Code và story phải sửa cho khớp.
- Bản đồ hầm ngục của Smuggler bán bằng **Tàn Trang** (`merchant-economy.md:105`).
- Giá gốc để tính phí sửa đồ Rare: **300** (`inventory-system.md:193`).
- Rèn Boss Soul làm đúng GDD: **4 bộ phận khác nhau** (Sừng, Vảy đuôi, Giáp ngực, Cánh), đồ ra theo class. Chi phí **5.000 Gold**, ghi bổ sung vào `blacksmithing-system.md`.

**ROADMAP & quy trình**
- Phạm vi giai đoạn theo ROADMAP: **0 → 7**, gồm cả 1.5 và 2B.
- Các mục ở `ROADMAP.md:29` và `:33` hạ về `[~]` vì chưa có bằng chứng. Thêm vào Giai đoạn 0 mục "bảo vệ nhánh main", đánh `[x]` với bằng chứng từ X3.
- Khi các file mâu thuẫn nhau:
  - Mâu thuẫn chạm tới DECISIONS hoặc ROADMAP: **dừng và hỏi**.
  - Các mâu thuẫn khác: chọn theo file và ghi vào PROGRESS.
- Agent được đổi `[~]` → `[x]` trong ROADMAP, nhưng chỉ khi có đủ PR đã merge, log test và kết luận reviewer. Phần nội dung còn lại của ROADMAP vẫn chỉ chủ dự án được sửa.
- Backend: **chưa chốt** NestJS. Để tới Giai đoạn 6 mới quyết, qua ADR-0006.
- Ngày của sprint 5–7: đổi theo ngày commit thật.
- **Chèn luồng "vòng chơi được" lên trước thứ tự ROADMAP.** Luồng gồm: UI login/đăng ký/chọn nhân vật/HUD/shop/forge, đánh boss Stone Golem, và chạy 2 client.
  - Phần 2 client gộp vào Giai đoạn 1.5.
  - Đánh boss kéo theo Giai đoạn 1 và việc gắn boss/stagger vào game.
  - Login thật vẫn thuộc Giai đoạn 6.
- **Thứ tự ưu tiên:**
  - Trên trang duyệt, chủ dự án chọn `i-next` là "Dựng test replication thật (1 server + 2 client)".
  - Sau đó, trong terminal, chủ dự án đổi ưu tiên, nguyên văn: *"làm tới giai đoạn hiện tại chỉ có phần BE không có UI/UX cũng chả có tác dùng gì cũng chả test được BE có hoặc động đúng k"*.
  - Vì vậy test replication thật **vẫn được làm**, nhưng xếp sau bản chơi thử, nằm trong Giai đoạn 1.5 và vẫn giữ 🛑 ở `ROADMAP.md:57`.
  - **§11 vẫn áp dụng đầy đủ.** Các tính năng gameplay/combat trong vòng chơi được (boss/stagger, shop/forge) chưa được tính là Complete cho tới khi có test replication headless với 1 server và ít nhất 2 client.

**Thẩm mỹ & asset**
- Khung độ hiếm trong `art-bible.md:127-128` dùng bảng màu ở `inventory-system.md` §1. Không tự chọn màu mới.
- Asset sinh bằng AI: duyệt **theo lô**. Mỗi lô gom vào một trang xem trước, và vẫn phải dừng chờ chủ dự án duyệt.
- Đổi tên các asset và nhãn "Divine" sang **Legendary**.
- Art proxy của visual-004..007: chủ dự án sẽ tự xem `Art_Gallery` rồi báo lại.

**Hạ tầng**
- Git LFS áp dụng từ giờ về sau: chuyển các file hiện có bằng **một commit mới**, không viết lại lịch sử.
- Antigravity: chủ dự án tự mở rộng allow-list. Sau đó agy chỉ nhận việc tài liệu/rà soát, và có Claude reviewer kiểm lại.
