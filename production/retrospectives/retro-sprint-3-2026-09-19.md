# Sprint 3 Retrospective: Expansion Layer (Economy, Crafting & Progression)

> **Sprint**: Sprint 3 (Expansion Layer — Economy, Blacksmithing Forge & Skill Progression)  
> **Period**: 2026-09-18 to 2026-09-19  
> **Generated**: 2026-09-19  
> **Stage**: Production  
> **Review Mode**: Lean  

---

## 1. Metrics & Velocity

| Metric | Planned | Actual | Delta |
|---|---|---|---|
| **Tasks / Stories** | 8 | 8 | 0 (100% Complete) |
| **Completion Rate** | 100% | 100% | 0% |
| **Effort (Days)** | 8.0 days | 2.0 days | -6.0 days (High Velocity) |
| **Must-Have Stories** | 5 | 5 | 0 |
| **Should-Have Stories** | 2 | 2 | 0 |
| **Nice-to-Have Stories** | 1 | 1 | 0 |
| **Bugs / Issues Found** | — | 0 blocking | 0 |
| **Automated Tests Pass Rate** | 100% | 100% | 0 failures |

### Velocity Trend

| Sprint | Layer | Planned Stories | Completed Stories | Rate |
|---|---|---|---|---|
| **Sprint 1** | Foundation Layer (Attributes, Controller, Inventory, Netcode) | 13 | 13 | 100% |
| **Sprint 2** | Core Layer (World, Character, Combat, Items, Boss) | 8 | 8 | 100% |
| **Sprint 3** | Expansion Layer (Economy, Blacksmithing, Progression) | 8 | 8 | 100% |

**Trend**: **Exceptional & Consistent**.  
Quy chuẩn kiến trúc ranh giới module rõ rệt theo ADR-0001 (Server Authority), ADR-0002 (GAS & PaperZD), và ADR-0003 (FastArray Grid Inventory) giúp đội ngũ triển khai các hệ thống kinh tế và thợ rèn 3 tầng mà không gặp xung đột phụ thuộc.

---

## 2. What Went Well

1. **Hệ Thống Song Tiền Tệ & Mạng Lưới Giao Thương Dã Ngoại (`econ-001`, `econ-002`, `econ-003`)**:
   - `UPACurrencyComponent` sử dụng `FFastArraySerializer` tối ưu hóa delta replication cho Vàng và Tàn Trang qua mạng Iris.
   - Giao thức giao dịch nguyên tử (Atomic Server Transactions) đảm bảo khấu trừ tiền và cấp vật phẩm trong cùng 1 server tick, triệt tiêu nguy cơ duping.
   - Thương nhân 3 Bậc (Outpost Provisioner, Wandering Smuggler, Sanctum Black Market Broker) vận hành hoàn chỉnh với cửa sổ Mua Lại (Buyback Window) FIFO 10 ô và phụ phí Wanted Karma (+20%).
   - Timer restock đồng bộ toàn server (`UPARestockComponent`) giúp toàn bộ thương nhân cùng bậc làm mới hàng tồn kho giới hạn theo chu kỳ 3600s.

2. **Hệ Thống Thợ Rèn Phân Vùng 3 Cấp & Cường Hóa Không Phá Hủy Đồ (`crft-001`, `crft-002`, `crft-003`)**:
   - Hiện thực hóa trọn vẹn triết lý "Trang bị không bao giờ bị phá hủy khi rèn thất bại":
     - Mốc $+1 \rightarrow +3$: Thành công 100% an toàn.
     - Mốc $+4 \rightarrow +6$: Thất bại giữ nguyên cấp.
     - Mốc $+7 \rightarrow +10$: Thất bại tụt 1 cấp nếu không có Đá Bảo Hộ (`item_blacksmith_ward`).
   - Khai mở Đục Lỗ Khảm Ngọc (tối đa 2 lỗ tại Tier 2, 3 lỗ Prismatic tại Tier 3) và tháo ngọc an toàn bảo toàn ngọc với phí 100 Vàng.
   - Nghi thức Đúc Thần Binh Bậc 5 (Divine Equipment) từ Linh Hồn Lãnh Chúa, Mảnh vỡ Boss và Quặng Hư Không độc quyền tại Lò Rèn Cấm Địa Tier 3.
   - Chuỗi nâng cấp sức chứa kho đồ tuần tự ($30 \rightarrow 40 \rightarrow 50 \rightarrow 60$ ô).

3. **Tiến Trình Cấp Độ & Cây Kỹ Năng Tài Năng (`prog-001`, `prog-002`)**:
   - Đường cong kinh nghiệm Exponential-Polynomial chuẩn hóa cấp 1 đến 50 với điểm tài nguyên Attribute Points và Talent Points.
   - Hệ thống Skill Tree hỗ trợ 3 nhánh chuyên biệt (Vanguard, Ranger, Mage), kiểm tra điều kiện tiên quyết (Prerequisites), cấp độ yêu cầu, và tự động liên kết / gán `UGameplayAbility` qua GAS Ability Spec Handle.

4. **Kiểm Thử Tự Động TDD Toàn Diện (100% Automated Test Coverage)**:
   - Toàn bộ 8 stories đều đi kèm bộ kiểm thử tự động Unreal Automation Tests chuyên biệt, kiểm thử từ các trường hợp biên (edge cases), giới hạn số dư, xác thực khoảng cách $\le 300\text{cm}$, trạng thái giao chiến (`InCombat`), và ghi đè RNG xúc xắc (`TestRollOverride`) để đảm bảo tính tất định (deterministic).

---

## 3. What Went Poorly & Root Causes

1. **Cân Nhắc Ghi Đè Giá Tạm Thời Khi Mua Hàng Có Phụ Phí Karma**:
   - *Hiện tượng*: Khi áp dụng phụ phí 20% cho người chơi Wanted tại Tier 2 Smuggler, việc chỉnh sửa trực tiếp giá trong catalog nếu không hoàn trả sẽ làm hỏng dữ liệu gốc của NPC.
   - *Nguyên nhân*: Catalog entry dùng chung cho mọi người chơi tiếp cận NPC.
   - *Khắc phục*: Trong `BuyItemWithKarmaCheck`, giá được tính toán và patch tạm thời trong quá trình giao dịch nguyên tử, sau đó lập tức khôi phục về giá gốc dù giao dịch thành công hay thất bại.
2. **Đồng Bộ Thứ Tự Nâng Cấp Túi Đồ**:
   - *Hiện tượng*: Cần tránh trường hợp người chơi nhảy cóc từ 30 lên 50 ô hoặc mang nguyên liệu Bậc 3 đến Lò rèn Tier 1.
   - *Khắc phục*: Tích hợp hàm `GetBackpackExpansionRequirement` kiểm tra chặt chẽ `CurrentSlots` và `RequiredForgeTier`, trả mã lỗi `MaxTierLevelReached` hoặc `MaxBackpackCapacity` rõ ràng.

---

## 4. Key Takeaways & Action Items

- **Action Item 1**: Sẵn sàng cho Tầng Trải Nghiệm Người Dùng (UI / CommonUI & HUD) ở Sprint kế tiếp: Tích hợp các Component đã xây dựng (`UPAMerchantComponent`, `UPABlacksmithComponent`, `UPAProgressionComponent`) vào các Widget giao diện tương tác.
- **Action Item 2**: Chuẩn bị asset hình ảnh pixel art và âm thanh / hiệu ứng VFX cho các hành động Rèn đúc, Tôi luyện $+10$ hào quang vũ khí và Nghi thức Đúc Thần Binh.
