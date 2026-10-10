# UX Spec: Cửa Sổ Thương Nhân (Merchant Shop — `UPAMerchantShopWidget`)

> **Status**: In Design — MOCKUP CHỜ CHỦ DỰ ÁN DUYỆT (Z1)
> **Author**: ux-designer (Z1, chế độ tự vận hành)
> **Last Updated**: 2026-10-10
> **Journey Phase(s)**: Vòng lặp chính — nghỉ tại Sanctuary / Lửa Trại giữa các chuyến dã ngoại
> **Platform Target**: PC (bàn phím + chuột, chính) · Gamepad (Full, CommonUI)
> **Template**: UX Spec
> **Mockup**: `design/ux/mockups/ascendant-ui-mockups.html` — màn 5 (Thương Nhân Lang Thang, tab Mua)
> **Nguồn yêu cầu**: `design/gdd/merchant-economy.md` §1–§5, Edge Cases E1–E10, Visual Requirements, UI Requirements · `design/gdd/inventory-system.md` §1 (bảng màu 5 độ hiếm), §2 (Tàn Trang là tiền tệ thứ hai), dòng 103 (Bán Tất Cả Rác), AC-4 · `production/DECISIONS.md` §5 (hai thang độ hiếm), §12 · `production/epics/presentation-ui/story-004-shop-forge-windows.md`

---

## Purpose & Player Need

Người chơi vừa về từ dã ngoại muốn: mua nhu yếu phẩm, bán chiến lợi phẩm, bán nhanh mọi đồ đã đánh dấu rác, chuộc lại món bán nhầm. Màn phải cho thấy rõ giá (Vàng hoặc Tàn Trang), số dư hai loại tiền, chỗ trống ba lô và hàng tồn có hạn.

## Player Context on Arrival

Nhẹ nhõm ("ốc đảo văn minh giữa hoang dã", merchant-economy Player Fantasy) nhưng có thể bị áp lực: hàng Limited Stock, người chơi khác tới gần, quái có thể tấn công (Shop tự đóng). Đến chủ động bằng phím tương tác.

## Navigation Position

`Thế giới` → (tương tác NPC Thương nhân, ≤ 300 cm) → **Cửa sổ Shop** (modal `UCommonActivatableWidget` trên HUD). Mỗi NPC một cửa sổ riêng (Buyback gắn theo NPC, E7).

## Entry & Exit Points

| Entry Source | Trigger | Player carries this context |
|---|---|---|
| Thế giới | Phím tương tác (`[E]` bàn phím / `[X]` gamepad, interaction-patterns §3.5) trong 300 cm, ngoài combat | Karma, ví (Gold + Tàn Trang), túi đồ |
| Thế giới (Wanted, NPC Tier 1) | Như trên khi Karma < -50 | Không mở Shop; NPC nói thoại từ chối (merchant-economy §2 Tier 1, E3) |

| Exit Destination | Trigger | Notes |
|---|---|---|
| Thế giới | `Esc` / `B` / nút Đóng | Không hủy giao dịch đã được server xác nhận |
| Thế giới (tự đóng) | Rời > 500 cm hoặc vào combat (E4, E10) | Yêu cầu đang chờ bị server hủy (rollback); hiện toast "Giao dịch bị hủy: bạn đã vào giao tranh" |

---

## Layout Specification

### ASCII Wireframe

