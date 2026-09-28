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

### 3.3 Phân Định Ranh Giới Dữ Liệu: Tài Khoản vs Nhân Vật (PostgreSQL Schema)
Theo nguyên tắc kiến trúc ADR-0001, dữ liệu người chơi được phân định tách bạch thành 3 bảng cơ sở:

```sql
-- 1. BẢNG TÀI KHOẢN (Account Level)
CREATE TABLE accounts (
    account_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    email VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    gold_balance BIGINT NOT NULL DEFAULT 0 CHECK (gold_balance >= 0),
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- 2. BẢNG NHÂN VẬT (Character Level)
CREATE TABLE characters (
    character_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    account_id UUID NOT NULL REFERENCES accounts(account_id) ON DELETE CASCADE,
    character_name VARCHAR(64) UNIQUE NOT NULL,
    character_level INT NOT NULL DEFAULT 1 CHECK (character_level BETWEEN 1 AND 50),
    character_exp BIGINT NOT NULL DEFAULT 0 CHECK (character_exp >= 0),
    primary_class_tag VARCHAR(128) NOT NULL,
    secondary_class_tag VARCHAR(128),
    -- Lưu tiến trình độc lập của toàn bộ các class đã từng luyện (Khởi tạo rỗng)
    class_progression_history JSONB NOT NULL DEFAULT '{}',
    completed_quests TEXT[] NOT NULL DEFAULT '{}',
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- 3. BẢNG VẬT PHẨM RIÊNG BIỆT (Items Table - Single Source of Truth)
CREATE TABLE items (
    item_instance_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    item_def_id VARCHAR(64) NOT NULL,      -- Tham chiếu ID trong DataAsset (VD: 'scroll_dragon_knight')
    owner_type VARCHAR(16) NOT NULL,       -- 'CHARACTER' (Balo/Trang bị) hoặc 'ACCOUNT' (Hòm kho chung)
    owner_id UUID NOT NULL,                -- Chứa character_id HOẶC account_id tương ứng
    slot_type VARCHAR(32) NOT NULL,        -- 'INVENTORY', 'MAINHAND', 'OFFHAND', 'BODY', 'SHARED_STASH'
    slot_index INT NOT NULL,               -- Tọa độ vị trí ô đồ (0..N)
    quantity INT NOT NULL DEFAULT 1 CHECK (quantity > 0),
    durability FLOAT NOT NULL DEFAULT 100.0,
    enhancement_level INT NOT NULL DEFAULT 0,
    item_data JSONB NOT NULL DEFAULT '{}', -- Dữ liệu riêng (ngọc khảm, chỉ số ngẫu nhiên)
    b_is_locked BOOLEAN NOT NULL DEFAULT FALSE,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    -- Khai báo ràng buộc duy nhất DEFERRABLE INITIALLY DEFERRED để hoán đổi ô đồ không vi phạm UNIQUE
    CONSTRAINT uk_owner_slot UNIQUE (owner_type, owner_id, slot_type, slot_index) DEFERRABLE INITIALLY DEFERRED
);
```

### 3.4 Giao Dịch Chuyển Chức Chống Dupe Tuyệt Đối (Atomic Transaction & Row-Locking)
Mọi thao tác sử dụng Quyển Trục thăng chức được Dedicated Server thực thi với khóa dòng độc quyền:

```sql
BEGIN;

-- 1. Khóa độc quyền bản ghi quyển trục chống race condition / duplicate
SELECT item_instance_id, item_def_id, owner_type, owner_id, quantity 
FROM items 
WHERE item_instance_id = $1 AND owner_type = 'CHARACTER' AND owner_id = $2
FOR UPDATE;

-- 2. Kiểm tra nghiệp vụ trên Server:
-- IF item_def_id != ExpectedScrollDefId OR server_validation_failed THEN
--     ROLLBACK;
--     RETURN ERROR;
-- END IF;

-- 3. Cập nhật thẻ class vào đúng slot đạt điều kiện (Primary HOẶC Secondary):
-- (Nếu thăng chức cho Class Chính):
UPDATE characters 
SET primary_class_tag = $3,
    class_progression_history = jsonb_set(class_progression_history, ARRAY[$3], $4, true)
WHERE character_id = $2;

-- (Nếu thăng chức cho Class Phụ):
-- UPDATE characters 
-- SET secondary_class_tag = $3,
--     class_progression_history = jsonb_set(class_progression_history, ARRAY[$3], $4, true)
-- WHERE character_id = $2;

-- 4. Tiêu hủy Quyển Trục sau khi thăng chức thành công
DELETE FROM items WHERE item_instance_id = $1;

COMMIT;
```

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
