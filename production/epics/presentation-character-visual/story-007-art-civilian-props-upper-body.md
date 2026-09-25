# Story 007: Civilian Upper Body & Town Props (255 Assets)

> **Epic**: `EPIC-CHARACTER-VISUAL-001` (Character & NPC Visual Identity System)  
> **Story ID**: `visual-007`  
> **Layer**: Presentation / Pixel Art Asset Production  
> **Type**: Art & Production (Handcrafted Pixel Sprites)  
> **Estimate**: 1.5 days (12 hours)  
> **Status**: Completed  
> **Owner**: Pixel Artist  
> **Governing Spec**: [`character-visual-system.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/gdd/character-visual-system.md)  

---

## 1. Bối Cảnh & Mục Tiêu Sản Xuất

Lấp đầy hệ thống NPC dân cư thị trấn trong các khu an toàn (Verdant Bastion, Ashen Keep, Sanctum Fortress) bằng tài nguyên đồ họa hoàn chỉnh:
- **Upper Body Dân Sự (205 frames)**:
  - Thợ rèn: Gõ búa (Idle 4f), vác búa (Walk 4f) $\times 5$ hướng = $40\text{ frames}$.
  - Thương nhân: Xoa tay đếm tiền (Idle 4f), ôm hòm hàng (Walk 4f) $\times 5$ hướng = $40\text{ frames}$.
  - Dân làng: Chắp tay sau lưng (Idle 4f), đi dạo (Walk 4f) $\times 5$ hướng = $40\text{ frames}$.
  - Lính gác: Tựa giáo (Idle 4f), bồng giáo tuần tra (Walk 4f) $\times 5$ hướng = $40\text{ frames}$.
  - Quest Giver: Đọc cuộn thư (Idle 4f), đi khoan thai (Walk 4f), Callout vẫy tay (1f) $\times 5$ hướng = $45\text{ frames}$.
- **10 Đạo Cụ Cầm Tay Tĩnh (50 sprites)**: $10\text{ props} \times 5\text{ hướng xoay}$.
  - Búa rèn, Kìm gắp phôi, Túi tiền vàng, Cân tiểu ly, Giỏ bánh mì, Chổi quét rơm, Giáo thành dài, Khiên huy hiệu, Cuộn da phong ấn sáp đỏ, Đèn bão.

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [x] **AC-1 (Đầy Đủ 205 Frames Upper Body Dân Sự)**:
  - Xuất xưởng vào `Content/Art/Characters/Civilian/UpperBody/`.
  - Khớp khít đường nối eo $Y = 80$ với các Master Rigs Lower Body có sẵn.
- [x] **AC-2 (10 Đạo Cụ Tĩnh Khớp Hand Sockets)**:
  - Xuất xưởng vào `Content/Art/Characters/Civilian/Props/`.
  - Snap chính xác vào `HandSocket_R` $(96, 76)$ hoặc `HandSocket_L` $(32, 76)$, không bị lệch tâm khi xoay 5 hướng.
- [x] **AC-3 (Quest Giver Callout Frame)**:
  - Frame vẫy tay thể hiện rõ tư thế khẩn thiết, đón ánh nhìn từ xa của người chơi.
