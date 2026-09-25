# Epic: EPIC-CHARACTER-VISUAL-001 — Character & NPC Visual Identity System

> **Epic ID**: `EPIC-CHARACTER-VISUAL-001`  
> **Target Sprint**: Sprint 7  
> **Status**: Completed  
> **Stage**: Production (Visual Identity & Modular Animation Layer)  
> **Governing GDD**: [`design/gdd/character-visual-system.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/gdd/character-visual-system.md)  
> **Technical Contract**: [`SPEC-ART-2026-09-23-V2`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/art/pixel-asset-specifications.md)  
> **Underlying Architecture**: [`UPAPaperdollComponent`](file:///mnt/Data/Projects/project-games/ProjectAscendant/Source/ProjectAscendant/Public/Character/PAPaperdollComponent.h) (Story `item-004`), PaperZD 2.5D State Machine, Lumen HD-2D

---

## 1. Mục Tiêu Cốt Lõi (Epic Goal)

Thiết lập toàn bộ khung kiến trúc bản sắc thị giác cho 12 Class nhân vật, 4 bậc quái vật thế giới và hệ thống NPC dân cư thị trấn trong Project Ascendant:
- **Nguyên tắc "100% Placeholder Trước, Ráp Art Thật Sau"**: Tách rời hoàn toàn các story lập trình (Code/Engineering) khỏi các story sản xuất mỹ thuật (Art/Asset). Toàn bộ logic lắp ghép rig, chuyển động phân tầng, socket mào mũ, rãnh cờ giáp và đổi bảng màu Palette Swap được hoàn thiện và kiểm thử tự động 100% bằng dummy placeholder textures trước khi nhập sprite vẽ tay.
- **Giải bài toán 8.400 frame**: Triển khai 4 Master Animation Rigs kết hợp phân tách Upper Body (7 Weapon Families) và Lower Body (Locomotion), cắt giảm 86.4% khối lượng vẽ tay cho toàn dự án.
- **Bảo toàn nhận diện không phụ thuộc màu giáp**: Tích hợp các điểm neo silhouette (Custom Idle Stance 4f, Helm Crest Socket, Tabard Overlay) vào hệ thống Paperdoll 9-Slot.

---

## 2. Danh Mục Stories Thuộc Epic (Phân Tách Code vs Art)

```
EPIC-CHARACTER-VISUAL-001 (Sprint 7)
├── NHÓM 1: CODE & ARCHITECTURE (100% Placeholder Testable)
│   ├── visual-001: Master Rig Architecture & Decoupled State Machine Binding
│   ├── visual-002: Helm Crest Socket & Tabard Cutout Component Integration
│   └── visual-003: Civilian NPC Component, Dynamic Palette Swap & Town Guard AI
│
└── NHÓM 2: ART & ASSET PRODUCTION (Handcrafted Sprites)
    ├── visual-004: 4 Master Lower Body Animation Sets (420 frames)
    ├── visual-005: 7 Weapon Family Upper Body Combat Sets (560 frames)
    ├── visual-006: 12 Class Idle Stances, Crest Sockets & Tabard Overlays (360 assets)
    └── visual-007: Civilian Upper Body & Town Props (255 assets)
```

---

## 3. Bảng Chi Tiết Stories

| Story ID | Tên Story | Phân Loại | Chủ Trì (Owner) | Ước Lượng | Ràng Buộc / Dependencies | Acceptance Criteria Cốt Lõi |
| :--- | :--- | :---: | :---: | :---: | :--- | :--- |
| **`visual-001`** | [Master Rig Architecture & Decoupled State Machine Binding](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-001-master-rig-decoupled-state-machine.md) | **Code** | `gameplay-programmer` | 1.5 ngày | `item-004` (Paperdoll 9-Slot), PaperZD plugin | AC-1: Quản lý 4 Master Rigs trong C++; AC-2: Decoupled Upper/Lower Body state machine; AC-3: Hoán đổi 7 Weapon Families độc lập; AC-4: Unit test headless 100% pass với dummy flipbooks. |
| **`visual-002`** | [Helm Crest Socket & Tabard Cutout Component Integration](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-002-helm-crest-tabard-overlay-system.md) | **Code** | `gameplay-programmer` | 1.0 ngày | `item-004`, `visual-001` | AC-1: Khởi tạo Socket `Socket_HelmCrest` $(64, 40)$ và `Socket_Tabard` $(64, 60)$; AC-2: Cơ chế overlay đè lên 9 bộ giáp cơ sở; AC-3: Directional Sort Key bảo toàn độ sâu khi quay Tây; AC-4: Unit test headless. |
| **`visual-003`** | [Civilian NPC Component, Dynamic Palette Swap & Town Guard AI](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-003-civilian-npc-palette-guard-ai.md) | **Code** | `systems-designer` | 1.0 ngày | `item-003` (Blacksmith), `zone-001` (Safe Zones) | AC-1: `UPACivilianNPCComponent` quản lý 5 vai trò; AC-2: Master Material `M_PaperZD_Civilian_Base` Palette Swap theo Zone; AC-3: Socket 1-frame Prop gắn tay; AC-4: Lính gác chuyển combat đâm giáo khi gặp Outlaw. |
| **`visual-004`** | [4 Master Lower Body Animation Sets (420 Frames)](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-004-art-lower-body-master-rigs.md) | **Art** | `technical-artist` | 2.5 ngày | `visual-001`, `SPEC-ART-2026-09-23-V2` | AC-1: 4 Rigs (Heavy/Agility/Caster/Monk) x 5 animations (Idle/Walk/Run/Dash/HitStun) x 5 hướng; AC-2: Đạt chuẩn 4-tone ramp, không Mixels; AC-3: Pivot `(64, 114)`. |
| **`visual-005`** | [7 Weapon Family Upper Body Combat Sets (560 Frames)](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-005-art-upper-body-weapon-families.md) | **Art** | `technical-artist` | 3.0 ngày | `visual-001`, `item-005` (Weapon Arsenal) | AC-1: 7 Weapon Families x Combo 3 đòn + Parry/Charge x 5 hướng; AC-2: Bàn tay khớp khít `HandSocket_R` và `HandSocket_L`; AC-3: Đồng bộ với 21 base weapon sprites. |
| **`visual-006`** | [12 Class Idle Stances, Crest Sockets & Tabard Overlays (360 Assets)](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-006-art-class-idle-crest-tabard.md) | **Art** | `pixel-artist` | 2.0 ngày | `visual-002`, `character-visual-system.md` | AC-1: 12 Idle loops (4f x 5 hướng); AC-2: 12 Helm Crest sprites ($32 \times 32$); AC-3: 12 Tabard/Sash sprites ($48 \times 64$); AC-4: Vượt qua bài kiểm tra Silhouette QA Gate Check. |
| **`visual-007`** | [Civilian Upper Body & Town Props (255 Assets)](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/epics/presentation-character-visual/story-007-art-civilian-props-upper-body.md) | **Art** | `pixel-artist` | 1.5 ngày | `visual-003`, `SPEC-ART-2026-09-23-V2` | AC-1: Upper Body 5 vai trò NPC (Idle + Walk x 5 hướng); AC-2: 1 frame Callout Quest Giver; AC-3: 10 props cầm tay tĩnh (5 hướng). |

---

## 4. Định Nghĩa Hoàn Thành Toàn Epic (Definition of Done)
1. **Pha 1 (Code Complete)**: 3 Code Stories (`visual-001`, `visual-002`, `visual-003`) hoàn thành trước với 100% placeholder textures, test suite `ProjectAscendant.CharacterVisual` pass xanh trên Linux headless editor.
2. **Pha 2 (Art Complete)**: 4 Art Stories (`visual-004` đến `visual-007`) nghiệm thu qua script kiểm thử tự động `qa_silhouette_check.py` (Pure Black Convert, Thumbnail 16x16/32x32, và Overlay Clash Test).
3. **Pha 3 (Integration Complete)**: Thay thế toàn bộ dummy assets bằng art thật, kiểm tra trong scene thực tế đạt chuẩn 60 FPS, không lỗi lệch tay vũ khí hay tearing đường nối thân.
