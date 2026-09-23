# Story 001: FastArray Grid Inventory & Item DataAsset Hierarchy

> **Epic**: Inventory & 5-Tier Item Database  
> **Status**: Complete  
> **Layer**: Foundation  
> **Type**: Logic  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-09-16  

## Context

**GDD**: [`design/gdd/inventory-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/inventory-system.md)  
**Requirement**: `TR-inv-001`  
*(Requirement text lives in `docs/architecture/tr-registry.yaml` — Server-authoritative 30-slot grid inventory replicating item entries via FFastArraySerializer)*  

**ADR Governing Implementation**: [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md)  
**ADR Decision Summary**: Replaces standard `TArray` replication with `FFastArraySerializer` structs to enable delta-only network serialization for inventory slots, linking immutable definitions to `UPrimaryDataAsset` and mutable instances to runtime entries.

**Engine**: Unreal Engine 5.7 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `NetDeltaSerialize` on `FPAInventoryItemList` and `FFastArraySerializerItem` on `FPAInventoryItemEntry`.

**Control Manifest Rules (Foundation Layer)**:
- Required: All replicated inventory collections must inherit from `FFastArraySerializer` and implement `NetDeltaSerialize`.
- Forbidden: Never replicate raw `TArray` for item collections; Never store runtime mutable state inside `UPrimaryDataAsset`.
- Guardrail: FastArray replication delta overhead $\le 0.8\text{ms}$ per server tick.

---

## Acceptance Criteria

*From GDD `design/gdd/inventory-system.md`, scoped to this story:*

- [x] **AC-1 (Backpack Capacity & Slot Limit)**: Character spawns with 30 unlocked grid slots (`base_inventory_slots = 30`), expandable up to 60 slots (`max_inventory_slots = 60`) in increments of 10.
- [x] **AC-2 (Item DataAsset Hierarchy)**: Base item definitions inherit from `UPrimaryDataAsset` (`UItemStaticDataAsset`), defining ItemID, Rarity Tier (1–5), Category, Base Value, and Max Stack Size.
- [x] **AC-3 (Stacking Rules Enforcement)**: Adding items respects category stack limits: Equipment and Skill Books max stack 1; Consumables max stack 20; Materials and Shards max stack 999.
- [x] **AC-4 (FastArray Delta Serialization)**: Modifying a single slot in the 30-slot grid serializes only the changed entry across Iris replication channels, verifying zero full-array cloning overhead.

---

## Implementation Notes

*Derived from ADR-0003 Implementation Guidelines:*

1. **FastArray Item Entry Structs (`Source/ProjectAscendant/Public/Inventory/PAInventoryTypes.h`)**:
   ```cpp
   USTRUCT(BlueprintType)
   struct FPAInventoryItemEntry : public FFastArraySerializerItem
   {
       GENERATED_BODY()
       UPROPERTY() int32 SlotIndex = INDEX_NONE;
       UPROPERTY() FGuid ItemInstanceUID;
       UPROPERTY() TObjectPtr<UItemStaticDataAsset> StaticData = nullptr;
       UPROPERTY() int32 StackCount = 1;
       UPROPERTY() FItemInstanceData DynamicData;
   };

   USTRUCT(BlueprintType)
   struct FPAInventoryItemList : public FFastArraySerializer
   {
       GENERATED_BODY()
       UPROPERTY() TArray<FPAInventoryItemEntry> Items;
       bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
       {
           return FFastArraySerializer::FastArrayDeltaSerialize<FPAInventoryItemEntry, FPAInventoryItemList>(Items, DeltaParms, *this);
       }
   };
   ```
2. **Capacity & Slot Index Clamping**:
   - `UPAInventoryComponent` maintains `int32 CurrentMaxSlots = 30;`
   - Adding an item checks for existing matching stacks first, then populates the lowest available `SlotIndex < CurrentMaxSlots`.

---

## Out of Scope

*Handled by neighbouring stories — do not implement here:*

- Story 002: Server transactional RPCs (move, split, drop) and anti-duplication guards.
- Story 003: 6-slot Paperdoll equipment and GAS attribute binding.

---

## QA Test Cases

*Written by qa-lead at story creation:*

- **AC-1 Test: Grid Capacity & Expansion**:
  - Given: A newly created character actor with `UPAInventoryComponent`.
  - When: Querying unlocked slot count.
  - Then: Asserts `CurrentMaxSlots == 30`.
  - When 2: Server unlocks +10 slots via upgrade function.
  - Then 2: Asserts `CurrentMaxSlots == 40`, and slots up to index 39 can accept items.

- **AC-3 Test: Category Stacking Rules**:
  - Given: An inventory slot containing 15 Health Potions (Consumables).
  - When: Player adds 10 more Health Potions.
  - Then: The slot stacks to 20 (max consumable stack), and the remaining 5 potions overflow into a new empty slot.
  - When 2: Player picks up a second Sword of the same type.
  - Then 2: The sword occupies an independent slot with `StackCount == 1`.

- **AC-4 Test: Delta Network Replication**:
  - Given: A 30-slot populated inventory on Dedicated Server.
  - When: A single item in Slot 5 is moved to Slot 6.
  - Then: `NetDeltaSerialize` replicates only the delta changes for Slot 5 and 6; network packet payload size is $< 80\text{ bytes}$.

---

## Test Evidence

**Story Type**: Logic  
**Required evidence**: `ProjectAscendant/Tests/unit/inventory/fast_array_inventory_test.cpp` — covers AC-1, AC-2, AC-3, AC-4  
**Status**: [x] Complete  

---

## Dependencies

- Depends on: None (Foundation story for Inventory System)
- Unlocks: Story 002 (Transaction RPCs) & Story 003 (Paperdoll GAS Binding)