```
┌──────────────────────────────────────────────────────────────────────────┐
│ THƯƠNG NHÂN LANG THANG  [Tier 2]           Hàng mới trong: 47:12  [Đóng]│
│ [ MUA HÀNG ]  Bán   Mua lại (3/10)                                       │
├─────────────────────────────────┬────────────────────────────────────────┤
│ DANH MỤC NPC                    │ BA LÔ CỦA BẠN                 28/40 ô   │
│ Cố định                         │ ┌──┬──┬──┬──┬──┬──┐                    │
│ [▣] Bình Máu Lớn     150 ◉ Vàng │ │▣ │▣🔒│▣R│  │  │  │ ← viền theo độ hiếm│
│ [▣] Bình Mana Lớn    150 ◉      │ ├──┼──┼──┼──┼──┼──┤   🔒 khóa, R rác   │
│ [▣] Đá Bảo Hộ Ép Đồ  800 ◉      │ │  │  │  │  │  │  │                    │
│ Xoay vòng ▬▬▬▬▬▬░░ 47:12        │ └──┴──┴──┴──┴──┴──┘                    │
│ [▣] Bản Đồ Mật Cảnh   12 ◈ Tàn  │ Tooltip: tên, mô tả, giá bán 30%       │
│     Trang  · còn 1              │                                        │
│ [▢] Quyển Trục (Rare) HẾT HÀNG  │ [ BÁN TẤT CẢ RÁC (5 món · 145 ◉) ]     │
├─────────────────────────────────┴────────────────────────────────────────┤
│ ◉ 1,240 Vàng   ◈ 37 Tàn Trang   Ba lô trống: 12        [Enter] MUA      │
└──────────────────────────────────────────────────────────────────────────┘
```

### Layout Zones

| Zone | Nội dung |
|---|---|
| Z1 Header | Tên NPC + huy hiệu Tier + đồng hồ restock (UI Req "Header") |
| Z2 Tab | `[Mua hàng]` `[Bán]` `[Mua lại]` `[Sửa chữa]` `[Chuộc tội]` (Chuộc tội chỉ Tier 3) — UI Req |
| Z3 Trái | Danh mục NPC (Mua) / danh sách Buyback (Mua lại) |
| Z4 Phải | Ba lô người chơi (lưới 6 cột, `inventory-system.md` §3.1) |
| Z5 Footer | Vàng · Tàn Trang · ô trống ba lô (UI Req "Footer") + nút hành động chính |

### Component Inventory

| Component | Loại | Nội dung | Tương tác | Pattern |
|---|---|---|---|---|
| Dòng hàng NPC | List row | Icon, tên, giá + icon tiền (Gold `◉` / Tàn Trang `◈`), tồn kho nếu Limited, viền độ hiếm | Có | Inventory Slot (dạng hàng) |
| Thanh restock | Progress | "Hàng mới trong: MM:SS" | Không | Mới: Countdown Bar |
| Ô ba lô | Grid cell | Icon, số lượng, viền độ hiếm, icon khóa 🔒, cờ rác | Có | Inventory Slot (§3.2) |
| Tooltip | Popup | Tên, mô tả, stats, giá mua/bán, so sánh đồ đang mặc | Không | Tooltip & Item Compare (§3.7) |
| Nút Bán Tất Cả Rác | Button | Số món + tổng Vàng dự kiến | Có | Button (Secondary) |
| Nút hành động chính | Button | Mua / Bán / Mua lại theo tab | Có | Button (Primary) |
| Modal xác nhận | Dialog | Món giá ≥ 50% số dư hoặc bán đồ Epic/Legendary (đề xuất) | Có | Modal Dialog |
| Toast lỗi | Popup trên nút | Lỗi đỏ / cảnh báo vàng, tự tắt 3s (UI Req "Thông Báo Lỗi") | Không | — |

**Viền độ hiếm** (quyết định chủ dự án `a-rarityframe=palette` → dùng `inventory-system.md` §1): Common `#D1D5DB`, Uncommon `#10B981`, Rare `#3B82F6`, Epic `#8B5CF6`, Legendary `#F59E0B`. Mỗi độ hiếm kèm hình khung khác nhau để không chỉ dựa vào màu (`accessibility-requirements.md` §2.2: Common tròn, Rare thoi, Epic lục giác, Legendary vương miện tia; Uncommon chưa có hình — xem Q3). Sách/Quyển trục dùng thang kỹ năng riêng (Normal/Rare/Epic/Mythic, DECISIONS §5).

### Information Hierarchy

1. Giá + loại tiền của món đang chọn, và có đủ tiền không. 2. Số dư Vàng + Tàn Trang. 3. Hàng tồn / Hết hàng. 4. Chỗ trống ba lô. 5. Đồng hồ restock. 6. Tooltip chi tiết.

---

## States & Variants

