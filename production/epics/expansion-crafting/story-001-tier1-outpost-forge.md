# Story 001: Blacksmith Tier 1 Outpost Forge & Item Repair

> **Epic**: Expansion Crafting & Blacksmithing Forge  
> **Status**: Done  
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
- [x] **AC-2 (Item & Skill Book Salvaging)**: Disassembling equipment or skill books yields Ash Shards based on rarity tier (Common: 1, Uncommon: 3, Rare: 10, Epic: 25, Legendary: 75; SkillBook: 5), destroys the source item from `UPAInventoryComponent`, and credits Ash Shards to `UPACurrencyComponent`. Rejects locked items (`bIsLocked == true`).
- [x] **AC-3 (Safe Enhancement +1 to +3)**: Outpost Forge (Tier 1) supports safe enhancement up to +3 with 100% success rate, consuming Gold and Iron Ore (`iron_ore`), incrementing `EnhancementLevel`, and preventing enhancement beyond the Tier 1 ceiling (+3).
- [x] **AC-4 (Interaction & Server Authority Guardrails)**: All operations enforce server authority, interaction distance $\le 300\text{cm}$ between character and forge, and reject requests when `In-Combat == true` or funds/materials are insufficient.

---

## Implementation Notes

1. **Blacksmith Types (`PABlacksmithTypes.h`)**:
   - `EPABlacksmithTier`: `Tier1_Outpost`, `Tier2_Wilderness`, `Tier3_Sanctuary`.
   - `EPACraftingError`: `None`, `InsufficientGold`, `InsufficientMaterials`, `ItemNotFound`, `ItemLocked`, `MaxDurabilityAlready`, `MaxTierLevelReached`, `DistanceExceeded`, `InCombat`, `ServerRejected`.
   - Helper struct `FPABlacksmithFormulas`:
     - `CalculateRepairCost(int32 BasePrice, float CurrentDurability, float MaxDurability)`
     - `GetSalvageAshShards(EPAItemRarity Rarity, EPAItemCategory Category)`
     - `GetEnhancementRequirements(int32 TargetLevel, int32& OutGoldCost, int32& OutIronOreCost)`
2. **Blacksmith Component (`PABlacksmithComponent.h` / `PABlacksmithComponent.cpp`)**:
   - `RepairItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, EPACraftingError& OutError)`
   - `SalvageItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32& OutShardsGained, EPACraftingError& OutError)`
   - `EnhanceItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, EPACraftingError& OutError)`

---

## Out of Scope

- Story 002: Tier 2 Wilderness Forge (+4 to +6 enhancement failure retention, gem sockets).
- Story 003: Tier 3 Ancient Sanctuary Forge (+7 to +10, Prismatic socket, Boss Soul divine forging).

---

## QA Test Cases

- **Test 1: Repair Cost & Restoration**:
  - Given an item with BasePrice 200 Gold and Durability 40% (40.0 / 100.0).
  - Repair cost is `ceil(200 * 0.25 * 0.6) = 30 Gold`.
  - Wallet with 100 Gold pays 30 Gold (balance becomes 70 Gold); item durability restores to 100.0%.
  - Attempting to repair an item already at 100% durability returns `MaxDurabilityAlready`.
- **Test 2: Salvage Equipment & Skill Books**:
  - Salvaging a Rare (Tier 3) chestplate yields 10 Ash Shards; item is removed from slot.
  - Salvaging a Skill Book yields 5 Ash Shards.
  - Salvaging a locked item (`bIsLocked == true`) returns `ItemLocked` and leaves item intact.
- **Test 3: Safe Enhancement +1 to +3**:
  - Weapon +0 enhanced to +1 consumes 100 Gold + 2 Iron Ore; level becomes +1.
  - Weapon +1 enhanced to +2 consumes 250 Gold + 4 Iron Ore; level becomes +2.
  - Weapon +2 enhanced to +3 consumes 500 Gold + 8 Iron Ore; level becomes +3.
  - Attempting to enhance weapon +3 at Tier 1 Forge returns `MaxTierLevelReached` (requires Tier 2 Forge).
