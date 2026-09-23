# 2D Pixel Asset Technical Specifications & Production Contract
## Quy Chuẩn Kỹ Thuật Đồ Họa 2D Pixel & Hợp Đồng Sản Xuất Tài Nguyên

> **Dự án**: Project Ascendant  
> **Phiên bản tài liệu**: v1.0.0 — Production Standard  
> **Cơ quan ban hành**: Lead Game Designer & Lead Art Director  
> **Tham chiếu chuẩn mực (Benchmarks)**: *Stardew Valley* (ConcernedApe), *Terraria* (Re-Logic), *Octopath Traveler* (Square Enix), *Chrono Trigger* (Square).  
> **Bộ kỹ năng áp dụng**: `game-art-studio` (Anti-AI Craft Rules & Asset Contract Presets)  
> **Engine mục tiêu**: Unreal Engine 5.7 (Paper2D / PaperZD / CommonUI / Lumen HD-2D)  
> **Trạng thái**: **BẮT BUỘC THỰC THI (MANDATORY ENFORCEMENT)** — Mọi asset đồ họa không đạt bài kiểm tra nghiệm thu (QA Gate Check) sẽ bị loại bỏ khỏi build game.

---

## 1. Triết Lý Thiết Kế: "Hardcore Indie Pixel Artisan"

Khác biệt hoàn toàn với xu hướng lạm dụng AI tạo ra các bức ảnh "trông hào nhoáng nhưng rỗng tuếch, đơ cứng và bẩn màu", *Project Ascendant* tuân thủ nghiêm ngặt tinh thần **Thủ Công Mỹ Nghệ Pixel (Handcrafted Pixel Artistry)**:

1. **Pixel-Perfect First**: Mỗi điểm ảnh (pixel) trên màn hình phải có lý do tồn tại. Từng đường nét, điểm nhấn ánh sáng (specular highlight) và bóng đổ (cast shadow) đều do ý đồ thiết kế quyết định, không phải ngẫu nhiên do thuật toán nội suy.
2. **Kỷ Luật Bảng Màu (Palette Discipline)**: Nói không với dải màu trôi nổi hàng ngàn sắc độ mờ mờ (Color Bleed). Mọi chất liệu từ sắt rỉ, vàng ròng, gỗ mun đến ngọc bích đều có công thức nấc màu (Color Ramp) cố định từ 3 đến 4 bậc.
3. **Thổi Hồn Vào Tư Thế (Action-First & Contrapposto)**: Nhân vật không đứng thẳng đơ như ma-nơ-canh cửa hiệu. Dáng đứng luôn có đường cong động lực (Line of Action) và trọng tâm lệch rõ rệt, sẵn sàng lao vào giao tranh.
4. **Đọc Được Từ Khoảng Cách (Silhouette Readability)**: Ở góc nhìn Isometric nghiêng $-45^\circ$, người chơi phải nhận biết tức khắc loại vũ khí trên tay và trang bị trên người chỉ qua hình khối bao ngoài (Silhouette) trong vòng 0.1 giây.

---

## 2. Quy Chuẩn Kỹ Thuật: Hệ Thống Nhân Vật & Modular Paperdoll

```
                          [ ATOMICAL PIVOT & PROPORTIONS ]
      ┌─────────────────────────────────────────────────────────────────┐
      │ (0, 0)                                                (128, 0)  │
      │                                                                 │
      │                    ┌──────────────┐                             │
      │                    │   ĐẦU (Head) │ ◄── [Y: 28-56] (28px)       │
      │                    │  Eye: Y=44   │                             │
      │                    └──────┬───────┘                             │
      │                   ┌───────┴────────┐                            │
      │                   │ THÂN TRÊN (Torso) ◄── [Y: 56-82] (26px)     │
      │    VŨ KHÍ PHỤ ──► │  Waist: Y=80   │ ◄── VŨ KHÍ CHÍNH (X:96,Y:76│
      │   (X:32, Y:76)    └───────┬────────┘                            │
      │                    ┌──────┴───────┐                             │
      │                    │ CHÂN (Legs)  │ ◄── [Y: 82-114] (32px)      │
      │                    │  Chân Trụ:80%│                             │
      │                    └──────┬───────┘                             │
      │                          ▼ PIVOT ANCHOR (X: 64, Y: 114)         │
      │                      (Bottom-Center Tiếp Đất)                   │
      │ (0, 128)                                              (128, 128)│
      └─────────────────────────────────────────────────────────────────┘
```

