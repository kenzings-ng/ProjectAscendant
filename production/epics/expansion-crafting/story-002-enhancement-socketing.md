# Story 002: Enhancement Pipeline +4 to +10 & Socketing

> **Epic**: Expansion Crafting & Blacksmithing Forge  
> **Status**: Done  
> **Layer**: Expansion  
> **Type**: Logic  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-18  
> **Last Updated**: 2026-09-18  

## Context

**GDD**: [`design/gdd/blacksmithing-system.md`](../../design/gdd/blacksmithing-system.md)  
**Requirement**: `TR-crft-002`  

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Server-Authoritative crafting, enhancement RNG resolution, and atomic item modification)
- [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](../../docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md) (Item instance mutation for enhancement level and socketed gems)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Extends `UPABlacksmithComponent` and `FPABlacksmithFormulas` for Tier 2 Wilderness and Tier 3 Ancient Sanctuary Forges.

**Control Manifest Rules (Expansion Layer)**:
- Required: Server-authoritative validation for all enhancement and gem socketing operations.
- Required: Atomic resource deduction (Gold, materials, gems, ward stones) in the same server tick as item modification.
- Forbidden: Client-side speculative item enhancement or socket mutation.
- Guardrail: Items NEVER break or disappear on enhancement failure. +4 to +6 failure keeps current level; +7 to +10 failure drops 1 level unless protected by `item_blacksmith_ward`.

---

## Acceptance Criteria

- [x] **AC-1 (Enhancement +4 to +6 - Tier 2 Wilderness Forge)**: Cường hóa +4 đến +6 tại Tier 2 Forge. Tỷ lệ thành công: +4 (70%), +5 (60%), +6 (50%). Thất bại giữ nguyên cấp độ hiện tại, tiêu hao nguyên liệu và vàng. Chặn cường hóa vượt mốc +6 tại Tier 2.
- [x] **AC-2 (Enhancement +7 to +10 - Tier 3 Ancient Sanctuary Forge)**: Cường hóa +7 đến +10 tại Tier 3 Forge. Tỷ lệ thành công: +7 (40%), +8 (30%), +9 (25%), +10 (15%). Thất bại tụt chính xác 1 cấp (ví dụ: +7 lên +8 thất bại tụt về +6). Nếu có Đá Bảo Hộ (`item_blacksmith_ward`), tiêu hao đá và giữ nguyên cấp. TUYỆT ĐỐI không vỡ đồ.
- [x] **AC-3 (Gem Socketing & Unsocketing)**: Đục tối đa 2 Lỗ Khảm Ngọc (Gem Sockets) cho trang bị từ Bậc Rare trở lên tại Tier 2 Forge. Khảm ngọc (`ruby`, `sapphire`, `topaz`) vào socket trống. Tháo ngọc với phí 100 Vàng hoàn trả ngọc về kho đồ và đưa socket về trạng thái trống.

---

## Implementation Notes

1. **Blacksmith Types (`PABlacksmithTypes.h`)**:
   - Extended `EPACraftingError` with socketing and ward error codes (`ItemCannotBeSocketed`, `MaxSocketsReached`, `InvalidSocketIndex`, `SocketNotEmpty`, `SocketEmpty`, `InvalidGemItem`, `InventoryFull`, `NoWardItem`).
   - Extended `FPABlacksmithFormulas`:
     - `GetEnhancementSuccessRate(int32 TargetLevel)`
     - `GetMaxGemSockets(EPABlacksmithTier ForgeTier, EPAItemRarity Rarity)`
     - `GetUnsocketGoldFee()`
     - `IsGemItem(FName ItemId)`
2. **Blacksmith Component (`PABlacksmithComponent.h` / `PABlacksmithComponent.cpp`)**:
   - `EnhanceItem` with `bUseWard` support and RNG resolution.
   - `SetTestRollOverride(float InRoll)` for deterministic TDD tests.
   - `UnlockSocket(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, EPACraftingError& OutError)`
   - `SocketGem(UPAInventoryComponent* Inventory, int32 EquipmentSlotIndex, int32 SocketIndex, FName GemItemId, EPACraftingError& OutError)`
   - `UnsocketGem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 EquipmentSlotIndex, int32 SocketIndex, EPACraftingError& OutError)`
   - New Server RPCs: `Server_RequestEnhanceWithWard`, `Server_RequestUnlockSocket`, `Server_RequestSocketGem`, `Server_RequestUnsocketGem`.
