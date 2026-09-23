# Story 003: Anatomical Part Breaking & Skill Disabling Matrix

> **Epic**: Encounter & Boss Mechanics Layer  
> **Status**: ✅ Done  
> **Layer**: Combat / Encounter  
> **Type**: Gameplay / Systems  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-23  
> **Last Updated**: 2026-09-23  

## Context

**GDD**: [`design/gdd/stagger-system.md`](../../design/gdd/stagger-system.md), [`design/gdd/boss-ai.md`](../../design/gdd/boss-ai.md)  
**Requirement**: `TR-stgr-002` (Boss part damage tracking, skill lock in Behavior Tree, weakpoint reveal +50%, craft drop spawning)

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md)
- [`ADR-0002: GAS Integration & PaperZD Pixel Sprites`](../../docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `FPAPartBreakingModel` / `UPAPartBreakingComponent` linking damage distribution to boss moveset locks.

---

## Acceptance Criteria

- [x] **AC-1 (Anatomical Part Health Tracking)**: Mỗi bộ phận giải phẫu của Trùm sở hữu thanh máu độc lập (tính theo tỷ lệ Max HP của Trùm):
  - **Horn (Sừng)**: Chiếm $20\%$ Max HP.
  - **Tail (Đuôi)**: Chiếm $15\%$ Max HP.
  - **Chest Armor (Giáp ngực)**: Chiếm $25\%$ Max HP.
  - Mọi sát thương đánh trúng bộ phận đều đồng thời trừ vào Tổng máu (Total HP) của Trùm.
- [x] **AC-2 (Behavior Tree Skill Disabling)**: Khi một bộ phận bị phá vỡ (Part HP $\le 0$):
  - Khi **Horn** gãy: Khóa vĩnh viễn chiêu `Iron Horn Charge` trong Behavior Tree, làm choáng trùm 1.5s.
  - Khi **Tail** đứt: Khóa vĩnh viễn chiêu `Iron Tail Sweep` trong Behavior Tree.
- [x] **AC-3 (Chest Weakpoint Vulnerability)**: Khi **Chest Armor** vỡ:
  - Vùng ngực chuyển thành **Tử huyệt (Weakpoint)**.
  - Mọi đòn đánh trúng ngực sau đó được nhân hệ số sát thương $+50\%$ ($Damage \times 1.50$).
- [x] **AC-4 (Crafting Reagent Drop Notification)**: Khi bộ phận vỡ thành công, phát tín hiệu kèm thông tin vật phẩm rơi đặc thù (`Item_Beast_Horn_Shard`, `Item_Dragon_Tail_Sinew`, `Item_Hardened_Carapace`) tại tọa độ điểm gãy.

---

## Implementation Notes

1. **`PAPartBreakingTypes.h`**:
   - `EPABossPartType`: `Horn`, `Tail`, `ChestArmor`.
   - `FPABossPartData`: MaxHealth, CurrentHealth, bIsBroken, DropItemId, DisabledAttack.
   - `FPAPartBreakingModel`: Pure data model quản lý danh sách bộ phận, trừ máu đồng thời, kích hoạt sự kiện gãy bộ phận và tính toán hệ số sát thương yếu điểm (+50%).
2. **`PAPartBreakingComponent.h` / `PAPartBreakingComponent.cpp`**:
   - ActorComponent lắng nghe sát thương theo hitbox bộ phận và đồng bộ cờ Blackboard cho Behavior Tree.

---

## QA Test Cases

- **Test 1: Part Health & Dual Damage**: Đánh 500 damage vào Sừng -> Máu Sừng giảm 500, Tổng máu Trùm cũng giảm 500.
- **Test 2: Horn Break & Skill Ban**: Khi Máu Sừng về 0 -> Cờ `bCanUseHornCharge` chuyển thành false, kỹ năng bị loại khỏi danh sách chọn của AI.
- **Test 3: Tail Break & Skill Ban**: Khi Máu Đuôi về 0 -> Cờ `bCanUseTailSweep` chuyển thành false.
- **Test 4: Chest Weakpoint Multiplier**: Khi Giáp ngực vỡ -> Đòn đánh tiếp theo có giá trị gốc 100 gây ra 150 damage.
