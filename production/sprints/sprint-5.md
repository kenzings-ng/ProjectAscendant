# Sprint 5 — 2026-10-03 to 2026-10-17

> **Sprint**: Sprint 5 (Encounter & Boss Mechanics Layer — Boss AI, Stagger & Perfect Evasion)  
> **Stage**: Production  
> **Review Mode**: Lean  
> **Capacity**: 10 days (80 hours) | Buffer (20%): 2 days (16 hours) | Available: 8 days (64 hours)  

---

## Sprint Goal

Hoàn thành Tầng Giao Chiến & Cơ Chế Đấu Trùm (**Encounter & Boss Mechanics Layer**) cho *Project Ascendant*, hoàn tất vòng lặp chiến đấu đỉnh cao liên kết chặt chẽ với hệ thống HUD ở Sprint 4:
Triển khai trí tuệ nhân tạo Trùm **Ironclad Warlord** với 4 pha ra chiêu chuẩn mực (Telegraph decal, Flash Cue phản xạ, Hitbox, Recovery) và cơ chế choáng khi húc tường; Hệ thống Phá vỡ thế trận (**Posture Stagger**) với 3.0s quỳ gối, đòn kết liễu gây **25% True Damage** kèm 1.2s bất tử; Hệ thống Phá hủy bộ phận (**Part Breaking**) khóa vĩnh viễn chiêu thức tương ứng của trùm; và Hệ thống Né đòn hoàn hảo (**Dash I-Frame & Perfect Dodge**) với cửa sổ vàng 0.05–0.15s thưởng +15 thể lực và 0.08s hitstop toàn cục.

---

## Tasks

### Must Have (Critical Path — 16 hours / 2.0 days)

| ID | Task | Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `boss-001` | [Prototype Boss AI & 4-Phase Telegraph System](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/encounter-boss/story-001-boss-ai-telegraphs.md) | `gameplay-programmer` | 1.0 (8h) | `cmbt-001`, `hud-002` | AC-1: 4 pha ra chiêu (Telegraph, Flash Cue 0.10s, Hitbox, Recovery); AC-2: 5 kỹ năng với chấm điểm EQS theo góc/cự ly; AC-3: 3 giai đoạn máu trùm; AC-4: Choáng 1.8s khi húc tường. |
| `stgr-001` | [Posture Stagger, Kneel Window & True Damage Execution](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/encounter-boss/story-002-stagger-execution.md) | `gameplay-programmer` | 1.0 (8h) | `attr-002`, `hud-002` | AC-1: Trùm quỳ gối 3.0s tại 100% Posture; AC-2: Đòn kết liễu gây 25% True Damage + I-Frame 1.2s; AC-3: Suy giảm Posture 20/s sau 4s; AC-4: Hoàn lại 50% Posture nếu lỡ cơ hội. |

### Should Have (16 hours / 2.0 days)

| ID | Task | Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `stgr-002` | [Anatomical Part Breaking & Skill Disabling Matrix](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/encounter-boss/story-003-part-breaking-matrix.md) | `gameplay-programmer` | 1.0 (8h) | `stgr-001`, `boss-001` | AC-1: Theo dõi máu độc lập Sừng (20%), Đuôi (15%), Giáp ngực (25%); AC-2: Gãy sừng/đuôi khóa vĩnh viễn chiêu tương ứng; AC-3: Giáp ngực vỡ nhận +50% sát thương; AC-4: Báo tin rơi nguyên liệu rèn. |
| `dash-001` | [Dash I-Frame, Perfect Dodge Sweet Spot & Hitstop](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/encounter-boss/story-004-dash-iframe-perfect-dodge.md) | `gameplay-programmer` | 1.0 (8h) | `attr-003`, `hud-001` | AC-1: Lướt 0.45s, I-Frame 0.28s; AC-2: Cửa sổ Perfect Dodge 0.05-0.15s (+15 thể lực, 0.08s hitstop); AC-3: Chống rơi mép vực; AC-4: Hủy phục hồi sang Dash Attack từ 0.35s. |

---

## Total Planned Scope
- **Must Have**: 2 stories (16 hours / 2.0 days)
- **Should Have**: 2 stories (16 hours / 2.0 days)
- **Total**: 4 stories (32 hours / 4.0 days) vs **Available Capacity**: 8.0 days (64 hours)

---

## Definition of Done for this Sprint
- [ ] 100% Must Have và Should Have tasks hoàn thành và vượt qua kiểm thử đơn vị & tích hợp.
- [ ] Vòng lặp chiến đấu: Boss ra chiêu (Telegraph) -> Người chơi né chuẩn (Perfect Dodge) -> Đánh tích Posture -> Boss vỡ thế (Stagger 3.0s) -> Kết liễu 25% True Damage hoạt động chính xác.
- [ ] Bộ phận bị phá vỡ tác động trực tiếp và vĩnh viễn đến cây hành vi Behavior Tree của trùm.
- [ ] Bộ kiểm thử tự động Unreal Automation Tests đạt 100% Pass.
