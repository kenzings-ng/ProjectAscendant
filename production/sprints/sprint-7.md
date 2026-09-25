# Sprint 7 — 2026-11-01 to 2026-11-14

> **Sprint**: Sprint 7 (Character & NPC Visual Identity System)  
> **Stage**: Production (Visual Identity & Modular Animation Layer)  
> **Review Mode**: Lean  
> **Capacity**: 10 days (80 hours) | Buffer (20%): 2 days (16 hours) | Available: 8 days (64 hours)  

---

## Sprint Goal

Thiết lập hệ thống bản sắc thị giác nhân vật và NPC cho *Project Ascendant*:
Triển khai kiến trúc C++ Master Rig và máy trạng thái PaperZD phân tầng độc lập (Upper / Lower Body), hệ thống socket mào nón (Helm Crest) và cờ ngực (Tabard Cutout) trên Paperdoll 9-slot, NPC dân cư thị trấn với Dynamic Palette Swap và AI Lính gác trừng phạt tội phạm.
Đồng thời tạo lập toàn bộ khung xương wireframe/stick-figure proxy cho 4 bộ Lower Body Master Rigs, 7 Weapon Families Upper Body, 12 Class Idles/Crests/Tabards, và Civilian Upper Body/Props phục vụ kiểm thử nhịp hoạt ảnh (animation timing), alignment socket và state machine transitions.

> [!WARNING]
> Toàn bộ 1,595 assets hoạt ảnh và sprite sinh ra từ các story mỹ thuật `visual-004` đến `visual-007` hiện tại là **wireframe/stick-figure proxy** (sinh tự động qua script hình học Python phục vụ timing và alignment socket). Chúng là placeholder-tier và **cần được thay thế hoàn toàn bằng pixel art vẽ tay thật sự trước khi release**.

---

## Tasks

### Must Have (Code & Architecture — 28 hours / 3.5 days)

| ID | Task | Owner | Est. Days | Status | Acceptance Criteria |
|---|---|---|---|---|---|
| `visual-001` | [Master Rig Architecture & Decoupled State Machine Binding](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-001-master-rig-decoupled-state-machine.md) | `gameplay-programmer` | 1.5 (12h) | **Complete** | AC-1: Quản lý 4 Master Rigs; AC-2: Decoupled Upper/Lower State Machine; AC-3: Hoán đổi 7 Weapon Families; AC-4: Unit test headless pass. |
| `visual-002` | [Helm Crest Socket & Tabard Cutout Component Integration](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-002-helm-crest-tabard-overlay-system.md) | `gameplay-programmer` | 1.0 (8h) | **Complete** | AC-1: Socket `Socket_HelmCrest` (64, 40) và `Socket_Tabard` (64, 60); AC-2: Overlay trên 9 bộ giáp; AC-3: Sort Priority; AC-4: Unit test headless pass. |
| `visual-003` | [Civilian NPC Component, Dynamic Palette Swap & Town Guard AI](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-003-civilian-npc-palette-guard-ai.md) | `systems-designer` | 1.0 (8h) | **Complete** | AC-1: UPACivilianNPCComponent quản lý 5 vai trò; AC-2: Material Palette Swap theo Zone; AC-3: Hand Socket Props; AC-4: AI Guard tấn công Outlaw; AC-5: Unit test headless pass. |

### Should Have (Animation Timing & Socket Validation Proxies — 72 hours / 9.0 days)

| ID | Task | Owner | Est. Days | Status | Acceptance Criteria |
|---|---|---|---|---|---|
| `visual-004` | [4 Master Lower Body Animation Sets (420 Frames)](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-004-art-lower-body-master-rigs.md) | `technical-artist` | 2.5 (20h) | **Completed (Placeholder-tier art)** | AC-1: 4 Rigs x 5 anims x 5 hướng wireframe proxy; AC-2: Timing & locomotion validation; AC-3: Foot pivot (64, 114) & waist seam (Y=80). |
| `visual-005` | [7 Weapon Family Upper Body Combat Sets (560 Frames)](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-005-art-upper-body-weapon-families.md) | `technical-artist` | 3.0 (24h) | **Completed (Placeholder-tier art)** | AC-1: 7 Families x 16f x 5 hướng combat wireframe proxy; AC-2: Bàn tay khớp HandSocket_R/L; AC-3: Căn chỉnh quỹ đạo vung đòn vũ khí. |
| `visual-006` | [12 Class Idle Stances, Crest Sockets & Tabards (360 Assets)](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-006-art-class-idle-crest-tabard.md) | `pixel-artist` | 2.0 (16h) | **Completed (Placeholder-tier art)** | AC-1: 12 Class idles wireframe proxy; AC-2: 12 Crest socket sprites; AC-3: 12 Tabard cutout sprites; AC-4: Silhouette check pass. |
| `visual-007` | [Civilian Upper Body & Town Props (255 Assets)](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-007-art-civilian-props-upper-body.md) | `pixel-artist` | 1.5 (12h) | **Completed (Placeholder-tier art)** | AC-1: Upper Body 5 vai trò NPC wireframe proxy; AC-2: Hand socket props alignment; AC-3: Callout frame. |

---

## Total Planned Scope
- **Must Have**: 3 stories (28 hours / 3.5 days) — **100% Code Complete**
- **Should Have**: 4 stories (72 hours / 9.0 days) — **Completed as Placeholder-tier Wireframe Proxies**

---

## Definition of Done for this Sprint
- [x] Kiến trúc C++ Master Rig, Decoupled State Machine, Sockets và AI NPC hoàn thành 100%, bộ test headless pass xanh.
- [x] 1,595 assets wireframe proxy được import và cấu hình hoàn chỉnh vào Paper2D/PaperZD để xác thực animation timing, locomotion và socket alignment.
- [ ] **Release Gate Requirement**: Thay thế toàn bộ wireframe/stick-figure proxies của visual-004..007 bằng pixel art thủ công hoàn thiện (Handcrafted 4-tone ramp shading, Anti-AI) trước khi đóng bản phát hành chính thức (Release).
