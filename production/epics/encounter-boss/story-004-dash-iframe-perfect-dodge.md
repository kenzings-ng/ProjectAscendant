# Story 004: Dash I-Frame, Perfect Dodge Sweet Spot & Hitstop

> **Epic**: Encounter & Boss Mechanics Layer  
> **Status**: Ready  
> **Layer**: Combat / Movement  
> **Type**: Gameplay / Mechanics  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-23  
> **Last Updated**: 2026-09-23  

## Context

**GDD**: [`design/gdd/dash-evasion.md`](../../design/gdd/dash-evasion.md), [`design/gdd/combat-system.md`](../../design/gdd/combat-system.md)  
**Requirement**: `TR-dash-001` (Dash phases: 0.45s total, 0.28s I-Frame, 0.05-0.15s Perfect Dodge sweet spot, +15 Stamina refund, 0.08s hitstop)

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md)
- [`ADR-0002: GAS Integration & PaperZD Pixel Sprites`](../../docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `FPADashModel` / `UPADashEvasionComponent` with precise timeline tick verification and hitstop management.

---

## Acceptance Criteria

- [ ] **AC-1 (Dash Timeline & Absolute I-Frame)**:
  - Tổng thời gian lướt cố định ở **0.45 giây**, tiêu tốn **25 Stamina**, quãng đường di chuyển $\approx 380\text{cm}$.
  - Trong **0.28 giây đầu tiên** ($t \in [0.00, 0.28]$): Nhân vật sở hữu thẻ bất tử `State.Invulnerable`, miễn nhiễm hoàn toàn mọi sát thương và khống chế.
  - Từ $t = 0.28\text{s} \to 0.45\text{s}$: Trở về trạng thái bình thường (hồi phục), có thể bị đánh trúng.
- [ ] **AC-2 (Perfect Dodge Sweet Spot & Rewards)**:
  - Khi đòn đánh của kẻ địch quét trúng capsule trong khung thời gian vàng **0.05s đến 0.15s** của cú lướt:
  - Hoàn trả ngay lập tức **+15 Stamina** (chi phí thực tế giảm từ $25 \to 10$).
  - Kích hoạt **Hitstop 0.08 giây** (làm chậm thời gian môi trường/quái vật $10\times$ với TimeDilation = 0.1, người chơi giữ nguyên TimeDilation = 1.0).
  - Cấp trạng thái `State.PerfectDodgeTriggered` trong 2.0s để kích hoạt đòn phản công.
- [ ] **AC-3 (Ledge-Fall Prevention)**: Trong suốt toàn bộ thời gian 0.45s của cú lướt, nhân vật không bao giờ bị rơi khỏi mép vực (`bCanWalkOffLedges = false`).
- [ ] **AC-4 (Dash Attack Cancel)**: Từ mốc thời gian $t = 0.35\text{s}$, người chơi có thể nhấn nút Đánh để hủy 0.10s hồi phục còn lại và chuyển ngay lập tức sang đòn Dash Attack.

---

## Implementation Notes

1. **`PADashTypes.h`**:
   - `EPADashPhase`: `None`, `IFramePeak`, `Recovery`, `AttackCancelable`.
   - `FPADashConfig`: TotalDuration (0.45s), IFrameDuration (0.28s), PerfectDodgeStart (0.05s), PerfectDodgeEnd (0.15s), StaminaCost (25.0f), StaminaRefund (15.0f), HitstopDuration (0.08s), HitstopDilation (0.1f), CancelWindowStart (0.35s).
   - `FPADashModel`: Pure data model quản lý tiến trình lướt, kiểm tra trúng đòn trong sweet-spot, tính toán hoàn thể lực và hitstop.
2. **`PADashEvasionComponent.h` / `PADashEvasionComponent.cpp`**:
   - ActorComponent bọc data model và tương tác với `CharacterMovementComponent` và `UAbilitySystemComponent`.

---

## QA Test Cases

- **Test 1: Dash Timeline**: Tại t=0.20s -> có I-Frame (Invulnerable = true); tại t=0.30s -> mất I-Frame (Invulnerable = false).
- **Test 2: Perfect Dodge Sweet Spot**: Bị đánh tại t=0.10s -> kích hoạt Perfect Dodge, hoàn +15 Stamina, kích hoạt Hitstop 0.08s.
- **Test 3: Non-Sweet Spot Dodge**: Bị đánh tại t=0.22s (vẫn trong I-frame nhưng ngoài sweet-spot) -> né thành công không mất máu, nhưng KHÔNG nhận thưởng Perfect Dodge.
- **Test 4: Dash Attack Cancel**: Tại t=0.36s gọi lệnh Attack -> thành công hủy phục hồi chuyển sang Dash Attack.
