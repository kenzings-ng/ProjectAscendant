# Story 004: 4 Master Lower Body Animation Sets (420 Frames)

> **Epic**: `EPIC-CHARACTER-VISUAL-001` (Character & NPC Visual Identity System)  
> **Story ID**: `visual-004`  
> **Layer**: Presentation / Animation Timing & Socket Validation  
> **Type**: Art & Production (Wireframe / Stick-figure Proxy)  
> **Estimate**: 2.5 days (20 hours)  
> **Status**: Completed (Placeholder-tier art — cần thay thế bằng pixel art thật trước khi release)  
> **Owner**: Technical Artist  
> **Governing Spec**: [`SPEC-ART-2026-09-23-V2`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/art/pixel-asset-specifications.md)  

---

## 1. Bối Cảnh & Mục Tiêu Kỹ Thuật

Thay thế các dummy placeholder flipbooks màu khối trong `visual-001` bằng các bộ khung hình wireframe/stick-figure proxy (do script Python vẽ hình học đơn giản) cho phần thân dưới (Chân, Bàn chân và Đai thắt lưng) của nhân vật để phục vụ kiểm tra kỹ thuật (animation timing, nhịp bước, kiểm tra chu kỳ chạy/nhảy và khớp nối PaperZD).

> [!NOTE]
> Đây là tài nguyên **wireframe/stick-figure proxy for animation timing and socket validation**, KHÔNG PHẢI pixel art hoàn chỉnh (không có 4-tone ramp shading hay giải phẫu pixel chi tiết). Cần được thay thế hoàn toàn bằng pixel art vẽ tay thật sự trước khi release.

- **Quy chuẩn Canvas**: $128 \times 128\text{ px}$, Lưới pixel $1:1$, Pivot bàn chân tại $(64, 114)$, Khớp nối thắt lưng tại $Y = 80$.
- **4 Master Rigs**:
  1. `FB_Lower_HeavyTank_Set` (Vanguard, Berserker, Templar, Dragon Knight): Bước chân đầm chắc, tấn thấp, chìm trọng tâm.
  2. `FB_Lower_Agility_Set` (Ranger, Shadowblade, Void Blade, God Slayer): Kiễng mũi chân, bước sải nhanh nhẹn.
  3. `FB_Lower_Caster_Set` (Arcanist, Elementalist, Chronomancer): Dáng đứng thẳng, tà buông thẳng.
  4. `FB_Lower_Monk_Set` (Acolyte): Thế tấn mã bộ tĩnh, chuyển động ổn định cân bằng.
- **5 Trạng thái hoạt ảnh**: `Idle` (4f), `Walk` (6f), `Run` (6f), `Dash` (3f burst I-frame), `HitStun` (2f) $\times 5$ hướng nhìn = $105\text{ frames/Rig}$ (Tổng cộng 420 frames wireframe proxy).

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [x] **AC-1 (Đầy Đủ 420 Frames Wireframe Proxy)**:
  - Xuất xưởng 4 bộ Flipbook hoàn chỉnh vào `Content/Art/Characters/MasterRigs/`:
    - `FB_Lower_HeavyTank_Set` (105 frames).
    - `FB_Lower_Agility_Set` (105 frames).
    - `FB_Lower_Caster_Set` (105 frames).
    - `FB_Lower_Monk_Set` (105 frames).
- [x] **AC-2 (Kiểm Tra Hoạt Ảnh & Timing)**:
  - Khung xương hình học phân biệt rõ ràng chân trái/phải, phục vụ kiểm tra chu kỳ chuyển động, nhịp bước (timing) và chuyển đổi trạng thái locomotion trong PaperZD.
- [x] **AC-3 (Khớp Khít Khớp Nối & Pivot)**:
  - Pivot điểm tiếp đất luôn nằm chính xác tại $(64, 114)$ trên toàn bộ 420 frames.
  - Vị trí nối eo $Y = 80$ phẳng phiu, đảm bảo khớp nối liền mạch khi ghép với Upper Body.
