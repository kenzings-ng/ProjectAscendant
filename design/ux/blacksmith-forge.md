# UX Spec: Cửa Sổ Lò Rèn (Blacksmith Forge — `UPABlacksmithForgeWidget`)

> **Status**: In Design — MOCKUP CHỜ CHỦ DỰ ÁN DUYỆT (Z1)
> **Author**: ux-designer (Z1, chế độ tự vận hành)
> **Last Updated**: 2026-10-10
> **Journey Phase(s)**: Vòng lặp chính — nâng cấp trang bị giữa các chuyến dã ngoại
> **Platform Target**: PC (bàn phím + chuột, chính) · Gamepad (Full, CommonUI)
> **Template**: UX Spec
> **Mockup**: `design/ux/mockups/ascendant-ui-mockups.html` — màn 6 (Thợ Rèn Tiền Trạm, tab Cường Hóa)
> **Nguồn yêu cầu**: `design/gdd/blacksmithing-system.md` §1 (ma trận 3 lò), §2 A–D, Formulas 1–5, Edge Cases 1–6, UI Requirements, AC-1…AC-7 · `design/gdd/inventory-system.md` §1 (bảng màu độ hiếm), §3.1 · `design/gdd/merchant-economy.md` (`gem_unsocket_fee`, `repair_cost_formula`) · `production/epics/presentation-ui/story-004-shop-forge-windows.md`

---

## Purpose & Player Need

Người chơi muốn làm mạnh trang bị: sửa độ bền, cường hóa +1…+10, đục lỗ/khảm ngọc, đúc Thần Binh từ Linh Hồn Boss, phân rã đồ/sách thừa, mở rộng túi đồ. Màn phải cho thấy trước khi bấm: chi phí, nguyên liệu đủ/thiếu, **tỉ lệ thành công**, hậu quả khi thất bại, và chỉ số trước → sau. Thao tác tốn tài nguyên phải khó bấm nhầm (giữ 0.8s).

## Player Context on Arrival

Cân nhắc, căng thẳng khi đập mốc cao ("canh bạc sống còn", merchant-economy Player Fantasy). Ở lò dã ngoại (Tier 2/3) có thể bị quái tấn công → cửa sổ tự đóng.

## Navigation Position

`Thế giới` → (tương tác đe/NPC Thợ Rèn ≤ 300 cm) → **Cửa sổ Lò Rèn** (modal `UCommonActivatableWidget`). Dịch vụ khả dụng phụ thuộc bậc lò (Tier 1/2/3).

## Entry & Exit Points

| Entry Source | Trigger | Player carries this context |
|---|---|---|
| Thế giới | Phím tương tác trong 300 cm (`FPAForgeUIModel::OpenDistance`), ngoài combat (`CanOpen()`) | Túi đồ, ví Vàng + Tàn Trang, bậc lò |

| Exit Destination | Trigger | Notes |
|---|---|---|
| Thế giới | `Esc`/`B`/nút Đóng | Đồ đặt trên đe trả về túi |
| Thế giới (tự đóng) | Rời > 500 cm (`AutoCloseDistance`) hoặc vào combat → `OnForgeAutoClose` | Trang bị và nguyên liệu trên ô chế tác trả nguyên vẹn về túi (Edge Case 2) |

---

## Layout Specification

### ASCII Wireframe