| State / Variant | Trigger | What Changes |
|---|---|---|
| Default (Mua) | Mở Shop | Tab Mua; chọn sẵn món đầu; focus danh mục |
| Không đủ tiền | `FPAShopItemEntry::bCanAfford == false` | Giá đổi Crimson + icon ✕; nút Mua khóa với nhãn "Thiếu 160 Vàng" |
| Phụ phí Wanted | `IsKarmaSurchargeActive()` (Tier 2, Karma < -50) | Huy hiệu "+20% phí rủi ro" ở header; giá gốc gạch ngang, giá `FinalPrice` bên cạnh |
| Hết hàng | Stock = 0 | Dòng xám, chữ đỏ "Hết hàng" + đồng hồ restock (UI Req) |
| Restock khi đang mở | Restock server | Flash vàng trên các ô hàng mới |
| Đang chờ server (loading) | `IsRequestPending()` | Nút chính → spinner "Đang giao dịch…"; chặn yêu cầu thứ hai (một yêu cầu một lúc, X11b) |
| Thành công | `OnTransactionCompleted` | Flash vàng trên icon + số Vàng/Tàn Trang bay lên mờ dần |
| Bị từ chối | `OnTransactionRejected(EPATransactionError)` | Toast đỏ trên nút + flash đỏ nút (rung màn hình micro chỉ khi Screen Shake > 0) |
| Tab Bán | `SwitchTab(Sell)` | Trái hiện giá thu mua `floor(base × 0.30)` (Tier 3: 0.25) của món đang chọn |
| Đồ khóa | `bIsLocked` | Ô có icon 🔒; không chọn bán được; tooltip "Đã khóa" (inventory AC-4) |
| Không có rác | Không món nào `bIsJunk` | Nút Bán Tất Cả Rác khóa, nhãn "Chưa có đồ đánh dấu rác ([J] để đánh dấu)" |
| Buyback trống (empty) | `GetBuybackCount() == 0` | "Chưa bán món nào cho thương nhân này trong phiên." |
| Buyback sắp đầy | 9/10 | Cảnh báo vàng "Buyback sắp đầy!" (UI Req) |
| Ba lô đầy | `InventoryFull` | Nút Mua khóa với nhãn "Ba lô đã đầy" |
| Từ chối Wanted (Tier 1) | Karma < -50 | Không mở Shop; bong bóng thoại NPC |

Thông báo lỗi theo `EPATransactionError` (chuỗi theo merchant-economy Edge Cases):

| Mã | Thông báo |
|---|---|
| `InsufficientGold` | "Không đủ Vàng." (Buyback: "Không đủ Vàng để mua lại.") |
| `InventoryFull` | "Ba lô đã đầy! Hãy bán hoặc bỏ bớt vật phẩm." |
| `OutOfStock` | "Hết hàng!" |
| `ItemLocked` | "Vật phẩm đã khóa, không thể bán." |
| `DistanceExceeded` | "Bạn đã đi quá xa thương nhân." |
| `InCombat` | "Không thể giao dịch khi đang giao tranh." |
| `BuybackEmpty` / `ItemNotFound` | "Vật phẩm không còn." |
| `ServerRejected` | "Giao dịch bị từ chối. Thử lại sau giây lát." (gồm rate-limit 2 giao dịch/giây, E6) |

## Interaction Map

| Component | Bàn phím / Chuột | Gamepad | Phản hồi | Kết quả |
|---|---|---|---|---|
| Đổi tab | Click / `Q`,`E` | `LB/RB` | Lật tab, `SFX_UI_Focus` | `SwitchTab(EPAShopTab)` |
| Chọn hàng NPC | Click / `↑↓` | D-pad | Viền sáng +30%, tooltip 0.1s | `SelectCatalogItem(Index)` |
| Chọn ô ba lô | Click / mũi tên | D-pad (chuyển cột bằng `←/→` ở mép) | Viền sáng | `SelectInventoryItem(SlotIndex)` |
| Mua | `Enter` / double-click / nút | `A` | Tiếng đồng xu (Gold) / thủy tinh (Tàn Trang) | `ExecuteBuy()` |
| Bán | `Enter` / nút; chuột phải trên ô = bán nhanh (đề xuất) | `A`; `Y` = bán nhanh | Tiếng túi vải "thịch" | `ExecuteSell()` |
| Mua lại | `Enter` / nút | `A` | Tiếng xu kéo ngược | `ExecuteBuyback()` (theo `ItemInstanceUID`) |
| Bán Tất Cả Rác | `Ctrl+J` (đề xuất) / nút | `X` | Modal xác nhận liệt kê số món + tổng Vàng | **Gap:** widget chưa có `ExecuteSellAllJunk()`; router đã có `Server_RequestMerchantSellAllJunk` |
| Đóng | `Esc` / nút | `B` | Trượt ra | Đóng, trả focus về game |

