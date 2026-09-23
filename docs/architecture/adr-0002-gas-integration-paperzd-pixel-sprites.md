# ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites

## Status
Accepted

## Date
2026-09-16

## Engine Compatibility

| Field | Value |
|---|---|
| **Engine** | Unreal Engine 5.7 |
| **Domain** | Animation, Gameplay Ability System (GAS), Collision & 2.5D Rendering |
| **Knowledge Risk** | 🟡 MEDIUM — Tích hợp plugin PaperZD với GAS trên UE 5.7 |
| **References Consulted** | `docs/engine-reference/unreal/VERSION.md`, `docs/engine-reference/unreal/modules/animation.md` |
| **Post-Cutoff APIs Used** | Không có |
| **Verification Required** | Kiểm tra tín hiệu Notify của PaperZD trên Dedicated Server khi chạy chế độ Headless (không render GPU). |

---

## ADR Dependencies

| Field | Value |
|---|---|
| **Depends On** | [`ADR-0001: Open World MMO Combat Networking`](adr-0001-open-world-mmo-combat-networking.md) |
| **Enables** | Các Stories lập trình Combo, Hitbox quét 3D, Lướt né I-Frame và Chiêu thức 12 Class. |
| **Blocks** | Không thể bắt đầu Sprint 1 cho Core Combat nếu chưa chốt hợp đồng PaperZD ➔ GAS. |
| **Ordering Note** | Bắt buộc phải Accepted trước khi tạo Epics/Stories cho Tầng 1 và Tầng 2. |

---

## Context

### Problem Statement
Project Ascendant sử dụng đồ họa 2.5D Pixel Art thông qua plugin **PaperZD** (kế thừa Paper2D) kết hợp với không gian vật lý 3D và hệ thống kỹ năng chuẩn của Epic Games: **Gameplay Ability System (GAS)**.
Tuy nhiên, GAS mặc định của Unreal Engine (`UAbilityTask_PlayMontageAndWait`) được thiết kế riêng cho **3D Skeletal Mesh Montages**, hoàn toàn không nhận diện được **PaperZD Flipbook Animations**. Nếu không có một cầu nối chuẩn mực:
1. Không thể kích hoạt hay ngắt Gameplay Ability theo frame diễn hoạt của Sprite.
2. Trạng thái bất tử (I-Frame của Dash) và khung kích hoạt sát thương (Active Hitbox Window) không thể đồng bộ chính xác với từng khung hình pixel của nhân vật.
3. Nguy cơ desync giữa Client và Dedicated Server nếu Server không thể "chạy" Flipbook để phát sinh Notify va chạm.

### Constraints & Requirements
- **Server-Authoritative 100%**: Server phải tự tính toán và xác thực va chạm đòn đánh mà không phụ thuộc vào hiển thị hình ảnh của Client.
- **Không dùng 3D Mesh giả lập**: Không tạo Skeletal Mesh 3D chạy ngầm để tiết kiệm tối đa CPU/RAM cho máy chủ MMO.
- **Độ trễ cảm nhận bằng 0**: Client phải kích hoạt hoạt ảnh và hiển thị vệt kiếm/bóng mờ ngay lập tức khi người chơi ấn nút.

---

## Decision

Nhóm kiến trúc quyết định lựa chọn **Phương Án A**: Xây dựng bộ cầu nối C++ chuyên biệt giữa **PaperZD** và **GAS** bao gồm 3 trụ cột cốt lõi:

