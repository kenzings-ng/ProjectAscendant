# Story 001: Blacksmith Tier 1 Outpost Forge & Item Repair

> **Epic**: Expansion Crafting & Blacksmithing Forge  
> **Status**: In Progress  
> **X14 (2026-10-10) — đối chiếu trạng thái** (trước đây ghi `Done`): Bằng chứng: `Crafting.Blacksmith`, `Network.ServerAuthority.ServerComputedCostsAndTier`, `...InteractionRangeAndCombatEnforced`, `...ForeignOwnerComponentsRejected` trong `Tests/evidence/x11b-e34f428-ue-automation.log` (77/77 PASS). AC-4 (server authority) đã sửa ở X11a/X11b (PR #21, #23) nhưng client chỉ được mô phỏng (DECISIONS.md §11). Còn mở: AC-2 salvage trang bị ra shard trong khi GDD ghi ra quặng (M13); AC-3 phí cường hoá là bảng cố định, lệch công thức GDD (B3-12); chưa có forge actor nào trong game (M6). Bảng tổng: `production/qa/x14-status-reconciliation.md`.  
> **Layer**: Expansion  
> **Type**: Logic  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-18  
> **Last Updated**: 2026-09-18  

## Context

**GDD**: [`design/gdd/blacksmithing-system.md`](../../design/gdd/blacksmithing-system.md)  
**Requirement**: `TR-crft-001`  

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Server-Authoritative crafting, repair validation, and atomic resource consumption)
- [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](../../docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md) (Item instance mutation for durability and enhancement level)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `UPABlacksmithComponent` interacting with `UPAInventoryComponent` and `UPACurrencyComponent`.

**Control Manifest Rules (Expansion Layer)**:
- Required: Server-authoritative validation for all crafting, repair, and enhancement operations.
- Required: Atomic resource deduction (Gold, materials, input items) in the same server tick as item modification.
- Forbidden: Client-side speculative item enhancement or durability restoration.
- Guardrail: Enhancements up to +3 are 100% loss-free and items NEVER break or disappear on failure.

---

## Acceptance Criteria

- [x] **AC-1 (Durability Repair Engine)**: `UPABlacksmithComponent` repairs equipped or inventory items calculating cost via `repair_cost = ceil(base_price * 0.25 * (1.0 - durability_pct))`, deducting Gold from `UPACurrencyComponent` and resetting `CurrentDurability = 100.0f`.
- [x] **AC-2 (Item & Skill Book Salvaging)**: Disassembling equipment or skill books yields Skill Shards based on rarity (Common: 1, Uncommon: 3, Rare: 10, Epic: 25, Legendary: 75; Skill Books by skill rarity `EPASkillRarity` Normal/Rare/Epic/Mythic: 1/3/8/25 per `skill-progression-system.md` — *corrected 2026-10-09 (X6), was flat 5*), destroys the source item from `UPAInventoryComponent`, and credits Skill Shards to `UPACurrencyComponent`. Rejects locked items (`bIsLocked == true`). *(Cập nhật 2026-10-10 (X13): "rarity tier" → "rarity"; độ hiếm không dùng chữ "Tier", DECISIONS.md §1.)* *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*
- [x] **AC-3 (Safe Enhancement +1 to +3)**: Outpost Forge (Tier 1) supports safe enhancement up to +3 with 100% success rate, consuming Gold and Iron Ore (`iron_ore`), incrementing `EnhancementLevel`, and preventing enhancement beyond the Tier 1 ceiling (+3). *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*
- [x] **AC-4 (Interaction & Server Authority Guardrails)**: All operations enforce server authority, interaction distance $\le 300\text{cm}$ between character and forge, and reject requests when `In-Combat == true` or funds/materials are insufficient.

---

## Implementation Notes

1. **Blacksmith Types (`PABlacksmithTypes.h`)**:
   - `EPABlacksmithTier`: `Tier1_Outpost`, `Tier2_Wilderness`, `Tier3_Sanctuary`.
   - `EPACraftingError`: `None`, `InsufficientGold`, `InsufficientMaterials`, `ItemNotFound`, `ItemLocked`, `MaxDurabilityAlready`, `MaxTierLevelReached`, `DistanceExceeded`, `InCombat`, `ServerRejected`.
   - Helper struct `FPABlacksmithFormulas`:
     - `CalculateRepairCost(int32 BasePrice, float CurrentDurability, float MaxDurability)`
     - `GetSalvageSkillShards(EPAItemRarity Rarity, EPAItemCategory Category, EPASkillRarity SkillRarity)` + `GetSkillBookSalvageShards(EPASkillRarity)` *(updated 2026-10-09, X6)*
     - `GetEnhancementRequirements(int32 TargetLevel, int32& OutGoldCost, int32& OutIronOreCost)`
2. **Blacksmith Component (`PABlacksmithComponent.h` / `PABlacksmithComponent.cpp`)**:
   - `RepairItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, EPACraftingError& OutError)`
   - `SalvageItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32& OutShardsGained, EPACraftingError& OutError)`
   - `EnhanceItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, EPACraftingError& OutError)`

---

## Out of Scope

- Story 002: Tier 2 Wilderness Forge (+4 to +6 enhancement failure retention, gem sockets).
- Story 003: Tier 3 Ancient Sanctuary Forge (+7 to +10, Prismatic socket, Boss Soul Legendary forging).

---

## QA Test Cases

- **Test 1: Repair Cost & Restoration**:
  - Given an item with BasePrice 200 Gold and Durability 40% (40.0 / 100.0).
  - Repair cost is `ceil(200 * 0.25 * 0.6) = 30 Gold`.
  - Wallet with 100 Gold pays 30 Gold (balance becomes 70 Gold); item durability restores to 100.0%.
  - Attempting to repair an item already at 100% durability returns `MaxDurabilityAlready`.
- **Test 2: Salvage Equipment & Skill Books**:
  - Salvaging a Rare chestplate yields 10 Skill Shards; item is removed from slot. *(Cập nhật 2026-10-10 (X7): bỏ "(Tier 3)" — độ hiếm không đánh số Tier, DECISIONS.md §1.)*
  - Salvaging a Skill Book yields shards by skill rarity: Normal 1 / Rare 3 / Epic 8 / Mythic 25 *(corrected 2026-10-09, X6; was flat 5)*.
  - Salvaging a locked item (`bIsLocked == true`) returns `ItemLocked` and leaves item intact.
- **Test 3: Safe Enhancement +1 to +3**:
  - Weapon +0 enhanced to +1 consumes 100 Gold + 2 Iron Ore; level becomes +1.
  - Weapon +1 enhanced to +2 consumes 250 Gold + 4 Iron Ore; level becomes +2.
  - Weapon +2 enhanced to +3 consumes 500 Gold + 8 Iron Ore; level becomes +3.
  - Attempting to enhance weapon +3 at Tier 1 Forge returns `MaxTierLevelReached` (requires Tier 2 Forge).
