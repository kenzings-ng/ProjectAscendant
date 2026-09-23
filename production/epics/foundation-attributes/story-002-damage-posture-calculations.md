# Story 002: Non-Linear Damage & Posture Execution Calculations

> **Epic**: Character Attributes & Stats Engine (GAS)  
> **Status**: Complete  
> **Layer**: Foundation  
> **Type**: Logic  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-09-16  

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

- [x] **AC-1 (Effective Damage Calculation)**: Implement `UGEC_DamageCalculation` using formula:
  $$\text{DamageTaken} = \text{RawDamage} \times \left( \frac{100}{100 + \text{Armor}} \right)$$
  If target has `GameplayTag.State.Invulnerable`, $\text{DamageTaken}$ is forced to 0.0f.
- [x] **AC-2 (Posture Damage Calculation)**: Implement `UGEC_PostureCalculation` using formula:
  $$\text{PostureDamage} = \text{BaseStagger} \times (1 + \text{StaggerBonus}) \times \text{HitMultiplier}$$
  Applying $\text{HitMultiplier} = 1.5$ on weakspots, and instantly applying $35\%$ of target's $\text{MaxPosture}$ on a Perfect Parry.
- [x] **AC-3 (Stagger Execution Damage)**: When executing a staggered boss, calculate execution damage as:
  $$\text{ExecuteDamage} = (\text{TargetMaxHP} \times 0.25) + (\text{BaseDamage} \times 3.0)$$
  bypassing standard armor reduction to guarantee boss defeat in 4 clean execution cycles.

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
**Status**: [x] Created and passes automated test suite (`tests/unit/combat/damage_execution_calc_test.cpp` — 4 test suites, 16 assertions)  

---

## Dependencies

- Depends on: Story 001 (AttributeSet Definition)
- Unlocks: Story 003 (Stamina Pipeline) & Core Combat Stories

---

## Completion Notes

**Completed**: 2026-09-16  
**Criteria**: 3/3 passing (AC-1, AC-2, AC-3)  
**Deviations**: None  
**Test Evidence**: Logic: [`tests/unit/combat/damage_execution_calc_test.cpp`](file:///mnt/Data/Projects/project-games/tests/unit/combat/damage_execution_calc_test.cpp) (16 assertions — PASS)  
**Code Review**: Complete — APPROVED (by lead-programmer & ue-gas-specialist)  

