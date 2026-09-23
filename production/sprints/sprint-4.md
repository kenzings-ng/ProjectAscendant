# Sprint 4 — 2026-09-19 to 2026-10-03

> **Sprint**: Sprint 4 (Presentation & Interface Layer — Combat HUD & Interactive Windows)  
> **Stage**: Production  
> **Review Mode**: Lean  
> **Capacity**: 10 days (80 hours) | Buffer (20%): 2 days (16 hours) | Available: 8 days (64 hours)  

---

## Sprint Goal

Hiện thực hóa Tầng Giao Diện & Trải Nghiệm Người Dùng (**Presentation & Interface Layer**) cho *Project Ascendant* theo chuẩn **CommonUI** & **UMG**:  
Triển khai cụm trạng thái Người chơi (**Player Vitals HUD**, thanh máu bóng mờ Catch-up Ghost Bar, tia chớp vàng Perfect Dodge, viền màn hình kiệt sức & nhịp tim máu thấp), Cụm Đấu Trùm (**Boss Health & Stagger Posture Bar**, nhấp nháy đỏ 4.0 Hz khi vỡ thế, tâm ngắm tử huyệt Execution Reticle chiếu từ 3D lên 2D), Hệ thống Số sát thương nảy động (**Floating Combat Text** với quỹ đạo tán xạ hình vòng cung và chữ nổi "PERFECT!"), cùng Cửa sổ tương tác thương nhân & đe rèn (**Merchant Shop & Blacksmith Forge Windows** với cơ chế giữ chuột an toàn 0.8s).

---

## Tasks

### Must Have (Critical Path — 16 hours / 2.0 days)

| ID | Task | Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `hud-001` | [Player Vitals & Status HUD Engine](file:///mnt/Data/Projects/project-games/production/epics/presentation-ui/story-001-player-vitals-hud.md) | `ui-ux-designer` | 1.0 (8h) | `attr-001`, `dash-001` | AC-1: Đồng bộ realtime thuộc tính GAS (HP, Stamina, Mana); AC-2: Bóng mờ Catch-up Ghost Bar trễ 0.40s; AC-3: Tia chớp vàng Golden Flash 0.20s khi Perfect Dodge; AC-4: Viền máu thấp nhịp tim 60-100 BPM và viền xám kiệt sức. |
| `hud-002` | [Boss Health, Stagger Posture & Execution Reticle](file:///mnt/Data/Projects/project-games/production/epics/presentation-ui/story-002-boss-stagger-reticle.md) | `ui-ux-designer` | 1.0 (8h) | `pzd-003`, `cmbt-001` | AC-1: Thanh máu Boss chia 3 phân đoạn 75% và 25% Max HP; AC-2: Thanh Posture tích tụ và nhấp nháy đỏ 4.0 Hz trong 3.0s vỡ thế; AC-3: Gạch chéo icon bộ phận bị phá vỡ; AC-4: Tâm ngắm tử huyệt chiếu từ Socket 3D lên màn hình 2D. |

### Should Have (16 hours / 2.0 days)

| ID | Task | Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `hud-003` | [Floating Combat Text & Action Feedback](file:///mnt/Data/Projects/project-games/production/epics/presentation-ui/story-003-floating-combat-text.md) | `gameplay-programmer` | 1.0 (8h) | `cmbt-001`, `dash-001` | AC-1: Định dạng số sát thương thường (trắng), bạo kích (cam nảy 1.5x), và phá thế (vàng); AC-2: Quỹ đạo tán xạ vật lý đạn đạo $V_0 = 180$, $g = -300$ trong 0.6s; AC-3: Chữ nổi "PERFECT!" khi né chuẩn; AC-4: Object pooling 50 đối tượng chống lag. |
| `hud-004` | [Merchant Shop & Blacksmith Forge Interactive Windows](file:///mnt/Data/Projects/project-games/production/epics/presentation-ui/story-004-shop-forge-windows.md) | `ui-ux-designer` | 1.0 (8h) | `econ-002`, `crft-001` | AC-1: Giao diện Shop 2 cột (Catalog / Inventory / Buyback 10 ô / phụ phí Wanted); AC-2: Giao diện Đe rèn với ô trang bị, ô nguyên liệu vệ tinh, ô Đá bảo hộ; AC-3: Nhấn giữ chuột 0.8s để rèn an toàn; AC-4: Tự đóng khi cách xa > 500cm hoặc vào combat. |

---

## Total Planned Scope
- **Must Have**: 2 stories (16 hours / 2.0 days)
- **Should Have**: 2 stories (16 hours / 2.0 days)
- **Total**: 4 stories (32 hours / 4.0 days) vs **Available Capacity**: 8.0 days (64 hours)

---

## Definition of Done for this Sprint
- [ ] 100% Must Have tasks hoàn thành và vượt qua kiểm thử đơn vị & tích hợp
- [ ] Giao diện HUD cập nhật tức thời theo GAS mà không dùng tick polling gây tụt FPS
- [ ] Số sát thương nảy động tản mượt mà, không đè chữ khi đánh lan (cleave)
- [ ] Cửa sổ giao dịch tự động đóng an toàn khi bị quái tấn công hoặc rời xa NPC
- [ ] Bộ kiểm thử tự động Unreal Automation Tests cho UI models đạt 100% Pass
