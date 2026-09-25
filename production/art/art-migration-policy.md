# Chính Sách Di Chuyển & Chuẩn Hóa Tài Nguyên Mỹ Thuật (Art Migration Policy)

> **Tài liệu**: `production/art/art-migration-policy.md`  
> **Phiên bản**: 1.0.0 (Package 0 — Production Baseline `481106e`)  
> **Áp dụng cho**: Toàn bộ tài sản đồ họa tiền kỳ, mẫu thử Vertical Slice và tài nguyên mới nhập vào Project Ascendant.

---

## 1. Nguyên Tắc Cốt Lõi (Core Principles)

> [!IMPORTANT]
> **Legacy artwork is not production-authoritative until it passes the current production pipeline and Art Gate.**  
> *(Tài nguyên đồ họa tiền kỳ tuyệt đối không có giá trị sản xuất chính thức cho đến khi được chuẩn hóa qua quy trình kỹ thuật và được phê duyệt bởi Art Gate — Agent 5).*

Để bảo vệ sự toàn vẹn của dự án quy mô MMO/ARPG với hàng nghìn tài sản đa tầng, Project Ascendant thiết lập phân định thẩm quyền tuyệt đối (Authority Boundaries):

```text
┌───────────────────────────────────┬────────────────────────────────────────────────────────────────┐
│ Thư Mục / Phân Vùng              │ Vai Trò & Thẩm Quyền (Authority Level)                         │
├───────────────────────────────────┼────────────────────────────────────────────────────────────────┤
│ Content/Art/                      │ PRODUCTION ART (Chân lý sản xuất chính thức trong Unreal)     │
│ Tools/Aseprite/                   │ PRODUCTION TOOLING (Công cụ sinh sprite, Lua scripts, Master)  │
│ production/qa/                    │ ART GATE (Cổng thẩm định chất lượng & phê duyệt của Agent 5)   │
│ Art_Gallery/legacy/               │ HISTORICAL / REFERENCE (Tư liệu lưu trữ, cách ly & tham khảo)  │
└───────────────────────────────────┴────────────────────────────────────────────────────────────────┘
```

---

## 2. Lưu Đồ Di Chuyển Chuẩn (Migration Lifecycle Flow)

Mọi tài sản mỹ thuật tiền kỳ khi muốn đưa vào sử dụng trong bản phát hành chính thức bắt buộc phải đi qua 6 chặng liên hoàn:

```text
               ┌───────────────────────────┐
               │       LEGACY ASSET        │
               │   (Art_Gallery/legacy/)   │
               └─────────────┬─────────────┘
                             │
                             ▼
               ┌───────────────────────────┐
               │    CENSUS & INVENTORY     │
               │ (legacy-art-inventory.md) │
               └─────────────┬─────────────┘
                             │
       ┌─────────────┬───────┴───────┬─────────────┬─────────────────┐
       ▼             ▼               ▼             ▼                 ▼
    [KEEP]       [REWORK]      [REFERENCE]     [ARCHIVE]    [DELETE_CANDIDATE]
       │             │               │             │                 │
       │             │           (Giữ làm     (Lưu trữ lịch     (Chờ lệnh xóa
       │             │          tư liệu vẽ)    sử, cách ly)      trong commit sau)
       └──────┬──────┘
              ▼
 ┌───────────────────────────┐
 │ PRODUCTION SPECIFICATION  │  Đặc tả kỹ thuật (Lưới 32x32 px, CPS2 30 màu,
 │ (Kỹ thuật & Giải phẫu)    │  Tỷ lệ Chibi Heroic 3.2-3.5 đầu, ghim chân 64,114)
 └────────────┬──────────────┘
              ▼
 ┌───────────────────────────┐
 │   ASEPRITE MASTER RIG     │  Dựng lại trên Master Rigs (.aseprite)
 │   & PRODUCTION TOOLING    │  Xuất bản qua aseprite_export_pipeline.py
 └────────────┬──────────────┘
              ▼
 ┌───────────────────────────┐
 │     AGENT 5: ART GATE     │  Thẩm định 5 tiêu chí Anti-AI, Mixel, Silhouette,
 │ (production/qa/criteria)  │  Color Bleed, Pillow Shading & Paper2D Frame Check
 └────────────┬──────────────┘
              │
       ┌──────┴──────┐
       ▼             ▼
  [REJECTED]     [APPROVED]
  (Về Rework)        │
                     ▼
        ┌─────────────────────────┐
        │      Content/Art/       │  Nhập chính thức vào Engine kèm .uasset,
        │    (Production Engine)  │  Paper2D Flipbooks & PaperZD AnimBP
        └─────────────────────────┘
```

---

## 3. Hệ Thống Tiêu Chuẩn Phân Loại (Disposition Criteria)

Mỗi tài sản cũ khi được kiểm kê trong bản tổng điều tra phải được gắn duy nhất 1 trong 5 nhãn định đoạt (Disposition):

### 3.1. KEEP (Bảo tồn & Chuẩn hóa Nguồn)
- **Định nghĩa**: Tài sản có cấu trúc kỹ thuật sát với tiêu chuẩn sản xuất hiện hành, có thể sử dụng làm nguồn cơ sở để chuyển đổi sang Master Rig mà không cần thiết kế lại từ đầu.
- **Lưu ý**: Nhãn `KEEP` **không** đồng nghĩa với việc tài sản đã được duyệt vào game. Nó có nghĩa: *"Giữ lại trong pipeline để chuẩn hóa qua Art Gate"*.
- **Ví dụ**: Các lớp giáp Paperdoll 32x32 px trong `Content/art/characters/paperdoll/`, các bộ khung animation MasterRigs.

