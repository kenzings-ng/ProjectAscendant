# HUD Design

> **Status**: In Design — MOCKUP CHỜ CHỦ DỰ ÁN DUYỆT (Z1)
> **Author**: ux-designer (Z1, chế độ tự vận hành)
> **Last Updated**: 2026-10-10
> **Platform Targets**: PC (bàn phím + chuột, chính) · Gamepad (Full, CommonUI); 16:9 safe zone, ultrawide neo về 16:9 (`combat-hud.md` Edge Cases)
> **Template**: HUD Design (`/ux-design hud`)
> **Mockup**: `design/ux/mockups/ascendant-ui-mockups.html` — màn 4 (HUD trong game, có Boss)
> **Nguồn yêu cầu**: `design/gdd/combat-hud.md` (toàn bộ) · `design/gdd/attributes-system.md` §Core Attributes, §UI Requirements · `design/gdd/stagger-system.md` §UI Requirements · `design/gdd/inventory-system.md` §3.3 (Quickbar 1–4) · `design/gdd/foundational-classes.md` §1 NOTE (Action Deck Q/E/R/F) · `design/gdd/dash-evasion.md` (Perfect Dodge) · `design/accessibility-requirements.md` §2, §5.1 · `design/ux/interaction-patterns.md` §3.3, §3.4, §3.6 · `production/epics/presentation-ui/*`

---

## HUD Philosophy

"Tối giản nhưng hiện diện" (`combat-hud.md` Overview): 85% màn hình dành cho trận đấu isometric; chỉ hiện thông tin sinh tử. Trạng thái nguy hiểm được báo bằng hiệu ứng viền màn hình (vignette) thay vì thêm bảng chữ. Ánh sáng và màu chỉ dùng cho điều sống còn (art-bible §1.2 Nguyên tắc 2).

---

## Information Architecture

### Full Information Inventory

| # | Thông tin | Hệ thống nguồn | GDD |
|---|---|---|---|
| 1 | Máu người chơi + bóng mờ catch-up | Attributes (`Health/MaxHealth`) | combat-hud §1, Formulas 1 |
| 2 | Thể lực + Golden Flash + Kiệt Sức | Attributes (`Stamina`), Dash & Evasion | combat-hud §1 |
| 3 | Mana | Attributes (`Mana`) | combat-hud §1 |
| 4 | Thế đứng người chơi (Posture 0→100) | Attributes (`Posture/MaxPosture`); Vanguard Guard Break | attributes-system Core Attributes; foundational-classes Edge Case 1 |
| 5 | Action Deck 4 ô (Q/E/R/F) + hồi chiêu + chi phí | Skill Progression / GAS | combat-hud §1; interaction-patterns §3.3 |
| 6 | Quickbar tiêu hao 1–4 + số lượng | Inventory | inventory-system §3.3 |
| 7 | Tên + cấp Boss | Boss AI | combat-hud §1 |
| 8 | Máu Boss chia 3 pha (vạch 75% / 25%) | Boss AI / Attributes | combat-hud §1 |
| 9 | Posture Boss + nhấp nháy vỡ thế 3.0s | Stagger | combat-hud §1; stagger-system UI Req |
| 10 | Icon bộ phận Boss + gạch chéo khi gãy | Stagger (Part Breaking) | combat-hud §1 |
| 11 | Tâm ngắm tử huyệt + icon phím | Stagger | combat-hud §1 |
| 12 | Số sát thương nổi (thường / bạo kích / posture) | Combat | combat-hud §1, Formulas 3 |
| 13 | Chữ "PERFECT!" | Dash & Evasion | combat-hud §1 |
| 14 | Vignette máu thấp (< 20%, 60→100 BPM) | Attributes | combat-hud Formulas 2 |
| 15 | Viền xám Kiệt Sức (khử bão hòa 50%) | Attributes (`State.Exhausted`) | combat-hud §1 |
| 16 | Biểu ngữ phá bộ phận (1.5s) | Stagger | stagger-system UI Req |
| 17 | Biểu ngữ "THỦ LĨNH ĐÃ BỊ TIÊU DIỆT" | Boss AI | combat-hud Edge Cases |
| 18 | Prompt tương tác (Shop/Forge/NPC) | Interaction | interaction-patterns §3.5 |
| 19 | Viền vàng Vùng An Toàn | Zone | accessibility-requirements §5.2 |

