# UX Spec: Chọn Nhân Vật (Character Select — `UPACharacterSelectWidget`)

> **Status**: In Design — MOCKUP CHỜ CHỦ DỰ ÁN DUYỆT (Z1)
> **Author**: ux-designer (Z1, chế độ tự vận hành)
> **Last Updated**: 2026-10-10
> **Journey Phase(s)**: Khởi động game → trước khi vào thế giới
> **Platform Target**: PC (bàn phím + chuột, chính) · Gamepad (Full, CommonUI)
> **Template**: UX Spec
> **Mockup**: `design/ux/mockups/ascendant-ui-mockups.html` — màn 3
> **Nguồn yêu cầu**: `production/DECISIONS.md` §1, §2, §3, §7 · `design/gdd/foundational-classes.md` §1 (bảng chỉ số), §2 · `design/gdd/advanced-classes.md` dòng 16-19 (điều kiện mở khóa từng Bậc) · `design/gdd/authentication-account-system.md` §3.2, §3.3 (bảng `characters`) · `design/art/art-bible.md` §3.1 (dáng hình class)

---

## Purpose & Player Need

Người chơi vừa đăng nhập muốn chọn lối chơi rồi vào thế giới. Màn này cho thấy **toàn bộ cây 16 class** (DECISIONS §2) để người chơi hiểu mình sẽ đi đâu, nhưng **chỉ 4 class Bậc T1 chọn được khi tạo nhân vật**: Vanguard, Ranger, Arcanist, Acolyte. Căn cứ: `foundational-classes.md` §1 ("Tất cả 4 Class cơ bản được mở khóa miễn phí ngay từ màn hình tạo nhân vật") và `advanced-classes.md` dòng 16-19 (T1 có sẵn khi tạo; T2 mở qua Quyển Trục Tinh Anh hoặc kỳ ngộ; T3 qua Quyển Trục Lãnh Chúa; T4 qua Thử Thách No-Hit và tiến trình đa nhánh).

## Player Context on Arrival

Vừa đăng nhập thành công, tò mò, sẵn sàng đầu tư thời gian đọc. Không áp lực thời gian. Người chơi mới chưa biết các class — cần mô tả ngắn, chỉ số và vũ khí.

## Navigation Position

`LoginMap` → **CharacterSelectMap / Chọn Nhân Vật** → OpenWorldMap (`L_VerdantFrontier_Outpost`, `TargetGameWorldMapName` trong `PACharacterSelectWidget.h`).

## Entry & Exit Points

| Entry Source | Trigger | Player carries this context |
|---|---|---|
| Login | `OnLoginSuccess` → `UPALoginWidget::OpenCharacterSelection()` | `FPAAccountProfile` (AccountID, DisplayName, AuthToken) |
| Dev Playtest | `LoginFastPlaytest` thành công | Profile dev |

| Exit Destination | Trigger | Notes |
|---|---|---|
| Thế giới (`L_VerdantFrontier_Outpost`) | Bấm "Vào Thế Giới" → `ConfirmSelectionAndEnterWorld()` | Ghi class đã chọn qua `UPAAccountSubsystem::SetSelectedCharacterClass(ClassTag)`; đổi map bằng `OpenLevel` |
| Login | Bấm "Đăng Xuất" / `Esc`/`B` | Modal xác nhận → `Logout()` |

---

## Layout Specification

### ASCII Wireframe

```
┌──────────────────────────────────────────────────────────────────────────┐
│ CHỌN NHÂN VẬT                                   Tài khoản: AscendantKnight│
├───────────────────────────┬──────────────────────────────────────────────┤
│ CÂY CHỨC NGHIỆP (16)      │   ┌────────────┐  VANGUARD — Tiên Phong       │
│ Hộ Vệ (Guard)             │   │  sprite    │  Bậc T1 · Nhánh Hộ Vệ        │
│  [T1 Vanguard ●]          │   │  idle 5    │  Vũ khí: Kiếm 1 tay +        │
│   T2 Templar 🔒 Berserker🔒│   │  hướng     │  Khiên Sắt Vuông             │
│      Swordmaster 🔒        │   └────────────┘  "Pháo đài thép..."          │
│   T3 Dragon Knight🔒 Void… │   Máu       ██████████░░ 120                 │
│ Du Hiệp (Scout)           │   Thể lực   █████████░░░ 110                 │
│  [T1 Ranger]  T2 … T3 …🔒 │   Mana      ██████░░░░░░  80                 │
│ Pháp Sư (Caster)          │   Thế đứng  ██████████░░ 120                 │
│  [T1 Arcanist] T2 … T3 …🔒│   Tốc chạy  520 cm/s                         │
│ Tín Đồ (Faith)            │   Kỹ năng: Q Khiên Kích · E Kiếm Khí Trảm    │
│  [T1 Acolyte]  T2 … T3 …🔒│   🔒 Templar: mở bằng Quyển Trục Tinh Anh   │
│ Apex: T4 God Slayer 🔒    │                                              │
├───────────────────────────┴──────────────────────────────────────────────┤
│ [Esc] Đăng xuất   [◀ ▶ / LB RB] Đổi class   [Enter / A] VÀO THẾ GIỚI     │
└──────────────────────────────────────────────────────────────────────────┘
```

