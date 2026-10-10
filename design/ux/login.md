# UX Spec: Đăng Nhập (Login — `WBP_LoginScreen`, tab Login + khu Dev Playtest)

> **Status**: In Design — MOCKUP CHỜ CHỦ DỰ ÁN DUYỆT (Z1)
> **Author**: ux-designer (Z1, chế độ tự vận hành)
> **Last Updated**: 2026-10-10
> **Journey Phase(s)**: Khởi động game (phiên đầu và mỗi lần mở game)
> **Platform Target**: PC (bàn phím + chuột, chính) · Gamepad (Full, hot-swap qua CommonUI) — theo `design/accessibility-requirements.md` §4.1
> **Template**: UX Spec (`/ux-design`, `.claude/skills/ux-design/references/3-file-skeleton.md`)
> **Mockup**: `design/ux/mockups/ascendant-ui-mockups.html` — màn 1
> **Nguồn yêu cầu**: `design/gdd/authentication-account-system.md` §2.1, §2.2 (1) và (3), §3.2, §4 · `design/art/art-bible.md` §3.4, §4 · `design/ux/interaction-patterns.md` §3.1, §3.8 · `docs/architecture/control-manifest.md` §4 (CommonUI)

---

## Purpose & Player Need

Người chơi mở game muốn vào thế giới nhanh nhất có thể với tài khoản của mình. Màn này nhận Email + Mật khẩu, tùy chọn "Ghi nhớ đăng nhập", báo lỗi rõ ràng và chuyển sang Chọn Nhân Vật khi thành công. Trong bản Development/Playtest, màn này còn chứa khu **Dev Playtest 1-Click** để QA vào game không cần mật khẩu (GDD §2.2 mục 3).

Nếu màn này khó dùng: người chơi kẹt ở cổng vào, QA mất thời gian mỗi lần chạy nhiều client.

## Player Context on Arrival

- Lần đầu: vừa xem logo/khởi động, chưa có tài khoản → cần thấy đường sang tab Đăng Ký.
- Các lần sau: có thể đã có phiên "Ghi nhớ đăng nhập" → `AutoLoginWithSavedSession()` bỏ qua màn này.
- Trạng thái cảm xúc: bình tĩnh, chờ đợi. Không áp lực thời gian.
- Người chơi đến chủ động (mở game) hoặc bị đưa về (Đăng Xuất, phiên hết hạn).

## Navigation Position

`Khởi động` → **LoginMap / `WBP_LoginScreen` (tab Login)** ↔ tab Register (`design/ux/register.md`) → CharacterSelectMap (`design/ux/character-select.md`).

Màn gốc của luồng mở đầu; không có màn cha.

## Entry & Exit Points

| Entry Source | Trigger | Player carries this context |
|---|---|---|
| Khởi động game | Map `LoginMap` nạp, widget được thêm vào viewport | Không có; có thể có phiên đã lưu (`HasSavedSession()`) |
| Tab Đăng Ký | Bấm tab "Đăng Nhập" hoặc đăng ký thành công | Email vừa đăng ký được điền sẵn (đề xuất, xem Open Questions) |
| Trong game | Đăng Xuất (`Logout()`) | Không |

| Exit Destination | Trigger | Notes |
|---|---|---|
| Chọn Nhân Vật | `OnLoginSuccess` | Theo GDD §3.2: `LoginMap → CharacterSelectMap`. Code hiện tại mở `UPACharacterSelectWidget` ngay trên cùng màn (`UPALoginWidget::OpenCharacterSelection`) — xem Open Questions Q1 |
| Tab Đăng Ký | Bấm tab "Đăng Ký" | Không mất dữ liệu đã nhập ở tab Login |
| Chọn Nhân Vật (Dev) | Bấm `[Tester 1]`/`[Tester 2]`/`[Tester 3]` hoặc "Vào Game Ngay" | Profile dev (`bIsDevProfile = true`), không lưu phiên |
| Thoát game | Nút "Thoát" / `Esc` tại gốc | Hỏi xác nhận bằng Modal Dialog |

