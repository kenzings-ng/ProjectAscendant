# Báo Cáo Di Chuyển Vũ Khí Tĩnh 32×32 (M1 Weapon Migration Report)

> **Gói công việc**: Package M1 — Legacy 32×32 Weapon Migration  
> **Baseline**: `481106e`  
> **Nhánh thực hiện**: `chore/m1-weapon-art-migration`  
> **Thời điểm thực hiện**: 2026-09-25  
> **Vai trò**: Technical Art / Asset Migration Agent  

---

## 1. Tổng Kết (Summary)

- **Số lượng ứng viên nguồn (Source Candidates)**: 23 tệp `Art_Gallery/legacy/weapons/*_32x32_native.png`
- **Số lượng đã xử lý (Processed)**: 23
- **Số lượng đạt chuẩn Art Gate (PASS)**: 22
- **Số lượng yêu cầu làm lại (REWORK_REQUIRED)**: 1 (`Vanguard_06_Broadsword_Combat_Bloodied_32x32_native.png`)
- **Số lượng cần duyệt thủ công (MANUAL_REVIEW)**: 0
- **Số lượng tài sản xuất xưởng chính thức (Production Outputs)**: 22 PNGs + 22 Metadata JSONs tại `Content/Art/Weapons/`

---

## 2. Bảng Thẩm Định Từng Tài Sản (Per-Asset Table)

