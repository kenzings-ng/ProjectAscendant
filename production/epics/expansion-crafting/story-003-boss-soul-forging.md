# Story 003: Boss Soul Forging & Divine Equipment

> **Epic**: Expansion Crafting & Blacksmithing Forge  
> **Status**: Done  
> **Layer**: Expansion  
> **Type**: Integration  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-19  
> **Last Updated**: 2026-09-19  

## Context

**GDD**: [`design/gdd/blacksmithing-system.md`](../../design/gdd/blacksmithing-system.md)  
**Requirement**: `TR-crft-003` (Tier 3 Ancient Sanctuary Forge: Boss Soul divine forging, Prismatic sockets, backpack expansion sequence)

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Server-Authoritative crafting, recipe resolution, material/currency atomic deductions, anti-duping)
- [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](../../docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md) (Item creation, slot manipulation, capacity unlocks)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Extends `UPABlacksmithComponent` and `FPABlacksmithFormulas` for Tier 3 Ancient Sanctuary Forge services.

**Control Manifest Rules (Expansion Layer)**:
- Required: Server-authoritative validation for all crafting, socketing, and inventory capacity expansion operations.
- Required: Atomic resource deduction (currency, monster parts, void ore, boss soul) in the same server tick as item delivery.
- Forbidden: Client-side speculative item creation or socket/capacity modification.
- Guardrail: Only Tier 3 Ancient Sanctuary Forge can forge Divine Tier 5 equipment and unlock the 3rd Prismatic gem socket. Backpack capacity sequence strictly enforced (Tier 1 -> 40, Tier 2 -> 50, Tier 3 -> 60 max).

---

## Acceptance Criteria

- [x] **AC-1 (Boss Soul Divine Forging - Tier 3 Exclusive)**: Rèn đúc Thần Binh Bậc 5 (Divine Equipment) độc quyền tại Tier 3 Ancient Sanctuary Forge. Yêu cầu: 1x Linh Hồn Lãnh Chúa (`item_boss_soul_*`), 4x Mảnh vỡ bộ phận Boss (`item_boss_part_*`), 5x Quặng Hư Không Cổ Đại (`void_ore`), và 5,000 Vàng. Kiểm tra nghiêm ngặt nguyên liệu và Dedicated Server Authority; từ chối khi thực hiện tại Tier 1 hoặc Tier 2 Forge.
- [x] **AC-2 (Prismatic Socket - 3rd Gem Socket Expansion)**: Cho phép đục Lỗ Khảm thứ 3 (Prismatic Socket / Lỗ Đa Sắc) trên trang bị Bậc Rare trở lên (đặc biệt Immortal & Divine) tại Tier 3 Ancient Sanctuary Forge. Hỗ trợ khảm tất cả các loại ngọc thông thường và Ngọc Đa Sắc (`prismatic_gem` / `gem_prismatic`).
- [x] **AC-3 (Backpack Capacity Expansion Sequence)**: Nâng cấp sức chứa kho đồ theo 3 cấp độ tương ứng với 3 Bậc Lò Rèn:
  - Bậc 1 (30 -> 40 ô tại Tier 1 Forge): 10x Da Thú + 5x Quặng Đồng + 500 Vàng.
  - Bậc 2 (40 -> 50 ô tại Tier 2 Forge): 15x Da Cường Lực + 5x Quặng Sắt Đen + 2,000 Vàng.
  - Bậc 3 (50 -> 60 ô tại Tier 3 Forge): 5x Vảy Đuôi Boss + 2x Quặng Hư Không + 8,000 Vàng.
  - Chặn nâng cấp vượt cấp (ví dụ chưa lên 40 ô mà đòi lên 50 ô, hoặc cố nâng cấp Bậc 2 tại Tier 1 Forge). Khi đã đạt trần 60/60 ô, từ chối giao dịch với mã lỗi `MaxBackpackCapacity`.
- [x] **AC-4 (Interaction & Combat Guardrails)**: Toàn bộ dịch vụ Tier 3 (Đúc Thần Binh, Prismatic Socket, Mở Rộng Kho Đồ) tuân thủ kiểm tra khoảng cách tương tác (≤ 300cm), từ chối khi đang giao chiến (`InCombat == true`), và bắt buộc chạy trên Dedicated Server có Authority.

---

## Implementation Notes

1. **Blacksmith Types (`PABlacksmithTypes.h`)**:
   - Extended `EPACraftingError`: `MaxBackpackCapacity`, `BackpackSequenceMismatch`.
   - `FPABackpackExpansionRequirements`: Cấu trúc mô tả nguyên liệu, vàng và Bậc lò rèn yêu cầu cho từng mốc (40, 50, 60).
   - Helper methods:
     - `GetBackpackExpansionRequirement(int32 CurrentSlots, FPABackpackExpansionRequirements& OutReq)`
     - `GetBossSoulForgingCost(int32& OutGoldCost, int32& OutVoidOreCount, int32& OutBossPartCount, int32& OutBossSoulCount)`
     - `IsPrismaticGem(FName ItemId)`
     - `IsPrismaticSocket(int32 SocketIndex)`
2. **Blacksmith Component (`PABlacksmithComponent.h` / `PABlacksmithComponent.cpp`)**:
   - `ForgeBossSoulEquipment(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, UItemStaticDataAsset* DivineItemData, FName BossSoulItemId, FName BossPartItemId, FName VoidOreItemId, EPACraftingError& OutError)`
   - `ExpandBackpackCapacity(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, EPACraftingError& OutError)`
   - Delegates:
     - `FPAOnBossSoulForged`: `ItemId`, `RarityTier`
     - `FPAOnBackpackExpanded`: `OldCapacity`, `NewCapacity`, `GoldCost`
   - Server RPCs:
     - `Server_RequestForgeBossSoul`
     - `Server_RequestExpandBackpack`

---

## Out of Scope

- Client UMG 3D anvil particle animations (CommonUI widget bindings handled in UI sprint).
- Audio cues (Niagara VFX & SFX asset integration).
