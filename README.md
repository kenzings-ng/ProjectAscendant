# Project Ascendant

<p align="center">
  <strong>2.5D Isometric Hardcore Action RPG MMO</strong>
  <br />
  <em>Unreal Engine 5 | Gameplay Ability System (GAS) | Iris Replication Netcode | Paper2D & PaperZD</em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Unreal_Engine-5.8-313131?logo=unrealengine" alt="UE5">
  <img src="https://img.shields.io/badge/Language-C%2B%2B_20-00599C?logo=c%2B%2B" alt="C++20">
  <img src="https://img.shields.io/badge/Architecture-Server--Authoritative-orange" alt="Architecture">
  <img src="https://img.shields.io/badge/Sprints_Completed-5%2F6-brightgreen" alt="Sprints">
  <img src="https://img.shields.io/badge/Automation_Tests-100%25_Passing-success" alt="Tests">
</p>

---

## ⚔️ Tổng quan (Elevator Pitch)

**Project Ascendant** là tựa game **2.5D Isometric Action RPG MMO Thế Giới Mở (Open World)** hardcore lấy cảm hứng từ thể loại LitRPG và lối chơi biểu đạt kỹ năng cao (*Hades, Elden Ring, Albion Online*):

- **Chiến đấu cạnh tranh thực tế (Contested Combat / Open Tagging)**: Không có cơ chế khóa mục tiêu (no kill-lock). Mọi người chơi đều có thể cùng tham gia săn Boss thế giới mở.
- **Thanh Posture Chung & Quyền Kết Liễu (Execution)**: Đòn đánh bẻ gãy thế đứng (Posture Break) mở ra cửa sổ 3.0s quỳ gối; người chơi kết liễu tung đòn rút **25% Max HP True Damage** với $1.2\text{s}$ I-Frame.
- **Biểu đạt kỹ năng thuần túy**: Né đòn chính xác với $0.28\text{s}$ bất tử tuyệt đối (**I-Frame**), khung thời gian phản xạ vàng $0.05\text{s} - 0.15\text{s}$ kích hoạt **Perfect Dodge** hoàn lại $+15$ thể lực và **Hitstop $0.08\text{s}$**.
- **Luật pháp & Hệ thống Truy nã (Karma & Wanted System)**: Tự do PvP ngoài vùng hoang dã nhưng chịu chế tài nghiêm khắc. Kẻ sát nhân bị gắn cờ Wanted đỏ thẫm, bị lính gác trục xuất và toàn server săn tiền thưởng.

---

## 🏛️ Kiến trúc kỹ thuật (Technical Architecture)

Toàn bộ hệ thống được xây dựng theo chuẩn **100% Server-Authoritative** (ADR-0001), ngăn chặn triệt để gian lận máy khách:

| Hệ thống | Công nghệ / Module | Đặc tả kỹ thuật |
|---|---|---|
| **Thuộc tính & Hiệu ứng** | `GameplayAbilities` (GAS) | `UAscendantAttributeSet` (HP, Stamina, Mana, Posture, IFrameDuration, PostureDecayRate). Replication tối ưu qua Iris Netcode. |
| **Animation 2.5D** | `Paper2D`, `PaperZD` | Cắt sprite điểm ảnh, máy trạng thái Animation State Machine 8 hướng, đồng bộ Notify đòn đánh với Hitbox server. |
| **Mạng & Đồng bộ** | `IrisCore`, `NetCore` | Spatial Ghost Body, Lag Compensation bù trễ chuyển động, Bảng cừu hận phân tán (Contested Threat Table). |
| **Túi đồ & Giao dịch** | `FFastArraySerializer` | Đồng bộ delta inventory theo mạng với độ trễ cực thấp, chống đúp đồ qua Transaction RPCs nghiêm ngặt (ADR-0003). |
| **Giao diện người dùng** | `CommonUI`, `UMG` | Mô hình dữ liệu thuần túy (Pure Data Model) phân tách 100% logic với render Slate, không tick polling gây sụt giảm FPS. |

---

## 🎮 Các tính năng cốt lõi đã hoàn thành

