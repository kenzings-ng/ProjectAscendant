# Kế Hoạch: Vòng Chơi Được + Thực Thi 26 Lựa Chọn Duyệt (2026-10-10)

> **Nguồn:**
> 1. 26 lựa chọn chủ dự án lưu lúc 2026-10-10 11:11 UTC trên trang "Ascendant Owner Approvals" (bản sao ở §1).
> 2. Chỉ thị trong terminal cùng ngày, nguyên văn: *"làm tới giai đoạn hiện tại chỉ có phần BE không có UI/UX cũng chả có tác dùng gì cũng chả test được BE có hoặc động đúng k"*.
> 3. Ba câu hỏi chủ dự án trả lời trong terminal cùng ngày (§1.1).
>
> **Đổi ưu tiên:** câu `i-next` đã chọn "test replication thật". Câu này được chuyển xuống thành Z5, làm sau Z4. Ưu tiên 1 giờ là một vòng chơi được để chủ dự án tự chạy thử.
>
> **Quy trình mỗi PR:** theo CLAUDE.md. Mỗi việc một PR, chạy đủ các cổng, reviewer phản biện, chỉ merge khi PASS, xác nhận bằng `git ls-remote`, ghi vào `PROGRESS.md`.

## 1. Bản sao lựa chọn đã lưu (26 câu)

```
feel-core=playtest  feel-dashcancel=scale  feel-ranger=0.30  feel-buffer=later  feel-fov=90
d-radius=300  d-skillpts=grimoire  d-dragonknight=remove  d-pvedrop=drop  d-smuggler=shards
d-repair=300  d-bosssoul=gdd
r-phase0=downgrade  r-protect=add  r-range=roadmap  r-conflict=mixed  r-edit=agent
r-nestjs=open  r-sprintdates=commit
a-rarityframe=palette  a-aiasset=batch  a-divine=rename  a-proxies=review
i-lfs=forward  i-agy=run  i-next=netest (đã đổi ưu tiên, xem trên)
notes = (trống)   savedAt = 2026-10-10T11:11:36Z
```

### 1.1 Trả lời thêm trong terminal (2026-10-10)

| Câu hỏi | Trả lời |
|---|---|
| Chèn luồng "vòng chơi được" lên trước thứ tự ROADMAP? | **Chèn lên trước.** Phần 2 client (Z4, Z5) gộp vào ROADMAP Giai đoạn 1.5 (`ROADMAP.md:54-57`). Login thật vẫn để Giai đoạn 6. Z1–Z3 nằm **ngoài** ROADMAP, đã được chủ dự án cho phép; việc lệch thứ tự được ghi vào DECISIONS §13 và PROGRESS |
| Bản chơi thử có đánh boss Stone Golem? | **Có, làm luôn.** Thêm ROADMAP Giai đoạn 1 (nối hình và animation cho Vanguard, `ROADMAP.md:42-45`) và M6 (gắn boss/stagger vào game) vào luồng Z |
| Phí 5.000 Gold khi rèn Boss Soul? | **Ghi 5.000 Gold vào GDD**, giữ đúng như code |

## 2. Luồng Z — Vòng chơi được (ưu tiên 1)

Mục tiêu: chủ dự án chạy `run_game.sh` và đi hết vòng sau:
**Đăng nhập / Đăng ký → Chọn nhân vật → Vào map → HUD (máu, stamina, posture) → đánh boss Stone Golem → mua/bán ở Smuggler, rèn ở Lò rèn → 2 client cùng vào một server.**

