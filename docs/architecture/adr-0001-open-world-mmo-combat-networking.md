# ADR-0001: Open World MMO Contested Combat, Netcode & Dynamic Scaling Architecture

## Status
Accepted

## Date
2026-09-15

## Engine Compatibility

| Field | Value |
|---|---|
| **Engine** | Unreal Engine 5.7 |
| **Domain** | Networking & Core Gameplay (Server-Authoritative GAS, Dedicated Servers, Iris Replication) |
| **Knowledge Risk** | HIGH — UE 5.7 Iris Replication System & World Partition Network Streaming |
| **References Consulted** | `docs/engine-reference/unreal/VERSION.md` |
| **Post-Cutoff APIs Used** | Iris Replication System, World Partition Replication Streaming |

---

## Context and Problem Statement

Ban đầu, Project Ascendant được định hình quanh mô hình Co-op 1–4 người chơi. Tuy nhiên, theo định hướng điều chỉnh phạm vi dự án (v2), trò chơi chuyển dịch toàn diện sang mô hình **MMO Thế Giới Mở (Open World MMO)** với cơ chế **Chiến Đấu Cạnh Tranh Thực Tế (Contested Combat / Open Tagging)** lấy cảm hứng từ thể loại LitRPG sống động.

Trong môi trường thế giới mở nơi hàng trăm người chơi xa lạ có thể cùng lúc thâm nhập một vùng đất và tự do tham chiến vào cùng một mục tiêu quái vật hoặc Boss thế giới (không có quyền sở hữu độc quyền, no kill-lock, no tag exclusivity):
1. Làm thế nào để đảm bảo tính công bằng, chống gian lận (cheat/aimbot/godmode/desync) khi hàng chục người cùng tung chiêu vào một mục tiêu?
2. Phân phối phần thưởng (Loot & EXP) như thế nào để vừa khuyến khích hợp tác mở, vừa triệt tiêu nạn "ăn hôi" (leeching 1 đòn) và không gây tranh cướp vật lý độc hại?
3. Thanh Posture và cơ chế Kết Liễu (Execution 25% Max HP) từ [`stagger-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/stagger-system.md) vận hành ra sao khi có nhiều người cùng đánh dồn dập?
4. Điều tiết độ khó của Boss thế giới (Dynamic Difficulty Scaling) như thế nào để ngăn chặn hiện tượng "lấy thịt đè người" (Zerg rush) làm mất đi độ khó hardcore cốt lõi?
5. Hệ thống Tổ đội 4 người và Hệ thống Danh dự & Truy nã (Karma / Wanted System) được tích hợp vào kiến trúc mạng như thế nào?

---

## Decision Drivers

* **Trụ Cột 1 & Trụ Cột 3**: Biểu đạt kỹ năng tối thượng (True Skill Expression) và thế giới mở cạnh tranh rủi ro cao - phần thưởng lớn.
* **Chống Độc Hại & Chống Gian Lận (Anti-Griefing & Anti-Cheat)**: Bảo vệ nạn nhân bị PK, trừng phạt nghiêm khắc kẻ ác (Red Name), ngăn chặn hoàn toàn việc client tự báo cáo sát thương gian lận.
* **Tính Nhất Quán Trong Gameplay**: Đảm bảo các chỉ số GAS (`UAscendantAttributeSet`), I-frame 0.28s, Stagger 3.0s được đồng bộ hoàn hảo trên máy chủ.
* **Khả Năng Mở Rộng Kỹ Thuật (Scalability)**: Sử dụng Dedicated Server với UE 5.7 Iris Replication để tối ưu băng thông khi đông người chơi cùng tụ tập săn Boss thế giới.

---

## Considered Options

### 1. Phân Phối Loot & EXP
* **Option 1A (Chấp Thuận)**: **Instanced Loot Cá Nhân Có Ngưỡng Đóng Góp (Contribution Threshold >= 5%)**. Máy chủ tính toán riêng; người chơi/tổ đội đạt ngưỡng nhận bảng rơi đồ cá nhân độc lập.
* **Option 1B**: Instanced Loot thuần túy (chỉ cần đánh trúng 1 hit). *Nhược điểm: Bị lạm dụng để kéo cày phụ, ăn hôi.*
* **Option 1C**: Rơi đồ vật lý chung trên mặt đất (Free-For-All). *Nhược điểm: Gây ức chế tiêu cực, phân biệt đối xử với class cận chiến/hỗ trợ.*

### 2. Thanh Posture Của Boss & Quyền Execution
* **Option 2A (Chấp Thuận)**: **1 Thanh Posture CHUNG Có Dynamic Scaling + Quyền Ưu Tiên Execution Cho Finisher (1.5s đầu)**. Người tung đòn bẻ gãy thế đứng cuối cùng được ưu tiên kết liễu trong 1.5s; sau đó mở tự do cho bất kỳ ai.
* **Option 2B**: Tách riêng thanh Posture trên client của từng người chơi. *Nhược điểm: Boss bị choáng không đồng bộ, phá vỡ tính logic của thế giới thực.*
* **Option 2C**: 1 Thanh Posture chung, ai bấm trước ăn trước (First-Come-First-Served). *Nhược điểm: Gây ức chế cho người dồn công sức bẻ gãy mà bị kẻ khác cướp công.*

### 3. Cân Bằng Độ Khó Boss Thế Giới
* **Option 3A (Chấp Thuận)**: **Dynamic Difficulty Scaling (DDS) Mở + Kháng Hiệu Ứng Đám Đông**. Boss tự động scale Max HP và Posture theo số người trong bán kính 3000 cm; tăng chỉ số Hyper-Armor khi đối đầu >= 4 người.
* **Option 3B**: Giới hạn cứng (Hard Cap 8 người). *Nhược điểm: Phá vỡ cảm giác thế giới mở không ranh giới.*

---

## Decision Outcome: Chosen Architecture

Quyết định chọn **Mô Hình Kiến Trúc Server-Authoritative 100% Cho Contested MMO**, tích hợp:
1. **Server Authority**: Dedicated Server quản lý trọn vẹn Collision Tracing, I-frame Validation, GAS GameplayAbility activation, và Health/Posture mutations.
2. **Open Tagging & Instanced Contribution Loot**: Bảng rơi đồ cá nhân độc lập per-player, điều kiện kích hoạt: Gây >= 5% tổng sát thương HP hoặc tích lũy >= 10% tổng Posture Damage lên mục tiêu (hoặc thuộc tổ đội đạt điều kiện).
3. **Shared Boss Posture & Priority Execution Window**: 
   * Thanh Posture chung của Boss scale theo công thức: Posture_max = BasePosture * [1 + 0.35 * (N_combatants - 1)].
   * Cửa sổ Stagger kéo dài 3.0s: Trong 1.5s đầu tiên, chỉ duy nhất người chơi tung đòn đánh hạ gục điểm Posture cuối cùng (Posture Break Finisher) mới nhìn thấy nút bấm Execution (rút 25% Max HP). Nếu sau 1.5s Finisher không bấm, nút Execution sẽ kích hoạt cho toàn bộ người chơi tham chiến.
4. **Dynamic Difficulty Scaling (DDS)**:
   * Máu Boss scale theo: HP_max = BaseHP * [1 + 0.50 * (N_combatants - 1)].
   * Kháng khống chế: Khi N_combatants >= 4, thời gian bị choáng/đẩy lùi giảm 30%; khi N >= 8, giảm 50%.
5. **Hệ Thống Tổ Đội 4 Người (4-Player Party Netcode)**:
   * Bán kính chia sẻ EXP: 2500 cm. Đóng góp sát thương của mọi thành viên được cộng dồn vào Ngưỡng Đóng Góp của cả nhóm.
   * Party Morale Buffs: Kích hoạt tự động qua GAS GameplayEffect khi thành viên ở gần nhau.
6. **Hệ Thống Danh Dự & Truy Nã (Karma / Wanted System)**:
   * State Machine đồng bộ biến `KarmaPoints` (từ -100 đến +100).
   * Kẻ giết người vô tội bị trừ -30 Karma/mạng. Dưới -50 Karma gán cờ `bIsWanted = true` (Đầu lâu đỏ).
   * Cơ chế Trừng phạt Bất đối xứng (Asymmetric PK Penalty): Kẻ Wanted khi chết rơi 100% Vàng, 100% Tàn Trang, có 10% tỷ lệ rơi trang bị đang mặc và bị giam ngục 15 phút. Nạn nhân vô tội chỉ mất 25% Vàng (chuyển cho kẻ PK), bảo toàn 100% Tàn Trang và Trang Bị.

---

## Consequences

### Positive
* **Chân thực & Cạnh tranh**: Tạo nên một thế giới sống thực thụ theo phong cách LitRPG, nơi mọi hành động đều có hệ quả xã hội.
* **Tuyệt đối chống Hack/Cheat**: 100% logic sát thương và thời gian I-frame chạy trên Dedicated Server, client không thể can thiệp số liệu.
* **Công bằng xã hội**: Cơ chế chia loot có ngưỡng chặn đứng nạn bot/leeching; thợ săn tiền thưởng có động lực săn lùng kẻ PK.
* **Bảo vệ người chơi yếu**: Nạn nhân bị PK không bị phạt kép, duy trì trải nghiệm tích cực.

### Negative & Mitigations
* **Tải CPU Máy Chủ (Server CPU Overhead)**: Tính toán hitbox và trace đạn cho hàng chục người cùng lúc đòi hỏi tối ưu hóa cao.
  * *Giải pháp giảm thiểu*: Sử dụng hệ thống Iris Replication của UE 5.7, tối ưu hóa Hitbox Tracing qua Sweep Multi by Channel và giảm tần số tick của quái vật ở xa người chơi.
* **Tranh chấp quyền Execution**: Finisher có thể bị hụt nếu đang ở quá xa Boss.
  * *Giải pháp giảm thiểu*: Cơ chế cửa sổ ưu tiên 1.5s đủ cho Finisher lướt Dash tới gần; nếu Finisher bị khống chế/ở quá xa, quyền Execution tự động chuyển giao cho đồng đội ở 1.5s tiếp theo.

---

## ADR Dependencies
* **Depends on**: None
* **Enables / Unlocks**: `zone-system.md` (SYS12), `multiplayer-coop.md` (SYS14 — nâng cấp thành MMO Netcode), `world-bosses.md` (SYS17)
* **Blocks**: Không có

## GDD Requirements Addressed
* `game-concept.md`: Open World MMO, Contested Combat, 4-Player Party, LitRPG Karma/Wanted System.
* `attributes-system.md`: GAS Replication Stance.
* `stagger-system.md`: Shared Boss Posture & Dynamic Scaling, Priority Execution.
* `inventory-system.md`: Instanced Drop Generation & Asymmetric Death Drop.
* `zone-system.md`: Open World PvP & Sanctuary Safe Zones.