```mermaid
flowchart TD
    subgraph Client["Client (Local Prediction)"]
        Input[Người Chơi Nhấn Phím] --> GA_Client[Kích Hoạt UGameplayAbility Cục Bộ]
        GA_Client --> Task_Client[UAbilityTask_PlayPaperZDAnimAndWait]
        Task_Client --> Flipbook_Client[PaperZD Character Component]
        Flipbook_Client --> Notify_Client[UPaperZDAnimNotifyState_AbilityEvent]
        Notify_Client --> VFX[Hiện Vệt Bóng Ghost Trail / Âm Thanh Cục Bộ]
    end

    subgraph Server["Dedicated Server (Authoritative)"]
        GA_Client -.->|Server RPC| GA_Server[Kích Hoạt UGameplayAbility Trên Server]
        GA_Server --> Task_Server[UAbilityTask_PlayPaperZDAnimAndWait (Headless)]
        Task_Server --> Notify_Server[UPaperZDAnimNotifyState_AbilityEvent]
        Notify_Server -->|NotifyBegin| Tag_On[Gán Tag: State.Invulnerable / State.Combat.HitboxActive]
        Notify_Server --> Sweep[Thực Hiện 3D Hitbox Sweep (Sphere/Box)]
        Notify_Server -->|NotifyEnd| Tag_Off[Xóa Tag: State.Invulnerable / State.Combat.HitboxActive]
    end

    Sweep -->|Trúng Đòn| GE[Áp Dụng UGameplayEffect Trừ Máu & Posture]
    GE -.->|Iris Replication| UI[Cập Nhật HUD Client]
```

### 1. Custom PaperZD Anim Notify State (`UPaperZDAnimNotifyState_AbilityEvent`)
- Được gắn trực tiếp vào các khung hình (Frames) trên Timeline của PaperZD Animation Source trong Editor.
- Khai báo 2 thuộc tính cấu hình:
  - `EventTag`: Thẻ `FGameplayTag` gửi lên ASC (ví dụ: `Event.Combat.Hitbox.Active`, `State.Invulnerable.Dash`).
  - `bServerOnly`: Xác định Notify chỉ chạy trên Server (dành cho tính toán va chạm) hay cả Client (dành cho VFX).
- Khi Playhead đi vào vùng Notify ➔ Gọi `UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(..., EventTag, Payload)`.

### 2. Custom Ability Task C++ (`UAbilityTask_PlayPaperZDAnimAndWait`)
- Kế thừa trực tiếp từ `UAbilityTask`, thay thế hoàn toàn cho task 3D mặc định của Epic.
- Giao tiếp trực tiếp với `UPaperZDAnimationComponent`:
  - Lắng nghe sự kiện hoàn tất hoạt ảnh (`OnAnimationPlaybackEnd`).
  - Lắng nghe sự kiện ngắt chiêu (`OnAnimationInterrupted`).
- Đảm bảo Ability tự động kết thúc hoặc mở cửa sổ combo chuẩn xác theo độ dài frame của sprite sheet.

### 3. Hitbox 3D Quét Trong Không Gian 2.5D
- Mặc dù nhân vật hiển thị bằng 2D Pixel Sprite, toàn bộ va chạm chiến đấu được tính toán bằng hình học 3D trong không gian thế giới (`FCollisionShape::MakeSphere` hoặc `MakeBox`).
- Tọa độ quét được gắn theo hướng quay 8 hướng của nhân vật (`FacingDirection`) và nạp thông số từ `UWeaponDataAsset` (Bán kính chém, góc quạt, tầm với).

---

## Key Interfaces

### A. Chữ Ký Lớp Cầu Nối C++
```cpp
// Source/ProjectAscendant/Core/Abilities/Tasks/AbilityTask_PlayPaperZDAnimAndWait.h
UCLASS()
class PROJECTASCENDANT_API UAbilityTask_PlayPaperZDAnimAndWait : public UAbilityTask
{
    GENERATED_BODY()
public:
    // Factory method khởi tạo task
    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility"))
    static UAbilityTask_PlayPaperZDAnimAndWait* PlayPaperZDAnimationAndWait(
        UGameplayAbility* OwningAbility,
        FName TaskInstanceName,
        UPaperZDAnimSequence* AnimationSequence,
        float PlayRate = 1.0f,
        bool bStopWhenAbilityEnds = true);

    UPROPERTY(BlueprintAssignable)
    FMontageWaitSimpleDelegate OnCompleted;

    UPROPERTY(BlueprintAssignable)
    FMontageWaitSimpleDelegate OnBlendOut;

    UPROPERTY(BlueprintAssignable)
    FMontageWaitSimpleDelegate OnInterrupted;

    UPROPERTY(BlueprintAssignable)
    FMontageWaitSimpleDelegate OnCancelled;
};
```

