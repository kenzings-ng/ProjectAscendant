# Story 003: Paperdoll Equipment Binding & GAS Attribute Integration

> **Epic**: Inventory & 5-Tier Item Database  
> **Status**: Complete  
> **Layer**: Foundation  
> **Type**: Integration  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-09-16  

## Context

**GDD**: [`design/gdd/inventory-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/inventory-system.md)  
**Requirement**: `TR-inv-002`  
*(Requirement text lives in `docs/architecture/tr-registry.yaml` — Item instance data structure (UID, DataAsset ID, Rarity, Durability, Suffixes) serialized to JSON/Binary)*  

**ADR Governing Implementation**: [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md) (Primary), [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) (Secondary)  
**ADR Decision Summary**: Connects inventory item equipment slots directly to the Gameplay Ability System, granting and revoking `GameplayEffect` handles on equip/unequip, managing Quickbar consumables, and processing Skill Book ability unlocks.

**Engine**: Unreal Engine 5.7 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Manages `FActiveGameplayEffectHandle` array on character pawn and queries `UAbilitySystemComponent` ability specs.

**Control Manifest Rules (Foundation Layer)**:
- Required: Equipping gear must apply GameplayEffects to `UAscendantAttributeSet`; unequipping must cleanly revoke active effect handles.
- Forbidden: Never modify base character attributes directly without GameplayEffects; Never read or write transient stats to DataAssets.
- Guardrail: Equip/unequip calculation overhead $\le 0.1\text{ms}$ per action on server game thread.

---

## Acceptance Criteria

*From GDD `design/gdd/inventory-system.md`, scoped to this story:*

- [x] **AC-1 (6-Slot Paperdoll System)**: Implements 6 designated equipment slots: Mainhand Weapon, Offhand/Shield, Body Armor, Amulet, Ring 1, and Ring 2.
- [x] **AC-2 (GAS GameplayEffect Binding)**: Equipping an item with stat modifiers applies a dynamic `UGameplayEffect` to the character's `UAbilitySystemComponent` and stores the active handle; unequipping removes the effect and restores stats immediately.
- [x] **AC-3 (Quickbar 1–4 Consumables)**: Pressing keys 1–4 uses the assigned consumable potion, playing a 0.8s drinking action (`potion_use_duration = 0.8s`), reducing movement speed by 30% during drinking, and decrementing the stack count by 1.
- [x] **AC-4 (Skill Book Class-Lock & Grant)**: Right-clicking a Skill Book checks `RequiredClassTag` and verifies character is not in combat (`!State.InCombat`); upon validation, consumes 1 book and grants the corresponding `UGameplayAbility` to the character's ASC.

---

## Implementation Notes

*Derived from ADR-0003 & ADR-0002 Implementation Guidelines:*

1. **Equipment Component (`UPAEquipmentComponent`)**:
   - Manages: `TMap<EEquipmentSlot, FPAInventoryItemEntry> EquippedItems;`
   - Manages: `TMap<EEquipmentSlot, FActiveGameplayEffectHandle> ActiveEffectHandles;`
2. **Equip Logic**:
   - On `Server_EquipItem(int32 BackpackSlot, EEquipmentSlot TargetSlot)`:
     ```cpp
     if (UItemStaticDataAsset* Data = ItemEntry.StaticData) {
         if (TSubclassOf<UGameplayEffect> GEClass = Data->EquipGameplayEffect) {
             FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GEClass, 1.0f, ASC->MakeEffectContext());
             ActiveEffectHandles.Add(TargetSlot, ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get()));
         }
     }
     ```
3. **Unequip Logic**:
   - If slot is unequipped, fetch handle: `ASC->RemoveActiveGameplayEffect(ActiveEffectHandles[TargetSlot]);`
4. **Quickbar Slots (`int32 QuickbarSlots[4]`)**:
   - Maps indices 0–3 to specific inventory item UIDs. Activating slot invokes `Server_UseConsumable(int32 QuickbarIndex)`.

---

## Out of Scope

*Handled by neighbouring stories — do not implement here:*

- Story 001: FastArray data structures and grid capacity.
- Story 002: Server transactional RPCs, move/split validation, and anti-duping.

---

## QA Test Cases

*Written by qa-lead at story creation:*

- **AC-2 Test: Armor Stat Application & Removal**:
  - Given: Character has 0 Armor and 500 Max Health. Body Armor DataAsset provides $+50\text{ Armor}$ and $+100\text{ Max Health}$.
  - When: Armor is equipped into Body Armor slot.
  - Then: `UAscendantAttributeSet` asserts `Armor == 50.0f` and `MaxHealth == 600.0f`.
  - When 2: Armor is unequipped back into backpack.
  - Then 2: `UAscendantAttributeSet` immediately reverts to `Armor == 0.0f` and `MaxHealth == 500.0f`.

- **AC-3 Test: Quickbar Potion Consumption**:
  - Given: Health Potion assigned to Quickbar Slot 1 with stack count 5. Character has 300/500 HP.
  - When: Player presses key `[1]`.
  - Then: Stack count decrements to 4; movement speed drops by 30% for 0.8s; after 0.8s, Health increases by 150 points.
  - Edge cases: When stack reaches 0, quickbar slot clears automatically.

- **AC-4 Test: Skill Book Class Lock Validation**:
  - Given: Ranger character (has tag `Class.Ranger`) with a Vanguard Skill Book (`Class.Vanguard`).
  - When: Player attempts to use Skill Book.
  - Then: Server rejects request, book remains in backpack, and UI displays "Chức nghiệp không phù hợp".

---

## Test Evidence

**Story Type**: Integration  
**Required evidence**: `tests/integration/inventory/paperdoll_gas_equipment_test.cpp` — must exist and pass automated CI  
**Status**: [x] Passed (`ProjectAscendant/Tests/integration/inventory/paperdoll_gas_equipment_test.cpp`, 5 test blocks passing, build succeeded)  

---

## Dependencies

- Depends on: Story 002 (Transaction RPCs) & foundation-attributes Story 001 (AttributeSet)
- Unlocks: Epic Completion (Foundation Inventory fully ready for implementation)
