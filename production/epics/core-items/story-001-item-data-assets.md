# Story 001: Item DataAssets & Quickbar Assets Setup

> **Epic**: Core Items & DataAssets  
> **Status**: Complete  
> **Layer**: Core  
> **Type**: Asset  
> **Estimate**: 8 hours (L)  
> **Manifest Version**: 2026-09-17  
> **Last Updated**: 2026-09-17  

## Context

**GDD**: [`design/gdd/inventory-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/inventory-system.md)  
**Requirement**: `TR-inv-001`, `TR-inv-002`  
**ADR Governing Implementation**: [`ADR-0003: Item DataAsset Architecture & Rarity Matrix`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0003-item-dataasset-architecture-rarity.md)  

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Generates `.uasset` instances of `UItemStaticDataAsset` in `Content/Items/DataAssets/`.

---

## Acceptance Criteria

- [x] **AC-1 (Equipment DataAssets)**: 
  - `DA_Weapon_IronSword`: Slot `MainHand`, Physical Damage +15, Rarity `Uncommon`, MaxStack 1.
  - `DA_Armor_IronPlate`: Slot `Chest`, Armor +40, Rarity `Rare`, MaxStack 1.
- [x] **AC-2 (Consumable Quickbar DataAssets)**:
  - `DA_Potion_Health`: Slot `Quickbar_1`, ConsumableDuration 0.8s, SpeedDebuff 30%, Restores 100 HP, MaxStack 20.
- [x] **AC-3 (Skill Book DataAsset)**:
  - `DA_SkillBook_Dash`: ItemType `SkillBook`, `RequiredClassTag = Class.Vanguard`, `GrantedAbilityClass = UGA_Dash`, MaxStack 1.
