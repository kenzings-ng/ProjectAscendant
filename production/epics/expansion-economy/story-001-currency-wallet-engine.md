# Story 001: Dual Currency Wallet & Transaction Engine

> **Epic**: Expansion Economy & Merchant Network  
> **Status**: In Progress  
> **X14 (2026-10-10) — đối chiếu trạng thái** (trước đây ghi `Complete`): Bằng chứng: `Economy.CurrencyWallet`, `Itemization.DualCurrencyTransactions` trong `Tests/evidence/x11b-e34f428-ue-automation.log` (77/77 PASS). Tên tiền tệ đã sửa ở X5 (PR #16). Còn mở: AC-2 không có `Server_AddCurrency`/`Server_DeductCurrency`, `Server_TransferCurrency` là chuyển tiền giữa người chơi trong khi DECISIONS §6 hoãn giao dịch (B3-21); AC-3 đường chết luôn áp phạt PvE, kể cả PvP và boss (M18). Bảng tổng: `production/qa/x14-status-reconciliation.md`.  
> **Layer**: Expansion  
> **Type**: Logic  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-18  
> **Last Updated**: 2026-09-18  

## Context

**GDD**: [`design/gdd/merchant-economy.md`](../../design/gdd/merchant-economy.md)  
**Requirement**: `TR-econ-001`, `TR-econ-003`  

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Server-Authoritative transactions, zero client trust)
- [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](../../docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md) (FastArray delta serialization for high-performance MMO replication)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `UPACurrencyComponent` with `FPACurrencyEntry` and `FPACurrencyList` implementing `FFastArraySerializer`.

**Control Manifest Rules (Expansion Layer)**:
- Required: Server-authoritative mutation of player currency balances.
- Required: FastArray delta-only replication across Iris network channels.
- Forbidden: Client-side prediction or speculative modification of currency amounts.
- Guardrail: Atomic transaction execution prevents balance duplication or negative balances under any circumstance.

---

## Acceptance Criteria

- [x] **AC-1 (Dual Currency FastArray Replication)**: `UPACurrencyComponent` manages Gold (`EPACurrencyType::Gold`, max 9,999,999) and Skill Shards / Tàn Trang (`item_skill_shard`, `EPACurrencyType::SkillShards`, max 99,999) replicating state changes to owning client via `FFastArraySerializer` delta serialization.
- [x] **AC-2 (Atomic Transaction RPCs & Anti-Duping)**: Server RPCs (`Server_AddCurrency`, `Server_DeductCurrency`, `Server_TransferCurrency`) validate sender authority, enforce non-negative balances, clamp to maximum capacity, and reject unauthorized or invalid operations with specific error codes (`EPACurrencyTransactionError`). *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*
- [x] **AC-3 (PvE Death Penalty)**: Upon player death in PvE (`HandlePvEDeathPenalty`), exactly 50% of carried Gold (`floor(Gold * 0.50)`) is deducted and prepared for world droplet drop, while 100% of Skill Shards are safeguarded. *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*

---

## Implementation Notes

1. **Currency Types (`PACurrencyTypes.h`)**:
   - `EPACurrencyType`: `Gold`, `SkillShards`.
   - `EPACurrencyTransactionError`: `None`, `InsufficientFunds`, `ExceedsMaxCapacity`, `InvalidAmount`, `TargetNotFound`, `ServerRejected`.
   - `FPACurrencyEntry : public FFastArraySerializerItem` containing `CurrencyType`, `Amount`.
   - `FPACurrencyList : public FFastArraySerializer` wrapping `TArray<FPACurrencyEntry> Entries`.
2. **Currency Component (`PACurrencyComponent.h` / `PACurrencyComponent.cpp`)**:
   - Manages balances, handles `GetCurrency(EPACurrencyType)`, `HasSufficientCurrency(EPACurrencyType, int64)`.
   - Server-only mutators: `Server_AddCurrency`, `Server_DeductCurrency`, `Server_TransferCurrency`.
   - PvE death penalty helper: `HandlePvEDeathPenalty(int64& OutGoldLost)`.
3. **Character Integration (`PABaseCharacter.h` / `PABaseCharacter.cpp`)**:
   - `UPACurrencyComponent` added as default subobject.
   - On death event (`HandleDeath`), calls `CurrencyComponent->HandlePvEDeathPenalty()`.

---

## Out of Scope

- Story 002: NPC Merchant trade shop UI and Buyback window.
- Story 003: Wilderness wandering smuggler and stock refresh timers.

---

## QA Test Cases

- **Test 1: Balance Queries & Limits**:
  - Given a fresh `UPACurrencyComponent`, default balances are 0.
  - Adding Gold up to limit (9,999,999) succeeds; adding beyond limit clamps to 9,999,999.
  - Adding Skill Shards up to limit (99,999) succeeds; clamps to 99,999.
- **Test 2: Atomic Deductions & Insufficient Funds**:
  - Wallet with 500 Gold attempting to deduct 600 Gold returns `InsufficientFunds` error; balance remains 500 Gold.
  - Deducting 300 Gold succeeds; balance becomes 200 Gold.
- **Test 3: Transfer Currency**:
  - Transferring 150 Gold from Player A (balance 500) to Player B (balance 100):
    - Player A becomes 350 Gold.
    - Player B becomes 250 Gold.
  - Invalid amounts ($\le 0$) or insufficient funds reject atomically with zero side-effects.
- **Test 4: PvE Death Penalty**:
  - Character with 1,000 Gold and 50 Skill Shards dies in PvE.
  - Death penalty triggers: Gold becomes 500 (`floor(1000 * 0.5)`), Skill Shards remains 50.
  - Character with 1 Gold dies: Gold becomes 0 (`floor(1 * 0.5)` = 0), drops 1 Gold.
