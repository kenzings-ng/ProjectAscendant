# Story 004: Dash I-Frame, Perfect Dodge Sweet Spot & Hitstop

> **Epic**: Encounter & Boss Mechanics Layer  
> **Status**: In Progress  
> **X14 (2026-10-10) — đối chiếu trạng thái** (trước đây ghi `✅ Done`): Bằng chứng: `Combat.DashIFramePerfectDodge` (model `FPADashModel`) trong `Tests/evidence/x11b-e34f428-ue-automation.log` (77/77 PASS). AC-2, AC-3, AC-4 chưa triển khai ở runtime (ghi chú X10); AC-1 model còn bộ số cũ; `UPADashEvasionComponent` không gắn vào actor nào (M6). Bảng tổng: `production/qa/x14-status-reconciliation.md`.  
> **Layer**: Combat / Movement  
> **Type**: Gameplay / Mechanics  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-23  
> **Last Updated**: 2026-10-09 (X10: đồng bộ thông số Dash theo code runtime, DECISIONS.md §12)  

## Context

**GDD**: [`design/gdd/dash-evasion.md`](../../design/gdd/dash-evasion.md), [`design/gdd/combat-system.md`](../../design/gdd/combat-system.md)  
**Requirement**: `TR-dash-001` (Dash phases: 0.35s total, 0.20s I-Frame từ t = 0.05s đến 0.25s, 0.05-0.15s Perfect Dodge sweet spot, +15 Stamina refund, 0.08s hitstop)

> **Ghi chú X10 (2026-10-09)**: Theo DECISIONS.md §12, giá trị code runtime `UPAGameplayAbility_Dash` (`PAGameplayAbility_Dash.h:30-36, 144-166`) là chuẩn: 0.35s, 450cm, I-frame 0.05s–0.25s, hồi chiêu 0.5s, 25 Stamina, chặn khi Stamina < 25. Story này hiện thực `FPADashModel` / `UPADashEvasionComponent` với bộ số cũ (`PADashTypes.h:37-83`: 0.45s, I-frame 0.00s–0.28s); hai lớp này **không được dùng ở runtime** (không actor nào gắn `UPADashEvasionComponent`). Các tiêu chí đánh dấu [x] dưới đây được kiểm bằng test của `FPADashModel`, không phải của ability runtime.

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md)
- [`ADR-0002: GAS Integration & PaperZD Pixel Sprites`](../../docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `FPADashModel` / `UPADashEvasionComponent` with precise timeline tick verification and hitstop management.

---

## Acceptance Criteria

- [x] **AC-1 (Dash Timeline & Absolute I-Frame)**: *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*
  - Tổng thời gian lướt cố định ở **0.35 giây**, tiêu tốn **25 Stamina**, quãng đường di chuyển **450cm** (giá trị code runtime; trước 2026-10-09 ghi 0.45s / ≈380cm).
  - Từ $t = 0.05\text{s}$ đến $t = 0.25\text{s}$ (0.20s): Nhân vật sở hữu thẻ bất tử `State.Invulnerable`, miễn nhiễm hoàn toàn mọi sát thương và khống chế (trước 2026-10-09 ghi $t \in [0.00, 0.28]$).
  - Từ $t = 0.25\text{s} \to 0.35\text{s}$: Trở về trạng thái bình thường (hồi phục), có thể bị đánh trúng. Kết thúc lướt gán `Cooldown.Dash` 0.5s.
  - *Lưu ý: `FPADashModel` vẫn dùng 0.45s / 0.28s; cần task code riêng để hợp nhất với ability runtime.*
- [x] **AC-2 (Perfect Dodge Sweet Spot & Rewards)** — chưa triển khai trong code (theo quyết định 2026-10-09: giá trị code là chuẩn) (chỉ có trong `FPADashModel`): *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*
  - Khi đòn đánh của kẻ địch quét trúng capsule trong khung thời gian vàng **0.05s đến 0.15s** của cú lướt:
  - Hoàn trả ngay lập tức **+15 Stamina** (chi phí thực tế giảm từ $25 \to 10$).
  - Kích hoạt **Hitstop 0.08 giây** (làm chậm thời gian môi trường/quái vật $10\times$ với TimeDilation = 0.1, người chơi giữ nguyên TimeDilation = 1.0).
  - Cấp trạng thái `State.PerfectDodgeTriggered` trong 2.0s để kích hoạt đòn phản công.
- [x] **AC-3 (Ledge-Fall Prevention)**: Trong suốt toàn bộ thời gian của cú lướt (0.35s theo code runtime; trước 2026-10-09 ghi 0.45s), nhân vật không bao giờ bị rơi khỏi mép vực (`bCanWalkOffLedges = false`). *(chưa triển khai trong code (theo quyết định 2026-10-09: giá trị code là chuẩn): `UPAGameplayAbility_Dash` không đặt cờ này.)* *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*
- [x] **AC-4 (Dash Attack Cancel)**: Từ mốc thời gian $t = 0.35\text{s}$, người chơi có thể nhấn nút Đánh để hủy 0.10s hồi phục còn lại và chuyển ngay lập tức sang đòn Dash Attack. *(chưa triển khai trong code (theo quyết định 2026-10-09: giá trị code là chuẩn); mốc 0.35s–0.45s dựa trên cú lướt 0.45s cũ và cần định lại cho cú lướt 0.35s.)* *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*

---

## Implementation Notes

1. **`PADashTypes.h`**:
   - `EPADashPhase`: `None`, `IFramePeak`, `Recovery`, `AttackCancelable`.
   - `FPADashConfig` (giữ bộ số cũ, không dùng ở runtime — xem ghi chú X10): TotalDuration (0.45s), IFrameDuration (0.28s), PerfectDodgeStart (0.05s), PerfectDodgeEnd (0.15s), StaminaCost (25.0f), StaminaRefund (15.0f), HitstopDuration (0.08s), HitstopDilation (0.1f), CancelWindowStart (0.35s).
   - `FPADashModel`: Pure data model quản lý tiến trình lướt, kiểm tra trúng đòn trong sweet-spot, tính toán hoàn thể lực và hitstop.
2. **`PADashEvasionComponent.h` / `PADashEvasionComponent.cpp`**:
   - ActorComponent bọc data model và tương tác với `CharacterMovementComponent` và `UAbilitySystemComponent`.

---

## QA Test Cases

*Ghi chú X10 (2026-10-09): các test dưới đây chạy trên `FPADashModel` (bộ số cũ). Với ability runtime, mốc I-frame là 0.05s–0.25s: tại t=0.20s vẫn có I-Frame, tại t=0.30s đã mất I-Frame; t=0.22s vẫn trong I-frame.*

- **Test 1: Dash Timeline**: Tại t=0.20s -> có I-Frame (Invulnerable = true); tại t=0.30s -> mất I-Frame (Invulnerable = false).
- **Test 2: Perfect Dodge Sweet Spot**: Bị đánh tại t=0.10s -> kích hoạt Perfect Dodge, hoàn +15 Stamina, kích hoạt Hitstop 0.08s.
- **Test 3: Non-Sweet Spot Dodge**: Bị đánh tại t=0.22s (vẫn trong I-frame nhưng ngoài sweet-spot) -> né thành công không mất máu, nhưng KHÔNG nhận thưởng Perfect Dodge.
- **Test 4: Dash Attack Cancel**: Tại t=0.36s gọi lệnh Attack -> thành công hủy phục hồi chuyển sang Dash Attack.
