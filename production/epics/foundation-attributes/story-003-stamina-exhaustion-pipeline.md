# Story 003: Stamina Regeneration, Depletion & Exhaustion State Pipeline

> **Epic**: Character Attributes & Stats Engine (GAS)  
> **Status**: Complete  
> **Layer**: Foundation  
> **Type**: Logic  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-09-16  

## Context

**GDD**: [`design/gdd/attributes-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/attributes-system.md)  
**Requirement**: `TR-attr-003`  
*(Requirement text lives in `docs/architecture/tr-registry.yaml` — Client-side optimistic resource prediction with server-authoritative reconciliation on mispredicted depletion)*  

**ADR Governing Implementation**: [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) (Primary), [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Secondary)  
**ADR Decision Summary**: Establishes client-side optimistic prediction for stamina consumption during dash and attacks, with server-authoritative reconciliation, handling recovery delays, exhaustion tags, and desperation roll penalties.

**Engine**: Unreal Engine 5.7 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Uses GAS tag-based ability blocking (`Ability.Block.Dash`) and periodic timer delegates for recovery to eliminate per-frame actor tick overhead.

**Control Manifest Rules (Foundation Layer)**:
- Required: Stamina deduction must predict locally and reconcile with server; Exhaustion must block dash abilities via GameplayTags.
- Forbidden: Never poll stamina regeneration inside `Tick()`; Use timer managers or GAS periodic effects.
- Guardrail: Recovery timer precision $\le 10\text{ms}$ under 60Hz tick rate.

---

## Acceptance Criteria

*From GDD `design/gdd/attributes-system.md`, scoped to this story:*

- [x] **AC-1 (Stamina Consumption & Desperation Roll)**: Dashing consumes 25 Stamina (`StaminaCost_Dash = 25.0f`). If Stamina is $>0$ but $<25$, a "Desperation Roll" is granted, deducting all remaining Stamina and setting an extended exhaustion penalty.
- [x] **AC-2 (Regeneration Delay & Rate)**: Following any stamina consumption, regeneration pauses for 0.6 seconds (`StaminaRegenDelay = 0.6s`), after which Stamina regenerates at 45.0 points per second (`StaminaRegenRate = 45.0/s`) until reaching `MaxStamina`.
- [x] **AC-3 (Exhaustion State & Penalty)**: When Stamina reaches 0, apply `GameplayTag.State.Exhausted`, which:
  - Reduces `MoveSpeed` by $25\%$
  - Blocks dash activation (`Ability.Block.Dash`)
  - Lasts for 1.5 seconds (or 2.2 seconds if triggered via Desperation Roll) until Stamina exceeds $30\%$.

---

## Implementation Notes

*Derived from ADR-0002 Implementation Guidelines:*

1. **Stamina Manager (`UPAStaminaComponent` or `UAscendantAttributeSet`)**:
   - Manage `FTimerHandle StaminaRegenDelayTimer;`
   - Whenever Stamina is decremented: clear regen timer and start 0.6s delay:
     `GetWorld()->GetTimerManager().SetTimer(StaminaRegenDelayTimer, this, &UPAStaminaComponent::StartStaminaRegen, 0.6f, false);`
   - Active regeneration utilizes a 0.05s periodic timer incrementing `Stamina += StaminaRegenRate * DeltaTime`.
2. **Exhaustion Tag & GameplayEffect**:
   - Create `GE_ExhaustedState`: applies `State.Exhausted` and `Ability.Block.Dash`, multiplies `MoveSpeed` by 0.75.
   - If Stamina hits 0 via standard action: duration is 1.5s. If triggered when starting Stamina was $<25.0f$: duration is 2.2s.
   - When Stamina recovers past $30.0f$ and duration expires, remove `GE_ExhaustedState`.

---

## Out of Scope

*Handled by neighbouring stories — do not implement here:*

- Story 001: UAttributeSet definition and network replication.
- Story 002: Damage and posture execution calculations.

---

## QA Test Cases

*Written by qa-lead at story creation:*

- **AC-1 Test: Standard Dash vs Desperation Roll**:
  - Given: Character has 100 Stamina.
  - When: Player triggers Dash.
  - Then: Stamina decreases to 75.0f; no exhaustion tag applied.
  - When 2: Character has 10 Stamina and triggers Dash.
  - Then 2: Dash succeeds (I-frame granted), Stamina drops to 0, and `State.Exhausted` applies with 2.2s duration.

- **AC-2 Test: Regeneration Timing**:
  - Given: Character Stamina dropped from 100 to 50 at $T=0$.
  - When: Advancing time to $T=0.5\text{s}$ ($< 0.6\text{s}$).
  - Then: Stamina remains 50.0f.
  - When 2: Advancing time to $T=1.6\text{s}$ (1.0s of active regeneration after 0.6s delay).
  - Then 2: Stamina asserts at $50 + (45 \times 1.0) = 95.0\text{f}$.

- **AC-3 Test: Exhaustion Penalties & Dash Lock**:
  - Given: Character in `State.Exhausted`.
  - When: Client attempts to activate Dash ability.
  - Then: GAS rejects ability activation (`Ability.Block.Dash`), and `MoveSpeed` evaluates to $412.5\text{ cm/s}$ ($550 \times 0.75$).

---

## Test Evidence

**Story Type**: Logic  
**Required evidence**: `tests/unit/combat/stamina_exhaustion_test.cpp` — must exist and pass automated CI  
**Status**: [x] Created and passes automated test suite (`tests/unit/combat/stamina_exhaustion_test.cpp` — 4 test suites, 16 assertions)  

---

## Dependencies

- Depends on: Story 001 (AttributeSet Definition)
- Unlocks: Core Dash Evasion & Core Combat Stories

---

## Completion Notes

**Completed**: 2026-09-16  
**Criteria**: 3/3 passing (AC-1, AC-2, AC-3)  
**Deviations**: None  
**Test Evidence**: Logic: [`tests/unit/combat/stamina_exhaustion_test.cpp`](file:///mnt/Data/Projects/project-games/tests/unit/combat/stamina_exhaustion_test.cpp) (16 assertions — PASS)  
**Code Review**: Complete — APPROVED (by lead-programmer & ue-gas-specialist)  

