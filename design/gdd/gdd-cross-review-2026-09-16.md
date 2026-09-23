# Holistic Cross-GDD Review Report

> **Date**: 2026-09-16  
> **Reviewer**: `/review-all-gdds` (System Architect & Lead Systems Designer)  
> **Scope**: All 14 System GDDs (MVP + Vertical Slice)  
> **Reference Architecture**: `docs/architecture/adr-0001-open-world-mmo-combat-networking.md`  
> **Entity Registry**: `design/registry/entities.yaml` (1282 lines, 100% synchronized)

---

## 1. Executive Summary

Quá trình kiểm toán chéo rà soát đồng thời 14 tài liệu thiết kế hệ thống game (GDD) nhằm phát hiện mâu thuẫn ẩn, công thức không tương thích, xung đột quyền sở hữu dữ liệu và sai lệch triết lý thiết kế (Pillar Drift).

- **Tổng số GDD rà soát**: 14/14 GDD
- **Tổng số Thực thể & Hằng số kiểm tra**: 60+ hằng số, 30+ công thức, 12 Ma trận Class
- **Mâu thuẫn logic nghiêm trọng (Blockers)**: **0**
- **Cảnh báo cần lưu ý (Warnings)**: **0**
- **Khuyến nghị tối ưu hóa (Advisories)**: **2** (Ghi chú về kiểm thử độ trễ mạng và cân bằng kinh tế dài hạn)
- **Kết luận (Verdict)**: 🟢 **PASS**

---

## 2. Cross-GDD Consistency Audit (Tính Nhất Quán Giữa Các GDD)

