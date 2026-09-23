# Story 002: Posture Stagger, Kneel Window & True Damage Execution

> **Epic**: Encounter & Boss Mechanics Layer  
> **Status**: ✅ Done  
> **Layer**: Combat / Encounter  
> **Type**: Gameplay / Mechanics  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-23  
> **Last Updated**: 2026-09-23  

## Context

**GDD**: [`design/gdd/stagger-system.md`](../../design/gdd/stagger-system.md), [`design/gdd/combat-system.md`](../../design/gdd/combat-system.md)  
**Requirement**: `TR-stgr-001` (Posture break 3.0s kneels, True Damage Execution 25% Max HP, Motion Warping, Posture decay 20/s after 4.0s)

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md)
- [`ADR-0002: GAS Integration & PaperZD Pixel Sprites`](../../docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Implements `FPAStaggerModel` / `UPAStaggerHandlerComponent` with GAS Posture tracking and execution resolution.

---

## Acceptance Criteria

- [x] **AC-1 (Posture Break Trigger & Kneel State)**: Khi Posture của mục tiêu đạt $100\%$ ($Posture \ge MaxPosture$), mục tiêu lập tức bị hủy toàn bộ hoạt ảnh, quỳ gối rơi vào trạng thái `State.Staggered` trong đúng **3.00 giây**.
- [x] **AC-2 (25% True Damage Execution)**: Khi người chơi ở trong phạm vi $\le 250\text{cm}$ và kích hoạt lệnh kết liễu:
  - Gây chính xác **25% Max HP của Trùm** dưới dạng **True Damage** (bỏ qua mọi chỉ số giáp/kháng).
  - Cấp trạng thái bất tử `State.Invulnerable` cho người chơi trong **1.20 giây**.
  - Sau khi kết liễu thành công, trùm nhận **2.00s Miễn nhiễm Posture** (`State.PostureImmune`) để tránh bị lặp vô tận (stagger-lock).
- [x] **AC-3 (Posture Decay Logic)**: Nếu mục tiêu không phải nhận bất kỳ sát thương nào trong **4.00 giây** (`PostureDecayDelay`), Posture sẽ tự suy giảm với tốc độ **20 điểm/giây** (`PostureDecayRate`). Bất kỳ đòn đánh nào trúng đích đều reset lại bộ đếm delay về 0.
- [x] **AC-4 (Missed Window Recovery Shockwave)**: Nếu hết thời gian 3.0s mà không có ai thực hiện đòn kết liễu, trùm tự đứng dậy phát sóng xung kích hất văng người chơi trong bán kính 300cm, và hoàn lại **50% Posture** (thay vì về 0).

---

## Implementation Notes

1. **`PAStaggerTypes.h`**:
   - `EPAStaggerState`: `Normal`, `StaggeredKneel`, `PostureImmune`.
   - `FPAStaggerConfig`: StaggerDuration (3.0s), DecayDelay (4.0s), DecayRate (20.0/s), ExecutionRange (250cm), ExecutionHPPct (0.25f), ExecutionInvulnDuration (1.2s), PostureImmunityDuration (2.0s), MissedRefundPct (0.50f).
   - `FPAStaggerModel`: Pure data model quản lý Posture accumulation, decay timers, kneel countdown, execution validation, và recovery shockwave.
2. **`PAStaggerComponent.h` / `PAStaggerComponent.cpp`**:
   - ActorComponent tích hợp GAS để áp dụng sát thương True Damage lên `UAscendantAttributeSet`.

---

## QA Test Cases

- **Test 1: Posture Stagger Trigger**: Tích lũy đủ 100% Posture -> mục tiêu rơi vào trạng thái StaggeredKneel đúng 3.0s.
- **Test 2: Execution Resolution**: Ở cự ly 200cm (<250cm), thực thi Execution -> trừ đúng 25% Max HP, cấp 1.2s I-Frame, cấp 2.0s PostureImmune.
- **Test 3: Posture Decay**: Sau 3.9s không bị đánh -> Posture chưa giảm; tại 4.5s -> Posture giảm đúng 10 điểm (0.5s * 20/s). Bị đánh ở 2.0s -> reset delay.
- **Test 4: Missed Execution Refund**: Để trôi hết 3.0s -> trùm hoàn lại 50% Posture, thoát Stagger.
