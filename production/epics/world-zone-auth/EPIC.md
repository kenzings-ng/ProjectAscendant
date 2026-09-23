# Epic: World Integration, Sanctuaries & Account Authentication Layer

> **Layer**: World / Auth  
> **GDD**: [`design/gdd/zone-system.md`](../../design/gdd/zone-system.md), [`design/gdd/authentication-account-system.md`](../../design/gdd/authentication-account-system.md)  
> **Architecture Module**: World Partition, Netcode & Security Architecture ([`docs/architecture/architecture.md`](../../docs/architecture/architecture.md))  
> **Status**: ✅ Complete  
> **Stories**: 4/4 Stories Done  

## Overview

Hoàn thiện tầng Thế giới Mở & Xác thực Tài khoản (**World Integration & Auth Layer**) - Sprint Backend cuối cùng (Sprint 6) để khép lại toàn bộ 18 GDD của *Project Ascendant*:
1. **Seamless Zones & Campfire Sanctuaries (`zone-001`)**: Phân tầng 3 vùng thế giới mở nối liền (Tier 1 Outpost $\to$ Tier 2 Wilderness $\to$ Tier 3 Sanctum), ranh giới bảo vệ Thánh địa Campfire bán kính $1000\text{cm}$ (xóa cừu hận, cấm PvP, hồi 100% HP/MP/Flask), dịch chuyển nhanh Fast Travel $2.0\text{s}$, và ranh giới xích quái AI Leash Boundary $2500\text{cm}$ (hồi máu & bất tử quay về).
2. **Dynamic Difficulty Scaling & Instanced Loot (`zone-002`)**: Công thức co giãn độ khó động lực (DDS) cho Boss HP ($+50\%$ mỗi người chơi thêm) và Posture ($+35\%$), ngưỡng đóng góp nhận thưởng Instanced Loot ($\ge 5\%$ tổng sát thương HP hoặc $\ge 10\%$ Posture) triệt tiêu đánh hôi.
3. **5-Tier Karma & Death Penalties (`zone-003`)**: Máy trạng thái 5 bậc Danh dự (Righteous $\to$ Neutral $\to$ Aggressor $120\text{s} \to$ Criminal $\to$ Wanted Outlaw $<-50$), cơ chế rơi đồ khi chết PvE (50% Vàng vào Tàn tích Tro) vs PvP Victim (mất 25% Vàng cho sát thủ) vs Wanted Outlaw (rơi 100% Vàng, lưu đày ngục tối Lao Động 5 phút hoặc đào 20 khối quặng).
4. **Account Authentication & Fast Playtest Subsystem (`auth-001`)**: `UPAAccountSubsystem` kế thừa `UGameInstanceSubsystem` sống qua chuyển map, chế độ 1-Click Fast Playtest $(<0.1\text{s})$, kiểm tra chuẩn RFC 5322, và sinh chuỗi mã xác thực `PA-TOKEN-[UUID]-[TIMESTAMP]` cho Dedicated Server PreLogin handshake.

## Stories

| ID | Title | Type | Status | Governing ADR |
|---|---|---|---|---|
| 001 | [`3-Tier Seamless Zones, Campfire Sanctuaries & AI Leash`](story-001-seamless-zones-campfires.md) | World / Gameplay | ✅ Done | ADR-0001 |
| 002 | [`Dynamic Difficulty Scaling & Contested Instanced Loot`](story-002-dynamic-difficulty-instanced-loot.md) | Combat / Netcode | ✅ Done | ADR-0001, ADR-0003 |
| 003 | [`5-Tier Karma State Machine & Death Penalties Matrix`](story-003-karma-death-penalties.md) | Economy / Combat | ✅ Done | ADR-0001 |
| 004 | [`Account Subsystem, 1-Click Fast Playtest & Server Token Handshake`](story-004-account-subsystem-auth-handshake.md) | Security / Subsystem | ✅ Done | ADR-0001 |

## Definition of Done
- [ ] 100% các Stories đạt chuẩn Acceptance Criteria.
- [ ] Pure Data Model cho Zone, DDS, Karma, và Auth Subsystem hỗ trợ headless testing.
- [ ] Tích hợp mượt mà với các hệ thống trước đó (Boss Encounter, Attribute Set, FastArray Inventory).
- [ ] Tất cả unit/automation tests chạy Pass 100%.
