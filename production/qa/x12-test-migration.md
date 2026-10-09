# X12 — Đưa test ở `Tests/` vào module và sửa test (2026-10-09)

> **Việc**: X12 trong `production/plans/execution-owner-decisions-2026-10-09.md` (đề xuất 2; finding R4 = B1-1 = B3-6 = B4-1 = C-18).
> **Nhánh**: `test/x12-compile-root-tests`. Đã merge `origin/main` (gồm X12a, PR #14). Chưa mở PR.
> **Engine**: UE 5.8.2, Linux, `ProjectAscendantEditor Development`.

## 1. Kết quả

| Hạng mục | Trước X12 | Sau X12 (commit `7c2f61f`) |
|---|---|---|
| File `.cpp` ở `Tests/` (thư mục gốc) | 24, không được module nào biên dịch | 0 (22 file được `git mv` vào module, 2 bản trùng bị `git rm`) |
| Build `ProjectAscendantEditor` | — | `Result: Succeeded` |
| UE Automation (`run_headless_tests.sh --ue`) | Discovered=45, Passed=45 | **Discovered=67, Passed=67, Failed=0, Errors=0, QueueFinished=1, ExitCode=1** (ExitCode=1 là cách UE trên Linux thoát sạch khi dùng `-TestExit`) |
| Exit code của script | — | **0** (`ALL AUTOMATED GATES PASSED`) |
| GDD Consistency Gate | PASS | PASS |
| Backend Postgres Gate | — | WARN, bỏ qua ở máy local (không có Postgres), đúng như dự kiến |

**Bằng chứng**: `Tests/evidence/x12-7c2f61f-ue-automation.log`. Đây là **bản trích rút gọn**: log gốc dài 3162 dòng, khoảng 380 KB, nên chỉ giữ dòng tiêu đề (HEAD, ngày, exit code của script), command line, dòng "Found 67", toàn bộ 67 dòng `Result=` (đều là Success) và dòng queue-empty. File này nằm trong `.gitignore` (`*.log`), nên được thêm bằng `git add -f`.

Lịch sử: trước khi merge X12a, kết quả là 65/67. Hai test `PaperZDAnimBPIntegration` và `GASDashAbilityIntegration` fail vì hai lỗi trong code/config (§3). X12a đã sửa hai lỗi này trên `main` (`e0a2a5e`).

## 2. Bảng chuyển file và kết quả

Đường dẫn mới đều nằm dưới `Source/ProjectAscendant/Private/Tests/` (viết tắt là `PT/`). Cột "Story" là những story mà claim "Complete/PASS" của chúng dựa vào test này. Kết quả lấy theo lần chạy ở commit `7c2f61f`.

| File: cũ → mới | Tên test | Kết quả | Thay đổi và lý do | Story |
|---|---|---|---|---|
| `Tests/integration/character/boss_paperzd_aggro_test.cpp` → (xoá; bản giống hệt từng byte đã có ở `Private/Character/`) | `ProjectAscendant.Core.Character.BossPaperZDAggroIntegration` | PASS | `git rm` sau khi `cmp` xác nhận giống hệt | core-character/story-003 |
| `Tests/integration/combat/code_review_fixes_regression_test.cpp` → (xoá; bản giống hệt từng byte đã có ở `Private/Combat/`) | `ProjectAscendant.Core.Combat.ReviewFixesRegression` | PASS | `git rm` sau khi `cmp` xác nhận giống hệt | core-character/story-003 |
| `Tests/integration/character/paper2d_flipbooks_test.cpp` → `PT/integration/character/` | `...Core.Character.Paper2DFlipbooksIntegration` | PASS | Chỉ sửa flag | core-character/story-001 (pzd-001) |
| `Tests/integration/character/paperzd_animbp_test.cpp` → `PT/integration/character/` | `...Core.Character.PaperZDAnimBPIntegration` | PASS | Flag; LWC; `AnimInstance->Init(PaperZDAnimComponent)` để notify Hitbox tìm được nhân vật (trước đó `GetOwningActor()` trả null). Chỉ pass sau X12a (§3.1) | core-character/story-002 (pzd-002) |
| `Tests/integration/combat/contested_loot_finisher_test.cpp` → `PT/integration/combat/` | `...Foundation.Netcode.ContestedLootFinisherIntegration` | PASS | Flag; include `PABaseCharacter.h` (cho `TWeakObjectPtr`); WorldSubsystem cần outer là `UWorld`, có kiểm tra `GEngine` trước | foundation-netcode/story-004 |
| `Tests/integration/combat/gas_combo_finisher_test.cpp` → `PT/integration/combat/` | `...Core.Combat.GASComboFinisherIntegration` | PASS | Chỉ sửa flag | core-combat/story-002 (cmbt-001) |
| `Tests/integration/combat/gas_dash_ability_test.cpp` → `PT/integration/combat/` | `...Core.Combat.GASDashAbilityIntegration` | PASS | Sửa theo API hiện tại: `CanActivateDash` không còn tham số `bBlocked` (xem §6); `OnCooldownExpired()` đã bị xoá nên thay bằng truy vấn GE `Cooldown.Dash`. Thời lượng cooldown được so với hằng số **0.5s**, và việc hết hạn được **mô phỏng** bằng `RemoveActiveEffects`. Sửa setup: spawn nhân vật trong world (trước đây làm editor crash SIGSEGV), `GiveAbility` theo class, đăng ký AttributeSet vào ASC. Chỉ pass sau X12a (§3) | core-combat/story-001 (dash-001) |
| `Tests/integration/controller/camera_lookahead_occlusion_test.cpp` → `PT/integration/controller/` | `...Foundation.Controller.CameraLookAheadAndOcclusion` | PASS | **Arm length 1200 → 1400** theo quyết định của chủ dự án (code `PAIsometricMovementMath.h:51`; nằm trong khoảng 1000–1400 của GDD; B1-9). Flag; LWC | foundation-controller/story-003 |
| `Tests/integration/inventory/item_data_assets_test.cpp` → `PT/integration/inventory/` | `...Core.Items.ItemDataAssetsIntegration` | PASS | Chỉ sửa flag. Test đòi tag `Class.Vanguard`, tag này bị cấm ở `DECISIONS.md` §8; sẽ sửa cùng X7 | core-items/story-001 (item-001) |
| `Tests/integration/inventory/paperdoll_gas_equipment_test.cpp` → `PT/integration/inventory/` | `...Foundation.Inventory.PaperdollGASEquipmentIntegration` | PASS | Flag; `Set*` → `Init*` (`Set*` trên AttributeSet không có ASC gây fatal `CastChecked`). **AC-2 là tautology** (§4) | foundation-inventory/story-003 |
| `Tests/integration/world/map_blockout_camera_test.cpp` → `PT/integration/world/` | `...Core.World.MapBlockoutCameraIntegration` | PASS | Flag. **FOV 50 → 90**: không tài liệu nào quy định FOV; test chỉ khoá giá trị runtime. **Chờ chủ dự án** (§7) | core-world/story-001 (map-001) |
| `Tests/integration/world/sanctuary_leash_volume_test.cpp` → `PT/integration/world/` | `...Core.World.SanctuaryLeashVolumeIntegration` | PASS | Flag. Có 3 assert `TestTrue(..., true)`, đã ghi chú trong file (§4) | core-world/story-002 (map-002) |
| `Tests/unit/account/account_auth_test.cpp` → `PT/unit/account/` | `...Foundation.Account.AuthAndIdentityPipeline` | PASS | Flag; trước đây truyền `*static_cast<FSubsystemCollectionBase*>(nullptr)` (UB) nên đổi thành `FSubsystemCollection` thật; GameInstanceSubsystem cần outer là `UGameInstance` | world-zone-auth/story-004 (không story nào trích tên file) |
| `Tests/unit/combat/CombatFormulasTest.cpp` → `PT/unit/combat/` | `...Combat.Formulas.DamageAndPostureCalculations` | PASS | Chỉ sửa flag | foundation-attributes/EPIC.md (không phải story-002) |
| `Tests/unit/combat/attributes_replication_test.cpp` → `PT/unit/combat/` | `...Foundation.Combat.AttributesAndReplication` | PASS | Flag; `SetHealth/SetPosture` → `Init*` (trước đây editor crash vì fatal). **AC-3 là tautology** (§4) | foundation-attributes/story-001 |
| `Tests/unit/combat/damage_execution_calc_test.cpp` → `PT/unit/combat/` | `...Foundation.Combat.DamageExecutionCalculations` | PASS | Chỉ sửa flag | foundation-attributes/story-002 |
| `Tests/unit/combat/stamina_exhaustion_test.cpp` → `PT/unit/combat/` | `...Foundation.Combat.StaminaExhaustionPipeline` | PASS | Flag. **AC-3 case C: 30 → 31**: GDD `attributes-system.md:56,64` và story-003:40,56 yêu cầu Stamina **lớn hơn** 30%. **Thêm case C2**: đúng 30 thì phải là `false`. Case này pass **chỉ nhờ làm tròn float**; code đang dùng `>=` (§3.3, có TODO trong file) | foundation-attributes/story-003 |
| `Tests/unit/combat/threat_table_test.cpp` → `PT/unit/combat/` | `...Foundation.Combat.ContestedThreatTable` | PASS | Chỉ sửa flag | foundation-netcode/story-003 |
| `Tests/unit/controller/cursor_deprojection_aim_test.cpp` → `PT/unit/controller/` | `...Foundation.Controller.CursorDeprojectionAndAiming` | PASS | Flag; LWC. **AC-2: input lùi (-1,-1) → (0,-1)**: input tính theo màn hình (camera yaw 45°). Màn hình (-1,-1) ứng với hướng thế giới (0,-1), nên dot product ra -0.707 mới là đúng toán. Bấm S (0,-1) thì đi theo hướng thế giới (-0.707,-0.707), ngược hẳn hướng ngắm Đông Bắc | foundation-controller/story-002 |
| `Tests/unit/controller/screen_relative_movement_test.cpp` → `PT/unit/controller/` | `...Foundation.Controller.ScreenRelativeMovement` | PASS | Flag; LWC | foundation-controller/story-001 |
| `Tests/unit/inventory/fast_array_inventory_test.cpp` → `PT/unit/inventory/` | `...Foundation.Inventory.FastArrayGridInventory` | PASS | Chỉ sửa flag | foundation-inventory/story-001 |
| `Tests/unit/inventory/inventory_transaction_test.cpp` → `PT/unit/inventory/` | `...Foundation.Inventory.TransactionalRPCsAndSafeguards` | PASS | Chỉ sửa flag | foundation-inventory/story-002 |
| `Tests/unit/network/iris_ghost_body_test.cpp` → `PT/unit/network/` | `...Foundation.Netcode.IrisSpatialAndGhostBody` | PASS | Flag; đổi nullptr-collection thành collection thật; WorldSubsystem cần outer là `UWorld`, có kiểm tra `GEngine` trước | foundation-netcode/story-002 |
| `Tests/unit/network/net_lag_compensation_test.cpp` → `PT/unit/network/` | `...Foundation.Netcode.LocomotionLagCompensation` | PASS | Flag; LWC | foundation-netcode/story-001 |

Ý nghĩa các thay đổi lặp lại:
- **Flag**: `EAutomationTestFlags::ApplicationContextMask` không còn trong UE 5.8, nên đổi sang `EAutomationTestFlags_ApplicationContextMask`, đúng cách viết của các test trong `Private/`.
- **LWC**: các thành phần của `FVector`/`FRotator` là `double`, khiến lời gọi `TestEqual`/`TestNearlyEqual` bị mơ hồ giữa overload `float` và `double`. Đã bọc các tham số bằng `static_cast<double>`; **giá trị assert giữ nguyên**.
- Mọi test đều nằm trong `#if WITH_DEV_AUTOMATION_TESTS` và có tiền tố `ProjectAscendant.`.
- `Source/Tests/README.md` và `Tests/README.md` đã trỏ về `Source/ProjectAscendant/Private/Tests/`.

## 3. Lỗi code/config phát hiện qua X12

### 3.1 `Config/DefaultGameplayTags.ini` không nạp được tag nào: **đã sửa ở X12a (PR #14, `ec88a60`)**
File dùng `+TagList=`, nhưng UE đọc property `GameplayTagList`, nên không tag nào trong ini được đăng ký. Hậu quả: Dash không có cooldown và không bị chặn khi kiệt sức; AnimInstance không bao giờ bật `bIsDashing`/`bIsHurt`.

### 3.2 Dash không kiểm tra `Ability.Block.Dash`: **đã sửa ở X12a**

### 3.3 Còn mở: `CanRecoverFromExhaustion` dùng `>=` thay vì `>` (code ↔ GDD)
`PAStaminaComponent.cpp:66` viết `CurrentStamina >= MaxStamina * ThresholdPct`, trong khi GDD (`attributes-system.md:56,64`) và story-003:40,56 yêu cầu **vượt** 30%. Tại đúng mốc 30/100, `100 * 0.30f` ra `30.0000019f`, nên kết quả vô tình đúng với GDD. Với giá trị Max khác (khi tích float không bị làm tròn lên), code sẽ cho thoát kiệt sức ngay tại mốc. Đây là thay đổi code gameplay, ngoài phạm vi X12. Đã ghi TODO tại case C2 trong `stamina_exhaustion_test.cpp`.

## 4. Test không chứng minh được gì (tautology): vẫn giữ để biên dịch, đã ghi chú `X12 NOTE` trong file
- `attributes_replication_test.cpp` AC-3: test tự `Broadcast` delegate rồi tự kiểm tra là delegate đã chạy.
- `paperdoll_gas_equipment_test.cpp` AC-2: test tự cộng/trừ chỉ số, không trang bị item qua `UPAEquipmentComponent` hay GE.
- `gas_dash_ability_test.cpp` AC-4: tự `Broadcast` rồi `TestTrue(..., true)`.
- `gas_dash_ability_test.cpp` AC-3 (expiry): việc cooldown hết hạn được mô phỏng bằng `RemoveActiveEffects`, không chạy thời gian thật.
- `sanctuary_leash_volume_test.cpp`: 3 chỗ `TestTrue(..., true)` (AC-3 zone delegate, AC-2 `ExecuteReturnMovement`, AC-2 leash delegates).
- `stamina_exhaustion_test.cpp`: chỉ kiểm hàm thuần. Pipeline runtime không có caller (M12). Case C2 pass nhờ làm tròn float (§3.3).
- `camera_lookahead_occlusion_test.cpp` AC-2: chỉ kiểm phần toán. Look-ahead đang tắt mặc định (B1-9).

## 5. Trạng thái thật của các story (R4)

**Có test biên dịch và PASS** (xem cột ghi chú):

| Story | Test | Ghi chú |
|---|---|---|
| foundation-attributes/EPIC.md | DamageAndPostureCalculations (`CombatFormulasTest.cpp`) | Bằng chứng ở cấp EPIC, không gắn với story nào |
| foundation-attributes/story-001 | AttributesAndReplication | AC-3 là tautology; không có test replication server + 2 client (`DECISIONS.md` §11) |
| foundation-attributes/story-002 | DamageExecutionCalculations | Lưu ý M11: code melee không đi qua execution calc này |
| foundation-attributes/story-003 | StaminaExhaustionPipeline | Chỉ có hàm thuần; pipeline runtime không chạy (M12); còn lỗi `>=` (§3.3) |
| foundation-controller/story-001 | ScreenRelativeMovement | — |
| foundation-controller/story-002 | CursorDeprojectionAndAiming | Hướng ngắm không được gửi lên server (B1-10) |
| foundation-controller/story-003 | CameraLookAheadAndOcclusion | Arm length 1400; look-ahead tắt mặc định. Tài liệu vẫn ghi 1200 (§8) |
| foundation-inventory/story-001 | FastArrayGridInventory | — |
| foundation-inventory/story-002 | TransactionalRPCsAndSafeguards | — |
| foundation-inventory/story-003 | PaperdollGASEquipmentIntegration | AC-2 là tautology, nên binding GAS **chưa được chứng minh** |
| foundation-netcode/story-001 | LocomotionLagCompensation | — |
| foundation-netcode/story-002 | IrisSpatialAndGhostBody | Iris filter không gắn vào Iris (M14) |
| foundation-netcode/story-003 | ContestedThreatTable | — |
| foundation-netcode/story-004 | ContestedLootFinisherIntegration | — |
| core-character/story-001 | Paper2DFlipbooksIntegration | — |
| core-character/story-002 | PaperZDAnimBPIntegration | Chỉ pass sau X12a |
| core-character/story-003 | BossPaperZDAggroIntegration, ReviewFixesRegression | Vốn đã được biên dịch trước X12 |
| core-combat/story-001 | GASDashAbilityIntegration | Chỉ pass sau X12a; AC-4 là tautology; cooldown hết hạn chỉ là mô phỏng |
| core-combat/story-002 | GASComboFinisherIntegration | Thông số combo theo code (DECISIONS §12, đang chờ duyệt cảm giác chơi) |
| core-items/story-001 | ItemDataAssetsIntegration | Đòi tag cấm `Class.Vanguard` (X7) |
| core-world/story-001 | MapBlockoutCameraIntegration | **AC-2 CHƯA được tính là có bằng chứng**: FOV đang chờ chủ dự án quyết (§7) |
| core-world/story-002 | SanctuaryLeashVolumeIntegration | Có 3 assert `TestTrue(true)` |
| world-zone-auth/story-004 | AuthAndIdentityPipeline (+ PAAccountTests có sẵn) | — |

**Claim PASS chưa có bằng chứng đầy đủ:**
- core-world/story-001 AC-2: FOV chưa được quy định (§7).
- foundation-inventory/story-003 AC-2: binding GAS khi trang bị item (test là tautology).

**Ngoài phạm vi X12** (test đã nằm sẵn trong `Private/` và đã chạy trong 45 test cũ; X12 không đánh giá lại): encounter-boss/001–004, expansion-crafting/001–003, expansion-economy/001–003, expansion-progression/001–002, presentation-character-visual/001–007, presentation-ui/001–004, world-zone-auth/001–003. Các vấn đề M6, M15 và B1-2 vẫn áp dụng cho nhóm này. Việc cập nhật trạng thái thuộc X14.

## 6. Assert đã bỏ và nơi chuyển bằng chứng sang
- **Đã bỏ**: assert pipeline `CanActivateDash(100, false, /*bBlocked*/ true, 25)` → `false` (bản gốc ở `Tests/integration/combat/gas_dash_ability_test.cpp:51-52`; nay ghi chú ở `PT/integration/combat/gas_dash_ability_test.cpp:41`). Lý do: `FPADashPipeline::CanActivateDash` không còn tham số `bBlocked` (`PAGameplayAbility_Dash.h:42`).
- **Bằng chứng chuyển sang**: Test 2.1 case 4 trong cùng file, kiểm ở tầng GAS: thêm loose tag `Ability.Block.Dash` thì `CanActivateAbility` phải trả `false`. Case này PASS sau X12a.

## 7. Câu hỏi chờ chủ dự án
1. **FOV camera** (`map_blockout_camera_test.cpp:85-87`): GDD `isometric-controller.md`, story core-world/story-001 và `control-manifest.md` đều không quy định FOV. Test gốc đòi 50° nhưng không có nguồn; code không set FOV nên đang dùng mặc định của engine là 90°. Test hiện chỉ khoá giá trị 90 đang chạy. Cần chủ dự án/thiết kế chọn FOV. Đến khi có quyết định, **core-world/story-001 AC-2 không được tính là có bằng chứng**.

## 8. Việc chuyển cho X14 (tài liệu, X12 không sửa)
- `production/epics/foundation-controller/story-003-camera-lookahead-occlusion.md:34,46,81` và `design/gdd/isometric-controller.md:34,142` (giá trị mặc định) vẫn ghi `TargetArmLength = 1200`. Code và test dùng 1400 (theo quyết định B1-9).
- Cập nhật trạng thái các story theo §5.

## 9. Commit

| Commit | Nội dung |
|---|---|
| `c667c25` | `git mv` 22 file, `git rm` 2 bản trùng, sửa `Source/Tests/README.md` |
| `290fb1f` | Sửa để biên dịch được trên UE 5.8 (flag, LWC, include, API Dash) |
| `0c32e27` | Sửa setup test để không còn crash/ensure |
| `98051a5` | Sửa các kỳ vọng lỗi thời hoặc sai (camera 1400, stamina > 30%, input lùi, FOV) |
| `d468eff` | Bản đầu tiên của báo cáo này |
| `6a59832` | Merge `origin/main` (gồm X12a) |
| `7c2f61f` | Sửa theo reviewer: biên stamina, cooldown 0.5s, ghi chú tautology, kiểm tra `GEngine`, `Tests/README.md` |
| (commit này) | Bằng chứng `Tests/evidence/x12-7c2f61f-ue-automation.log` và báo cáo |
