# Kế Hoạch Phân Công Công Việc 5 Agent (5-Agent Execution & WBS Plan)

> **Mã Kế Hoạch**: `PLAN-5-AGENT-2026-V1`  
> **Dự Án**: Project Ascendant (2.5D Isometric Hardcore ARPG MMO on UE 5.8)  
> **Nguyên Tắc**: Khắt khe về pháp lý, phân định rõ ràng giữa Tooling - Production - QA, kiểm thử hồi quy nghiêm ngặt.  
> **Thứ Tự Kích Hoạt**: `Gói 1 (Agent 1: Asset/Legal)` → `Gói 2 (Agent 2: Aseprite Tooling)` → `Gói 5 (Agent 5: Art Gate)` → `Gói 3 (Agent 3: PCG)` → `Gói 4 (Agent 4: GAS/Combat)`  

---

## 1. Sơ Đồ Điều Phối & Luồng Dữ Liệu (Execution Flow)

```
                 ORCHESTRATOR
                      │
          ┌───────────┴───────────┐
          ▼                       ▼
      AGENT 1                  AGENT 2
   Asset intake            Aseprite pipeline
   (CC0 Pack-by-Pack)      (Lua & Export Script)
          │                       │
          └───────────┬───────────┘
                      ▼
                  AGENT 5
                  ART GATE
              (Approve / Reject)
                      │
              ┌───────┴───────┐
              ▼               ▼
           AGENT 3          AGENT 4
           PCG             GAS/Combat
      (World Biome)      (Hardening & Regression)
```

---

## 2. Chi Tiết WBS & Phân Công Nhiệm Vụ Từng Agent

### 🛡️ AGENT 1: ASSET CURATOR & LEGAL COMPLIANCE OFFICER (Gói 1)
- **Vai trò**: Quản trị nguồn gốc pháp lý và nhập liệu tài nguyên bên thứ 3 (Asset Intake & Provenance).
- **Nguyên tắc bất di bất dịch**:
  - Không mặc định mọi asset trong các kho mở là CC0. Phải rà soát từng pack/thư mục cụ thể.
  - Phân biệt rõ ràng giữa Code License (thường là MIT/Apache) và Media License (CC0, CC-BY, v.v.).
  - `THIRD_PARTY_ASSETS.md` là **Intake Ledger** (sổ cái nhập liệu), không phải giấy phép thay thế license gốc.
  - Chỉ sao chép asset vào dự án sau khi license được xác thực.
- **Tiêu chuẩn Provenance (học từ `voidclad`)**:
  - Source URL (đường dẫn kho/pack gốc).
  - Exact pack / file path.
  - License xác thực (kèm link điều khoản).
  - Acquisition date (YYYY-MM-DD).
  - Checksum SHA-256 đối soát.
  - Modification status (Unmodified / Palettized / Cropped).
  - Commercial redistribution status (Được phép thương mại & tái phân phối hay không).
