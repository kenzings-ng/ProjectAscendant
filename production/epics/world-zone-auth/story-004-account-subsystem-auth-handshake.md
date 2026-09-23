# Story 004: Account Subsystem, 1-Click Fast Playtest & Server Token Handshake

> **Epic**: World Integration, Sanctuaries & Account Authentication Layer  
> **Status**: Ready  
> **Layer**: Security / Subsystem  
> **Type**: Subsystem / Network  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-23  
> **Last Updated**: 2026-09-23  

## Context

**GDD**: [`design/gdd/authentication-account-system.md`](../../design/gdd/authentication-account-system.md)  
**Requirement**: `TR-auth-001` (GameInstance Subsystem, 1-click playtest < 0.1s, RFC 5322 validation, token handshake)

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `FPAAccountProfile`, `FPAAuthModel` and `UPAAccountSubsystem` (`UGameInstanceSubsystem`).

---

## Acceptance Criteria

- [ ] **AC-1 (GameInstance Subsystem Persistence)**: `UPAAccountSubsystem` kế thừa `UGameInstanceSubsystem`:
  - Tồn tại xuyên suốt vòng đời game client mà không bị hủy khi nạp bản đồ (`LoginMap` $\to$ `CharacterSelectMap` $\to$ `OpenWorldMap`).
  - Lưu giữ thông tin phiên chơi `FPAAccountProfile` (AccountId, DisplayName, AuthToken, bIsDevProfile).
- [ ] **AC-2 (1-Click Fast Playtest Mode - < 0.1s)**:
  - Cung cấp các nút tester sẵn (`Tester 1`, `Tester 2`, `Tester 3`) hoặc nhập tên tự do.
  - Bỏ qua mọi bước nhập mật khẩu, tự sinh ngẫu nhiên UUID và gắn cờ `bIsDevProfile = true`.
  - Hoàn tất phiên đăng nhập và kích hoạt sự kiện thành công trong thời gian $< 0.10$ giây.
- [ ] **AC-3 (Email & Password Validation Rules)**:
  - Email phải tuân thủ chuẩn RFC 5322 (chứa `@` và dấu chấm hợp lệ ở phần tên miền, không chứa khoảng trắng).
  - Mật khẩu tối thiểu **6 ký tự**.
  - Kiểm tra xác nhận mật khẩu (Confirm Password) phải khớp 100%, nếu sai trả về lỗi rõ ràng.
- [ ] **AC-4 (Server Join Token Handshake)**:
  - Khi đăng nhập thành công, tự động sinh mã Token phiên chơi theo định dạng: `PA-TOKEN-[UUID]-[TIMESTAMP]`.
  - Cung cấp hàm xác thực token phía server (`ValidateJoinToken`) để Dedicated Server kiểm tra tính hợp lệ trước khi cho phép nhân vật spawn vào thế giới.

---

## Implementation Notes

1. **`PAAccountTypes.h`**:
   - `EPAAuthLoginState`: `LoggedOut`, `Authenticating`, `LoggedIn`, `Error`.
   - `FPAAccountProfile`: AccountId, Email, DisplayName, AuthToken, bIsDevProfile, CreatedAt.
   - `FPAAuthModel`: Pure data model kiểm tra regex email, độ dài password, sinh UUID/token handshake, và quản lý profile.
2. **`PAAccountSubsystem.h` / `PAAccountSubsystem.cpp`**:
   - `UGameInstanceSubsystem` quản lý state, delegate `OnLoginSuccess`, `OnLoginStateChanged`.

---

## QA Test Cases

- **Test 1: 1-Click Fast Playtest**: Nhấn nút Tester 1 -> sinh profile hợp lệ với bIsDevProfile = true trong < 0.1s.
- **Test 2: Email & Password Validation**: Email "invalid-email" -> báo lỗi; email "tester@projectascendant.com" -> hợp lệ. Password 5 ký tự -> lỗi; 6 ký tự -> hợp lệ.
- **Test 3: Token Format & Validation**: Token sinh ra có định dạng `PA-TOKEN-` hợp lệ; server xác thực khớp profile.
- **Test 4: Subsystem Persistence**: Lưu profile -> chuyển bản đồ giả định -> profile vẫn được bảo toàn nguyên vẹn.
