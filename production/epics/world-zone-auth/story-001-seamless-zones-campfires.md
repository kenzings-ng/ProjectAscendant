# Story 001: 3-Tier Seamless Zones, Campfire Sanctuaries & AI Leash

> **Epic**: World Integration, Sanctuaries & Account Authentication Layer  
> **Status**: Ready  
> **Layer**: World / Gameplay  
> **Type**: World / Gameplay  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-23  
> **Last Updated**: 2026-09-23  

## Context

**GDD**: [`design/gdd/zone-system.md`](../../design/gdd/zone-system.md), [`design/gdd/combat-system.md`](../../design/gdd/combat-system.md)  
**Requirement**: `TR-zone-001` (3-tier zones, Campfire 1000cm sanctuary, 2.0s fast travel, AI leash 2500cm)

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Implements `FPAZoneModel` / `UPACampfireSanctuaryComponent` with leash boundary tracking.

---

## Acceptance Criteria

- [ ] **AC-1 (Campfire Sanctuary Boundary - 1000cm)**: Mỗi Đống Lửa (Campfire) kích hoạt một vùng Thánh Địa bán kính **1000cm**:
  - Khi người chơi bước vào: Xóa sạch cừu hận của toàn bộ quái vật đang đuổi theo.
  - Quái vật không thể tiến vào trong bán kính 1000cm (bị chặn bởi rào chắn vô hình).
  - Vô hiệu hóa mọi hành vi PvP bên trong (`State.Pacified`).
- [ ] **AC-2 (Rest & Restoration)**: Tương tác nghỉ ngơi tại Đống Lửa:
  - Hồi phục tức thời **100% HP, Mana, Stamina** và đầy bình Dược phẩm (Flask charges).
  - Đặt lại điểm hồi sinh (Spawn Anchor) về vị trí Đống Lửa này.
  - Làm hồi sinh lại toàn bộ quái thường trong phân vùng (trừ Boss/Miniboss).
- [ ] **AC-3 (Fast Travel Teleportation - 2.0s)**: Cho phép dịch chuyển tức thời giữa 2 Đống Lửa đã kích hoạt:
  - Thời gian niệm chú cố định **2.00 giây** (không tốn vàng).
  - Niệm chú bị hủy ngay lập tức nếu người chơi di chuyển hoặc nhận sát thương.
- [ ] **AC-4 (AI Leash Boundary - 2500cm)**: Quái vật khi bị người chơi dụ cách xa vị trí xuất phát quá **2500cm** (hoặc khi chạm ranh giới Thánh địa 1000cm):
  - Lập tức nhận trạng thái bất tử `State.Invulnerable`, xóa sổ bảng cừu hận.
  - Tự động quay về vị trí ban đầu với tốc độ tăng $+50\%$.
  - Hồi phục đầy đủ 100% HP và Posture ngay khi về đến tổ.

---

## Implementation Notes

1. **`PAZoneTypes.h`**:
   - `EPAZoneTier`: `Tier1_VerdantFrontier`, `Tier2_AshenWilderness`, `Tier3_ForbiddenSanctum`.
   - `EPACampfireState`: `Dormant`, `Active`, `Resting`, `FastTraveling`.
   - `FPACampfireData`: CampfireId, ZoneTier, Location, bIsLit.
   - `FPAZoneModel`: Pure data model quản lý danh sách campfire, kiểm tra khoảng cách Sanctuary 1000cm, tiến trình niệm chú 2.0s, và logic AI leash 2500cm.
2. **`PACampfireComponent.h` / `PACampfireComponent.cpp`**:
   - ActorComponent gắn vào Campfire Actor.

---

## QA Test Cases

- **Test 1: Sanctuary Boundary**: Di chuyển vào phạm vi <=1000cm -> bInsideSanctuary = true, aggro cleared.
- **Test 2: Campfire Rest**: Gọi Rest -> hồi phục 100% tài nguyên, cập nhật SpawnAnchor.
- **Test 3: Fast Travel**: Bắt đầu niệm chú -> sau 2.0s thành công; di chuyển hoặc nhận sát thương ở 1.0s -> hủy niệm chú.
- **Test 4: AI Leash**: Khoảng cách quái >2500cm -> kích hoạt LeashReturn, Invulnerable = true, speed +50%.
