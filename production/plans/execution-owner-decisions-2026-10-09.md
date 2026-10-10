# Kế Hoạch Thực Thi: Quyết Định Của Chủ Dự Án Sau Rà Soát (2026-10-09)

> **Nguồn**: lựa chọn chủ dự án lưu lúc 2026-10-09 11:49 UTC trên trang "Ascendant Review Decisions" (bản sao nguyên văn ở §1.1); báo cáo `production/qa/review-plan-vs-code-2026-10-09.md`; ghi nhận chính thức ở `production/DECISIONS.md` §12.
> **Quy trình mỗi PR**: theo CLAUDE.md. Mỗi đầu việc một nhánh và một PR; chạy đủ các cổng; reviewer phản biện; chỉ merge khi PASS; xác nhận bằng `git ls-remote`; ghi vào `PROGRESS.md`.

## 1. Quyết định

| Mục | Lựa chọn | Nguồn |
|---|---|---|
| Nhóm 1 (đề xuất 1–5) | Làm tất cả | Chủ dự án |
| Nhóm 2 (đề xuất 6–10) | Làm tất cả | Chủ dự án |
| ROADMAP (đề xuất 11) | Khôi phục bản ở commit `493a442` | Chủ dự án |
| Talent tree (đề xuất 12) | Gỡ talent tree, skill point và respec của talent. **Giữ** respec kỹ năng mà GDD định nghĩa (`skill-progression-system.md:210`, `foundational-classes.md:445`) | Chủ dự án |
| Thông số dash/combo/finisher (đề xuất 13) | Giữ giá trị trong code; sửa tài liệu theo code. **Cảm giác chơi vẫn cần chủ dự án duyệt** | Chủ dự án |
| Epic class phụ và thăng chức (đề xuất 14) | Lập sau khi chốt ROADMAP, ở Giai đoạn 2B của ROADMAP | Chủ dự án |
| Hệ thống có code nhưng chưa gắn vào game (đề xuất 15: M6, M10–M18) | **Hoãn**. Các hệ thống này thuộc công việc roadmap (Giai đoạn 1, 1.5 và 2B), sẽ làm theo thứ tự ROADMAP sau đợt này | Hệ quả của đề xuất 11 |
| `item_skill_shard` | Là tiền tệ, theo `merchant-economy.md:13,65` | **Mặc định** của đề xuất 6; chủ dự án không ghi chú khác |

### 1.1 Bản sao lựa chọn đã lưu

```
g1-1..g1-5 = do · g2-6..g2-10 = do · q-roadmap = restore · q-talent = remove
q-source = code · q-dualclass = after-roadmap · notes = (trống) · savedAt = 2026-10-09T11:49:41Z
```

## 2. Phân loại & phân công

Ký hiệu người làm: **Main** = phiên Claude chính; **SA** = Claude subagent (general-purpose); **RV** = subagent phản biện (theo quy tắc trong `reviewer.md`).

| PR | Đầu việc | Loại | Người làm | Build UE | Phụ thuộc | Tiêu chí hoàn thành |
|---|---|---|---|---|---|---|
| X1 | Khôi phục `ROADMAP.md` từ `493a442` | Tài liệu | Main | Không | — | Trùng khớp từng byte với `493a442` |
| X1b | Plan này | Tài liệu | Main | Không | — | RV PASS |
| X2 | Ghi quyết định vào `DECISIONS.md` §12 | Tài liệu | Main | Không | X1b | Đúng các lựa chọn ở §1.1; không đổi quyết định cũ |
| X3 | Bảo vệ nhánh `main` (đề xuất 1) | Cấu hình GitHub | Main | Không | — | Ruleset `active` gồm: chặn xoá nhánh, chặn force push, bắt buộc PR, bắt buộc check có tên đúng **"Fast Gates (GDD & Backend QA)"**; đã kiểm chứng bằng API |
| X4 | Sửa CI (đề xuất 3) | Hạ tầng | SA | Không | — | Đường dẫn `.uproject` đúng; job UE đọc kết quả test; ở máy local, lỗi Postgres không còn bị biến thành PASS; không đổi tên job "Fast Gates" |
| X10 | Sửa tài liệu theo giá trị trong code (dash/combo/finisher) | Tài liệu | SA | Không | Quyết định ở §1 | Mọi tài liệu khớp code; có bảng trước/sau; GDD gate PASS |
| X12 | Đưa 22 test trong `Tests/` vào module và sửa test fail (đề xuất 2) | Code test | SA | Có | X4 | Test được biên dịch và chạy; test camera dùng 1400 (B1-9); ghi lại trạng thái thật của từng story có liên quan (R4) |
| X5 | Ash Shards → `item_skill_shard` (đề xuất 6) | Code + tài liệu | SA | Có | X2, X12 | Không còn "Ash Shards"/`AshShards`, trừ chỗ ghi lịch sử; sửa `inventory-system.md:49,62` cho khớp; mọi test PASS |
| X6 | Thang độ hiếm kỹ năng 4 bậc (đề xuất 7) | Code | SA | Có | X5 | Có enum Normal→Mythic riêng; skill book dùng enum này; salvage theo 1/3/8/25; có test mới trong module `Source/` |
| X7 | Tag `Class.Line.*`; bỏ Tier/Bậc/Divine khỏi tên độ hiếm (đề xuất 8) | Code + tài liệu | SA | Có | X6 | Không còn tag kiểu cũ `Class.<TênClass>` (vd. `Class.Vanguard`), `Class.TierX.*`, `Class.RankX.*` (`DECISIONS.md:148`); không còn "Tier N:" trong DisplayName của độ hiếm |
| X8 | Replication `Minimal` cho quái, boss và NPC (đề xuất 9) | Code | SA | Có | X7 | Mọi actor không phải người chơi có ASC đều dùng `Minimal` (`DECISIONS.md:185`); có test |
| X9 | Gỡ talent tree, skill point và respec của talent | Code + tài liệu | SA | Có | X8 | Không còn code hay story về talent; respec kỹ năng của GDD vẫn còn; mọi test PASS |
| X11 | Sửa lỗi server-authority (đề xuất 10) | Code | SA | Có | X9 | Server tự tính chi phí; kiểm tra quyền sở hữu; RPC đi qua actor của người chơi; UI chờ server xác nhận; có test |
| X13 | Mở rộng validator (đề xuất 4) | Hạ tầng | SA | Không | X5, X7, X10 | Validator quét story, sprint-status.yaml và `docs/architecture`; bắt được Ash Shards, Divine/Immortal, tag kiểu cũ trong story; có ca thử chứng minh bắt được lỗi; PASS trên repo đã sửa |
| X14 | Sửa trạng thái trong `PROGRESS`, story, EPIC, sprint-status (đề xuất 5) | Tài liệu | SA | Không | X12 | Mọi "PASS"/"Complete" đều có bằng chứng, hoặc được hạ về trạng thái thật. **Không** sửa `ROADMAP.md` (chỉ chủ dự án được sửa) |

