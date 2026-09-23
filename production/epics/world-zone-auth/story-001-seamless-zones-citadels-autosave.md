# Story 001: 3-Tier Seamless Zones, Citadel Safe Zones, Auto-Save & Relog Return

> **Epic**: World Integration, Sanctuaries & Account Authentication Layer  
> **Status**: ✅ Done  
> **Layer**: World / Gameplay  
> **Type**: World / Gameplay  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-23  
> **Last Updated**: 2026-09-23  

## Context

**GDD**: [`design/gdd/zone-system.md`](../../design/gdd/zone-system.md), [`design/gdd/combat-system.md`](../../design/gdd/combat-system.md)  
**Requirement**: `TR-zone-001` (3-tier zones, Citadel Safe Zones, Auto-Save on entry, Relog return to last visited citadel, 2.0s fast travel, AI leash 2500cm)

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Implements `FPAZoneModel` / `UPACitadelComponent` with Safe Zone boundary tracking, instant Auto-Save, and relog spawn location resolution.

---

## Acceptance Criteria

- [x] **AC-1 (Citadel Safe Zone Boundary)**: Mỗi Tòa Thành (Citadel / Stronghold) kích hoạt một Vùng An Toàn bán kính **SafeZoneRadius** (mặc định 5000cm):
  - Khi người chơi bước vào: Xóa sạch cừu hận của toàn bộ quái vật đang đuổi theo (`DropAggro`).
  - Quái vật không thể tiến vào trong thành (bị chặn bởi NavMesh barrier).
  - Vô hiệu hóa mọi hành vi giao tranh PvP bên trong (`State.Pacified`).
- [x] **AC-2 (Citadel Enter Auto-Save & Full Restoration)**: Ngay khi người chơi bước chân vào cổng Tòa Thành:
  - Máy chủ lập tức kích hoạt cơ chế **Tự Động Lưu (Auto-Save)** toàn bộ dữ liệu nhân vật.
  - Hồi phục tức thời **100% HP, Mana, Stamina** và đầy 5/5 bình Dược phẩm (Flask charges).
  - Cập nhật Tòa Thành này thành `LastVisitedCitadelId` (Tòa thành người chơi vào gần nhất).
- [x] **AC-3 (Relog Return to Last Visited Citadel)**: Khi người chơi thoát game (Quit, Disconnect, ngắt kết nối mạng) ở bất kỳ đâu ngoài hoang dã hoặc trong hầm ngục:
  - Khi đăng nhập lại vào game (Login / Relog), máy chủ luôn đưa nhân vật xuất hiện an toàn tại **Tòa Thành mà người chơi ghé thăm gần nhất (Last Visited Citadel)** với 100% tài nguyên.
  - Ngăn chặn hoàn toàn tình trạng bị quái cắn chết lén hoặc bị phục kích khi đang nạp game.
- [x] **AC-4 (Citadel Fast Travel Teleportation - 2.0s)**: Cho phép dịch chuyển tức thời giữa các Tòa Thành đã khám phá:
  - Thời gian niệm chú cố định **2.00 giây** (không tốn vàng).
  - Niệm chú bị hủy ngay lập tức nếu người chơi di chuyển hoặc nhận sát thương.
- [x] **AC-5 (AI Leash Boundary - 2500cm)**: Quái vật khi bị người chơi dụ cách xa vị trí xuất phát quá **2500cm** (hoặc khi chạm ranh giới cổng thành):
  - Lập tức nhận trạng thái bất tử `State.Invulnerable`, xóa sổ bảng cừu hận.
  - Tự động quay về vị trí ban đầu với tốc độ tăng $+50\%$.
  - Hồi phục đầy đủ 100% HP và Posture ngay khi về đến tổ.

---

## Implementation Notes

1. **`PAZoneTypes.h`**:
   - `EPAZoneTier`: `Tier1_VerdantFrontier`, `Tier2_AshenWilderness`, `Tier3_ForbiddenSanctum`.
   - `EPACitadelState`: `Undiscovered`, `Discovered`, `FastTraveling`.
   - `FPACitadelNode`: CitadelId, CitadelDisplayName, ZoneTier, WorldLocation, SafeZoneRadius, bIsDiscovered, bIsLastVisited.
   - `FPAAutoSaveRecord`: PlayerId, LastVisitedCitadelId, SavedSpawnLocation, AutoSaveTimestamp, bSavedSuccessfully, SavedHP/MP/Stamina/Flasks.
   - `FPAZoneModel`: Pure data model quản lý danh sách Tòa Thành, kiểm tra khoảng cách SafeZoneRadius, kích hoạt Auto-Save tức thì khi vào thành, tra cứu vị trí xuất hiện khi Relog (`GetRelogSpawnTransform`), tiến trình niệm chú 2.0s, và logic AI leash 2500cm.
2. **`PACitadelComponent.h` / `PACitadelComponent.cpp`**:
   - ActorComponent gắn vào Citadel Actor/Volume.
   - Quản lý sự kiện người chơi bước vào thành, kích hoạt Auto-Save, phát Delegate `OnCitadelEnteredAndSaved`.

---

## QA Test Cases

- **Test 1: Safe Zone Boundary**: Di chuyển vào phạm vi <=5000cm -> `IsInsideSafeZone` = true, quái drop aggro, PvP pacified.
- **Test 2: Citadel Enter Auto-Save**: Bước vào thành -> kích hoạt Auto-Save thành công, hồi 100% HP/MP/Stamina/Flasks, ghi nhận `LastVisitedCitadelId`.
- **Test 3: Relog Return**: Thoát game ngoài hoang dã -> khi đăng nhập lại, `GetRelogSpawnTransform` luôn trả về vị trí Tòa Thành vào gần nhất.
- **Test 4: Citadel Fast Travel**: Niệm chú 2.0s giữa các thành đã khám phá thành công; di chuyển hoặc nhận sát thương -> hủy niệm chú.
- **Test 5: AI Leash**: Khoảng cách quái >2500cm -> kích hoạt LeashReturn, `IsBeyondLeash` = true.
