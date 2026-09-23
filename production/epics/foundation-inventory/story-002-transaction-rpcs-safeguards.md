# Story 002: Server-Authoritative Transaction RPCs & Duplication Safeguards

> **Epic**: Inventory & 5-Tier Item Database  
> **Status**: Complete  
> **Layer**: Foundation  
> **Type**: Logic  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-09-16  

## Context

**GDD**: [`design/gdd/inventory-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/inventory-system.md)  
**Requirement**: `TR-inv-003`  
*(Requirement text lives in `docs/architecture/tr-registry.yaml` — Transaction validation with atomic item transfers preventing item duplication under network packet loss)*  

**ADR Governing Implementation**: [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md) (Primary), [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Secondary)  
**ADR Decision Summary**: Enforces atomic transaction verification on Dedicated Server for all slot manipulations, preventing race-condition item duplication or desync under network latency, and implementing QoL flags and overflow stashes.

**Engine**: Unreal Engine 5.7 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements Server Reliable RPCs with client-side rollback on rejection and unique 128-bit `FGuid` instance validation.

**Control Manifest Rules (Foundation Layer)**:
- Required: Item move/split/drop operations must send atomic Server RPCs; Server validates source/target slots in single thread step.
- Forbidden: Never mutate slot counts client-side; Never permit selling or salvaging items with `bIsLocked == true`.
- Guardrail: Transaction validation latency $\le 0.05\text{ms}$ per request on server game thread.

---

## Acceptance Criteria

*From GDD `design/gdd/inventory-system.md`, scoped to this story:*

- [x] **AC-1 (Atomic Move & Swap Validation)**: `Server_MoveItem(int32 SourceSlot, int32 TargetSlot)` executes an atomic swap or merge on the Dedicated Server, verifying item existence and source slot lock state before mutation.
- [x] **AC-2 (Split Stack Validation)**: `Server_SplitStack(int32 SourceSlot, int32 SplitAmount, int32 TargetSlot)` validates that `SplitAmount < SourceStack` and target slot is empty; if invalid, transaction rejects and client UI reverts.
- [x] **AC-3 (QoL Item Lock & Junk Flags)**: Items with `bIsLocked == true` cannot be dropped, sold, or dismantled; pressing `[J]` toggles `bIsJunk == true` which qualifies the item for bulk liquidation at merchants.
- [x] **AC-4 (Overflow Stash Routing)**: When inventory is full ($30/30$), items of Tier Rare or higher dropped by defeated bosses are routed to a 20-slot persistent Overflow Stash (`overflow_stash_limit = 20`) available at campfires.

---

## Implementation Notes

*Derived from ADR-0003 Implementation Guidelines:*

1. **Transactional RPC Implementation (`UPAInventoryComponent`)**:
   ```cpp
   UFUNCTION(Server, Reliable, WithValidation)
   void Server_MoveItem(int32 SourceSlot, int32 TargetSlot, FGuid ExpectedItemUID);
   ```
   - Server checks:
     1. `SourceSlot` has valid item entry matching `ExpectedItemUID`.
     2. `SourceSlot` is not locked by an in-flight trade or crafting lock.
     3. If `TargetSlot` is empty, move item entry and mark `MarkItemDirty`.
     4. If `TargetSlot` holds matching stackable item, increment target and decrement source.
     5. If `TargetSlot` holds different item, execute atomic pointer/data swap.
2. **Anti-Duplication Token**:
   - Each transaction carries an incrementing `uint32 TransactionID`. Server rejects duplicate requests with identical or outdated sequence IDs.
3. **Overflow Stash Manager**:
   - Component tracks `TArray<FPAInventoryItemEntry> OverflowStash;` (clamped at 20 slots).
   - If player backpack cannot fit an eligible rare drop, append to `OverflowStash` and trigger toast notification.

---

## Out of Scope

*Handled by neighbouring stories — do not implement here:*

- Story 001: FastArray data structures and DataAsset definitions.
- Story 003: 6-slot Paperdoll equipment and GAS attribute binding.

---

## QA Test Cases

*Written by qa-lead at story creation:*

- **AC-1 Test: Rapid Double-Click Move (Anti-Duplication)**:
  - Given: A valuable Legendary sword in Slot 0. Attacker client simulates rapid network packet spam, sending two identical `Server_MoveItem` RPCs to Slot 1 within 1ms.
  - When: Server processes both RPCs sequentially.
  - Then: First RPC successfully moves sword to Slot 1; second RPC detects Slot 0 is now empty, rejects the transaction, and asserts only 1 instance of the sword exists.

- **AC-3 Test: Item Lock Protection**:
  - Given: Armor in Slot 3 with `bIsLocked == true`.
  - When: Client attempts to invoke `Server_SellItem` or `Server_DropItem`.
  - Then: Server rejects operation with error code `ITEM_LOCKED`; armor remains intact in Slot 3.

- **AC-4 Test: Full Backpack Overflow Stash Routing**:
  - Given: Player backpack full ($30/30$ slots occupied). Boss drops a Rare Tier Ring.
  - When: Loot pickup is executed.
  - Then: Ring is redirected to campfire `OverflowStash`, occupying slot 0 of 20, and player receives UI notification.

---

## Test Evidence

**Story Type**: Logic  
**Required evidence**: `tests/unit/inventory/inventory_transaction_test.cpp` — must exist and pass automated CI  
**Status**: [x] Passed automated verification in Tests/unit/inventory/inventory_transaction_test.cpp  

---

## Dependencies

- Depends on: Story 001 (FastArray Inventory Data)
- Unlocks: Story 003 (Paperdoll GAS Binding) & Economy Stories