---

## Alternatives Considered

### Alternative 1: Tạo Skeletal Mesh 3D Vô Hình (Invisible 3D Skeleton Proxy)
- **Mô tả**: Gán một mô hình 3D người que vô hình chạy song song với 2D Sprite để dùng Montage mặc định của GAS.
- **Lý do từ chối**: Gây lãng phí tài nguyên CPU tính toán xương khớp và tăng gấp đôi số lượng ActorComponent cho mỗi nhân vật; hoàn toàn không phù hợp với mục tiêu 100 người chơi cùng lúc trên Dedicated Server MMO.

### Alternative 2: Pure Timer & DataTable (Đếm thời gian thuần)
- **Mô tả**: Không dùng Notify; lưu trữ thời gian bắt đầu và kết thúc của từng frame trong bảng DataTable rồi dùng C++ Timer để bật hitbox.
- **Lý do từ chối**: Tách rời dữ liệu diễn hoạt khỏi file hoạt ảnh gốc. Mỗi lần Animator thay đổi tốc độ frame hoặc cắt ngắn đòn đánh, Game Designer phải sửa tay toàn bộ số liệu thời gian trong DataTable, gây ra rủi ro lỗi lệch frame cực cao.

---

## Consequences

### Positive
- **Đồng bộ hoàn hảo**: Kỹ năng GAS và va chạm Hitbox gắn chặt vào từng frame pixel của Sprite Sheet.
- **Tối ưu hóa máy chủ**: Máy chủ Dedicated Server chạy PaperZD ở chế độ không đồ họa (Headless) chỉ cập nhật biến thời gian frame và bắn Notify mà không tốn tài nguyên render xương 3D.
- **Tương thích 100% với GAS**: Tận dụng toàn bộ sức mạnh của Gameplay Effects, Gameplay Tags và Cooldowns có sẵn của Epic Games.

### Negative & Risks
- **Rủi ro PaperZD chạy trên Dedicated Server**: Nếu plugin PaperZD bị tắt cập nhật component khi không có màn hình hiển thị (`bEnableServerTick`), Notify sẽ không phát sinh.
  - *Giải pháp giảm thiểu*: Bắt buộc thiết lập `UPaperZDAnimationComponent::bTickInEditorAndServer = true` trong hàm khởi tạo C++ của `APABaseCharacter`.

---

## GDD Requirements Addressed

| GDD System | Yêu Cầu Cụ Thể | Cách Thức ADR-0002 Giải Quyết |
|---|---|---|
| `dash-evasion.md` | Thời gian lướt 0.35s, I-frame 0.20s từ frame 2 đến frame 8. | Sử dụng `UPaperZDAnimNotifyState_AbilityEvent` gắn tag `State.Invulnerable` từ frame 2 đến frame 8. |
| `combat-system.md` | Chuỗi 3 combo đòn đánh, hitbox quét hình quạt theo vệt kiếm. | Notify kích hoạt `SweepMultiByChannel` trong không gian 3D tại frame vung kiếm cao trào. |
| `stagger-system.md` | Đòn kết liễu Finisher khóa góc camera và kích hoạt hoạt ảnh tất sát. | `UAbilityTask_PlayPaperZDAnimAndWait` đảm bảo animation kết liễu phát trọn vẹn không bị ngắt quãng. |
| `attributes-system.md` | Khấu trừ Thể Lực (Stamina Cost) khi tung đòn. | Chi phí được GAS trừ thông qua `UGameplayEffect` gắn kèm trước khi task phát hoạt ảnh. |

---

## Validation Criteria
1. Khi nhân vật thi triển đòn Dash, tag `State.Invulnerable` phải xuất hiện chính xác trên ASC tại frame 2 và biến mất tại frame 8.
2. Dedicated Server khi chạy dòng lệnh `-nullrhi` (Headless mode) vẫn phải ghi nhận trúng đòn va chạm của quái vật thông qua Anim Notify.