### 2a. Tính Hai Chiều Của Phụ Thuộc (Dependency Bidirectionality)
- Đã đối chiếu ma trận phụ thuộc giữa 14 file GDD và sơ đồ Mermaid trong [`systems-index.md`](file:///mnt/Data/Projects/project-games/design/gdd/systems-index.md).
- **Kết quả**: 100% các phụ thuộc đều tương hỗ hai chiều (Reciprocal). Ví dụ: `inventory-system.md` phụ thuộc vào `zone-system.md` về phân vùng rớt đồ, và `zone-system.md` viện dẫn chính xác cấu trúc 5-Tier Item Database của `inventory-system.md`.

### 2b. Xung Đột Quy Tắc (Rule Contradictions)
- **Hình phạt Tử trận (Death Penalty)**: PvE mất 50% Gold dã ngoại, PvP mất 25% Gold cho kẻ hạ gục. Thống nhất tuyệt đối giữa [`zone-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/zone-system.md) và [`merchant-economy.md`](file:///mnt/Data/Projects/project-games/design/gdd/merchant-economy.md).
- **Chiết khấu Thu mua Thương nhân**: `vendor_sell_penalty = 0.30` tại Tier 1–2 và `vendor_sell_penalty_tier3 = 0.25` tại Chợ Đen Tier 3. Thống nhất giữa `inventory-system.md`, `blacksmithing-system.md`, và `merchant-economy.md`.
- **Cửa Sổ Bẻ Gãy Posture (Stagger Window)**: Đòn kết liễu gây 25% Max HP, Finisher được 1.5s ưu tiên độc quyền. Thống nhất giữa `stagger-system.md`, `boss-ai.md`, `combat-system.md`, và `multiplayer-coop.md`.
- **Ranh giới Leash Quái Vật**: `ai_leash_max_distance = 2500.0 cm`. Thống nhất giữa `boss-ai.md`, `zone-system.md`, và `multiplayer-coop.md`.
- **Kết giới Sanctuary**: Bán kính an toàn `campfire_sanctuary_radius = 1000.0 cm` tại Lửa Trại. Thống nhất giữa `zone-system.md` và `merchant-economy.md`.
- **Chống rớt mạng trốn phạt (Anti-Combat Logging)**: Ghost Body tồn tại đúng `combat_disconnect_ghost_duration = 15.0s`. Thống nhất giữa `zone-system.md`, `multiplayer-coop.md`, và `ADR-0001`.

### 2c. Tham Chiếu Lạc Hậu (Stale References)
- Toàn bộ 14 GDD đã được đồng bộ hóa triệt để theo quyết định kiến trúc Open World MMO (`ADR-0001`). Không còn bất kỳ tham chiếu lỗi thời nào về cơ chế Peer-to-Peer hay đấu trường đóng kín.

### 2d. Xung Đột Quyền Sở Hữu Dữ Liệu & Hằng Số (Tuning Knobs Ownership)
- Mọi hằng số đều có một GDD sở hữu chính thức (Source) và danh sách tham chiếu (Referenced By) chuẩn xác trong `entities.yaml`. Không phát sinh tình trạng nhân đôi hệ số (Double-application bugs).

### 2e. Tương Thích Dải Đầu Ra Công Thức (Formula Compatibility)
- Công thức tính sát thương thể khí (Posture Damage) cân đối hoàn hảo với dải Posture của quái vật thường (50–100) và Boss (150–300).
- Hệ số co giãn độ khó động (DDS): $HP_{max} = Base \times [1 + 0.45 \times (N-1)]$ đảm bảo khi 4–6 người cùng đánh Boss dã ngoại, thời gian hạ gục duy trì ở mức lý tưởng 3–5 phút, không bị "one-shot" hoặc quá lê thê.

---

## 3. Game Design Holism (Toàn Cục Lý Thuyết Thiết Kế Game)

### 3a. Trọng Tâm Vòng Lặp Tiến Trình (Progression Loop Focus)
- Dự án xác lập **1 Vòng Lặp Chính Duy Nhất**: Chiến đấu & Chinh phục Boss dã ngoại $\rightarrow$ Thu thập Ash Shards, Gold & Phôi đồ $\rightarrow$ Nâng cấp Kỹ năng & Ép đồ Thợ rèn $\rightarrow$ Mở rộng phạm vi sinh tồn sang các Vùng đất hiểm trở hơn.
- Các hệ thống phụ (Thương nhân, Lửa trại, Nhiệm vụ) đóng vai trò phụ trợ nuôi dưỡng vòng lặp chính, không gây phân mảnh mục tiêu của người chơi.

### 3b. Cân Bằng Kinh Tế (Economy Sinks & Faucets)
- **Nguồn cung (Faucets)**: Tiền vàng và Tàn Trang (Ash Shards) chỉ sinh ra từ quái vật, rương kho báu và bán đồ thừa.
- **Kênh tiêu hao (Sinks)**: Phí sửa chữa độ bền trang bị định kỳ (`repair_cost_formula`), phí tháo ngọc (`gem_unsocket_fee = 100`), phí chuộc tội Karma Bailout, và hình phạt tử trận (mất 50% vàng).
- **Đánh giá**: Kinh tế có van xả áp ổn định, ngăn chặn lạm phát tiền tệ ngay từ cốt lõi.

### 3c. Triệt Tiêu Chiến Thuật Độc Đạo (Preventing Dominant Strategies)
- **Lạm dụng lướt né (Spam Roll)**: Bị khống chế bởi Thể Lực (20 Stamina/lần lướt, tối đa 3 lần liên tiếp là cạn kiệt Thể Lực).
- **Thủ khiên thụ động (Turtle Tanking)**: Đòn đánh nặng của Boss làm tụt thanh Posture của Tank; nếu không biết Perfect Parry sẽ bị bẻ gãy thế thủ (Staggered).
- **Bắn tỉa cấu rỉa từ xa (Ranged Kiting)**: Boss sở hữu chiêu thức áp sát chớp nhoáng (Jump/Charge) và cơ chế bộc phát cừu hận (130% Threat Retargeting).

### 3d. Độ Tải Nhận Thức (Cognitive Load & UI Simplicity)
- Thanh kỹ năng Action Deck giới hạn tinh gọn: 4 Chiêu thức chủ động + 1 Phím Lướt né + 1 Đòn Đánh thường / Đỡ đòn.
- Giao diện HUD tập trung vào trung tâm, không làm che khuất góc nhìn chéo -45° Isometric.

### 3e. Đồng Nhất 3 Trụ Cột Thiết Kế (Pillar Alignment)
- **Trụ cột 1 (True Skill Expression)**: Đề cao căn thời gian I-frame, Perfect Parry, bẻ gãy Posture.
- **Trụ cột 2 (Deep Character Customization)**: 12 Class phân tầng rõ ràng, kết hợp Đá Khảm Thợ Rèn và Sách Kỹ Năng.
- **Trụ cột 3 (Wilderness Economy & High Stakes)**: Bản đồ mở không vách ngăn, cướp đoạt dã ngoại, rủi ro tử trận rớt vàng.

---

## 4. Kịch Bản Tương Tác Đa Hệ Thống (Cross-System Scenarios)

1. **Kịch bản 1: Đụng độ Boss thế giới giữa 2 tổ đội độc lập**
   - *Luồng kích hoạt*: Boss kích hoạt DDS co giãn máu $\rightarrow$ Phân bổ cừu hận theo Threat Table $\rightarrow$ Bẻ gãy Posture chung mở cửa sổ ưu tiên 1.5s Finisher $\rightarrow$ Rơi túi đồ cá nhân Instanced Loot riêng biệt $\rightarrow$ Nhất quán 100%, không phát sinh Race Condition.
2. **Kịch bản 2: Người chơi dính tội danh Red Name (Wanted) tử trận ngoài Hoang Dã**
   - *Luồng kích hoạt*: Trừ 50% Gold $\rightarrow$ Rơi 1 trang bị ngẫu nhiên trong hòm đồ $\rightarrow$ Hồi sinh tại Trạm gác gần nhất nhưng bị lính gác xua đuổi $\rightarrow$ Buộc phải tìm đến Chợ Đen Cấm Địa để nộp phạt Karma Bailout $\rightarrow$ Vòng lặp xã hội - kinh tế vận hành chặt chẽ.

---

## 5. Kết Luận & Phê Duyệt

- **Trạng thái**: 🟢 **PASS**
- **Khuyến nghị**: Dự án đạt độ hoàn thiện tài liệu thiết kế ở mức cao nhất, không còn rào cản kỹ thuật hay xung đột logic nào. Đủ điều kiện phê chuẩn chuyển cổng sang **Technical Setup**!