---

## Layout Specification

### ASCII Wireframe

```
┌──────────────────────────────────────────────────────────────────────┐
│   (nền: khói bóng tối + lửa tím lập lòe — GDD §2.1)                  │
│                         PROJECT ASCENDANT                            │
│                ╱──────────────────────────────────╲                  │
│               │  [ ĐĂNG NHẬP ]   Đăng Ký           │  ← tab bar       │
│               │                                    │                  │
│               │  Email                             │                  │
│               │  [ ten@vidu.com                 ]  │                  │
│               │  Mật khẩu                          │                  │
│               │  [ ••••••••                  (👁)] │  ← nút xem MK    │
│               │  [x] Ghi nhớ đăng nhập             │                  │
│               │  ⚠ Mật khẩu không chính xác…       │  ← status line   │
│               │  [        ĐĂNG NHẬP         ]      │  ← Primary       │
│                ╲──────────────────────────────────╱                  │
│               ┌ DEV PLAYTEST (chỉ bản Development) ┐                 │
│               │ [Tester 1] [Tester 2] [Tester 3]   │                 │
│               │ [Tên tùy chọn      ] [Vào Game Ngay]│                 │
│               └────────────────────────────────────┘                 │
│  [Esc] Thoát      [Enter] Đăng nhập      [Tab] Đổi tab   (gợi ý phím)│
└──────────────────────────────────────────────────────────────────────┘
```

Khung có góc vát 45° (art-bible §3.4). Khung "pixel art gothic mạ vàng đen" (GDD §2.1) được thể hiện bằng viền Amber Gold `#E6A122` trên Obsidian `#121316`.

### Layout Zones

| Zone | Nội dung | Vị trí |
|---|---|---|
| Z1 Nền | Khói, lửa tím (Void Purple `#6A1B9A`), không tương tác | Toàn màn |
| Z2 Tiêu đề | Logo chữ "PROJECT ASCENDANT" | Trên giữa |
| Z3 Thẻ xác thực | Tab bar + form Login | Giữa màn, rộng 480px @1080p |
| Z4 Dev Playtest | Nút Tester + ô tên tùy chọn | Dưới Z3; chỉ hiện khi build Development |
| Z5 Thanh gợi ý phím | Prompt phím đổi theo thiết bị (CommonUI) | Đáy màn |

### Component Inventory

| Component | Loại | Nội dung | Tương tác | Pattern |
|---|---|---|---|---|
| Tab "Đăng Nhập" / "Đăng Ký" | Tab button | Nhãn | Có | Button (Secondary) — `interaction-patterns.md` §3.1 |
| Ô Email | Text input | Email, kiểm tra cú pháp (`IsValidEmail`) | Có | **Mới: Text Input Field** (chưa có trong pattern library) |
| Ô Mật khẩu | Text input (mask `*`) + nút con mắt | Mật khẩu | Có | Text Input Field + Icon toggle (mới) |
| Checkbox "Ghi nhớ đăng nhập" | Checkbox | `bRememberMe` | Có | **Mới: Checkbox** |
| Dòng trạng thái | Text | Lỗi/đang xử lý | Không | Thông báo inline |
| Nút "Đăng Nhập" | Button | — | Có | Button (Primary) |
| Nút `[Tester 1..3]` | Button | Tên tester | Có | Button (Secondary) |
| Ô "Tên Tùy Chọn" + "Vào Game Ngay" | Input + Button | Tên hiển thị dev | Có | Text Input + Button (Primary) |
| Thanh gợi ý phím | Action bar | Phím hiện tại | Không | CommonUI Bound Action Bar |

### Information Hierarchy

1. Ô Email, Mật khẩu và nút Đăng Nhập (việc chính).
2. Dòng trạng thái/lỗi (ngay trên nút, luôn trong tầm mắt).
3. Tab Đăng Ký (đường thoát cho người mới).
4. Ghi nhớ đăng nhập.
5. Khu Dev Playtest (tách biệt bằng khung nét đứt, chỉ bản Development).

