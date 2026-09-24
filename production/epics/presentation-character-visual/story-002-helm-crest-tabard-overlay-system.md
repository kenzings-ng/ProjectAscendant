# Story 002: Helm Crest Socket & Tabard Cutout Component Integration

> **Epic**: `EPIC-CHARACTER-VISUAL-001` (Character & NPC Visual Identity System)  
> **Story ID**: `visual-002`  
> **Layer**: Presentation / Modular Paperdoll Extension  
> **Type**: Architecture & Code (100% Placeholder Testable)  
> **Estimate**: 1.0 day (8 hours)  
> **Status**: Ready for Dev  
> **Owner**: Gameplay Programmer  

---

## 1. Bối Cảnh & Mục Tiêu Kỹ Thuật

Theo đặc tả [`character-visual-system.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/gdd/character-visual-system.md), khi người chơi mặc full bộ giáp sắt dã chiến `Armor_Heavy_T1` hoặc `Armor_Medium_T1`, toàn bộ cơ thể bị che kín dẫn đến nguy cơ các class trông giống hệt nhau.
Story này mở rộng `UPAPaperdollComponent` đã code ở `item-004` để hỗ trợ **2 Socket Phụ Kiện Nhận Diện Độc Quyền (Class Identity Sockets)**:
1. **`Socket_HelmCrest`**: Tọa độ $(X: 64, Y: 40)$ trên canvas $128 \times 128$, gắn các sprite mào nón / sừng giáp (Bờm đỏ Vanguard, Sừng thú Berserker, Cánh thép Templar, Sừng rồng Dragon Knight).
2. **`Socket_Tabard`**: Tọa độ $(X: 64, Y: 60)$ trên canvas $128 \times 128$, gắn các sprite dải cờ / khăn choàng / ấn chú phủ đè lên rãnh khoét (Cutout Channel) của giáp ngực.
3. **Quy tắc phân lớp độ sâu**: Socket Crest và Tabard luôn có Z-order hiển thị nằm đè lên trên `Helm` và `Chest Armor` ở mọi hướng nhìn.

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [ ] **AC-1 (Khởi Tạo Sockets & Sub-Components Mới)**:
  - Khởi tạo hằng số socket trong `FPAPaperdollConstants`:
    - `Socket_HelmCrest = TEXT("Socket_HelmCrest")` tại $(64, 40)$.
    - `Socket_Tabard = TEXT("Socket_Tabard")` tại $(64, 60)$.
  - Tự động tạo 2 `UPaperFlipbookComponent` phụ trợ trong `Initialize9SlotSubcomponents()` với `SnapToTargetNotIncludingScale`.
- [ ] **AC-2 (Auto Class Identity Binding)**:
  - Khi nhân vật nhận thẻ Gameplay Tag Chức Nghiệp (vd `Class.Vanguard`, `Class.Templar`, `Class.Acolyte`), hệ thống tự động gán đúng Asset ID của Crest và Tabard tương ứng mà không cần người chơi phải trang bị thủ công.
  - Khi người chơi tháo/mặc giáp `Helm` hoặc `Chest`, các phụ kiện Crest và Tabard vẫn duy trì hiển thị, không bị ẩn theo giáp.
- [ ] **AC-3 (Directional Sort Key & Mirroring)**:
  - Tích hợp vào `FPAPaperdollSortKey`:
    - `Socket_HelmCrest` luôn có Sort Priority $= 36$ (nằm trên `Helm` có Sort Priority $= 35$).
    - `Socket_Tabard` luôn có Sort Priority $= 26$ (nằm trên `Chest` có Sort Priority $= 25$).
  - Khi xoay sang hướng Tây (`EPAAimDirection8Way::West`), không xảy ra lỗi giáp ngực đè bẹp dải khăn Tabard.
- [ ] **AC-4 (Headless Unit Tests Pass 100%)**:
  - Triển khai test trong `PACharacterVisualIdentityTests.cpp`:
    - Trang bị lần lượt 3 hạng cân giáp (`Armor_Heavy_T1`, `Armor_Medium_T1`, `Armor_Light_T1`) kết hợp đổi qua 12 class tags.
    - Xác nhận cả 12 class đều giữ được Crest và Tabard hiển thị đúng component, không bị crash hay null pointer.
