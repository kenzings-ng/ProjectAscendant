# Epic: EPIC-CHARACTER-VISUAL-001 — Character & NPC Visual Identity System

> **Epic ID**: `EPIC-CHARACTER-VISUAL-001`  
> **Target Sprint**: Sprint 7  
> **Status**: In Progress (Code Complete; Art in Placeholder-tier — cần thay thế bằng pixel art thật trước khi release)  
> **Stage**: Production (Visual Identity & Modular Animation Layer)  
> **Governing GDD**: [`design/gdd/character-visual-system.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/gdd/character-visual-system.md)  
> **Technical Contract**: [`SPEC-ART-2026-09-23-V2`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/art/pixel-asset-specifications.md)  
> **Underlying Architecture**: [`UPAPaperdollComponent`](file:///mnt/Data/Projects/project-games/ProjectAscendant/Source/ProjectAscendant/Public/Character/PAPaperdollComponent.h) (Story `item-004`), PaperZD 2.5D State Machine, Lumen HD-2D

---

## 1. Mục Tiêu Cốt Lõi (Epic Goal)

Thiết lập toàn bộ khung kiến trúc bản sắc thị giác cho 12 Class nhân vật, 4 bậc quái vật thế giới và hệ thống NPC dân cư thị trấn trong Project Ascendant:
- **Nguyên tắc "100% Placeholder Trước, Ráp Art Thật Sau"**: Tách rời hoàn toàn các story lập trình (Code/Engineering) khỏi các story sản xuất mỹ thuật (Art/Asset). Toàn bộ logic lắp ghép rig, chuyển động phân tầng, socket mào mũ, rãnh cờ giáp và đổi bảng màu Palette Swap được hoàn thiện và kiểm thử tự động 100% bằng dummy placeholder textures trước khi nhập sprite hoàn thiện.
- **Giải bài toán 8.400 frame**: Triển khai 4 Master Animation Rigs kết hợp phân tách Upper Body (7 Weapon Families) và Lower Body (Locomotion), cắt giảm 86.4% khối lượng vẽ cho toàn dự án.
- **Bảo toàn nhận diện không phụ thuộc màu giáp**: Tích hợp các điểm neo silhouette (Custom Idle Stance 4f, Helm Crest Socket, Tabard Overlay) vào hệ thống Paperdoll 9-Slot.

> [!IMPORTANT]
> **Xác Nhận Hiện Trạng Tài Nguyên Mỹ Thuật (Art Asset Status Confirmation)**:
> Toàn bộ 1,595 assets tạo ra trong các story `visual-004`, `visual-005`, `visual-006`, và `visual-007` hiện nay là **wireframe/stick-figure proxy** được sinh tự động bằng script Python hình học. Chúng phục vụ mục đích kỹ thuật là kiểm tra animation timing, chu kỳ di chuyển (locomotion) và khớp nối socket, **KHÔNG PHẢI** pixel art thành phẩm hoàn chỉnh (chưa có 4-tone ramp shading thủ công, giải phẫu pixel chi tiết hay chất lượng hoàn thiện Anti-AI). Toàn bộ nhóm này cần được thay thế bằng pixel art vẽ tay thật sự trước khi phát hành chính thức.

---

## 2. Danh Mục Stories Thuộc Epic (Phân Tách Code vs Art Proxy)

```
EPIC-CHARACTER-VISUAL-001 (Sprint 7)
├── NHÓM 1: CODE & ARCHITECTURE (100% Complete & Testable)
│   ├── visual-001: Master Rig Architecture & Decoupled State Machine Binding (Complete)
│   ├── visual-002: Helm Crest Socket & Tabard Cutout Component Integration (Complete)
│   └── visual-003: Civilian NPC Component, Dynamic Palette Swap & Town Guard AI (Complete)
│
└── NHÓM 2: ART & ASSET PRODUCTION (Wireframe / Stick-figure Proxies)
    ├── visual-004: 4 Master Lower Body Animation Sets (420 frames wireframe proxy)
    ├── visual-005: 7 Weapon Family Upper Body Combat Sets (560 frames wireframe proxy)
    ├── visual-006: 12 Class Idle Stances, Crest Sockets & Tabard Overlays (360 assets wireframe proxy)
    └── visual-007: Civilian Upper Body & Town Props (255 assets wireframe proxy)
```

---

## 3. Bảng Chi Tiết Stories

| Story ID | Tên Story | Phân Loại | Trạng Thái | Chủ Trì (Owner) | Ước Lượng | Ràng Buộc / Dependencies | Acceptance Criteria Cốt Lõi |
| :--- | :--- | :---: | :---: | :---: | :---: | :--- | :--- |
| **`visual-001`** | [Master Rig Architecture & Decoupled State Machine Binding](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-001-master-rig-decoupled-state-machine.md) | **Code** | **Complete** | `gameplay-programmer` | 1.5 ngày | `item-004` (Paperdoll 9-Slot), PaperZD plugin | AC-1: Quản lý 4 Master Rigs trong C++; AC-2: Decoupled Upper/Lower Body state machine; AC-3: Hoán đổi 7 Weapon Families độc lập; AC-4: Unit test headless 100% pass với dummy flipbooks. |
| **`visual-002`** | [Helm Crest Socket & Tabard Cutout Component Integration](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-002-helm-crest-tabard-overlay-system.md) | **Code** | **Complete** | `gameplay-programmer` | 1.0 ngày | `item-004`, `visual-001` | AC-1: Khởi tạo Socket `Socket_HelmCrest` $(64, 40)$ và `Socket_Tabard` $(64, 60)$; AC-2: Cơ chế overlay đè lên 9 bộ giáp cơ sở; AC-3: Directional Sort Key bảo toàn độ sâu khi quay Tây; AC-4: Unit test headless. |
| **`visual-003`** | [Civilian NPC Component, Dynamic Palette Swap & Town Guard AI](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-003-civilian-npc-palette-guard-ai.md) | **Code** | **Complete** | `systems-designer` | 1.0 ngày | `item-003` (Blacksmith), `zone-001` (Safe Zones) | AC-1: `UPACivilianNPCComponent` quản lý 5 vai trò; AC-2: Master Material `M_PaperZD_Civilian_Base` Palette Swap theo Zone; AC-3: Socket 1-frame Prop gắn tay; AC-4: Lính gác chuyển combat đâm giáo khi gặp Outlaw. |
| **`visual-004`** | [4 Master Lower Body Animation Sets (420 Frames)](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-004-art-lower-body-master-rigs.md) | **Art Proxy** | **Completed (Placeholder-tier art — cần thay thế bằng pixel art thật trước khi release)** | `technical-artist` | 2.5 ngày | `visual-001`, `SPEC-ART-2026-09-23-V2` | AC-1: 4 Rigs x 5 anims x 5 hướng wireframe/stick-figure proxy; AC-2: Kiểm tra animation timing và chu kỳ bước; AC-3: Pivot tiếp đất `(64, 114)` và khớp eo $Y=80$. |
| **`visual-005`** | [7 Weapon Family Upper Body Combat Sets (560 Frames)](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-005-art-upper-body-weapon-families.md) | **Art Proxy** | **Completed (Placeholder-tier art — cần thay thế bằng pixel art thật trước khi release)** | `technical-artist` | 3.0 ngày | `visual-001`, `item-005` (Weapon Arsenal) | AC-1: 7 Families x 16f x 5 hướng combat wireframe/stick-figure proxy; AC-2: Căn chỉnh vị trí bàn tay với `HandSocket_R` và `HandSocket_L`; AC-3: Kiểm tra góc vung đòn vũ khí. |
| **`visual-006`** | [12 Class Idle Stances, Crest Sockets & Tabard Overlays (360 Assets)](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-006-art-class-idle-crest-tabard.md) | **Art Proxy** | **Completed (Placeholder-tier art — cần thay thế bằng pixel art thật trước khi release)** | `pixel-artist` | 2.0 ngày | `visual-002`, `character-visual-system.md` | AC-1: 12 Idle loops wireframe proxy (4f x 5 hướng); AC-2: 12 Helm Crest sprites ($32 \times 32$); AC-3: 12 Tabard/Sash sprites ($48 \times 64$); AC-4: Vượt qua bài kiểm tra Silhouette QA Gate Check. |
| **`visual-007`** | [Civilian Upper Body & Town Props (255 Assets)](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-007-art-civilian-props-upper-body.md) | **Art Proxy** | **Completed (Placeholder-tier art — cần thay thế bằng pixel art thật trước khi release)** | `pixel-artist` | 1.5 ngày | `visual-003`, `SPEC-ART-2026-09-23-V2` | AC-1: Upper Body 5 vai trò NPC wireframe proxy; AC-2: 10 đạo cụ cầm tay tĩnh (5 hướng) kiểm tra hand socket; AC-3: 1 frame Callout Quest Giver. |

---

## 4. Định Nghĩa Hoàn Thành Toàn Epic (Definition of Done)
1. **Pha 1 (Code Complete)**: 3 Code Stories (`visual-001`, `visual-002`, `visual-003`) hoàn thành 100%, test suite `ProjectAscendant.CharacterVisual` pass xanh trên Linux headless editor.
2. **Pha 2 (Technical Validation via Wireframe Proxies Complete)**: 4 Art Stories (`visual-004` đến `visual-007`) đã tạo và nạp thành công 1,595 assets wireframe/stick-figure proxy để hoàn tất kiểm tra animation timing, PaperZD state machine transitions và socket alignments.
3. **Pha 3 (Art Production & Final Replacement - Required Before Release)**: Thay thế toàn bộ 1,595 assets wireframe proxy bằng các sprite pixel art vẽ tay hoàn chỉnh (chuẩn 4-tone ramp shading, Anti-AI) trước khi đóng bản phát hành chính thức (Release).