---

## States & Variants

| State / Variant | Trigger | What Changes |
|---|---|---|
| Default | Màn nạp, `EPAAuthLoginState::LoggedOut` | Focus vào ô Email |
| Tự đăng nhập | `HasSavedSession()` true | Hiện "Đang khôi phục phiên…" + spinner; nút bị khóa; thành công → Chọn Nhân Vật; thất bại → Default + lỗi |
| Đang xác thực (loading) | `Authenticating` | Nút Đăng Nhập đổi thành spinner + "Đang xác thực…", mọi input khóa; chặn bấm lặp |
| Lỗi nhập liệu | `IsValidEmail` false / mật khẩu rỗng | Viền ô lỗi đổi Crimson + icon cảnh báo + chữ lỗi (không dùng màu đơn độc) |
| Lỗi máy chủ | `Error` với `EPAAuthErrorCode::UserNotFound` / `IncorrectPassword` / `UnknownError` | Dòng trạng thái hiện thông báo từ `OnDisplayStatusMessage(Message, true)`; giữ nguyên Email, xóa Mật khẩu |
| Thành công | `LoggedIn` | Dòng trạng thái "Đăng nhập thành công", SFX chuông đồng (GDD §2.1), chuyển màn sau ≤ 0.3s |
| Bản Shipping | Build không phải Development | Ẩn hoàn toàn Z4 Dev Playtest |
| Empty | Không áp dụng (form không phụ thuộc dữ liệu) | — |

## Interaction Map

Mapping cho: Bàn phím/Chuột và Gamepad (Full).

| Component | Bàn phím / Chuột | Gamepad | Phản hồi | Kết quả |
|---|---|---|---|---|
| Ô Email / Mật khẩu | Click hoặc `Tab`/`Shift+Tab` để vào | D-pad lên/xuống; `A` mở bàn phím ảo | Viền focus vàng nhấp nháy (pattern §3.1 Focused) | Nhập liệu |
| Nút xem mật khẩu | Click / `Alt+V` (đề xuất) | `Y` khi đang ở ô Mật khẩu | Icon con mắt đổi trạng thái | Bật/tắt mask |
| Checkbox ghi nhớ | `Space` / click | `A` | Dấu tích | Đổi `bRememberMe` |
| Nút Đăng Nhập | `Enter` (từ bất kỳ ô nào) / click | `A` khi focus; `Start` = gửi nhanh (đề xuất) | Pressed 98%, `SFX_UI_Click` | `RequestLogin(Email, Password, bRememberMe)` |
| Tab Đăng Ký | Click / `Ctrl+Tab` | `RB` | Trượt tab, tiếng lật sách (GDD §2.1) | `SwitchViewMode(Register)` |
| `[Tester N]` | Click | `A` | Pressed | `RequestFastPlaytest("Tester N")` |
| "Vào Game Ngay" | Click / `Enter` trong ô tên | `A` | Pressed | `RequestFastPlaytest(Tên)` |
| Thoát | `Esc` tại gốc | `B` tại gốc | Mở Modal Dialog xác nhận | Thoát game |

## Events Fired

| Player Action | Event Fired | Payload / Data |
|---|---|---|
| Bấm Đăng Nhập | `UPAAccountSubsystem::LoginWithCredentials` → `OnLoginStateChanged` | Email, bRememberMe (mật khẩu không bao giờ log) |
| Đăng nhập thành công | `OnLoginSuccess(FPAAccountProfile)` | AccountID, DisplayName, AuthToken |
| Dev Playtest | `LoginFastPlaytest(DevDisplayName)` | DisplayName |
| Đổi tab | `OnViewModeChanged(EPALoginViewMode)` | Mode |
| Analytics | Chưa có hệ thống analytics — không bắn sự kiện analytics | — |

Ghi trạng thái bền vững: Ghi nhớ đăng nhập lưu `SavedSessionProfile` (GDD §5: dự kiến SaveGame mã hóa AES ở Vertical Slice).