### 1. Vòng lặp Chiến đấu & Đấu Trùm (Encounter Layer — Sprint 5)
- **Boss AI Ironclad Warlord**: Vòng đời 4 pha ra chiêu chuẩn hóa (*Telegraph Decal 0%→100% $\to$ Flash Cue 0.10s $\to$ Active Hitbox $\to$ Recovery Punish Window*). Chấm điểm hành vi EQS theo góc và cự ly. Choáng húc tường $1.8\text{s}$.
- **Posture Stagger & Execution**: Vỡ thế ở 100% Posture, trùm quỳ gối $3.0\text{s}$, kết liễu gây **25% Max HP True Damage**, miễn nhiễm $2.0\text{s}$ chống lặp stagger-lock, suy giảm Posture $20/\text{s}$ sau $4.0\text{s}$.
- **Phá hủy bộ phận (Part Breaking)**: Sừng (20% HP $\to$ cấm Horn Charge, choáng 1.5s), Đuôi (15% HP $\to$ cấm Tail Sweep), Giáp ngực (25% HP $\to$ mở Tử huyệt ngực nhận $+50\%$ sát thương). Rơi nguyên liệu rèn đặc thù.
- **Dash I-Frame & Perfect Dodge**: Lướt $0.45\text{s}$ tiêu tốn 25 thể lực, $0.28\text{s}$ bất tử tuyệt đối, cửa sổ vàng $0.05\text{s}-0.15\text{s}$ hoàn $+15$ thể lực và Hitstop $0.08\text{s}$, chống trượt mép vực.

### 2. Giao diện & Trải nghiệm (Presentation Layer — Sprint 4)
- **Player Vitals HUD**: Đồng bộ thời gian thực HP / Stamina / Mana; thanh máu bóng mờ Catch-up Ghost Bar trễ $0.40\text{s}$; nhịp tim đập dồn dập 60–100 BPM khi máu $<20\%$.
- **Boss Health & Stagger Bar**: Chia 3 phân đoạn máu (75% và 25%), nhấp nháy đỏ 4.0 Hz khi vỡ thế, tâm ngắm tử huyệt Execution Reticle chiếu từ Socket 3D lên màn hình 2D.
- **Floating Combat Text**: Số sát thương nảy động theo quỹ đạo đạn đạo ($V_0=180\text{cm/s}, g=-300\text{cm/s}^2$), Object Pooling 50 instances chống giật lag, chữ nổi "PERFECT!".
- **Interactive Windows**: Cửa sổ Merchant Shop 2 cột (FIFO Buyback 10 ô, phụ phí Karma $+20\%$) và Đe rèn Blacksmith Forge với cơ chế nhấn giữ chuột an toàn $0.8\text{s}$ (Hold-to-Craft) cùng tự động đóng khi rời xa quá $500\text{cm}$ hoặc vào combat.

### 3. Kinh tế & Chế tạo (Expansion Layer — Sprint 3)
- **Dual Currency Economy**: Hệ thống ví Tiền Vàng (Gold) giao dịch tự do và Tàn Trang (Skill Shards) gắn liền nhân vật. Mạng lưới thương nhân tiền trạm và Thương buôn lang thang (Wandering Smuggler).
- **Blacksmithing Forge**: Cường hóa trang bị 3 cấp độ (Tier 1 Outpost $\to$ Tier 2 Dã ngoại $\to$ Tier 3 Vùng cấm), khảm ngọc Gem Socketing, Đá bảo hộ Blacksmith Ward chống tụt cấp, và Rèn vũ khí thần thánh từ Boss Soul.
- **Progression & Mastery**: Cấp độ nhân vật 1–50, Mastery bảng nhánh kỹ năng, ràng buộc Talent Tree.

