# Advanced Classes & Dual-Class Specialization System

> **Trạng thái**: Approved (Quy Chuẩn Kiến Trúc Chức Nghiệp)  
> **Áp dụng**: Unreal Engine 5 (Gameplay Ability System, GameplayTags, Server-Authoritative Netcode)  
> **Tài liệu liên quan**:  
> - `design/gdd/foundational-classes.md`  
> - `design/gdd/skill-progression-system.md`  
> - `design/gdd/itemization.md`  
> - `design/gdd/character-visual-system.md`  

---

## 1. Cấu Trúc Cây Chuyển Chức 4 Nhánh (The 4 Class Lines)

Hệ thống chức nghiệp trong Project Ascendant được tổ chức theo cấu trúc phân nhánh hình cây gồm 4 Bậc tiến trình rõ rệt:
- **Bậc 1 (T1 Sơ Cấp - Foundational)**: Có sẵn từ màn hình tạo nhân vật, là gốc rễ của 4 Nhánh chính (`Guard`, `Scout`, `Caster`, `Faith`).
- **Bậc 2 (T2 Trung Cấp - Specialization)**: Chuyên hóa phong cách chiến đấu, mở khóa qua Quyển Trục Tinh Anh (Rare Promotion Scroll) hoặc kỳ ngộ dã ngoại.
- **Bậc 3 (T3 Cao Cấp - Mastery)**: Làm chủ sức mạnh tối thượng, mở khóa qua Quyển Trục Lãnh Chúa (Epic Promotion Scroll / Boss Soul Relic).
- **Bậc 4 (T4 Ẩn - Apex / Mythic)**: Danh hiệu siêu việt, mở khóa qua Thử Thách No-Hit kết hợp điều kiện tiến trình đa nhánh.

### Sơ Đồ Cây Chuyển Chức Toàn Diện:

```
                    ┌──────────────────────────────────────────────────────────┐
                    │               BẬC 4 (T4 ẨN / MYTHIC)                     │
                    │               GOD SLAYER (Thí Thần Giả)                  │
                    │  (Yêu cầu: Kết hợp Đa Nhánh + Khiêu Chiến No-Hit Boss)  │
                    └─────────────────────────────▲────────────────────────────┘
                                                  │ (Kỳ ngộ đa nhánh)
         ┌───────────────────────┬────────────────┴───────────────────────┬───────────────────────┐
         │                       │                                        │                       │
┌────────┴────────┐     ┌────────┴────────┐                      ┌────────┴────────┐     ┌────────┴────────┐
│ BẬC 3 (T3 CAO)  │     │ BẬC 3 (T3 CAO)  │                      │ BẬC 3 (T3 CAO)  │     │ BẬC 3 (T3 CAO)  │
│  DRAGON KNIGHT  │     │   VOID BLADE    │                      │  PHANTOM STALKER│     │  CHRONOMANCER   │     │    SERAPH       │
│  (Long Kỵ Sĩ)   │     │(Hư Không Kiếm Sư)│                     │(Ảo Ảnh Dạ Hành) │     │ (Thời Gian Ma Đạo)│   │ (Lục Dực Thần Sứ)│
└────────▲────────┘     └────────▲────────┘                      └────────▲────────┘     └────────▲────────┘     └────────▲────────┘
         │                       │                                        │                       │                       │
┌────────┴────────┐     ┌────────┴────────┐                      ┌────────┴────────┐     ┌────────┴────────┐     ┌────────┴────────┐
│ BẬC 2 (T2 TRUNG)│     │ BẬC 2 (T2 TRUNG)│                      │ BẬC 2 (T2 TRUNG)│     │ BẬC 2 (T2 TRUNG)│     │ BẬC 2 (T2 TRUNG)│
│TEMPLAR/BERSERKER│     │   SWORDMASTER   │                      │   SHADOWBLADE   │     │   ELEMENTALIST  │     │   INQUISITOR    │
│(Thánh Binh/Cuồng)│    │   (Kiếm Sư)     │                      │ (Ảo Ảnh Thích Khách)│  │  (Nguyên Tố Sư) │     │  (Thẩm Phán)    │
└────────▲────────┘     └────────▲────────┘                      └────────▲────────┘     └────────▲────────┘     └────────▲────────┘
         │                       │                                        │                       │                       │
         └───────────┬───────────┘                                        │                       │                       │
                     │                                                    │                       │                       │
            ┌────────┴────────┐                                  ┌────────┴────────┐     ┌────────┴────────┐     ┌────────┴────────┐
            │ BẬC 1 (T1 SƠ)   │                                  │ BẬC 1 (T1 SƠ)   │     │ BẬC 1 (T1 SƠ)   │     │ BẬC 1 (T1 SƠ)   │
            │    VANGUARD     │                                  │     RANGER      │     │    ARCANIST     │     │     ACOLYTE     │
            │  (Chiến Binh)   │                                  │   (Du Hiệp)     │     │   (Thuật Sĩ)    │     │    (Tu Sĩ)      │
            └─────────────────┘                                  └─────────────────┘     └─────────────────┘     └─────────────────┘
             [NHÁNH 1: GUARD]                                     [NHÁNH 2: SCOUT]        [NHÁNH 3: CASTER]       [NHÁNH 4: FAITH]
```

