# PROJECT ASCENDANT — ROADMAP

> **Trạng thái**: Đã duyệt bởi chủ dự án. Chỉ chủ dự án được sửa nội dung file này. Ngoại lệ (DECISIONS §13): agent được đổi `[~]` → `[x]` khi có đủ PR đã merge, log test và kết luận reviewer.
> **Nguồn chuẩn đi kèm**: `production/DECISIONS.md`. Mâu thuẫn giữa hai file → dừng và hỏi.
> **Quy trình thực thi**: theo `CLAUDE.md` (Autonomous Mode).

## Quy ước trạng thái

- `[x]` Đã xong **và có bằng chứng** (link PR đã merge, commit trên remote, log test).
- `[~]` Được báo là xong nhưng **chưa có bằng chứng**. Agent phải kiểm chứng trước, ghi bằng chứng vào `PROGRESS.md`, rồi mới đổi thành `[x]`. Không kiểm chứng được → đổi về `[ ]`.
- `[ ]` Chưa làm.
- 🛑 **Điểm dừng**: agent bắt buộc dừng và hỏi chủ dự án.

## Thứ tự thực hiện

```
0 → 1 → 1.5 → [2 (chờ duyệt thẩm mỹ) song song 2B] → 3 → 4 → 5 → 6 → 7
```

Giai đoạn 2B không cần duyệt thẩm mỹ, nên agent làm 2B trong lúc chờ chủ dự án duyệt ở Giai đoạn 2.

---

## Giai đoạn 0 — Nền móng repo

**Mục tiêu**: repo an toàn, build và test chạy được bằng lệnh.

