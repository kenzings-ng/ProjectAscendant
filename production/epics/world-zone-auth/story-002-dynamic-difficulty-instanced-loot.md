# Story 002: Dynamic Difficulty Scaling & Contested Instanced Loot

> **Epic**: World Integration, Sanctuaries & Account Authentication Layer  
> **Status**: In Progress  
> **X14 (2026-10-10) — đối chiếu trạng thái** (trước đây ghi `✅ Done`): Bằng chứng: `Network.DifficultyScalingLoot` (model) trong `Tests/evidence/x11b-e34f428-ue-automation.log` (77/77 PASS). Rà soát B4: PARTIAL/CONTRADICTS. Không có caller runtime (B4-8); AC-1 hệ số 0.50 lệch `multiplayer-coop.md` 0.45 (B4-16, chờ chủ dự án); AC-4 loot instanced dựa vào relevancy mà Iris bỏ qua (B4-14). Bảng tổng: `production/qa/x14-status-reconciliation.md`.  
> **Layer**: Combat / Netcode  
> **Type**: Gameplay / Netcode  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-23  
> **Last Updated**: 2026-09-23  

## Context

**GDD**: [`design/gdd/zone-system.md`](../../design/gdd/zone-system.md), [`design/gdd/multiplayer-coop.md`](../../design/gdd/multiplayer-coop.md)  
**Requirement**: `TR-zone-002` (Boss HP/Posture scaling formulas, Anti-Zerg CC reduction, 5% HP / 10% Posture loot threshold)

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md)
- [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](../../docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `FPADifficultyScalingModel` and `FPAInstancedLootTracker`.

---

## Acceptance Criteria

- [x] **AC-1 (Dynamic HP Scaling Formula)**: Lượng máu tối đa của Boss tự động co giãn theo số lượng người chơi tham chiến trong bán kính 3000cm: *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*
  $$\text{ScaledMaxHP} = \text{BaseMaxHP} \times (1.0 + 0.50 \times \max(0, N - 1))$$
  - Ví dụ: 1 người $\to 10,000\text{ HP}$; 4 người $\to 25,000\text{ HP}$.
- [x] **AC-2 (Dynamic Posture Scaling Formula)**: Thanh Posture tối đa của Boss tự động co giãn theo số người tham chiến:
  $$\text{ScaledMaxPosture} = \text{BaseMaxPosture} \times (1.0 + 0.35 \times \max(0, N - 1))$$
  - Ví dụ: 1 người $\to 800\text{ Posture}$; 4 người $\to 1,640\text{ Posture}$.
- [x] **AC-3 (Anti-Zerg Crowd Control Reduction)**:
  - Khi $N \ge 4$ người tham chiến: Boss nhận kháng khống chế $+30\%$ (giảm thời gian choáng/knockback 30%).
  - Khi $N \ge 8$ người tham chiến: Boss nhận kháng khống chế $+50\%$ và tăng tốc độ xoay chuyển hướng đòn đánh $+25\%$.
- [x] **AC-4 (Instanced Loot Contribution Threshold)**: Chỉ những người chơi hoặc tổ đội gây ra: *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*
  - $\ge 5\%$ Tổng sát thương Máu ($\text{DamageHP} \ge 0.05 \times \text{ScaledMaxHP}$) **HOẶC**
  - $\ge 10\%$ Tổng sát thương Phá thế ($\text{DamagePosture} \ge 0.10 \times \text{ScaledMaxPosture}$)
  - Mới đủ tư cách nhận hòm đồ rơi cá nhân riêng biệt (Instanced Loot). Người chơi dưới ngưỡng không nhận được bất kỳ vật phẩm nào (loại bỏ hoàn toàn vấn nạn đánh hôi).

---

## Implementation Notes

1. **`PADifficultyScalingTypes.h`**:
   - `FPADDSConfig`: HPScaleCoeff (0.50), PostureScaleCoeff (0.35), CombatantRadius (3000cm), MinContributionHPPct (0.05), MinContributionPosturePct (0.10).
   - `FPACombatantRecord`: PlayerId, TotalHPDamage, TotalPostureDamage.
   - `FPADifficultyScalingModel`: Pure data model tính toán Scaled HP, Scaled Posture, CC reduction, theo dõi đóng góp và xuất danh sách người chơi đủ điều kiện nhận Loot.

---

## QA Test Cases

- **Test 1: HP Scaling Math**: Với 1 người -> 10,000 HP; với 4 người -> đúng 25,000 HP.
- **Test 2: Posture Scaling Math**: Với 1 người -> 800 Posture; với 4 người -> đúng 1,640 Posture.
- **Test 3: Anti-Zerg CC Resistance**: Với 3 người -> 0% CC resist; 5 người -> 30% CC resist; 9 người -> 50% CC resist.
- **Test 4: Loot Qualification**: Người chơi gây 6% HP damage -> đủ điều kiện nhận loot; người chơi gây 3% HP và 5% Posture -> bị loại (không đủ điều kiện).
