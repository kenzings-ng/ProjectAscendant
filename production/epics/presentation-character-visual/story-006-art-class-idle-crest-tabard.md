# Story 006: 12 Class Idle Stances, Crest Sockets & Tabard Overlays (360 Assets)

> **Epic**: `EPIC-CHARACTER-VISUAL-001` (Character & NPC Visual Identity System)  
> **Story ID**: `visual-006`  
> **Layer**: Presentation / Pixel Art Asset Production  
> **Type**: Art & Production (Handcrafted Pixel Sprites)  
> **Estimate**: 2.0 days (16 hours)  
> **Status**: Ready for Dev  
> **Owner**: Pixel Artist & Art Director  
> **Governing Spec**: [`character-visual-system.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/gdd/character-visual-system.md)  

---

## 1. Bối Cảnh & Mục Tiêu Sản Xuất

Đảm bảo người chơi nhận diện ngay tức thì 12 Class nhân vật ngay cả khi đang mặc trang bị giáp sắt thô che kín cơ thể.
Story này sản xuất toàn bộ các chi tiết bản sắc độc quyền của 12 Class:
1. **12 Bộ Dáng Đứng Tĩnh (Idle Silhouette Loops)**: Mỗi class có 1 loop thở nhẹ 4 frames $\times 5$ hướng = $20\text{ frames/class}$ (Tổng **240 frames**).
2. **12 Mào Nón / Sừng Giáp (Helm Crest Sprites)**: Sprite tĩnh $32 \times 32\text{ px}$, snap đỉnh đầu $(64, 40)$, 5 hướng = **60 sprites**.
3. **12 Cờ Ngực / Khăn Choàng (Tabard / Sash Sprites)**: Sprite phủ $48 \times 64\text{ px}$, snap rãnh ngực $(64, 60)$, 5 hướng = **60 sprites**.

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [ ] **AC-1 (Đầy Đủ 12 Idle Loops Riêng Biệt)**:
  - Thể hiện rõ nét các tư thế độc nhất theo GDD:
    - Vanguard (khiên che 40%, kiếm chúc $45^\circ$), Ranger (cung vút cao 12px), Arcanist (trượng đứng $90^\circ$, đá bay lơ lửng), Acolyte (đồng hồ cát cân bằng), Berserker (lưng gù, đại đao vác vai 18px), Shadowblade (cúi sát đất, đao chéo X), Elementalist (lơ lửng 4px), Templar (tháp Gothic thẳng tắp), Void Blade (zigzag áo xé), Chronomancer (kim đồng hồ xoay), Dragon Knight (chiến kích vươn 24px), God Slayer (không nhịp thở, đại kiếm cắm đất).
- [ ] **AC-2 (12 Helm Crest Sprites)**:
  - Đầy đủ 12 biểu tượng: Bờm đỏ, Lông chim ưng, Mảnh lam ngọc, Vòng kim cô, Sừng thú, Băng bịt mắt, Vương miện 3 hạt, Cánh thép, Sừng hư không, Con lắc xoay, Sừng rồng, Vòng gai đen.
- [ ] **AC-3 (12 Tabard / Sash Sprites)**:
  - Khớp chính xác với rãnh khoét giáp ngực của 9 bộ giáp cơ sở từ Itemization.
- [ ] **AC-4 (Vượt Qua Silhouette QA Gate Check 100%)**:
  - Toàn bộ sprite vượt qua script `qa_silhouette_check.py`:
    - Biến thành Pure Black (#000000) trên nền trắng.
    - Phân biệt rõ ràng ở kích thước Thumbnail $32 \times 32$ và $16 \times 16$.
    - Gắn đè giáp Heavy T1 không làm mất silhouette nhận diện.
