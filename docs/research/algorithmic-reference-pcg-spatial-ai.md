# Technical Research: High-Performance Game Algorithms for Future Roadmap

> **Document ID**: `TECH-RESEARCH-001`  
> **Topic**: Procedural Content Generation (WFC), Spatial Hashing, KD-Tree & Flocking Behaviors  
> **Source Reference**: [`InfiniBrains/Awesome-GameDev-Resources`](https://github.com/InfiniBrains/Awesome-GameDev-Resources)  
> **Target Engine**: Unreal Engine 5.8 (C++, Paper2D, Server-Authoritative MMO Architecture)  
> **Integration Milestones**: Alpha (Dungeons & Ambient Polish), Beta (Massive MMO Scale & Spatial Optimization)  
> **Status**: Approved for Future Backlog Integration  

---

## 1. Bối Cảnh & Mục Đích Lưu Trữ

Tài liệu này trích xuất, chuẩn hóa và chuyển giao các kiến thức thuật toán giá trị cao từ nguồn nghiên cứu [`InfiniBrains/Awesome-GameDev-Resources`](https://github.com/InfiniBrains/Awesome-GameDev-Resources) để chuẩn bị sẵn sàng tích hợp vào kiến trúc của **Project Ascendant** khi lộ trình phát triển (Roadmap) chạm đến các cột mốc tương ứng:
1. **Sinh địa hình hầm ngục ngẫu nhiên (Procedural Ruin Dungeons)** trong giai đoạn Alpha.
2. **Tối ưu hóa bảng cừu hận MMO diện rộng (Spatial Threat Table)** trong giai đoạn Beta.
3. **Hệ thống sinh thái dã ngoại & bầy quái nhỏ (Ambient Life & Swarm AI)** trong giai đoạn Polish.

---

## 2. Các Thuật Toán Trọng Điểm & Kế Hoạch Ứng Dụng

### A. Wave Function Collapse (WFC) — Sinh Bản Đồ & Hầm Ngục Thủ Tục
- **Nguồn tài liệu gốc**: `docs/artificialintelligence/01-pcg/WaveFunctionCollapse.md`
- **Nguyên lý cốt lõi**:
  - Giải bài toán thỏa mãn ràng buộc (Constraint Satisfaction Problem) dựa trên việc cực tiểu hóa độ hỗn loạn (Entropy).
  - Khởi tạo lưới không gian ở trạng thái chồng chập (Superposition) gồm tất cả các tile tiềm năng.
  - Sụp đổ (Collapse) ô có entropy thấp nhất trước, sau đó lan truyền ràng buộc (Constraint Propagation) sang các ô lân cận theo ma trận kề (Adjacency Matrix).
- **Ứng dụng vào Project Ascendant**:
  - **Mục tiêu**: Tự động sinh layout cho các phế tích ngầm, hầm mộ (Catacombs) và hang động dã ngoại tại 3 phân vùng (Verdant Bastion, Ashen Keep, Sanctum Fortress).
  - **Khớp nối kỹ thuật**:
    - Tích hợp vào C++ class dự kiến: `UPADungeonWFCGeneratorComponent`.
    - Định nghĩa ma trận kề cho các Tileset Isometric $32 \times 32\text{ px}$ (sàn đá, tường rêu, cửa vòm, ngõ cụt, hố vực).
    - Đảm bảo tính liên thông (Solvability): Tích hợp kiểm tra đường đi A* từ điểm Spawn đến Boss Room sau khi sụp đổ toàn bộ lưới WFC.
- **Cột mốc Roadmap dự kiến**: **Alpha Phase — Dungeon & Crypt Exploration**.

---

### B. Spatial Hashing & KD-Tree — Tối Ưu Hóa Tìm Kiếm Mục Tiêu MMO
- **Nguồn tài liệu gốc**: `docs/artificialintelligence/04-spatialhashing/` & `docs/artificialintelligence/05-kdtree/`
- **Nguyên lý cốt lõi**:
  - Ánh xạ tọa độ không gian 2D/3D liên tục $(X, Y)$ thành các chỉ số ô lưới rời rạc (Grid Buckets) thông qua hàm băm (Hash Function).
  - Biến các phép toán tìm kiếm lân cận (Proximity Query / Nearest Neighbor) từ độ phức tạp $O(N)$ hoặc $O(N^2)$ thành truy vấn trong $O(1)$ hoặc $O(\log N)$.
- **Ứng dụng vào Project Ascendant**:
  - **Mục tiêu**: Giảm tải CPU cho Dedicated Server khi xử lý hàng trăm Player và Quái vật chiến đấu đồng thời trong cùng một phân vùng thế giới mở.
  - **Khớp nối kỹ thuật**:
    - **Town Guard AI (visual-003)**: Thay vì mỗi lính gác quét toàn bộ Player trong map để tìm kẻ có `Karma < 0` trong bán kính $1000\text{ cm}$, lính gác chỉ truy vấn các Cell lân cận trong Spatial Hash Grid.
    - **Bảng Cừu Hận (UPAAggroComponent / net-003)**: Tối ưu hóa việc lan truyền sát thương diện rộng (AoE Threat Propagation) và kiểm tra ranh giới xích quái **AI Leash Boundary ($2500\text{ cm}$)**.
    - Class dự kiến: `FPASpatialHashGrid2D` tích hợp trong `UPAAggroComponent` và `UPAZoneManagerSubsystem`.
- **Cột mốc Roadmap dự kiến**: **Beta Phase — Open World Boss Raids & Contested Battles (System 17)**.

---

### C. Boids / Flocking Steering Behaviors — Sinh Vật Môi Trường & Quái Bầy Đàn
- **Nguồn tài liệu gốc**: `docs/artificialintelligence/assignments/flocking/`
- **Nguyên lý cốt lõi**:
  - Mô phỏng chuyển động bầy đàn tự nhiên theo 3 quy tắc của Craig Reynolds:
    1. **Separation**: Tránh đâm vào các cá thể lân cận.
    2. **Alignment**: Điều chỉnh hướng di chuyển theo vận tốc trung bình của bầy.
    3. **Cohesion**: Di chuyển về phía trọng tâm của nhóm.
- **Ứng dụng vào Project Ascendant**:
  - **Mục tiêu**:
    - **Ambient Fauna**: Đàn chim bay trên bầu trời Verdant Bastion, đàn dơi túa ra từ phế tích Ashen Keep khi Boss xuất hiện.
    - **Swarm Combat Minions**: Các bầy quái côn trùng nhỏ (nhện con, bọ hầm ngục) đi theo số lượng lớn (20–50 con) quấy rối người chơi mà không cần gắn full Behavior Tree nặng nề cho từng con.
  - **Khớp nối kỹ thuật**:
    - Class dự kiến: `APASwarmActorPool` kết hợp Paper2D Sprite Component nhẹ (Instanced / Batched Rendering).
- **Cột mốc Roadmap dự kiến**: **Alpha/Beta Transition — World Immersion & Ecology Polish**.

---

## 3. Bảng Ánh Xạ Roadmap & Hành Động Kỹ Thuật

| Hạng Mục Kỹ Thuật | Thuật Toán Ứng Dụng | File Nguồn Tham Khảo | Class C++ Dự Kiến | Milestone Kích Hoạt |
| :--- | :--- | :--- | :--- | :---: |
| **Hầm Ngục Phế Tích Tự Động** | Wave Function Collapse (WFC) | `01-pcg/WaveFunctionCollapse.md` | `UPADungeonWFCGeneratorComponent` | **Alpha Sprint** (Dungeons) |
| **Tối Ưu Hóa Server Cừu Hận & Lính Gác** | Spatial Hashing Grid 2D | `04-spatialhashing/` | `FPASpatialHashGrid2D` | **Beta Sprint** (Massive MMO Scale) |
| **Tìm Mục Tiêu Boss Đa Điểm** | KD-Tree Nearest Neighbor | `05-kdtree/` | `FPAKdTree2D` | **Beta Sprint** (World Boss Raids) |
| **Bầy Sinh Vật & Quái Bọ Đàn** | Boids Steering Behaviors | `assignments/flocking/` | `APASwarmActorPool` | **Polish Sprint** (World Immersion) |

---

## 4. Hướng Dẫn Kích Hoạt Cho Kỹ Sư Khi Đến Sprint

Khi lập kế hoạch Sprint cho các Epic tương ứng:
1. Đọc lại tài liệu đặc tả thuật toán này kết hợp mã nguồn mẫu tại repository tham chiếu.
2. Thiết kế C++ Header và Struct bọc trong namespace `ProjectAscendant`.
3. Viết Unit Test headless kiểm tra tính đúng đắn trước khi gắn vào Gameplay Component:
   - *WFC Test*: Khởi tạo lưới $10 \times 10$, xác nhận không có tile nào vi phạm ma trận kề và có đường đi thông suốt.
   - *Spatial Hash Test*: Thêm 500 thực thể ngẫu nhiên, xác nhận truy vấn bán kính $1000\text{ cm}$ trả về chính xác tập hợp con trong thời gian $< 0.05\text{ ms}$.