| Mục | Việc | ROADMAP | Người làm | Build UE | Điểm dừng |
|---|---|---|---|---|---|
| Z1 | Spec UX bằng skill `/ux-design` và mockup cho 6 màn hình: Login, Register, Chọn nhân vật, HUD (gồm boss HUD), Shop, Forge. Dùng tên và luồng trong `authentication-account-system.md:24-45,65`: `WBP_LoginScreen`, email + mật khẩu ≥ 6 ký tự, nút Dev Playtest `[Tester 1/2/3]`, chuỗi map `LoginMap → CharacterSelectMap → OpenWorldMap`. Phong cách theo `art-bible.md`; màu độ hiếm theo `inventory-system.md` §1. Menu tạm dừng chưa có nguồn GDD → ghi thành câu hỏi mở | Ngoài ROADMAP | SA-ux | Không | 🛑 **Chủ dự án duyệt thẩm mỹ mockup** trước Z2 |
| Z2 | Luồng mở đầu: bật CommonUI (TR-hud-001); tạo `LoginMap` và `CharacterSelectMap` theo GDD bằng script Python headless. Làm `WBP_LoginScreen`/Register/Character Select theo mockup đã duyệt, nối vào `UPAAccountSubsystem` và `PACharacterSelectWidget`/`PALoginWidget` sẵn có, kèm nút Dev Playtest. **Sửa lỗi bảo mật:** stub phải hash mật khẩu, không lưu dạng rõ (`PAAccountSubsystem.cpp:93`). Hash không phải tính năng mới | Ngoài ROADMAP | SA-ui | Có | — |
| Z3 | Trong game: tạo HUD khi vào map (vitals, boss HUD, chữ sát thương); đặt Smuggler và một Lò rèn vào map Outpost (`L_VerdantFrontier_Outpost`, đóng vai `OpenWorldMap` cho tới Giai đoạn 5); phím tương tác mở Shop/Forge qua `UPAServiceRequestComponent` | Ngoài ROADMAP | SA-ui | Có | — |
| Z3b | Đánh boss: ROADMAP Giai đoạn 1 (nối hình và animation cho Vanguard) và M6 (gắn BossAI/Stagger/PostureSync/Threat vào Stone Golem; đăng ký `State.Staggered` và các tag còn thiếu) | Giai đoạn 1 + M6 | SA-gameplay | Có | Đã duyệt trong §1.1. 🛑 Asset AI nếu có thì duyệt theo lô |
| Z4 | Bản chơi thử: `run_game.sh` đã có chế độ `server` (`-server -PORT=7777`) và `client`. Cần thêm: client tự kết nối tới `127.0.0.1:7777`, và lệnh mở 1 server + 2 client trên cùng máy. Kèm checklist chơi thử | Giai đoạn 1.5 | SA-qa | Có | 🛑 **Chủ dự án chơi thử, duyệt cảm giác chơi** (`feel-core=playtest`) |
| Z5 | Test replication tự động thật (1 server + 2 client, multi-process) theo DECISIONS §11 | Giai đoạn 1.5 | SA-net | Có | Giữ 🛑 ở `ROADMAP.md:57` |

## 3. Luồng Y — Thực thi 26 lựa chọn (song song với Z)

