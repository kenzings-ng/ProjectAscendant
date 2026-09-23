# Story 002: Merchant Vendor Network & Buyback Window

> **Epic**: Expansion Economy & Merchant Network  
> **Status**: Done  
> **Layer**: Expansion  
> **Type**: Logic  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-18  
> **Last Updated**: 2026-09-18  

## Context

**GDD**: [`design/gdd/merchant-economy.md`](../../design/gdd/merchant-economy.md)  
**Requirement**: `TR-econ-002`  

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Server-Authoritative currency transactions, distance check $\le 300\text{cm}$, out-of-combat enforcement)
- [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](../../docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md) (Inventory item manipulation, locked items, junk flags)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `UPAMerchantComponent` interacting with `UPAInventoryComponent` and `UPACurrencyComponent`.

**Control Manifest Rules (Expansion Layer)**:
- Required: Server-authoritative validation for all purchase, sell, and buyback transactions.
- Required: Atomic resource exchange (currency deduction + item delivery) in the same server tick.
- Forbidden: Client-side speculative item acquisition or gold injection.
- Guardrail: FIFO buyback window of 10 slots per merchant session to safeguard against accidental sales.

---

## Acceptance Criteria

- [x] **AC-1 (Vendor Catalog & Purchasing)**: `UPAMerchantComponent` provides a catalog of stock items with prices. Purchasing an item validates Gold balance, ensures available inventory slot, deducts Gold from `UPACurrencyComponent`, and adds the item to `UPAInventoryComponent` atomically on the server.
- [x] **AC-2 (Item Selling & Vendor Penalty)**: Selling inventory items yields Gold based on `vendor_sell_price = floor(base_price * vendor_sell_penalty)` (default 30%). Rejects locked items (`bIsLocked == true`). Supports bulk selling all items flagged as junk (`bIsJunk == true`).
- [x] **AC-3 (Buyback Window FIFO)**: Sold items are recorded into a session Buyback list (up to 10 slots). When full, the oldest sold item is discarded (FIFO). Buying back costs the exact Gold received when sold, returns the item with intact dynamic data, and removes it from the Buyback list.
- [x] **AC-4 (Interaction & Server Authority Guardrails)**: All trading operations enforce server authority, interaction distance $\le 300\text{cm}$ between character and merchant, reject operations when `In-Combat == true`, and validate parameters.

---

## Implementation Notes

1. **Merchant Types (`PAMerchantTypes.h`)**:
   - `EPAMerchantTier`: `Tier1_Outpost`, `Tier2_Wilderness`, `Tier3_Sanctuary`.
   - `EPATransactionError`: `None`, `InsufficientGold`, `InventoryFull`, `ItemNotFound`, `ItemLocked`, `DistanceExceeded`, `InCombat`, `BuybackEmpty`, `ServerRejected`.
   - `FPAMerchantCatalogEntry`: `UItemStaticDataAsset* ItemData`, `int32 PriceGold`, `int32 AvailableStock` (-1 for infinite).
   - `FPABuybackItemEntry`: `UItemStaticDataAsset* ItemData`, `int32 Quantity`, `FPAItemInstanceData DynamicData`, `int32 BuybackPriceGold`, `FDateTime SoldTime`.
2. **Merchant Component (`PAMerchantComponent.h` / `PAMerchantComponent.cpp`)**:
   - `BuyItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 CatalogIndex, int32 Quantity, EPATransactionError& OutError)`
   - `SellItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32 Quantity, EPATransactionError& OutError)`
   - `SellAllJunk(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32& OutTotalGoldReceived, int32& OutItemsSold, EPATransactionError& OutError)`
   - `BuybackItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 BuybackIndex, EPATransactionError& OutError)`
   - `ValidateInteraction(const AActor* InteractingActor, bool bInCombat, EPATransactionError& OutError) const`

---

## Out of Scope

- Story 003: Wilderness Wandering Smuggler timed restock and limited stock rotation.
- Post-MVP: Auction House & Player-to-Player Trading.
