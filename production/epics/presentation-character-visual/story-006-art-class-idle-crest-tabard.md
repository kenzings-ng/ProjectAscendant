# Story 006: 16 Class Idle Stances, Crest Sockets & Tabard Overlays (500 Assets)

> **Epic**: `EPIC-CHARACTER-VISUAL-001` (Character & NPC Visual Identity System)  
> **Story ID**: `visual-006`  
> **Layer**: Presentation / Silhouette & Socket Alignment  
> **Type**: Art & Production (Wireframe / Stick-figure Proxy)  
> **Estimate**: 2.5 days (20 hours)  
> **Status**: Completed (Placeholder-tier art — cần thay thế bằng pixel art thật trước khi release)  
> **Owner**: Technical Artist & Pixel Artist  
> **Governing Spec**: [`character-visual-system.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/gdd/character-visual-system.md), [`DECISIONS.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/DECISIONS.md)  

---

## 1. Bối Cảnh & Mục Tiêu Kỹ Thuật

Tạo lập các tài nguyên wireframe proxy và placeholder geometric sprites (do script Python tạo ra) cho 16 Class nhân vật để xác thực nhận diện hình bóng (silhouette) và căn chỉnh socket trước khi vẽ pixel art thật:

> [!NOTE]
> Đây là tài nguyên **wireframe/stick-figure proxy for animation timing and socket validation**, KHÔNG PHẢI pixel art hoàn chỉnh (không có 4-tone ramp shading thủ công hay chi tiết giải phẫu hoàn thiện). Cần được thay thế hoàn toàn bằng pixel art vẽ tay thật sự trước khi release.

1. **16 Bộ Dáng Đứng Tĩnh Wireframe (Idle Silhouette Loops)**: Mỗi class có 1 loop thở nhẹ 4 frames $\times 5$ hướng = $20\text{ frames/class}$ (Tổng **320 frames proxy**).
2. **16 Mào Nón / Sừng Giáp Geometric Proxy (Helm Crest Sprites)**: Sprite $32 \times 32\text{ px}$, snap đỉnh đầu $(64, 40)$, 5 hướng = **80 sprites**.
3. **16 Cờ Ngực / Khăn Choàng Geometric Proxy (Tabard / Sash Sprites)**: Sprite $48 \times 64\text{ px}$, snap rãnh ngực $(64, 60)$, 5 hướng = **80 sprites**.
4. **Bộ Cánh Lục Dực Seraph (Seraph Wings Overlay Sprites)**: 4 frames $\times 5$ hướng = **20 sprites** gắn `Socket_Back`.
*(Tổng cộng: 320 frames idle + 80 crests + 80 tabards + 20 cánh Seraph = **500 assets**)*.

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [x] **AC-1 (Đầy Đủ 320 Frames Idle Wireframe Proxy)**:
  - Khung xương hình học phân định 16 tư thế cơ bản theo thiết kế silhouette (Vanguard khiên kiếm, Ranger giương cung, Arcanist cầm trượng, Berserker vác đại đao, Swordmaster buông kiếm, Shadowblade đao chéo X, v.v.).
- [x] **AC-2 (16 Helm Crest Proxy Sprites - 80 Sprites)**:
  - 16 hình khối mào nón cơ sở ($32 \times 32$) $\times 5$ hướng gắn khớp vào `Socket_HelmCrest` $(64, 40)$.
- [x] **AC-3 (16 Tabard / Sash Proxy Sprites - 80 Sprites)**:
  - 16 hình khối cờ ngực ($48 \times 64$) $\times 5$ hướng gắn khớp vào rãnh ngực của 9 bộ giáp cơ sở từ Itemization.
- [x] **AC-4 (Bộ Cánh Lục Dực Seraph - 20 Sprites)**:
  - 20 sprites cánh Lục Dực (4 frames idle $\times 5$ hướng) gắn khớp vào `Socket_Back`.
- [x] **AC-5 (Kiểm Tra Silhouette Check)**:
  - Toàn bộ sprite proxy vượt qua script `qa_silhouette_check.py` để đảm bảo độ tương phản cơ bản và kích thước nhận diện tối thiểu ở mức thumbnail.
