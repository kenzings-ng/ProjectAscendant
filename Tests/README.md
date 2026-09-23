# Test Infrastructure — Project Ascendant

**Engine**: Unreal Engine 5.8.2
**Test Framework**: Unreal Engine Automation Testing Framework (`AutomationSpec` / `IMPLEMENT_SIMPLE_AUTOMATION_TEST`)  
**CI Workflow**: `.github/workflows/tests.yml`  
**Setup Date**: 2026-09-16  

---

## Directory Layout

```
tests/
  unit/           # Kiểm thử đơn vị cô lập (công thức sát thương, máy trạng thái, Posture clamping)
  integration/    # Kiểm thử tích hợp (liên hệ thống, chuỗi sự kiện mạng, Save/Load round-trip)
  smoke/          # Danh mục kiểm thử critical-path cho cổng /smoke-check (< 15 phút)
  evidence/       # Nhật ký ảnh chụp màn hình và biên bản ký duyệt kiểm thử thủ công
```

---

## Running Tests

### 1. Chạy Trực Tiếp Trong Unreal Editor
1. Mở Editor: **Window ➔ Test Automation** (hoặc Session Frontend ➔ tab Automation).
2. Tích chọn nhóm test: `ProjectAscendant.*`.
3. Nhấn nút **Start Tests**.

### 2. Chạy Chế Độ Dòng Lệnh Headless (CI & Terminal)
```bash
"$UE_EDITOR_PATH" "$PROJECT_DIR/ProjectAscendant.uproject" \
  -nullrhi -nosound -unattended -stdout \
  -ExecCmds="Automation RunTests ProjectAscendant.; Quit" \
  -log=AutomationTest.log
```

---

## Test Naming Conventions

- **Tệp kiểm thử**: `[System]_[Feature]Test.cpp` (Ví dụ: `Combat_FormulasTest.cpp`, `Inventory_FastArrayTest.cpp`)
- **Tên lớp kiểm thử**: `F[System][Feature]Test`
- **Tên hạng mục kiểm tra**: `"ProjectAscendant.[System].[Feature]"`

---

## Story Type ➔ Test Evidence Mapping

| Phân Loại Story | Bằng Chứng Yêu Cầu | Vị Trí Lưu Trữ |
|---|---|---|
| **Logic** | Kiểm thử đơn vị C++ tự động — bắt buộc 100% Pass | `tests/unit/[system]/` |
| **Integration** | Automation Spec liên module hoặc log kịch bản mạng | `tests/integration/[system]/` |
| **Visual / Feel** | Ảnh chụp màn hình hoạt ảnh Pixel + Ký duyệt của Art Director | `tests/evidence/` |
| **UI** | Kiểm thử tương tác CommonUI hoặc biên bản thủ công | `tests/evidence/` |
| **Config / Data** | Vượt qua cổng Smoke Check không cảnh báo | `production/qa/smoke-*.md` |

---

## CI / CD

Kiểm thử tự động được kích hoạt trên mỗi commit đẩy lên nhánh `main` và mọi Pull Request. Bất kỳ bài test nào thất bại sẽ chặn lệnh Merge.