### 2.1 Tỷ Lệ Nhân Vật (Proportions)
- **Tỷ lệ cơ thể**: Chuẩn **3.2 - 3.5 Đầu** (Heroic Chibi tỉ lệ vàng phong cách *Stardew Valley* x *Octopath Traveler*).
  - Chiều cao tổng thể hiển thị: $86$ pixel trên lưới bản vẽ.
  - Phần Đầu & Tóc: $28$ pixel (Chiếm $32\%$, đôi mắt to rõ 2–3px để biểu cảm).
  - Phần Thân & Giáp Ngực: $26$ pixel (Chiếm $30\%$, khối giáp và thắt lưng).
  - Phần Chân & Ủng: $32$ pixel (Chiếm $38\%$, thế đứng tấn vững chắc).
- **Quy chuẩn Canvas**:
  - **Lưới hiển thị cơ thể (Native Body Grid)**: $64 \times 96$ pixel.
  - **Canvas Flipbook xuất khẩu (Export Canvas)**: $128 \times 128$ pixel (chuẩn lũy thừa cơ số 2 cho GPU Texture).
  - **Khoảng đệm an toàn động lực (Motion Padding)**: Dành riêng $32$px phía trước và $14$px phía trên để chứa vệt kiếm chém (Slash VFX), mũi giáo đâm hoặc cánh cung giương rộng mà không bao giờ bị cắt cụt biên.
  - **Điểm neo trọng tâm (Pivot Anchor Point)**: Khóa cứng tại tọa độ đáy giữa `(X: 64, Y: 114)` (Bottom-Center, vị trí bàn chân tiếp xúc mặt đất). Trùng khớp $100\%$ với tâm của `CapsuleComponent` trong Unreal Engine 5.

### 2.2 Quy Chuẩn Khớp Khóa Mô-đun Trang Phục (Modular Paperdoll Alignment)
Tất cả các lớp trang phục (Lớp Vải Tân Thủ, Giáp Sắt Vanguard, Giáp Da Ranger, Pháp Bào Arcanist) bắt buộc phải tuân thủ nghiêm ngặt tọa độ giải phẫu (Landmark Coordinates) để khi người chơi click thay đồ, sprite mới khớp kín khít vào cơ thể mà không hở da hay lệch khớp:

| Mốc Giải Phẫu (Landmark) | Tọa Độ Pixel Chuẩn (X, Y) | Dung Sai Cho Phép | Mục Đích Khóa Khớp |
| :--- | :---: | :---: | :--- |
| **Trục Mắt (Eye Level)** | `Y = 44` | $\pm 0$ px | Khóa vị trí mũ sắt, nón da, khăn trùm đầu. |
| **Cổ Áo (Neck Seam)** | `Y = 56` | $\pm 0$ px | Mép giao thoa giữa giáp ngực và đầu/tóc. |
| **Thắt Lưng (Waistline)** | `Y = 80` | $\pm 0$ px | Điểm nối giữa giáp thân trên và giáp chân/quần. |
| **Khớp Tay Cầm Chính (Mainhand Grip)** | `(X: 96, Y: 76)` | $\pm 1$ px | Vị trí bàn tay nắm chuôi kiếm, cán búa, thân cung. |
| **Khớp Tay Cầm Phụ (Offhand Grip)** | `(X: 32, Y: 76)` | $\pm 1$ px | Vị trí bàn tay cầm khiên, dao găm, sách phép. |

