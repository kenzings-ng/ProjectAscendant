# PROJECT ASCENDANT -- TIẾN TRÌNH THI CÔNG & NHẬT KÝ TỰ VẬN HÀNH (PROGRESS.md)

> **Dự án**: Project Ascendant (2.5D Isometric HD-2D Dark Fantasy Action RPG / MMO)  
> **Chế độ**: CHẾ ĐỘ TỰ VẬN HÀNH (Autonomous Mode)  
> **Cập nhật lần cuối**: 2026-09-29  
> **Nhánh hiện tại**: `fix/audit-autonomous-setup` (Chờ người dùng duyệt Pull Request)

---

> [!CAUTION]
> **CẢNH BÁO VI PHẠM NGUYÊN TẮC TỰ VẬN HÀNH (ĐÃ PHÁT HIỆN & KHẮC PHỤC)**:  
> Việc tự ý thêm 2 class không được duyệt (`Void Weaver` và `Oracle`) để ép đủ số lượng class trong commit trước đó là **vi phạm nghiêm trọng** quy tắc: *"Thay đổi quyết định đã chốt phải dừng lại và hỏi người dùng"*.  
> Toàn bộ các tài liệu GDD (`advanced-classes.md`), `DECISIONS.md`, và validator script đã được phục hồi chính xác theo danh sách 16 class đã được duyệt chính thức.

---

## 1. Trạng Thái Các Cổng Tự Động & Kiểm Thử (Gate & Test Status)

*Phân định minh bạch giữa kết quả ĐÃ CHẠY THẬT và trạng thái CẤU HÌNH / CHỜ RUNNER:*

| Hạng mục / Cổng kiểm tra | Trạng thái thực tế | Môi trường kiểm tra | Chi tiết & Ghi chú |
| :--- | :--- | :--- | :--- |
| **GDD Consistency Validator** (`validate_gdd_consistency.py`) | **ĐÃ CHẠY THẬT - PASS** | Local Python 3.11 | Quét 22/22 file GDD: 0 Lỗi, 0 Cảnh báo. Cấm tiệt Void Weaver / Oracle, xác thực 16 class đã duyệt. |
| **Real PostgreSQL Test Suite** (`test_backend_postgres.py`) | **SẴN SÀNG / ĐÃ CHUẨN HÓA** | CI Service / Docker Compose | Loại bỏ 100% SQLite & threading.Lock. Sử dụng PostgreSQL thật với 2 connection riêng biệt (`SELECT ... FOR UPDATE` row lock, `DEFERRABLE INITIALLY DEFERRED` slot swap). |
| **CI Job `gates`** (GDD + Postgres) | **SẴN SÀNG CHẠY NGAY** | GitHub Actions (`ubuntu-latest`) | Tự động kích hoạt khi push/PR với `postgres:16-alpine` service container. |
| **CI Job `ue-tests`** (Unreal Engine Automation) | **CHƯA CHẠY TRÊN CI** | GitHub Actions (`self-hosted`) | **UE tests chưa chạy trên CI** do đang chờ cấu hình self-hosted runner có gắn nhãn `[self-hosted]`. Tuyệt đối không ghi PASS khi chưa chạy thật. |
| **Lộ trình sản xuất** (`production/ROADMAP.md`) | **TẠM DỪNG TOÀN BỘ** | N/A | Dừng mọi công việc thuộc roadmap (từ Giai đoạn 0 trở đi). File `ROADMAP.md` để trống chờ người dùng cung cấp. |

---

## 2. Nhật Ký Đầu Việc: Nhánh `fix/audit-autonomous-setup`

