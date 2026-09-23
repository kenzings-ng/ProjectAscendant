# Architecture Traceability Index: Project Ascendant

<!-- Living document — updated by /architecture-review after each review run.
     Do not edit manually unless correcting an error. -->

## Document Status

- **Last Updated**: 2026-09-16
- **Target Phase**: Pre-Production
- **Engine**: Unreal Engine 5.7 (C++, Gameplay Ability System, Iris, PaperZD)
- **GDDs Indexed**: 14
- **ADRs Indexed**: 3 ([ADR-0001](adr-0001-open-world-mmo-combat-networking.md), [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md), [ADR-0003](adr-0003-server-authoritative-grid-inventory-fast-array.md))
- **Master Blueprint**: [`docs/architecture/architecture.md`](architecture.md) (All 9 Chapters Approved)
- **Last Review**: [`docs/architecture/architecture-review-2026-09-16.md`](architecture-review-2026-09-16.md)

---

## Coverage Summary

| Layer | Total Req | ✅ Covered | ⚠️ Partial | ❌ Gap | Coverage Rate | Gate Status |
|---|---|---|---|---|---|---|
| **Foundation Layer** | 12 | 12 | 0 | 0 | 100% | **PASS (Zero Gaps)** |
| **Core Layer** | 12 | 12 | 0 | 0 | 100% | **PASS (Zero Gaps)** |
| **Feature Layer** | 15 | 15 | 0 | 0 | 100% | **PASS** |
| **Presentation Layer** | 3 | 3 | 0 | 0 | 100% | **PASS** |
| **Total** | **42** | **42** | **0** | **0** | **100%** | **READY FOR PRE-PRODUCTION** |

---

## Traceability Matrix

