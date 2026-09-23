# Story 002: Iris Dynamic Spatial Prioritization & Ghost Body Disconnect Protection

> **Epic**: Open World MMO Netcode & Contested Aggro Sync  
> **Status**: Complete  
> **Layer**: Foundation  
> **Type**: Integration  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-09-16  

## Context

**GDD**: [`design/gdd/multiplayer-coop.md`](file:///mnt/Data/Projects/project-games/design/gdd/multiplayer-coop.md)  
**Requirement**: `TR-net-002`  
*(Requirement text lives in `docs/architecture/tr-registry.yaml` — Iris Replication system configuration with bandwidth throttling and dynamic spatial prioritization)*  

**ADR Governing Implementation**: [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md)  
**ADR Decision Summary**: Establishes Unreal Engine 5.7 Iris dynamic spatial replication filtering with 3 frequency tiers ($\le 15\text{m}$ 60Hz, $15-35\text{m}$ 30Hz, $>35\text{m}$ culling) to enforce bandwidth ceilings, and implements a 15.0s server-authoritative Ghost Body to prevent combat-logging exploits.

**Engine**: Unreal Engine 5.7 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Iris Replication requires configuring `IrisFilterConfig` and spatial data streams in `DefaultEngine.ini`.

**Control Manifest Rules (Foundation Layer)**:
- Required: Network replication must be prioritized dynamically across 3 spatial tiers; Disconnect in combat must maintain Ghost Body.
- Forbidden: Never broadcast raw transforms at 60Hz without spatial filter; Never allow instant logout while in combat.
- Guardrail: Max 50 KB/s ingress and 20 KB/s egress per client in dense combat.

---

## Acceptance Criteria

*From GDD `design/gdd/multiplayer-coop.md`, scoped to this story:*

- [x] **AC-10 (Iris 3-Tier Spatial Throttling)**: Actors within $1500\text{ cm}$ replicate at 60Hz (`iris_tier1_radius`); actors between $1500\text{ cm}$ and $3500\text{ cm}$ replicate at 30Hz (`iris_tier2_radius`); actors beyond $3500\text{ cm}$ are culled from high-frequency replication and represented as dormant proxies.
- [x] **AC-4 (Anti-Combat-Logging Ghost Body)**: When a client connection drops while possessing `GameplayTag.State.InCombat`, the character pawn remains instantiated on the dedicated server for exactly 15.0s (`combat_disconnect_ghost_duration = 15.0s`), remaining vulnerable to damage and death penalties.

---

## Implementation Notes

*Derived from ADR-0001 Implementation Guidelines:*

1. **Iris Spatial Filter Configuration (`UPASpatialDynamicFilter`)**:
   - Register custom Iris spatial filter inheriting from `UIrisSpatialFilterDefinition`.
   - Calculate distance $D = \|\text{ClientPawn.Location} - \text{TargetActor.Location}\|$.
   - Assign update dynamic rates: $D \le 1500 \implies 60\text{Hz}$; $1500 < D \le 3500 \implies 30\text{Hz}$; $D > 3500 \implies \text{Dormant (0Hz or map-only update)}$.
2. **Ghost Body Lifecycle Manager (`UPAGhostBodySubsystem`)**:
   - Bind to `UNetDriver::OnConnectionLost` or `AGameModeBase::Logout`.
   - If player character has `InCombat` tag, cancel immediate unpossess/destruction.
   - Detach controller, set AI dummy brain or freeze locomotion, and start a 15.0s timer `FTimerHandle GhostLifespanTimer`.
   - If killed before 15.0s expires, execute standard death drop logic. When timer expires, cleanly despawn character actor.

---

## Out of Scope

*Handled by neighbouring stories — do not implement here:*

- Story 001: Locomotion prediction and 200ms lag compensation rewind buffer.
- Story 003: Threat score calculation, boss aggro switching, and taunt multipliers.

---

## QA Test Cases

*Written by qa-lead at story creation:*

- **AC-10 Test: Iris 3-Tier Spatial Filtering**:
  - Given: Dedicated Server with test player at coordinate $(0, 0, 0)$. Three target dummy actors placed at $1000\text{ cm}$ (Tier 1), $2500\text{ cm}$ (Tier 2), and $4000\text{ cm}$ (Tier 3).
  - When: Running network profiling over a 1.0-second window.
  - Then: Verify Tier 1 actor receives $\sim 60$ property updates, Tier 2 receives $\sim 30$ updates, and Tier 3 receives 0 transform packets.
  - Edge cases: Moving from Tier 2 to Tier 1 seamlessly elevates tick rate to 60Hz without packet burst.

- **AC-4 Test: Ghost Body In-Combat Persistence**:
  - Given: Player character in combat with an enemy mob (has `GameplayTag.State.InCombat`).
  - When: Client socket forcibly closed on test runner.
  - Then: Character actor on Dedicated Server remains active with `bIsGhostBody == true` for exactly 15.0s; if mob deals lethal damage during this window, character dies and drops loot.
  - Edge cases: If client reconnects within 15.0s, controller seamlessly repossesses the existing Ghost Body.

---

## Test Evidence

**Story Type**: Integration  
**Required evidence**: `ProjectAscendant/Tests/unit/network/iris_ghost_body_test.cpp` — must exist and pass automated CI  
**Status**: [x] Created and Passing (7 test suites, 20 assertions PASS)  

---

## Dependencies

- Depends on: Story 001 (Locomotion Prediction & Lag Compensation)
- Unlocks: Story 003 (Contested Threat Table & Boss Aggro)