```
┌──────────────────────────────────────────────────────────────────────────┐
│ THỢ RÈN TIỀN TRẠM  [Tier 1 · tối đa +3]                         [Đóng]   │
├──────────────┬───────────────────────────────────────┬───────────────────┤
│ DỊCH VỤ      │             ĐE RÈN                    │ BA LÔ (lọc: trang │
│ [Cường Hóa]  │      ◇ Quặng Sắt  ✓ đủ                │ bị đặt lên đe được)│
│  Sửa Chữa    │                                       │ ┌──┬──┬──┬──┐     │
│  Khảm Ngọc 🔒│   ◇ Vàng 385 ✓  ╔════════╗  ◇ Bảo Hộ  │ │▣ │▣ │▣ │  │     │
│  Thần Binh 🔒│                 ║ Trảm   ║   (khóa:  │ └──┴──┴──┴──┘     │
│  Túi Đồ      │                 ║ Kiếm +2║   từ +7)  │                   │
│  Phân Rã     │                 ╚════════╝            │                   │
│              │              TỈ LỆ 100%               │                   │
│              │   Sát thương: 55 → 58 (+3)            │                   │
│              │   Thất bại: giữ nguyên cấp            │                   │
│              │   [■■■■■■□□□  GIỮ ĐỂ TÔI LUYỆN  0.8s] │                   │
├──────────────┴───────────────────────────────────────┴───────────────────┤
│ ◉ 1,240 Vàng   ◈ 37 Tàn Trang     [Giữ Space / A] Tôi luyện   [Esc] Đóng │
└──────────────────────────────────────────────────────────────────────────┘
```

### Layout Zones

| Zone | Nội dung |
|---|---|
| Z1 Header | Tên lò + bậc + giới hạn cường hóa |
| Z2 Tab dịch vụ (trái) | Theo GDD UI Req: Cường Hóa · Chế Tác Thần Binh (chỉ sáng ở Tier 3) · Khảm Ngọc & Đục Lỗ · Mở Rộng Túi Đồ · Phân Rã Trang Bị & Sách. Thêm **Sửa Chữa** (dịch vụ ở ma trận §1, không có trong danh sách tab — Q1) |
| Z3 Đe rèn (giữa) | Ô trung tâm (trang bị), ô vệ tinh (nguyên liệu, Vàng), ô Bảo Hộ, tỉ lệ %, so sánh chỉ số, nút giữ |
| Z4 Ba lô (phải) | Lưới lọc theo dịch vụ |
| Z5 Footer | Vàng · Tàn Trang · gợi ý phím |

### Nội dung từng tab

| Tab | Ô trung tâm | Vệ tinh / thông tin | Nút | Khả dụng |
|---|---|---|---|---|
| Cường Hóa | Trang bị | Nguyên liệu theo mốc (+1–3 Quặng Đồng/Sắt; +4–6 Quặng Sắt Đen + Tinh Thể Ma Pháp; +7–9 Quặng Hư Không + Tàn Trang Boss), Vàng `round(BaseFee × (1 + 0.35 × Level^1.4))`, ô Bảo Hộ (+7…+10), tỉ lệ %, chỉ số trước→sau, hậu quả thất bại | Giữ 0.8s | Tier 1 ≤ +3, Tier 2 ≤ +6, Tier 3 ≤ +10 |
| Sửa Chữa | Trang bị | Độ bền %, chi phí `ceil(Price × 0.25 × (1 − DurPct))` | Bấm (đề xuất "Sửa tất cả") | Mọi lò (Edge Case 1 "quy tắc cứu hộ") |
| Khảm Ngọc & Đục Lỗ | Trang bị Rare+ | Hàng ô ngọc (trống/có ngọc), danh sách ngọc trong túi, phí tháo 100 Vàng/viên | Đục lỗ: giữ 0.8s (đề xuất); Khảm/Tháo: bấm + xác nhận | Tier 2 (2 lỗ), Tier 3 (lỗ thứ 3 Prismatic) |
| Chế Tác Thần Binh | — | 1 Linh Hồn Lãnh Chúa, 4 bộ phận Boss, 5 Quặng Hư Không (đủ/thiếu) | Giữ 0.8s | Chỉ Tier 3 |
| Mở Rộng Túi Đồ | — | Bậc hiện tại (30/40/50/60), nguyên liệu + Vàng bậc kế (500 / 2,000 / 8,000) | Bấm + xác nhận | Bậc 1 ở Tier 1, Bậc 2 ở Tier 2, Bậc 3 ở Tier 3; 60/60 → khóa "Đã đạt sức chứa tối đa (60/60 Ô)" |
| Phân Rã | Trang bị hoặc Sách/Quyển trục | Sản lượng `2 + (Lv≥5) + (Lv≥9)` quặng theo độ hiếm; Sách → Tàn Trang 1/3/8/25 | Bấm; đồ Legendary hoặc ≥ +4: modal cảnh báo + giữ 1.5s (Edge Case 6) | Mọi lò (Phân rã Sách: Tier 1 theo ma trận) |

