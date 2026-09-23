# Master Art Bible — Project Ascendant

> **Status**: Approved (Sections 1–4: Visual Identity Foundation)  
> **Target Aesthetic**: 2.5D Isometric HD-2D (PaperZD Pixel Art Sprites + 3D Volumetric Lighting)  
> **Engine**: Unreal Engine 5.7 (Lumen, Niagara VFX, Paper2D/PaperZD)  
> **Last Updated**: 2026-09-16  
> **Core References**: *Octopath Traveler*, *Triangle Strategy*, *Hades*, *Dark Souls*  

---

## 1. Visual Identity Statement

### 1.1. Quy Tắc Thị Giác Cốt Lõi (One-Line Visual Rule)
> **"Pixel Precision Meets Volumetric Darkness — Tinh hoa Pixel Art 2D sắc sảo đặt trong không gian chiều sâu ánh sáng động 3D u tối."**

Mọi quyết định về tạo hình, dựng shader và ánh sáng trong game đều phải trả lời câu hỏi: *“Yếu tố này có tôn vinh nét vẽ pixel sắc nét của nhân vật và làm nổi bật sự nguy hiểm u tối của thế giới hay không?”*

### 1.2. Ba Nguyên Tắc Thị Giác Trụ Cột (Supporting Visual Principles)

1. **Nguyên Tắc 1: Nhận Diện Dáng Hình Tuyệt Đối (Silhouette Readability First)**
   - *Nguyên tắc*: Ở góc camera Isometric -45° với hàng chục nhân vật trên màn hình, vũ khí và tư thế của 4 Class (Vanguard, Ranger, Arcanist, Acolyte) phải được nhận diện ngay lập tức ở kích cỡ thu nhỏ (Thumbnail size) thông qua hình khối đặc trưng.
   - *Design Test*: Khi có sự mơ hồ về việc bổ sung chi tiết trang phục, nguyên tắc này yêu cầu **loại bỏ chi tiết vụn vặt để giữ vững độ tương phản và dáng hình viền sắc nét**.
   - *Trụ cột phục vụ*: *True Skill Expression* (Nhận biết nhanh để né đòn).

2. **Nguyên Tắc 2: Ánh Sáng Khắc Họa Sự Hoang Tàn (Lighting Carries the Danger)**
   - *Nguyên tắc*: Thế giới dã ngoại mang tông màu trầm tối, hoang phế. Mọi điểm sáng rực rỡ xuất hiện đều mang ý nghĩa sống còn: đốm lửa trại an toàn tại Sanctuary, hào quang kỹ năng thi triển, hoặc vùng đỏ rực báo động đòn quét của Boss (Telegraph Decals).
   - *Design Test*: Khi băn khoăn về độ sáng của một khu vực, nguyên tắc này yêu cầu **để bóng tối bao trùm cảnh quan và chỉ dùng ánh sáng động để dẫn dắt sự chú ý của người chơi vào tâm điểm chiến đấu**.
   - *Trụ cột phục vụ*: *High Stakes Wilderness & Living World*.

3. **Nguyên Tắc 3: Gắn Kết Chân Thực Vào Không Gian 3D (Authentic Grounding)**
   - *Nguyên tắc*: Nhân vật 2D Sprite tuyệt đối không phải là những hình dán phẳng (flat stickers) trôi nổi trên mặt đất. Sprite nhân vật phải nhận ánh sáng thực từ Lumen, đổ bóng chiếu xiên thời gian thực xuống địa hình 3D, và in bóng phản chiếu trên mặt nước.
   - *Design Test*: Khi cân nhắc giữa hiệu ứng 2D vẽ sẵn và tích hợp hiệu ứng ánh sáng động 3D, nguyên tắc này yêu cầu **chọn giải pháp tương tác ánh sáng vật lý để tạo cảm giác hòa quyện HD-2D chân thực**.
   - *Trụ cột phục vụ*: *LitRPG Immersion*.

---

## 2. Mood & Atmosphere