GDD không có UI Requirements riêng cho HUD ở: `zone-system.md`, `multiplayer-coop.md`, `itemization.md`, `skill-progression-system.md`, `isometric-controller.md`, `character-visual-system.md`, `core-game-loop.md`, `combat-system.md`, `boss-ai.md`. Spec này **không thêm** mục HUD cho các hệ thống đó (minimap, quest tracker, party frame, EXP bar, Karma/Wanted…) — xem Open Questions Q7.

### Categorization

| Loại | Mục |
|---|---|
| **Must Show** | 1 Máu, 2 Thể lực, 3 Mana, 4 Thế đứng người chơi, 5 Action Deck |
| **Contextual** | 6 Quickbar (hiện khi có vật phẩm gán; mờ 40% ngoài combat), 7–11 Cụm Boss (khi giao tranh Boss), 12–13 chữ nổi, 14–15 vignette, 16–17 biểu ngữ, 18 prompt, 19 viền an toàn |
| **On Demand** | Không có trong spec này (túi đồ, bản đồ là màn riêng) |
| **Hidden** | Nhịp tim máu thấp còn báo bằng âm `SFX_LowHealth_Heartbeat`; kiệt sức bằng `SFX_Exhaustion_Gasp` |

---

## Layout Zones

```
┌──────────────────────────────────────────────────────────────────────────┐
│            ┌───────── CỤM BOSS (900×45 @1080p, đỉnh giữa) ─────────┐    │
│            │ [◆] STONE GOLEM · Cấp 25      [Sừng] [Đuôi ✕]          │    │
│            │ ██████████████████│█████████████████████│████░░░░░░░    │    │
│            │ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░ Thế đứng (Posture)        │    │
│            └────────────────────────────────────────────────────────┘    │
│                                                                          │
│                         (sàn đấu isometric — 85%)                        │
│                   1240      ⟡ tâm ngắm tử huyệt [LMB]                    │
│                       PERFECT!                                           │
│                                                                          │
│ ┌ CỤM NGƯỜI CHƠI 400×160 ┐                       ┌ ACTION DECK + QUICKBAR┐│
│ │ ❤ Máu  ███████▓▓░░ 96/120│                     │ [Q][E][R][F]           ││
│ │ ⚡ Thể lực ████████░ 88   │                     │ [1][2][3][4]           ││
│ │ ◆ Mana ██████░░░░ 52     │                     └────────────────────────┘│
│ │ ⛨ Thế đứng ▓▓░░░░ 22    │                                               │
│ └──────────────────────────┘                                               │
└──────────────────────────────────────────────────────────────────────────┘
```

- **Trên giữa:** Cụm Boss (`WBP_BossHealthBar`) — chỉ khi giao tranh Boss.
- **Dưới trái:** Cụm người chơi (`WBP_PlayerVitals`) 400×160px.
- **Dưới phải:** Action Deck 4 ô + Quickbar 4 ô. combat-hud đặt Action Deck trong cụm người chơi; spec này tách sang góc dưới phải để giữ cụm 400×160 cho 4 thanh — **chờ duyệt** (Open Questions Q2).
- **Không gian thế giới:** tâm ngắm, chữ nổi, prompt tương tác.
- Mọi cụm neo trong safe zone 16:9; lề tối thiểu 48px @1080p.

---

## HUD Element Specifications