### Danh Mục 16 Chức Nghiệp & Quy Chuẩn GameplayTag:

| Nhánh (Line) | Bậc (Rank) | Tên Chức Nghiệp | GameplayTag Chuẩn Hóa | Vũ Khí Định Danh | Phong Cách Chiến Đấu Cốt Lõi |
| :--- | :---: | :--- | :--- | :--- | :--- |
| **Guard** | T1 | **Vanguard (Chiến Binh)** | `Class.Line.Guard.Vanguard` | `1H.Blade` + Khiên Sắt Vuông | Cận chiến cân bằng, phản đòn chớp nhoáng (Parry), càn lướt vững chắc. |
| **Guard** | T2 | **Templar (Thánh Binh)** | `Class.Line.Guard.Templar` | `1H.Mace` + Đại Thuẫn | Tanker thuần túy, thu hút hận thù (Taunt), tạo khiên thánh hóa, phản sát thương. |
| **Guard** | T2 | **Berserker (Cuồng Chiến Sĩ)** | `Class.Line.Guard.Berserker` | `2H.Heavy` (Đại Đao/Đại Rìu) | Liều mạng đổi sát thương, Super Armor không ngắt chiêu, máu càng thấp đánh càng nhanh. |
| **Guard** | T2 | **Swordmaster (Kiếm Sư)** | `Class.Line.Guard.Swordmaster` | `1H.Blade` (Kiếm đơn, bỏ khiên) | Kiếm đạo tốc độ, bộ pháp lướt né thanh thoát, phản kích chuẩn xác từng khung hình. |
| **Guard** | T3 | **Dragon Knight (Long Kỵ Sĩ)** | `Class.Line.Guard.DragonKnight` | `2H.Polearm` (Chiến Kích) | Bật nhảy không chiến, giáng đòn phá vỡ thế đứng cực mạnh (Posture Crush), hơi thở rồng lửa. |
| **Guard** | T3 | **Void Blade (Hư Không Kiếm)** | `Class.Line.Guard.VoidBlade` | `1H.Blade` (Kiếm đơn Hư Không) | Trảm kích không thời gian (*Judgement Cut*), lướt dịch chuyển tức thời, tàn ảnh hư vô. |
| **Scout** | T1 | **Ranger (Du Hiệp)** | `Class.Line.Scout.Ranger` | `2H.Bow` (Cung hai tay) | Xạ kích tầm xa, thả diều cơ động, đặt bẫy chông làm chậm mục tiêu. |
| **Scout** | T2 | **Shadowblade (Ảo Ảnh Thích Khách)** | `Class.Line.Scout.Shadowblade` | `Dual.Daggers` (Song đoản đao) | Ám sát tàng hình, lướt để lại phân thân đánh lừa quái, đòn đánh sau lưng (Backstab) x3 bạo kích. |
| **Scout** | T3 | **Phantom Stalker (Ảo Ảnh Dạ Hành)** | `Class.Line.Scout.PhantomStalker` | `Dual.Daggers` / Cung ám khí | Bẫy hư không bóng tối, phân thân liên hoàn cùng tấn công, biến mất vào màn đêm. |
| **Caster** | T1 | **Arcanist (Thuật Sĩ)** | `Class.Line.Caster.Arcanist` | `2H.Staff` (Pháp trượng) | Kiểm soát chiến trường diện rộng, dồn quái thành cụm, phóng hồ quang nguyên tố bùng nổ. |
| **Caster** | T2 | **Elementalist (Nguyên Tố Sư)** | `Class.Line.Caster.Elementalist` | `2H.Staff` / Cầu Phép (Orbs) | Phản ứng nguyên tố xoay vần: Đóng băng khống chế, Thiêu đốt rút máu, Sét giật lan truyền. |
| **Caster** | T3 | **Chronomancer (Thời Gian Ma Đạo)** | `Class.Line.Caster.Chronomancer` | `Relic` (Đồng Hồ Cát) | Bong bóng ngưng đọng thời gian, tua ngược vị trí và lượng máu bản thân về 3 giây trước. |
| **Faith** | T1 | **Acolyte (Tu Sĩ)** | `Class.Line.Faith.Acolyte` | `1H.Mace` + Tràng Hạt Khí | Hỗ trợ sinh tồn, thanh tẩy hiệu ứng xấu, hồi phục thể lực (Stamina) cho bản thân và đồng đội. |
| **Faith** | T2 | **Inquisitor (Thẩm Phán)** | `Class.Line.Faith.Inquisitor` | `1H.Mace` (Chiến Chùy) | Trừng phạt dị giáo, sát thương đập nện cực mạnh, ấn chú làm suy yếu giáp và thế đứng của Boss. |
| **Faith** | T3 | **Seraph (Lục Dực Thánh Sứ)** | `Class.Line.Faith.Seraph` | `1H.Mace` / Thánh Tích | Đôi cánh ánh sáng thánh hóa (chân chạm đất), dựng kết giới bất tử ngắn hạn, giáng sấm sét thánh tích toàn bản đồ. |
| **Apex** | T4 | **God Slayer (Thí Thần Giả)** | `Class.Line.Apex.GodSlayer` | Thần Binh Hư Vô Tự Biến Hình | Né hoàn hảo ngưng đọng thời gian 1s, tước đoạt và sao chép chiêu thức đặc trưng của Boss. |

