# PROJECT ASCENDANT -- TIẾN TRÌNH THI CÔNG & NHẬT KÝ TỰ VẬN HÀNH (PROGRESS.md)

> **Dự án**: Project Ascendant (2.5D Isometric HD-2D Dark Fantasy Action RPG / MMO)  
> **Chế độ**: CHẾ ĐỘ TỰ VẬN HÀNH (Autonomous Mode)  
> **Cập nhật lần cuối**: 2026-09-28  

---

## 1. Bản Đồ Lộ Trình (Roadmap Overview: Giai đoạn 0 → 6)

| Giai đoạn | Nội dung trọng tâm | Trạng thái | Ghi chú & Nhánh |
| :--- | :--- | :--- | :--- |
| **Giai đoạn 0** | Hoàn tất Spine Boss Stone Golem (IK constraint, squash slam, Nearest filter, Art Gate exception) | **HOÀN THÀNH** | Merged to `main` (`45c1103`) |
| **Giai đoạn 1** | Vanguard Locomotion, 8-directional Flipbooks, Socket System (`Hand_R`, `Hand_L`) | **HOÀN THÀNH** | Merged to `main` (`22605f7`) |
| **Giai đoạn 1.5** | Iris Network Replication, PIE 2-Player Dedicated Server, Mixed ASC Mode | **HOÀN THÀNH** | Merged to `main` (`7b8459f`) |
| **Việc đầu tiên** | Cổng tự động (GDD validator, CI headless, Postgres test), DECISIONS.md, Reviewer subagent, Settings safety | **ĐANG THỰC HIỆN** | Nhánh `infra/autonomous-mode-setup` |
| **Docs Sync** | Đồng bộ GDD Cây chuyển chức 4 Nhánh (Guard, Scout, Caster, Faith) + Apex, Dual-Class (A1/B1) | **SẴN SÀNG AUDIT** | Nhánh `docs/class-tree-dual-class` |
| **Giai đoạn 2** | Whitebox / Prototype Level: Graybox Citadel, Tilemap Biome, Collision NavMesh | **CHƯA BẮT ĐẦU** | Chờ Giai đoạn 1 & Docs |
| **Giai đoạn 3** | Vanguard Production Art Gate (Aesthetic Approval sample) & Paperdoll System | **CHỜ DUYỆT ART** | Cần hỏi người dùng về art mẫu |
| **Giai đoạn 4** | Combat System Vertical Slice: Vanguard + Golem Boss Encounter (Stagger, Hitstun, Damage Numbers) | **CHƯA BẮT ĐẦU** | Theo spec `combat-system.md` |
| **Giai đoạn 5** | Dedicated Server Integration & Network Stress Test (Iris Replication under 100ms jitter) | **CHƯA BẮT ĐẦU** | Theo spec `multiplayer-coop.md` |
| **Giai đoạn 6** | Polish, VFX/SFX, Audio Ambience, Build Package Linux/Windows | **CHƯA BẮT ĐẦU** | Hoàn thiện Vertical Slice |

---

## 2. Nhật Ký Đầu Việc & Kết Quả Kiểm Tra (Execution & Test Log)

### [Đầu việc 0.1]: Thiết lập Cơ chế Tự Vận Hành & Các Cổng Tự Động
- **Nhánh**: `infra/autonomous-mode-setup`
- **Mục tiêu**: Xây dựng toàn bộ hạ tầng bảo vệ, validator nhất quán GDD, test suite Postgres, cấu hình subagent reviewer, và tài liệu quyết định `DECISIONS.md`.
- **Các thành phần đã triển khai**:
  1. `CLAUDE.md`: Tích hợp nguyên tắc Chế Độ Tự Vận Hành, quy trình 6 bước, điều kiện dừng và các lệnh cấm.
  2. `production/DECISIONS.md`: Văn bản hóa 10 quyết định kiến trúc đã chốt (Thuật ngữ Bậc vs Độ hiếm, 16 class 4 nhánh + Apex, Dual-Class A1/B1, Dual-track levels 1-50 & 1-20, phân lập 2 thang độ hiếm, không dùng Ash Shards, quy chuẩn vũ khí/khiên, tag `Class.Line.<Line>.<Class>`, database `uk_owner_slot DEFERRABLE`, Spine Art Gate exception).
  3. `.claude/agents/reviewer.md` & Subagent System: Định nghĩa reviewer phản biện độc lập chuyên tìm lỗi và trích dẫn `file:dòng`.
  4. `.claude/settings.json` & `Tools/git-hooks/pre-push`: Thiết lập cơ chế chặn direct push `main`, chặn force push, chặn rewrite lịch sử, chặn `rm -rf` ngoài thư mục tạm.
  5. `Tools/QA/validate_gdd_consistency.py`: Script tự động quét và kiểm tra tính nhất quán của toàn bộ tài liệu GDD.
  6. `Tools/QA/test_backend_postgres.py`: Test suite tự động cho schema Postgres và giao dịch chuyển chức khóa dòng nguyên tử chống dupe đồ.
  7. `Tools/QA/run_headless_tests.sh`: Trình chạy kiểm thử headless hợp nhất.
- **Kết quả Cổng Tự Động**:
  - `test_backend_postgres.py`: PASS 100% (4/4 tests: unique constraint, atomic transaction, replay attack prevention, class tag update).
- **Trạng thái**: Sẵn sàng gọi subagent Reviewer và merge vào `main`.

---

## 3. Quyết Định Đã Chốt & Ràng Buộc Bắt Buộc (Summary of Locked Decisions)
*(Chi tiết đầy đủ xem tại [`DECISIONS.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/DECISIONS.md))*
1. **Thuật ngữ**: Bậc chức nghiệp (T1, T2, T3, T4) $\ne$ Độ hiếm vật phẩm.
2. **16 Class**: 4 Nhánh (Guard, Scout, Caster, Faith) + Apex God Slayer.
3. **Dual-Class**: 1 Chính + 1 Phụ, Bậc Phụ $\le$ Bậc Chính, lưu tiến trình JSONB, đổi Chính $\leftrightarrow$ Phụ theo Quy tắc A1.
4. **Levels**: Character Level 1–50 (Base stats) & Class Level 1–20 (Skills/Passives).
5. **Rarity**: Trang bị 5 bậc (`Common`..`Legendary`) vs Kỹ năng/Quyển trục 4 bậc (`Normal`..`Mythic`). Tiền tệ phân rã: Tàn Trang (`item_skill_shard`), tuyệt đối không dùng Ash Shards.
6. **Vũ khí**: Vanguard = Khiên sắt vuông + 1H.Blade; Templar = Đại thuẫn + 1H.Mace; Inquisitor = 1H.Mace (bỏ roi xích); Berserker = 2H.Heavy; Swordmaster = 1H.Blade (không khiên).
7. **Database**: Bảng `items` độc lập, `uk_owner_slot DEFERRABLE INITIALLY DEFERRED`, transaction `SELECT ... FOR UPDATE` row-locking + `ROLLBACK` chống dupe.
8. **Art Gate**: Ngoại lệ Boss Stone Golem Spine 4.3 được chấp nhận rotation artifacts.