| Phần tử | Loại | Nội dung | Dạng | Cập nhật | Kích hoạt | Hoạt ảnh |
|---|---|---|---|---|---|---|
| Thanh Máu | Must Show | HP hiện tại/tối đa | Thanh ngang + số | Event (GAS `OnAttributeChanged`) | Luôn | Tụt tức thì; bóng mờ giữ 0.40s rồi `FInterpTo` tốc độ 3.5 |
| Thanh Thể Lực | Must Show | Stamina | Thanh + số | Event | Luôn | Golden Flash 0.20s khi Perfect Dodge; xám tro + icon khóa khi Kiệt Sức |
| Thanh Mana | Must Show | Mana | Thanh + số | Event | Luôn | Không |
| Thanh Thế Đứng người chơi | Must Show | Posture 0→100 (tích lũy, càng đầy càng nguy) | Thanh mảnh + số | Event | Luôn (mờ khi = 0) | Nhấp nháy viền khi ≥ 80% (đề xuất, Q3) |
| Action Deck Q/E/R/F | Must Show | Icon kỹ năng, kim quét hồi chiêu, số giây, chi phí | Ô 56×56 | Event (GAS cooldown) | Luôn | Flash vàng 0.15s khi dùng; viền đỏ giật khi bấm lúc hồi chiêu (pattern §3.3) |
| Quickbar 1–4 | Contextual | Icon tiêu hao + số lượng | Ô 48×48 | Event (inventory) | Có vật phẩm gán | Mờ khi trống |
| Tên + cấp Boss | Contextual | `BossName`, `BossLevel` | Chữ La Mã cổ điển | Khi đặt mục tiêu | Giao tranh Boss | Fade-in 0.3s |
| Máu Boss 3 pha | Contextual | HP%, vạch 75%/25% | Thanh 900px | Event | Giao tranh Boss | Dissolve vàng khi chết |
| Posture Boss | Contextual | Posture% | Thanh dưới thanh máu | Event | Giao tranh Boss | Đạt 100%: bùng sáng + nhấp nháy đỏ 4.0 Hz trong 3.0s |
| Icon bộ phận | Contextual | `Parts[].PartId`, `bIsBroken` | Icon + gạch chéo đỏ | Event | Giao tranh Boss | Gạch chéo xuất hiện tức thì |
| Tâm ngắm tử huyệt | Contextual | Vị trí `Socket_Execution` + icon phím | Vòng xoay + chip phím | Mỗi frame (World-to-Screen) | `State.Staggered` | Xoay chậm; biến mất khi hết 3.0s |
| Số sát thương | Contextual | Số nguyên | Chữ nổi | Event | Gây sát thương | Quỹ đạo P(t) với V0z 180 cm/s, g −300; mờ sau 0.60s; bạo kích ×1.5 + nảy; posture ×0.85 |
| PERFECT! | Contextual | Chữ cố định | Chữ nổi | Event | Perfect Dodge | 0.50s trên đầu nhân vật |
| Vignette máu thấp | Contextual | — | Viền đỏ 4 góc | Event | HP < 20% | Nhịp 60→100 BPM |
| Viền Kiệt Sức | Contextual | — | Khử bão hòa viền 50% | Event | `State.Exhausted` | Fade 0.2s |
| Biểu ngữ phá bộ phận | Contextual | "BỘ PHẬN ĐÃ BỊ PHÁ HỦY: …" | Banner giữa trên | Event | Part break | 1.5s |
| Biểu ngữ diệt Boss | Contextual | "THỦ LĨNH ĐÃ BỊ TIÊU DIỆT" | Banner lớn | Event | Boss chết | 3.0s (đề xuất) |
| Prompt tương tác | Contextual | Chip phím + động từ | Billboard | Khoảng cách | Trong bán kính tương tác | Fade 0.2s |

---

## HUD States by Gameplay Context