### 2.3 Thứ Tự Xếp Lớp Hiển Thị Trong PaperZD (Layer Sorting Order)
Để đảm bảo chiều sâu 2.5D chính xác ở góc nhìn nghiêng $-45^\circ$, các thành phần được gán số thứ tự Z-Order cố định:
1. `Z = 0`: **Vết Bóng Đổ Thời Gian Thực (Lumen Dynamic Shadow)** — Do Engine tính toán, tuyệt đối cấm vẽ đốm đen phẳng dưới chân sprite.
2. `Z = 1`: **Thân Nhân Vật Trần (Base Body Skin)** — Nam / Nữ.
3. `Z = 2`: **Trang Phục Vải Lót / Tân Thủ (`Visual_StarterCloth`)**.
4. `Z = 3`: **Giáp Thân / Áo Choàng (`Visual_Armor`)**.
5. `Z = 4`: **Mũ Giáp / Tóc Giả (`Visual_Helmet`)**.
6. `Z = 5`: **Vũ Khí Phụ / Khiên (`Visual_Offhand`)**.
7. `Z = 6`: **Vũ Khí Chính (`Visual_Mainhand`)**.

---

## 3. Quy Chuẩn Kỹ Thuật: Vật Phẩm, Vũ Khí & Nguyên Liệu (Items & Gear)

Hệ thống vật phẩm được chuẩn hóa theo phong cách **Chunky Micro-Pixel kinh điển của *Stardew Valley*** nhưng được nâng cấp độ nét để chạy mượt mà trên UI hiện đại.

```
                           [ ITEM CANVAS ARCHITECTURE ]
     ┌─────────────────────────────────────────────────────────────────┐
     │ (0, 0)                                                 (32, 0)  │
     │       MŨI VŨ KHÍ / ĐIỂM SÁNG                                    │
     │               \                                                 │
     │                \   [ 1px Outer Contour: #121316 ]               │
     │                 \                                               │
     │                  \   ◄── GÓC NGHIÊNG 45° CHUẨN STARDREW         │
     │                   \                                             │
     │                    \                                            │
     │                     \                                           │
     │                      CHUÔI VŨ KHÍ / TÂM ĐÁ                     │
     │ (0, 32)                                                (32, 32) │
     └─────────────────────────────────────────────────────────────────┘
        Native Resolution: 32 x 32 px  ──►  Upscale 8x (Nearest Neighbor)
                                       ──►  Export Canvas: 256 x 256 px
```

### 3.1 Kích Thước Bản Vẽ & Kích Thước Xuất Khẩu (Canvas Resolutions)
1. **Lưới Điểm Ảnh Gốc (Native Pixel Grid)**:
   - **Icon Chuẩn (Vũ khí 1 tay, Khiên, Potion, Quặng, Đá quý, Mũ)**: **$32 \times 32$ pixel**.
   - **Icon Cỡ Lớn (Đại kiếm 2 tay, Trượng Ma Pháp Cổ, Lưỡi Hái)**: **$40 \times 40$ pixel** (đặt trong canvas đệm $48 \times 48$).
   - *Cấm tuyệt đối*: Không vẽ icon trực tiếp ở độ phân giải lớn rồi thu nhỏ, hành vi này tạo ra pixel nhòe (anti-aliasing) làm mất chất retro.
2. **Kích Thước Xuất Xưởng Engine (Export Engine Canvas)**:
   - **Độ phân giải file PNG**: **$256 \times 256$ pixel**.
   - **Thuật toán phóng to (Upscale Algorithm)**: **Nearest Neighbor $8\times$** chính xác từng hạt pixel (1 pixel gốc = khối $8 \times 8$ pixel trên file PNG).
   - *Mục đích*: Tương thích hoàn hảo với hệ thống `UImage` của CommonUI trên mọi màn hình từ Full HD (1080p), 2K đến 4K mà không bao giờ bị vỡ hạt hay mờ cạnh.

### 3.2 Quy Chuẩn Góc Đặt Vật Phẩm (Composition & Angles)