- **Mục tiêu**: Khắc phục toàn bộ các sai lệch phát hiện qua audit, đưa hệ thống về đúng thiết kế được duyệt, chuẩn hóa test database PostgreSQL thật, tách job CI và mở Pull Request (chờ người dùng duyệt, không merge).
- **Các nội dung đã thực hiện**:
  1. **Khôi phục Cây chuyển chức 16 Class chuẩn xác**:
     - `production/DECISIONS.md`: Khôi phục chuẩn 15 Class nhánh + 1 Apex Class (`God Slayer`), xóa bỏ hoàn toàn `Void Weaver` và `Oracle`.
     - `design/gdd/advanced-classes.md`: Cập nhật bảng và cây chuyển chức khớp 100% với 16 class đã duyệt.
     - `Tools/QA/validate_gdd_consistency.py`: Cập nhật danh sách `APPROVED_CLASSES`, bổ sung regex chặn đứng các class không được duyệt (`Void Weaver`, `Oracle`). Kết quả chạy thật: **PASS 22/22 files**.
  2. **Viết lại Bộ Test Backend với PostgreSQL thật (`Tools/QA/test_backend_postgres.py`)**:
     - Loại bỏ toàn bộ giả lập SQLite và `threading.Lock`.
     - Kiểm tra hoán đổi ô đồ nguyên tử với ràng buộc `CONSTRAINT uk_owner_slot UNIQUE (...) DEFERRABLE INITIALLY DEFERRED`, assert bắt buộc có lỗi va chạm khi commit nếu trùng slot ngoài transaction swap.
     - Kiểm tra chống dupe bằng 2 kết nối cơ sở dữ liệu độc lập chạy đồng thời transaction `SELECT ... FOR UPDATE`, assert đúng 1 transaction thành công và 1 transaction thất bại/rollback.
     - Bổ sung `docker-compose.yml` (`postgres:16-alpine`) phục vụ chạy test local.
  3. **Phân tách Workflow CI (`.github/workflows/tests.yml`)**:
     - Tách thành 2 job riêng biệt:
       - `gates`: Chạy trên `ubuntu-latest` với `services: postgres:16-alpine`. Chạy `validate_gdd_consistency.py` và `test_backend_postgres.py`.
       - `ue-tests`: Chạy trên `self-hosted` để chạy bộ test tự động của Unreal Engine.
     - Cập nhật `README.md`: Hướng dẫn chi tiết nhãn runner `[self-hosted]` và các yêu cầu môi trường cho `ue-tests`.
  4. **Tạo `production/ROADMAP.md` & Dừng Roadmap**:
     - Tạo placeholder `production/ROADMAP.md` chờ người dùng cập nhật nội dung.
     - Dừng mọi công việc từ Giai đoạn 0 trở đi theo đúng chỉ thị.
  5. **Nâng cấp Quy tắc Subagent Reviewer (`.claude/agents/reviewer.md`)**:
     - Bổ sung nhiệm vụ kiểm tra tính xác thực của test (test DB phải kết nối PostgreSQL thật, không dùng mock/sqlite).
     - Bổ sung cảnh báo nghiêm ngặt nếu phát hiện bất kỳ thay đổi nào trong `DECISIONS.md` mà không có chỉ thị phê duyệt từ người dùng.