### 4. Nền tảng cốt lõi (Foundation & Core Layers — Sprint 1 & 2)
- **Isometric Controller**: Điều khiển 8 hướng màn hình, ngắm bắn độc lập tách rời thân dưới (Decoupled Aiming), Camera góc nhìn $-45^\circ / 45^\circ$ với Lookahead $250\text{cm}$ và làm mờ vật cản che khuất (Occlusion Dithering).
- **FastArray Inventory**: Túi đồ lưới ô phân tầng 5 cấp độ hiếm (Common $\to$ Legendary), Paperdoll trang bị đồng bộ thuộc tính GAS tức thì.
- **Raw World Blockout**: 3 phân vùng thế giới mở nối liền (*Verdant Frontier $\to$ Ashen Wilderness $\to$ Forbidden Sanctum*), Sanctuary Campfires $1000\text{cm}$ xóa cừu hận, ranh giới AI Leash $2500\text{cm}$.

---

## 📁 Cấu trúc thư mục (Directory Layout)

```
ProjectAscendant/
├── Source/ProjectAscendant/       # Mã nguồn C++ Engine
│   ├── Public/ & Private/
│   │   ├── AI/                   # Trí tuệ nhân tạo Boss, EQS, Telegraphs
│   │   ├── Combat/               # GAS Abilities, Stagger, Dash, Damage Calcs
│   │   ├── Controller/           # Isometric Controller, Decoupled Aiming, Camera
│   │   ├── Economy/              # Currency Wallet, Merchant Vendor, Smuggler
│   │   ├── Crafting/             # Blacksmith Forge, Socketing, Boss Soul Craft
│   │   ├── Inventory/            # FastArray Grid Inventory, Paperdoll Binding
│   │   ├── Network/              # Iris Netcode, Threat Table, Lag Compensation
│   │   ├── Progression/          # XP Leveling, Talent Trees, Class Mastery
│   │   ├── UI/                   # CommonUI Widgets & Models (Vitals, Boss, Shop)
│   │   └── Character/            # Base Characters, PaperZD AnimBP State Machines
├── Content/                      # Tài nguyên Unreal Engine (.uasset, .umap, flipbooks)
├── Config/                       # Thiết lập cấu hình dự án (.ini)
├── design/gdd/                   # Toàn bộ Game Design Documents (18 hệ thống chi tiết)
├── production/                   # Quản lý tiến độ sản xuất
│   ├── epics/                    # Danh sách Epic theo tầng kiến trúc
│   └── sprints/                  # Tài liệu chi tiết các Sprint 1 đến 5
└── docs/                         # Kiến trúc phần mềm & Architecture Decision Records (ADRs)
```

---

## 🧪 Kiểm thử tự động (Automation Tests)

Dự án áp dụng triệt để mô hình **Pure Data Model** cho mọi hệ thống logic, cho phép chạy kiểm thử đơn vị & tích hợp không cần nạp giao diện đồ họa (Headless Automation Tests):

```bash
# Chạy toàn bộ bộ kiểm thử tự động của Project Ascendant qua UnrealEditor-Cmd
/mnt/Data/Engine/Binaries/Linux/UnrealEditor-Cmd \
    ProjectAscendant.uproject \
    -ExecCmds="Automation RunTests ProjectAscendant" \
    -nullrhi -nosound -unattended -stdout
```

Các bộ test chính:
- `ProjectAscendant.AI.BossAITelegraphs` (4-phase attack lifecycle, EQS, Wall Stun)
- `ProjectAscendant.Combat.StaggerExecution` (100% posture break, 25% True Damage, decay)
- `ProjectAscendant.Combat.PartBreakingMatrix` (Part HP tracking, dual damage, skill locks)
- `ProjectAscendant.Combat.DashIFramePerfectDodge` (0.28s I-frame, sweet-spot, hitstop)
- `ProjectAscendant.UI.PlayerVitals` (Ghost bar, heartbeat BPM, golden flash)
- `ProjectAscendant.UI.BossHealthHUD` (Phase notches, 4.0 Hz blink, reticle)
- `ProjectAscendant.UI.FloatingCombatText` (Ballistic arc physics, pool recycling)
- `ProjectAscendant.UI.ShopForgeUI` (2-column shop, FIFO buyback, hold-to-craft)

---

## 📜 Bản quyền & Giấy phép

Dự án phát triển nội bộ cho **Project Ascendant**.  
Bản quyền thuộc về © Project Ascendant Team. Mọi quyền được bảo lưu.
