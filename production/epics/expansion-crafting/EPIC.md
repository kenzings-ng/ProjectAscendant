# Epic: Expansion Crafting & Blacksmithing Forge

> **Layer**: Expansion  
> **GDD**: [`design/gdd/blacksmithing-system.md`](../../design/gdd/blacksmithing-system.md)  
> **Architecture Module**: Crafting, Enhancement & Durability Repair Architecture ([`docs/architecture/architecture.md`](../../docs/architecture/architecture.md))  
> **Status**: Complete  
> **Stories**: 3 Stories (3 Done)  

## Overview

This epic implements the zone-tiered blacksmithing and item enhancement engine for *Project Ascendant*. Spanning 3 world hazard tiers—from the safe Outpost Forge to the deep Wilderness Forge and the dangerous Ancient Sanctuary Forge—it establishes durability repair mechanisms, equipment and skill book salvaging into Ash Shards, 100% loss-free item enhancement (+1 to +10), gem socketing, and boss soul crafting, strictly bound to Dedicated Server Authority.

## Governing ADRs

| ADR | Decision Summary | Engine Risk |
|---|---|---|
| [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md) | Enforces 100% Dedicated Server authority over crafting, repair transactions, item mutation, and material deduction. | 🟡 MEDIUM |
| [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](../../docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md) | Standardizes item instance modification (`FPAItemInstanceData`) and delta synchronization across network channels. | 🟢 LOW |

## GDD Requirements

| TR-ID | Requirement | ADR Coverage |
|---|---|---|
| **TR-crft-001** | Tier 1 Outpost Forge: Durability repair formula (`repair_cost = ceil(base_price * 0.25 * (1 - durability_pct))`), item/skill-book salvaging to Ash Shards, and 100% safe enhancement (+1 to +3) | ADR-0001, ADR-0003 ✅ |
| **TR-crft-002** | Tier 2 Wilderness Forge: Enhancement +4 to +6 with failure level retention, up to 2 gem socket expansions | ADR-0001, ADR-0003 ✅ |
| **TR-crft-003** | Tier 3 Ancient Sanctuary Forge: Enhancement +7 to +10 (fail drops 1 level, no destruction), Prismatic sockets, Boss Soul divine forging | ADR-0001, ADR-0003 ✅ |

## Stories

| # | Story | Type | Status | Governing ADR |
|---|---|---|---|---|
| 001 | [`Blacksmith Tier 1 Outpost Forge & Item Repair`](story-001-tier1-outpost-forge.md) | Logic | Done | ADR-0001, ADR-0003 |
| 002 | [`Enhancement Pipeline +4 to +10 & Socketing`](story-002-enhancement-socketing.md) | Logic | Done | ADR-0001, ADR-0003 |
| 003 | [`Boss Soul Forging & Divine Equipment`](story-003-boss-soul-forging.md) | Integration | Done | ADR-0001, ADR-0003 |

## Definition of Done

This epic is complete when:
- `UPABlacksmithComponent` correctly resolves Tier 1, 2, and 3 capabilities, enforcing proximity ($\le 300\text{cm}$) and out-of-combat states.
- Equipment repair accurately calculates costs and restores durability to 100%.
- Salvaging equipment or skill books generates the exact expected quantity of Ash Shards and destroys the input item.
- Enhancement strictly adheres to non-destructive design (weapons NEVER shatter or disappear on failure).
- 100% automated tests for crafting calculations, item mutations, and server validations pass cleanly.