| Nhóm Vật Phẩm | Góc Đặt Quy Chuẩn | Mô Tả Bố Cục Thẩm Mỹ (Stardew Style) |
| :--- | :---: | :--- |
| **Vũ Khí Cận Chiến** (Kiếm, Đại kiếm, Dao găm, Chùy, Rìu) | **$45^\circ$ Đường Chéo** | Chuôi kiếm nằm tại góc dưới-trái `(X:4, Y:28)`, mũi kiếm vươn tới góc trên-phải `(X:28, Y:4)`. Chiếm trọn $85-90\%$ đường chéo ô đồ. |
| **Cung Tên (Bows)** | **$45^\circ$ Đường Chéo** | Cánh cung cong hướng về góc trên-trái, dây cung căng chéo, mũi tên gắn sẵn hướng thẳng lên góc trên-phải. |
| **Gậy Phép & Trượng (Staffs)** | **$45^\circ$ Đường Chéo** | Cán trượng thẳng tắp, đầu trượng chứa viên ngọc hoặc tinh thể phát quang ở góc trên-phải, đính kèm 2–3 hạt bụi ma thuật lơ lửng. |
| **Khiên Phòng Ngự (Shields)** | **Chính diện nghiêng $15^\circ$** | Mặt khiên hướng thẳng về người chơi, viền khiên vát cạnh $15^\circ$ để khoe độ dày kim loại/gỗ và hoa văn gia tộc. |
| **Bình Thuốc (Potions)** | **Thẳng đứng $90^\circ$** | Cổ bình thắt nút bấc ở đỉnh `Y=6`, thân bình tròn/vuông chứa dung dịch chiếm $70\%$ chiều cao, vệt phản quang thủy tinh 1px trắng chéo qua thân. |
| **Nguyên Liệu Quặng & Gạch** | **Khối 3D Isometry** | 3 mặt diện rõ rệt (Mặt đỉnh đón sáng, Mặt trái chuyển sắc, Mặt phải bóng đổ tối). Nứt gãy góc cạnh, không bo tròn. |
| **Đá Quý Cắt Giác (Cut Gems)** | **Đa giác kim cương $0^\circ$** | Cắt giác hình học sắc nét (Brilliant / Octagonal cut). Tâm ngọc sáng rực, viền ngoài đổ bóng sâu tạo độ khúc xạ thủy tinh. |
| **Trang Bị Mặc (Áo Giáp, Mũ, Ủng)** | **Chính diện $0^\circ$** | Mô phỏng dạng trưng bày trên giá đỡ (Armor Stand), cân đối trục dọc đối xứng $50/50$. |

---

## 4. Kỷ Luật Màu Sắc & Bộ Quy Tắc Chống "Mùi AI" (Anti-AI Artisan Rules)

Mọi họa sĩ hoặc công cụ tạo asset bắt buộc phải vượt qua 5 điều răn cấm kỵ sau:

### 4.1 Điều Răn 1: Diệt Trừ Pillow Shading (Cấm Đánh Bóng Viền Gối)
- **Hành vi AI vi phạm**: Lấy trung bình màu rồi làm tối dần từ mép ngoài vào tâm của từng chi tiết, khiến vật phẩm trông như túi cát mềm, không có chất rắn kim loại.
- **Quy chuẩn bắt buộc**: Thiết lập **Nguồn sáng đơn góc $45^\circ$ (Key Light)** từ góc trên-trái ($10$ giờ). Toàn bộ bề mặt hứng sáng phải sáng rõ, toàn bộ mặt khuất sáng phải đổ bóng cứng (Hard Cast Shadow) với ranh giới phân định dứt khoát.

### 4.2 Điều Răn 2: Bắt Buộc Dịch Chuyển Nhiệt Độ Màu (Strict Hue-Shifting)
- **Hành vi AI vi phạm**: Tạo bóng bằng cách pha thêm Đen/Xám, tạo sáng bằng cách pha Trắng $\rightarrow$ Bức tranh bị đục, bẩn và tái ngắt.
- **Quy tắc vàng**:
  - Khi tăng sáng (Highlight): **Dịch bước sóng màu về phía Vàng Chanh / Kem Ấm (Warm Spectrum)**.
  - Khi hạ tối (Shadow): **Dịch bước sóng màu về phía Tím Indigo / Xanh Navy / Đỏ Rượu Vang (Cool Spectrum)** do ảnh hưởng của ánh sáng vòm trời (Sky Ambient Light).

