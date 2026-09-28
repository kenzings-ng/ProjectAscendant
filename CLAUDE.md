# Claude Code Game Studios -- Game Studio Agent Architecture

Indie game development managed through 49 coordinated Claude Code subagents.
Each agent owns a specific domain, enforcing separation of concerns and quality.

## Technology Stack

- **Engine**: Unreal Engine 5
- **Language**: C++ / Blueprint
- **Version Control**: Git with trunk-based development
- **Build System**: UnrealBuildTool (UBT)
- **Asset Pipeline**: Unreal Engine Content Browser, Niagara VFX, Enhanced Input

> **Note**: Engine-specialist agents exist for Godot, Unity, and Unreal with
> dedicated sub-specialists. Use the set matching your engine.

## Project Structure

@.claude/docs/directory-structure.md

## Engine Version Reference

@docs/engine-reference/unreal/VERSION.md

## Technical Preferences

@.claude/docs/technical-preferences.md

## Coordination Rules

@.claude/docs/coordination-rules.md

## CHẾ ĐỘ TỰ VẬN HÀNH (Autonomous Mode)

Mục tiêu: thực hiện tuần tự roadmap (Giai đoạn 0 → 6) và các quyết định thiết kế đã chốt, không cần tôi duyệt từng bước.

### Quy trình mỗi đầu việc
1. Tạo nhánh riêng từ main. Làm việc nhỏ, commit nhỏ.
2. Trước khi sửa file nào, đọc nội dung gốc của file đó. Nếu chỉ thị (kể cả của tôi) mâu thuẫn với file, ghi lại mâu thuẫn và chọn theo file; báo trong PROGRESS.md.
3. Chạy toàn bộ cổng tự động: build, test ProjectAscendant.*, pixel-review (nếu có art), script kiểm tra nhất quán GDD (nếu sửa GDD), migration + test backend (nếu sửa backend).
4. Gọi subagent "reviewer" với context mới: nhiệm vụ là TÌM LỖI, mỗi nhận xét phải trích file:dòng. Sửa hết lỗi reviewer nêu rồi chạy lại bước 3.
5. Chỉ merge vào main khi mọi cổng pass và reviewer đồng ý. Push lên remote. Xác nhận bằng git ls-remote.
6. Ghi vào production/PROGRESS.md: việc đã làm, commit hash đã push, log test (số test, pass, exit code), kết luận reviewer.

### Việc đầu tiên
Trước khi làm roadmap: xây các cổng tự động còn thiếu (script kiểm tra nhất quán GDD, CI chạy test headless, Postgres test cho backend), cấu hình bảo vệ nhánh main, và cấu hình .claude/settings.json chặn git push --force, xóa lịch sử, rm -rf ngoài thư mục tạm.

### BẮT BUỘC DỪNG VÀ HỎI TÔI khi
- Cần duyệt thẩm mỹ: bộ art mẫu Vanguard (Giai đoạn 3), cảm giác chơi.
- Cần tiêu tiền (asset pack, dịch vụ trả phí).
- Thêm asset có license không phải CC0, hoặc asset sinh bằng AI.
- Xóa asset ngoài danh sách DELETE đã duyệt, hoặc thay đổi quyết định đã chốt.
- Thử 3 lần vẫn không qua một cổng.

### CẤM
- Push thẳng vào main, force push, viết lại lịch sử git.
- Báo cáo "đã xong / đã merge" khi chưa có commit hash trên remote và log test.
- Tự tạo thuật ngữ, tiền tệ, thang độ hiếm hay hệ thống mới không có trong GDD.
- Tự rewrite code hoặc refactor diện rộng ngoài phạm vi task.

> **First session?** If the project has no engine configured and no game concept,
> run `/start` to begin the guided onboarding flow.

## Coding Standards

@.claude/docs/coding-standards.md

## Context Management

@.claude/docs/context-management.md