| Trạng Thái Game | Cảm Xúc Mục Tiêu (Mood Target) | Đặc Tính Ánh Sáng (Lighting Character) | Yếu Tố Thị Giác Dẫn Dắt | Mức Độ Năng Lượng |
|---|---|---|---|:---:|
| **1. Khám Phá Dã Ngoại (Wilderness Exploration)** | Căng thẳng, cô độc, cảnh giác cao độ (*Suspense, Desolation*) | Ánh sáng trăng mờ lạnh (~6500K), sương mù thể tích (Volumetric Fog), tương phản trung bình. | Rêu lân tinh phát sáng le lói trên phế tích cổ; đom đóm trôi dạt trong đêm tối. | *Measured, Tense* |
| **2. Giao Tranh Boss Thế Giới (Boss Encounter)** | Kịch tính, ngột ngạt, dồn dập (*High Stakes, Adrenaline*) | Tương phản cực cao (High Contrast). Đấu trường chìm trong bóng tối, chỉ được thắp sáng bởi ánh mắt rực lửa của Boss (~2500K). | Decal cảnh báo quét đòn đỏ rực (Telegraphs) tương phản mạnh với mặt đất đá đen; mảnh vụn đá nổ tung. | *Frenetic, Explosive* |
| **3. Trạm Nghỉ An Toàn (Sanctuary / Camp)** | Nhẹ nhõm, ấm cúng, an tâm (*Relief, Haven*) | Ánh lửa vàng hổ phách ấm áp (~2200K) từ lò than Thợ rèn xua tan bóng tối trong bán kính 1000cm. | Tàn than hồng bay lơ lửng, khói bếp bốc lên nhẹ nhàng; mặt nước hồ phản chiếu ánh lửa. | *Contemplative, Peaceful* |
| **4. Chạm Trán Đồ Tể (Wanted PK Confrontation)** | Đe dọa, rình rập, nguy hiểm chết chóc (*Dread, Predatory*) | Tông màu toàn cảnh bị khử bão hòa nhẹ (Desaturate 15%), nổi bật luồng hào quang đỏ thẫm (Crimson Outline) quanh kẻ PK. | Vệt bóng mờ đen kéo dài sau mỗi bước chạy của kẻ Đồ Tể; ánh mắt rực đỏ như thú săn mồi. | *Sudden, Threatening* |

---

## 3. Shape Language

### 3.1. Triết Lý Tạo Dáng Nhân Vật (Character Silhouette Philosophy)
Mỗi Class nhân vật trong *Project Ascendant* được định hình bởi một khối hình học cơ bản bất biến, bảo đảm người chơi nhận diện ngay tức khắc ở góc nhìn chéo -45° từ khoảng cách 12 mét:

- **Vanguard (Tiên Phong)** — **Khối Hình Thang / Vuông (Heavy Trapezoid)**:
  - *Tạo hình*: Trọng tâm thấp, vai giáp bản to gồ ghề, chiếc khiên lớn chiếm $40\%$ bề ngang thân hình. Tư thế đứng bám rễ vững chắc như pháo đài thép.
- **Ranger (Du Mục)** — **Tam Giác Ngược / Đường Cắt Xiên (Inverted Triangle & Sharp Slits)**:
  - *Tạo hình*: Dáng người thon gọn, áo choàng rách bay tà nhọn theo gió, cây cung dài vắt chéo qua lưng tạo thành đường xiên động lực học.
- **Arcanist (Thuật Sĩ)** — **Hình Thoi / Hình Trụ Đứng (Diamond & Vertical Column)**:
  - *Tạo hình*: Tà áo choàng dài rủ chạm đất, cây trượng dựng thẳng đứng, quanh người có các khối ngọc ma pháp bay lơ lửng tạo thành hình thoi bảo hộ.
- **Acolyte (Tu Sĩ)** — **Khối Đồng Hồ Cát / Chữ Thập (Hourglass & Sacred Cross)**:
  - *Tạo hình*: Cầu vai giáp nhô cao kết hợp dải khăn kinh thánh buông dài đối xứng; tay cầm chùy xích và đỉnh hương tạo thế cân bằng tâm linh.

### 3.2. Ngôn Ngữ Hình Khối Môi Trường (Environment Geometry)
- **Kiến trúc Hoang Dã & Di Tích**: Thống trị bởi **Đường nét góc cạnh, nứt nẻ và gãy khúc (Angular & Fractured Brutalism)** — những cột đá gothic đổ nát, vách đá sắc nhọn thể hiện sự khắc nghiệt và suy tàn của thế giới.
- **Thiên Nhiên Bị Tha Hóa**: Sử dụng **Đường cong xoắn vặn bất thường (Gnarled & Distorted Curves)** — rễ cây cổ thụ đen kịt quấn chặt tảng đá, nấm phát quang mọc chùm méo mó.

### 3.3. Hình Học Boss & Quái Vật (Enemy Shape Grammar)
- **Stone Golem Boss**: Khối cự thạch khổng lồ không đối xứng. Các phiến đá sắc nhọn chắp vá quanh lõi năng lượng phát sáng rực rỡ; tạo cảm giác bất khả chiến bại nhưng để lộ những vết nứt hình học nơi người chơi có thể nhắm vào để phá Posture.

### 3.4. Ngôn Ngữ Giao Diện UI (UI Shape Grammar)
- Giao diện CommonUI sử dụng các đường viền vát góc $45^\circ$ (Chamfered Corners) đồng điệu với góc nghiêng camera Isometric, tạo cảm giác hiện đại sắc cạnh nhưng đậm chất huyền ảo đen tối.