| Req ID | GDD | System | Requirement Summary | Governing ADR / Blueprint | Status | Layer |
|---|---|---|---|---|---|---|
| **TR-attr-001** | `attributes-system.md` | Attributes | GAS AttributeSet with Health/Mana/Stamina Iris replication | ADR-0002, Arch Ch. 2 | ✅ Covered | Foundation |
| **TR-attr-002** | `attributes-system.md` | Attributes | Execution calculations for non-linear defense scaling | ADR-0002, Arch Ch. 2 | ✅ Covered | Foundation |
| **TR-attr-003** | `attributes-system.md` | Attributes | Client prediction with server authoritative reconciliation | ADR-0001, ADR-0002 | ✅ Covered | Foundation |
| **TR-ctrl-001** | `isometric-controller.md` | Controller | Screen-to-world isometric de-projection plane | Arch Ch. 3 | ✅ Covered | Foundation |
| **TR-ctrl-002** | `isometric-controller.md` | Controller | Enhanced Input 8-way directional movement | Arch Ch. 3 | ✅ Covered | Foundation |
| **TR-ctrl-003** | `isometric-controller.md` | Controller | Occluding 3D geometry dither-fade shader | Arch Ch. 3 | ✅ Covered | Presentation |
| **TR-dash-001** | `dash-evasion.md` | Dash | Server-validated 0.25s I-frame invulnerability tag | ADR-0001, ADR-0002 | ✅ Covered | Core |
| **TR-dash-002** | `dash-evasion.md` | Dash | Stamina deduction via GameplayEffect with prediction | ADR-0002 | ✅ Covered | Core |
| **TR-dash-003** | `dash-evasion.md` | Dash | Sprite rotation lock along dash trajectory | ADR-0002 | ✅ Covered | Core |
| **TR-combat-001** | `combat-system.md` | Combat | Server-authoritative 3D capsule sweep via PaperZD notify | ADR-0001, ADR-0002 | ✅ Covered | Core |
| **TR-combat-002** | `combat-system.md` | Combat | Multi-hit combo state machine with 250ms buffer window | ADR-0002 | ✅ Covered | Core |
| **TR-combat-003** | `combat-system.md` | Combat | Server-side damage mitigation formula calculation | ADR-0001, ADR-0002 | ✅ Covered | Core |
| **TR-stagger-001** | `stagger-system.md` | Stagger | Posture break attribute with 4.0s decay rate | ADR-0002, Arch Ch. 4 | ✅ Covered | Core |
| **TR-stagger-002** | `stagger-system.md` | Stagger | Vulnerable state granting 200% critical damage multiplier | ADR-0002, Arch Ch. 4 | ✅ Covered | Core |
| **TR-stagger-003** | `stagger-system.md` | Stagger | Part-breaking threshold disabling specific boss ability tags | ADR-0002, Arch Ch. 4 | ✅ Covered | Core |
| **TR-boss-001** | `boss-ai.md` | Boss AI | StateTree decision making with 75/50/25% HP transitions | Arch Ch. 6 | ✅ Covered | Feature |
| **TR-boss-002** | `boss-ai.md` | Boss AI | Ground telegraph decal and Niagara sync 0.8s prior to hit | Arch Ch. 6 | ✅ Covered | Feature |
| **TR-boss-003** | `boss-ai.md` | Boss AI | Threat calculation evaluating damage, heals, proximity | ADR-0001, Arch Ch. 6 | ✅ Covered | Feature |
| **TR-hud-001** | `combat-hud.md` | Combat HUD | CommonUI widget hierarchy for combat interface | `interaction-patterns.md`, Arch Ch. 8 | ✅ Covered | Presentation |
| **TR-hud-002** | `combat-hud.md` | Combat HUD | Ghosting damage bar with 0.4s interpolation | `interaction-patterns.md`, Arch Ch. 8 | ✅ Covered | Presentation |
| **TR-hud-003** | `combat-hud.md` | Combat HUD | Object-pooled floating combat text (64 max instances) | `interaction-patterns.md`, Arch Ch. 8 | ✅ Covered | Presentation |
| **TR-skill-001** | `skill-progression-system.md` | Skills | Server-validated Skill Book ability grant to ASC | ADR-0002, Arch Ch. 2 | ✅ Covered | Feature |
| **TR-skill-002** | `skill-progression-system.md` | Skills | Skill tree prerequisite graph validation | ADR-0002, Arch Ch. 2 | ✅ Covered | Feature |
| **TR-skill-003** | `skill-progression-system.md` | Skills | Hotbar ability slot configuration serialization | ADR-0002, Arch Ch. 5 | ✅ Covered | Feature |
| **TR-inv-001** | `inventory-system.md` | Inventory | 30-slot grid inventory replicating via FFastArraySerializer | ADR-0003, Arch Ch. 5 | ✅ Covered | Foundation |
| **TR-inv-002** | `inventory-system.md` | Inventory | Item instance data structure with durability and affixes | ADR-0003, Arch Ch. 5 | ✅ Covered | Foundation |
| **TR-inv-003** | `inventory-system.md` | Inventory | Atomic transaction validation preventing duplication | ADR-0003, Arch Ch. 5 | ✅ Covered | Foundation |
| **TR-class-001** | `foundational-classes.md` | Classes | 4 Archetype class traits and starting GAS abilities | ADR-0002, Arch Ch. 2 | ✅ Covered | Feature |
| **TR-class-002** | `foundational-classes.md` | Classes | Weapon proficiency scaling matrices to attributes | ADR-0002, Arch Ch. 2 | ✅ Covered | Feature |
| **TR-class-003** | `foundational-classes.md` | Classes | Class PaperZD animation blueprint templates | ADR-0002, Arch Ch. 4 | ✅ Covered | Feature |
| **TR-blacksmith-001** | `blacksmithing-system.md` | Blacksmithing | Server-authoritative upgrade probability roll (+1 to +10) | ADR-0003, Arch Ch. 5 | ✅ Covered | Feature |
| **TR-blacksmith-002** | `blacksmithing-system.md` | Blacksmithing | Dynamic affix injection into item instance structure | ADR-0003, Arch Ch. 5 | ✅ Covered | Feature |
| **TR-blacksmith-003** | `blacksmithing-system.md` | Blacksmithing | Zone forge proximity validation | ADR-0003, Arch Ch. 7 | ✅ Covered | Feature |
| **TR-zone-001** | `zone-system.md` | Zones | Seamless level streaming via World Partition & data layers | ADR-0001, Arch Ch. 7 | ✅ Covered | Core |
| **TR-zone-002** | `zone-system.md` | Zones | Checkpoint and shrine respawn state persistence | ADR-0001, Arch Ch. 7 | ✅ Covered | Core |
| **TR-zone-003** | `zone-system.md` | Zones | Zone border dynamic Iris replication priorities | ADR-0001, Arch Ch. 1 | ✅ Covered | Core |
| **TR-econ-001** | `merchant-economy.md` | Economy | Server-authoritative currency atomic transactions | ADR-0003, Arch Ch. 5 | ✅ Covered | Feature |
| **TR-econ-002** | `merchant-economy.md` | Economy | Vendor limited stock timed restock intervals | ADR-0003, Arch Ch. 5 | ✅ Covered | Feature |
| **TR-econ-003** | `merchant-economy.md` | Economy | Gold sinks balancing economic inflation | ADR-0003, Arch Ch. 5 | ✅ Covered | Feature |
| **TR-net-001** | `multiplayer-coop.md` | Networking | Dedicated server authority with client prediction & rollback | ADR-0001, Arch Ch. 1 | ✅ Covered | Foundation |
| **TR-net-002** | `multiplayer-coop.md` | Networking | Iris replication dynamic spatial prioritization | ADR-0001, Arch Ch. 1 | ✅ Covered | Foundation |
| **TR-net-003** | `multiplayer-coop.md` | Networking | Contested open-world mob and boss aggro sync (32 players) | ADR-0001, Arch Ch. 1 | ✅ Covered | Foundation |

