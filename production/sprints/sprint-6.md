# Sprint 6 — 2026-10-17 to 2026-10-31

> **Sprint**: Sprint 6 (World Integration, Sanctuaries & Account Authentication Layer)  
> **Stage**: Production (Final Backend Sprint)  
> **Review Mode**: Lean  
> **Capacity**: 10 days (80 hours) | Buffer (20%): 2 days (16 hours) | Available: 8 days (64 hours)  

---

## Sprint Goal

Khép lại toàn bộ Tầng Máy Chủ & Hệ Thống Thế Giới Mở (**World Integration, Sanctuaries & Account Auth Layer**) cho *Project Ascendant*, hoàn thành 100% tài liệu Game Design Documents (18/18 GDDs):
Triển khai hệ thống 3 phân vùng thế giới mở nối liền với Thánh Địa Đống Lửa (**Campfire Sanctuary 1000cm**, hồi 100% máu/mana/thể lực, dịch chuyển nhanh Fast Travel 2.0s) và Ranh giới xích quái **AI Leash 2500cm**; Công thức co giãn độ khó động lực (**Dynamic Difficulty Scaling - DDS**) cho Boss và Ngưỡng đóng góp nhận thưởng (**Instanced Loot >= 5% HP / 10% Posture**); Máy trạng thái Danh dự 5 bậc (**Karma System**) kèm kịch bản phạt chết PvE/PvP và Nhà tù Lao Động; cùng Phân hệ Tài khoản (**UPAAccountSubsystem**), chế độ 1-Click Fast Playtest (<0.1s) và bắt tay mã Token xác thực máy chủ Dedicated Server.

---

## Tasks

### Must Have (Critical Path — 16 hours / 2.0 days)

| ID | Task | Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `zone-001` | [3-Tier Seamless Zones, Campfire Sanctuaries & AI Leash](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/world-zone-auth/story-001-seamless-zones-campfires.md) | `world-designer` | 1.0 (8h) | `world-001`, `cmbt-001` | AC-1: Thánh địa 1000cm xóa cừu hận & cấm PvP; AC-2: Nghỉ ngơi hồi 100% và ghim điểm hồi sinh; AC-3: Fast travel 2.0s; AC-4: Quái vượt 2500cm bật bất tử quay về. |
| `zone-002` | [Dynamic Difficulty Scaling & Contested Instanced Loot](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/world-zone-auth/story-002-dynamic-difficulty-instanced-loot.md) | `gameplay-programmer` | 1.0 (8h) | `boss-001`, `stgr-001` | AC-1: Co giãn Max HP (+50%/người); AC-2: Co giãn Posture (+35%/người); AC-3: Kháng khống chế Anti-Zerg (+30%/+50%); AC-4: Ngưỡng nhận Loot >=5% HP hoặc >=10% Posture. |

### Should Have (16 hours / 2.0 days)

| ID | Task | Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `zone-003` | [5-Tier Karma State Machine & Death Penalties Matrix](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/world-zone-auth/story-003-karma-death-penalties.md) | `systems-designer` | 1.0 (8h) | `econ-001`, `zone-001` | AC-1: 5 bậc Karma (-100 đến +100, Aggressor 120s); AC-2: Chết PvE rơi 50% Vàng vào Tàn tích Tro; AC-3: Nạn nhân PvP mất 25% Vàng, sát nhân trừ -30 Karma; AC-4: Kẻ Wanted rơi 100% Vàng & đi tù 5 phút / 20 quặng. |
| `auth-001` | [Account Subsystem, 1-Click Fast Playtest & Server Token Handshake](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/world-zone-auth/story-004-account-subsystem-auth-handshake.md) | `backend-engineer` | 1.0 (8h) | `net-001` | AC-1: UPAAccountSubsystem sống qua nạp map; AC-2: 1-Click Playtest < 0.1s; AC-3: Xác thực RFC 5322 & password min 6 ký tự; AC-4: Sinh Token PA-TOKEN- & kiểm tra PreLogin. |

---

## Total Planned Scope
- **Must Have**: 2 stories (16 hours / 2.0 days)
- **Should Have**: 2 stories (16 hours / 2.0 days)
- **Total**: 4 stories (32 hours / 4.0 days) vs **Available Capacity**: 8.0 days (64 hours)

---

## Definition of Done for this Sprint
- [ ] 100% Must Have và Should Have tasks hoàn thành và vượt qua kiểm thử đơn vị & tích hợp.
- [ ] Hoàn tất toàn bộ logic phân vùng, Thánh địa, Co giãn độ khó, Danh dự & Xác thực tài khoản.
- [ ] 100% các hệ thống Gameplay & Backend của 18 GDD đạt chuẩn hoàn thiện (Backend Feature Complete).
- [ ] Bộ kiểm thử tự động Unreal Automation Tests đạt 100% Pass.
