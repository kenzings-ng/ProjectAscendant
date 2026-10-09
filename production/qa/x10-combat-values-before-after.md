# X10 — Đồng bộ thông số Dash / Combo / Finisher theo code runtime (trước / sau)

> **Ngày**: 2026-10-09
> **Căn cứ**: Quyết định của chủ dự án 2026-10-09 (DECISIONS.md §12, PR #9): giá trị trong code runtime `UPAGameplayAbility_Dash`, `UPAGameplayAbility_MeleeAttack`, `UPAGameplayAbility_Finisher` là **chuẩn**; tài liệu kế hoạch sửa theo code. Cảm giác chơi với các giá trị này **vẫn chờ chủ dự án duyệt**.
> **Nguồn rà soát**: `production/qa/review-plan-vs-code-2026-10-09.md` (M9); `review-plan-vs-code-2026-10-09/review-A.md` #5–7; `review-B1-B2.md` B1-7, B2-2, B2-3, B2-4, B2-8.
> **Quy ước**: Đường dẫn code tính từ `Source/ProjectAscendant/`. Số dòng "giá trị cũ" tính trên `origin/main` tại commit `8bd5d40` (trước khi sửa). Không sửa code; không sửa `DECISIONS.md`.
> **Nhãn dùng cho cơ chế thiết kế mà code chưa có**: "chưa triển khai trong code (theo quyết định 2026-10-09: giá trị code là chuẩn)".

## 1. Dash (`UPAGameplayAbility_Dash`)

| Tham số | Giá trị code (file:dòng) | Giá trị cũ trong tài liệu (file:dòng) | Giá trị mới |
|---|---|---|---|
| Tổng thời lượng lướt | 0.35s — `Public/Combat/PAGameplayAbility_Dash.h:31` (`kDefaultDuration`), `:150` (`DashDuration`) | 0.45s — `design/gdd/dash-evasion.md:13,35,39,57,61,92,113,129,151`; `attributes-system.md:43,54`; `isometric-controller.md:82`; `foundational-classes.md:235` (BaseDuration); `design/registry/entities.yaml:1453` (`dash_total_duration`); `encounter-boss/EPIC.md:15`; `encounter-boss/story-004:14,28,30,36`; `sprints/sprint-5.md:31`; `README.md:50` | 0.35s |
| Quãng đường lướt | 450 cm — `PAGameplayAbility_Dash.h:32`, `:154` | ≈380 cm — `dash-evasion.md:35,115,183`; `encounter-boss/story-004:28` | 450 cm |
| Đường cong vận tốc | Giảm tuyến tính; V_peak = 2D/T ≈ 2571.43 cm/s — `Private/Combat/PAGameplayAbility_Dash.cpp:53`, `:259-260` (curve 1→0) | Đỉnh 1200 cm/s giữ 0.20s rồi hãm phi tuyến về 550 cm/s — `dash-evasion.md:13,46,53,58,112-114,158`; `isometric-controller.md:82` | V(t) = V_peak·(1 − t/T), về 0 tại 0.35s |
| Thời điểm bắt đầu I-frame | 0.05s — `PAGameplayAbility_Dash.h:33`, `:158`; timer `PAGameplayAbility_Dash.cpp:295` | 0.00s — `dash-evasion.md:52,84`; `combat-system.md:72`; `encounter-boss/story-004:29` | 0.05s |
| Độ dài / kết thúc I-frame | 0.20s, kết thúc 0.25s — `PAGameplayAbility_Dash.h:34-35`, `:162` | 0.28s — `dash-evasion.md:13,52,87,102,103,133,152,183`; `attributes-system.md:43,63,111,137,175,184`; `combat-system.md:72`; `boss-ai.md:112,142,154`; `foundational-classes.md:238,283,428`; `docs/architecture/adr-0001-…:38`; `entities.yaml:1410` (`iframe_duration`); `encounter-boss/EPIC.md:15`; `encounter-boss/story-004:14,29,30`; `sprint-5.md:31`; `README.md:25,50` · 0.25s — `docs/architecture/control-manifest.md:50`; `tr-registry.yaml:86`; `requirements-traceability.md:40` · 200ms (chỉ ghi độ dài, không ghi mốc) — `core-game-loop.md:25,58,119` | 0.20s, từ t = 0.05s đến t = 0.25s |
| Hồi chiêu sau lướt | 0.5s `Cooldown.Dash`, luôn gán khi EndAbility — `PAGameplayAbility_Dash.h:36`, `:166`; `PAGameplayAbility_Dash.cpp:450`; kiểm tra tại `:158` | Không ghi trong GDD; `dash-evasion.md:91` cho phép "Bấm Dash tiếp (Combo lướt)" ngay trong cửa sổ hủy | 0.5s (ghi vào `dash-evasion.md` Tuning Knobs, AC-1, State diagram) |
| Chi phí thể lực | 25 — `PAGameplayAbility_Dash.h:30`, `:146` | 25 (khớp) | 25 (không đổi) |
| Chặn khi Stamina < 25 | Chặn tuyệt đối (cả `State.Exhausted`) — `PAGameplayAbility_Dash.cpp:24,30` | Desperation Roll cho lướt khi Stamina < 25 — `attributes-system.md:111`; `dash-evasion.md:103`; `foundational-classes.md:293`; `foundation-attributes/story-003:34,77` | Chặn khi < 25; Desperation Roll được đánh dấu "chưa triển khai trong code (…)" |
| Cửa sổ Dash Cancel / Dash Attack | Không có trong code | 0.35s – 0.45s — `dash-evasion.md:49,61-62,88,157,186`; `combat-system.md:15,48,73,99,190`; `encounter-boss/story-004:37`; `sprint-5.md:31` | Không đặt mốc mới (không bịa số); đánh dấu "chưa triển khai trong code (…)" và ghi Open Question cần định lại cho cú lướt 0.35s |
| Perfect Dodge (0.05–0.15s, +15 Stamina, hitstop 0.08s) | Không có trong ability runtime (chỉ trong `FPADashModel`, `Public/Combat/PADashTypes.h:300-337`) | `dash-evasion.md:64-70,117-122,153-156,184`; `boss-ai.md:112`; `encounter-boss/story-004:31-35`; `README.md:25,50` | Giữ ý đồ thiết kế, đánh dấu "chưa triển khai trong code (…)" |
| Chống rơi mép vực / xuyên quái | Không có trong ability runtime | `dash-evasion.md:55,60,129,185`; `encounter-boss/story-004:36` | Giữ ý đồ thiết kế, đánh dấu "chưa triển khai trong code (…)" |

## 2. Combo 3 nhịp (`UPAGameplayAbility_MeleeAttack`)

| Tham số | Giá trị code (file:dòng) | Giá trị cũ trong tài liệu (file:dòng) | Giá trị mới |
|---|---|---|---|
| Cửa sổ reset combo | 1.2s giữa hai lần kích hoạt (`>` 1.2s → về Nhịp 1) — `Public/Combat/PAGameplayAbility_MeleeAttack.h:36`, `:167`; `Private/Combat/PAGameplayAbility_MeleeAttack.cpp:41` | 0.60s — `combat-system.md:45,79,80,151,189` | 1.2s, tính từ lần kích hoạt đòn trước |
| Hệ số sát thương Nhịp 1/2/3 | 1.0 / 1.2 / 1.6 — `PAGameplayAbility_MeleeAttack.cpp:24-28` | Nhịp 3 = 1.8× (180%) — `combat-system.md:43,110` | 1.0 / 1.2 / 1.6 |
| Sát thương Posture Nhịp 1/2/3 | 10 / 15 / 25 — `PAGameplayAbility_MeleeAttack.cpp:24-28` | Nhịp 3 = 30 — `combat-system.md:43,115`; dải "10–30" — `stagger-system.md:94`, `boss-ai.md:38` | 10 / 15 / 25 (dải 10–25) |
| Thời lượng mỗi đòn | 0.40s cho mọi nhịp — `PAGameplayAbility_MeleeAttack.h:199`; timer `PAGameplayAbility_MeleeAttack.cpp:263` | 0.25s / 0.28s / 0.38s — `combat-system.md:41-43,81` | 0.40s mọi nhịp |
| Vùng quét | Nón 90° (±45°), bán kính 180cm, mọi nhịp — `PAGameplayAbility_MeleeAttack.h:37-38`, `:171,175` | Nhịp 2 "góc rộng 120°" — `combat-system.md:42` | Nón 90°, 180cm mọi nhịp |
| Input buffer | **Không có** trong code production | 0.15s — `combat-system.md:37,135,152,189` · 250ms — `control-manifest.md:52`; `tr-registry.yaml:125`; `requirements-traceability.md:44`; `design/accessibility-requirements.md:109` | Giữ ý đồ, đánh dấu "chưa triển khai trong code (…)"; ghi rõ hai giá trị thiết kế còn lệch nhau, chốt khi triển khai |
| Sát thương máu cơ sở | `BaseAttackDamage` 20 × hệ số — `PAGameplayAbility_MeleeAttack.h:179`, `.cpp:343` | Công thức GDD dùng BaseATK/Defense/Crit (`combat-system.md:109`) | Ghi chú giá trị code trong `combat-system.md`; công thức không sửa (thuộc M11, ngoài phạm vi X10) |

## 3. Đòn Kết Liễu (`UPAGameplayAbility_Finisher`)

| Tham số | Giá trị code (file:dòng) | Giá trị cũ trong tài liệu (file:dòng) | Giá trị mới |
|---|---|---|---|
| Bất tử người kết liễu | 1.5s `State.Invulnerable` — `Public/Combat/PAGameplayAbility_Finisher.h:91`; `Private/Combat/PAGameplayAbility_Finisher.cpp:379` | 1.2s — `stagger-system.md:46,77,130,156,188`; `encounter-boss/EPIC.md:13`; `encounter-boss/story-002:30,41,51`; `sprint-5.md:13,24`; `README.md:24`; `itemization.md:125` | 1.5s |
| Choáng mục tiêu | 1.5s `State.Stunned` — `PAGameplayAbility_Finisher.h:91`; `.cpp:414`; kết thúc chuỗi sau 1.5s `.cpp:241` | Không ghi (GDD chỉ có cửa sổ Stagger 3.0s trước kết liễu) | 1.5s (bổ sung vào `stagger-system.md`, story/sprint liên quan) |
| Miễn nhiễm Posture sau kết liễu | **Không có** trong code (Finisher chỉ reset Posture về 0 — `.cpp:337`) | 2.0s `State.PostureImmune` — `stagger-system.md:47,79`; `encounter-boss/story-002:31,41,51`; `README.md:48` | Giữ ý đồ, đánh dấu "chưa triển khai trong code (…)" |
| Sát thương kết liễu | `MaxHP × 0.25` — `PAGameplayAbility_Finisher.h:95`; `PAGameplayAbility_MeleeAttack.cpp:104-108`; dùng tại `PAGameplayAbility_Finisher.cpp:312` | `(MaxHP × 0.25) + (BaseDamage × 3.0)` — `attributes-system.md:98`; `foundation-attributes/story-002:41,89` | `MaxHP × 0.25` |
| Cự ly kết liễu | 250 cm (2D) — `PAGameplayAbility_Finisher.h:87` | 3m — `attributes-system.md:177` (các chỗ khác đã ghi 250cm) | 250 cm |
| Cửa sổ Stagger trước kết liễu | 3.0s (`APABaseCharacter::HandlePostureBroken`, ngoài Finisher) | 3.0s (khớp) | Không đổi |

## 4. Lệch trong chính code (ghi nhận, không sửa code)

| Vị trí | Giá trị | Ghi chú |
|---|---|---|
| `Public/Combat/PADashTypes.h:37-83` (`FPADashConfig`) | Tổng 0.45s, I-frame 0.28s (từ 0.00s), đỉnh 1200 cm/s, hủy từ 0.35s | `FPADashModel` / `UPADashEvasionComponent` không được actor nào dùng ở runtime; chỉ test `PADashTests.cpp`, `PACombatRegressionHardeningTests.cpp` dùng |
| `Private/Combat/AscendantAttributeSet.cpp:24` | `InitIFrameDuration(0.28f)` | Ability Dash không đọc attribute này |
| `Public/Combat/PAStaggerTypes.h:56,60` (`FPAStaggerConfig`) | ExecutionInvulnDuration 1.20s, PostureImmunityDuration 2.0s | `FPAStaggerModel` / `UPAStaggerComponent` không dùng ở runtime |
| `Private/Combat/PADamageExecutionCalculation.cpp:64` | `MaxHP × 0.25 + BaseDamage × 3.0` | Finisher runtime không gọi |
| `Private/Combat/PAStaminaComponent.cpp:32-35` | Nhánh Desperation Roll | `FPAStaminaPipeline` không được gọi ở runtime (M12) |

## 5. File tài liệu đã sửa

- `design/gdd/dash-evasion.md`, `combat-system.md`, `stagger-system.md`, `attributes-system.md`, `boss-ai.md`, `core-game-loop.md`, `foundational-classes.md`, `isometric-controller.md`, `itemization.md`
- `design/registry/entities.yaml` (`iframe_duration` 0.28 → 0.20, `dash_total_duration` 0.45 → 0.35, ghi chú `ranger_dash_duration`)
- `design/accessibility-requirements.md` (đánh dấu input buffer)
- `docs/architecture/control-manifest.md`, `tr-registry.yaml` (TR-dash-001, TR-combat-002), `requirements-traceability.md`, `adr-0001-open-world-mmo-combat-networking.md`, `adr-0002-gas-integration-paperzd-pixel-sprites.md`
- `production/epics/encounter-boss/EPIC.md`, `story-002-stagger-execution.md`, `story-004-dash-iframe-perfect-dodge.md`
- `production/epics/foundation-attributes/story-002-damage-posture-calculations.md`, `story-003-stamina-exhaustion-pipeline.md`
- `production/sprints/sprint-5.md`
- `README.md`

Không cần sửa (đã khớp code): `production/epics/core-combat/EPIC.md`, `story-001-gas-dash-ability.md`, `story-002-gas-combo-finisher.md`, `production/sprints/sprint-2.md`, `docs/architecture/architecture.md`.

## 6. Việc còn mở

1. Chủ dự án duyệt cảm giác chơi với bộ số code (DECISIONS.md §12).
2. Định lại mốc cửa sổ Dash Cancel / Dash Attack cho cú lướt 0.35s + hồi chiêu 0.5s.
3. Định lại biến thể lướt của Ranger: công thức `BaseDuration − 0.05` trên nền 0.35s cho 0.30s, còn `entities.yaml` vẫn giữ `ranger_dash_duration = 0.40`; đỉnh tốc 1350 cm/s không còn căn cứ.
4. Chốt một giá trị input buffer duy nhất khi triển khai (0.15s và 250ms đang lệch nhau).
5. Task code riêng cho các lệch code–code ở mục 4.
6. Các story [x] của `FPADashModel` / `FPAStaggerModel` / Desperation Roll được kiểm bằng model không dùng ở runtime: trạng thái story chưa đổi (thuộc M19), chỉ thêm ghi chú.
7. `production/qa/qa-plan-sprint-1-2026-09-16.md:72,99,216` (kế hoạch QA lịch sử) vẫn mô tả Desperation Roll / IFrameDuration 0.28; không sửa vì là tài liệu QA đã chốt theo ngày.