**Mỗi PR có code** phải: build `ProjectAscendantEditor`, chạy `Tools/QA/run_headless_tests.sh --ue`, chạy GDD gate; sau đó RV phản biện; chỉ merge khi PASS.

**Antigravity:** không giao việc trong đợt này. Ở chế độ headless, agy vẫn bị từ chối quyền `command`. Sẽ giao lại khi chủ dự án mở rộng allow-list của agy.

## 3. Thứ tự

- **Đợt 1** (không build UE, chạy song song): X1, X1b, X2 (sau X1b), X3, X4, X10.
- **Đợt 2** (tuần tự trên cây làm việc chính, vì các PR sửa chung file và cần build UE): **X12 trước tiên** để các PR sau có test hồi quy thật, rồi X5 → X6 → X7 → X8 → X9 → X11.
- **Đợt 3**: X13, X14.
- Sau mỗi đợt: lập bảng đối chiếu plan ↔ kết quả.

## 4. Điểm dừng — hỏi chủ dự án

**Đã phát hiện, đang chờ trả lời (ghi lại, không tự sửa ROADMAP):**
1. **Cảm giác chơi** với các giá trị dash/combo/finisher trong code. Cần chủ dự án duyệt (DECISIONS §12, CLAUDE.md).
2. **ROADMAP Giai đoạn 0** ghi `[x]` ở `ROADMAP.md:29` (cổng Postgres) và `:33` (job UE), nhưng báo cáo rà soát cho thấy hai mục này chưa đạt (M2, M20). Ngoài ra Giai đoạn 0 không có mục bảo vệ nhánh, dù CLAUDE.md yêu cầu. Chủ dự án quyết: hạ các mục này về `[~]`/`[ ]`, hay thêm mục mới.
3. **Phạm vi giai đoạn**: CLAUDE.md và `DECISIONS.md:6` ghi "Giai đoạn 0 → 6", còn ROADMAP có thêm 1.5, 2B và 7 (Alpha).
4. **Quy tắc khi các file mâu thuẫn**: `ROADMAP.md:4` bảo dừng và hỏi; CLAUDE.md bước 2 bảo chọn theo file và ghi vào PROGRESS.
5. **`ROADMAP.md` tự mâu thuẫn**: dòng 3 nói chỉ chủ dự án được sửa, nhưng dòng 10 cho agent tự đổi `[~]` thành `[x]`.
6. **Backend**: `ROADMAP.md:136,139 (dòng trước Y3, `dec7374`)` chốt NestJS, trong khi ADR-0006 trong `architecture.md:766` chưa quyết.
7. **Art sinh bằng AI**: `DECISIONS.md:168` (nhân vật làm bằng AI + Aseprite) ↔ CLAUDE.md và `ROADMAP.md:119 (dòng trước Y3, `dec7374`)` (asset AI phải dừng và hỏi). Hệ quả là mọi asset nhân vật ở Giai đoạn 3–4 đều phải dừng hỏi.

**Điểm dừng chung:**
- Một cổng không qua sau 3 lần thử.
- Cần đổi một quyết định đã chốt ngoài §1.
- X3 khiến chủ dự án không merge được PR. Lúc đó báo lại, không tự gỡ bảo vệ.
