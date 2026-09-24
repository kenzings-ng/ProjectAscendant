# Story 005: 7 Weapon Family Upper Body Combat Sets (560 Frames)

> **Epic**: `EPIC-CHARACTER-VISUAL-001` (Character & NPC Visual Identity System)  
> **Story ID**: `visual-005`  
> **Layer**: Presentation / Pixel Art Asset Production  
> **Type**: Art & Production (Handcrafted Pixel Sprites)  
> **Estimate**: 3.0 days (24 hours)  
> **Status**: Ready for Dev  
> **Owner**: Technical Artist & Pixel Artist  
> **Governing Spec**: [`SPEC-ART-2026-09-23-V2`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/art/pixel-asset-specifications.md), [`itemization.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/gdd/itemization.md)  

---

## 1. Bối Cảnh & Mục Tiêu Sản Xuất

Thay thế placeholder Upper Body trong `visual-001` bằng hoạt ảnh chiến đấu vẽ tay cho phần thân trên (Ngực, Cánh tay và Đầu) của nhân vật theo **7 Weapon Families**:
- `Family 1: 1H Blades` (`FB_Upper_1H_Blade_Combo`): Chém ngang góc $120^\circ$, tầm $220\text{ cm}$.
- `Family 2: 2H Heavy` (`FB_Upper_2H_Heavy_Combo`): Bổ dọc $180\text{ cm}$, quét nặng $160^\circ$ Hyper-Armor.
- `Family 3: Polearms` (`FB_Upper_2H_Polearm_Combo`): Đâm thẳng tầm xa $400\text{ cm}$, quét xoay $360^\circ$.
- `Family 4: Bows` (`FB_Upper_2H_Bow_Combo`): Kéo căng, ngắm bắn và xả tiễn.
- `Family 5: Twin Daggers` (`FB_Upper_Dual_Daggers_Combo`): Đâm chém liên hoàn cực tốc $2.2\text{ đòn/s}$.
- `Family 6: Staves` (`FB_Upper_2H_Staff_Combo`): Giộng trượng, vung tay phóng ma pháp.
- `Family 7: Maces & Relics` (`FB_Upper_1H_MaceRelic_Combo`): Nện chùy phá thế đứng, giơ pháp bảo.
- **Quy chuẩn**: Mỗi bộ gồm Combo 3 đòn (12f) + Thế thủ/Phản đòn (4f) = $16\text{ frames} \times 5\text{ hướng} = \mathbf{80\text{ frames/Family}}$ (Tổng 7 bộ = **560 frames**).

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [ ] **AC-1 (Đầy Đủ 560 Frames Vẽ Tay)**:
  - Xuất xưởng 7 bộ Flipbook vào `Content/Art/Characters/UpperBodyWeapons/`.
- [ ] **AC-2 (Khớp Khít Hand Sockets Tuyệt Đối)**:
  - Khung xương tay luôn giữ vị trí bàn tay trùng khít với `HandSocket_R` $(96, 76)$ và `HandSocket_L` $(32, 76)$ trong tư thế chuẩn, và di chuyển mượt mà theo quỹ đạo vung đòn.
- [ ] **AC-3 (Tương Thích Kho Vũ Khí Itemization)**:
  - Khi gắn 21 Base Weapon Sprites từ Itemization lên tay, vũ khí xoay theo đúng góc vát đòn đánh mà không bị lệch chuôi.
  - Vùng phía trước mặt luôn chừa khoảng đệm chuyển động (Motion Padding) $32\text{ px}$ để đón vệt chém Niagara Slash Trail.