| State / Variant | Trigger | What Changes |
|---|---|---|
| Khám phá (Default) | Không giao tranh | Cụm người chơi + Action Deck; Quickbar mờ 40%; không có Cụm Boss |
| Giao tranh thường | `State.InCombat` | Quickbar sáng 100%; chữ nổi bật |
| Giao tranh Boss | Boss được đặt làm mục tiêu (`UPABossHealthWidget::SetBossTarget`) | Cụm Boss fade-in đỉnh màn |
| Boss vỡ thế | `TriggerPostureBroken` / `State.Staggered` | Posture Boss nhấp nháy đỏ 4.0 Hz + icon Khiên Vỡ (art-bible §4.3); tâm ngắm hiện 3.0s |
| Máu thấp | HP < 20% | Vignette nhịp tim + viền thanh máu nhấp nháy + icon tim nứt |
| Kiệt Sức | `State.Exhausted` | Thanh Thể Lực xám tro + icon khóa; viền màn khử bão hòa |
| Vùng an toàn | Vào Sanctuary | Viền vàng mảnh quanh màn (accessibility §5.2) |
| Hội thoại / cửa sổ Shop-Forge | Mở `UCommonActivatableWidget` | Ẩn Action Deck + Quickbar + chữ nổi; cụm người chơi giữ 60% opacity |
| Cutscene | Sequencer | Ẩn toàn bộ HUD |
| Tạm dừng | Menu tạm dừng | Chưa spec (không có GDD) — xem Q8 |
| Chết | HP = 0 | Ẩn HUD, chờ màn chết (chưa spec) |
| Empty / no-data | Ô Action Deck chưa gán kỹ năng | Ô rỗng có viền nét đứt, không ẩn |

---

## Information Hierarchy

| Phần tử | Tier | Lý do | Khi ẩn, thông tin đi qua |
|---|---|---|---|
| Máu, Thể lực | MUST KEEP | Quyết định né/đánh | — |
| Mana, Thế đứng người chơi | SHOULD KEEP | Quyết định dùng kỹ năng / đỡ đòn | Không |
| Action Deck | MUST KEEP | Hồi chiêu | — |
| Cụm Boss | MUST KEEP (khi có Boss) | Pha + cơ hội phá thế | — |
| Tâm ngắm tử huyệt | MUST KEEP | Cửa sổ 3.0s | Icon Khiên Vỡ trên Boss + âm `SFX_Combat_PostureBreak` |
| Quickbar | CAN HIDE | Có phím tắt cố định | Không |
| Số sát thương | CAN HIDE (toggle, combat-hud UI Req) | Phản hồi phụ | Âm va chạm |
| Biểu ngữ | CAN HIDE | Thông tin trễ được | Icon bộ phận gạch chéo |

## Visual Budget

- Khám phá: ≤ 8% diện tích màn; Giao tranh Boss: ≤ 15% (giữ "85% không gian cho trận đấu", combat-hud §1).
- Tối đa 64 chữ nổi đồng thời (`control-manifest.md` §4 Object-Pooled Floating Combat Text).
- Thứ tự bỏ khi vượt ngân sách: biểu ngữ → số sát thương thường → Quickbar.

## Feedback & Notification Systems

| Thông báo | Nguồn | Vị trí | Thời lượng | Tối đa cùng lúc | Ưu tiên | Hàng đợi |
|---|---|---|---|---|---|---|
| Số sát thương | Combat | Trên mục tiêu, tản quạt ±25px | 0.60s | 64 (pool) | Thấp | Không xếp hàng; pool đầy thì tái dùng bản cũ nhất (`FPACombatTextPool`) |
| PERFECT! | Dash | Trên đầu nhân vật | 0.50s | 1 | Cao | Thay thế bản đang hiện |
| Biểu ngữ phá bộ phận | Stagger | Giữa trên, dưới Cụm Boss | 1.5s | 1 | Trung | Xếp hàng nối tiếp; giữ trong combat |
| Biểu ngữ diệt Boss | Boss AI | Giữa màn | 3.0s | 1 | Cao nhất | Hủy mọi biểu ngữ đang chờ |
| Prompt tương tác | Interaction | Trên vật thể | Khi trong bán kính | 1 (gần nhất) | Trung | Chỉ hiện mục gần nhất |

## Platform Adaptation

- PC: prompt phím bàn phím; Gamepad: CommonUI đổi icon trong 1 frame khi chạm tay cầm.
- Ultrawide 21:9/32:9: neo cụm theo safe zone 16:9.
- Console (dự kiến): safe area 90%; không có touch.

## Accessibility