### 3.2. REWORK (Tái Thiết Kế & Chuyển Đổi Format)
- **Định nghĩa**: Ý tưởng hình ảnh, kiểu dáng thiết kế hoặc concept gameplay có giá trị cao, nhưng định dạng hiện tại (kích thước sai lệch, độ phân giải không đồng nhất 128x128/256x256, bảng màu chưa theo CPS2, thiếu tách lớp Paperdoll) đòi hỏi phải vẽ lại hoặc xuất bản lại từ file gốc thông qua Aseprite Master Rig.
- **Ví dụ**: 23 mẫu vũ khí trong `Art_Gallery/legacy/weapons/`, các biểu tượng dược phẩm/trang bị 256x256 px.

### 3.3. REFERENCE (Tư Liệu Tham Khảo Mỹ Thuật)
- **Định nghĩa**: Tác phẩm nghệ thuật có chất lượng hoàn thiện cao về mặt thị giác, giải phẫu hoặc phối cảnh, nhưng được tạo ra cho mục đích trưng bày (Showcase, Marketing, Concept Art) chứ không được thiết kế cho sprite in-game 2.5D Isometric.
- **Xử lý**: Giữ nguyên trong `Art_Gallery/legacy/` làm bảng tâm trạng (Moodboard) và tài liệu định hướng cho họa sĩ; tuyệt đối không import vào Engine.
- **Ví dụ**: Tranh minh họa 1024x1024 của Stone Golem Boss và Lich Necromancer, các bản vẽ 512x512 Showcase Class.

### 3.4. ARCHIVE (Lưu Trữ Lịch Sử)
- **Định nghĩa**: Tài liệu, ảnh chụp, mô hình blockout đại diện cho các mốc tiến độ trong quá khứ (như bản Vertical Slice Sprint 1 & 2), đã hoàn thành sứ mệnh kiểm chứng gameplay và không còn mục đích sử dụng trực tiếp trong sản xuất.
- **Xử lý**: Đóng gói lưu trữ vĩnh viễn trong kho Legacy; không xóa bỏ để phục vụ đối chiếu lịch sử phát triển.
- **Ví dụ**: `Ruins_Blockout_3D_Isometric.gif`, `Complete_Map_Showcase.jpg`, các tệp ảnh kiểm tra bóng đen (Silhouette QA check).

### 3.5. DELETE_CANDIDATE (Ứng Viên Chờ Tiêu Hủy)
- **Định nghĩa**: Các tệp tạm, tệp trùng lặp 100% nội dung (duplicate) nằm rải rác ngoài thư mục quy định, hoặc các bản nháp hỏng không có giá trị tham khảo.
- **Quy tắc An Toàn Tuyệt Đối**: **Không xóa vĩnh viễn** bất kỳ tệp nào trong quá trình kiểm kê. Mọi tệp vô dụng đều phải gắn cờ `DELETE_CANDIDATE` để Orchestrator xem xét và ban hành lệnh xóa trong một commit dọn dẹp độc lập riêng biệt.

---

## 4. Quy Định Nghiêm Ngặt Về Phụ Thuộc Unreal Engine (.uasset / .umap)

Để ngăn chặn triệt để lỗi gãy tham chiếu (Broken References) và lỗi `Missing Dependency` trong Unreal Engine:

1. **Cấm Di Chuyển Tự Động Các Tệp Có Phụ Thuộc Kỹ Thuật**:
   - Bất kỳ tài sản nào đang được tham chiếu bởi mã C++ (`StaticLoadObject`, `StaticLoadClass`), Blueprint, DataAsset, hoặc Integration Test **bắt buộc phải được giữ nguyên vị trí thư mục hiện tại trong Content/**.
   - Các tài sản này được định danh trạng thái: `Referenced? Yes` và chỉ được chuyển đổi khi có mã C++ thay thế tương ứng.
2. **Danh Mục Tài Sản Cấm Di Chuyển Đợt Này**:
   - `Content/art/characters/vanguard/` (C++ `PABaseCharacter.cpp`, `paper2d_flipbooks_test.cpp`).
   - `Content/art/characters/boss/` (C++ `PAStoneGolemBoss.cpp`, `paper2d_flipbooks_test.cpp`).
   - `Content/art/characters/T_Vanguard_Spritesheet.uasset`, `ranger_pixel_spritesheet.png`, `arcanist_pixel_spritesheet.png` (C++ `PACharacterSelectTypes.cpp`).
   - `Content/art/characters/MasterRigs/`, `UpperBodyWeapons/`, `ClassIdentity/`, `Civilian/` (Hệ thống DataAsset và Flipbooks tích hợp).
3. **Quy Trình Hoán Đổi (Deprecation Swapping)**:
   - Khi tài sản mới (Production-Approved) sẵn sàng trong `Content/Art/`:
     1. Cập nhật mã nguồn C++ và Blueprint trỏ sang đường dẫn mới.
     2. Biên dịch UBT và chạy bộ kiểm thử `RunTests` đạt 100% PASS.
     3. Lúc này tài sản cũ mới được tháo gỡ tham chiếu và chuyển vào kho Legacy.
