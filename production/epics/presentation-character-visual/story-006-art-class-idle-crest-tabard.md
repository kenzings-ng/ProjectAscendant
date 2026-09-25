# Story 006: 12 Class Idle Stances, Crest Sockets & Tabard Overlays (360 Assets)

> **Epic**: `EPIC-CHARACTER-VISUAL-001` (Character & NPC Visual Identity System)  
> **Story ID**: `visual-006`  
> **Layer**: Presentation / Silhouette & Socket Alignment  
> **Type**: Art & Production (Wireframe / Stick-figure Proxy)  
> **Estimate**: 2.0 days (16 hours)  
> **Status**: Completed (Placeholder-tier art — cần thay thế bằng pixel art thật trước khi release)  
> **Owner**: Technical Artist & Pixel Artist  
> **Governing Spec**: [`character-visual-system.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/gdd/character-visual-system.md)  

---

## 1. Bối Cảnh & Mục Tiêu Kỹ Thuật

Tạo lập các tài nguyên wireframe proxy và placeholder geometric sprites (do script Python tạo ra) cho 12 Class nhân vật để xác thực nhận diện hình bóng (silhouette) và căn chỉnh socket trước khi vẽ pixel art thật:

> [!NOTE]
> Đây là tài nguyên **wireframe/stick-figure proxy for animation timing and socket validation**, KHÔNG PHẢI pixel art hoàn chỉnh (không có 4-tone ramp shading thủ công hay chi tiết giải phẫu hoàn thiện). Cần được thay thế hoàn toàn bằng pixel art vẽ tay thật sự trước khi release.

1. **12 Bộ Dáng Đứng Tĩnh Wireframe (Idle Silhouette Loops)**: Mỗi class có 1 loop thở nhẹ 4 frames $\times 5$ hướng = $20\text{ frames/class}$ (Tổng **240 frames proxy**).
2. **12 Mào Nón / Sừng Giáp Geometric Proxy (Helm Crest Sprites)**: Sprite $32 \times 32\text{ px}$, snap đỉnh đầu $(64, 40)$, 5 hướng = **60 sprites**.
3. **12 Cờ Ngực / Khăn Choàng Geometric Proxy (Tabard / Sash Sprites)**: Sprite $48 \times 64\text{ px}$, snap rãnh ngực $(64, 60)$, 5 hướng = **60 sprites**.

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [x] **AC-1 (Đầy Đủ 240 Frames Idle Wireframe Proxy)**:
  - Khung xương hình học phân định 12 tư thế cơ bản theo thiết kế silhouette (Vanguard khiên kiếm, Ranger giương cung, Arcanist cầm trượng, Berserker vác đại đao, Shadowblade đao chéo X, v.v.).
- [x] **AC-2 (12 Helm Crest Proxy Sprites)**:
  - 12 hình khối mào nón cơ sở ($32 \times 32$) gắn khớp vào `Socket_HelmCrest` $(64, 40)$.
- [x] **AC-3 (12 Tabard / Sash Proxy Sprites)**:
  - 12 hình khối cờ ngực ($48 \times 64$) gắn khớp vào rãnh ngực của 9 bộ giáp cơ sở từ Itemization.
- [x] **AC-4 (Kiểm Tra Silhouette Check)**:
  - Bộ sprite proxy vượt qua script `qa_silhouette_check.py` để đảm bảo độ tương phản cơ bản và kích thước nhận diện tối thiểu ở mức thumbnail.
