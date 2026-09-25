# Story 003: Civilian NPC Component, Dynamic Palette Swap & Town Guard AI

> **Epic**: `EPIC-CHARACTER-VISUAL-001` (Character & NPC Visual Identity System)  
> **Story ID**: `visual-003`  
> **Layer**: Presentation & AI Gameplay Layer  
> **Type**: Architecture & Code (100% Placeholder Testable)  
> **Estimate**: 1.0 day (8 hours)  
> **Status**: Completed  
> **Owner**: Systems Designer & Gameplay Programmer  

---

## 1. Bối Cảnh & Mục Tiêu Kỹ Thuật

Khu vực an toàn tại 3 Tòa Thành (*Verdant Bastion*, *Ashen Keep*, *Sanctum Fortress*) cần một hệ thống NPC dân sự sống động nhưng tuyệt đối không làm bùng nổ ngân sách đồ họa.
Story này hiện thực hóa:
1. **`UPACivilianNPCComponent`**: Quản lý 5 vai trò dân sự (`Blacksmith`, `Merchant`, `Villager`, `TownGuard`, `QuestGiver`) kế thừa hoạt ảnh từ 4 Master Rigs có sẵn (không sinh Rig thứ 5).
2. **Master Material `M_PaperZD_Civilian_Base`**: Hỗ trợ Dynamic Material Instance (MID) cho phép đổi màu vải trang phục (Palette Swap) theo ZoneId mà không tốn thêm sprite vẽ tay.
3. **Prop Socket Attachment**: Gắn các đạo cụ tĩnh 1-frame (Búa rèn, Túi tiền, Giỏ bánh, Giáo thành) vào `HandSocket_R` và `HandSocket_L`.
4. **Hành vi Lính Gác Thị Trấn (Town Guard Dual-State AI)**:
   - Bình thường (Passive): Đứng nghiêm hoặc tuần tra (`Idle` / `Walk`).
   - Gặp Outlaw (Active): Khi người chơi có `bIsOutlaw = true` (Karma < 0 / Cờ Wanted) tiến vào bán kính $1000\text{ cm}$ cổng thành, kích hoạt đòn đâm chí mạng Knockback đẩy lùi bằng cách **tái dùng trực tiếp `FB_Upper_2H_Polearm_Combo` (Family 3: Polearms) trên Rig 1 (Heavy Tank)**.

> 💡 **SẴN SÀNG CODE NGAY (ZERO BLOCKER)**: Dependency `zone-001` (`UPACitadelComponent` và `FPAZoneModel`) đã được implement hoàn chỉnh trong codebase tại [`PACitadelComponent.h`](file:///mnt/Data/Projects/project-games/ProjectAscendant/Source/ProjectAscendant/Public/World/PACitadelComponent.h) và vượt qua 100% test `ProjectAscendant.World.CitadelSafeZonesAndAutoSave`. Story `visual-003` gọi trực tiếp API `IsInsideSafeZone()`, `GetSafeZoneRadius()` (5000cm) và `GetCitadelId()` mà không cần viết lại logic hay chờ đợi hệ thống nào khác!

---

## 2. Tiêu Chí Nghiệm Thu (Acceptance Criteria)

- [x] **AC-1 (5 Vai Trò NPC Dân Sự & Rig Mapping)**:
  - Định nghĩa enum `EPACivilianRole : uint8 { Blacksmith, Merchant, Villager, TownGuard, QuestGiver }`.
  - Khởi tạo `UPACivilianNPCComponent` tự động gán đúng Master Rig theo thiết kế:
    - Thợ Rèn, Lính Gác $\rightarrow$ `EPAMasterRig::HeavyTank`.
    - Thương Nhân, Dân Làng $\rightarrow$ `EPAMasterRig::Agility` / `Caster`.
    - Quest Giver $\rightarrow$ `EPAMasterRig::Caster`.
- [x] **AC-2 (Dynamic Palette Swap Material)**:
  - Master Material `M_PaperZD_Civilian_Base` nhận tham số `ZoneColorRamp` (Vector Parameter) trong Dynamic Material Instance.
  - Hàm `SetZonePalette(FName ZoneId)` tự động nạp palette màu tương ứng:
    - `Verdant_Bastion`: Tông nâu/rêu vải thô.
    - `Ashen_Keep`: Tông xám tro/đỏ gạch nung.
    - `Sanctum_Fortress`: Tông trắng ngà/lam ngọc hoàng kim.
- [x] **AC-3 (Đạo Cụ Cầm Tay & Frame Callout Quest Giver)**:
  - Hàm `AttachCivilianProp(FName PropVisualId, bool bLeftHand)` gắn sprite đạo cụ tĩnh vào đúng Hand Socket.
  - Quest Giver hỗ trợ trigger trạng thái `PlayCalloutAnimation()`, chuyển sang frame vẫy tay kêu gọi và bật widget dấu chấm than `!` màu vàng trên đầu.
- [x] **AC-4 (Town Guard Outlaw Deterrence & Combat Reuse)**:
  - Khi Target Actor là người chơi có `bIsOutlaw == true` trong bán kính $1000\text{ cm}$:
    - Town Guard lập tức chuyển State Machine sang `CombatActive`.
    - Gọi Upper Body Animation `FB_Upper_2H_Polearm_Combo` vung ngọn giáo `Prop_Guard_CitySpear` tấn công mục tiêu với thuộc tính Knockback.
    - Tốn **0 frame vẽ mới**, tái sử dụng 100% asset của Weapon Family 3.
- [x] **AC-5 (Headless Unit Tests Pass 100%)**:
  - Triển khai test trong `PACivilianNPCTests.cpp`:
    - Khởi tạo 5 loại NPC, kiểm tra mapping Master Rig và Palette Swap theo 3 Zones.
    - Giả lập một `APABaseCharacter` với Karma = -50 (Outlaw) bước vào phạm vi $800\text{ cm}$ của Town Guard, xác nhận Guard kích hoạt đúng đòn đánh `FB_Upper_2H_Polearm_Combo`.