---

## 2. Cơ Chế Song Chức Nghiệp: Class Chính & Class Phụ (Dual-Class System)

Mỗi nhân vật được trang bị đồng thời **1 Class Chính (Primary Class)** và **1 Class Phụ (Secondary Sub-Class)**:

### 2.1 Quyền Hạn & Vai Trò Của Class Chính (Primary Class):
1. **Ngoại Hình Độc Quyền (Visual Identity)**: Mô hình nhân vật hiển thị đúng dáng đứng (Idle Stance), mũ giáp, phù hiệu (Crest) và áo choàng (Tabard) của Class Chính.
2. **Vai Trò Tổ Đội (Party Role)**: Nhãn tìm đội (LFG) và tính năng kết giao đội hình liên kết trực tiếp với vai trò của Class Chính.
3. **Vũ Khí & Hoạt Ảnh Căn Bản**: Xác định họ vũ khí chính và toàn bộ hoạt ảnh di chuyển/chiến đấu căn bản.
4. **Action Deck**: Chiếm **3 Slot Chủ Động (Q, E, R)** và **2 Slot Nội Tại (P1, P2)**.

### 2.2 Vai Trò & Giới Hạn Của Class Phụ (Secondary Sub-Class):
1. **Gia Tăng Chiều Sâu Lối Chơi (Build Diversity)**: Đóng góp **1 Slot Chủ Động (F)** và **1 Slot Nội Tại (P3)** vào Action Deck.
2. **Nguyên Tắc Tương Thích Vũ Khí (Weapon Compatibility Rule - Quyết định B1)**:
   - Khi Class Phụ khác họ vũ khí với Class Chính: Slot F của Class Phụ chỉ được phép gắn kỹ năng **Không Phụ Thuộc Vũ Khí (Weapon-Agnostic Active)** như đặt bẫy, tạo kết giới, hồi phục, ngưng đọng thời gian hoặc phân thân. Kỹ năng đòi hỏi vũ khí riêng (như bắn cung) sẽ bị khóa xám nếu nhân vật đang cầm kiếm/khiên.