### Layout Zones

| Zone | Nội dung | Vị trí |
|---|---|---|
| Z1 Header | Tiêu đề + tên tài khoản (`DisplayName`) | Trên |
| Z2 Cây chức nghiệp | 5 nhánh, 16 ô class; 4 ô T1 chọn được, 12 ô khóa | Cột trái (40%) |
| Z3 Chi tiết | Sprite preview, tên, Bậc, nhánh, vũ khí, mô tả, 4 thanh chỉ số + tốc chạy, kỹ năng chính | Cột phải (60%) |
| Z4 Action bar | Đăng xuất, đổi class, Vào Thế Giới | Đáy |

Ở 1280px trở xuống: Z2 thu thành dải 4 thẻ T1 nằm ngang phía trên, nút "Xem cây 16 class" mở cây đầy đủ dạng Modal.

### Component Inventory

| Component | Loại | Nội dung | Tương tác | Pattern |
|---|---|---|---|---|
| Ô class T1 (×4) | Card button | Icon dáng hình (art-bible §3.1), tên, nhánh | Có | Button (Secondary) + trạng thái Selected (mới: Selectable Card) |
| Ô class khóa (×12) | Card button (disabled-focusable) | Tên, Bậc, icon khóa | Focus được để đọc điều kiện mở, không chọn được | Mới: Locked Card |
| Sprite preview | Image | Idle 5 hướng (DECISIONS §10) | Không (xoay hướng bằng `Q/E` — đề xuất) | — |
| Thanh chỉ số (×4) | Stat bar + số | Max HP / Stamina / Mana / Posture | Không | Health & Posture Bar (dạng tĩnh) |
| Danh sách kỹ năng | Text list | `KeyAbilities` | Không | — |
| Nút "Vào Thế Giới" | Button | — | Có | Button (Primary) |
| Nút "Đăng Xuất" | Button | — | Có | Button (Secondary) |

Bảng 4 class chọn được (nguồn: `foundational-classes.md` §1; vũ khí theo DECISIONS §7):

| Class (T1) | Nhánh | Vũ khí | Máu | Thể lực | Mana | Thế đứng | Tốc chạy |
|---|---|---|---|---|---|---|---|
| Vanguard | Hộ Vệ (Guard) | `Weapon.1H.Blade` + `Item.Shield.Square` | 120 | 110 | 80 | 120 | 520 cm/s |
| Ranger | Du Hiệp (Scout) | `Weapon.2H.Bow` | 90 | 120 | 90 | 90 | 570 cm/s |
| Arcanist | Pháp Sư (Caster) | `Weapon.2H.Staff` | 85 | 90 | 140 | 80 | 530 cm/s |
| Acolyte | Tín Đồ (Faith) | `Weapon.1H.Mace` | 110 | 100 | 110 | 110 | 530 cm/s |

Các class còn lại bị khóa (16 − 4 = mười hai; DECISIONS §2): T2 Templar, Berserker, Swordmaster, Shadowblade, Elementalist, Inquisitor; T3 Dragon Knight, Void Blade, Phantom Stalker, Chronomancer, Seraph; T4 God Slayer. Mỗi ô khóa hiện dòng điều kiện mở theo `advanced-classes.md` dòng 17-19, không thêm chi tiết khác.

### Information Hierarchy

1. Class đang chọn (tên + sprite). 2. Vai trò + vũ khí. 3. Bốn chỉ số. 4. Kỹ năng chính. 5. Cây 16 class (khám phá). 6. Điều kiện mở khóa (khi focus ô khóa).