## Transitions & Animations

- Vào màn: fade-in nền 0.4s, thẻ xác thực trượt lên 16px + fade 0.25s.
- Đổi tab: nội dung trượt ngang 0.2s + tiếng lật sách.
- Rời màn: fade-out 0.3s.
- Lỗi: dòng trạng thái rung ngang 4px trong 0.15s (tắt khi Reduced Motion / Flash Suppression bật).
- Không có nhấp nháy nhanh; spinner xoay chậm (≤ 1 vòng/giây).

## Data Requirements

| Data | Source System | Read / Write | Notes |
|---|---|---|---|
| Email, Mật khẩu, Ghi nhớ | Người chơi → `UPALoginWidget::RequestLogin` → `UPAAccountSubsystem::LoginWithCredentials` | Write | Mật khẩu chỉ đi qua bộ nhớ, không hiển thị lại |
| Trạng thái đăng nhập | `UPAAccountSubsystem::OnLoginStateChanged(EPAAuthLoginState, FString)` | Read | `LoggedOut / Authenticating / LoggedIn / Error` |
| Mã lỗi | `EPAAuthErrorCode` | Read | Map sang chuỗi đã bản địa hóa (xem Localization) |
| Có phiên đã lưu | `HasSavedSession()`, `AutoLoginWithSavedSession()` | Read | — |
| Chế độ xem | `UPALoginWidget::CurrentViewMode` (`EPALoginViewMode`) | Read/Write | — |
| Cờ build Development | Cấu hình build | Read | Quyết định hiện Z4 |

**Ghi chú bảo mật (bắt buộc sửa ở Z2):** stub hiện lưu mật khẩu dạng chữ thường: `Source/ProjectAscendant/Private/Account/PAAccountSubsystem.cpp:93` (`RegisteredCredentials.Add(InEmail, InPassword)`), so khớp chữ thường ở `:125-126`, và tài khoản mặc định ở `:21`. Stub phải lưu và so khớp **hash có salt** (không lưu mật khẩu thô), đúng với cột `password_hash` của GDD §3.3. Đây là sửa lỗi bảo mật của Z2, không phải hệ thống mới.

## Input Method Completeness Checklist

**Bàn phím**
- [x] Mọi control tới được bằng `Tab`/`Shift+Tab`, thứ tự: Email → Mật khẩu → Xem MK → Ghi nhớ → Đăng Nhập → Tester 1..3 → Tên tùy chọn → Vào Game Ngay
- [x] `Enter` gửi form từ ô nhập
- [x] `Esc` tại gốc mở xác nhận thoát
- [ ] Phím tắt nút xem mật khẩu (`Alt+V`) — chờ duyệt

**Chuột**
- [x] Vùng bấm tối thiểu 48×48px (pattern §3.1)
- [x] Hover state cho mọi nút

**Gamepad**
- [x] D-pad/thumbstick điều hướng dọc; `RB/LB` đổi tab; `A` chọn; `B` lùi
- [x] Focus nhìn thấy được (góc vàng nhấp nháy)
- [ ] Bàn phím ảo nền tảng khi `A` vào ô nhập — chưa xác minh trên UE 5.8 (Open Questions)
- [x] Rút tay cầm: CommonUI chuyển prompt về bàn phím trong 1 frame (combat-hud Edge Cases)

Touch: không hỗ trợ (accessibility-requirements.md không cam kết touch).

## Accessibility

Tier cam kết: **Standard** (`design/accessibility-requirements.md` §1).
- Chữ body ≥ 24px @1080p, tương phản ≥ 4.5:1 (Bone White `#E8ECEB` trên `#121316` ≈ 15:1); tiêu đề ≥ 7:1.
- Lỗi = viền + icon cảnh báo + chữ, không chỉ màu đỏ (§2.2).
- Hỗ trợ UI Scaling 80–150% (§5.1): thẻ xác thực co giãn theo scale, không cắt chữ.
- Reduced motion/Flash Suppression: tắt rung dòng lỗi và hiệu ứng lửa nhấp nháy nền.
- Screen reader: ngoài phạm vi tier Standard; vẫn đặt `AccessibleText` cho các ô nhập và nút.
- Không giới hạn thời gian nhập.