3. **Nguyên Tắc Giới Hạn Trần Bậc (Rank Capping Rule - Quyết định A1)**:
   - Bậc của Class Phụ **tuyệt đối không được vượt quá Bậc của Class Chính** ($\text{Rank}_{\text{Sub}} \le \text{Rank}_{\text{Primary}}$).
   - Nếu Class Phụ có cấp tiến trình cao hơn, toàn bộ chỉ số cộng thêm và cấp độ kỹ năng của Class Phụ sẽ tự động bị giới hạn (clamp) ở mức tối đa của Bậc Class Chính hiện thời.
4. **Hoán Đổi Vị Trí Chính ↔ Phụ (Quy Tắc A1)**:
   - Người chơi được phép hoán đổi vị trí Class Chính ↔ Class Phụ **hoàn toàn miễn phí tại Tòa Thành (Citadel Safe Zone)**.
   - **Ứng dụng cày cấp**: Người chơi có thể tận dụng quy tắc hoán đổi này để đổi một class phụ mới luyện lên vị trí Class Chính nhằm hưởng tỷ lệ nhận EXP cao hơn (70% thay vì 30%), giúp cày cấp chức nghiệp phụ nhanh chóng hơn.
5. **Tiến Trình Độc Lập & Bảo Toàn Vĩnh Viễn**:
   - Mỗi chức nghiệp có tiến trình cấp độ riêng biệt gắn liền với Nhân vật. Khi chuyển sang class mới, tiến trình class cũ được bảo toàn $100\%$, không bao giờ bị mất khi chọn lại.

---

## 3. Hệ Thống Hai Thang Đo Tiến Trình (Dual-Track Progression)

Tiến trình phát triển trong Project Ascendant vận hành trên 2 thang đo song hành:

### 3.1 Cấp Độ Nhân Vật (Character Level 1–50):
- Là cấp độ sinh mệnh xuyên suốt của nhân vật, nhận EXP từ mọi hoạt động (tiêu diệt quái, nhiệm vụ, khám phá).
- **Không bao giờ bị giảm hay reset** khi đổi chức nghiệp.
- Quyết định lượng Máu/Stamina/Mana nền tảng và cấp độ trang bị mang được ($iLvl$ 1–50).

### 3.2 Cấp Độ Chức Nghiệp (Class Level 1–20 mỗi Bậc):
- Đại diện cho mức độ thuần thục của nhân vật với chức nghiệp cụ thể.
- Khi tiêu diệt quái vật hoặc hoàn thành nhiệm vụ:
  - $100\%$ EXP thu được nạp vào **Character Level**.
  - $70\%$ Class EXP nạp vào **Class Chính**.
  - $30\%$ Class EXP nạp vào **Class Phụ**.
- Khi đạt Class Level 20 của Bậc hiện tại, nhân vật đủ điều kiện kích hoạt Quyển Trục thăng chức để lên Bậc tiếp theo.

---

## 4. Cơ Chế Thăng Chức Bằng Quyển Trục (Class Promotion Scrolls)

