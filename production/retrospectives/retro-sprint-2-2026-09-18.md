# Sprint 2 Retrospective: Core Layer Gameplay Loop

> **Sprint**: Sprint 2 (Core Layer — Combat, Locomotion & Level Blockout)  
> **Period**: 2026-09-17 to 2026-09-18  
> **Generated**: 2026-09-18  
> **Stage**: Pre-Production  
> **Review Mode**: Lean  

---

## 1. Metrics & Velocity

| Metric | Planned | Actual | Delta |
|---|---|---|---|
| **Tasks / Stories** | 8 | 8 | 0 (100% Complete) |
| **Completion Rate** | 100% | 100% | 0% |
| **Effort (Days)** | 7.5 days | 2.0 days | -5.5 days (High Velocity) |
| **Must-Have Stories** | 5 | 5 | 0 |
| **Should-Have Stories** | 2 | 2 | 0 |
| **Nice-to-Have Stories** | 1 | 1 | 0 |
| **Bugs / Issues Found** | — | 6 | +6 (Resolved in Review) |
| **Bugs Fixed** | — | 6 | 6/6 (100% Resolved) |
| **Automated Tests Pass Rate** | 100% | 100% | 0 failures |

### Velocity Trend

| Sprint | Layer | Planned Stories | Completed Stories | Rate |
|---|---|---|---|---|
| **Sprint 1** | Foundation Layer (Attributes, Controller, Inventory, Netcode) | 13 | 13 | 100% |
| **Sprint 2** | Core Layer (World, Character, Combat, Items, Boss) | 8 | 8 | 100% |

**Trend**: **Increasing & Stable**.  
Kiến trúc Deep Modules (ADR-0001, ADR-0002, ADR-0003) phân tách ranh giới rõ ràng giúp việc phát triển các tính năng Core diễn ra tốc độ cao, không bị xung đột phụ thuộc chéo.

---

## 2. What Went Well

1. **Chuỗi Kỹ Năng Tác Chiến & Cảm Giác Điều Khiển (Responsive Combat & True Skill Expression)**:
   - Hệ thống Lướt Né I-frame 0.20s kết hợp 3-hit combo (`1.0x` $\rightarrow$ `1.2x` $\rightarrow$ `1.6x`) và đòn kết liễu Finisher 25% Max HP vận hành mượt mà, phản hồi ngay lập tức không có độ trễ.
   - Cơ chế ngắm độc lập (Decoupled Aiming) kết hợp xoay bù 45° Isometric Screen-Relative cho phép người chơi vừa lùi vừa vung kiếm chuẩn xác.
2. **Quy Trình Tự Động Hóa PaperZD & Automation Testing**:
   - Khả năng trích xuất flipbook 12 FPS và thiết lập PaperZD State Machine qua script Python headless trong môi trường Linux chạy trơn tru.
   - Bộ kiểm thử tự động của Unreal Automation Testing (`boss_paperzd_aggro_test.cpp`, `code_review_fixes_regression_test.cpp`) chạy trực tiếp qua `UnrealEditor-Cmd -nullrhi` giúp phát hiện sớm mọi lỗi hồi quy logic.
3. **Môi Trường Thế Giới & Thể Chế An Toàn (Sanctuary & Leash Volumes)**:
   - Tách biệt rõ ràng giữa vùng an toàn Sanctuary (khóa chiêu thức tấn công, miễn nhiễm sát thương) và khu vực chiến đấu, cùng Leash reset 2500cm chống tình trạng kéo quái phá vỡ bản đồ.

---

## 3. What Went Poorly & Root Causes

1. **Bất cập giữa Cơ chế Sát thương Truyền thống của Unreal (`ApplyDamage`) và GAS**:
   - *Hiện tượng*: Boss Slam gọi `UGameplayStatics::ApplyDamage`, nhưng do `APABaseCharacter` quản lý máu qua `UAscendantAttributeSet` của GAS nên đòn đánh không trừ máu người chơi.
   - *Nguyên nhân*: Thiếu hàm ghi đè `TakeDamage()` làm cầu nối chuyển đổi sát thương Unreal thành `UGameplayEffect`.
   - *Khắc phục*: Đã ghi đè `TakeDamage()` trên `APABaseCharacter` và chuyển toàn bộ các đòn đánh Boss sang `UGameplayEffect` chuẩn GAS.
2. **Bỏ quên Cờ Bảo Vệ I-Frame trên Một Số Đòn Đánh**:
   - *Hiện tượng*: Kỹ năng chém quét MeleeAttack và Boss Slam ban đầu không kiểm tra thẻ `State.Invulnerable` và `State.InSanctuary`.
   - *Khắc phục*: Đã bổ sung kiểm tra triệt tiêu sát thương khi mục tiêu đang trong khung bất tử né lướt.

---

## 4. Technical Debt & TODO Status

- **Current TODO count**: 0
- **Current FIXME count**: 0
- **Architectural Gaps**: 0
- **Tech Debt Trend**: **Shrinking** (Toàn bộ 6 điểm nghi vấn sau Code Review đã được giải quyết dứt điểm và kiểm chứng bằng test).

---

## 5. Action Items for Next Iteration

| # | Action | Owner | Priority | Target |
|---|---|---|---|---|
| 1 | Thực hiện nghiệm thu Playtest bản **Vertical Slice** hoàn chỉnh (Outpost $\rightarrow$ Ruins $\rightarrow$ Boss Battle) | `producer` / `lead-qa` | **High** | Pre-Production Gate |
| 2 | Chạy `/gate-check` chuẩn bị hồ sơ chuyển từ **Pre-Production $\rightarrow$ Production** | `producer` | **High** | Pre-Production Gate |
| 3 | Lên kế hoạch Sprint 3: Triển khai các hệ thống mở rộng (Multiplayer Server, Blacksmithing & Merchant) | `producer` | **Medium** | Sprint 3 Kickoff |
| 4 | Kích hoạt `game-art-studio` để sản xuất trọn bộ Pixel Art Assets HD-2D thực tế thay thế blockout | `art-director` | **Medium** | Sprint 3 |

---

## 6. Summary

Sprint 2 đã xuất sắc hoàn thành 100% khối lượng công việc đặt ra cho Tầng Cốt Lõi (Core Layer). Toàn bộ vòng lặp chiến đấu ARPG Isometric đã chứng minh được tính hấp dẫn, sự mượt mà và tính bền vững về mặt kiến trúc. Dự án đã hội đủ điều kiện để bước vào nghiệm thu Vertical Slice và vượt cổng chuyển giao sang giai đoạn Production.
