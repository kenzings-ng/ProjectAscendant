# Quy Chế Thẩm Định Mỹ Thuật Độc Lập (Art QA Gate Criteria)

> **Mã Tài Liệu**: `QA-CRITERIA-ART-GATE-2026-V1`  
> **Cơ Quan Thẩm Định**: Art Director & Silhouette QA Gatekeeper (Agent 5)  
> **Nguyên Tắc Cốt Lõi**: **GATE, NOT PRODUCTION** — Agent 5 chỉ kiểm tra và trả về **APPROVE** hoặc **REJECT** kèm lý do kỹ thuật; tuyệt đối không tự sửa asset để tránh biến QA thành khâu sản xuất.  
> **Căn Cứ Kỹ Thuật**: [`SPEC-ART-2026-09-23-V2`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/art/pixel-asset-specifications.md), [`character-visual-system.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/design/gdd/character-visual-system.md).  

---

## 1. Hệ Thống 5 Tiêu Chí Nghiệm Thu (The 5 Gate Criteria)

Mọi tài nguyên đồ họa (kể cả asset thu hoạch CC0 từ bên thứ 3 lẫn template hay sprite vẽ tay mới) bắt buộc phải vượt qua 5 tiêu chí sau:

### Tiêu Chí 1: Zero-Mixels & Tỷ Lệ Điểm Ảnh Chuẩn (Pixel Density)
- **Yêu cầu**: Tỷ lệ pixel vuông tuyệt đối $1:1$.
- **Cấm đoán**:
  - Không có hiện tượng "Mixels" (pixel to nhỏ lẫn lộn trên cùng một khung hình).
  - Không scale bằng các bộ lọc làm mờ (Bilinear / Bicubic). Khi phóng to phải dùng **Nearest Neighbor** theo bội số nguyên ($1\times, 2\times, 4\times, 8\times$).
- **Ngưỡng đạt**: Icon $32 \times 32$ px (hoặc $24 \times 24$ px cho potions), Rig Canvas $128 \times 128$ px.

### Tiêu Chí 2: Bảng Màu 4-Tone Ramp & Nguồn Sáng 10 Giờ (Lighting & Palette)
- **Yêu cầu**:
  - Mỗi loại vật liệu (Thép, Vàng, Da, Vải, Da người) phải tuân thủ dải màu **4 nấc sắc độ (4-Tone Ramp)**.
  - Áp dụng kỹ thuật **Hue-Shifting**: Vùng sáng ngả Vàng chanh, vùng tối ngả Xanh Navy/Tím.
  - Nguồn sáng cứng (Key Light) cố định ở góc $10$ giờ ($45^\circ$). Đổ bóng dứt khoát dưới cằm, lưỡi kiếm, quai đeo.
- **Cấm đoán**: Tuyệt đối không dùng Pillow Shading (đánh bóng đều từ mép vào tâm).

### Tiêu Chí 3: Mốc Khớp Nối Giải Phẫu & Sockets (Anatomy & Alignment)
- **Yêu cầu đối với Sprite Nhân Vật & Rig**:
  - **Điểm tựa chân (Foot Pivot Anchor)**: Cố định tuyệt đối tại tọa độ $(64, 114)$ trên canvas $128 \times 128$.
  - **Khớp nối thắt lưng (Waist Seam)**: Phẳng phiu trên trục ngang $Y = 80$, không được có pixel răng cưa để ghép nối khít giữa Upper Body và Lower Body.
  - **Mào nón (Helm Crest Socket)**: Snap tại $(64, 40)$.
  - **Cờ ngực (Tabard Cutout Socket)**: Snap tại $(64, 60)$.
  - **Hand Sockets**: Tay phải tại $(96, 76)$, Tay trái tại $(32, 76)$.

### Tiêu Chí 4: Nhận Diện Hình Bóng (Silhouette Legibility Check)
- **Yêu cầu**:
  - Khi chuyển sang Pure Black (`#000000`) trên nền trắng, silhouette phải giữ nguyên hình khối nhận diện đặc trưng của vật phẩm/vũ khí.
  - Khi thu nhỏ về kích thước thumbnail $16 \times 16$, khối hình không bị tiêu biến hoặc biến thành cụm pixel vô nghĩa.

### Tiêu Chí 5: Phù Hợp Đề Tài & Bối Cảnh (Theme & Lore Compliance)
- **Yêu cầu**: Thuần chất **Dark Medieval Fantasy ARPG**.
- **Cấm đoán**: Từ chối ngay lập tức mọi asset thuộc thể loại súng đạn hiện đại, khoa học viễn tưởng, máy móc cơ khí anachronistic hoặc phong cách hoạt hình chibi sặc sỡ không phù hợp.

---

## 2. Quy Trình Xét Duyệt Của Cửa Khẩu (Gate Workflow)

```
[Asset Input từ Agent 1 hoặc Agent 2]
                │
                ▼
   ┌─────────────────────────┐
   │ BƯỚC 1: Format & Pixels │ ──> Fail: REJECT (Mixel / Blur)
   └────────────┬────────────┘
                │ Pass
                ▼
   ┌─────────────────────────┐
   │ BƯỚC 2: Palette & Light │ ──> Fail: REJECT (Pillow Shading / Bleed)
   └────────────┬────────────┘
                │ Pass
                ▼
   ┌─────────────────────────┐
   │ BƯỚC 3: Sockets & Pivot │ ──> Fail: REJECT (Lệch Socket / Hở Eo)
   └────────────┬────────────┘
                │ Pass
                ▼
   ┌─────────────────────────┐
   │ BƯỚC 4: Silhouette QA   │ ──> Fail: REJECT (Mất dạng ở 16x16)
   └────────────┬────────────┘
                │ Pass
                ▼
   ┌─────────────────────────┐
   │ BƯỚC 5: Lore & Theme    │ ──> Fail: REJECT (Lệch bối cảnh)
   └────────────┬────────────┘
                │ Pass
                ▼
           [ APPROVED ]
```