```
   [ QUY TẮC COLOR RAMP 4 BẬC CHUẨN MỰC ]
   
   1. Thép Sáng (Steel / Iron):
      Highlight:  #F0F6FC (Trắng pha xanh băng nhẹ) ── 10%
      Midtone:    #8B949E (Xám kim loại sạch)        ── 50%
      Shadow:     #30363D (Chàm than u tối)          ── 35%
      Deep Crease:#161B22 (Xanh đen rãnh sâu)        ── 5%
      
   2. Vàng Ròng (Gold Ingot / Divine Trim):
      Highlight:  #FEF08A (Vàng chanh rực rỡ)        ── 10%
      Midtone:    #E6A122 (Vàng hổ phách rực lửa)    ── 50%
      Shadow:     #92400E (Đồng nâu cháy)            ── 35%
      Deep Crease:#451A03 (Nâu socola đen)           ── 5%
      
   3. Máu & Sinh Mệnh (Crimson Potion / Ruby):
      Highlight:  #FCA5A5 (Hồng san hô sáng)         ── 10%
      Midtone:    #DC2626 (Đỏ tươi cờ)               ── 50%
      Shadow:     #7F1D1D (Đỏ mận rượu vang)         ── 35%
      Deep Crease:#450A0A (Đỏ đen huyết dụ)          ── 5%
```

### 4.3 Điều Răn 3: Tỷ Lệ Nghỉ Mắt 70 - 20 - 10 (Resting Areas)
- Cấm vẽ các đường vân vàng uốn lượn, ren ren, dây xích rác (Ornate Greeble) mà AI hay tự ý vẽ bừa lên vũ khí.
- **$70\%$ Vùng nghỉ mắt**: Lưỡi kiếm phẳng, thân khiên trơn, mảng giáp đơn sắc.
- **$20\%$ Chi tiết chức năng**: Rãnh thoát máu (Blood groove), ốc tán khiên, dây quấn chuôi kiếm.
- **$10\%$ Điểm nhấn định vị**: Đốm sáng lóe trên mũi nhọn, viên hồng ngọc đính ở đốc kiếm.

### 4.4 Điều Răn 4: Kỹ Thuật Viền Màu Chọn Lọc (Selective Outlining - Selout)
- **Viền bao ngoài (Outer Silhouette Outline)**: Bắt buộc viền 1px màu đen than pha sắc chất liệu (`#121316` hoặc `#181124`) để tách biệt vật thể khỏi mọi loại nền bản đồ.
- **Viền chia khối nội bộ (Internal Seams)**: CẤM dùng nét đen kịt. Phải dùng **phiên bản sẫm hơn 2 bậc của chính mảng màu đó** (ví dụ: lằn ranh giữa các ngón tay da người dùng màu Nâu Đỏ sẫm `#8B4513`, không dùng màu đen `#000000`).

### 4.5 Điều Răn 5: Cấm Tuyệt Đối "Mixels" & "Jaggies"
- **Mixels (Mixed Pixels)**: Cấm tình trạng trong cùng một icon/sprite lại có pixel hạt to lẫn hạt nhỏ bất nhất.
- **Jaggies**: Mọi đường cong và đường xiên pixel phải tuân thủ dãy số bậc thang toán học đều đặn: `1-1-1`, `2-2-2`, hoặc `1-2-3`. Cấm nhảy bước dị tật như `1-3-1-4` làm đường nét méo mó.

---

## 5. Quy Chuẩn Khung Viền Phân Hạng Vật Phẩm (5-Tier Rarity Matrix)

