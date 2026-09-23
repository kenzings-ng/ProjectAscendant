# Story 004: Shared Posture Finisher Priority & Instanced Loot Allocation

> **Epic**: Open World MMO Netcode & Contested Aggro Sync  
> **Status**: Complete  
> **Layer**: Foundation  
> **Type**: Integration  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-09-16  

## Context

**GDD**: [`design/gdd/multiplayer-coop.md`](file:///mnt/Data/Projects/project-games/design/gdd/multiplayer-coop.md)  
**Requirement**: `TR-net-003`  
*(Requirement text lives in `docs/architecture/tr-registry.yaml` — Contested open-world mob and boss aggro synchronization across up to 32 players in single zone)*  

**ADR Governing Implementation**: [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md)  
**ADR Decision Summary**: Establishes cooperative combat reward fairness by providing an exclusive 1.5s execution window to the player who landed the final posture-breaking hit, allocating individual server-side Instanced Loot droplets only to players contributing $\ge 5\%$ HP or $\ge 10\%$ Posture damage, and calculating radius-shared Party EXP with team morale bonuses.

**Engine**: Unreal Engine 5.7 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Integrates GAS GameplayTags (`State.Staggered`, `State.FinisherPriority`), Iris relevance filtering for loot drop actors, and party subsystem delegates.

**Control Manifest Rules (Foundation Layer)**:
- Required: Dedicated Server owns 100% of damage calculations and item drop allocation; Loot instances must be strictly individual.
- Forbidden: Never permit ninja-looting or shared free-for-all drop pools for boss encounters; Never allow non-finishers to execute within 1.5s.
- Guardrail: Instanced loot generation $\le 1.0\text{ms}$ upon boss death on server game thread.

---

## Acceptance Criteria

*From GDD `design/gdd/multiplayer-coop.md`, scoped to this story:*

- [x] **AC-7 (1.5s Finisher Exclusive Window)**: When a boss's Posture reaches 0, the player who delivered the final posture break is tagged as `Finisher`; only the Finisher can trigger the Execution interact prompt (`[E]`) during the first 1.5 seconds (`boss_finisher_exclusive_window = 1.5s`). After 1.5s, the window opens to any nearby player.
- [x] **AC-6 (Instanced Loot Contribution Threshold)**: When a boss dies, the server calculates contribution ratios: only players who dealt $\ge 5\%$ of total HP damage (`boss_loot_contribution_hp = 0.05`) OR $\ge 10\%$ of total Posture damage receive an Instanced Loot droplet. Droplets are completely invisible and unpickable by other players.
- [x] **AC-8 (Party EXP Sharing with Morale Bonus)**: When a monster dies, all party members within $3000\text{ cm}$ (`party_exp_radius = 3000.0f`) receive shared experience according to `party_exp_share` formula with morale modifiers (+10% for 2 players, +20% for 3 players, +35% for 4 players).

---

## Implementation Notes

*Derived from ADR-0001 Implementation Guidelines:*

1. **Shared Posture Execution Controller (`UPAPostureSyncComponent`)**:
   - On Posture reaching 0, record `TWeakObjectPtr<APABaseCharacter> PostureFinisherPlayer`.
   - Apply `GameplayTag.State.FinisherPriority` to `PostureFinisherPlayer` and set a 1.5s timer.
   - Replicate `FinisherPlayer` ID to all clients so UI can display golden execution ring to the Finisher only.
   - When interact RPC `ServerRequestExecuteStagger` is received: if `GetWorld()->GetTimeSeconds() < BreakTime + 1.5s` and `Sender != PostureFinisherPlayer`, reject execution.
2. **Instanced Loot Allocator (`UPALootDistributionSubsystem`)**:
   - Track cumulative HP damage and Posture damage per player UID throughout encounter in `TMap<FGuid, FContributionRecord>`.
   - On boss death, iterate records: if `(DamageDealt / MaxHP) >= 0.05f || (PostureDealt / MaxPosture) >= 0.10f`, spawn `APALootDropletActor` set to Iris client-connection-only visibility for that specific PlayerController.
3. **Party EXP Sharing**:
   - Calculate total EXP: $\text{BaseEXP} \times (1.0 + \text{MoraleBonus})$.
   - Divide equally among all connected party members where distance to death location $\le 3000\text{ cm}$.

---

## Out of Scope

*Handled by neighbouring stories — do not implement here:*

- Story 001: Movement prediction and 200ms lag compensation buffer.
- Story 002: Iris dynamic spatial filtering and ghost body persistence.
- Story 003: Threat score math and boss retargeting.

---

## QA Test Cases

*Written by qa-lead at story creation:*

- **AC-7 Test: 1.5s Finisher Window Exclusivity**:
  - Given: Boss Posture depleted to 0 by Player 1. Player 2 stands right next to Boss.
  - When: Player 2 sends `ServerRequestExecuteStagger` at time $T=0.8\text{s}$ (within 1.5s).
  - Then: Server rejects request, Boss remains in Stagger, and Player 1 can still execute.
  - When 2: Time reaches $T=1.6\text{s}$ and Player 2 sends request.
  - Then 2: Server accepts execution, dealing 25% Max HP damage.

- **AC-6 Test: Instanced Loot Qualification**:
  - Given: Boss with 10,000 HP. Player A deals 600 damage (6%), Player B deals 450 damage (4.5%), Player C deals 0 HP damage but 12% Posture damage.
  - When: Boss is defeated.
  - Then: Server spawns an Instanced Loot Droplet for Player A and Player C; Player B receives zero loot droplets.
  - And: Player A cannot see or interact with Player C's loot droplet.

- **AC-8 Test: Party EXP Sharing & Radius Culling**:
  - Given: 4-player party. Players 1, 2, 3 stand within $1000\text{ cm}$ of mob. Player 4 stands at $3500\text{ cm}$ ($> 3000\text{ cm}$). Mob gives 1,000 Base EXP.
  - When: Mob dies.
  - Then: Players 1, 2, 3 receive shared EXP with +35% party morale bonus; Player 4 receives 0 EXP.

---

## Test Evidence

**Story Type**: Integration  
**Required evidence**: `tests/integration/combat/contested_loot_finisher_test.cpp` OR automated multi-client combat test  
**Status**: [x] Passed (`ProjectAscendant/Tests/integration/combat/contested_loot_finisher_test.cpp`, 4 test blocks passing, build succeeded)  

---

## Dependencies

- Depends on: Story 003 (Threat Table & Boss Aggro)
- Unlocks: Epic Completion (Foundation Netcode fully ready for implementation)