- **Danh mục Tasks cụ thể**:
  - [ ] **Task 1.1**: Khởi tạo file sổ cái [`THIRD_PARTY_ASSETS.md`](file:///mnt/Data/Projects/project-games/ProjectAscendant/THIRD_PARTY_ASSETS.md) tại thư mục gốc.
  - [ ] **Task 1.2**: Rà soát pack-by-pack repo `doficia/project-cordon-sprites` (kiểm tra `LICENSE`, cấu trúc `markdown/` và các spritesheet vũ khí/công cụ).
  - [ ] **Task 1.3**: Rà soát pack-by-pack repo `Papyszoo/CC0-Public-Domain-Sprites` (xác thực các pack `Items & Icons`, `Tilesets & Environments`, `UI`).
  - [ ] **Task 1.4**: Tuyển chọn đợt 1 cho các hệ thống đang cần art:
    - 21 phôi vũ khí cơ sở cho Itemization (`item-005`).
    - Quặng rèn, đá quý, đe búa cho Blacksmithing (`item-003`).
    - Icon slot kho đồ cho Combat HUD (`presentation-ui`).
  - [ ] **Task 1.5**: Lưu trữ asset đã thẩm định vào `Content/Art/ThirdParty/` kèm bảng SHA-256 tương ứng.

---

### 🎨 AGENT 2: TECHNICAL ART & ASEPRITE TOOLING ENGINEER (Gói 2)
- **Vai trò**: Xây dựng toolchain và template Aseprite cho họa sĩ.
- **Nguyên tắc bất di bất dịch**:
  - Làm trước template + export pipeline.
  - Chưa cố import metadata thẳng vào Paper2D nếu chưa xác định rõ format/API import mà engine/project đang dùng.
- **Cấu trúc thư mục đầu ra chuẩn**:
  ```
  Tools/Aseprite/
  ├── lua/
  │   ├── generate_template.lua
  │   └── setup_palette.lua
  ├── templates/
  │   ├── master_rig_01.aseprite
  │   ├── master_rig_02.aseprite
  │   ├── master_rig_03.aseprite
  │   └── master_rig_04.aseprite
  └── aseprite_export_pipeline.py
  ```
- **Danh mục Tasks cụ thể**:
  - [ ] **Task 2.1**: Thiết lập thư mục `Tools/Aseprite/lua/` và `Tools/Aseprite/templates/`.
  - [ ] **Task 2.2**: Viết `setup_palette.lua` định nghĩa bảng màu 4-tone ramp hue-shifting chuẩn của `SPEC-ART-2026-09-23-V2`.
  - [ ] **Task 2.3**: Viết `generate_template.lua` tạo canvas $128 \times 128\text{ px}$, phân tầng sẵn các layers Paperdoll (`LowerBody`, `UpperBody`, `HelmCrest`, `Tabard`, `Hand_R`, `Hand_L`), đánh dấu mốc Pivot chân $(64, 114)$, Khớp eo $Y=80$, Hand Sockets $(96, 76), (32, 76)$ và animation tags chuẩn.
  - [ ] **Task 2.4**: Sinh 4 file template `.aseprite` cho 4 Master Rigs: `master_rig_01` (HeavyTank), `02` (Agility), `03` (Caster), `04` (Monk).
  - [ ] **Task 2.5**: Viết `aseprite_export_pipeline.py` chạy qua CLI (`aseprite -b`) xuất spritesheet PNG + JSON metadata frames/slices tiêu chuẩn.

---

### ⚖️ AGENT 5: ART DIRECTOR & SILHOUETTE QA GATEKEEPER (Gói 5)
- **Vai trò**: Cửa khẩu kiểm định chất lượng độc lập (Quality Gate).
- **Nguyên tắc bất di bất dịch**:
  - **Là GATE, KHÔNG PHẢI người sửa asset**: Nhận đầu vào từ Agent 1 (CC0) hoặc Agent 2 (Template/Export) → Thẩm định → Trả về **Approve** hoặc **Reject** kèm lý do kỹ thuật.
  - Tuyệt đối không tự chỉnh sửa asset để tránh biến khâu QA thành khâu sản xuất (production).
- **Tiêu chí nghiệm thu (Gate Criteria)**:
  - *Tiêu chuẩn A*: Tỷ lệ pixel vuông $1:1$ (Zero Mixels).
  - *Tiêu chuẩn B*: Bảng màu 4-tone ramp, nguồn sáng cố định góc $10$ giờ ($45^\circ$).
  - *Tiêu chuẩn C*: Pivot chân cố định $(64, 114)$, khớp eo phẳng phiu tại $Y=80$.
  - *Tiêu chuẩn D*: Vượt qua `qa_silhouette_check.py` (Pure black convert, nhận diện tốt ở $32 \times 32$ và $16 \times 16$).
- **Danh mục Tasks cụ thể**:
  - [ ] **Task 5.1**: Ban hành tài liệu quy chế kiểm định `production/qa/art-gate-criteria.md`.
  - [ ] **Task 5.2**: Kiểm định đợt 1 cho các asset do Agent 1 thu gom.
  - [ ] **Task 5.3**: Kiểm định cấu trúc file template và metadata do Agent 2 xuất ra.
  - [ ] **Task 5.4**: Lập báo cáo thẩm định chính thức `production/qa/art-gate-report-batch-01.md`.

---

### 🌲 AGENT 3: WORLD & LEVEL PCG ARCHITECT (Gói 3)
- **Vai trò**: Kiến trúc sư dựng môi trường dã ngoại bằng Unreal PCG.
- **Nguyên tắc bất di bất dịch**:
  - Làm `PCG_WorldBiomeGraph` + `PCG_CitadelRoadSplineGraph` trước để tạo giá trị ngay cho bản đồ.
  - Thuật toán WFC C++ hầm ngục được giữ ở nhánh nghiên cứu (experimental/backlog), không để block tiến độ dựng môi trường.
- **Vị trí tài nguyên**: `Content/Maps/PCG/`
- **Danh mục Tasks cụ thể**:
  - [ ] **Task 3.1**: Thiết kế cấu trúc đồ thị `PCG_CitadelRoadSplineGraph` nối 3 Citadel Safe Zones (*Verdant Bastion, Ashen Keep, Sanctum Fortress*) theo Spline, tự động chiếu (project) xuống landscape và tạo độ dốc hợp lý.
  - [ ] **Task 3.2**: Thiết kế đồ thị `PCG_WorldBiomeGraph` rải thảm thực vật (Foliage) và đá tảng (Rocks) theo mật độ, có volume exclusion loại trừ vùng an toàn Citadel ($R = 5000\text{ cm}$).
  - [ ] **Task 3.3**: Viết tài liệu hướng dẫn kỹ thuật Level PCG tại `docs/architecture/pcg-environment-architecture.md`.

---

### ⚔️ AGENT 4: COMBAT & GAS HARDENING ENGINEER (Gói 4)
- **Vai trò**: Kỹ sư kiểm thử độ tin cậy và hồi quy cho hệ thống chiến đấu GAS.
- **Nguyên tắc bất di bất dịch**:
  - Không mở rộng architecture nếu Vertical Slice đã ổn định.
  - Tuyệt đối không copy-paste code từ các sample repo (`GASDocumentation`, `GASam`, `GAS_Combo`) vào production. Chỉ dùng chúng làm tài liệu tham chiếu và testbed.
  - Tập trung 100% vào **Regression Testing & Edge Cases**.
- **Danh mục Tasks cụ thể**:
  - [ ] **Task 4.1**: Kiểm thử hồi quy **Input Buffer**: Bấm đòn đánh sớm trước khi đòn hiện tại kết thúc, đảm bảo buffer queue nhận và kích hoạt ngay frame đầu tiên sau khi kết thúc đòn.
  - [ ] **Task 4.2**: Kiểm thử hồi quy **Cancel Window**: Ngắt đòn đánh bằng Dash I-frame trong khoảng $[0.2\text{s} - 0.4\text{s}]$, đảm bảo `PAGameplayAbility_MeleeAttack` hủy sạch sẽ không gây rò rỉ gameplay tags.
  - [ ] **Task 4.3**: Kiểm thử hồi quy **Combo State Reset**: Đứng yên $> 1.2\text{s}$ hoặc dính đòn choáng (Stun/HitStun) thì combo reset về Hit 1.
  - [ ] **Task 4.4**: Kiểm thử hồi quy **Network Replication & Packet Loss**: Giả lập mất gói tin 5-10%, đảm bảo Client không bị desync vị trí hoặc kẹt hoạt ảnh vĩnh viễn.
  - [ ] **Task 4.5**: Xây dựng test suite C++ headless `PACombatRegressionHardeningTests.cpp` chạy tự động trên Linux Editor.

---

## 3. Thứ Tự Triển Khai Thực Tế

1. **Bước 1 (Gói 1 - Agent 1)**: Khởi tạo `THIRD_PARTY_ASSETS.md` và kiểm tra provenance cho các pack từ `doficia` & `Papyszoo`.
2. **Bước 2 (Gói 2 - Agent 2)**: Xây dựng toolchain `Tools/Aseprite/` (Lua scripts, templates, export pipeline).
3. **Bước 3 (Gói 5 - Agent 5)**: Chạy Art Gate thẩm định toàn bộ đầu ra của Gói 1 và Gói 2.
4. **Bước 4 (Gói 3 - Agent 3 & Gói 4 - Agent 4)**: Sau khi Art Gate thông qua, Agent 3 và Agent 4 chạy độc lập song song (PCG World Environment & GAS Hardening Regression).
