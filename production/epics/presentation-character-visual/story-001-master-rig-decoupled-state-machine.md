# Story 001: Master Rig Architecture & Decoupled State Machine Binding

> **Epic**: `EPIC-CHARACTER-VISUAL-001` (Character & NPC Visual Identity System)  
> **Story ID**: `visual-001`  
> **Layer**: Presentation / Animation Architecture  
> **Type**: Architecture & Code (100% Placeholder Testable)  
> **Estimate**: 1.5 days (12 hours)  
> **Status**: Ready for Dev  
> **Owner**: Gameplay Programmer  

---

## 1. Bối Cảnh & Mục Tiêu Kỹ Thuật

Hiện tại nhân vật `APABaseCharacter` hiển thị đơn lớp qua `UPaperFlipbookComponent`. Để hỗ trợ hệ thống 12 Class và 7 Weapon Families mà không phải vẽ riêng 8.400 frame, story này kiến trúc hóa **Mô hình Hoạt Ảnh Đa Tầng Phân Tách (Decoupled Layered State Machine)** trong C++ và PaperZD:
- Phân chia nhân vật thành 2 kênh hoạt ảnh độc lập:
  1. **Lower Body (Chân & Thắt Lưng)**: Quản lý di chuyển (`Idle`, `Walk`, `Run`, `Dash`, `HitStun`), ánh xạ theo **4 Master Rigs** (`HeavyTank`, `Agility`, `Caster`, `Monk`).
  2. **Upper Body (Thân Trên & Tay)**: Quản lý chiến đấu (`AttackCombo`, `Parry`, `Charge`), ánh xạ trực tiếp theo **7 Weapon Families** (`1H.Blade`, `2H.Heavy`, `2H.Polearm`, `2H.Bow`, `Dual.Daggers`, `2H.Staff`, `1H.Mace`).
- Quản lý đồng bộ khung hình (Frame Lockstep Synchronization) và vị trí gắn nối tại thắt lưng ($Y=80$) giữa hai tầng.
- **Tiêu chuẩn kiểm thử**: Sử dụng 100% Dummy Placeholder Flipbooks (Khung ảnh đơn sắc $128 \times 128$ màu Xanh cho Lower Body, màu Đỏ cho Upper Body) để chạy unit test headless tự động.

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [ ] **AC-1 (4 Master Rig Enum & State Machine)**:
  - Định nghĩa enum `EPAMasterRigType : uint8 { HeavyTank, Agility, Caster, Monk }`.
  - Hàm `UPAPaperdollComponent::SetMasterRig(EPAMasterRigType InRig)` khởi tạo và gán đúng bộ Flipbook Lower Body tương ứng.
  - Tọa độ chân tiếp đất luôn khóa cứng tại `FPAPaperdollConstants::FootPivot` $(64, 114)$.
- [ ] **AC-2 (7 Weapon Family Upper Body Binding)**:
  - Hàm `UPAPaperdollComponent::SetUpperBodyWeaponFamily(EPAWeaponFamily InFamily)` hoán đổi Upper Body Flipbook sang bộ hoạt ảnh của vũ khí đó mà không làm gián đoạn trạng thái bước chân của Lower Body.
  - Các socket tay `HandSocket_R` $(96, 76)$ và `HandSocket_L` $(32, 76)$ di chuyển đồng bộ theo từng khung hình của Upper Body Flipbook.
- [ ] **AC-3 (Frame Lockstep & Directional Sync)**:
  - Khi nhân vật xoay qua 8 hướng (`EPAAimDirection8Way`), cả Lower Body và Upper Body đồng thời chuyển hướng và áp dụng `FPAPaperdollSortKey`.
  - Tốc độ phát (Playback Speed) và frame index của Upper Body và Lower Body được đồng bộ hóa, không bị lệch pha khi chuyển từ `Walk` sang `Run`.
- [ ] **AC-4 (Headless Unit Tests Pass 100%)**:
  - Triển khai `PACharacterVisualRigTests.cpp` kiểm thử tự động trong môi trường headless (`-nullrhi -nosound`):
    - Đổi qua lại giữa 4 Master Rigs và 7 Weapon Families xác nhận pointer component và visual asset ID cập nhật chính xác.
    - Kiểm tra không xảy ra memory leak hay crash dereference khi Upper Body hoặc Lower Body flipbook là nullptr.
