# Legacy Art Repository (Quarantine & Historical Reference)

> **Location**: `Art_Gallery/legacy/`  
> **Status**: **QUARANTINED / HISTORICAL REFERENCE ONLY**  
> **Authority Level**: **NON-AUTHORITATIVE**  
> **Art Gate Status**: **NOT_SUBMITTED** (None of these assets are production-approved)  
> **Date Established**: 2026-09-25 (Package 0 — Legacy Art Census & Migration)

---

## 1. Mục Đích & Bối Cảnh (Purpose & Context)

Thư mục `Art_Gallery/legacy/` được thiết lập nhằm phân tách triệt để ranh giới kỹ thuật giữa **Tài nguyên Đồ họa Sản xuất Thực thụ (Production-Authoritative Art)** và **Tài nguyên Mỹ thuật Tiền Kỳ / Nguyên Mẫu Vertical Slice (Legacy / Prototype Art)**.

Trước khi thiết lập 5 Gói Quy chuẩn Sản xuất của Project Ascendant:
- **Gói 1**: Quản lý Bản quyền & Nguồn gốc Tài sản bên thứ ba (`THIRD_PARTY_ASSETS.md`).
- **Gói 2**: Quy chuẩn Xuất bản & Khung xương Aseprite CLI (`Tools/Aseprite/`).
- **Gói 5**: Cổng Kiểm định Chất lượng Mỹ thuật (`production/qa/art-gate-criteria.md`).
- **Gói 3**: Quy hoạch Sinh cảnh Môi trường PCG (`Content/Maps/PCG/`).
- **Gói 4**: Gia cố Hệ thống Chiến đấu & Khung kiểm thử tự động GAS (`PACombatRegressionHardeningTests.cpp`).

Dự án đã tích lũy một lượng lớn tác phẩm tự vẽ, ảnh minh họa ý tưởng, ảnh render showcase độ phân giải cao và các tệp kiểm tra chất lượng từ giai đoạn Vertical Slice (Sprint 1 & Sprint 2). Các tệp này **không tuân thủ đồng nhất** chuẩn lưới pixel 32x32 px HD-2D, hệ màu CPS2 30 màu, ghim chân Paper2D (64, 114) hoặc chưa từng trải qua quy trình kiểm duyệt khắt khe của Art Gate.

Thư mục này đóng vai trò là **khu vực cách ly (quarantine)** và **kho lưu trữ lịch sử**, bảo đảm không một tài sản tiền kỳ nào bị xóa bỏ hoặc thất thoát, đồng thời ngăn chặn việc sử dụng nhầm lẫn các tài nguyên chưa qua thẩm định vào pipeline sản xuất chính thức.

---

## 2. Cấu Trúc Phân Loại Legacy (Directory Layout)

```text
Art_Gallery/legacy/
├── README.md             # Tài liệu định hướng và quy chuẩn cách ly
├── characters/           # Minh họa nhân vật 4 hệ phái và trùm (Vanguard, Ranger, Arcanist, Acolyte, Golem, Lich)
├── weapons/              # 23 mẫu vũ khí tiền kỳ & đá rèn đúc (Broadswords, Bows, Staves, Daggers, Maces, etc.)
├── armor/                # 8 bộ giáp, mũ trụ, giày và xà cạp tiền kỳ (Knight Cuirass, Ranger Hood, Scout Vest, etc.)
├── environment/          # Ảnh render blockout 3D di tích, concept art và bản đồ tổng thể Vertical Slice
├── ui/                   # Icon phù hiệu, biểu tượng giao diện tiền kỳ (Blacksmith Ward Icon)
├── misc/                 # Đá quý, dược phẩm, nguyên liệu khoáng sản và 48 ảnh kiểm thử bóng đen (Silhouette QA)
└── index.html            # Trình hiển thị đồ họa tiền kỳ (Legacy Showcase Viewer)
```

---

## 3. Quy Tắc Bản Quyền & Thẩm Quyền (Authority & Governance)

1. **Không Tự Động Phê Duyệt (Not Production-Approved)**:
   - Sự hiện diện của bất kỳ tệp đồ họa nào trong `Art_Gallery/legacy/` **không** đồng nghĩa với việc tài sản đó đạt chuẩn đưa vào game.
   - Toàn bộ tài sản tại đây mang trạng thái mặc định:
     ```yaml
     ownership: ProjectAscendant
     production_status: REWORK (hoặc REFERENCE / ARCHIVE)
     art_gate_status: NOT_SUBMITTED
     ```
2. **Không Chỉnh Sửa Trực Tiếp (Read-Only Heritage)**:
   - Tuyệt đối không vẽ đè, scale lại, chỉnh màu, crop hoặc tối ưu hóa trực tiếp trên các tệp legacy này.
   - Mọi cải tiến phải được thực hiện bằng cách tạo tệp nguồn mới trong quy trình Aseprite (`Tools/Aseprite/templates/`).
3. **Nguồn Chân Lý Duy Nhất (Single Source of Truth)**:
   - `Content/Art/`: Thư mục duy nhất chứa tài sản đồ họa sản xuất chính thức.
   - `Tools/Aseprite/`: Thư mục chứa công cụ và mẫu dựng (Master Rigs).
   - `production/qa/`: Nơi Art Gate (Agent 5) ban hành biên bản thẩm định đạt chuẩn.
   - `Art_Gallery/legacy/`: Kho tư liệu tham khảo và lưu trữ lịch sử.

---

## 4. Quy Trình Di Chuyển Lại Vào Sản Xuất (Migration Workflow)

Khi một tài sản trong kho Legacy được phê duyệt để tái sản xuất cho phiên bản chính thức, đội ngũ kỹ thuật và họa sĩ phải tuân thủ quy trình 6 bước:

```text
       [TÀI SẢN LEGACY] (Art_Gallery/legacy/)
               │
               ▼
       1. CENSUS & INVENTORY (Đánh giá phân loại: KEEP / REWORK / REFERENCE)
               │
               ▼
       2. ART SPECIFICATION (Lập hồ sơ kỹ thuật theo chuẩn 32x32 px HD-2D)
               │
               ▼
       3. ASEPRITE MASTER RIG (Dựng lại trên master_rig_*.aseprite hoặc Lua export)
               │
               ▼
       4. ART GATE AUDIT (Agent 5 kiểm định: Palette, Silhouette, Mixels, Anti-AI)
               │
               ▼
       5. PHÊ DUYỆT (Trạng thái: APPROVED)
               │
               ▼
       6. NHẬP ENGINE (Đưa vào Content/Art/ hoặc Content/Sprites/ kèm .uasset)
```

Chi tiết quy chuẩn di chuyển được quy định tại [`production/art/art-migration-policy.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/production/art/art-migration-policy.md).