Tier Standard (`design/accessibility-requirements.md`).
- Số HUD (HP, Mana, Posture) ≥ 22px đậm + bóng 2px (§2.1); chữ nổi ≥ 26px.
- Không dùng màu đơn độc: mỗi thanh có icon riêng (tim, tia sét, hình thoi, khiên) và nhãn; máu thấp = vignette + icon tim nứt + âm; vỡ thế = icon Khiên Vỡ (art-bible §4.3); bộ phận gãy = gạch chéo hình học.
- Flash Suppression: nhấp nháy 4.0 Hz của Posture Boss đổi thành sáng đều + viền dày; Golden Flash dùng đường cong 0.2s.
- Screen Shake slider 0–100% (§2.3).
- HUD Scale 80–150% (§5.1), HUD Opacity 50–100% (combat-hud UI Req).
- Colorblind modes (§2.2) áp dụng qua post-process; HUD giữ icon nên vẫn đọc được.

## Tuning Knobs

| Cài đặt người chơi | Khoảng | Mặc định | Nguồn |
|---|---|---|---|
| Hiện số sát thương | Bật/Tắt | Bật | combat-hud UI Req |
| Rung màn hình | 0–100% | 100% | accessibility §2.3 (combat-hud: Bật/Tắt) |
| Độ trong suốt HUD | 50–100% | 100% | combat-hud UI Req |
| Tỉ lệ HUD | 80–150%, bước 10% | 100% | accessibility §5.1 |
| Giảm nhấp nháy | Bật/Tắt | Tắt | accessibility §2.3 |

Hằng số thiết kế (không phải cài đặt người chơi): `CatchUpDelay` 0.40s, `CatchUpInterpSpeed` 3.5, `GoldenFlashDuration` 0.20s, `DamageNumberDuration` 0.60s, `LowHealthThreshold` 0.20, `StaggerBlinkRate` 4.0 Hz (`combat-hud.md` Tuning Knobs; khớp `FPAVitalsConfig`, `FPABossHUDConfig`, `FPACombatTextConfig`).

## Data Bindings (ánh xạ vào C++ hiện có)

| Phần tử | Lớp / model hiện có | Gap |
|---|---|---|
| Máu / Thể lực / Mana / bóng mờ / Golden Flash / Kiệt Sức / vignette | `UPAPlayerVitalsWidget` + `FPAVitalsModel` (`GetHealthPercent`, `GetGhostHealthPercent`, `GetStaminaBarState`, `GetHeartbeatBPM`, delegates `OnHealthPercentChanged`…) | Widget là `UUserWidget` (phải CommonUI); story-001 AC-1 chưa có code tạo widget và bind AttributeSet |
| Thế đứng người chơi | Không có | `FPAVitalsModel` không có Posture; cần thêm `CurrentPosture/MaxPosture` + bind `OnPostureChanged` |
| Action Deck | Không có widget | Cần widget ô kỹ năng đọc cooldown GAS |
| Quickbar 1–4 | `UPAInventoryComponent` (dữ liệu) | Chưa có widget HUD |
| Cụm Boss | `UPABossHealthWidget` + `FPABossHUDModel` (`GetBossHPPercent`, `GetBossPosturePercent`, `IsStaggerFlashing`, `IsBlinkVisible`, `GetPhaseNotchPercents`, `IsPartBroken`, `IsExecutionReticleVisible`; events `OnBossStaggerStarted/Ended`, `OnBossPartBroken`, `OnBossPhaseChanged`) | Là `UUserWidget`; **nối runtime Boss là phụ thuộc** của ROADMAP Giai đoạn 1 (M6.1 Runtime Wiring, `production/ROADMAP.md:40`) — spec chỉ định nghĩa giao diện |
| Chữ nổi | `UPAFloatingCombatTextComponent` + `FPACombatTextPool` (`EPACombatTextType`: Normal, Critical, Posture, PerfectDodgeCallout) | Pool mặc định 50 (`FPACombatTextConfig::MaxPoolSize`) ≠ 64 của control-manifest §4; chưa có renderer UMG |
| Prompt tương tác | — | Chưa có widget |

## Acceptance Criteria

