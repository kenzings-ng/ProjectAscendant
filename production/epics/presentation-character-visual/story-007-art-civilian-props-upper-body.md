# Story 007: Civilian Upper Body & Town Props (255 Assets)

> **Epic**: `EPIC-CHARACTER-VISUAL-001` (Character & NPC Visual Identity System)  
> **Story ID**: `visual-007`  
> **Layer**: Presentation / NPC Alignment & Prop Socket Validation  
> **Type**: Art & Production (Wireframe / Stick-figure Proxy)  
> **Estimate**: 1.5 days (12 hours)  
> **Status**: Completed (Placeholder-tier art — cần thay thế bằng pixel art thật trước khi release)  
> **Owner**: Technical Artist & Pixel Artist  
> **Governing Spec**: [`character-visual-system.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/gdd/character-visual-system.md)  

---

## 1. Bối Cảnh & Mục Tiêu Kỹ Thuật

Tạo lập các tài nguyên wireframe proxy và placeholder geometric sprites (do script Python tạo ra) cho NPC dân cư thị trấn để phục vụ kiểm tra kỹ thuật khớp nối Upper Body, chu kỳ idle/walk và socket đạo cụ cầm tay (HandSocket_R/L):

> [!NOTE]
> Đây là tài nguyên **wireframe/stick-figure proxy for animation timing and socket validation**, KHÔNG PHẢI pixel art hoàn chỉnh (không có 4-tone ramp shading thủ công hay chi tiết giải phẫu hoàn thiện). Cần được thay thế hoàn toàn bằng pixel art vẽ tay thật sự trước khi release.

- **Upper Body Dân Sự Proxy (205 frames)**:
  - Thợ rèn: Gõ búa (Idle 4f), vác búa (Walk 4f) $\times 5$ hướng = $40\text{ frames}$.
  - Thương nhân: Xoa tay đếm tiền (Idle 4f), ôm hòm hàng (Walk 4f) $\times 5$ hướng = $40\text{ frames}$.
  - Dân làng: Chắp tay sau lưng (Idle 4f), đi dạo (Walk 4f) $\times 5$ hướng = $40\text{ frames}$.
  - Lính gác: Tựa giáo (Idle 4f), bồng giáo tuần tra (Walk 4f) $\times 5$ hướng = $40\text{ frames}$.
  - Quest Giver: Đọc cuộn thư (Idle 4f), đi khoan thai (Walk 4f), Callout vẫy tay (1f) $\times 5$ hướng = $45\text{ frames}$.
- **10 Đạo Cụ Cầm Tay Geometric Proxy (50 sprites)**: $10\text{ props} \times 5\text{ hướng xoay}$.
  - Búa rèn, Kìm gắp phôi, Túi tiền vàng, Cân tiểu ly, Giỏ bánh mì, Chổi quét rơm, Giáo thành dài, Khiên huy hiệu, Cuộn da phong ấn sáp đỏ, Đèn bão.

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [x] **AC-1 (Đầy Đủ 205 Frames Upper Body Dân Sự Proxy)**:
  - Xuất xưởng vào `Content/Art/Characters/Civilian/UpperBody/`.
  - Khớp khít đường nối eo $Y = 80$ với các Master Rigs Lower Body.
- [x] **AC-2 (10 Đạo Cụ Hình Học Khớp Hand Sockets)**:
  - Xuất xưởng vào `Content/Art/Characters/Civilian/Props/`.
  - Snap chính xác vào `HandSocket_R` $(96, 76)$ hoặc `HandSocket_L` $(32, 76)$ trên 5 hướng.
- [x] **AC-3 (Quest Giver Callout Frame Proxy)**:
  - Frame vẫy tay hình học để kiểm tra kích hoạt logic tương tác NPC từ xa.
