# Kế Hoạch Thực Thi: Quyết Định Của Chủ Dự Án Sau Rà Soát (2026-10-09)

> **Nguồn**: lựa chọn chủ dự án lưu ngày 2026-10-09 11:49 UTC trên trang "Ascendant Review Decisions"; báo cáo `production/qa/review-plan-vs-code-2026-10-09.md`.
> **Quy trình mỗi PR**: theo CLAUDE.md. Mỗi đầu việc một nhánh và một PR; chạy đủ cổng; reviewer phản biện; chỉ merge khi PASS; xác nhận bằng `git ls-remote`; ghi vào `PROGRESS.md`.

## 1. Quyết định của chủ dự án

| Mục | Lựa chọn |
|---|---|
| Nhóm 1 (đề xuất 1–5) | Làm tất cả |
| Nhóm 2 (đề xuất 6–10) | Làm tất cả |
| ROADMAP | Khôi phục bản ở commit `493a442` |
| Talent tree, skill point, respec | Gỡ bỏ khỏi code và story |
| Thông số dash/combo/finisher | Giữ giá trị trong code; sửa GDD, control-manifest và tr-registry theo code |
| Epic class phụ và thăng chức | Lập sau khi chốt ROADMAP, tức trong Giai đoạn 2B của ROADMAP |
| `item_skill_shard` | Không có ghi chú, nên theo mặc định: coi là tiền tệ như `merchant-economy.md:13,65` |

## 2. Phân loại & phân công

Ký hiệu người làm: **Main** = phiên Claude chính; **SA** = Claude subagent (general-purpose); **RV** = subagent reviewer (quy tắc `reviewer.md`).

| PR | Đầu việc | Loại | Người làm | Cần build UE | Phụ thuộc | Tiêu chí hoàn thành |
|---|---|---|---|---|---|---|
| X1 | Khôi phục `ROADMAP.md` từ `493a442` | Tài liệu | Main | Không | — | File trùng khớp `493a442`; GDD gate PASS |
| X2 | Ghi các quyết định ngày 2026-10-09 vào `DECISIONS.md` | Tài liệu | Main | Không | X1 | Mỗi quyết định có ngày và nguồn; không đổi quyết định cũ nào khác |
| X3 | Bật bảo vệ nhánh `main` (đề xuất 1) | Cấu hình GitHub | Main | Không | — | Ruleset `active`: chặn xoá nhánh, chặn force push, bắt buộc PR, bắt buộc check "Fast Gates"; kiểm chứng bằng API |
| X4 | Sửa CI (đề xuất 3) | Hạ tầng | SA-devops | Không | — | Đường dẫn `.uproject` đúng; đọc kết quả UE; ở máy local, lỗi Postgres không còn bị biến thành PASS |
| X5 | Ash Shards → `item_skill_shard` (đề xuất 6) | Code + tài liệu | SA-economy | Có | X2 | Không còn chuỗi "Ash Shards"/`AshShards` trong code và tài liệu, trừ chỗ ghi lịch sử; 45+ test PASS |
| X6 | Thang độ hiếm kỹ năng 4 bậc (đề xuất 7) | Code | SA-economy | Có | X5 | Có enum riêng Normal→Mythic; skill book dùng enum này; salvage 1/3/8/25; có test mới |
| X7 | Tag `Class.Line.*` và bỏ cách gọi Tier/Bậc/Divine cho độ hiếm (đề xuất 8) | Code + tài liệu | SA-gameplay | Có | X6 | Không còn tag `Class.<Name>` và chữ "Tier N:" trong DisplayName độ hiếm |
| X8 | Boss dùng replication `Minimal` (đề xuất 9) | Code | SA-gameplay | Có | X7 | Boss gọi `SetReplicationMode(Minimal)`; có test |
| X9 | Gỡ talent tree, skill point, respec | Code + tài liệu | SA-gameplay | Có | X8 | Không còn code hay story về talent; build và test PASS |
| X10 | Sửa GDD, manifest, tr-registry theo giá trị code (dash/combo/finisher) | Tài liệu | SA-design | Không | X2 | Mọi tài liệu khớp code; có bảng đối chiếu trước/sau |
| X11 | Sửa lỗi server-authority (đề xuất 10) | Code | SA-network | Có | X9 | Server tự tính chi phí; kiểm tra quyền sở hữu; RPC đi qua actor của người chơi; UI chờ server xác nhận; có test |
| X12 | Đưa 22 test trong `Tests/` vào module và sửa test fail (đề xuất 2) | Code test | SA-qa | Có | X11 | Test được biên dịch và chạy; số test tăng tương ứng; mọi test PASS |
| X13 | Mở rộng validator GDD (đề xuất 4) | Hạ tầng | SA-qa | Không | X5, X7, X10 | Validator quét story, yaml, `docs/architecture`; PASS trên repo đã sửa; có ca thử cho thấy validator bắt được lỗi |
| X14 | Sửa trạng thái trong `PROGRESS`, story, EPIC, sprint-status (đề xuất 5) | Tài liệu | SA-producer | Không | X12 | Mọi trạng thái "PASS" và "Complete" đều có bằng chứng, hoặc được hạ về trạng thái thật |

**Mỗi PR có code** đều phải: build `ProjectAscendantEditor`, chạy `Tools/QA/run_headless_tests.sh --ue`, chạy validator GDD; sau đó RV phản biện; chỉ khi PASS mới merge.

**Antigravity:** không giao việc trong đợt này. Ở chế độ headless, agy vẫn bị từ chối quyền `command` (xem `production/qa/review-plan-vs-code-2026-10-09.md` §1). Sẽ giao lại khi chủ dự án mở rộng allow-list của agy.

## 3. Thứ tự

- **Đợt 1** (song song, không cần build UE): X1 → X2, X3, X4, X10.
- **Đợt 2** (tuần tự trên cây làm việc chính, vì các PR này sửa chung file và đều cần build UE): X5 → X6 → X7 → X8 → X9 → X11 → X12.
- **Đợt 3**: X13, X14.
- Sau mỗi đợt: lập bảng đối chiếu plan ↔ kết quả.

## 4. Điểm dừng (hỏi chủ dự án)

- Một cổng không qua sau 3 lần thử.
- Việc sửa đòi hỏi đổi một quyết định đã chốt ngoài các mục ở §1.
- X3 khiến chủ dự án không merge được PR của chính mình. Lúc đó sẽ báo lại, không tự gỡ bảo vệ.