### 4.1 Định Nghĩa Data Asset Quyển Trục (`UClassPromotionScrollDefinition`):
```cpp
UCLASS(BlueprintType)
class PROJECTASCENDANT_API UClassPromotionScrollDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Promotion")
    FName ScrollID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Promotion")
    FGameplayTag TargetClassTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Promotion")
    TArray<FGameplayTag> AllowedSourceClassTags;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Promotion")
    uint8 RequiredMinRank = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Promotion")
    int32 RequiredSourceClassLevel = 20;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Promotion")
    FGameplayTag RequiredQuestFlag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Promotion|Apex")
    bool bRequiresDualSlotCondition = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Promotion|Apex", meta = (EditCondition = "bRequiresDualSlotCondition"))
    TArray<FGameplayTag> SecondaryAllowedSourceTags;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Promotion|Apex", meta = (EditCondition = "bRequiresDualSlotCondition"))
    uint8 SecondaryRequiredMinRank = 2;
};
```

### 4.2 Thẩm Định Giao Dịch Thăng Chức Server-Authoritative:
Mọi thao tác sử dụng quyển trục được máy chủ Dedicated Server thẩm định nghiêm ngặt qua giao dịch Database có khóa dòng (Row-level Locking) chống trùng lặp tài nguyên (Dupe Prevention):

1. Client gửi Server RPC yêu cầu sử dụng Quyển Trục tại Tòa Thành.
2. Server thực thi giao dịch SQL có khóa:
   ```sql
   BEGIN;
   SELECT item_instance_id, item_def_id, owner_type, owner_id, quantity 
   FROM items 
   WHERE item_instance_id = $1 AND owner_type = 'CHARACTER' AND owner_id = $2
   FOR UPDATE;
   ```
3. Server kiểm tra đối chiếu: Nếu `item_def_id` không khớp với Quyển Trục yêu cầu hoặc nhân vật không thỏa mãn mảng `AllowedSourceClassTags` và `RequiredMinRank`, Server lập tức thực hiện `ROLLBACK` và từ chối thao tác.
4. Nếu kiểm tra hợp lệ, Server cập nhật thẻ class mới vào đúng slot (Primary HOẶC Secondary tùy theo người chơi chỉ định) và tiêu hủy Quyển Trục:
   ```sql
   UPDATE characters 
   SET primary_class_tag = $3,
       class_progression_history = jsonb_set(class_progression_history, ARRAY[$3], $4, true)
   WHERE character_id = $2;

   DELETE FROM items WHERE item_instance_id = $1;
   COMMIT;
   ```

---

## 5. Hai Thang Độ Hiếm Phân Biệt Tuyệt Đối

Tránh nhầm lẫn trong toàn bộ hệ thống trò chơi bằng việc tách biệt 2 thang độ hiếm:
1. **Thang Độ Hiếm Trang Bị (5 Bậc theo `itemization.md`)**:
   - `Common` (Trắng) $\rightarrow$ `Uncommon` (Lục) $\rightarrow$ `Rare` (Lam) $\rightarrow$ `Epic` (Tím) $\rightarrow$ `Legendary` (Vàng Kim).
2. **Thang Độ Hiếm Sách Kỹ Năng & Quyển Trục (4 Bậc theo `skill-progression-system.md`)**:
   - `Normal` (Sách cơ bản khởi đầu)
   - `Rare` (Quyển Trục Bậc 2 Specialization)
   - `Epic` (Quyển Trục Bậc 3 Mastery)
   - `Mythic` (Quyển Trục Bậc 4 Apex - God Slayer)

### Phân Rã Quyển Trục Thừa:
Quyển trục không dùng có thể phân rã tại Thợ Rèn thành **Tàn Trang Kỹ Năng (`item_skill_shard`)**:
- Quyển Trục Bậc 2 (Rare Scroll): Nhận **3 Tàn Trang**.
- Quyển Trục Bậc 3 (Epic Scroll): Nhận **8 Tàn Trang**.
- Quyển Trục Bậc 4 (Mythic Scroll): Nhận **25 Tàn Trang**.
