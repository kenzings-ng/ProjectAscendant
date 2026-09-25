# Kiến Trúc Môi Trường Thế Giới Mở Bằng Unreal PCG (PCG Environment Architecture)

> **Mã Tài Liệu**: `ARCH-PCG-ENV-2026-V1`  
> **Dự Án**: Project Ascendant (2.5D Isometric Hardcore ARPG MMO on UE 5.8)  
> **Tài Liệu Tham Chiếu**: [`knissen/unreal-pcg-examples`](https://github.com/knissen/unreal-pcg-examples), [`lostcrowdev/EssentialUE5PCG`](https://github.com/lostcrowdev/EssentialUE5PCG)  
> **Mục Tiêu**: Tự động hóa việc dựng đường mòn kết nối và rải thảm thực vật/đá tảng cho 3 Phân Vùng Lớn (*Verdant Bastion, Ashen Keep, Sanctum Fortress*), loại trừ 100% việc phải đặt tay thủ công.  

---

## 1. Tổng Quan Kiến Trúc PCG Cho Project Ascendant

Dự án áp dụng khung **Procedural Content Generation Framework (PCG)** của Unreal Engine 5.8 để giải bài toán xây dựng thế giới mở quy mô lớn mà vẫn giữ được độ chính xác của góc nhìn Isometric $2.5\text{D}$:

```
                                  [ LANDSCAPE DATA ]
                                          │
            ┌─────────────────────────────┴─────────────────────────────┐
            ▼                                                           ▼
┌───────────────────────────────────────┐   ┌───────────────────────────────────────┐
│     PCG_CitadelRoadSplineGraph        │   │         PCG_WorldBiomeGraph           │
│ (Sinh đường mòn liên kết 3 Tòa Thành) │   │ (Rải thảm thực vật & Đá tảng dã ngoại)│
└───────────────────┬───────────────────┘   └───────────────────┬───────────────────┘
                    │                                           │
                    ▼                                           ▼
         [ Spline Projection ]                       [ Poisson Disk Sampling ]
         [ Path Border Falloff ]                     [ Citadel Safe Zone Mask ]
         [ Cobblestone Decals ]                      [ Biome Foliage Palette ]
                    │                                           │
                    └─────────────────────┬─────────────────────┘
                                          ▼
                             [ UNREAL WORLD PARTITION ]
                        (Stream & Render ở 60 FPS mượt mà)
```

---

## 2. Đặc Tả Chi Tiết 2 Đồ Thị PCG Cốt Lõi

### A. Đồ Thị 1: `PCG_CitadelRoadSplineGraph` (Đường Mòn Nối 3 Tòa Thành)
- **Vị trí tài nguyên**: `Content/Maps/PCG/PCG_CitadelRoadSplineGraph.uasset`
- **Mục tiêu**: Nối liền 3 Citadel Safe Zones (*Verdant Bastion* $\leftrightarrow$ *Ashen Keep* $\leftrightarrow$ *Sanctum Fortress*) bằng các tuyến đường mòn tự nhiên uốn lượn theo địa hình.
- **Chuỗi Node Kỹ Thuật (Node Pipeline)**:
  1. **Get Spline Data**: Đọc dữ liệu từ `USplineComponent` kéo dài giữa các cổng thành.
  2. **Spline Sampler**:
     - Mode: `Distance`.
     - Step Distance: $100\text{ cm}$.
     - Subdivide: Bật làm mịn tiếp tuyến góc cua.
  3. **Projection to Landscape**:
     - Chiếu toàn bộ điểm lấy mẫu xuống mặt phẳng Landscape theo phương thẳng đứng ($Z$).
     - Giới hạn độ dốc tối đa $25^\circ$ để nhân vật không bị trượt khi di chuyển 2.5D.
  4. **Road Profile & Width**:
     - Lòng đường chính (Main Path): Bán kính $200\text{ cm}$ (Tổng bề rộng $400\text{ cm}$ đủ cho 4 người chơi cưỡi ngựa hoặc chạy song song).
     - Ranh giới chuyển tiếp (Falloff Shoulder): $100\text{ cm}$ hai bên mép đường, mật độ sỏi giảm dần.
  5. **Spawner Nodes**:
     - Spawn Mesh/Decal: Đá cuội lát đường, vệt mòn đất trơ rễ cây.
     - Spawn Đèn đường / Cột mốc dã ngoại (Roadside Lanterns / Milestone Props) ở mỗi khoảng cách $1500\text{ cm}$.

---

### B. Đồ Thị 2: `PCG_WorldBiomeGraph` (Thảm Thực Vật & Đá Tảng Theo Phân Vùng)
- **Vị trí tài nguyên**: `Content/Maps/PCG/PCG_WorldBiomeGraph.uasset`
- **Mục tiêu**: Phủ xanh và trang trí cảnh quan dã ngoại cho 3 Biome riêng biệt, tự động chừa trống các khu vực cấm và đấu trường Boss.
- **Chuỗi Node Kỹ Thuật (Node Pipeline)**:
  1. **Surface Sampler (Landscape Data)**:
     - Phương pháp lấy mẫu: `Poisson Disk Sampling` để tránh các cây/đá đâm xuyên vào nhau (chống visual clipping).
     - Bán kính tối thiểu giữa 2 đại thụ: $800\text{ cm}$.
     - Bán kính tối thiểu giữa bụi rậm: $200\text{ cm}$.
  2. **Citadel Safe Zone Exclusion Filter**:
     - Sử dụng node **Distance Filter** kết hợp với tọa độ 3 Citadel Safe Zones.
     - Bán kính cách ly: $R = 5000\text{ cm}$ quanh tâm mỗi Tòa Thành. Mọi cây cối dã ngoại dày đặc bị triệt tiêu 100% trong phạm vi này để nhường chỗ cho kiến trúc thành lũy và tầm nhìn NPC lính gác.
  3. **Slope & Height Filter**:
     - Vách đá dốc $> 45^\circ$: Loại trừ cây cối, chỉ cho phép rải rêu bám và đá tảng nhô ra.
     - Vùng đất trũng ven sông: Ưu tiên sậy, lau và rêu nước.
  4. **Biome Palette Partitioning (Bảng Phân Bổ Tài Nguyên)**:
     - **Biome 1: Verdant Bastion** (Thung lũng trù phú): Cây sồi đại thụ, thông non, hoa dại vàng/trắng, tảng đá phủ rêu xanh.
     - **Biome 2: Ashen Keep** (Chiến trường hoang tàn): Cây gỗ khô cháy đen, gai nhọn, đá nham thạch nứt nẻ, gò tro tàn.
     - **Biome 3: Sanctum Fortress** (Thánh địa tuyết phủ): Thông tuyết, bạch dương trắng, cột đá hoa cương phong hóa.

---

## 3. Quy Trình Vận Hành & Tích Hợp Cho Level Designer

1. Mở màn chơi thế giới mở (World Partition Level: `L_World_Master`).
2. Kéo thả Blueprint `BP_CitadelRoadManager` vào level, nối các điểm Spline giữa 3 cổng thành.
3. Thêm Volume `PCG_WorldBiomeVolume` bao trùm toàn bộ khu vực phân vùng cần sinh cảnh quan.
4. Đồ thị PCG sẽ tự động chạy trong Editor (hoặc lúc Build Cook) để sinh ra hàng triệu thể hiện thực vật dạng **Hierarchical Instanced Static Mesh (HISM)**, đảm bảo tiêu chuẩn 60 FPS ổn định trên phần cứng mục tiêu.