### Component Inventory

| Component | Loại | Tương tác | Pattern |
|---|---|---|---|
| Tab dịch vụ | Vertical tabs | Có | Button (Secondary) |
| Ô trung tâm đe | Item slot lớn | Có (đặt/gỡ) | Inventory Slot |
| Ô vệ tinh nguyên liệu | Slot + "có/cần" | Không | Inventory Slot (readonly) |
| Ô Bảo Hộ | Toggle slot | Có | Inventory Slot + toggle |
| Tỉ lệ thành công | Số lớn % | Không | Mới: Big Stat |
| So sánh chỉ số | Text list | Không | Tooltip & Item Compare (dòng delta) |
| Nút giữ để tôi luyện | Hold button + thanh tiến độ | Có | **Mới: Hold-to-Confirm** (chưa có trong pattern library) |
| Modal cảnh báo phân rã | Dialog | Có | Modal Dialog + Button (Destructive) |

### Information Hierarchy

1. Tỉ lệ thành công + hậu quả thất bại. 2. Chỉ số trước → sau. 3. Đủ/thiếu nguyên liệu và Vàng. 4. Ô Bảo Hộ. 5. Số dư.

---

## States & Variants

| State / Variant | Trigger | What Changes |
|---|---|---|
| Empty (chưa đặt đồ) | `TargetEquipmentId == NAME_None` | Ô trung tâm nét đứt "Chọn trang bị từ ba lô"; nút giữ khóa |
| Sẵn sàng | `CanForge()` true | Nút giữ sáng Amber |
| Thiếu nguyên liệu / Vàng | `!AllMaterialsSufficient()` / `!CanAffordGold()` | Ô vệ tinh đỏ + icon ✕ + "có/cần"; nút khóa với lý do |
| Vượt giới hạn lò | Cấp kế > giới hạn lò (`MaxTierLevelReached`) | Thông báo Edge Case 1: "Ngọn lửa tiền trạm quá yếu, không thể tôi luyện thần binh này. Hãy tìm đến Thợ Rèn Dã Ngoại hoặc Lò Rèn Cấm Địa!" |
| Tab khóa theo lò | Dịch vụ không có ở bậc lò này | Tab có icon khóa + dòng "Cần Lò Rèn Cấm Địa (Tier 3)" |
| Đang giữ | `bIsHolding` | Thanh `HoldProgress` 0→1 trong 0.8s; thả sớm → về 0, không gửi |
| Chờ server (loading) | `IsRequestPending()` | Hoạt ảnh búa 3 nhịp; khóa input |
| Thành công | `OnTransactionCompleted(true, None)` | Hào quang màu độ hiếm của món (inventory §1) + chuông ngân; cấp mới cập nhật |
| Thất bại roll | `OnTransactionCompleted(false, None)` | Khói đen, chữ "Thất bại — giữ nguyên +6" hoặc "Tụt về +7" (Formulas 3) |
| Bị từ chối | `OnTransactionCompleted(false, ErrorCode)` | Toast theo `EPACraftingError` (vd. `NoWardItem` "Không có Đá Bảo Hộ trong ba lô", `InsufficientSkillShards` "Không đủ Tàn Trang") |
| Đồ khóa | `bIsLocked` | Phân Rã khóa hoàn toàn (Edge Case 6) |
| Đã tối đa | +10 / độ bền 100% / 60 ô | Nút khóa với nhãn tương ứng (`MaxDurabilityAlready`, `MaxBackpackCapacity`) |

## Interaction Map