---

## States & Variants

| State / Variant | Trigger | What Changes |
|---|---|---|
| Default | Màn mở | Chọn sẵn Vanguard (`CurrentlySelectedClass` mặc định), focus ô Vanguard |
| Đổi class | `SelectClass` / `SelectNextClass` / `SelectPreviousClass` | Z3 cập nhật qua `OnClassSelectionChanged` / `OnClassSelectedBP`, sprite cross-fade 0.15s |
| Focus ô khóa | Di focus vào class T2-T4 | Z3 hiện tên, Bậc, nhánh, điều kiện mở; nút "Vào Thế Giới" vẫn giữ class T1 đang chọn |
| Đang vào thế giới (loading) | `ConfirmSelectionAndEnterWorld` | Khóa input, overlay "Đang vào Tiền Trạm Verdant…" tới khi map mới nạp |
| Lỗi nạp map | `OpenLevel` thất bại / mất kết nối | Quay lại màn với thông báo lỗi + nút Thử lại (hiện chưa có đường lỗi trong code) |
| Thiếu dữ liệu class | `GetAvailableClasses()` rỗng | Thông báo "Không tải được danh sách chức nghiệp" + Thử lại; nút Vào Thế Giới khóa |
| Thiếu sprite | Asset chưa có (`SpritesheetAssetPath` rỗng) | Hiện bóng dáng hình khối theo art-bible §3.1 thay sprite |

## Interaction Map

| Component | Bàn phím / Chuột | Gamepad | Phản hồi | Kết quả |
|---|---|---|---|---|
| Ô class T1 | Click; `←/→` hoặc `A/D` | `LB/RB` hoặc D-pad | Viền Amber + `SFX_UI_Focus` | `SelectClass(EPACharacterClass)` |
| Ô class khóa | Click / di focus | D-pad | Viền xám, icon khóa, `SFX_UI_Ability_Cooldown` khi bấm | Chỉ hiện điều kiện, không chọn |
| Xoay sprite (đề xuất) | `Q/E` | Right stick | Đổi hướng | Chỉ hiển thị |
| Vào Thế Giới | `Enter` / click | `A` (giữ focus) hoặc `Start` | `SFX_UI_Confirm` | `ConfirmSelectionAndEnterWorld()` |
| Đăng Xuất | `Esc` / click | `B` | Modal xác nhận | `Logout()` → Login |

## Events Fired

| Player Action | Event Fired | Payload / Data |
|---|---|---|
| Đổi class | `OnClassSelectionChanged(FPACharacterClassInfo)` | ClassType, ClassTag |
| Vào thế giới | `OnEnteringWorldBP(MapName)`; `SetSelectedCharacterClass(ClassTag)` | `Class.Line.<Nhánh>.<Class>`, map |
| Analytics | Không có | — |

Ghi bền vững: class chính của nhân vật (`characters.primary_class_tag`, GDD auth §3.3). Hiện chỉ lưu trong `UPAAccountSubsystem` (bộ nhớ); backend Giai đoạn 6.

## Transitions & Animations

- Vào màn: fade 0.3s; cây class hiện theo nhánh (stagger 40ms) — tắt khi Reduced Motion.
- Đổi class: cross-fade sprite + thanh chỉ số chạy tới giá trị mới 0.2s.
- Rời màn: fade đen 0.4s → nạp map.

## Data Requirements

| Data | Source System | Read / Write | Notes |
|---|---|---|---|
| Danh sách class | `FPACharacterClassRegistry::GetAllClasses()` → `UPACharacterSelectWidget::AvailableClasses` | Read | **Gap:** registry chỉ có 3 class, thiếu Acolyte (`EPACharacterClass` trong `PACharacterSelectTypes.h`) |
| Thông tin class | `FPACharacterClassInfo` (DisplayName, Tagline, Lore, Role, Weapon, Base*, MoveSpeed, KeyAbilities, SpritesheetAssetPath) | Read | Số liệu khớp `foundational-classes.md` §1 cho 3 class hiện có |
| Các class khóa (T2–T4) | DECISIONS §2 + `advanced-classes.md` | Read | **Gap:** không có model nào cho class khóa; cần dữ liệu tĩnh (tên, Bậc, nhánh, điều kiện) |
| Class đã chọn | `UPAAccountSubsystem::SetSelectedCharacterClass(FName)` | Write | Ghi ClassTag |
| Tên tài khoản | `FPAAccountProfile::DisplayName` | Read | — |

