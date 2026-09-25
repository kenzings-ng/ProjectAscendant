# Story 005: 7 Weapon Family Upper Body Combat Sets (560 Frames)

> **Epic**: `EPIC-CHARACTER-VISUAL-001` (Character & NPC Visual Identity System)  
> **Story ID**: `visual-005`  
> **Layer**: Presentation / Combat Timing & Socket Alignment  
> **Type**: Art & Production (Wireframe / Stick-figure Proxy)  
> **Estimate**: 3.0 days (24 hours)  
> **Status**: Completed (Placeholder-tier art — cần thay thế bằng pixel art thật trước khi release)  
> **Owner**: Technical Artist  
> **Governing Spec**: [`SPEC-ART-2026-09-23-V2`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/art/pixel-asset-specifications.md), [`itemization.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/gdd/itemization.md)  

---

## 1. Bối Cảnh & Mục Tiêu Kỹ Thuật

Tạo lập các bộ khung hình wireframe/stick-figure proxy (do script Python hình học tạo ra) cho hoạt ảnh chiến đấu phần thân trên (Ngực, Cánh tay và Đầu) của nhân vật theo **7 Weapon Families** để phục vụ kiểm tra animation timing, quỹ đạo vung đòn và alignment các socket cầm nắm:

> [!NOTE]
> Đây là tài nguyên **wireframe/stick-figure proxy for animation timing and socket validation**, KHÔNG PHẢI pixel art hoàn chỉnh (không có 4-tone ramp shading hay giải phẫu chi tiết). Cần được thay thế hoàn toàn bằng pixel art vẽ tay thật sự trước khi release.

- `Family 1: 1H Blades` (`FB_Upper_1H_Blade_Combo`): Chém ngang góc $120^\circ$, tầm $220\text{ cm}$.
- `Family 2: 2H Heavy` (`FB_Upper_2H_Heavy_Combo`): Bổ dọc $180\text{ cm}$, quét nặng $160^\circ$ Hyper-Armor.
- `Family 3: Polearms` (`FB_Upper_2H_Polearm_Combo`): Đâm thẳng tầm xa $400\text{ cm}$, quét xoay $360^\circ$.
- `Family 4: Bows` (`FB_Upper_2H_Bow_Combo`): Kéo căng, ngắm bắn và xả tiễn.
- `Family 5: Twin Daggers` (`FB_Upper_Dual_Daggers_Combo`): Đâm chém liên hoàn cực tốc.
- `Family 6: Staves` (`FB_Upper_2H_Staff_Combo`): Giộng trượng, vung tay phóng ma pháp.
- `Family 7: Maces & Relics` (`FB_Upper_1H_MaceRelic_Combo`): Nện chùy phá thế đứng, giơ pháp bảo.
- **Quy chuẩn**: Mỗi bộ gồm Combo 3 đòn (12f) + Thế thủ/Phản đòn (4f) = $16\text{ frames} \times 5\text{ hướng} = \mathbf{80\text{ frames/Family}}$ (Tổng 7 bộ = **560 frames wireframe proxy**).

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [x] **AC-1 (Đầy Đủ 560 Frames Wireframe Proxy)**:
  - Xuất xưởng 7 bộ Flipbook proxy vào `Content/Art/Characters/UpperBodyWeapons/`.
- [x] **AC-2 (Căn Chỉnh Hand Sockets)**:
  - Khung xương tay wireframe giữ vị trí bàn tay trùng khớp với `HandSocket_R` $(96, 76)$ và `HandSocket_L` $(32, 76)$ trong tư thế chuẩn, di chuyển theo đúng quỹ đạo vung đòn để kiểm tra timing đánh và đón đòn.
- [x] **AC-3 (Kiểm Tra Quỹ Đạo Vũ Khí & Padding)**:
  - Kiểm tra việc gắn kết các Sprite Vũ Khí cơ sở lên tay, đảm bảo vũ khí xoay theo góc vát đòn đánh mà không bị lệch chuôi.
  - Vùng phía trước mặt chừa khoảng đệm chuyển động (Motion Padding) $32\text{ px}$ để đón vệt chém Niagara Slash Trail.
