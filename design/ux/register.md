# UX Spec: Đăng Ký (Register — `WBP_LoginScreen`, tab Register)

> **Status**: In Design — MOCKUP CHỜ CHỦ DỰ ÁN DUYỆT (Z1)
> **Author**: ux-designer (Z1, chế độ tự vận hành)
> **Last Updated**: 2026-10-10
> **Journey Phase(s)**: Khởi động game — người chơi mới
> **Platform Target**: PC (bàn phím + chuột, chính) · Gamepad (Full, CommonUI)
> **Template**: UX Spec
> **Mockup**: `design/ux/mockups/ascendant-ui-mockups.html` — màn 2
> **Nguồn yêu cầu**: `design/gdd/authentication-account-system.md` §2.2 (2), §3.3, §4 · `design/ux/login.md` (màn anh em, cùng `WBP_LoginScreen`)

---

## Purpose & Player Need

Người chơi mới muốn tạo tài khoản bằng Email + Mật khẩu rồi đăng nhập. Màn kiểm tra cú pháp email, mật khẩu tối thiểu **6 ký tự** (GDD §2.2 (2), §4.2; code `kMinPasswordLength = 6`, `PAAccountSubsystem.cpp:245`) và trùng khớp ô xác nhận, báo lỗi ngay tại ô sai.

## Player Context on Arrival

Người chơi mới, chưa có tài khoản, vừa thấy màn Login và chọn tab Đăng Ký. Bình tĩnh nhưng nóng lòng vào game: form phải ngắn (3 ô).

## Navigation Position

`LoginMap / WBP_LoginScreen` → **tab Register** → (thành công) tab Login. Chỉ vào được từ màn Login.

## Entry & Exit Points

| Entry Source | Trigger | Player carries this context |
|---|---|---|
| Tab Login | Bấm tab "Đăng Ký" / `Ctrl+Tab` / `RB` | Không |

| Exit Destination | Trigger | Notes |
|---|---|---|
| Tab Login | Đăng ký thành công | Code hiện tại: `RegisterAccount` đặt trạng thái `LoggedOut` (`PAAccountSubsystem.cpp:95`), không tự đăng nhập. Đề xuất: điền sẵn Email ở tab Login + thông báo "Tạo tài khoản thành công. Hãy đăng nhập." |
| Tab Login | Bấm tab "Đăng Nhập" | Giữ dữ liệu đã nhập (trừ mật khẩu) |
| Thoát game | `Esc`/`B` tại gốc | Modal xác nhận |

---

## Layout Specification

### ASCII Wireframe

```
                ╱──────────────────────────────────╲
               │   Đăng Nhập   [ ĐĂNG KÝ ]           │
               │  Email                              │
               │  [ ten@vidu.com                  ]  │
               │  Mật khẩu   (tối thiểu 6 ký tự)     │
               │  [ ••••••                     (👁)] │
               │  ✓ Đủ 6 ký tự                       │  ← gợi ý trực tiếp
               │  Xác nhận mật khẩu                  │
               │  [ •••••                      (👁)] │
               │  ✕ Mật khẩu xác nhận không khớp     │  ← lỗi Crimson + icon
               │  [       TẠO TÀI KHOẢN        ]     │
                ╲──────────────────────────────────╱
```

### Layout Zones

Giống `design/ux/login.md` (nền, tiêu đề, thẻ xác thực, thanh gợi ý phím). Khu Dev Playtest vẫn hiện bên dưới ở bản Development (cùng widget).

### Component Inventory

| Component | Loại | Nội dung | Tương tác | Pattern |
|---|---|---|---|---|
| Ô Email | Text input | Email | Có | Text Input Field (mới) |
| Ô Mật khẩu | Text input mask + con mắt | Mật khẩu | Có | Text Input Field |
| Dòng yêu cầu mật khẩu | Text + icon ✓/✕ | "Đủ 6 ký tự" | Không | Inline validation (mới) |
| Ô Xác nhận mật khẩu | Text input mask + con mắt | — | Có | Text Input Field |
| Nút "Tạo Tài Khoản" | Button | — | Có | Button (Primary) |
| Dòng trạng thái | Text | Lỗi máy chủ | Không | — |

### Information Hierarchy

1. Ba ô nhập theo thứ tự. 2. Gợi ý/lỗi ngay dưới từng ô. 3. Nút Tạo Tài Khoản. 4. Tab về Đăng Nhập.

---

## States & Variants

| State / Variant | Trigger | What Changes |
|---|---|---|
| Default | Mở tab | Focus ô Email; nút Tạo Tài Khoản mở (kiểm tra khi bấm) |
| Kiểm tra trực tiếp | Gõ mật khẩu | Dòng yêu cầu đổi ✕ → ✓ khi đủ 6 ký tự |
| Lỗi email | `EPAAuthErrorCode::InvalidEmail` | Viền lỗi + icon + chữ ở ô Email |
| Lỗi mật khẩu ngắn | `PasswordTooShort` | Lỗi ở ô Mật khẩu |
| Lỗi không khớp | `PasswordMismatch` | Lỗi ở ô Xác nhận |
| Email đã tồn tại | `AccountAlreadyExists` | Lỗi ở ô Email + liên kết "Đăng nhập bằng email này" |
| Đang xử lý (loading) | `Authenticating` | Nút → spinner, khóa input |
| Thành công | `RegisterAccount` trả `true` | Chuyển tab Login, điền Email, thông báo thành công |
| Empty | Không áp dụng | — |