## Input Method Completeness Checklist

**Bàn phím:** [x] `←/→` đổi class T1 · [x] `Tab` duyệt cả cây (gồm ô khóa) · [x] `Enter` xác nhận · [x] `Esc` đăng xuất có xác nhận
**Chuột:** [x] click thẻ · [x] hover hiện điều kiện mở · [x] vùng bấm ≥ 48px
**Gamepad:** [x] `LB/RB` đổi class T1 · [x] D-pad duyệt cây · [x] `A` xác nhận · [x] `B` lùi · [ ] xoay sprite bằng right stick (đề xuất)

## Accessibility

Tier Standard. Ô khóa = icon ổ khóa + chữ "Khóa" + giảm độ sáng, không chỉ màu. Thanh chỉ số luôn kèm số. Mỗi class có dáng hình riêng (art-bible §3.1) nên nhận ra được khi mù màu. Chữ ≥ 24px; tương phản ≥ 4.5:1. Không giới hạn thời gian.

## Localization Considerations

- Tên class giữ tiếng Anh (DECISIONS §2) + tên tiếng Việt trong ngoặc; khung tên phải chứa "Phantom Stalker (U Hồn Đoạt Mệnh)" — dài nhất, HIGH PRIORITY.
- Mô tả lore có thể dài: vùng chữ cuộn được, không cắt.
- Đơn vị "cm/s" và số định dạng theo locale.

## Acceptance Criteria

- [ ] Màn hiện trong ≤ 1.0s sau khi đăng nhập thành công, Vanguard được chọn sẵn.
- [ ] Đủ 16 ô class theo DECISIONS §2; đúng 4 ô T1 chọn được; 12 ô còn lại có icon khóa + dòng điều kiện mở.
- [ ] Chọn từng class T1 hiển thị đúng 4 chỉ số và tốc chạy như bảng `foundational-classes.md` §1 (vd. Arcanist: Máu 85, Mana 140).
- [ ] Bấm ô khóa không đổi class đang chọn và không cho vào thế giới bằng class đó.
- [ ] "Vào Thế Giới" ghi ClassTag `Class.Line.Faith.Acolyte` khi chọn Acolyte và nạp `L_VerdantFrontier_Outpost`.
- [ ] Danh sách class rỗng → thông báo lỗi + Thử lại, nút Vào Thế Giới khóa.
- [ ] Toàn màn dùng được bằng bàn phím và gamepad, focus luôn thấy được.

## Open Questions

- **Q1 (gap code):** Thêm `Acolyte` vào `EPACharacterClass` và `FPACharacterClassRegistry` (hiện 3 class). Việc này nằm trong Z2.
- **Q2 (Class Phụ):** DECISIONS §3 yêu cầu mỗi nhân vật có 1 Class Chính + 1 Class Phụ. Không file nào nói Class Phụ được chọn lúc tạo nhân vật hay sau này. Mockup chỉ chọn Class Chính. Chủ dự án quyết định.
- **Q3 (danh sách nhân vật):** Bảng `characters` (GDD auth §3.3) cho phép nhiều nhân vật mỗi tài khoản và có `character_name` bắt buộc, nhưng chưa GDD nào mô tả danh sách nhân vật hay ô đặt tên. Mockup có ô "Tên nhân vật" đánh dấu "đề xuất — chờ duyệt"; code hiện không có.
- **Q4 (tên tiếng Việt):** DECISIONS §2 ghi Vanguard = Tiên Phong, Ranger = Xạ Thủ, Arcanist = Bí Thuật Sư, Acolyte = Tập Sự; `foundational-classes.md` và code ghi Chiến Binh, Du Hiệp, Thuật Sĩ, Tu Sĩ. Mockup theo DECISIONS (tài liệu đã khóa). Cần chốt một bộ tên.
- **Q5:** `UPACharacterSelectWidget` là `UUserWidget` + Slate, phải chuyển CommonUI (control-manifest §4). Cũng chỉ có 3 border thẻ cứng (`VanguardCardBorder`, `RangerCardBorder`, `ArcanistCardBorder`) — cần dựng danh sách động.
- **Q6:** Luồng map (`CharacterSelectMap` theo GDD auth §3.2 vs một map `L_FrontEnd` theo plan Z2) — xem `login.md` Q1.
