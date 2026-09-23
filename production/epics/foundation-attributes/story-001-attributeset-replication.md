# Story 001: Core AttributeSet Definition & Iris Network Replication

> **Epic**: Character Attributes & Stats Engine (GAS)  
> **Status**: Complete  
> **Layer**: Foundation  
> **Type**: Logic  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-09-16  

## Context

**GDD**: [`design/gdd/attributes-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/attributes-system.md)  
**Requirement**: `TR-attr-001`  
*(Requirement text lives in `docs/architecture/tr-registry.yaml` — Character stats (Health, Mana, Stamina, Posture) implemented via GAS UAttributeSet with Iris network replication)*  

**ADR Governing Implementation**: [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) (Primary), [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Secondary)  
**ADR Decision Summary**: Integrates GAS with PaperZD character pawns, standardizing attribute replication macros and server authority over resource modification.

**Engine**: Unreal Engine 5.7 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Implements `GAMEPLAYATTRIBUTE_REPNOTIFY` and registers properties with Iris replication condition macros (`DOREPLIFETIME_CONDITION_NOTIFY`).

**Control Manifest Rules (Foundation Layer)**:
- Required: All gameplay attributes must reside in a `UAttributeSet` and replicate via `GAMEPLAYATTRIBUTE_REPNOTIFY`; Dedicated Server owns 100% of game state.
- Forbidden: Never permit client-side mutation of persistent attributes without server authority.
- Guardrail: GAS execution overhead $\le 1.2\text{ms}$ on game thread per frame.

---

## Acceptance Criteria

*From GDD `design/gdd/attributes-system.md`, scoped to this story:*

- [x] **AC-1 (Core Attributes Definition)**: Class `UAscendantAttributeSet` declares `Health`, `MaxHealth`, `Mana`, `MaxMana`, `Stamina`, `MaxStamina`, `Posture`, `MaxPosture`, `MoveSpeed`, and `IFrameDuration` using standard GAS accessor macros (`ATTRIBUTE_ACCESSORS`).
- [x] **AC-2 (Value Clamping)**: `PreAttributeChange` clamps `Health` to $[0, \text{MaxHealth}]$, `Mana` to $[0, \text{MaxMana}]$, `Stamina` to $[0, \text{MaxStamina}]$, and `Posture` to $[0, \text{MaxPosture}]$.
- [x] **AC-3 (Network Replication)**: All attributes implement `OnRep_` callbacks using `GAMEPLAYATTRIBUTE_REPNOTIFY` and synchronize from Dedicated Server to Autonomous and Simulated Proxies under Iris replication without hitching.

---

## Implementation Notes

*Derived from ADR-0002 Implementation Guidelines:*

1. **Class Declaration (`UAscendantAttributeSet`)**:
   - Header: `Source/ProjectAscendant/Public/Combat/AscendantAttributeSet.h`
   - Use macro `ATTRIBUTE_ACCESSORS(UAscendantAttributeSet, PropertyName)` for each attribute.
   - Implement `GetLifetimeReplicatedProps` with `DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, Health, COND_None, REPNOTIFY_Always)`.
2. **PostGameplayEffectExecute & PreAttributeChange**:
   - In `PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)`: enforce mathematical boundaries.
   - In `PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)`: broadcast death delegate if `GetHealth() <= 0.0f`, broadcast posture break if `GetPosture() >= GetMaxPosture()`.

---

## Out of Scope

*Handled by neighbouring stories — do not implement here:*

- Story 002: Execution calculations for armor reduction and posture break scaling.
- Story 003: Stamina recovery timers, exhaustion status tags, and desperation rolls.

---

## QA Test Cases

*Written by qa-lead at story creation:*

- **AC-1 Test: Attribute Initialization**:
  - Given: A newly spawned character actor with `UAscendantAttributeSet` initialized from default DataTable.
  - When: Initial stats are queried.
  - Then: Asserts `Health == 500.0f`, `MaxHealth == 500.0f`, `Stamina == 100.0f`, `Posture == 0.0f`, `IFrameDuration == 0.28f`, `MoveSpeed == 550.0f`.
  - Edge cases: Level 1 baseline values must match GDD Core Attributes table exactly.

- **AC-2 Test: Boundary Clamping Enforcement**:
  - Given: Character has 50 Health remaining.
  - When: A GameplayEffect attempts to subtract 100 Health.
  - Then: `Health` is clamped at 0.0f (never negative).
  - When 2: A healing GameplayEffect adds 1,000 Health.
  - Then 2: `Health` is clamped at 500.0f (`MaxHealth`).

- **AC-3 Test: Dedicated Server Replication**:
  - Given: Character spawned on Dedicated Server and observed by Client proxy.
  - When: Server modifies `Health` from 500 to 350.
  - Then: Client proxy fires `OnRep_Health`, updates UI binding, and asserts `ClientHealth == 350.0f`.
  - Edge cases: Latency simulation at 150ms must not trigger client prediction misprediction rollback on valid server writes.

---

## Test Evidence

**Story Type**: Logic  
**Required evidence**: `tests/unit/combat/attributes_replication_test.cpp` — must exist and pass automated CI  
**Status**: [x] Passed (`tests/unit/combat/attributes_replication_test.cpp` — 3 test suites, 20 assertions)  

---

## Dependencies

- Depends on: None (Foundation story for Attributes Engine)
- Unlocks: Story 002 (Non-Linear Calculations) & Story 003 (Stamina Pipeline)

---

## Completion Notes
**Completed**: 2026-09-16  
**Criteria**: 3/3 passing (AC-1, AC-2, AC-3)  
**Deviations**: None  
**Test Evidence**: Logic unit test at `tests/unit/combat/attributes_replication_test.cpp` (20 assertions passing)  
**Code Review**: Approved  
