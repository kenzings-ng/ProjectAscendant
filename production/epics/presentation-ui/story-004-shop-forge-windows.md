# Story 004: Merchant Shop & Blacksmith Forge Interactive Windows

> **Epic**: Presentation & Interface Layer (Combat HUD & Interactive UI)  
> **Status**: ✅ Done  
> **Layer**: Presentation  
> **Type**: UI  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-19  
> **Last Updated**: 2026-09-23  

## Context

**GDD**: [`design/gdd/merchant-economy.md`](../../design/gdd/merchant-economy.md), [`design/gdd/blacksmithing-system.md`](../../design/gdd/blacksmithing-system.md)  
**Requirement**: `TR-hud-004` (Interactive Merchant Shop & Blacksmith Forge CommonUI widgets)

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Server-Authoritative transactions, distance validation <= 300cm, InCombat close)
- [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](../../docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md) (Inventory slot syncing)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `UPAMerchantShopWidget` and `UPABlacksmithForgeWidget`.

---

## Acceptance Criteria

- [x] **AC-1 (2-Column Merchant Shop Interface)**: Displays NPC catalog on left column and Player inventory on right column. Includes tab navigation: `[Buy]`, `[Sell]`, `[Buyback]`. Buyback tab displays up to 10 entries with exact refund prices. Displays 20% surcharge indicator when player Karma < -50 at Tier 2.
- [x] **AC-2 (Blacksmith Forge Anvil Interface)**: Central anvil item slot for target equipment, satellite slots displaying required materials and gold (red if deficient, green if sufficient). Ward slot accepts `item_blacksmith_ward`. Shows stat delta preview (e.g. *Attack: 50 -> 58 (+8)*).
- [x] **AC-3 (0.8s Hold-to-Craft Safe Interaction)**: Enhance and Boss Soul Forging actions require holding the forge button for 0.80 seconds (filling a progress bar) before dispatching the Server RPC, preventing accidental resource consumption.
- [x] **AC-4 (Proximity & In-Combat Auto-Close)**: Opens via interaction prompt (E key) when within 300cm of NPC/Anvil. Automatically closes if distance exceeds 500cm or player enters combat (`InCombat == true`).

---

## Implementation Notes

1. **`PAMerchantShopWidget.h` / `PAMerchantShopWidget.cpp`**:
   - `InitializeShop(UPAMerchantComponent* MerchantComp, UPAInventoryComponent* PlayerInv, UPACurrencyComponent* PlayerWallet, int32 PlayerKarma)`
   - `SelectCatalogItem(int32 Index)`
   - `SelectInventoryItem(int32 SlotIndex)`
   - `ExecuteBuy()`, `ExecuteSell()`, `ExecuteBuyback()`, `ExecuteSellAllJunk()`
2. **`PABlacksmithForgeWidget.h` / `PABlacksmithForgeWidget.cpp`**:
   - `InitializeForge(UPABlacksmithComponent* ForgeComp, UPAInventoryComponent* PlayerInv, UPACurrencyComponent* PlayerWallet)`
   - `SetTargetEquipmentSlot(int32 SlotIndex)`
   - `ToggleWardStone(bool bUseWard)`
   - `UpdateHoldProgress(float DeltaTime, bool bIsHolding)`
   - `OnHoldCompleted()` -> Dispatches `Server_RequestEnhanceWithWard`

---

## QA Test Cases

- **Test 1: Shop 2-Column Model**: Catalog and Inventory lists populate accurately from components.
- **Test 2: Buyback Tab**: Reflects exactly 10 FIFO slots.
- **Test 3: 0.8s Hold Progress**: Hold for 0.4s -> progress is 0.5; release before 0.8s -> cancels without firing RPC; hold >= 0.8s -> triggers RPC.
- **Test 4: Auto-Close**: Distance > 500cm or `bInCombat == true` triggers `CloseWidget()`.