## Events Fired

| Player Action | Event Fired | Payload / Data |
|---|---|---|
| Mua | `UPAServiceRequestComponent::Server_RequestMerchantBuy` | RequestId, MerchantActor, CatalogIndex, Quantity |
| Bán | `Server_RequestMerchantSell` | SlotIndex, Quantity |
| Bán tất cả rác | `Server_RequestMerchantSellAllJunk` | RequestId, MerchantActor |
| Mua lại | `Server_RequestMerchantBuyback` | ItemInstanceUID |
| Xác nhận | `Client_ConfirmMerchantRequest` → `OnTransactionCompleted` / `OnTransactionRejected` | bSuccess, `EPATransactionError` |
| Số dư đổi | `UPACurrencyComponent` → `HandleCurrencyBalanceChanged(EPACurrencyType, NewBalance, Delta)` | Gold / SkillShards |

Mọi hành động đổi tiền/túi đồ là **trạng thái bền vững do server quyết định** (ADR-0001, ADR-0003). UI không trừ tiền trước khi server xác nhận.

## Transitions & Animations

- Mở: trượt lên + fade 0.2s, tiếng lục lạc (Tier 1) / hòm gỗ (Tier 2) / chuông gió (Tier 3).
- Đóng: fade 0.15s.
- Thành công: flash vàng 0.3s + số tiền bay lên; Reduced Motion: chỉ đổi số, không bay.

## Data Requirements

| Data | Source System | Read / Write | Notes |
|---|---|---|---|
| Tab hiện tại | `FPAShopUIModel::CurrentTab` (`EPAShopTab`: Buy/Sell/Buyback) | R/W | **Gap:** enum thiếu `Repair` và `Bailout` mà GDD UI Req yêu cầu |
| Danh mục NPC | `FPAShopUIModel::CatalogItems` (`FPAShopItemEntry`: ItemId, DisplayName, Quantity, PriceGold, FinalPrice, bHasSurcharge, bCanAfford) | Read | **Gap:** không có loại tiền của giá (Tàn Trang), độ hiếm, tồn kho Limited, cố định/xoay vòng |
| Ba lô | `InventoryItems` / `UPAInventoryComponent` | Read | **Gap:** entry thiếu rarity, `bIsLocked`, `bIsJunk` |
| Buyback | `BuybackItems` (tối đa 10, FIFO) | Read | Đúng GDD §4 |
| Vàng | `FPAShopUIModel::PlayerGold` (int32), `GetPlayerGold()` | Read | Ví là int64; giới hạn 9,999,999 vừa int32 |
| Tàn Trang | `UPACurrencyComponent` (`EPACurrencyType::SkillShards`) | Read | **Gap:** model không có `PlayerSkillShards` |
| Ô trống ba lô | `UPAInventoryComponent` | Read | **Gap:** model không có |
| Đồng hồ restock | `UPARestockComponent` (Smuggler, `PAWanderingSmuggler.h`) | Read | **Gap:** model không có thời gian restock |
| Tên NPC + Tier | Actor NPC | Read | **Gap:** model không có |
| Phụ phí Wanted | `bKarmaSurchargeActive` | Read | Karma do client truyền vào `InitializeShop`, không replicate (story-004 B4-13) |
| Đang chờ | `IsRequestPending()` | Read | — |

## Input Method Completeness Checklist