---

## 4. Color System

### 4.1. Bảng Màu Cốt Lõi (Primary Palette - 6 Mã Màu Nền Tảng)

```
[#121316] Obsidian Black    ── Nền tảng thế giới, đá phế tích, bóng tối sâu thẳm
[#2C3038] Charcoal Slate    ── Giáp sắt hoen rỉ, thân gỗ cháy, đất đá trung tính
[#9E1A1A] Crimson Blood     ── Vùng báo động đỏ, sát khí Đồ Tể, vệt máu tử trận
[#E6A122] Amber Gold        ── Ngọn lửa Sanctuary, tiền tệ Vàng, trang bị Thần Thoại
[#1ED5C6] Spectral Cyan     ── Hào quang Lướt né I-frame, ma pháp Arcanist, mana
[#E8ECEB] Sacred Bone White ── Ánh sáng Thánh tẩy Acolyte, chữ viết UI sắc nét
```

### 4.2. Từ Điển Màu Ngữ Nghĩa (Semantic Color Vocabulary)

| Màu Sắc | Mã Màu | Ý Nghĩa Trong Trò Chơi | Ứng Dụng Thực Tế |
|---|:---:|---|---|
| 🔴 **Đỏ Máu (Crimson)** | `#9E1A1A` | **Cực Kỳ Nguy Hiểm & Tử Trận** | Decal cảnh báo đòn Boss (Telegraph), Thanh Máu kẻ thù, Hào quang kẻ PK Red-Name. |
| 🟡 **Vàng Hổ Phách (Amber)** | `#E6A122` | **An Toàn, Cơ Hội & Giá Trị Cao** | Lửa trại Sanctuary, Thanh Thể khí Boss bị vỡ (Stagger), Đồ rơi Legendary. |
| 🔵 **Xanh Lam/Ngọc (Cyan)** | `#1ED5C6` | **Ma Thuật & Bất Khả Xâm Phạm** | Thanh Thể lực & Mana, Vệt bóng mờ Pixel Dash (I-Frame), Kỹ năng dịch chuyển. |
| ⚪ **Trắng Xương (Bone White)** | `#E8ECEB` | **Thông Tin Trung Tính & Thánh Thiện** | Chữ số sát thương cơ bản, Font chữ HUD, Hào quang hồi phục của Acolyte. |
| 🟣 **Tím Hư Không (Void Purple)** | `#6A1B9A` | **Tha Hóa & Nguyền Rủa** | Vùng đất Contested độc hại, Hiệu ứng trừ giáp, Kỹ năng Class Dark Knight ẩn. |

### 4.3. Thiết Kế An Toàn Cho Người Mù Màu (Colorblind Accessibility)
Mọi màu sắc mang tính cảnh báo sống còn **tuyệt đối không được đứng đơn độc**, bắt buộc phải có hỗ trợ hình học đi kèm:
1. **Decal Báo Động Đỏ (Boss Telegraph)**: Luôn đi kèm vân sọc chéo $45^\circ$ chuyển động nhấp nháy bên trong vòng tròn + âm thanh rít gió (Audio Cue).
2. **Trạng Thái Gục Ngã (Posture Broken)**: Đi kèm biểu tượng **Chiếc Khiên Vỡ** nhấp nháy trên đầu Boss và nút bấm `[F]` màu vàng nổi bật.
3. **Kẻ PK Đồ Tể (Wanted Outlaw)**: Ngoài tên màu đỏ, luôn có **Biểu Tượng Đầu Lâu Màu Trắng** hiển thị cố định bên cạnh thanh máu.

---

## 5. Character Design Direction
*(To be detailed in Production Phase)*

## 6. Environment Design Language
*(To be detailed in Production Phase)*

## 7. UI/HUD Visual Direction
*(To be detailed in Production Phase)*

## 8. Asset Standards & Pipeline (Pixel HD-2D)
- **Pixel Density (PPU)**: 32 pixels per Unreal Unit (100 units = 1 meter).
- **Texture Filtering**: Nearest Neighbor (Strictly NO Mipmapping).
- **Atlas Resolution**: Max $2048 \times 2048$ per character flipbook set.
- **Lighting Shader**: Custom unlit/lit hybrid sprite material receiving directional sun + local point lights with custom normal mapping.

## 9. Style Prohibitions
- Không sử dụng hiệu ứng làm mờ vân ảnh (Bilinear / Trilinear filtering).
- Không vẽ đổ bóng giả lập (baking flat shadow) trên sprite nhân vật; bóng phải do Lumen tính toán thời gian thực.
- Không sử dụng màu Neon quá gắt ngoài nhóm kỹ năng phép thuật chuyên biệt.
