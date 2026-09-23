# Story 003: Contested Threat Table & Boss Aggro Synchronization

> **Epic**: Open World MMO Netcode & Contested Aggro Sync  
> **Status**: Complete  
> **Layer**: Foundation  
> **Type**: Logic  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-09-16  

## Context

**GDD**: [`design/gdd/multiplayer-coop.md`](file:///mnt/Data/Projects/project-games/design/gdd/multiplayer-coop.md)  
**Requirement**: `TR-net-003`  
*(Requirement text lives in `docs/architecture/tr-registry.yaml` — Contested open-world mob and boss aggro synchronization across up to 32 players in single zone)*  

**ADR Governing Implementation**: [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md)  
**ADR Decision Summary**: Defines a server-authoritative Threat Table system tracking cumulative damage, posture damage, and healing threat scores across up to 32 concurrent players, enforcing a 130% threat spike retargeting rule, 10%/s threat decay, and an absolute 2500 cm leash boundary.

**Engine**: Unreal Engine 5.7 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Interacts directly with StateTree AI evaluations and Iris prioritized replication of active aggro target IDs.

**Control Manifest Rules (Foundation Layer)**:
- Required: Dedicated Server owns 100% of Threat evaluations; Open-world contested aggro must sync across up to 32 players.
- Forbidden: Never permit client-side threat modification; Never allow leash-broken mobs to take damage during reset.
- Guardrail: Threat table update execution $\le 0.3\text{ms}$ per evaluation tick on server game thread.

---

## Acceptance Criteria

*From GDD `design/gdd/multiplayer-coop.md`, scoped to this story:*

- [x] **AC-2 (130% Threat Retargeting Rule)**: Boss re-evaluates Threat Table every 1.0s; if any player's threat score exceeds the current target's threat by $\ge 130\%$ (`threat_retarget_ratio = 1.30`), the boss immediately switches target and rotates towards the new threat leader.
- [x] **AC-3 (Threat Decay)**: If a combatant deals no damage, applies no crowd control, and heals no targets for 3.0 seconds (`threat_decay_delay = 3.0s`), their accumulated threat score decays by 10% per second (`threat_decay_rate = 0.10/s`).
- [x] **AC-11 (Taunt Multiplier & Snap)**: Activating a Tank Taunt ability multiplies current threat gained by $5.0\times$ (`taunt_multiplier = 5.0`) and immediately sets the caster's threat score to $\text{Max}(\text{CurrentTopThreat} + 100, \text{CasterThreat})$, forcing an instant target snap.
- [x] **AC-5 (Leash Boundary Reset)**: If a mob or boss is pulled beyond $2500\text{ cm}$ from its spawn origin (`ai_leash_max_distance = 2500.0f`), it enters `LeashReset` state, becomes invulnerable to all damage, drops all threat, and returns to spawn at accelerated speed.

---

## Implementation Notes

*Derived from ADR-0001 Implementation Guidelines:*

1. **Threat Table Component (`UPAThreatComponent`)**:
   - Maintains a map: `TMap<TWeakObjectPtr<APABaseCharacter>, float> ThreatScores;`
   - Weightings: $1\text{ HP Damage} = 1.0\text{ Threat}$; $1\text{ Posture Damage} = 2.5\text{ Threat}$; $1\text{ Healing} = 0.5\text{ Threat}$.
   - Method `AddThreat(APABaseCharacter* Instigator, float RawThreat, EThreatSource Source);`
2. **Retargeting & Decoupled AI Updates**:
   - Maintain `TWeakObjectPtr<APABaseCharacter> CurrentAggroTarget;`
   - Replicated via Iris to all players in Tier 1 so UI displays the current target crown.
   - On threat update, if `HighestThreat >= CurrentTargetThreat * 1.30f`, dispatch `OnAggroTargetChanged` delegate to StateTree.
3. **Leash Checker**:
   - Check distance in `NativeTick` or timer: `if (FVector::Dist2D(GetActorLocation(), SpawnOrigin) > 2500.0f) TriggerLeashReset();`

---

## Out of Scope

*Handled by neighbouring stories — do not implement here:*

- Story 001: Locomotion prediction and 200ms lag compensation buffer.
- Story 004: Shared posture finisher window (1.5s) and instanced loot allocation.

---

## QA Test Cases

*Written by qa-lead at story creation:*

- **AC-2 Test: 130% Threat Retargeting**:
  - Given: Boss target is Player A with 1,000 Threat points. Player B has 1,200 Threat points.
  - When: Boss executes 1.0s threat evaluation scan.
  - Then: Boss maintains Player A as target (1,200 is only 120%, $< 130\%$).
  - When 2: Player B deals damage bringing threat to 1,310 points ($131\%$).
  - Then 2: Boss instantly changes `CurrentAggroTarget` to Player B.

- **AC-3 Test: Threat Decay Over Inactivity**:
  - Given: Player has 1,000 Threat points and stops all combat actions at time $T=0$.
  - When: Time advances to $T=3.0\text{s}$.
  - Then: Threat remains 1,000 points.
  - When 2: Time advances to $T=4.0\text{s}$ (1s of decay).
  - Then 2: Threat is asserted at 900 points ($10\%$ deduction).

- **AC-11 Test: Tank Taunt Mechanic**:
  - Given: Boss target is Mage with 5,000 Threat. Vanguard has 500 Threat.
  - When: Vanguard activates "Taunt" ability.
  - Then: Vanguard threat snaps to 5,100 points, and boss rotates to target Vanguard within 1 tick.

- **AC-5 Test: Leash Reset Invulnerability**:
  - Given: Boss pulled to $2550\text{ cm}$ from spawn coordinates.
  - When: Player attacks boss while in reset transit.
  - Then: Damage numbers assert 0 (Immune), health remains unmodified, and boss returns to spawn.

---

## Test Evidence

**Story Type**: Logic  
**Required evidence**: `ProjectAscendant/Tests/unit/combat/threat_table_test.cpp` — exists and covers AC-2, AC-3, AC-11, AC-5.  
**Status**: [x] Complete  

---

## Dependencies

- Depends on: Story 001 (Locomotion & Netcode baseline)
- Unlocks: Story 004 (Shared Posture Finisher & Instanced Loot)