- [x] Git LFS (`.gitattributes`) cho ảnh, `.aseprite`, `.atlas`, `.spine`, `.uasset`, `.umap`.
- [~] Cổng tự động: `validate_gdd_consistency.py`, `test_backend_postgres.py` (PostgreSQL thật), CI job `gates` (PR #1). _(Hạ về `[~]` ngày 2026-10-10 theo DECISIONS §13: chưa có bằng chứng, xem PROGRESS.md mục 2/M20.)_
- [x] Nhánh Stone Golem Spine: asset đã có trên main (`Content/art/characters/boss/spine/`). Cần kiểm chứng: Walk không trượt chân, Slam có squash tiếp đất, texture Filter = Nearest, không mipmap.
- [x] `.claude/settings.json` đúng cú pháp Claude Code và **đã chứng minh** chặn được lệnh cấm.
- [x] Hook `pre-push` được cài (`git config core.hooksPath Tools/git-hooks`) và đã chứng minh chặn push vào main.
- [~] Job `ue-tests` chuyển sang `workflow_dispatch`; UE test chạy local, log dán vào `PROGRESS.md`. _(Hạ về `[~]` ngày 2026-10-10 theo DECISIONS §13: chưa có bằng chứng, xem PROGRESS.md mục 3/M2.)_
- [x] Sửa lỗi Character Select: `PACharacterSelectTypes.cpp` load `ranger_pixel_spritesheet` và `arcanist_pixel_spritesheet` nhưng repo chỉ có `.png`, không có `.uasset`.
- [x] Bảo vệ nhánh `main`: ruleset 24157209 `active` (deletion, non_fast_forward, pull_request, required_status_checks "Fast Gates (GDD & Backend QA)") từ 2026-10-09; bằng chứng X3 trong PROGRESS.md. _(Thêm ngày 2026-10-10 theo DECISIONS §13.)_

**Xong khi**: mọi mục trên là `[x]`.

---

## Giai đoạn 1 — Nối hình vào Vanguard (M6.1 Runtime Wiring)

- [~] `Speed` / `bIsRunning` trong `UPAPaperZDAnimInstance`, flipbook Walk/Run 6 frame.
- [~] State Hurt / Stunned / Dead điều khiển bằng GameplayTag (`State.Hurt`, `State.Stunned`, `State.Dead`).
- [~] Tạo Sprite Socket tự động từ `vanguard_metadata.json` (`PAPaper2DSocketUtility`).
- [~] Test `ProjectAscendant.Character.VanguardRuntimeWiring` pass.
- [ ] Legacy Đợt 3: AnimBP mới, đổi đường dẫn trong `PABaseCharacter.cpp` và `PAStoneGolemBoss.cpp`, test pass, rồi chuyển `Content/art/characters/vanguard/` và `boss/` vào kho legacy (theo `legacy-art-inventory.md` mục 5).

**Kiểm chứng `[~]`**: chạy UE test local, dán log. Code đã có trên main (`PAPaper2DSocketUtility.cpp`).

---

## Giai đoạn 1.5 — Netcode

- [~] PIE 2 client + dedicated server, Iris bật (`net.Iris.UseIrisReplication=1`).
- [~] ASC người chơi ở chế độ Mixed; tag trạng thái replicate đúng.
- [~] Test `ProjectAscendant.Network` pass (`PANetworkReplicationTests.cpp`).
- [ ] 🛑 Chủ dự án tự chơi thử: 2 cửa sổ PIE, `Net PktLag=150`, `Net PktLoss=2`, cùng đánh một quái; Hurt/Dead hiển thị giống nhau ở cả hai bên.

---

## Giai đoạn 2 — Chốt phong cách nhân vật

- [x] Vanguard: phương án A "Iron Bastion", khiên sắt vuông (DECISIONS.md).
- [x] Nguồn art Hybrid (DECISIONS.md).
- [ ] Mood board: 🛑 chủ dự án chọn 2–3 game tham chiếu.
- [ ] Cập nhật `SPEC-ART V2` theo mood board (tỷ lệ 3.2–3.5 đầu, pivot 64,114, ramp 4 tông, 5 hướng + lật).
- [ ] Design sheet Vanguard 5 hướng, qua `pixel-review` ở 32×32 và 16×16.
- [ ] 🛑 Chủ dự án duyệt design sheet.

**Xong khi**: chủ dự án nói "chốt".

---

## Giai đoạn 2B — Hệ thống Class Chính / Phụ & Quyển trục (code, không cần art)

Làm song song với Giai đoạn 2. Mọi quy tắc lấy từ `DECISIONS.md` và `advanced-classes.md`.

- [ ] GameplayTags `Class.Line.<Nhánh>.<Class>` cho 16 class, khai báo trong `DefaultGameplayTags.ini`.
- [ ] `UClassPromotionScrollDefinition` (Data Asset): `TargetClassTag`, `AllowedSourceClassTags`, `RequiredMinRank`, `RequiredSourceClassLevel`, `RequiredQuestFlag`, điều kiện hai slot cho Apex.
- [ ] Component tiến trình class: slot Chính / Phụ replicated, lưu tiến trình từng class đã chơi, Class Level 1–20 song song Character Level 1–50.
- [ ] Quy tắc A1: đổi Chính ↔ Phụ tại Tòa Thành, clamp bậc Phụ theo bậc Chính.
- [ ] Quy tắc B1: Class Phụ 1 Active (slot F, không phụ thuộc vũ khí) + 1 Passive; kỹ năng cần vũ khí bị khóa.
- [ ] Server RPC thăng chức, kiểm tra cả hai slot, tiêu hủy quyển trục.
- [ ] Test tự động: thăng chức hợp lệ / không hợp lệ, clamp bậc, khóa kỹ năng sai vũ khí, **test ma trận** mọi cặp Chính × Phụ kiểm tra Passive không cộng dồn vượt giới hạn.
- [ ] Quyển trục MVP: bán cho NPC lấy vàng, Chợ Đen bán lại, phân rã thành Tàn Trang.

---

## Giai đoạn 3 — Bộ art mẫu (vertical slice)

Chỉ làm **một** tổ hợp, chạy trọn pipeline trước khi sản xuất hàng loạt.

- [ ] 1 Lower Body Master Rig (visual-004): Idle / Walk / Run / Dash / HitStun, 5 hướng.
- [ ] 1 Weapon Family `1H.Blade` (visual-005): combo 3 đòn + thế thủ, 5 hướng.
- [ ] Vanguard Idle + Crest + Tabard (visual-006), 5 hướng.
- [ ] Qua Art Gate (`pixel-review`), sửa bằng `pixel-fix`.
- [ ] Xuất bằng `pixel-export`, import Paper2D (Nearest, không mipmap, Pixels Per Unit thống nhất), **giữ nguyên tên asset của proxy** để không phải sửa code.
- [ ] Chạy lại script socket với metadata mới.
- [ ] Ghi nguồn gốc asset (AI / thủ công / pack) vào `THIRD_PARTY_ASSETS.md`.
- [ ] 🛑 Chủ dự án chơi thử với art thật và duyệt.

---

## Giai đoạn 4 — Nhân bản art & dọn dẹp

Khối lượng theo 5 hướng + lật (story-004 → 007), 16 class:

| Story | Nội dung | Khối lượng |
|---|---|---|
| visual-004 | 4 Lower Body Master Rigs | 420 frame |
| visual-005 | 7 Weapon Families | 560 frame |
| visual-006 | 16 class Idle + Crest + Tabard, cộng cánh Seraph | ~480 + 20 asset |
| visual-007 | Civilian Upper Body & Town Props | 255 asset |

- [ ] Sản xuất theo từng lô, mỗi lô qua Art Gate trước khi import.
- [ ] Sổ cái CC0: thẩm định pack Pending, thay 7 asset bị từ chối, rà `doficia/project-cordon-sprites`.
- [ ] Legacy Đợt 4: xóa 3 file DELETE_CANDIDATE trùng lặp, sửa `Scripts/setup_art_gallery.py` dòng 214/222/230.
- [ ] Asset legacy `REWORK`: chỉ xóa sau khi bản thay thế qua Art Gate.
- [ ] 🛑 Mọi asset license khác CC0 hoặc sinh bằng AI; mọi khoản mua asset pack.

---

## Giai đoạn 5 — Dựng thế giới (PCG)

- [ ] Tileset mặt đất 32×32 có autotile bitmask (`pixel-tileset`).
- [ ] Translucent Sort Policy = Sort Along Axis cho sắp xếp độ sâu isometric.
- [ ] Nạp asset môi trường M6 vào `PCG_WorldBiomeGraph`.
- [ ] Đường và thành bằng `PCG_CitadelRoadSplineGraph`.
- [ ] WFC ngục tối: để backlog, chỉ làm khi mọi mục trên xong.
- [ ] 🛑 Chủ dự án đi dạo map và nhận xét.

---

## Giai đoạn 6 — Backend & lưu dữ liệu

- [ ] Service backend (công nghệ **chưa chốt**, quyết qua ADR-0006 — DECISIONS §13) + PostgreSQL theo schema trong `DECISIONS.md` mục 9 (bảng `accounts`, `characters`, `items`).
- [ ] Client đăng nhập lấy JWT; dedicated server xác thực token.
- [ ] Chỉ dedicated server gọi API ghi dữ liệu (`FHttpModule`); client không bao giờ ghi trực tiếp.
- [ ] Test backend gọi vào **code service backend thật** (công nghệ chưa chốt, ADR-0006) (không chỉ câu SQL trong file test): chống dupe, hoán đổi ô đồ, thăng chức bằng quyển trục.
- [ ] Lưu khi thoát và lưu định kỳ.

---

## Giai đoạn 7 — Alpha

- [ ] Playtest với người ngoài. 🛑
- [ ] Profile hiệu năng bằng Unreal Insights.
- [ ] Đóng gói bản build.
- [ ] Kiểm tra license lần cuối; khai báo nội dung AI theo yêu cầu nền tảng phát hành.

---

## Sau MVP (không làm trước khi Alpha xong)

- Giao dịch giữa người chơi / Nhà đấu giá; quyển trục class ẩn khóa sau lần giao dịch đầu; chống bot (cấp tối thiểu, thuế giao dịch).
- Self-hosted runner cho UE test (chỉ khi repo private hoặc đã chặn PR từ fork).
