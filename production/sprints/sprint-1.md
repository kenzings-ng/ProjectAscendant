# Sprint 1 — 2026-09-16 to 2026-09-30

## Sprint Goal
Establish the Foundation Layer core gameplay loop for *Project Ascendant* — screen-relative 8-way locomotion, de-coupled isometric aiming, camera occlusion, GAS AttributeSet definition (Health, Mana, Stamina, Posture), non-linear damage calculations, and dedicated server movement prediction.

## Review Mode
- Mode: `lean` (Saved in [`production/review-mode.txt`](file:///mnt/Data/Projects/project-games/production/review-mode.txt))
- Director Gates: PR-SPRINT skipped — Lean mode.

## Capacity
- Total days: 10 days (80 hours)
- Buffer (20%): 2 days (16 hours reserved for engine setup, debugging, and unexpected edge cases)
- Available: 8 days (64 hours)

## Tasks

### Must Have (Critical Path — 28 hours / 3.5 days)
| ID | Task | Agent/Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `ctrl-001` | [Screen-Relative 8-Way Locomotion & Diagonal Normalization](file:///mnt/Data/Projects/project-games/production/epics/foundation-controller/story-001-screen-relative-locomotion.md) | `gameplay-programmer` | 0.5 (4h) | None | AC-1: Screen-orthogonal movement via $45^\circ$ yaw rotation matrix; AC-2: Diagonal vector length normalized to 1.0; AC-3: Opposing input cancellation transitions to Idle cleanly. |
| `ctrl-002` | [Screen-to-World De-projection & Decoupled Twin-Stick Aiming](file:///mnt/Data/Projects/project-games/production/epics/foundation-controller/story-002-deprojection-decoupled-aiming.md) | `gameplay-programmer` | 0.5 (4h) | `ctrl-001` | AC-1: Plane raycast de-projection onto $Z=0$ ground plane; AC-2: Decoupled facing rotation independent of movement vector; AC-3: Twin-stick gamepad deadzone (0.15) and instant angle capture. |
| `ctrl-003` | [Dynamic Look-Ahead SpringArm Camera & Line-of-Sight Occlusion Dither](file:///mnt/Data/Projects/project-games/production/epics/foundation-controller/story-003-camera-lookahead-occlusion.md) | `gameplay-programmer` | 0.5 (4h) | `ctrl-001` | AC-1: Fixed isometric perspective (Pitch $-45^\circ$, Yaw $+45^\circ$, target arm 1400 units); AC-2: Velocity look-ahead dynamic arm offset (max 300 units, $\tau=0.25$s); AC-3: Capsule raycast wall occlusion with temporal dither fading. |
| `attr-001` | [Core AttributeSet Definition & Iris Network Replication](file:///mnt/Data/Projects/project-games/production/epics/foundation-attributes/story-001-attributeset-replication.md) | `ue-gas-specialist` | 0.5 (4h) | None | AC-1: Four primary attributes (Health, Mana, Stamina, Posture) with `GAMEPLAYATTRIBUTE_REPNOTIFY`; AC-2: Iris replication registration with COND_OwnerOnly where appropriate; AC-3: Server-authoritative clamp logic in `PreAttributeChange`. |
| `attr-002` | [Non-Linear Damage & Posture Execution Calculations](file:///mnt/Data/Projects/project-games/production/epics/foundation-attributes/story-002-damage-posture-calculations.md) | `ue-gas-specialist` | 0.5 (4h) | `attr-001` | AC-1: Non-linear armor diminishing returns formula $\text{Mitigation} = \frac{\text{Armor}}{\text{Armor} + 400}$; AC-2: Posture break threshold triggers `State.Broken` tag and 2.5s stun; AC-3: Execution calculation executes deterministically on server. |
| `attr-003` | [Stamina Regeneration, Depletion & Exhaustion State Pipeline](file:///mnt/Data/Projects/project-games/production/epics/foundation-attributes/story-003-stamina-exhaustion-pipeline.md) | `ue-gas-specialist` | 0.5 (4h) | `attr-001` | AC-1: 1.0s delay after stamina consumption before regen starts; AC-2: Depletion to 0 applies `State.Exhausted` blocking sprinting/dodging; AC-3: Recovery to 20% clears exhaustion. |
| `net-001` | [Dedicated Server Locomotion Prediction & Lag Compensation Rewind](file:///mnt/Data/Projects/project-games/production/epics/foundation-netcode/story-001-locomotion-lag-compensation.md) | `ue-replication-specialist` | 0.5 (4h) | `ctrl-001` | AC-1: Headless dedicated server runs 60Hz tick without rendering; AC-2: Client-side movement prediction with server reconciliation under 100ms latency; AC-3: 200ms circular history buffer for combat rewind validation. |

### Should Have (16 hours / 2.0 days)
| ID | Task | Agent/Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `net-002` | [Iris Dynamic Spatial Prioritization & Ghost Body Disconnect Protection](file:///mnt/Data/Projects/project-games/production/epics/foundation-netcode/story-002-iris-spatial-ghost-body.md) | `ue-replication-specialist` | 0.5 (4h) | `net-001` | AC-1: 3-tier Iris dynamic spatial frequency filtering ($\le 15\text{m}$ at 60Hz, $>35\text{m}$ at 5Hz); AC-2: Ghost body persists 30s upon player disconnect in combat; AC-3: Reconnection reinstates player controller and attributes seamlessly. |
| `net-003` | [Contested Threat Table & Boss Aggro Synchronization](file:///mnt/Data/Projects/project-games/production/epics/foundation-netcode/story-003-contested-threat-table.md) | `lead-programmer` | 0.5 (4h) | `attr-001`, `net-001` | AC-1: Server-authoritative threat table tracking up to 32 players; AC-2: 10% threat delta threshold required to switch primary target; AC-3: Iris replication of top target at 10Hz to all players in aggro radius. |
| `inv-001` | [FastArray Grid Inventory & Item DataAsset Hierarchy](file:///mnt/Data/Projects/project-games/production/epics/foundation-inventory/story-001-fastarray-inventory-data.md) | `gameplay-programmer` | 0.5 (4h) | None | AC-1: 30-slot grid inventory implemented with `FFastArraySerializer`; AC-2: Separation of immutable `UItemStaticDataAsset` and mutable `FItemInstanceData`; AC-3: 5-tier item rarity structure serialized correctly. |
| `inv-002` | [Server-Authoritative Transaction RPCs & Duplication Safeguards](file:///mnt/Data/Projects/project-games/production/epics/foundation-inventory/story-002-transaction-rpcs-safeguards.md) | `gameplay-programmer` | 0.5 (4h) | `inv-001` | AC-1: Server RPCs for Move, Split, Equip, Drop with unique transaction ID; AC-2: Atomic validation rejects invalid slot indices and negative stacks; AC-3: Duplicate RPC requests ignored under packet loss simulation. |

### Nice to Have (8 hours / 1.0 day)
| ID | Task | Agent/Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `net-004` | [Shared Posture Finisher Priority & Instanced Loot Allocation](file:///mnt/Data/Projects/project-games/production/epics/foundation-netcode/story-004-shared-finisher-instanced-loot.md) | `lead-programmer` | 0.5 (4h) | `net-003` | AC-1: First-arrival server RPC claim token for boss posture finisher; AC-2: Invulnerability during finisher execution; AC-3: Per-player instanced loot generation and replication upon boss defeat. |
| `inv-003` | [Paperdoll Equipment Binding & GAS Attribute Integration](file:///mnt/Data/Projects/project-games/production/epics/foundation-inventory/story-003-paperdoll-gas-binding.md) | `ue-gas-specialist` | 0.5 (4h) | `inv-001`, `attr-001` | AC-1: 8 equipment slots (Head, Chest, Hands, Legs, Feet, MainHand, OffHand, Ring); AC-2: Equipping items applies passive `UGameplayEffect` attribute modifiers; AC-3: Unequipping cleanly removes effect spec handles. |

## Carryover from Previous Sprint
*None — Initial sprint for Pre-Production.*

## Total Planned Scope
- **Must Have**: 7 stories (28 hours / 3.5 days)
- **Should Have**: 4 stories (16 hours / 2.0 days)
- **Nice to Have**: 2 stories (8 hours / 1.0 day)
- **Total**: 13 stories (52 hours / 6.5 days) vs **Available Capacity**: 8.0 days (64 hours)

## Risks
| Risk | Probability | Impact | Mitigation |
|---|---|---|---|
| Dedicated Server Headless Performance under high replication load | Medium | High | Verify `-nullrhi` 60Hz tick stability early in `net-001`; run Iris spatial filtering profiles. |
| PaperZD AnimNotify headless execution without GPU rendering | Medium | Medium | Test PaperZD notify firing on dedicated server in `attr-002` test suite; ensure server authority does not depend on client rendering. |
| Client movement prediction desync with 45-degree isometric projection | Low | Medium | Strict clamping and diagonal normalization in `ctrl-001`; automated test `screen_relative_movement_test.cpp`. |

## Dependencies on External Factors
- Unreal Engine 5.7 source/installed build with Iris replication enabled.
- PaperZD plugin enabled in `.uproject`.

## Definition of Done for this Sprint
- [ ] All Must Have tasks completed
- [ ] All tasks pass acceptance criteria
- [x] QA plan exists ([`production/qa/qa-plan-sprint-1-2026-09-16.md`](file:///mnt/Data/Projects/project-games/production/qa/qa-plan-sprint-1-2026-09-16.md))
- [ ] All Logic/Integration stories have passing unit/integration tests
- [ ] Smoke check passed (`/smoke-check sprint`)
- [ ] QA sign-off report: APPROVED or APPROVED WITH CONDITIONS (`/team-qa sprint`)
- [ ] No S1 or S2 bugs in delivered features
- [ ] Design documents updated for any deviations
- [ ] Code reviewed and merged

---

> **Scope check:** If this sprint includes stories added beyond the original epic scope, run `/scope-check [epic]` to detect scope creep before implementation begins.
