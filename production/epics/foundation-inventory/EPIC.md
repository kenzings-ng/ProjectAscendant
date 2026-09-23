# Epic: Inventory & 5-Tier Item Database

> **Layer**: Foundation  
> **GDD**: [`design/gdd/inventory-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/inventory-system.md)  
> **Architecture Module**: Inventory, Items & Economy Data Architecture ([`docs/architecture/architecture.md`](file:///mnt/Data/Projects/project-games/docs/architecture/architecture.md) Chapter 5)  
> **Status**: Ready  
> **Stories**: 3 Stories Created  

## Overview

This epic implements the server-authoritative 30-slot grid inventory system, 5-tier item database, and transactional replication architecture for *Project Ascendant*. It establishes the `FFastArraySerializer` network delta-serialization structures, separation between immutable item definitions (`UItemStaticDataAsset`) and mutable instance properties (`FItemInstanceData`), and atomic transactional Server RPCs (Move, Split, Equip, Drop, Salvage) that eliminate item duplication exploits under network packet loss.

## Governing ADRs

| ADR | Decision Summary | Engine Risk |
|---|---|---|
| [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md) | Standardizes FastArray delta replication for inventory items, server transaction validation, and DataAsset separation. | 🟢 LOW |
| [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md) | Enforces Dedicated Server authority over all persistent player inventory and currency states. | 🟡 MEDIUM |

## GDD Requirements

| TR-ID | Requirement | ADR Coverage |
|---|---|---|
| **TR-inv-001** | Server-authoritative 30-slot grid inventory replicating item entries via `FFastArraySerializer` | ADR-0003, Arch Ch. 5 ✅ |
| **TR-inv-002** | Item instance data structure (UID, DataAsset ID, Rarity, Durability, Suffixes) serialized to JSON/Binary | ADR-0003, Arch Ch. 5 ✅ |
| **TR-inv-003** | Transaction validation with atomic item transfers preventing item duplication under network packet loss | ADR-0003, Arch Ch. 5 ✅ |

## Stories

| # | Story | Type | Status | Governing ADR |
|---|---|---|---|---|
| 001 | [`FastArray Grid Inventory & Item DataAsset Hierarchy`](story-001-fastarray-inventory-data.md) | Logic | Ready | ADR-0003 |
| 002 | [`Server-Authoritative Transaction RPCs & Duplication Safeguards`](story-002-transaction-rpcs-safeguards.md) | Logic | Ready | ADR-0003, ADR-0001 |
| 003 | [`Paperdoll Equipment Binding & GAS Attribute Integration`](story-003-paperdoll-gas-binding.md) | Integration | Ready | ADR-0003, ADR-0002 |

## Definition of Done

This epic is complete when:
- All stories created from this epic are implemented, reviewed, and closed via `/story-done`
- Inventory mutations (moving items, splitting stacks, equipping gear) replicate with minimal delta bandwidth ($\le 0.8\text{ms}$ per server tick)
- Transaction validation tests confirm that simulated packet loss and rapid double-click inputs never result in duplicated items or negative stack counts
- Serialization and deserialization of `FItemInstanceData` maintain 100% fidelity across save/load and server reconnect cycles
- All acceptance criteria from [`design/gdd/inventory-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/inventory-system.md) are verified
- Implementation strictly complies with [`docs/architecture/control-manifest.md`](file:///mnt/Data/Projects/project-games/docs/architecture/control-manifest.md)

## Next Step

Validate readiness for implementation using `/story-readiness production/epics/foundation-inventory/story-001-fastarray-inventory-data.md` then `/dev-story`.
