# Epic: Open World MMO Netcode & Contested Aggro Sync

> **Layer**: Foundation  
> **GDD**: [`design/gdd/multiplayer-coop.md`](file:///mnt/Data/Projects/project-games/design/gdd/multiplayer-coop.md)  
> **Architecture Module**: Dedicated Server & MMO Combat Networking ([`docs/architecture/architecture.md`](file:///mnt/Data/Projects/project-games/docs/architecture/architecture.md) Chapter 1)  
> **Status**: Ready  
> **Stories**: 4 Stories Created  

## Overview

This epic implements the core multiplayer networking foundation for *Project Ascendant* on Unreal Engine 5.7. It establishes the Dedicated Server headless runtime architecture, Iris Replication with 3-tier dynamic spatial filtering, client-side movement prediction with server replay reconciliation, a 200ms circular history buffer for combat lag compensation rewind, and real-time synchronization of open-world monster and boss threat tables across up to 32 players in a single zone.

## Governing ADRs

| ADR | Decision Summary | Engine Risk |
|---|---|---|
| [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md) | Standardizes Dedicated Server authority, Iris 3-tier spatial filtering, client movement prediction, and lag compensation rewind buffer. | 🟡 MEDIUM |

## GDD Requirements

| TR-ID | Requirement | ADR Coverage |
|---|---|---|
| **TR-net-001** | Dedicated server architecture with client prediction for locomotion and server rollback validation for combat | ADR-0001, Arch Ch. 1 ✅ |
| **TR-net-002** | Iris Replication system configuration with bandwidth throttling and dynamic spatial prioritization | ADR-0001, Arch Ch. 1 ✅ |
| **TR-net-003** | Contested open-world mob and boss aggro synchronization across up to 32 players in single zone | ADR-0001, Arch Ch. 1 ✅ |

## Stories

| # | Story | Type | Status | Governing ADR |
|---|---|---|---|---|
| 001 | [`Dedicated Server Locomotion Prediction & Lag Compensation Rewind`](story-001-locomotion-lag-compensation.md) | Logic | Ready | ADR-0001 |
| 002 | [`Iris Dynamic Spatial Prioritization & Ghost Body Disconnect Protection`](story-002-iris-spatial-ghost-body.md) | Integration | Ready | ADR-0001 |
| 003 | [`Contested Threat Table & Boss Aggro Synchronization`](story-003-contested-threat-table.md) | Logic | Ready | ADR-0001 |
| 004 | [`Shared Posture Finisher Priority & Instanced Loot Allocation`](story-004-shared-finisher-instanced-loot.md) | Integration | Ready | ADR-0001 |

## Definition of Done

This epic is complete when:
- All stories created from this epic are implemented, reviewed, and closed via `/story-done`
- Dedicated server runs headlessly with `-nullrhi` maintaining 60Hz tick rate
- Iris spatial filtering throttles distant actor updates (Tier 3: 5Hz) while maintaining 60Hz replication in the combat bubble ($\le 15\text{m}$)
- Client network bandwidth stays strictly under 50 KB/s down and 20 KB/s up in 20-player combat load tests
- Lag compensation rewind query accurately rewinds and validates hits with $\le 0.5\text{ms}$ query time under simulated 150ms player ping
- All acceptance criteria from [`design/gdd/multiplayer-coop.md`](file:///mnt/Data/Projects/project-games/design/gdd/multiplayer-coop.md) are verified
- Implementation conforms strictly to [`docs/architecture/control-manifest.md`](file:///mnt/Data/Projects/project-games/docs/architecture/control-manifest.md)

## Next Step

Validate readiness for implementation using `/story-readiness production/epics/foundation-netcode/story-001-locomotion-lag-compensation.md` then `/dev-story`.