| Mục | Nội dung | Loại | Người làm |
|---|---|---|---|
| Y2 | **Làm trước Y1/Y4/Y5.** Thêm `DECISIONS.md` §13 ghi đủ 26 lựa chọn và §1.1, gồm cả các mục chỉ ghi nhận: `feel-buffer=later` (giữ nhãn "chưa triển khai", chốt khi làm input buffer), `a-proxies=review`, việc lệch thứ tự ROADMAP. Sửa `DECISIONS.md:6` từ "Giai đoạn 0 → 6" thành "0 → 7" theo `r-range` | Tài liệu | Main |
| Y1 | Thông số đã duyệt (**đã làm**, nhánh `docs/y1-approved-params`): Ranger 0.30s (`entities.yaml`, `foundational-classes.md` Q1); FOV 90 (`isometric-controller.md`); bán kính 300cm (control-manifest); giá Rare 300 (blacksmithing); "điểm kỹ năng" → cấp kỹ năng Grimoire; Dragon Knight chỉ dùng Polearm (gỡ allowlist); Dash Cancel ≈0.27–0.35s; registry Maces & Relics thêm Inquisitor/Seraph | Tài liệu | SA-design |
| Y3 | **ROADMAP** (chủ dự án đã cho phép):<br>• hạ `:29` và `:33` về `[~]`<br>• thêm mục "bảo vệ nhánh main" `[x]`, có bằng chứng từ X3<br>• `:136` và `:139`: NestJS → "chưa chốt (ADR-0006)"<br>• `:3`: theo `r-edit`, agent chỉ được đổi `[~]`→`[x]` khi có PR đã merge, log test và kết luận reviewer; phần nội dung còn lại vẫn chỉ chủ dự án được sửa<br>• ghi chú việc chèn luồng Z<br>**CLAUDE.md:**<br>• phạm vi "Giai đoạn 0 → 7"<br>• quy tắc khi file mâu thuẫn: chạm DECISIONS hoặc ROADMAP thì dừng hỏi; còn lại chọn theo file và ghi vào PROGRESS<br>• asset AI: gom mỗi lô vào **một trang xem trước**, mỗi lô vẫn phải dừng chờ chủ dự án duyệt | Tài liệu | Main |
| Y3b | Ngày sprint 5–7 theo ngày commit thật: `production/sprints/sprint-5.md:1`, `sprint-6.md`, `sprint-7.md` và `production/sprint-status.yaml`. Gỡ ghi chú X14 "Không tự đặt ngày mới" (`sprint-5.md:7` và tương tự) | Tài liệu | SA-producer |
| Y4 | PvE chết rơi 100% Tàn Trang (sửa code, test, story; `zone-system.md:88` là chuẩn). Bản đồ của Smuggler bán bằng Tàn Trang | Code | SA-economy |
| Y5 | Boss Soul đúng GDD: 4 bộ phận khác nhau (Sừng, Vảy đuôi, Giáp ngực, Cánh), đồ ra theo class (AC-5); ghi 5.000 Gold vào `blacksmithing-system.md` (§1.1) | Code + tài liệu | SA-economy |
| Y6 | `art-bible.md:127-128` dùng bảng màu ở `inventory-system.md` §1, không tự chọn màu mới; gỡ 2 mục allowlist | Tài liệu | SA-design |
| Y7 | Đổi tên asset và nhãn "Divine" sang Legendary (script gallery, tên file art, tham chiếu) | Asset/script | SA-art |
| Y8 | LFS từ giờ về sau, theo `i-lfs=forward`:<br>• giữ mọi pattern ở `.gitattributes:2-20`<br>• chạy `git add --renormalize .` trong **một commit mới**<br>• **cấm** `git lfs migrate import`, vì lệnh này viết lại lịch sử<br>• CI checkout có `lfs: true`<br>• kiểm tra quota GitHub LFS trước (hiện khoảng 53 MB, 5.803 file); nếu vượt mức miễn phí thì 🛑 dừng hỏi (dịch vụ trả phí) | Hạ tầng | SA-devops |
| — | `a-proxies=review`: chủ dự án tự xem `Art_Gallery` | — | Chủ dự án |
| — | `i-agy=run`: chủ dự án tự chạy lệnh mở rộng allow-list của agy. Sau đó Claude chỉ giao cho agy **việc kiểu tài liệu/rà soát**, và có Claude reviewer kiểm lại | — | Chủ dự án |

## 4. Thứ tự

1. **Y2 trước tiên**, vì nó ghi quyết định mà Y1, Y4, Y5 thực thi. Song song: Z1 (spec + mockup), Y3, Y3b, Y6.
2. Y1 merge sau Y2.
3. Trong lúc chờ duyệt mockup, các việc cần build UE chạy tuần tự trên cây làm việc chính: Y4 → Y5 → Y7 → Z3b.
4. Khi mockup được duyệt: Z2 → Z3 → Z4 (🛑 chơi thử) → Z5.
5. Y8 (LFS) chạy khi không còn PR nào đang mở.

## 5. Điểm dừng

- Duyệt mockup UI (Z1).
- Chơi thử và duyệt cảm giác chơi (Z4).
- Asset sinh bằng AI: duyệt theo lô qua trang xem trước.
- Quota LFS có thể phát sinh chi phí (Y8).
- `ROADMAP.md:57`, ở Giai đoạn 1.5.
- Một cổng không qua sau 3 lần thử.
