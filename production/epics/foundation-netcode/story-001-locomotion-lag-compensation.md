# Story 001: Dedicated Server Locomotion Prediction & Lag Compensation Rewind

> **Epic**: Open World MMO Netcode & Contested Aggro Sync  
> **Status**: Complete  
> **Layer**: Foundation  
> **Type**: Logic  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-09-16  

## Context

**GDD**: [`design/gdd/multiplayer-coop.md`](file:///mnt/Data/Projects/project-games/design/gdd/multiplayer-coop.md)  
**Requirement**: `TR-net-001`  
*(Requirement text lives in `docs/architecture/tr-registry.yaml` — Dedicated server architecture with client prediction for locomotion and server rollback validation for combat)*  

**ADR Governing Implementation**: [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md)  
**ADR Decision Summary**: Establishes 100% Dedicated Server authority for character positions and combat resolution, using client-side locomotion prediction with soft reconciliation on $>15\text{ cm}$ desync, and a 200ms circular history buffer for server-side hit validation rewind.

**Engine**: Unreal Engine 5.7 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Uses Network Prediction plugin standards in UE 5.7. Requires verification of rewind query overhead ($\le 0.5\text{ms}$) under headless `-nullrhi` server execution.

**Control Manifest Rules (Foundation Layer)**:
- Required: Dedicated Server owns 100% of game state; Lag Compensation Rewind Buffer must store 200ms of capsule history.
- Forbidden: Never trust client hit claims; Never use Peer-to-Peer or listen-server architectures.
- Guardrail: Rewind query latency $\le 0.5\text{ms}$ per check on server game thread.

---

## Acceptance Criteria

*From GDD `design/gdd/multiplayer-coop.md`, scoped to this story:*

- [x] **AC-1 (Lag Compensation Rewind)**: Server stores a circular buffer of character capsule transforms for the past 200ms (`server_max_lag_compensation = 0.20s`); when a client fires an attack with timestamp $T$, the server rewinds target positions to $T$ to validate collision sweeps.
- [x] **AC-12 (Server Soft Reconciliation)**: If client predicted location diverges from server-authoritative location by more than $15\text{ cm}$ (`reconciliation_threshold = 15.0f`), the server issues a position correction packet and the client smoothly interpolates without snapping.
- [x] **AC-9 (PvE Ally Collision Pass-Through)**: Character collision profiles between allied players in PvE are configured to ignore capsule pawn channels, allowing full movement pass-through (Zero Body Blocking).

---

## Implementation Notes

*Derived from ADR-0001 Implementation Guidelines:*

1. **Circular Position Buffer (`FPAPositionHistoryBuffer`)**:
   - Store an array of timestamped transforms: `struct FPAPastTransform { float Timestamp; FVector Location; FRotator Rotation; };`
   - Buffer capacity: 20 entries sampled at 100Hz (every 10ms), spanning 200ms total history.
   - Implement `GetInterpolatedTransform(float TargetTimestamp)` using linear interpolation between the two nearest frames.
2. **Hit Validation Service (`UPAHitValidationSubsystem`)**:
   - `bool ValidateMeleeSweep(const APABaseCharacter* Attacker, const APABaseCharacter* Target, float ClientTimestamp, const FCollisionShape& SweepShape);`
   - Rewind target to `ClientTimestamp` (clamped to `ServerTime - 0.20s`), execute 3D capsule test, and restore target transform immediately.
3. **Movement Reconciliation**:
   - Integrate with `UCharacterMovementComponent` client adjustment delegates to apply soft exponential decay smoothing when position error $> 15\text{ cm}$.

---

## Out of Scope

*Handled by neighbouring stories — do not implement here:*

- Story 002: Iris dynamic spatial filtering, network prioritization, and 15s ghost body disconnect handling.
- Story 003: Threat tables, aggro score calculations, and boss retargeting.

---

## QA Test Cases

*Written by qa-lead at story creation:*

- **AC-1 Test: Lag Compensation Rewind Under Latency**:
  - Given: Dedicated Server running at 60Hz. Target character moving horizontally at $600\text{ cm/s}$. Attacker client has simulated 150ms round-trip ping.
  - When: Attacker client fires melee sweep aimed at target's visual position and sends RPC with timestamp $T_{\text{client}}$.
  - Then: Server rewinds target position by 150ms, confirms sweep collision, and asserts `bHitConfirmed == true`.
  - Edge cases: Client timestamp older than 200ms is clamped to 200ms limit; future timestamps ($T_{\text{client}} > T_{\text{server}}$) are rejected as invalid.

- **AC-12 Test: Soft Position Reconciliation**:
  - Given: Client character artificially displaced by $25\text{ cm}$ on client game thread without server movement authorization.
  - When: Next server movement ack is processed by client.
  - Then: Client detects $>15\text{ cm}$ threshold, initiates smooth interpolation towards server position over $0.15\text{s}$, and asserts final delta is $<1\text{ cm}$.
  - Edge cases: Minor drift $\le 15\text{ cm}$ is ignored to prevent visual camera stutter.

- **AC-9 Test: PvE Ally Pass-Through**:
  - Given: Two friendly player characters facing each other in PvE zone.
  - When: Player A runs directly through Player B's world coordinates.
  - Then: Player A passes through Player B with zero velocity drop or capsule collision stopping.
  - Edge cases: Enemy mobs and hostile world bosses retain blocking collision.

---

## Test Evidence

**Story Type**: Logic  
**Required evidence**: `ProjectAscendant/Tests/unit/network/net_lag_compensation_test.cpp` — must exist and pass automated CI  
**Status**: [x] Created and Passing (6 test suites, 18 assertions PASS)  

---

## Dependencies

- Depends on: None (First story in Foundation Netcode)
- Unlocks: Story 002 (Iris Dynamic Spatial Prioritization)