**Bàn phím:** [x] đổi tab · [x] duyệt danh mục và lưới · [x] Enter thực hiện · [x] Esc đóng · [ ] phím tắt Bán Tất Cả Rác (đề xuất `Ctrl+J`)
**Chuột:** [x] click/double-click · [x] hover tooltip · [ ] chuột phải bán nhanh (đề xuất)
**Gamepad:** [x] `LB/RB` tab · [x] D-pad · [x] `A` thực hiện · [x] `B` đóng · [x] `X` bán rác · [x] tooltip neo cố định cạnh lưới (pattern §3.7)
Focus trap: Shop là modal; focus không lọt ra HUD.

## Accessibility

Tier Standard. Giá luôn có icon loại tiền + chữ "Vàng"/"Tàn Trang" (không chỉ màu). Độ hiếm = màu + hình khung. Không đủ tiền = màu + icon ✕ + nhãn "Thiếu N Vàng". Chữ ≥ 24px; số dùng chữ số đều (tabular). Không giới hạn thời gian trừ restock (chỉ thông tin).

## Localization Considerations

- Số tiền định dạng theo locale (`1,240` / `1.240`); giới hạn 7 chữ số Vàng, 5 chữ số Tàn Trang.
- Tên vật phẩm dài ("Đá Bảo Hộ Ép Đồ") phải xuống dòng trong dòng hàng, không cắt (HIGH PRIORITY).
- Nhãn tab phải vừa 5 tab ở 1280px.

## Acceptance Criteria

- [ ] Shop mở trong ≤ 0.3s sau khi bấm tương tác trong 300 cm, ngoài combat.
- [ ] Mỗi dòng hàng hiện icon, tên, giá kèm icon + chữ loại tiền, tồn kho nếu Limited, viền màu + hình theo độ hiếm (`inventory-system.md` §1).
- [ ] Footer luôn hiện số dư Vàng, Tàn Trang và số ô ba lô trống, cập nhật khi ví replicate.
- [ ] Mua khi thiếu tiền: nút khóa với nhãn "Thiếu N Vàng"; ép gửi vẫn bị server từ chối, toast đỏ 3s, không đổi số dư.
- [ ] Bán Tất Cả Rác chỉ bán món `bIsJunk`, không bán món `bIsLocked` (inventory AC-4), có modal xác nhận.
- [ ] Bán 11 món → tab Mua lại hiện đúng 10 (món 2–11), giá mua lại bằng đúng Vàng đã nhận.
- [ ] Vào combat hoặc rời > 500 cm khi Shop mở → Shop đóng, yêu cầu đang chờ báo hủy.
- [ ] Trong lúc chờ server, không gửi được yêu cầu thứ hai.
- [ ] Toàn cửa sổ dùng được bằng bàn phím và gamepad, focus bị giữ trong modal.

## Open Questions

- **Q1:** Tab `[Sửa chữa]` có trong UI Req của merchant-economy nhưng merchant-economy Formulas 4 nói công thức thuộc Thợ Rèn; widget/router hiện chỉ có sửa ở Forge. Shop có tab Sửa chữa thật không?
- **Q2:** `[Chuộc tội]` (Karma Bailout, Tier 3) chưa có trong model/router. Ngoài phạm vi playable slice (Z3 chỉ đặt Smuggler Tier 2) — để sau.
- **Q3:** Uncommon chưa có hình khung trong `accessibility-requirements.md` §2.2 (chỉ 4 hình). Mockup tạm dùng hình vuông vát.
- **Q4:** Mythic (thang kỹ năng) là "Đỏ ánh kim" nhưng chưa có mã màu ở `inventory-system.md` §1. Cần mã màu cho viền sách/quyển trục.
- **Q5:** Ngưỡng hiện modal xác nhận khi mua đắt / bán đồ quý là đề xuất, GDD không quy định.
- **Q6:** `UPAMerchantShopWidget` là `UUserWidget`, phải chuyển `UCommonActivatableWidget` (control-manifest §4).
- **Q7:** Merchant-economy UI Req dùng `[Mua hàng] | [Bán] | [Mua lại]`; code dùng `Buy/Sell/Buyback` — chỉ là nhãn hiển thị, giữ tiếng Việt theo GDD.
