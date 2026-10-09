# Báo Cáo Rà Soát: Tài Liệu Plan ↔ Code (2026-10-09)

> **Plan**: [`production/plans/review-plan-vs-code-2026-10-09.md`](../plans/review-plan-vs-code-2026-10-09.md)
> **Loại**: Rà soát read-only. Đợt này không sửa code hay GDD.
> **Báo cáo thô từng nhóm**: [`review-plan-vs-code-2026-10-09/`](review-plan-vs-code-2026-10-09/)
> **Phương pháp**: 6 nhóm rà soát (A, B1–B4, C). Sau đó 3 reviewer độc lập (D1–D3) cố bác bỏ từng phát hiện BLOCKER/MAJOR. Báo cáo này chỉ giữ lại các phát hiện **đã được kiểm chứng**, gộp theo nguyên nhân gốc.
> **Kết luận chung**: **FAIL**. Phần lớn story ghi Done/Complete chưa đạt tiêu chí nghiệm thu. Có 4 nguyên nhân gốc mức BLOCKER.

---

## 1. Bảng đối chiếu Plan ↔ Kết quả

| ID | Nhóm việc (plan §2) | Người làm theo plan | Người làm thực tế | Tiêu chí hoàn thành | Kết quả | Phát hiện thô (B/M/m) | Sau kiểm chứng |
|---|---|---|---|---|---|---|---|
| A | Nhất quán nội bộ tài liệu plan | Antigravity `agy` | **Claude subagent**: agy chạy 3 lần, cả 3 lần đều bị chế độ headless tự từ chối quyền `command`, lần lượt ở lệnh shell, `find` và `sed` | Danh sách mâu thuẫn, có file:dòng cả hai phía | ĐẠT | 2 / 16 / 11 | D3: 13 xác nhận, 5 xác nhận một phần, 0 bác bỏ |
| B1 | Code ↔ plan: nền tảng nhân vật | Claude subagent | Claude subagent | Bảng story → bằng chứng → kết luận | ĐẠT | 1 / 13 / 7 | D1: 18/27 xác nhận, 9 một phần, 0 bác bỏ (gộp B1+B2) |
| B2 | Code ↔ plan: combat & boss | Claude subagent | Claude subagent | Như B1 | ĐẠT | 0 / 13 / 6 | (gộp ở D1) |
| B3 | Code ↔ plan: item, kho đồ, chế tạo, kinh tế, tiến trình | Claude subagent | Claude subagent | Như B1 | ĐẠT | 2 / 13 / 7 | D2: 14 xác nhận, 1 một phần |
| B4 | Code ↔ plan: mạng, thế giới, tài khoản, UI | Claude subagent | Claude subagent | Như B1 | ĐẠT | 1 / 16 / 5 | D2: 2 bị **bác bỏ** (B4-4 Iris, B4-12 Sanctuary), 2 hạ mức (B4-1, B4-2) |
| C | Tính xác thực test & tiến độ | Claude subagent | Claude subagent | Danh sách test yếu, tuyên bố tiến độ sai | ĐẠT | 0 / 11 / 6 | D2: 1 bác bỏ một phần (C-14 lỗi `; Quit`), 3 hạ mức |
| D | Kiểm chứng phản biện | `reviewer` | 3 Claude subagent áp dụng quy tắc `reviewer.md` (session này nạp định nghĩa cũ trước khi PR #4 merge) | Mỗi phát hiện có CONFIRMED/REJECTED | ĐẠT | — | VERDICT: FAIL (cả 3) |
| E | Tổng hợp & đối chiếu | Claude (phiên chính) | Claude (phiên chính) | Mọi ID có trạng thái; báo cáo commit qua PR | ĐANG CHỜ: chưa merge PR; commit hash sẽ ghi vào `PROGRESS.md` | — | Reviewer lần 1: FAIL (8 lỗi), đã sửa |

**Lệch so với plan:**
1. A chuyển từ Antigravity sang Claude (ghi trong plan §2, mục "Điều chỉnh phân công").
2. D dùng general-purpose agent theo quy tắc `reviewer.md` thay cho agent `reviewer`.
3. Plan ghi 36 file test; thực tế có 38 file (2 file đuôi `_test.cpp` viết thường).

---

## 2. Nguyên nhân gốc đã kiểm chứng

Ký hiệu nguồn: A-n, B1-n … C-n là số phát hiện trong các báo cáo nhóm; D = đã qua kiểm chứng phản biện.

### BLOCKER

| # | Vấn đề | Bằng chứng chính | Nguồn |
|---|---|---|---|
| R1 | **Tiền tệ "Ash Shards" bị cấm** nhưng có mặt cả trong tài liệu lẫn code | `DECISIONS.md:96-97` ↔ `PACurrencyTypes.h:22`, `PABlacksmithComponent.cpp:141-145`, `expansion-economy/story-001:42`, `expansion-crafting/story-001:34`, `sprint-3.md:13`, `entities.yaml:2052`. Thêm: `zone-system.md:53` ghi "Void Shards" | A-1, B3-1, D2, D3 |
| R2 | **Thiếu thang độ hiếm 4 bậc cho kỹ năng/quyển trục** (Normal→Mythic). Skill book đang dùng thang trang bị | Chỉ có `EPAItemRarity` (`PAInventoryTypes.h:24`); `PAItemStaticDataAsset.cpp:154` (Dash = Rare); salvage cố định 5 (`PABlacksmithTypes.h:126-129`) ↔ `skill-progression-system.md:107-110` | B3-2, D2 |
| R3 | **ROADMAP trống nhưng PROGRESS ghi "ROADMAP đã duyệt, xong Giai đoạn 0"** | `ROADMAP.md:3-5` là placeholder. Commit `493a442` từng thêm ROADMAP 156 dòng "đã duyệt", rồi `d2943f4` revert vì "ngoài phạm vi". `PROGRESS.md:28,33,140` vẫn trích ROADMAP đó | A-2, D3 |
| R4 | **22/24 file test trong `Tests/` (thư mục gốc) chưa từng được biên dịch**, nhưng các story vẫn ghi "PASS" | `.uproject:7-8` chỉ có 1 module; `Build.cs`/`Target.cs` không thêm đường dẫn. Bằng chứng: `camera_lookahead_occlusion_test.cpp:36,50` kỳ vọng 1200, code là 1400 (`PAIsometricMovementMath.h:51`), nên test này nếu chạy sẽ fail. Có khoảng 20 story bị ảnh hưởng, gồm toàn bộ attr/ctrl/netcode/inventory. **Mức độ không thống nhất giữa các reviewer:** D1 giữ BLOCKER; D2 hạ B4-1 xuống MAJOR (vì vẫn có test biên dịch được trong `Source/`) và C-18 xuống MINOR; B3-6 vốn chỉ ở mức MAJOR. Báo cáo này giữ mức BLOCKER vì khoảng 20 story ghi "PASS" mà không có bằng chứng, trái quy định "CẤM báo cáo đã xong khi chưa có log test" trong CLAUDE.md | B1-1 = B3-6 = B4-1 = C-18, D1, D2 |

### MAJOR

| # | Nhóm | Vấn đề | Bằng chứng tiêu biểu | Nguồn |
|---|---|---|---|---|
| M1 | Hạ tầng | **Nhánh `main` không được bảo vệ** (yêu cầu "việc đầu tiên" trong CLAUDE.md) | API trả về "Branch not protected"; ruleset 24157209 `enforcement: disabled` | C-15, D2 |
| M2 | Hạ tầng | Job UE trên CI chỉ chạy khi kích hoạt tay, chưa chạy lần nào, sai đường dẫn `.uproject` (`tests.yml:65-66,78`). Cổng Postgres ở máy local biến lỗi assertion thành WARN và exit 0 (`run_headless_tests.sh:64-72`) | — | C-12, C-14, D2 |
| M3 | Quy ước | Tag class kiểu cũ `Class.<Name>` (bị cấm ở `DECISIONS.md:148`) có trong code, `DefaultGameplayTags.ini:2-5`, `DA_SkillBook_Dash.uasset` và các story | `PABaseCharacter.cpp:135,139`, `PAPaperdollComponent.cpp:146`, `PAItemStaticDataAsset.cpp:156` | A-12, B1-3, B3-5 |
| M4 | Quy ước | Gọi độ hiếm là "Tier/Bậc", dùng "Divine/Immortal" (trái `DECISIONS.md:17-19`). Không có độ hiếm thứ 6 | `PAInventoryTypes.h:27-31`, `blacksmithing-system.md:42-44`, `inventory-system.md:36-40`, `expansion-crafting/story-003:27` | A-3, B3-4, D3 |
| M5 | Phạm vi | **Có hệ thống ngoài GDD**: talent tree, skill point, respec (`PATalentTreeTypes.h`, `expansion-progression/story-002`). Nhánh talent tên "Chronomancer" trùng với class T3. **Ngược lại, hệ thống đã chốt chưa có**: class phụ, Class Level 1–20, tỉ lệ EXP 70/30, giao dịch thăng chức. Không có epic hay story nào cho chúng | `DECISIONS.md:49-73,159-160`; tr-registry TR-class-*, TR-skill-* không có story | A-4, B3-14, B3-15 |
| M6 | Lắp ráp runtime | **Code có nhưng không được gắn vào game**: các component BossAI, Stagger, PartBreaking, DashEvasion, Threat, PostureSync, Blacksmith; các subsystem GhostBody, LootDistribution, DifficultyScaling; Citadel autosave; Karma. Quét Content: không map nào tham chiếu class nào của project | Không nơi nào gọi `CreateDefaultSubobject` hay tham chiếu các class này trong Content; boss thật chạy AI tự viết (`PAStoneGolemBoss.cpp:104-130,236-300`) và chỉ tạo `LeashComponent` (`:29`); `PAMerchantComponent` chỉ có trên NPC (`PAWanderingSmuggler.cpp:17`); `OnPlayerEnterCitadel` không có caller (`PACitadelComponent.cpp:44-74`); cả hai map không tham chiếu class nào của `/Script/ProjectAscendant` (`verify-D2-B3B4C.md:2`). Kiến trúc boss AI lệch cả manifest (StateTree, `control-manifest.md:71`) lẫn GDD (BT+EQS, `boss-ai.md:13,34`); code dùng scoring tự viết chạy trên Tick (`PABossAIComponent.cpp:7,20-24`) | B2-1, B2-11, B3-8, B4-6,7,8,9,11 |
| M7 | Mạng | Boss dùng replication GAS `Mixed`, trái `DECISIONS.md:184-185` (yêu cầu `Minimal`) | `PABaseCharacter.cpp:81`, `PAStoneGolemBoss.h:22` | B1-12 = B2-9 = B4-5 |
| M8 | Server-authority | Server tin chi phí, tier và con trỏ do client gửi (`PABlacksmithComponent.cpp:872`→`Subsystem.cpp:496-500`; `:964`). RPC gọi lên component của NPC không có owning connection nên bị server bỏ qua (widget `PAMerchantShopWidget.cpp:114,136`). UI báo hoàn tất trước khi server xác nhận. Hướng ngắm không gửi lên server (`PABaseCharacter.cpp:270,326`). Lag-comp dùng đồng hồ client (`MeleeAttack.cpp:195,218`). Loot instanced dựa vào `IsNetRelevantFor`, mà Iris bỏ qua hàm này (`PALootDropletActor.cpp:31`) | — | B1-10, B2-6, B3-7, B3-8, B4-13, B4-14 |
| M9 | Thông số | Dash, combo, finisher và input buffer có nhiều giá trị lệch nhau: i-frame 0.28 / 0.20 / 0.25 / 200ms; combo reset 1.2s vs 0.60s; finisher 1.5s vs 3.0/1.2/2.0s; buffer 250ms vs 0.15s. Không có input buffer trong code production. Desperation Roll bị chặn | `dash-evasion.md:35,52,152` ↔ `PAGameplayAbility_Dash.h:31-35` ↔ `control-manifest.md:50`; `combat-system.md:41-45` ↔ `MeleeAttack.cpp:27` | A-5,6,7; B1-7; B2-2,3,4,8 |
| M10 | Combat | Tag `State.Staggered` / `State.FinisherPriority` chưa đăng ký. Posture break gắn `State.Broken`. `PostureImmune` chưa làm | `PABaseCharacter.cpp:407-416`, `PAPostureSyncComponent.cpp:139,307` | B1-4, B2-7 |
| M11 | Combat | Sát thương cận chiến bỏ qua công thức GDD: `UPADamageExecutionCalculation` không được dùng; dùng `BaseAttackDamage=20` hardcode | `MeleeAttack.cpp:398-406`, `.h:179`. Liên quan (suy luận, chưa chạy kiểm chứng): có 16 chỗ tạo `UGameplayEffect` tạm thời lúc runtime; Dash là ability `LocalPredicted` nhưng áp các GE tạm này mà không gate authority (`PAGameplayAbility_Dash.cpp:115,327-393`), nên dự đoán phía client có thể không khớp server | B2-5, B2-13 (B1-5 hạ MINOR) |
| M12 | GAS | Pipeline stamina/exhaustion không bao giờ chạy. MoveSpeed lấy từ biến thành viên, không lấy từ attribute GAS | `PAStaminaComponent.h:125-132` không có caller; `PABaseCharacter.cpp:41,317` | B1-6, B1-8 |
| M13 | Kinh tế | Salvage trang bị ra shard (GDD: ra quặng). Phí cường hoá lệch công thức. Giới hạn socket lệch GDD và lệch luôn generator. `OverflowStash` là TArray thô. Buyback dùng chung cho mọi người chơi. `ValidateInteraction` là code chết | `PABlacksmithTypes.h:131-142,154-200,298-315`; `PAInventoryComponent.h:232-233`; `PAMerchantComponent.h:137` | A-18, B3-3,9,10,11,12,13 |
| M14 | Mạng | Iris filter chỉ là thư viện tính khoảng cách, không gắn vào Iris. Section ini trỏ tới class không tồn tại (`DefaultEngine.ini:26-37`). Ghi chú: Iris **có** bật (B4-4 đã bị bác bỏ) | `PAIrisSpatialFilter.h:69-70` | B4-3, C-3 |
| M15 | Test | Có test kiểm tra mock tự định nghĩa trong chính file test, có test hiển nhiên đúng (`PACombatRegressionHardeningTests.cpp:30+`, `PAVanguardRuntimeWiringTests.cpp:40-53`, `PANetworkReplicationTests.cpp:106-124`). Không có test replication server + 2 client như `DECISIONS.md:186` yêu cầu | — | B2-10, B4-15, C-1,2,3 |
| M16 | Hình ảnh | AnimBP và AnimSequence chỉ là vỏ rỗng (`ABP_Vanguard`, `ABP_StoneGolem` chỉ có node Sink). Palette swap của NPC chưa làm. Chỉ 12/16 class có crest/tabard, test và asset | `PAPaperdollComponent.cpp:102-133` | B1-2, B1-13, B1-14, A-11 |
| M17 | Input/UI | Dùng input kiểu cũ (`IsInputKeyDown`/`BindKey`, `PABasePlayerController.cpp:78-81,117-124`). Không dùng CommonUI như manifest yêu cầu | `control-manifest.md:91,148` | B1-11, B4-17 |
| M18 | Gameplay | Hình phạt khi chết luôn áp kiểu PvE, kể cả khi chết do PvP và cả với boss. Không rơi shard. Model Karma có nhưng không được dùng | `PABaseCharacter.cpp:376-396` | B4-9 |
| M19 | Truy vết | TR-ID lệch nhau (18 ID có trong story mà không có trong registry, 19 ID theo chiều ngược lại). Số liệu trong `requirements-traceability.md` sai. Tầng Iris có 3 phiên bản. Save struct thiếu class phụ. Trạng thái giữa story, EPIC và sprint-status lệch nhau. Các thông số khác lệch giữa tài liệu (HP scaling 0.45/0.50, khoảng cách tương tác 250/300cm, pool FCT 64/50) | `tr-registry.yaml`, `architecture.md:268,567`, `control-manifest.md:22,74,94` | A-9,10,13,16,17; B4-16 |
| M20 | Backend/QA | Test Postgres chỉ kiểm tra schema tự viết trong file test, không có backend hay migration thật. Giao dịch thăng chức không kiểm `item_def_id` (`DECISIONS.md:160`). Validator GDD có vùng mù: không quét story, yaml hay `docs/architecture` | `test_backend_postgres.py:57-103,205-225`; `validate_gdd_consistency.py:201-204` | C-9, C-11 |
| M21 | Auth (tiềm ẩn) | Thông tin đăng nhập hardcode, mật khẩu lưu dạng rõ trong bộ nhớ. Hiện chỉ là stub phía client nên chưa có rủi ro thực | `PAAccountSubsystem.cpp:15-22,93` | B4-20 |

### Đã bị bác bỏ hoặc hạ mức khi kiểm chứng

- **B4-4 "Iris không bật": BÁC BỎ.** Engine 5.8.2 build có Iris, và cvar `net.Iris.UseIrisReplication=1` có hiệu lực. Chỉ còn một key ini thừa (MINOR).
- **B4-12 "Sanctuary không chặn tấn công": BÁC BỎ.** Tấn công bị chặn qua `ActivationBlockedTags` (`MeleeAttack.cpp:136-140`, `Finisher.cpp:36-40`).
- **C-14 "lỗi `; Quit` cắt lệnh": BÁC BỎ phần này.** Engine tự tách ExecCmds theo dấu `;`.
- **Hạ xuống MINOR:**
  - B1-5: code chết.
  - B1-9: 1400 nằm trong khoảng 1000–1400 của GDD.
  - B2-12: phụ thuộc M6.
  - B4-2 / C-4: token chưa được dùng ở đâu.
  - C-17: lỗi ghi chép sprint-status.
  - C-18: file test có tồn tại, chỉ là chưa được biên dịch (gộp vào R4).
  - A-8: `DECISIONS.md:80` đã chọn `itemization.md`.
  - A-14, A-15.
- **Hạ từ BLOCKER xuống MAJOR theo D2:** B4-1 (xem phần ghi chú ở R4).

### MINOR (tóm tắt)

- Phiên bản engine ghi 5.7 trong ADR, `architecture.md`, `control-manifest.md`.
- Trong tài liệu còn tham chiếu tới GDD không tồn tại (`world-bosses`, `hidden-classes`, `pvp-wanted-system`, `apothecary-system`, `reincarnation`). Thiếu `expansion-progression/EPIC.md`.
- Tên tiếng Việt của class không thống nhất giữa các tài liệu.
- Link hỏng do đường dẫn thiếu `ProjectAscendant/`.
- Ngày sprint 5–7 nằm trong tương lai nhưng story đã ghi hoàn thành.
- Một số thông số nhỏ lệch nhau: làm tròn vàng, lịch sử vị trí 50Hz vs 100Hz, ngưỡng wanted `<=` vs `<`.
- `PROGRESS.md` tự mâu thuẫn về ExitCode và số file.

---

## 3. Đề xuất sửa (chờ chủ dự án duyệt — mỗi mục là một PR riêng)

**Nhóm 1 — Có thể làm ngay vì không đổi thiết kế:**
1. Bật branch protection cho `main` (M1). Đây là thay đổi setting trên GitHub, cần chủ dự án xác nhận.
2. Đưa 22 test trong `Tests/` vào module `Source/ProjectAscendant/` và sửa các test fail. Riêng test camera sẽ được sửa theo 1400 vì giá trị này nằm trong khoảng 1000–1400 của GDD (B1-9). Ghi lại trạng thái thật của các story đang tuyên bố PASS (R4).
3. Sửa CI: đường dẫn `.uproject`, phân tích kết quả UE, và không biến lỗi Postgres thành PASS ở máy local (M2).
4. Mở rộng validator: quét story, yaml và `docs/architecture`; thêm kiểm tra Ash Shards/Divine/tag trong story (M20).
5. Sửa trạng thái trong `PROGRESS.md`, story, EPIC và `sprint-status.yaml` cho khớp thực tế (R3 phần ghi chép, M19).

**Nhóm 2 — Sửa cho đúng DECISIONS đã chốt (không đổi quyết định):**
6. Đổi Ash Shards thành `item_skill_shard` trong code, tài liệu và registry (R1). Cần chủ dự án xác nhận một điểm: `inventory-system.md:49,62` coi `item_skill_shard` là vật phẩm, còn `merchant-economy.md:13,65` coi là tiền tệ (giới hạn 99.999). Nếu không có chỉ định khác, sẽ làm theo `merchant-economy.md`.
7. Thêm enum độ hiếm kỹ năng 4 bậc và tách khỏi thang trang bị; sửa salvage skill book theo tỉ lệ 1/3/8/25 (R2).
8. Đổi tag sang `Class.Line.<Nhánh>.<Class>` (M3); bỏ cách gọi "Tier/Bậc/Divine" cho độ hiếm (M4).
9. Chuyển replication của boss sang `Minimal` (M7).
10. Sửa các lỗi server-authority: server tự tính chi phí, kiểm tra quyền sở hữu, đưa RPC lên actor của người chơi (M8).

**Nhóm 3 — Cần chủ dự án quyết định trước:**
11. ROADMAP: khôi phục bản ở `493a442` hay viết lại? Giai đoạn hiện tại là gì (R3)?
12. Talent tree, skill point, respec: giữ lại (và bổ sung GDD + DECISIONS) hay gỡ bỏ (M5)?
13. Với dash, combo và finisher, tài liệu nào là chuẩn: GDD, control-manifest hay story (M9)?
14. Lập epic cho hệ thống class phụ và thăng chức (M5).
15. Thứ tự gắn các hệ thống "có code nhưng chưa vào game" (M6), cùng M10–M18. Đây là công việc roadmap, nên phụ thuộc vào câu trả lời ở mục 11.
