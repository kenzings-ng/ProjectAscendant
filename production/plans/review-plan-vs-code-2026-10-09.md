# Kế Hoạch Rà Soát: Tài Liệu Plan ↔ Code (2026-10-09)

> **Loại việc**: Rà soát read-only. Không sửa code/GDD trong đợt này.
> **Lý do**: `production/ROADMAP.md` đang ghi "toàn bộ công việc roadmap tạm dừng". Mọi đề xuất sửa sẽ được trình chủ dự án duyệt trước, mỗi đầu việc một PR.
> **Đầu ra**: `production/qa/review-plan-vs-code-2026-10-09.md` (báo cáo + bảng đối chiếu plan ↔ kết quả).

## 1. Phạm vi

| Nhóm tài liệu plan | Vị trí |
|---|---|
| Quyết định đã chốt | `production/DECISIONS.md` |
| GDD (22 file) | `design/gdd/` |
| Kiến trúc, ADR, manifest, traceability | `docs/architecture/` |
| Epic & story (16 epic) | `production/epics/` |
| Sprint & trạng thái | `production/sprints/`, `production/sprint-status.yaml` |
| Kế hoạch & nhật ký | `production/plans/`, `production/PROGRESS.md`, `production/ROADMAP.md` |

Code: `Source/ProjectAscendant/` (178 file .h/.cpp, 36 file test), `Tools/QA/`, `Scripts/`, `docker-compose.yml`.

## 2. Phân loại & phân công

Mọi subagent chạy **read-only**, mỗi phát hiện phải trích `file:dòng`, phân mức **BLOCKER / MAJOR / MINOR**, và ghi rõ "đã kiểm chứng" hay "suy luận".

| ID | Nhóm việc | Phạm vi | Người làm | Tiêu chí hoàn thành |
|---|---|---|---|---|
| A | Nhất quán nội bộ tài liệu plan | DECISIONS ↔ GDD ↔ ADR ↔ control-manifest ↔ tr-registry/traceability ↔ epics/sprints | **Antigravity (`agy -p --mode plan`)** | Danh sách mâu thuẫn giữa tài liệu, kèm file:dòng hai phía |
| B1 | Code ↔ plan: Nền tảng nhân vật | epics `foundation-attributes`, `foundation-controller`, `core-character`, `presentation-character-visual`; code `Character/`, `Animation/`, `Controller/`, attributes GAS | Claude subagent (general-purpose) | Bảng story → trạng thái khai báo → bằng chứng code/test → kết luận |
| B2 | Code ↔ plan: Combat & Boss | epics `core-combat`, `encounter-boss`; code `Combat/`, `AI/` | Claude subagent | Như B1 |
| B3 | Code ↔ plan: Item, kho đồ, chế tạo, kinh tế, tiến trình | epics `foundation-inventory`, `core-items`, `expansion-crafting`, `expansion-economy`, `expansion-progression`; code `Inventory/`, `Itemization/`, `Crafting/`, `Economy/`, `Progression/` | Claude subagent | Như B1 |
| B4 | Code ↔ plan: Mạng, thế giới, tài khoản, UI | epics `foundation-netcode`, `world-zone-auth`, `core-world`, `presentation-ui`; code `Network/`, `World/`, `Account/`, `UI/`, `Game/` | Claude subagent | Như B1 |
| C | Tính xác thực test & tiến độ | 36 file test UE, `Tools/QA/*`, schema/test Postgres, CI; đối chiếu số liệu trong `PROGRESS.md` / `sprint-status.yaml` | Claude subagent | Danh sách test yếu/giả, tuyên bố tiến độ sai lệch |
| D | Kiểm chứng phản biện | Lấy mọi phát hiện BLOCKER/MAJOR của A–C và kiểm lại | Claude subagent `reviewer` | Mỗi phát hiện được gắn CONFIRMED / REJECTED |
| E | Tổng hợp & đối chiếu plan | Gộp kết quả, lập bảng plan ↔ kết quả, viết báo cáo | Claude (phiên chính) | Mọi ID A–D có trạng thái; báo cáo được commit qua PR |

**Điều chỉnh phân công (2026-10-09):** A ban đầu giao cho Antigravity, nhưng chạy 3 lần đều thất bại: `agy -p` ở chế độ headless tự từ chối quyền `command` (lần lượt ở lệnh shell, `find`, `sed`), không có output. Chuyển A sang Claude subagent (general-purpose). Muốn dùng lại agy cần chủ dự án quyết định: thêm allow-rule cho lệnh đọc trong settings của Antigravity, hoặc cho phép `--dangerously-skip-permissions`.

## 3. Quy tắc chung cho mọi subagent

- Không sửa, stage, commit hay push bất cứ file nào trong repo. Chỉ được ghi file kết quả vào scratchpad được giao.
- Chỉ dùng thuật ngữ, class, tiền tệ, thang độ hiếm có trong `DECISIONS.md` / GDD. Không đề xuất hệ thống mới.
- Story ở trạng thái `done`/`Complete` mà không tìm thấy code hoặc test tương ứng → tối thiểu MAJOR.
- Code không truy được về story/GDD nào → ghi nhận là "code ngoài plan" (MINOR, trừ khi vi phạm DECISIONS).

## 4. Thứ tự

1. Chạy song song A, B1–B4, C.
2. D kiểm chứng các phát hiện BLOCKER/MAJOR.
3. E tổng hợp, đối chiếu plan, commit báo cáo qua PR, cập nhật `PROGRESS.md`.
4. Trình danh sách đề xuất sửa cho chủ dự án duyệt. Không tự sửa.
