# Story 002: Non-Linear Damage & Posture Execution Calculations

> **Epic**: Character Attributes & Stats Engine (GAS)  
> **Status**: In Progress  
> **X14 (2026-10-10) — đối chiếu trạng thái** (trước đây ghi `Complete`): Bằng chứng: `Foundation.Combat.DamageExecutionCalculations` trong `Tests/evidence/x11b-e34f428-ue-automation.log` (77/77 PASS). AC-1/AC-2/AC-3 chỉ đạt ở helper `UPADamageExecutionCalculation`; đòn cận chiến runtime không đi qua execution calc này (M11), nhánh `State.Staggered` không chạy vì tag chưa đăng ký (M10), công thức kết liễu runtime khác AC-3 (ghi chú X10). Bảng tổng: `production/qa/x14-status-reconciliation.md`.  
> **Layer**: Foundation  
> **Type**: Logic  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-10-09 (X10: ghi chú công thức kết liễu theo code runtime)  

## Context

**GDD**: [`design/gdd/attributes-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/attributes-system.md)  
**Requirement**: `TR-attr-002`  
*(Requirement text lives in `docs/architecture/tr-registry.yaml` — Stat calculation pipelines using UGameplayEffectExecutionCalculation for non-linear defense and resistance scaling)*  

**ADR Governing Implementation**: [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md)  
**ADR Decision Summary**: Mandates custom `UGameplayEffectExecutionCalculation` classes for all combat interactions, decoupling math formulas from Actor classes and executing all reductions on the Dedicated Server.

**Engine**: Unreal Engine 5.7 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Uses GAS execution captures (`DECLARE_ATTRIBUTE_CAPTUREDEF`) for source attack power, target armor, and target posture.

**Control Manifest Rules (Foundation Layer)**:
- Required: Damage formulas and posture calculations must execute via server-authoritative GameplayEffect execution calculations.
- Forbidden: Never compute final damage on client proxies; Never bypass armor reduction curves.
- Guardrail: Execution calculation time $\le 0.1\text{ms}$ per hit on server game thread.

---

## Acceptance Criteria

*From GDD `design/gdd/attributes-system.md`, scoped to this story:*

- [x] **AC-1 (Effective Damage Calculation)**: Implement `UGEC_DamageCalculation` using formula: *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*
  $$\text{DamageTaken} = \text{RawDamage} \times \left( \frac{100}{100 + \text{Armor}} \right)$$
  If target has `GameplayTag.State.Invulnerable`, $\text{DamageTaken}$ is forced to 0.0f.
- [x] **AC-2 (Posture Damage Calculation)**: Implement `UGEC_PostureCalculation` using formula: *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*
  $$\text{PostureDamage} = \text{BaseStagger} \times (1 + \text{StaggerBonus}) \times \text{HitMultiplier}$$
  Applying $\text{HitMultiplier} = 1.5$ on weakspots, and instantly applying $35\%$ of target's $\text{MaxPosture}$ on a Perfect Parry.
- [x] **AC-3 (Stagger Execution Damage)**: When executing a staggered boss, calculate execution damage as: *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*
  $$\text{ExecuteDamage} = (\text{TargetMaxHP} \times 0.25) + (\text{BaseDamage} \times 3.0)$$
  bypassing standard armor reduction to guarantee boss defeat in 4 clean execution cycles.
  - *X10 (2026-10-09, DECISIONS.md §12): the runtime finisher `UPAGameplayAbility_Finisher` deals exactly `TargetMaxHP × 0.25` (`PAGameplayAbility_Finisher.h:95`, `PAGameplayAbility_MeleeAttack.cpp:104-108`) and is authoritative. The `+ (BaseDamage × 3.0)` term above exists only in `UPADamageExecutionCalculation` (`PADamageExecutionCalculation.cpp:64`), which the finisher does not call at runtime; `attributes-system.md` §3 now states the code formula. This [x] reflects the helper's tests, not runtime behaviour.*

---

## Implementation Notes

*Derived from ADR-0002 Implementation Guidelines:*

1. **Capture Definitions (`FPACombatDamageStatics`)**:
   - Source: `RawAttackPower`, `BaseStagger`, `StaggerBonus`.
   - Target: `Armor`, `Posture`, `MaxPosture`, `Health`, `MaxHealth`.
   - Tags: `State.Invulnerable`, `State.Staggered`, `Combat.WeakspotHit`, `Combat.ParryCounter`.
2. **Execution Calculation Structure (`UPADamageExecutionCalculation`)**:
   - Extract captured attributes using `ExecutionParams.AttemptCalculateCapturedAttributeMagnitude`.
   - Check target tags: `if (TargetTags->HasTag(FGameplayTag::RequestGameplayTag("State.Invulnerable"))) return;`
   - Apply clamped mitigation and output an evaluated modifier directly to `Health` and `Posture`.

---

## Out of Scope

*Handled by neighbouring stories — do not implement here:*

- Story 001: AttributeSet declaration and network replication.
- Story 003: Stamina consumption, recovery delay, and exhaustion state.

---

## QA Test Cases

*Written by qa-lead at story creation:*

- **AC-1 Test: Armor Mitigation & I-Frame Immunity**:
  - Given: Attacker has 200 Raw Damage. Target has 100 Armor.
  - When: Damage execution calculation runs.
  - Then: Asserts `DamageTaken == 100.0f` ($200 \times \frac{100}{200} = 100$).
  - When 2: Target is granted `State.Invulnerable` tag and hit with 500 Raw Damage.
  - Then 2: Asserts `DamageTaken == 0.0f`.

- **AC-2 Test: Posture Damage & Perfect Parry Multiplier**:
  - Given: Target boss has 1,000 Max Posture. Attacker delivers a Perfect Parry.
  - When: Posture calculation executes.
  - Then: Target's Posture increases by exactly 350 points (35% of 1,000).

- **AC-3 Test: Stagger Execution Formula**:
  - Given: Boss with 20,000 Max HP. Attacker executes Stagger Finisher with 100 Base Damage.
  - When: Execution calculation runs.
  - Then: Asserts damage dealt is $(20000 \times 0.25) + (100 \times 3.0) = 5000 + 300 = 5300\text{ HP}$.

---

## Test Evidence
 
**Story Type**: Logic  
**Required evidence**: `tests/unit/combat/damage_execution_calc_test.cpp` — must exist and pass automated CI  
**Status**: [x] Created and passes automated test suite (`tests/unit/combat/damage_execution_calc_test.cpp` — 4 test suites, 16 assertions) *(X14 2026-10-10: tuyên bố PASS/Complete này không có bằng chứng tại thời điểm ghi — test ở `Tests/` gốc chưa được biên dịch cho đến X12 (rà soát R4), và đường dẫn đã chuyển sang `Source/ProjectAscendant/Private/Tests/`. Trạng thái thật: xem dòng X14 ở đầu file.)*  
---

## Dependencies

- Depends on: Story 001 (AttributeSet Definition)
- Unlocks: Story 003 (Stamina Pipeline) & Core Combat Stories

---

## Completion Notes
> **X14 (2026-10-10)**: khối dưới đây là lịch sử, không còn đúng. Story hiện là `In Progress`; xem dòng X14 ở đầu file.


**Completed**: 2026-09-16  
**Criteria**: 3/3 passing (AC-1, AC-2, AC-3)  
**Deviations**: None  
**Test Evidence**: Logic: [`tests/unit/combat/damage_execution_calc_test.cpp`](file:///mnt/Data/Projects/project-games/tests/unit/combat/damage_execution_calc_test.cpp) (16 assertions — PASS)  
**Code Review**: Complete — APPROVED (by lead-programmer & ue-gas-specialist)  