| Component | Bàn phím / Chuột | Gamepad | Phản hồi | Kết quả |
|---|---|---|---|---|
| Đổi tab | Click / `W`,`S` hoặc `↑↓` trong cột tab | `LB/RB` | Lật tab | Đổi dịch vụ |
| Đặt đồ lên đe | Click / kéo thả ô ba lô | `A` trên ô ba lô | Tiếng kim loại đặt | `SetTargetEquipmentSlot(SlotIndex)` |
| Gỡ đồ | Click ô trung tâm / chuột phải | `Y` | — | Đặt lại Empty |
| Ô Bảo Hộ | Click | `X` | Ô sáng | `ToggleWardStone(bool)` |
| Giữ tôi luyện | Giữ chuột trái trên nút hoặc giữ `Space` 0.8s | Giữ `A` 0.8s | Thanh lấp đầy + tiếng búa | `UpdateHoldInput(true)` → `OnForgeHoldCompleted` → `Server_RequestForgeEnhance` (hoặc `Server_RequestForgeBossSoul`) |
| Sửa | Bấm nút | `A` | Tiếng mài | `Server_RequestForgeRepair` |
| Đục lỗ / Khảm / Tháo | Nút | `A` | — | `Server_RequestForgeUnlockSocket` / `Server_RequestForgeSocketGem` / `Server_RequestForgeUnsocketGem` |
| Mở rộng túi | Nút + xác nhận | `A` | — | `Server_RequestForgeExpandBackpack` |
| Phân rã | Nút (giữ 1.5s cho đồ quý) | `A` / giữ `A` | Modal Destructive | `Server_RequestForgeSalvage` |
| Đóng | `Esc` | `B` | — | Đóng, trả đồ về túi |

## Events Fired

| Player Action | Event Fired | Payload / Data |
|---|---|---|
| Hoàn tất giữ | `OnForgeHoldCompleted` | — |
| Mọi yêu cầu | `UPAServiceRequestComponent::Server_RequestForge*` | RequestId, ForgeActor, SlotIndex/UID, bUseWard… |
| Xác nhận | `Client_ConfirmForgeRequest` → `OnTransactionCompleted(bSuccess, EPACraftingError)` | — |
| Tự đóng | `OnForgeAutoClose` | — |

Mọi thao tác đổi trang bị, Vàng, Tàn Trang là trạng thái bền vững do server quyết định (ADR-0001/0003).

## Transitions & Animations

- Mở: fade + tia lửa nhỏ 0.25s. Đóng: fade 0.15s.
- Thành công: hào quang theo màu độ hiếm 0.6s; +10: hào quang vũ khí (Niagara, ngoài UI).
- Thất bại: khói đen 0.6s.
- Reduced Motion / Flash Suppression: bỏ chớp sáng, giữ đổi chữ + âm thanh.

## Data Requirements

| Data | Source System | Read / Write | Notes |
|---|---|---|---|
| Trang bị trên đe | `FPAForgeUIModel::TargetEquipmentId/Name` | Read | **Gap:** không có độ hiếm, cấp +N hiện tại, độ bền, số lỗ ngọc |
| Nguyên liệu | `MaterialSlots` (`FPAForgeSlotEntry`: Required/Owned/bIsSufficient) | Read | Khớp GDD "đỏ nếu thiếu, xanh nếu đủ" |
| Vàng | `GoldCost`, `PlayerGold`, `CanAffordGold()` | Read | — |
| Tàn Trang | `UPACurrencyComponent` (SkillShards) | Read | **Gap:** model không có chi phí/số dư Tàn Trang |
| Bảo Hộ | `bUsingWard`, `ToggleWardStone` | R/W | — |
| So sánh chỉ số | `StatPreviews` (`FPAStatDeltaPreview::GetPreviewText` → "Attack: 50 -> 58 (+8)") | Read | Đúng GDD UI Req |
| Tỉ lệ thành công | — | Read | **Gap:** model không có tỉ lệ; cần trường từ `UPABlacksmithComponent` (Formulas 3) |
| Bậc lò / dịch vụ khả dụng | `UPABlacksmithComponent` | Read | **Gap:** model không có bậc lò, không có tab/mode |
| Giữ 0.8s | `HoldDuration`, `HoldProgress`, `bHoldCompleted` | Read | — |
| Khoảng cách / combat | `UpdateProximityState`, `OpenDistance 300`, `AutoCloseDistance 500` | Read | — |
| Yêu cầu dịch vụ | Widget chỉ gửi Enhance | Write | **Gap:** widget chưa gọi Repair/Salvage/Socket/BossSoul/Backpack dù router đã có RPC |