Mọi icon trang bị khi hiển thị trong túi đồ lưới $6 \times 5$ hoặc thanh phím tắt Quickbar đều được lồng vào khung viền thẩm mỹ tương ứng với dữ liệu trong [`inventory-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/inventory-system.md):

| Bậc (Tier) | Tên Phân Cấp | Mã Viền Ngoài (Hex) | Hiệu Ứng Nền (Backdrop Style) | Chi Tiết Viền Góc (Corner Accent) |
| :---: | :--- | :---: | :--- | :--- |
| **Tier 1** | **Normal (Thường)** | `#4B5563` (Xám Thép) | Đá phiến đen mờ phẳng, không hiệu ứng hạt. | Vát góc $45^\circ$ phẳng phiu 2px. |
| **Tier 2** | **Rare (Hiếm)** | `#2563EB` (Lam Sapphire) | Ánh hào quang lam ngọc dịu nhẹ tỏa từ tâm ra biên. | 4 góc nẹp đinh tán thép mạ bạc. |
| **Tier 3** | **Legendary (Huyền Thoại)** | `#D97706` (Vàng Hổ Phách) | Khói tro than hồng bay nhẹ, nền tím than huyền bí. | Hoa văn sừng rồng vát nhọn bọc góc. |
| **Tier 4** | **Immortal (Bất Tử)** | `#DC2626` (Đỏ Huyết Ngọc) | Hào quang sát khí đỏ rực nhấp nháy chu kỳ 2.0s. | Khảm 4 mảnh ngọc đỏ rực tại 4 góc viền. |
| **Tier 5** | **Divine (Thần Thánh)** | `#F59E0B` + `#7C3AED` | Vòng xoáy thiên hà vũ trụ chuyển màu quang phổ. | Viền kép Hoàng kim cổ đại khắc ký tự Rune. |

---

## 6. Quy Trình Nghiệm Thu Chất Lượng (QA Gate Check Checklist)

Mọi tài nguyên đồ họa trước khi được merge vào nhánh `main` và import vào Unreal Engine 5 **BẮT BUỘC** phải vượt qua bảng kiểm định 6 bước sau:

- [ ] **1. Bài Kiểm Tra Bóng Đen (Blackout Silhouette Test)**:
  - *Thực hiện*: Đổ đen toàn bộ sprite/icon thành `#000000` trên nền trắng.
  - *Tiêu chuẩn*: Người chơi phải nhận biết được loại vật phẩm (kiếm/cung/trượng/bình thuốc) trong vòng 0.2 giây mà không cần nhìn màu sắc.
- [ ] **2. Bài Kiểm Tra Biên Trong Suốt (Zero Border Bleed Test)**:
  - *Thực hiện*: Quét mảng alpha 4 cạnh ngoài cùng (Top, Bottom, Left, Right).
  - *Tiêu chuẩn*: Giá trị Alpha tại viền ngoài cùng bắt buộc bằng $0$. Tuyệt đối không có pixel nào bị cụt góc, cụt chuôi kiếm hay đứt đầu mũi tên.
- [ ] **3. Bài Kiểm Tra Ngân Sách Màu (Color Budget Audit)**:
  - *Thực hiện*: Đếm số lượng màu độc nhất (Unique Color Palette).
  - *Tiêu chuẩn*: Một icon vật phẩm chuẩn $32 \times 32$ không được vượt quá **24 màu độc nhất**. Nếu vượt quá 32 màu chứng tỏ tranh bị nhòe dải màu AI (Color Bleed) $\rightarrow$ Trả về làm sạch màu.
- [ ] **4. Bài Kiểm Tra Đồng Bộ Tỷ Lệ Điểm Ảnh (Mixel-Free Audit)**:
  - *Thực hiện*: Đặt icon cạnh sprite nhân vật trên cùng một màn hình game.
  - *Tiêu chuẩn*: Kích thước hạt pixel của vật phẩm trên tay nhân vật phải đồng nhất $1:1$ với hạt pixel của cơ thể nhân vật.
- [ ] **5. Bài Kiểm Tra Khớp Khóa Paperdoll (Landmark Lockstep Test)**:
  - *Thực hiện*: Lồng ghép 4 lớp trang phục lên cơ thể nhân vật nữ/nam.
  - *Tiêu chuẩn*: Không có da thịt lòi ra ngoài áo giáp; tay cầm vũ khí không bị trôi khỏi khớp tay `(X: 96, Y: 76)` qua tất cả 8 hướng di chuyển.
- [ ] **6. Bài Kiểm Tra Tương Thích Ánh Sáng HD-2D (Normal Map / Depth Readiness)**:
  - *Thực hiện*: Chiếu đèn điểm (Point Light) của Unreal Engine quét qua sprite.
  - *Tiêu chuẩn*: Sprite nhận ánh sáng định hướng rõ rệt từ mặt phẳng vát cạnh, tôn vinh độ khối 2.5D chân thực.