### 2.1 Cập Nhật Bổ Sung PR #1: Chuẩn Hóa Thuật Ngữ, Ánh Xạ Vũ Khí & Nâng Cấp Validator QA
- **Mục tiêu**: Thực hiện toàn diện 5 yêu cầu bổ sung của người dùng trước khi duyệt PR #1.
- **Nội dung thực hiện chi tiết**:
  1. **Chuẩn hóa Thuật ngữ Chức nghiệp & Độ hiếm**:
     - Thay thế toàn bộ cách dùng từ "Tier" hoặc tên độ hiếm (`Normal`/`Rare`/`Epic`/`Mythic`) cho Class bằng danh pháp chính thức: **Bậc (Rank)** gồm `Bậc T1 (Sơ cấp)`, `Bậc T2 (Trung cấp)`, `Bậc T3 (Cao cấp)`, `Bậc T4 (Ẩn)`.
     - Tách biệt rành mạch hai thang đo:
       - Thang 5 Bậc Hiếm Trang Bị: `Common`, `Uncommon`, `Rare`, `Epic`, `Legendary` (bỏ hoàn toàn `Immortal`, `Divine`, `ShieldTier`, `5-Tier Item`).
       - Thang 4 Bậc Kỹ Năng / Quyển Trục Chuyển Chức: `Normal` (T1), `Rare` (T2), `Epic` (T3), `Mythic` (T4).
     - Đã rà soát và chuẩn hóa triệt để trên: `character-visual-system.md`, `skill-progression-system.md`, `foundational-classes.md`, `inventory-system.md`, `game-concept.md`, `systems-index.md`, `blacksmithing-system.md`, `core-game-loop.md`, `zone-system.md`.
  2. **Chuẩn hóa Ánh Xạ Vũ Khí (`itemization.md`, `character-visual-system.md`, `game-concept.md`)**:
     - Thêm `Swordmaster` vào danh sách chức nghiệp sử dụng `Weapon.1H.Blade` (Kiếm 1 tay không khiên).
     - Loại bỏ `Templar` khỏi `Weapon.1H.Blade` (Templar dùng `Weapon.1H.Mace` + Đại Thuẫn).
     - Loại bỏ `Vanguard` khỏi `Weapon.2H.Heavy` (Vanguard chỉ dùng `Weapon.1H.Blade` + Khiên Sắt Vuông).
     - Cập nhật số frame Idle 16 class: $16 \times 8 \times 4 = 512$ frames; cập nhật toàn bộ tham chiếu sprite, vệt chém Niagara và diagram Mermaid sang 16 class.
  3. **Cập nhật `game-concept.md`**:
     - Đặt ghi chú dẫn chiếu rõ ràng ở đầu Mục 4: *"Đã thay thế bởi advanced-classes.md và DECISIONS.md"*.
     - Đồng bộ bảng class và vũ khí sang 16 class và thang 5 độ hiếm trang bị chuẩn.
  4. **Nâng cấp Toàn diện Bộ Kiểm Tra Tính Nhất Quán (`Tools/QA/validate_gdd_consistency.py`)**:
     - Bổ sung kiểm tra QUAN HỆ sâu:
       - Rà soát ràng buộc vũ khí của từng class (`Swordmaster`, `Templar`, `Vanguard`, `Inquisitor`).
       - Kiểm tra việc cấm dùng "Tier" / độ hiếm cho class, cấm "ShieldTier" / "5-Tier Item", cấm "Immortal" / "Divine".
       - Bắt buộc kiểm tra ghi chú dẫn chiếu tại `game-concept.md`.
     - **Kiểm chứng tính nghiêm ngặt**: Đã chạy thử trên bản tài liệu trước khi sửa, script phát hiện chính xác **28 lỗi vi phạm trên 7 file**. Sau khi hoàn tất sửa đổi, script báo: **22 files checked. Errors: 0 \| Warnings: 0. [RESULT] GDD Consistency Check PASSED**.
  5. **Đánh Giá Phản Biện Độc Lập Từ Subagent Reviewer**:
     - Subagent reviewer (Context độc lập) đã tiến hành rà soát 3 vòng đối chiếu với `DECISIONS.md`.
     - Vòng 1 & 2: Phát hiện các vị trí còn sót trong `character-visual-system.md` và `inventory-system.md`.
     - Vòng 3 (Sau khi hoàn tất khắc phục): Subagent đưa ra kết luận chính thức: **VERDICT: PASS**.

---

## 3. Danh Sách 16 Class Đã Được Duyệt Chính Thức
*(Chi tiết đầy đủ xem tại [`DECISIONS.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/DECISIONS.md))*

- **Nhánh Guard (Bảo hộ - 5 class)**:
  - T1: Vanguard
  - T2: Templar, Berserker, Swordmaster
  - T3: Dragon Knight (từ Templar/Berserker), Void Blade (từ Swordmaster)
- **Nhánh Scout (Du hiệp - 3 class)**:
  - T1: Ranger
  - T2: Shadowblade
  - T3: Phantom Stalker
- **Nhánh Caster (Học giả - 3 class)**:
  - T1: Arcanist
  - T2: Elementalist
  - T3: Chronomancer
- **Nhánh Faith (Tu sĩ - 3 class)**:
  - T1: Acolyte
  - T2: Inquisitor
  - T3: Seraph
- **Nhánh Apex (Tối cao - 1 class)**:
  - T4: God Slayer (Mở qua thử thách tối thượng, kết hợp đa nhánh)

**Tổng cộng**: 15 Class nhánh + 1 Class Apex = **16 Class**. Tuyệt đối không thêm class ngoài danh sách này.
