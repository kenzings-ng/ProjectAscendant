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
- **Các thành phần đã triển khai & chỉnh sửa sau phản biện**:
  1. `CLAUDE.md`: Tích hợp nguyên tắc Chế Độ Tự Vận Hành, quy trình 6 bước, điều kiện dừng và các lệnh cấm.
  2. `production/DECISIONS.md`:
     - Chuẩn hóa: 16 Class thuộc 4 nhánh chính + 1 Apex Class đa nhánh = 17 Class hoàn chỉnh.
     - Quy chuẩn tag Apex: `Class.Line.Apex.GodSlayer`.
     - Bổ sung Mục 11: Kiến trúc Netcode 100% Server-Authoritative (ADR-0001), Iris Network Replication, Gameplay Ability System Mixed Mode.
  3. `.claude/agents/reviewer.md` & Subagent System: Định nghĩa reviewer phản biện độc lập chuyên tìm lỗi và trích dẫn `file:dòng`.
  4. `.claude/settings.json` & `Tools/git-hooks/pre-push`:
     - Chặn toàn diện mọi biến thể push vào `main` và `master` (`git push* main*`, `git push* master*`).
     - Chặn toàn bộ lệnh viết lại lịch sử (`git rebase*`, `git commit --amend*`, `git reset --hard*`).
     - Chặn toàn bộ các biến thể hủy diệt của `rm -r*` (`rm -rf *`, `rm -rf ./*`, `rm -rf .git`, `rm -rf Source`, v.v.).
     - Hook `pre-push` bảo vệ cả hai nhánh `main` và `master`.
  5. `Tools/QA/validate_gdd_consistency.py`:
     - Xóa dead code: Kiểm tra đối chiếu `class_name` với từng nhánh cụ thể (`Guard`, `Scout`, `Caster`, `Faith`, `Apex`).
     - Bắt toàn bộ các tag cũ/lỗi thời (`Class.Vanguard`, `Class.VoidBlade`, `Class.TierX.*`, `Class.RankX.*`).
     - Lọc ngữ cảnh thông minh, tránh bypass qua từ ngữ ngây thơ.
     - Hỗ trợ regex multiline cho ràng buộc SQL `CONSTRAINT uk_owner_slot`.
  6. `Tools/QA/test_backend_postgres.py`:
     - Thêm Test 3: Mô phỏng hoán đổi ô đồ nguyên tử (Atomic Inventory Slot Swapping).
     - Thêm Test 4: Kiểm thử đa luồng đồng thời (Concurrent Race Condition / Anti-Dupe Test) chứng minh cơ chế khóa dòng chặn đứng 100% request trùng lặp.
     - Kiểm tra nghiêm ngặt DDL PostgreSQL của GDD và DECISIONS.md.
  7. `Tools/QA/run_headless_tests.sh`:
     - Khắc phục lỗi false-positive: Bắt buộc `TOTAL_PASS > 0` và `TOTAL_FAIL == 0`.
     - Cảnh báo rõ ràng trạng thái skip của Gate 3 khi chạy chế độ nhanh.
- **Báo cáo Phản biện Subagent Reviewer (Vòng 1 - Commit `12c2186`)**:
  - Nhận xét: Nêu 6 nhóm lỗi (1.1–6.1) về mâu thuẫn số class, thiếu mục Netcode trong DECISIONS, dead code validator, SQLite thiếu test concurrency, pre-push thiếu nhánh master, settings.json lọt lưới rm-rf và thiếu minh bạch progress.
  - Kết luận Vòng 1: `VERDICT: FAIL`.
  - **Hành động khắc phục**: Toàn bộ 6 nhóm lỗi đã được giải quyết triệt để trong commit tiếp theo.
- **Kết quả Cổng Tự Động Cập Nhật**:
  - `test_backend_postgres.py`: **PASS 100%** (4/4 tests: DDL check, Atomic slot swap, Concurrency race-condition anti-dupe, Tag update). Exit Code: 0.
  - `validate_gdd_consistency.py`: Đã phát hiện chính xác 25 lỗi tồn đọng trên các file GDD cũ (chờ giải quyết khi audit và merge nhánh `docs/class-tree-dual-class`). Exit Code: 1 (Working as designed to guard GDD integrity).

---

## 3. Quyết Định Đã Chốt & Ràng Buộc Bắt Buộc (Summary of Locked Decisions)
*(Chi tiết đầy đủ xem tại [`DECISIONS.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/DECISIONS.md))*
1. **Thuật ngữ**: Bậc chức nghiệp (T1, T2, T3, T4) $\ne$ Độ hiếm vật phẩm.
2. **17 Class**: 16 Class thuộc 4 Nhánh (Guard, Scout, Caster, Faith) + 1 Apex Class God Slayer.
3. **Dual-Class**: 1 Chính + 1 Phụ, Bậc Phụ $\le$ Bậc Chính, lưu tiến trình JSONB, đổi Chính $\leftrightarrow$ Phụ theo Quy tắc A1.
4. **Levels**: Character Level 1–50 (Base stats) & Class Level 1–20 (Skills/Passives).
5. **Rarity**: Trang bị 5 bậc (`Common`..`Legendary`) vs Kỹ năng/Quyển trục 4 bậc (`Normal`..`Mythic`). Tiền tệ phân rã: Tàn Trang (`item_skill_shard`), tuyệt đối không dùng Ash Shards.
6. **Vũ khí**: Vanguard = Khiên sắt vuông + 1H.Blade; Templar = Đại thuẫn + 1H.Mace; Inquisitor = 1H.Mace (bỏ roi xích); Berserker = 2H.Heavy; Swordmaster = 1H.Blade (không khiên).
7. **Database**: Bảng `items` độc lập, `uk_owner_slot DEFERRABLE INITIALLY DEFERRED`, transaction `SELECT ... FOR UPDATE` row-locking + `ROLLBACK` chống dupe.
8. **Art Gate**: Ngoại lệ Boss Stone Golem Spine 4.3 được chấp nhận rotation artifacts.
9. **Netcode**: 100% Server Authoritative (ADR-0001), Iris Network Replication, GAS Mixed Replication Mode.
