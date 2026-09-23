# Smoke Test: Critical Paths — Project Ascendant

**Mục đích**: Chạy 10-12 bước kiểm tra then chốt trong vòng dưới 15 phút trước mỗi đợt bàn giao QA.  
**Công cụ thực thi**: Lệnh `/smoke-check`  
**Quy tắc cập nhật**: Bổ sung kịch bản kiểm tra mới ngay khi một cơ chế cốt lõi được hoàn tất.  

---

## 1. Tính Ổn Định Cốt Lõi (Core Stability - Bắt buộc chạy)
- [ ] 1. Máy chủ Dedicated Server khởi động ở chế độ headless (`-nullrhi`) không phát sinh Crash.
- [ ] 2. Client khởi chạy, nạp vào sảnh chờ (Main Menu) với tỷ lệ 60 FPS ổn định.
- [ ] 3. Client kết nối thành công vào Dedicated Server cục bộ qua địa chỉ `127.0.0.1:7777`.

## 2. Cơ Chế Di Chuyển & Điều Khiển 2.5D (Controller & Movement)
- [ ] 4. Nhân vật phản hồi phím WASD / cần xoay Gamepad di chuyển đúng 8 hướng Isometric (-45° pitch, 45° yaw).
- [ ] 5. Nhấn Phím Cách (Space) kích hoạt đòn Dash: tiêu tốn đúng 25 Stamina, vệt bóng mờ Pixel xuất hiện, không thể bị tổn thương trong 0.20s.

## 3. Cơ Chế Chiến Đấu & Thể Khí (Combat & Posture)
- [ ] 6. Nhấn Chuột Trái thi triển chuỗi 3 đòn combo chém cơ bản, hoạt ảnh Flipbook chạy mượt mà không khựng hình.
- [ ] 7. Đòn đánh trúng hình nộm thử nghiệm làm tụt thanh Máu và tích lũy thanh Thể khí (Posture).
- [ ] 8. Khi thanh Posture chạm mốc 100%, mục tiêu chuyển sang trạng thái gục ngã (Staggered) trong 1.5 giây, biểu tượng kết liễu Finisher (phím F) sáng lên.

## 4. Dữ Liệu & Ba Lô (Inventory Integrity)
- [ ] 9. Mở túi đồ (Phím I): Lưới 30 ô hiển thị chuẩn xác biểu tượng vật phẩm.
- [ ] 10. Kéo thả vật phẩm giữa 2 ô đồ phản hồi tức thời, chỉ số không bị nhân bản hay biến mất khi đăng xuất rồi đăng nhập lại.

## 5. Hiệu Năng & Phần Cứng (Performance Budget)
- [ ] 11. Khung hình duy trì $\ge 60\text{ FPS}$ cố định ở độ phân giải 1080p, không có hiện tượng giật cục (Micro-stutters).
- [ ] 12. Dung lượng RAM của Client không tăng liên tục (Không bị rò rỉ bộ nhớ) sau 10 phút di chuyển dã ngoại.
