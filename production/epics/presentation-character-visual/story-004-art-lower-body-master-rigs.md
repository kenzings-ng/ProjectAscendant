# Story 004: 4 Master Lower Body Animation Sets (420 Frames)

> **Epic**: `EPIC-CHARACTER-VISUAL-001` (Character & NPC Visual Identity System)  
> **Story ID**: `visual-004`  
> **Layer**: Presentation / Pixel Art Asset Production  
> **Type**: Art & Production (Handcrafted Pixel Sprites)  
> **Estimate**: 2.5 days (20 hours)  
> **Status**: Ready for Dev  
> **Owner**: Technical Artist & Pixel Artist  
> **Governing Spec**: [`SPEC-ART-2026-09-23-V2`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/art/pixel-asset-specifications.md)  

---

## 1. Bối Cảnh & Mục Tiêu Sản Xuất

Thay thế các dummy placeholder flipbooks trong `visual-001` bằng các bộ sprite vẽ tay chuẩn Anti-AI cho phần thân dưới (Chân, Bàn chân và Đai thắt lưng) của nhân vật.
- **Quy chuẩn Canvas**: $128 \times 128\text{ px}$, Lưới pixel $1:1$, Pivot bàn chân tại $(64, 114)$, Đường cắt thắt lưng tại $Y = 80$.
- **4 Master Rigs**:
  1. `FB_Lower_HeavyTank_Set` (Vanguard, Berserker, Templar, Dragon Knight): Bước chân đầm chắc, tấn thấp, chìm trọng tâm.
  2. `FB_Lower_Agility_Set` (Ranger, Shadowblade, Void Blade, God Slayer): Kiễng mũi chân, bước sải nhanh nhẹn, bùng nổ.
  3. `FB_Lower_Caster_Set` (Arcanist, Elementalist, Chronomancer): Dáng đứng thẳng, tà váy chùng buông thẳng, lướt nhẹ nhàng.
  4. `FB_Lower_Monk_Set` (Acolyte): Thế tấn mã bộ tĩnh, chuyển động ổn định cân bằng.
- **5 Trạng thái hoạt ảnh**: `Idle` (4f), `Walk` (6f), `Run` (6f), `Dash` (3f burst I-frame), `HitStun` (2f) $\times 5$ hướng nhìn = $105\text{ frames/Rig}$.

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [ ] **AC-1 (Đầy Đủ 420 Frames Vẽ Tay)**:
  - Xuất xưởng 4 bộ Flipbook hoàn chỉnh vào `Content/Art/Characters/MasterRigs/`:
    - `FB_Lower_HeavyTank_Set` (105 frames).
    - `FB_Lower_Agility_Set` (105 frames).
    - `FB_Lower_Caster_Set` (105 frames).
    - `FB_Lower_Monk_Set` (105 frames).
- [ ] **AC-2 (Quy Chuẩn Kỹ Thuật Anti-AI)**:
  - 100% sprite tuân thủ quy tắc 4-tone ramp, nguồn sáng cứng góc $10$ giờ ($45^\circ$).
  - Không Mixels, không Pillow Shading, viền ngoài 1px than sẫm, Selout nội bộ.
- [ ] **AC-3 (Khớp Khít Tuyệt Đối Với PaperZD)**:
  - Pivot điểm tiếp đất luôn nằm chính xác tại $(64, 114)$ trên toàn bộ 420 frames.
  - Vị trí nối eo $Y = 80$ phẳng phiu, không bị hở pixel khi ghép với Upper Body.
