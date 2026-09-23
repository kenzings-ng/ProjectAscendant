# Story 003: 5-Tier Karma State Machine & Death Penalties Matrix

> **Epic**: World Integration, Sanctuaries & Account Authentication Layer  
> **Status**: Ready  
> **Layer**: Economy / Combat  
> **Type**: Gameplay / Systems  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-23  
> **Last Updated**: 2026-09-23  

## Context

**GDD**: [`design/gdd/zone-system.md`](../../design/gdd/zone-system.md), [`design/gdd/merchant-economy.md`](../../design/gdd/merchant-economy.md)  
**Requirement**: `TR-zone-003` (5-tier karma, Wanted state, death penalties for PvE/PvP victim/Wanted outlaw, labor prison)

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `FPAKarmaModel` / `UPAKarmaComponent` with death penalty resolution.

---

## Acceptance Criteria

- [ ] **AC-1 (5-Tier Karma State Machine)**: Giá trị Karma từ $[-100, +100]$ điều khiển 5 bậc danh dự:
  - **Righteous (+50 to +100)**: Tên xanh lục, giảm 10% phí sửa đồ, thưởng danh vọng khi săn kẻ Wanted.
  - **Neutral (0 to +49)**: Tên trắng, trạng thái mặc định được pháp luật bảo hộ.
  - **Aggressor (Tạm thời 120s)**: Tên xám khi tấn công người vô tội trước, bị cấm vào thị trấn trong thời gian đếm ngược.
  - **Criminal (-1 to -49)**: Tên cam, lính gác từ chối phục vụ.
  - **Wanted Outlaw (-50 to -100)**: Tên đỏ kèm biểu tượng đầu lâu, bị lính gác tấn công ngay khi gặp, bị phát sóng tọa độ toàn server mỗi 30s.
- [ ] **AC-2 (PvE Death Penalty - Ash Remnant)**: Khi chết do quái vật/môi trường (PvE):
  - Rơi **50% số Vàng hiện có** và **100% Tàn Trang (Skill Shards)** thành một Tàn Tích Tro (Ash Remnant) tại tọa độ tử vong.
  - Khóa quyền nhặt trong 30 phút cho chính chủ. Trang bị đang mặc được bảo toàn 100%.
- [ ] **AC-3 (Innocent PvP Victim Protection)**: Khi người vô tội ($\text{Karma} \ge 0$) bị PK chết:
  - Chỉ mất **25% Vàng** (chuyển thẳng cho kẻ sát nhân làm tiền thưởng).
  - Bảo toàn 100% Tàn Trang và Trang bị đang mặc; không tạo Tàn tích Tro (tránh ức chế bị phạt kép).
  - Kẻ sát nhân bị trừ ngay lập tức **-30 điểm Karma**.
- [ ] **AC-4 (Wanted Outlaw Death & Labor Prison)**: Khi kẻ có lệnh truy nã ($\text{Karma} < -50$) bị tiêu diệt:
  - Rơi toàn bộ **100% Vàng** và **100% Tàn Trang** (ai cũng có thể nhặt tự do).
  - Có 15% tỷ lệ rơi 1 vật phẩm ngẫu nhiên trong túi đồ.
  - Bị tống vào **Nhà tù Lao Động (Labor Prison)** trong 5 phút thời gian thực HOẶC đào đủ 20 khối quặng để được trả tự do sớm. Khi ra tù, Karma được ấn định lại ở mức $-49$.

---

## Implementation Notes

1. **`PAKarmaTypes.h`**:
   - `EPAKarmaTier`: `Righteous`, `Neutral`, `Aggressor`, `Criminal`, `WantedOutlaw`.
   - `EPADeathScenario`: `PvE`, `PvPVictim`, `WantedOutlaw`.
   - `FPADeathPenaltyResult`: GoldLost, ShardsLost, bAshRemnantCreated, bSentToPrison, PrisonDurationSeconds, OreRequired.
   - `FPAKarmaModel`: Pure data model quản lý điểm Karma, thời gian đếm ngược Aggressor (120s), phân giải kịch bản chết và hình phạt ngục tù.

---

## QA Test Cases

- **Test 1: Karma Tier Transition**: Điểm 75 -> Righteous; 0 -> Neutral; -25 -> Criminal; -60 -> Wanted Outlaw.
- **Test 2: PvE Death Math**: Có 1000 Gold, 200 Shards chết PvE -> mất 500 Gold, 200 Shards vào Ash Remnant.
- **Test 3: PvP Victim Math**: Người vô tội có 1000 Gold bị PK -> mất 250 Gold cho sát thủ, giữ nguyên Shards, sát thủ bị trừ -30 Karma.
- **Test 4: Wanted Outlaw Prison**: Kẻ Wanted bị diệt -> mất 100% Gold, vào tù 300s / 20 quặng, ra tù reset về -49.