- [ ] Ở 1920×1080 và 3440×1440, cụm người chơi và Cụm Boss nằm trong safe zone 16:9, lề ≥ 48px.
- [ ] Khám phá: chỉ cụm người chơi + Action Deck (+ Quickbar mờ) hiện, HUD ≤ 8% màn; Boss: ≤ 15%.
- [ ] Mất 30 HP từ 80/100: thanh máu tụt ngay về 50%, bóng mờ đứng yên 0.40s rồi co dần.
- [ ] Posture Boss đạt 100%: thanh nhấp nháy đỏ 4.0 Hz trong 3.0s, tâm ngắm hiện ở ngực Boss, icon Khiên Vỡ hiện; hết 3.0s tất cả biến mất.
- [ ] Máu < 20%: vignette đập 60→100 BPM theo công thức combat-hud, kèm icon tim nứt (không chỉ màu).
- [ ] Hai biểu ngữ phá bộ phận đến cùng lúc hiện nối tiếp, không chồng nhau; biểu ngữ diệt Boss hủy hàng đợi.
- [ ] Tắt "Hiện số sát thương" → không spawn chữ nổi; HUD Opacity 50% áp dụng mọi cụm.
- [ ] Chạm tay cầm: icon phím trên Action Deck và tâm ngắm đổi trong 1 frame.

## Open Questions

- **Q1 (màu thanh):** Ba nguồn mâu thuẫn. combat-hud: Máu đỏ thẫm, Thể lực ngọc bích (Cyan-Green), Mana lam ngọc (Deep Blue), Posture Boss vàng kim. art-bible §4.2: Cyan dùng cho cả Thể lực và Mana; Amber cho Stagger Boss. interaction-patterns §3.4: Máu `#C42021`, Mana `#205FC4`, Thể lực Amber `#D49B2A`, Posture `#B0B4BC`. Mockup chọn: Máu Crimson `#9E1A1A`, Thể lực Spectral Cyan `#1ED5C6`, Mana Cobalt `#205FC4`, Posture Boss Amber `#E6A122`, Posture người chơi Stone Silver `#B0B4BC`. Cần chốt và sửa pattern library.
- **Q2:** Đặt Action Deck trong cụm người chơi (combat-hud) hay góc dưới phải (mockup)?
- **Q3:** combat-hud không liệt kê thanh Thế đứng người chơi; attributes-system có Posture cho người chơi và Guard Break của Vanguard dựa vào nó. Mockup hiện thanh này theo yêu cầu Z1 — chờ duyệt. Ngưỡng cảnh báo 80% là đề xuất.
- **Q4 (phím kết liễu):** combat-hud ghi icon `Attack`/`Interact`; attributes-system UI Req ghi `[E]`/`[X]`; art-bible §4.3 ghi `[F]`; attributes AC-3 ghi "bấm phím tấn công". Mockup dùng phím Tấn công. Cần chốt.
- **Q5 (Posture Boss nhấp nháy):** stagger-system ghi "nhấp nháy đỏ khi sắp chạm 100%" và tâm ngắm "trên đỉnh đầu"; combat-hud và code (`FPABossHUDModel`) nhấp nháy **sau** khi đạt 100% và tâm ngắm ở `Socket_Execution` ngực. Spec theo combat-hud + code.
- **Q6 (bộ phận Boss):** combat-hud: icon Sừng và Đuôi; blacksmithing: 4 bộ phận (Sừng, Vảy Đuôi, Giáp Ngực, Cánh). Model hỗ trợ danh sách bất kỳ; mockup hiện 2 theo combat-hud.
- **Q7:** Các hệ thống không có UI Requirements cho HUD (minimap, Karma/Wanted, EXP, party) — có phải không cần HUD không? Chưa thêm.
- **Q8 (Menu tạm dừng):** Đã bỏ khỏi Z1 vì không có GDD nguồn; chỉ có `accessibility-requirements.md` §5.2 nói tạm dừng trong Vùng An Toàn khi chơi một mình. Cần GDD/quyết định trước khi spec.
- **Q9:** Pool chữ nổi 50 (code) vs 64 (control-manifest) — chốt một số.