## Input Method Completeness Checklist

**Bàn phím:** [x] tab · [x] chọn ô ba lô · [x] giữ `Space` · [x] `Esc` đóng
**Chuột:** [x] kéo thả lên đe · [x] giữ chuột trái · [x] hover tooltip
**Gamepad:** [x] `LB/RB` · [x] D-pad · [x] giữ `A` · [x] `X` Bảo Hộ · [x] `B` đóng
Focus trap trong modal; modal phân rã trả focus về nút Phân Rã khi hủy.

## Accessibility

Tier Standard. Giữ-để-xác-nhận không phải bấm liên tục (accessibility §4.3); thời gian giữ 0.8s/1.5s cố định, đề xuất cho phép chuyển sang "bấm 2 lần để xác nhận" trong cài đặt (Q4). Đủ/thiếu = màu + ✓/✕ + số "có/cần". Tỉ lệ thành công là số, không chỉ màu. Chữ ≥ 24px; % ≥ 48px.

## Localization Considerations

- Thông báo Edge Case 1 dài — vùng thông báo xuống dòng, không cắt (HIGH PRIORITY).
- "Sát thương: 55 → 58 (+3)" dùng tham số; `GetPreviewText` hiện hardcode tiếng Anh "Attack" — cần `FText`.
- Số Vàng theo locale.

## Acceptance Criteria

- [ ] Cửa sổ mở trong ≤ 0.3s khi tương tác trong 300 cm ngoài combat.
- [ ] Đặt món Rare +2 lên đe ở Tier 1: hiện tỉ lệ 100%, Vàng 385 (`round(200 × (1 + 0.35 × 2^1.4))`), chỉ số "55 → 58 (+3)", "Thất bại: giữ nguyên cấp".
- [ ] Thả nút trước 0.8s không gửi RPC; giữ đủ 0.8s gửi đúng một RPC.
- [ ] Thiếu nguyên liệu: ô vệ tinh đỏ + ✕ + "có/cần"; nút khóa.
- [ ] Món đang +3 ở Tier 1 → thông báo lò quá yếu (Edge Case 1), tab Sửa Chữa vẫn dùng được.
- [ ] Phân rã đồ Legendary hoặc ≥ +4 cần modal + giữ 1.5s; đồ khóa không phân rã được.
- [ ] Vào combat khi đang mở → cửa sổ đóng, đồ trên đe về túi, `OnForgeAutoClose` bắn.
- [ ] Toàn cửa sổ dùng được bằng bàn phím và gamepad.

## Open Questions

- **Q1:** GDD UI Req liệt kê 5 tab, không có Sửa Chữa, dù Sửa Chữa là dịch vụ ở ma trận §1 và Edge Case 1. Mockup thêm tab Sửa Chữa — chờ duyệt.
- **Q2:** "Tàn Trang Boss" (nguyên liệu +7…+9, §2 A) có phải `item_skill_shard` không? Code có `InsufficientSkillShards`. Cần chốt để UI hiện đúng icon tiền.
- **Q3:** Số lượng quặng cho từng mốc cường hóa chưa có trong GDD (chỉ có loại quặng). Mockup chỉ hiện loại + trạng thái đủ, không hiện con số.
- **Q4:** Có cho phép tùy chọn thay giữ-0.8s bằng bấm-2-lần cho người khó giữ phím không? accessibility §4.2 chỉ liệt kê hold/toggle cho lock-on, sprint, guard, inspect.
- **Q5:** `UPABlacksmithForgeWidget` là `UUserWidget`, phải chuyển CommonUI.
- **Q6:** Z3 đặt "một Lò rèn" vào `L_VerdantFrontier_Outpost` — mockup giả định Thợ Rèn Tiền Trạm (Tier 1).
