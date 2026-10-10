# Epic: Expansion Economy & Merchant Network

> **Layer**: Expansion  
> **GDD**: [`design/gdd/merchant-economy.md`](../../design/gdd/merchant-economy.md)  
> **Architecture Module**: Economy & Currency Transaction Architecture ([`docs/architecture/architecture.md`](../../docs/architecture/architecture.md))  
> **Status**: In Progress  
> **X14 (2026-10-10) — đối chiếu trạng thái** (trước đây ghi `Complete`): trạng thái epic suy ra từ story (In Progress 2, Review 1). Bảng tổng: `production/qa/x14-status-reconciliation.md`.  
> **Stories**: 3 Stories (3 Done) *(X14 2026-10-10: thực tế In Progress 2, Review 1)*  

## Overview

This epic implements the server-authoritative dual currency economy, merchant trading network, and transactional safeguards for *Project Ascendant*. It establishes the `UPACurrencyComponent` managing Gold and Skill Shards (Tàn Trang, `item_skill_shard`) with FastArray delta replication (`FFastArraySerializer`), atomic server transaction RPCs for trading and transfers, 3-tier merchant vendor interaction with stock rotation, buyback window FIFO management, and the high-stakes PvE death penalty (dropping 50% Gold into an Ash Remnant upon defeat).

## Governing ADRs

| ADR | Decision Summary | Engine Risk |
|---|---|---|
| [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md) | Enforces 100% Dedicated Server authority over persistent player currency, eliminating client-side memory tampering and transaction spoofing. | 🟡 MEDIUM |
| [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](../../docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md) | Standardizes FastArray delta serialization patterns for item and currency network syncing across Iris channels. | 🟢 LOW |

## GDD Requirements

| TR-ID | Requirement | ADR Coverage |
|---|---|---|
| **TR-econ-001** | Dual currency wallet (Gold & Skill Shards) with FastArray replication and atomic server RPCs (Add, Deduct, Transfer) preventing duping | ADR-0001, ADR-0003 ✅ |
| **TR-econ-002** | 3-tier merchant vendor network with distance checks (≤300cm), out-of-combat enforcement, and FIFO buyback window (10 slots) | ADR-0001 ✅ |
| **TR-econ-003** | PvE death penalty dropping 50% Gold into an Ashen Remnant droplet while retaining 100% Skill Shards | ADR-0001 ✅ |

## Stories

| # | Story | Type | Status | Governing ADR |
|---|---|---|---|---|
| 001 | [`Dual Currency Wallet & Transaction Engine`](story-001-currency-wallet-engine.md) | Logic | In Progress | ADR-0001, ADR-0003 |
| 002 | [`Merchant Vendor Network & Buyback Window`](story-002-merchant-vendor-network.md) | Logic | Review | ADR-0001 |
| 003 | [`Wilderness Wandering Smuggler & Limited Stock`](story-003-wandering-smuggler.md) | World | In Progress | ADR-0001 |

## Definition of Done

This epic is complete when:
- `UPACurrencyComponent` replicates Gold and Skill Shards via `FFastArraySerializer` with minimal delta overhead ($\le 0.5\text{ms}$).
- Atomic transaction RPCs ensure money cannot be duplicated or lost across concurrent operations, packet loss, or server desyncs.
- PvE death penalty drops exactly 50% carried Gold into a world droplet, retaining 100% Skill Shards.
- 100% automated test coverage for currency operations passes cleanly.