| Source | Output | Size | Art Gate | Changes | Notes |
| :--- | :--- | :---: | :---: | :--- | :--- |
| `Acolyte_01_Holy_War_Mace_32x32_native.png` | `Content/Art/Weapons/WPN_Acolyte_01_Holy_War_Mace.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (12 colors, clean alpha, 10 o'clock light) |
| `Acolyte_02_Spiked_Morningstar_Mace_32x32_native.png` | `Content/Art/Weapons/WPN_Acolyte_02_Spiked_Morningstar_Mace.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (12 colors, clean alpha, 10 o'clock light) |
| `Acolyte_03_Death_Obsidian_Scythe_32x32_native.png` | `Content/Art/Weapons/WPN_Acolyte_03_Death_Obsidian_Scythe.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (11 colors, clean alpha, 10 o'clock light) |
| `Arcanist_01_Magic_Wizard_Staff_32x32_native.png` | `Content/Art/Weapons/WPN_Arcanist_01_Magic_Wizard_Staff.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (12 colors, clean alpha, 10 o'clock light) |
| `Arcanist_02_Crystal_Wizard_Staff_32x32_native.png` | `Content/Art/Weapons/WPN_Arcanist_02_Crystal_Wizard_Staff.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (12 colors, clean alpha, 10 o'clock light) |
| `Arcanist_03_Void_Corrupted_Staff_32x32_native.png` | `Content/Art/Weapons/WPN_Arcanist_03_Void_Corrupted_Staff.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (8 colors, clean alpha, 10 o'clock light) |
| `Arcanist_04_Astral_Archmage_Staff_32x32_native.png` | `Content/Art/Weapons/WPN_Arcanist_04_Astral_Archmage_Staff.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (13 colors, clean alpha, 10 o'clock light) |
| `Arcanist_05_Ancient_Arcane_Grimoire_32x32_native.png` | `Content/Art/Weapons/WPN_Arcanist_05_Ancient_Arcane_Grimoire.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (15 colors, clean alpha, 10 o'clock light) |
| `Ranger_01_Wooden_Hunting_Bow_32x32_native.png` | `Content/Art/Weapons/WPN_Ranger_01_Wooden_Hunting_Bow.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (8 colors, clean alpha, 10 o'clock light) |
| `Ranger_02_Recurve_Hunting_Bow_32x32_native.png` | `Content/Art/Weapons/WPN_Ranger_02_Recurve_Hunting_Bow.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (8 colors, clean alpha, 10 o'clock light) |
| `Ranger_03_Elven_Composite_Bow_32x32_native.png` | `Content/Art/Weapons/WPN_Ranger_03_Elven_Composite_Bow.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (8 colors, clean alpha, 10 o'clock light) |
| `Ranger_04_Rogue_Hunting_Dagger_32x32_native.png` | `Content/Art/Weapons/WPN_Ranger_04_Rogue_Hunting_Dagger.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (12 colors, clean alpha, 10 o'clock light) |
| `Ranger_05_Frost_Shard_Kris_32x32_native.png` | `Content/Art/Weapons/WPN_Ranger_05_Frost_Shard_Kris.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (10 colors, clean alpha, 10 o'clock light) |
| `Vanguard_01_Broadsword_Tier0_RustedIron_32x32_native.png` | `Content/Art/Weapons/WPN_Vanguard_01_Broadsword_Tier0_RustedIron.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (12 colors, clean alpha, 10 o'clock light) |
| `Vanguard_02_Broadsword_Tier1_SharpSteel_32x32_native.png` | `Content/Art/Weapons/WPN_Vanguard_02_Broadsword_Tier1_SharpSteel.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (10 colors, clean alpha, 10 o'clock light) |
| `Vanguard_03_Broadsword_Tier2_RunicFlame_32x32_native.png` | `Content/Art/Weapons/WPN_Vanguard_03_Broadsword_Tier2_RunicFlame.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (10 colors, clean alpha, 10 o'clock light) |
| `Vanguard_04_Broadsword_Tier2_GlacialIce_32x32_native.png` | `Content/Art/Weapons/WPN_Vanguard_04_Broadsword_Tier2_GlacialIce.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (9 colors, clean alpha, 10 o'clock light) |
| `Vanguard_05_Broadsword_Tier3_DivineAscendant_32x32_native.png` | `Content/Art/Weapons/WPN_Vanguard_05_Broadsword_Tier3_DivineAscendant.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (10 colors, clean alpha, 10 o'clock light) |
| `Vanguard_06_Broadsword_Combat_Bloodied_32x32_native.png` | None (Held in legacy) | 32x32 | **`REWORK_REQUIRED`** | None (rejected) | FAIL: 6 semi-transparent pixels (a=240) in blood splatter overlay |
| `Vanguard_07_Solar_Fire_Greatsword_32x32_native.png` | `Content/Art/Weapons/WPN_Vanguard_07_Solar_Fire_Greatsword.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (10 colors, clean alpha, 10 o'clock light) |
| `Vanguard_08_Ornate_Silver_Rapier_32x32_native.png` | `Content/Art/Weapons/WPN_Vanguard_08_Ornate_Silver_Rapier.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (11 colors, clean alpha, 10 o'clock light) |
| `Vanguard_09_Iron_Round_Shield_32x32_native.png` | `Content/Art/Weapons/WPN_Vanguard_09_Iron_Round_Shield.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (13 colors, clean alpha, 10 o'clock light) |
| `Vanguard_10_Knight_Heraldic_Kite_Shield_32x32_native.png` | `Content/Art/Weapons/WPN_Vanguard_10_Knight_Heraldic_Kite_Shield.png` | 32x32 | **PASS** | Normalized name to WPN_ prefix, generated sidecar JSON metadata | Approved 32x32 native weapon (13 colors, clean alpha, 10 o'clock light) |

---

## 3. Quy Trình Kỹ Thuật (Pipeline)

```text
Legacy Source PNG (Art_Gallery/legacy/weapons/*_32x32_native.png)
                       │
                       ▼
             INSPECTION & VALIDATION
   (Kích thước 32x32, Kênh Alpha, Zero-Mixel, Bảng màu 4-32)
                       │
                       ▼
          ASEPRITE EXPORT PIPELINE EXTENSION
  (Tools/Aseprite/aseprite_export_pipeline.py --weapons)
                       │
                       ▼
              ART GATE VERIFICATION
  - [x] Lưới pixel chuẩn 32x32 (Không phóng to/thu nhỏ)
  - [x] Độ trong suốt nhị phân (Alpha strictly 0 hoặc 255)
  - [x] Nhận diện hình bóng tại 16x16 thumbnail (Bounding Box >= 3x3)
  - [x] Nguồn sáng 10 giờ & bảng màu Medieval Dark Fantasy
                       │
           ┌───────────┴───────────┐
           ▼                       ▼
     [ APPROVED ]            [ REWORK_REQUIRED ]
           │                       │
           ▼                       ▼
   Content/Art/Weapons/     Giữ nguyên trong
   (WPN_<name>.png + meta)  Art_Gallery/legacy/weapons/
```

---

## 4. Báo Cáo Lỗi & Xử Lý Thất Bại (Failures & Remediation)

Chỉ có **1 tài sản duy nhất không vượt qua Art Gate**:
- **Tệp vi phạm**: `Vanguard_06_Broadsword_Combat_Bloodied_32x32_native.png`
- **Nguyên nhân kỹ thuật**: Phát hiện 6 điểm ảnh bán trong suốt (`alpha = 240`) tại các tọa độ `(25, 6)`, `(24, 7)`, `(23, 8)`, `(22, 9)`, `(19, 12)`, `(18, 13)` do hiệu ứng vệt máu loang tạo bằng cọ mềm (Soft Brush/Opacity Layer).
- **Vi phạm quy chuẩn**: SPEC-ART-2026-09-23-V2 & Art Gate Criteria cấm điểm ảnh bán trong suốt (Unintended Semi-Transparent Pixels) vì sẽ gây vỡ pixel và viền răng cưa khi đổ bóng Lumen trong Unreal Engine.
- **Hành động xử lý**: Giữ nguyên tệp gốc trong `Art_Gallery/legacy/weapons/`, tuyệt đối **không tự ý sửa pixel** và **không đưa vào production**. Chuyển sang trạng thái `REWORK_REQUIRED` để họa sĩ chỉnh sửa bằng bảng màu Crimson Ramp (`#800818` / `#D02020`) với alpha = 255.

---

## 5. Thay Đổi Công Cụ (Tooling Changes)

- **Tệp sửa đổi**: `Tools/Aseprite/aseprite_export_pipeline.py`
- **Mục đích**: Bổ sung hàm `validate_and_process_weapon()` và tham số dòng lệnh `--weapons` (`-w`).
- **Lý do**: Pipeline ban đầu của Gói 2 chỉ hỗ trợ tệp `.aseprite` động đa khung hình của nhân vật (`master_rig_*.aseprite`). Bổ sung chế độ `--weapons` cho phép pipeline xử lý các asset vũ khí tĩnh 32x32 px, tự động kiểm tra kênh alpha, kiểm định silhouette 16x16, chuẩn hóa tiền tố `WPN_` và xuất tệp metadata `_meta.json` kèm theo.
- **Tính tương thích ngược**: 100% chức năng xuất xưởng character rig cũ giữ nguyên vẹn không bị ảnh hưởng.

---

## 6. Xác Thực Hệ Thống (Validation Results)

- **Lệnh kiểm tra tài sản nguồn**: `find Art_Gallery/legacy/weapons -type f -name "*_32x32_native.png" | sort` $\rightarrow$ Đủ 23 tệp gốc.
- **Bảo tồn nguồn cũ**: 100% tệp gốc trong `Art_Gallery/legacy/weapons/` được bảo tồn nguyên vẹn (0 tệp bị xóa, 0 tệp bị đổi nội dung).
- **Tài sản xuất xưởng**: 22 tệp PNG + 22 tệp JSON tại `Content/Art/Weapons/`.
- **Biên dịch UBT Linux**: Hoàn thành thành công (Clean build, 0 error).
- **Kiểm thử tự động Unreal Editor CLI**: `ProjectAscendant.Combat.RegressionHardening` $\rightarrow$ **100% SUCCESS**.
- **Kiểm tra gãy tham chiếu (Broken References)**: 0.