---

## Known Gaps Analysis

### Foundation Layer Gaps (Gate Blocker)
- **Status**: **ZERO GAPS** (12 of 12 Foundation requirements completely satisfied by ADR-0001, ADR-0002, and ADR-0003).

### Core Layer Gaps
- **Status**: **ZERO GAPS** (12 of 12 Core requirements completely satisfied).

### Feature Layer Gaps
- **Status**: **ZERO GAPS** (All 15 Feature requirements addressed by Master Architecture Chapters and governing ADRs).

### Presentation Layer Gaps
- **Status**: **ZERO GAPS** (Covered by `design/ux/interaction-patterns.md` and Master Architecture Ch. 8).

---

## Cross-ADR Conflict Audit

| ADR Pair | Comparison Area | Potential Conflict | Audit Resolution | Status |
|---|---|---|---|---|
| **ADR-0001 vs ADR-0002** | Combat Hit Authority | Client hit detection vs Server sweep | Both mandate Server-Authoritative 3D capsule sweep with client-side optimistic animation prediction. No conflict. | 🟢 Resolved |
| **ADR-0001 vs ADR-0003** | Replication Channel | Iris replication bandwidth vs FastArray serialization | FastArray serializer operates cleanly over Iris network channels with delta-compression. No conflict. | 🟢 Resolved |
| **ADR-0002 vs ADR-0003** | Item Ability Grants | Item stats vs AttributeSet modification | Inventory passes DataAsset modifiers to GAS GameplayEffects. Clear seam. No conflict. | 🟢 Resolved |

---

## ADR → GDD Coverage (Reverse Index)

| ADR | Title | GDD Requirements Addressed | Engine Risk |
|---|---|---|---|
| **ADR-0001** | Open World MMO Combat Networking | TR-net-001, TR-net-002, TR-net-003, TR-combat-001, TR-combat-003, TR-dash-001, TR-boss-003, TR-zone-001, TR-zone-003 | 🟡 MEDIUM |
| **ADR-0002** | GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites | TR-attr-001, TR-attr-002, TR-attr-003, TR-ctrl-001, TR-ctrl-002, TR-dash-001, TR-dash-002, TR-dash-003, TR-combat-001, TR-combat-002, TR-combat-003, TR-stagger-001, TR-stagger-002, TR-stagger-003, TR-skill-001, TR-skill-002, TR-skill-003, TR-class-001, TR-class-002, TR-class-003 | 🟡 MEDIUM |
| **ADR-0003** | Server-Authoritative Grid Inventory via FFastArraySerializer | TR-inv-001, TR-inv-002, TR-inv-003, TR-blacksmith-001, TR-blacksmith-002, TR-blacksmith-003, TR-econ-001, TR-econ-002, TR-econ-003 | 🟢 LOW |

---

## Recommended Implementation Order (Topologically Sorted)

1. **Foundation Stage (Immediate Sprint 1 Target)**:
   - ADR-0001 (Dedicated Server Core, Iris Replication, Net Prediction)
   - ADR-0002 (GAS AttributeSets, PaperZD Bridge, 3D Sweep Hitbox Component)
   - ADR-0003 (`FFastArraySerializer` Inventory Data Structure)
2. **Core Gameplay Stage**:
   - Locomotion, 8-way isometric movement, and Dash I-frame validation
   - 3-hit Light/Heavy Combo with Posture damage calculation
   - World Partition zone streaming and shrine checkpoints
3. **Feature & Content Stage**:
   - Boss AI StateTree implementation and Niagara telegraphs
   - Blacksmith upgrade loop and Merchant trading UI
   - 4 Foundational class skill trees