## Interaction Map

| Component | Bàn phím / Chuột | Gamepad | Phản hồi | Kết quả |
|---|---|---|---|---|
| Ô nhập | `Tab`/`Shift+Tab`, click | D-pad, `A` mở bàn phím ảo | Focus vàng | Nhập |
| Con mắt | Click | `Y` | Icon đổi | Bật/tắt mask |
| Tạo Tài Khoản | `Enter` / click | `A` | Pressed + `SFX_UI_Click` | `UPALoginWidget::RequestRegister(Email, Password, ConfirmPassword)` |
| Tab Đăng Nhập | Click / `Ctrl+Shift+Tab` | `LB` | Lật sách | `SwitchViewMode(Login)` |

## Events Fired

| Player Action | Event Fired | Payload / Data |
|---|---|---|
| Tạo tài khoản | `UPAAccountSubsystem::RegisterAccount` → `OnLoginStateChanged` | Email (mật khẩu không log) |
| Analytics | Không có | — |

Ghi bền vững: tạo bản ghi tài khoản. Stub hiện tại lưu mật khẩu thô (`PAAccountSubsystem.cpp:93`) — **Z2 phải hash có salt**, khớp cột `password_hash` (GDD §3.3). Backend thật để Giai đoạn 6.

## Transitions & Animations

- Đổi tab: trượt ngang 0.2s.
- Dòng ✓ đổi màu + icon, không nhấp nháy.
- Thành công: fade sang tab Login 0.25s.

## Data Requirements

| Data | Source System | Read / Write | Notes |
|---|---|---|---|
| Email, Mật khẩu, Xác nhận | Người chơi → `RequestRegister` | Write | — |
| Quy tắc mật khẩu | `UPAAccountSubsystem::IsValidPassword` (min 6) | Read | UI đọc cùng hằng số, không tự đặt lại con số |
| Quy tắc email | `IsValidEmail` (`PAAccountSubsystem.cpp:200`) | Read | — |
| Mã lỗi | `EPAAuthErrorCode` | Read | — |

## Input Method Completeness Checklist

**Bàn phím:** [x] thứ tự Tab Email → MK → con mắt → Xác nhận → con mắt → Tạo Tài Khoản · [x] `Enter` gửi · [x] `Esc` về gốc
**Chuột:** [x] vùng bấm ≥ 48px · [x] hover
**Gamepad:** [x] D-pad + `A`/`B` · [x] `LB/RB` đổi tab · [ ] bàn phím ảo (chưa xác minh)

## Accessibility

Tier Standard. Lỗi luôn có icon ✕ + chữ, không chỉ màu; ✓ có icon. Tương phản ≥ 4.5:1. Không giới hạn thời gian. Nút con mắt có `AccessibleText` "Hiện mật khẩu".

## Localization Considerations

- "TẠO TÀI KHOẢN" 1 dòng, +40% (HIGH PRIORITY).
- Dòng yêu cầu dùng tham số: "Tối thiểu {N} ký tự" lấy N từ code.
- Chuỗi lỗi hiện hardcode (`PAAccountSubsystem.cpp:51`, `:66`, `:74`) → chuyển `FText`.

## Acceptance Criteria

- [ ] Tab Đăng Ký mở trong ≤ 0.2s từ tab Login, focus ở ô Email.
- [ ] Mật khẩu 5 ký tự → lỗi tại ô Mật khẩu, không tạo tài khoản; 6 ký tự → qua kiểm tra độ dài.
- [ ] Hai mật khẩu khác nhau → lỗi tại ô Xác nhận có icon + chữ.
- [ ] Email đã có (vd. tài khoản mặc định của stub) → lỗi "Tài khoản đã tồn tại".
- [ ] Đăng ký thành công → về tab Login với Email điền sẵn và thông báo thành công.
- [ ] Đi hết form bằng bàn phím và bằng gamepad.
- [ ] Sau Z2: mật khẩu lưu dạng hash có salt (test kiểm chứng giá trị lưu ≠ mật khẩu gốc).

## Open Questions

- **Q1:** Đăng ký xong nên tự đăng nhập hay quay về tab Login? GDD không nói; code quay về `LoggedOut`. Spec đề xuất quay về Login có điền sẵn Email.
- **Q2:** GDD §2.2 chỉ yêu cầu tối thiểu 6 ký tự; có thêm yêu cầu độ mạnh (chữ + số) không? Không tự thêm — chờ chủ dự án.
- **Q3:** Tên hiển thị hiện tự lấy phần trước `@` của email (`PAAccountSubsystem.cpp:82-88`); có cần ô nhập tên hiển thị không? GDD không có ô này.
- **Q4:** Chung với `login.md`: chuyển `UPALoginWidget` sang CommonUI; thiếu nút con mắt; font chờ duyệt; thiếu pattern Text Input/Checkbox.
