# Kế Hoạch: Vòng Chơi Được + Thực Thi 27 Lựa Chọn Duyệt (2026-10-10)

> **Nguồn:** lựa chọn của chủ dự án lưu lúc 2026-10-10 11:11 UTC trên trang "Ascendant Owner Approvals" (bản sao ở §1); chỉ thị trong terminal cùng ngày: *"chỉ có phần BE không có UI/UX cũng chả có tác dụng gì cũng chả test được BE có hoạt động đúng không"*.
> **Đổi ưu tiên:** câu `i-next` từng chọn "test replication thật" được **chuyển xuống sau**. Ưu tiên số 1 giờ là một vòng chơi được để chủ dự án tự chạy thử, kể cả mở 2 client.
> **Quy trình mỗi PR:** theo CLAUDE.md — một việc một PR, chạy đủ cổng, reviewer phản biện, merge khi PASS, xác nhận `git ls-remote`, ghi vào `PROGRESS.md`.

## 1. Bản sao lựa chọn đã lưu

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

## 2. Luồng Z — Vòng chơi được (ưu tiên 1)

Mục tiêu: chủ dự án chạy `run_game.sh` và đi hết vòng sau: **Đăng nhập / Đăng ký → Chọn nhân vật → Vào map Outpost → HUD (máu, stamina, posture) → đánh boss Stone Golem → mua/bán ở Smuggler, rèn ở Lò rèn → mở 2 client cùng vào một server**.

| Mục | Việc | Người làm | Build UE | Điểm dừng |
|---|---|---|---|---|
| Z1 | Spec UX (dùng skill `/ux-design` của CCGS) và mockup cho: Đăng nhập, Đăng ký, Chọn nhân vật, HUD, Shop, Forge, Menu tạm dừng. Phong cách theo `design/art/art-bible.md` và `combat-hud.md` | SA-ux | Không | 🛑 **Chủ dự án duyệt thẩm mỹ mockup** trước khi làm Z2 |
| Z2 | Luồng mở đầu: bật CommonUI; tạo map `L_FrontEnd` bằng script Python headless; màn Login/Register/Character Select theo mockup đã duyệt, chạy với account stub cục bộ (backend thật để Giai đoạn 6); đăng ký kiểm tra độ dài mật khẩu và không lưu mật khẩu dạng chữ thường | SA-ui | Có | — |
| Z3 | Trong game: tạo HUD lúc vào map (vitals, boss HUD, chữ sát thương); đặt Smuggler và một Lò rèn vào `L_VerdantFrontier_Outpost`; phím tương tác mở Shop/Forge qua `UPAServiceRequestComponent` | SA-ui | Có | — |
| Z4 | Bản chơi thử: `run_game.sh` có thêm chế độ `server` và `client` (1 server + 2 client trên cùng máy), kèm checklist chơi thử | SA-qa | Có | 🛑 **Chủ dự án chơi thử, duyệt cảm giác chơi** (`feel-core=playtest`) |
| Z5 | Test replication tự động thật (1 server + 2 client, multi-process) theo DECISIONS §11 | SA-net | Có | — (làm sau Z4) |

## 3. Luồng Y — Thực thi 27 lựa chọn (chạy song song với Z)

| Mục | Nội dung | Loại | Người làm |
|---|---|---|---|
| Y1 | Tài liệu và thông số: Ranger 0.30s; FOV 90 (đóng AC-2 của core-world/001); bán kính 300cm (sửa control-manifest); giá sửa đồ Rare 300 (sửa ví dụ blacksmithing); "điểm kỹ năng" đổi thành cấp kỹ năng Grimoire; Dragon Knight chỉ dùng Polearm (gỡ allowlist); cửa sổ Dash Cancel co theo tỉ lệ 0.35/0.45 (ghi là "chốt khi triển khai") | Tài liệu | SA-design |
| Y2 | `DECISIONS.md` §13 ghi các lựa chọn ngày 2026-10-10 | Tài liệu | Main |
| Y3 | ROADMAP (chủ dự án đã cho phép): hạ dòng 29 và 33 về `[~]`; thêm mục bảo vệ `main` có bằng chứng; NestJS → "chưa chốt (ADR-0006)"; ngày sprint 5–7 theo ngày commit. CLAUDE.md: phạm vi giai đoạn 0→7; quy tắc khi file mâu thuẫn (chạm DECISIONS/ROADMAP thì dừng hỏi, còn lại chọn theo file và ghi lại); agent được đổi `[~]`→`[x]` khi có bằng chứng; asset AI duyệt theo lô | Tài liệu | Main |
| Y4 | PvE chết rơi 100% Tàn Trang (sửa code + test + story); bản đồ của Smuggler bán bằng Tàn Trang | Code | SA-economy |
| Y5 | Boss Soul đúng GDD: 4 bộ phận khác nhau, đồ ra theo class; ghi 5.000 Gold vào GDD | Code | SA-economy |
| Y6 | Khung độ hiếm trong art-bible dùng bảng màu ở `inventory-system.md` §1; gỡ allowlist | Tài liệu | SA-design |
| Y7 | Đổi tên asset và nhãn "Divine" sang Legendary (script gallery + tên file art) | Asset/script | SA-art |
| Y8 | LFS từ giờ về sau: chuyển các `.uasset`/ảnh đang có sang LFS bằng commit mới, không viết lại lịch sử | Hạ tầng | SA-devops |
| — | `a-proxies=review`: chủ dự án tự xem `Art_Gallery` | — | Chủ dự án |
| — | `i-agy=run`: chủ dự án tự chạy lệnh mở rộng allow-list của agy; sau đó mới giao việc cho agy, có Claude reviewer kiểm | — | Chủ dự án |

## 4. Thứ tự

1. Z1 (spec + mockup) chạy song song với Y1, Y2, Y3, Y6 (chỉ tài liệu).
2. Khi chủ dự án duyệt mockup: Z2 → Z3 → Z4. Xen giữa là Y4, Y5, Y7 (đều cần build, chạy tuần tự trên cây làm việc chính).
3. Y8 (LFS) làm khi không có PR nào đang mở, vì nó chạm tới rất nhiều file.
4. Z5 sau Z4.

## 5. Điểm dừng

- Duyệt mockup UI (Z1), chơi thử và duyệt cảm giác chơi (Z4).
- Mọi asset sinh bằng AI: duyệt theo lô (Y3, `a-aiasset=batch`).
- Một cổng không qua sau 3 lần thử.
