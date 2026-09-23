# GDD: Authentication & Player Identity System

> **System ID**: `AUTH-SYS`  
> **Status**: Approved (Phase 1 Implemented)  
> **Layer**: Foundation  
> **Engine**: Unreal Engine 5.7 C++ / CommonUI  
> **Last Updated**: 2026-09-16  

---

## 1. Executive Summary

Hệ thống Xác thực & Định danh Người chơi (*Authentication & Player Identity*) quản lý toàn bộ vòng đời đăng nhập, đăng ký, lưu phiên và định danh người chơi trong *Project Ascendant*. Hệ thống đóng vai trò cổng kiểm soát an ninh đầu tiên (Gatekeeper) trước khi client được phép kết nối vào Unreal Engine 5.7 Dedicated Server theo quy định của `ADR-0001` (100% Server Authoritative).

Hệ thống được phát triển theo **lộ trình 3 giai đoạn (Phased Roadmap)**:
- **Giai đoạn 1 (Hiện tại - Playtest & Pre-Production)**: Cung cấp chế độ **1-Click Fast Playtest** (dành cho thử nghiệm nội bộ nhanh nhiều client) và form **Email/Password tiêu chuẩn** có lưu phiên cục bộ (*Remember Me*).
- **Giai đoạn 2 (Vertical Slice / Alpha)**: Tích hợp Master Backend API (PlayFab / Supabase / Custom Auth Gateway) qua REST/HTTPS với mã hóa JWT Token.
- **Giai đoạn 3 (Commercial Beta & Release)**: Tích hợp Single Sign-On nền tảng (Steamworks Ticket & Epic Online Services EOS Connect).

---

## 2. Core User Experience & UI Flow

### 2.1 Màn Hình Khởi Động (`WBP_LoginScreen`)
Giao diện tuân thủ phong cách đồ họa **HD-2D Dark Fantasy**:
- Khung viền pixel art gothic cổ điển mạ vàng đen.
- Hiệu ứng khói sương bóng tối và ngọn lửa tím lập lòe ở hậu cảnh.
- Âm thanh: Tiếng lật sách ma thuật cổ khi chuyển tab, tiếng chuông đồng trầm khi đăng nhập thành công.

### 2.2 Các Tab Tính Năng
1. **Tab Đăng Nhập (Login)**:
   - Trường nhập `Email`: Hỗ trợ kiểm tra cú pháp chuẩn RFC 5322 regex.
   - Trường nhập `Password`: Ẩn ký tự dấu hoa thị (`*`), có nút biểu tượng con mắt để xem trước mật khẩu.
   - Checkbox `Ghi nhớ đăng nhập` (Remember Me): Lưu mã phiên mã hóa để lần sau tự động vào game.
   - Nút `Đăng Nhập`: Kích hoạt kiểm tra thông tin và sinh Auth Token.
2. **Tab Đăng Ký (Register)**:
   - Trường nhập `Email`.
   - Trường nhập `Mật khẩu` (Yêu cầu tối thiểu 6 ký tự).
   - Trường nhập `Xác nhận mật khẩu`: Báo lỗi đỏ nếu không trùng khớp.
   - Nút `Tạo Tài Khoản`: Khởi tạo hồ sơ người chơi mới.
3. **Khu Vực Dev Playtest (Chế Độ Nhanh - 1 Click)**:
   - Dành riêng cho môi trường Development / Playtest:
   - Các nút tạo sẵn: `[Tester 1]`, `[Tester 2]`, `[Tester 3]`.
   - Ô nhập tự do `Tên Tùy Chọn` + Nút `Vào Game Ngay`.
   - Bỏ qua mọi bước nhập mật khẩu, tự động gán `AccountID` ngẫu nhiên và đưa người chơi vào game trong $< 0.1\text{s}$.

---

## 3. Data Structure & C++ Architecture

### 3.1 Dữ Liệu Hồ Sơ (`FPAAccountProfile`)
```cpp
struct FPAAccountProfile
{
    FString AccountID;       // UUID định danh duy nhất của tài khoản
    FString Email;           // Địa chỉ email (để trống nếu là Dev Profile)
    FString DisplayName;     // Tên hiển thị công khai của người chơi
    FString AuthToken;       // Mã phiên xác thực (JWT Token)
    bool bIsDevProfile;      // Đánh dấu tài khoản dùng thử nghiệm nội bộ
    FDateTime CreatedAt;     // Thời gian tạo tài khoản
};
```

### 3.2 Vòng Đời Subsystem (`UPAAccountSubsystem`)
- Kế thừa từ `UGameInstanceSubsystem`: Đảm bảo phiên đăng nhập tồn tại xuyên suốt từ lúc mở game, chuyển qua các bản đồ `LoginMap` $\rightarrow$ `CharacterSelectMap` $\rightarrow$ `OpenWorldMap` mà không bị hủy.
- Cung cấp Dynamic Multicast Delegates:
  - `OnLoginStateChanged(EPAAuthLoginState NewState, FString ErrorMessage)`
  - `OnLoginSuccess(FPAAccountProfile Profile)`

---

## 4. Quy Chuẩn Xác Thực & Bảo Mật

1. **Thẩm định Email (`IsValidEmail`)**:
   - Bắt buộc chứa ký tự `@` và dấu chấm `.` ở phần domain.
   - Không chứa khoảng trắng hoặc ký tự đặc biệt nguy hiểm.
2. **Thẩm định Mật khẩu (`IsValidPassword`)**:
   - Độ dài tối thiểu: 6 ký tự.
3. **Mã Token Xác Thực (Auth Token)**:
   - Mỗi phiên đăng nhập thành công sinh ra một `AuthToken` tiền tố `PA-TOKEN-[UUID]-[TIMESTAMP]`.
   - Khi Client gọi lệnh `OpenMap` hoặc RPC kết nối vào Dedicated Server, `AuthToken` được gửi kèm trong kết nối để Dedicated Server xác nhận danh tính trước khi cho phép spawn nhân vật.

---

## 5. Kế Hoạch Mở Rộng Tiếp Theo

- **Vertical Slice**: Thêm lưu trữ SaveGame mã hóa AES cho cấu hình Remember Me trên đĩa.
- **Dedicated Server Handshake**: Tích hợp hàm thẩm tra `ValidateJoinToken` vào `AGameModeBase::PreLogin`.