## Localization Considerations

- Nhãn nút "ĐĂNG NHẬP"/"TẠO TÀI KHOẢN" phải nằm trên 1 dòng; dành +40% độ dài (HIGH PRIORITY).
- Chuỗi lỗi hiện đang hardcode tiếng Việt trong `PAAccountSubsystem.cpp` (vd. `:111`, `:119`, `:128`): chuyển sang `FText`/`NSLOCTEXT` theo `EPAAuthErrorCode` để dịch được.
- Email không bản địa hóa; tên tester `Tester N` có thể giữ nguyên.

## Acceptance Criteria

- [ ] Màn Login hiện đầy đủ trong ≤ 1.0s sau khi `LoginMap` nạp xong, focus nằm ở ô Email.
- [ ] Nhập email sai cú pháp (vd. `abc`) và bấm Đăng Nhập → không gửi yêu cầu, ô Email có viền lỗi + icon + chữ "Email không hợp lệ".
- [ ] Sai mật khẩu → dòng trạng thái báo lỗi, Email giữ nguyên, ô Mật khẩu bị xóa, nút mở lại.
- [ ] Trong lúc `Authenticating`, bấm Đăng Nhập lần hai không gửi thêm yêu cầu.
- [ ] Đăng nhập đúng → chuyển sang Chọn Nhân Vật; có "Ghi nhớ" thì lần mở sau vào thẳng Chọn Nhân Vật.
- [ ] Bản Development: `[Tester 1]` đưa vào Chọn Nhân Vật với profile dev trong < 0.1s xử lý (GDD §2.2); bản Shipping không có khu Dev.
- [ ] Toàn bộ màn đi được bằng bàn phím và gamepad, focus luôn nhìn thấy được.
- [ ] Sau Z2: không còn mật khẩu dạng chữ thường trong bộ nhớ stub (`PAAccountSubsystem.cpp:93`, `:21`), test kiểm chứng giá trị lưu là hash.

## Open Questions

- **Q1 (luồng map):** GDD §3.2 ghi `LoginMap → CharacterSelectMap → OpenWorldMap`; code không có hai map đầu (Login mở Character Select ngay trong cùng widget, rồi `OpenLevel(L_VerdantFrontier_Outpost)`); plan Z2 lại ghi một map `L_FrontEnd`. Cần chủ dự án chọn: 2 map riêng theo GDD, hay 1 map `L_FrontEnd` chứa cả hai màn.
- **Q2:** `UPALoginWidget` kế thừa `UUserWidget` và dựng bằng Slate, trái quy tắc CommonUI (`control-manifest.md` §4). Z2 phải chuyển sang `UCommonActivatableWidget`.
- **Q3:** Code chỉ có một nút Fast Play (`"DevWarrior"`, `PALoginWidget.cpp:421`), chưa có `[Tester 1/2/3]` và ô tên tùy chọn như GDD §2.2 (3).
- **Q4:** Nút con mắt xem mật khẩu chưa có trong code (`.IsPassword(true)` cố định, `PALoginWidget.cpp:281`).
- **Q5:** Font chữ: art-bible chưa có mục typography. Mockup dùng Cormorant SC (chữ La Mã cổ điển, theo combat-hud "phông La Mã") cho tiêu đề và Be Vietnam Pro cho chữ thường — chờ art-director/chủ dự án duyệt.
- **Q6:** Chưa có `design/player-journey.md`; ngữ cảnh người chơi ở trên là giả định. Tạo từ template `.claude/docs/templates/player-journey.md`.
- **Q7:** Pattern library thiếu Text Input Field và Checkbox — cần bổ sung vào `design/ux/interaction-patterns.md`.
