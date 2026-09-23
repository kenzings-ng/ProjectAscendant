# Sprint 3 — 2026-09-18 to 2026-10-02

> **Sprint**: Sprint 3 (Expansion Layer — Economy, Crafting & Character Progression)  
> **Stage**: Production (First Production Sprint)  
> **Review Mode**: Lean  
> **Capacity**: 10 days (80 hours) | Buffer (20%): 2 days (16 hours) | Available: 8 days (64 hours)  

---

## Sprint Goal

Khởi động Tầng Mở Rộng (Expansion Layer) trong Giai đoạn Production cho *Project Ascendant*:  
Triển khai hệ thống Song Tiền Tệ (**Vàng & Tàn Trang - Gold & Ash Shards**) với các giao dịch Server RPC nguyên tử, Hệ thống Thợ Rèn Phân Vùng 3 Cấp (**Tier 1-3 Blacksmithing Forge**, sửa chữa độ bền, phân tách phế liệu, cường hóa +1 đến +10 an toàn), Mạng Lưới Thương Nhân Dã Ngoại (**Vendor Network & Buyback Window**), và Động cơ Thăng Tiến Kỹ Năng (**Skill Progression & Class Mastery Tree**).

---

## Tasks

### Must Have (Critical Path — 40 hours / 5.0 days)

| ID | Task | Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `econ-001` | [Dual Currency Wallet & Transaction Engine](file:///mnt/Data/Projects/project-games/production/epics/expansion-economy/story-001-currency-wallet-engine.md) | `gameplay-programmer` | 1.0 (8h) | `inv-002` | AC-1: `UPACurrencyComponent` quản lý Gold và Ash Shards với đồng bộ hóa FastArray delta replication; AC-2: Atomic Server Transaction RPCs (Add, Deduct, Transfer) chống duping; AC-3: Cơ chế tử trận PvE rơi 50% Gold vào Vệt Tro Tàn. |
| `crft-001` | [Blacksmith Tier 1 Outpost Forge & Item Repair](file:///mnt/Data/Projects/project-games/production/epics/expansion-crafting/story-001-tier1-outpost-forge.md) | `gameplay-programmer` | 1.0 (8h) | `item-001`, `econ-001` | AC-1: `UPABlacksmithComponent` tại Outpost Forge hỗ trợ sửa chữa độ bền trang bị tiêu hao Gold; AC-2: Phân tách trang bị thừa và sách kỹ năng ra Tàn Trang (Ash Shards); AC-3: Cường hóa cơ bản +1 đến +3 tỷ lệ thành công 100% tiêu hao Gold + Quặng Sắt. |
| `econ-002` | [Merchant Vendor Network & Buyback Window](file:///mnt/Data/Projects/project-games/production/epics/expansion-economy/story-002-merchant-vendor-network.md) | `gameplay-programmer` | 1.0 (8h) | `econ-001`, `inv-001` | AC-1: NPC Merchant tương tác trong phạm vi $\le 300$cm khi thoát giao tranh; AC-2: Mua bán vật phẩm với chiết khấu giá bán `vendor_sell_penalty = 0.30`; AC-3: Cửa sổ mua lại (Buyback) lưu trữ 10 vật phẩm vừa bán gần nhất; AC-4: Từ chối giao dịch với người chơi Red Name (Wanted). |
| `prog-001` | [Character XP, Leveling & Class Mastery](file:///mnt/Data/Projects/project-games/production/epics/expansion-progression/story-001-character-leveling-mastery.md) | `ue-gas-specialist` | 1.0 (8h) | `attr-001` | AC-1: `UPAProgressionComponent` quản lý XP đường cong phi tuyến Cấp 1 $\rightarrow$ 50; AC-2: Mỗi cấp độ tăng trưởng chỉ số cơ sở và ban thưởng 1 Điểm Kỹ Năng (Skill Point); AC-3: Replicate qua Iris mạng và cập nhật AttributeSet. |
| `crft-002` | [Enhancement Pipeline +4 to +10 & Socketing](file:///mnt/Data/Projects/project-games/production/epics/expansion-crafting/story-002-enhancement-socketing.md) | `gameplay-programmer` | 1.0 (8h) | `crft-001` | AC-1: Cường hóa +4 đến +6 (tỷ lệ 70%-50%, thất bại giữ nguyên cấp); AC-2: Cường hóa +7 đến +10 (tỷ lệ giảm dần, thất bại tụt 1 cấp, TUYỆT ĐỐI không vỡ đồ); AC-3: Đục tối đa 2 Lỗ Khảm Ngọc (Gem Sockets). |

### Should Have (16 hours / 2.0 days)

| ID | Task | Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `prog-002` | [Skill Tree & Talent Node Binding](file:///mnt/Data/Projects/project-games/production/epics/expansion-progression/story-002-talent-tree-binding.md) | `ue-gas-specialist` | 1.0 (8h) | `prog-001`, `cmbt-001` | AC-1: Cấu trúc cây kỹ năng 3 nhánh chuyên môn hóa cho Vanguard/Ranger/Arcanist; AC-2: Mở khóa các node nội tại (Passive Nodes) tăng sát thương, giảm hồi chiêu Dash, và tăng thanh thế đứng Posture. |
| `econ-003` | [Wilderness Wandering Smuggler & Limited Stock](file:///mnt/Data/Projects/project-games/production/epics/expansion-economy/story-003-wandering-smuggler.md) | `gameplay-programmer` | 1.0 (8h) | `econ-002` | AC-1: Thương nhân lang thang tại Lửa Trại Ashen Wilderness; AC-2: Danh mục hàng tồn có hạn (Limited Stock) như Đá Bảo Hộ Ép Đồ làm mới theo Timer 60 phút. |

### Nice to Have (8 hours / 1.0 days)

| ID | Task | Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `crft-003` | [Boss Soul Forging & Divine Equipment](file:///mnt/Data/Projects/project-games/production/epics/expansion-crafting/story-003-boss-soul-forging.md) | `gameplay-programmer` | 1.0 (8h) | `crft-002`, `pzd-003` | AC-1: Đúc Thần Binh Immortal/Divine từ Linh Hồn Lãnh Chúa (Stone Golem Core) tại Lò Rèn Cấm Địa Tier 3; AC-2: Cấp kỹ năng nội tại biến đổi cơ chế đòn đánh. |

---

## Total Planned Scope
- **Must Have**: 5 stories (40 hours / 5.0 days)
- **Should Have**: 2 stories (16 hours / 2.0 days)
- **Nice to Have**: 1 story (8 hours / 1.0 days)
- **Total**: 8 stories (64 hours / 8.0 days) vs **Available Capacity**: 8.0 days (64 hours)

---

## Definition of Done for this Sprint
- [x] 100% Must Have tasks hoàn thành và vượt qua kiểm thử đơn vị & tích hợp
- [x] Mọi giao dịch kinh tế, rèn đúc và thăng cấp được thẩm định nghiêm ngặt trên Server Authority
- [x] 0 lỗi duplicate vật phẩm hoặc mất tiền tệ khi rớt mạng
- [x] Bộ kiểm thử tự động Unreal Automation Tests đạt 100% Pass
