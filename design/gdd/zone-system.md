# Non-gated Open Zones & Checkpoints

> **Status**: In Design  
> **Author**: Systems Designer & World Builder  
> **Last Updated**: 2026-09-15  
> **Implements Pillar**: True Skill Expression & High-Risk High-Reward Exploration  
> **Target Engine**: Unreal Engine 5.7 (World Partition, Data Layers, Navigation Invokers)

---

## Overview

Hệ thống Bản Đồ Mở & Điểm Lưu (Non-gated Open Zones & Checkpoints) định hình cấu trúc không gian thế giới, phân tầng địa lý và nhịp độ thám hiểm cốt lõi trong Project Ascendant, kết nối trực tiếp cơ chế điều khiển góc nhìn 2.5D Isometric ([`isometric-controller.md`](file:///mnt/Data/Projects/project-games/design/gdd/isometric-controller.md)) với vòng lặp kinh tế dã ngoại và thợ rèn phân vùng ([`blacksmithing-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/blacksmithing-system.md)). Khác biệt hoàn toàn với các tựa game nhập vai tuyến tính khóa đường bằng rào cản cấp độ nhân vật (Level-gating) hay tường vô hình, thế giới trò chơi được kiến trúc hoàn toàn mở và kết nối liền mạch qua 3 bậc nguy hiểm—từ *Tiền Trạm Sơ Khai* (Tier 1: Outpost/Verdant Frontier), *Hoang Dã Tàn Tích* (Tier 2: Ashen Wilderness) cho đến *Cấm Địa Thần Tích* (Tier 3: Forbidden Sanctum). Thiết kế này trao trọn quyền tự quyết cho người chơi: những ai sở hữu phản xạ và kỹ năng né tránh i-frame ([`dash-evasion.md`](file:///mnt/Data/Projects/project-games/design/gdd/dash-evasion.md)) điêu luyện có thể vượt cấp thâm nhập sào huyệt nguy hiểm ngay từ những phút đầu tiên.

Trọng tâm điều tiết nhịp độ sinh tồn của thế giới là mạng lưới Điểm Lưu / Lửa Trại (Campfires / Checkpoints)—những ốc đảo an toàn hiếm hoi giữa vùng đất chết. Người chơi chủ động tương tác với Lửa Trại để hồi phục trọn vẹn Sinh Lực, Nội Lực và Thể Lực ([`attributes-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/attributes-system.md)), thiết lập điểm hồi sinh khi tử trận, tự do tinh chỉnh bộ kỹ năng xuất trận (Action Deck Loadout từ [`skill-progression-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/skill-progression-system.md)), luân chuyển Chức nghiệp trong số các Class đã mở khóa ([`foundational-classes.md`](file:///mnt/Data/Projects/project-games/design/gdd/foundational-classes.md)), và kích hoạt Dịch Chuyển Nhanh (Fast Travel) giữa các Lửa Trại đã được thắp sáng. Đổi lại, mỗi lần tĩnh dưỡng tại Lửa Trại sẽ lập tức tái tạo (respawn) toàn bộ quái vật thông thường ngoài tự nhiên, buộc người chơi phải liên tục đưa ra quyết định đánh cược chiến thuật: quay về trạm nghỉ an toàn hay dấn thân sâu hơn vào lãnh địa Lãnh Chúa.

## Player Fantasy

Hệ thống Bản Đồ Mở & Điểm Lưu phục vụ trực tiếp Trụ cột 1: **"Chinh phục điều không thể bằng kỹ năng tuyệt đỉnh"** và Trụ cột 3: **"Kinh tế dã ngoại & Thám hiểm rủi ro cao - phần thưởng lớn"**. Không gian mở không rào chắn mang lại cho người chơi cảm giác tự do vô bờ bến đi đôi với sự tính toán chiến thuật nghẹt thở thông qua góc nhìn Isometric 2.5D và sự ấm áp kỳ diệu khi tìm thấy đồng loại giữa tử địa:

> *"Từ góc nhìn Isometric nghiêng -45° khoáng đạt, toàn cảnh vùng đất chết hiện ra như một bàn cờ sinh tử sống động. Bạn lia nhẹ tâm ngắm về phía trước, hệ thống camera Lookahead mở rộng tầm nhìn cho bạn quan sát từ trên cao: những rãnh nứt nham thạch sủi bọt, cạm bẫy gai cổ đại rải rác trên lối mòn, và quỹ đạo tuần tra gắt gao của những con quái tinh anh cấp 30 to lớn gấp ba lần nhân vật cấp 12 của bạn.*
> 
> *Không có đường ranh giới nhân tạo hay màn hình tải cắt cảnh, việc thám hiểm trở thành một vũ điệu chiến thuật đầy mê hoặc. Tận dụng địa hình cao thấp và các vách đá che khuất tầm nhìn, bạn căn chỉnh từng bước chân, lướt né I-frame qua những góc mù của quái vật. Một bước sẩy chân, một va chạm nhỏ cũng đủ kích hoạt đàn quái truy sát và tiễn bạn về điểm xuất phát. Nhưng chính từ độ cao bao quát này, mắt bạn bắt trọn một dải khói lam thanh mảnh—Cột Khói Lửa Trại (Smoke Beacon) đang kiêu hãnh bốc lên giữa hẻm núi tử thần, xuyên qua lớp sương mù mờ ảo.*
> 
> *Bạn bấm lướt Dash liên hoàn qua khe nứt hiểm hóc, luồn qua cú quét móng vuốt chết người trong gang tấc với chỉ một vạch máu mong manh. Khi chạm tới bãi đá tàn tích, bạn rút gươm quẹt mạnh vào đá lửa.*
> 
> *'XOẸT! BÙNG!'*
> 
> *Ngọn lửa trại bùng lên rực rỡ, tạo thành một vầng hào quang ấm áp đẩy lùi lũ quái vật gầm gừ ngoài ranh giới an toàn. Toàn bộ thanh Sinh Lực, Thể Lực và bình hồi phục lập tức hồi sinh trong tiếng vĩ cầm trầm lắng. Và rồi, trong ánh lửa bập bùng, những bóng hình đồng loại thân quen dần hiện rõ: một gã Thương Nhân lang thang đang thong thả rít tẩu thuốc ngắm nhìn bầu trời đêm, và người Thợ Rèn Dã Ngoại bụi bặm đang nhịp nhàng gõ chiếc búa nặng lên đe sắt di động—'KENG... KENG... KENG...'—khẽ gật đầu chào bạn với ánh mắt tán thưởng.*
> 
> *Từ cảm giác cô độc cận kề cái chết chuyển sang khoảnh khắc an tâm, ấm áp bên những người bạn đồng hành dã ngoại giữa lòng hiểm nguy, đó là thứ khoái cảm sinh tồn chân thực nhất mà chỉ những kẻ dám đánh cược tính mạng mới có thể chạm tới."*

## Detailed Design

### Core Rules

#### 1. Ma Trận Phân Cấp 3 Bậc Vùng Đất Mở Không Rào Cản (3-Tier Non-gated Open Zones)
Thế giới Project Ascendant được xây dựng bằng công nghệ Unreal Engine 5 World Partition, kết nối liền mạch không màn hình chờ (No Loading Screens) và không rào cản nhân tạo (No Invisible Walls / Level-gating):

| Bậc Vùng Đất | Tên Khu Vực Địa Lý | Cấp Độ Quái Khuyến Nghị | Đặc Điểm Môi Trường & Địa Hình | Điểm An Toàn & Cơ Sở Dã Ngoại | Cơ Chế PvP / Cạnh Tranh |
| :---: | :--- | :---: | :--- | :--- | :--- |
| **Tier 1** | **Tiền Trạm Sơ Khai** *(Verdant Frontier)* | Cấp 1–15 | Rừng thưa ôn hòa, lối mòn rộng, ít cạm bẫy địa hình. Quái đơn lẻ hoặc nhóm nhỏ 2–3 con. | **Thị Trấn Tiền Trạm** (Trấn thủ bởi Vệ Binh NPC cấp 50); Thợ Rèn Tier 1 ([`blacksmithing-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/blacksmithing-system.md)); Thương Nhân Cơ Bản. | **Khu Vực Bảo Hộ**: Cấm PvP hoàn toàn trong bán kính thị trấn; ngoài hoang dã PvP tự do có áp dụng Karma. |
| **Tier 2** | **Hoang Dã Tàn Tích** *(Ashen Wilderness)* | Cấp 16–30 | Rừng rậm tro tàn, đầm lầy độc tố, hẻm núi dung nham. Quái tinh anh tuần tra theo bầy 4–6 con. | Các đốm **Lửa Trại Dã Ngoại** rải rác; Doanh Trại Thợ Rèn Dã Ngoại Tier 2 (đục 2 lỗ ngọc, lên +6); Thương Nhân Lang Thang. | **PvP Hoang Dã Tự Do**: Cạnh tranh Boss dã ngoại khốc liệt; hạ sát người vô tội trừ -30 Karma. |
| **Tier 3** | **Cấm Địa Thần Tích** *(Forbidden Sanctum)* | Cấp 31–50 | Vùng tử địa sương mù dày đặc, rãnh vực không thời gian, bẫy sét và nham thạch phun trào. Quái cấp cao mang kháng hiệu ứng và sát thương hủy diệt. | 1–2 Lửa Trại ẩn sâu trong hang động; **Lò Rèn Cấm Địa Tier 3** (nơi duy nhất đúc Thần Binh Bậc 5 từ Linh Hồn Boss); Chợ Đen bí mật. | **Vùng Đất Tử Thần (High-Risk)**: Điểm nóng của các trận săn World Boss và thợ săn tiền thưởng thanh trừng tội đồ Wanted. |

---

#### 2. Cơ Chế Lửa Trại & Thánh Địa An Toàn (Campfire & Sanctuary System)
Mỗi Lửa Trại dã ngoại là một ốc đảo sinh tồn có bán kính bảo vệ **Sanctuary Radius = 1000 cm**:
1. **Kết Giới Thánh Địa**:
   * Quái vật không thể bước qua ranh giới Sanctuary (AI NavMesh Barrier đẩy lùi quái). Mọi trạng thái Aggro hiện tại của người chơi được xóa sạch ngay khi bước chân vào kết giới.
   * **Cấm Giao Tranh PvP Trong Thánh Địa**: Mọi vũ khí và kỹ năng tấn công của người chơi tự động bị khóa (`State.Pacified`), ngăn chặn hoàn toàn việc camp cửa giết người tại điểm hồi sinh.
2. **Các Thao Tác Tại Lửa Trại**:
   * **Tĩnh Dưỡng (Rest)**: Kích hoạt GAS GameplayEffect hồi phục tức thì 100% Max HP, 100% Max Mana, 100% Max Stamina, và nạp đầy số lần dùng của Bình Dược Phẩm (Flask charges). Đồng thời, máy chủ lập tức tái tạo (respawn) toàn bộ quái vật thường và tinh anh trong vùng (World Boss và Miniboss độc bản không bị hồi sinh).
   * **Điểm Hồi Sinh (Spawn Anchor)**: Ghi đè tọa độ hồi sinh mới nhất của người chơi tại Lửa Trại vừa tương tác.
   * **Dịch Chuyển Nhanh (Fast Travel)**: Cho phép dịch chuyển tức thời giữa bất kỳ 2 Lửa Trại nào đã được thắp sáng. Thời gian niệm chú 2.0 giây tại Lửa Trại; hoàn toàn miễn phí; nếu chịu sát thương hoặc di chuyển sẽ bị ngắt niệm.
   * **Thay Đổi Class & Action Deck**: Cho phép người chơi tự do hoán đổi 4 Active Skills + 3 Passive Skills (Action Deck từ [`skill-progression-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/skill-progression-system.md)) và chuyển đổi linh hoạt giữa 12 Chức nghiệp đã mở khóa ([`foundational-classes.md`](file:///mnt/Data/Projects/project-games/design/gdd/foundational-classes.md)) mà không tốn bất kỳ chi phí nào.

---

#### 3. Luật Chiến Đấu Cạnh Tranh Thực & Mở Tag (Contested Combat & Open Tagging — ADR-0001)
1. **Không Khóa Mục Tiêu (No Kill-Lock / No Tag Exclusivity)**:
   * Bất kỳ người chơi nào cũng có thể tự do tấn công quái vật hoặc Boss thế giới đang giao tranh với người khác.
2. **Chia Loot & EXP Có Ngưỡng Đóng Góp (Contribution Threshold $\ge 5\%$)**:
   * Máy chủ (Dedicated Server) theo dõi độc lập lượng sát thương HP và Posture Damage của từng cá nhân/tổ đội.
   * Để đủ điều kiện nhận bảng rơi đồ cá nhân riêng biệt (Instanced Loot), người chơi (hoặc tổ đội) phải gây tối thiểu $\ge 5\%$ tổng sát thương HP hoặc tích lũy $\ge 10\%$ tổng Posture Damage lên mục tiêu.
   * Người chơi đạt ngưỡng nhận hòm đồ riêng độc lập trên máy chủ; không có tình trạng tranh cướp nhặt đồ vật lý (No Ninja Looting).
3. **Thanh Posture Chung & Quyền Kết Liễu (Execution)**:
   * Thanh Posture của Boss là thanh chung duy nhất, tự động scale theo số lượng người tham chiến: $Posture_{max} = BasePosture \times [1 + 0.35 \times (N_{combatants} - 1)]$.
   * **Cửa Sổ Ưu Tiên 1.5s Cho Finisher**: Khi thanh Posture bị bẻ gãy (mở cửa sổ Stagger 3.0s), trong 1.5s đầu tiên, chỉ duy nhất người chơi tung đòn bẻ gãy cuối cùng (Posture Break Finisher) mới nhìn thấy nút bấm Execution (gây 25% Max HP của Boss).
   * **Mở Tự Do Sau 1.5s**: Nếu sau 1.5s Finisher chưa bấm, nút bấm Execution kích hoạt công khai cho toàn bộ người chơi tham chiến. Bất kỳ ai bấm sau người đầu tiên tự động chuyển thành đòn bồi trợ (*Contested Assist Strike*) gây thêm 5% sát thương phụ.
4. **Cân Bằng Độ Khó Động (Dynamic Difficulty Scaling - DDS)**:
   * Máu Boss tự động scale: $HP_{max} = BaseHP \times [1 + 0.50 \times (N_{combatants} - 1)]$.
   * Cơ chế Kháng Đám Đông (Anti-Zerg): Khi có $\ge 4$ người tham chiến, Boss giảm 30% thời gian bị khống chế/đẩy lùi; khi $\ge 8$ người, giảm 50% và tăng tốc độ xoay chuyển hướng đòn đánh.

---

#### 4. Quy Tắc Tử Trận & Vết Tro Tàn (Death Penalty & Corpse Run — ADR-0001)
Để loại bỏ tình trạng "phạt kép" gây ức chế tiêu cực cho nạn nhân bị PK, cơ chế tử trận được chia thành 3 kịch bản nghiêm ngặt:

1. **Chết do Quái vật / Môi trường (PvE Death)**:
   * Người chơi đánh rơi **50% số Vàng** đang mang và **100% Tàn Trang Kỹ Năng** (`item_skill_shard`) tại vị trí tử nạn dưới dạng một **Vệt Tro Tàn (Ash Remnant)**.
   * Toàn bộ Trang Bị và Sách Kỹ Năng được bảo toàn 100%.
   * **Khóa Quyền Sở Hữu (Owner Lock)**: Vệt Tro Tàn được khóa độc quyền cho chính chủ trong **30 phút**. Người chơi khác không thể nhặt hôi tro tàn của bạn.
   * Người chơi hồi sinh tại Lửa Trại gần nhất. Nếu chết lần thứ 2 trước khi chạm vào Vệt Tro Tàn cũ, vệt cũ sẽ tan biến vĩnh viễn (Soulslike mechanic).
2. **Nạn Nhân Vô Tội Bị PK (Victim PvP Death)**:
   * Người chơi mang tên Trắng/Xanh (Karma $\ge 0$) bị kẻ khác hạ sát: **CHỈ MẤT 25% VÀNG** (chuyển thẳng vào túi của kẻ giết người làm chiến lợi phẩm).
   * **Bảo toàn 100% Tàn Trang Kỹ Năng (`item_skill_shard`) và 100% Trang Bị**.
   * Không sinh ra Vệt Tro Tàn (nạn nhân không phải chạy lại nhặt xác trong ức chế).
   * Được lựa chọn hồi sinh an toàn ngay tại Lửa Trại gần nhất.
3. **Kẻ Bị Truy Nã (Wanted Outlaw / Red Name) Bị Tiêu Diệt**:
   * Kẻ có Karma $< -50$ khi bị Vệ Binh hoặc Thợ Săn Tiền Thưởng tiêu diệt:
     * Đánh rơi **100% Vàng** và **100% Tàn Trang** hoàn toàn tự do trên mặt đất (**Free-For-All**, ai cũng có thể nhặt).
     * **Trang bị đang mặc trên người (Equipped Gear) được BẢO TOÀN 100%** (không bao giờ rơi để bảo vệ công sức rèn đúc).
     * Có **15% tỷ lệ đánh rơi 1 món trang bị ngẫu nhiên trong ba lô (Inventory Bag)** cho người kết liễu nhặt.
     * Bị áp giải cưỡng chế vào **Ngục Tối Khổ Sai (Labor Prison)**: Ngồi tù **5 phút thời gian thực**, HOẶC có thể chủ động cầm cuốc đập đủ **20 khối đá quặng trong ngục** để được ân xá ra sớm.
     * Khi mãn hạn tù, Karma được điều chỉnh về mốc $-49$ (xóa danh hiệu Red Name, chuyển về Orange Name).

---

#### 5. Cơ Chế Ranh Giới Quái Vật (AI Leash Boundary & Aggro Leash)
1. **Bán Kính Truy Đuổi Tối Đa (Max Leash Distance = 2500 cm)**:
   * Mỗi quái vật dã ngoại có tọa độ điểm sinh ban đầu (Spawn Origin). Nếu bị người chơi kéo xa quá 2500 cm tính từ điểm sinh, hoặc khi người chơi bước vào Sanctuary Radius của Lửa Trại:
     * Quái vật lập tức nhận thẻ trạng thái Bất Khả Xâm Phạm (`State.Invulnerable`), xóa toàn bộ bảng Aggro.
     * Quái vật lập tức quay đầu chạy về Spawn Origin với tốc độ $+50\%$ Movement Speed.
     * Trong suốt quá trình rút lui, quái vật không nhận bất kỳ sát thương nào và hồi phục đầy 100% Max HP / Posture khi về tới tổ.
   * Cơ chế này triệt tiêu hoàn toàn thủ thuật kéo quái cấp cao hoặc Boss thế giới về tàn sát người chơi mới tại Thị Trấn Tân Thủ.

---

### States and Transitions

#### Bảng Trạng Thái Lửa Trại (Campfire State Machine)

| Trạng Thái | Điều Kiện Vào (Entry Condition) | Điều Kiện Thoát (Exit Condition) | Hành Vi & Quyền Năng |
| :--- | :--- | :--- | :--- |
| **Dormant (Đã Tắt)** | Trạng thái mặc định của thế giới khi người chơi chưa khám phá tới. | Người chơi tiếp cận bấm tương tác thắp lửa (Hold E 1.5s). | Phát cột khói lam mảnh cao 3000 cm lên không trung; không cung cấp kết giới an toàn hay dịch chuyển. |
| **Active (Đã Kích Hoạt)** | Hoàn tất tương tác thắp lửa lần đầu. | Bấm phím ngồi tĩnh dưỡng (Rest) hoặc rời xa quá bán kính Sanctuary. | Kết giới Sanctuary 1000 cm kích hoạt; mở khóa tọa độ lên bản đồ; cho phép làm điểm hồi sinh và đích đến Dịch Chuyển Nhanh. |
| **Resting (Đang Nghỉ Ngơi)** | Người chơi bấm tương tác "Tĩnh Dưỡng" khi đang ở trạng thái Active. | Người chơi đứng dậy hoặc đóng menu Lửa Trại. | Hồi đầy 100% HP/Mana/Stamina/Flasks; respawn quái thường toàn vùng; mở giao diện đổi Class & Action Deck Loadout. |
| **Fast Traveling (Đang Niệm Dịch Chuyển)** | Người chơi chọn 1 Lửa Trại đích trên bản đồ và bấm "Dịch Chuyển". | Niệm chú thành công sau 2.0s HOẶC bị quái/người chơi ngắt chiêu. | Kích hoạt hiệu ứng biến mất; tải dữ liệu World Partition và dịch chuyển nhân vật tới Lửa Trại đích đến. |

---

#### Bảng Trạng Thái Danh Dự Người Chơi (LitRPG Karma State Machine)

| Danh Hiệu | Thang Điểm Karma | Màu Tên Hiển Thị | Phản Ứng Của NPC Vệ Binh | Quyền Lợi & Hậu Quả |
| :--- | :---: | :---: | :--- | :--- |
| **Chính Nghĩa (Righteous)** | $+50$ đến $+100$ | **Xanh Lá Cây (Green)** | Tôn kính; giảm 10% phí sửa đồ thợ rèn. | Nhận thưởng thêm danh vọng khi săn kẻ Bị Truy Nã; tự do ra vào thị trấn. |
| **Trung Lập (Neutral / Innocent)** | $0$ đến $+49$ | **Trắng (White)** | Bình thường; chào đón giao dịch. | Trạng thái mặc định của người chơi; được luật pháp bảo vệ khi bị PK. |
| **Kẻ Tấn Công (Aggressor)** | Tạm thời (120s) | **Xám (Gray)** | Cảnh giác; cấm vào thị trấn trong thời gian đếm ngược. | Kích hoạt khi chủ động đánh người vô tội trước; đối phương tự vệ đánh trả không bị tính là phạm pháp. |
| **Kẻ Tội Đồ (Criminal)** | $-1$ đến $-49$ | **Vàng Cam (Orange)** | Xua đuổi; không phục vụ giao dịch. | Bị trừ Karma sau khi giết người; Vệ Binh từ chối cho vào các khu nhà chính. |
| **Bị Truy Nã (Wanted Outlaw)** | $-50$ đến $-100$ | **Đỏ Thẫm (Red Name)** | **Truy sát tiêu diệt ngay lập tức** (KOS - Kill On Sight). | Tên hiển thị đầu lâu đỏ; vị trí bị ping định kỳ trên minimap server; thợ săn tiền thưởng có quyền giết nhận thưởng; chết rơi 100% Vàng/Tàn Trang, 15% rơi đồ ba lô, đi tù khổ sai 5 phút (hoặc đập 20 quặng). |

---

### Interactions with Other Systems

| Hệ Thống Liên Quan | Luồng Dữ Liệu Vào (Data In) | Luồng Dữ Liệu Ra (Data Out) | Trách Nhiệm Giao Diện |
| :--- | :--- | :--- | :--- |
| **`isometric-controller.md`** | Tọa độ nhân vật, góc ngắm, hướng di chuyển 8 hướng. | Tín hiệu dịch chuyển tức thời (Teleport transform), áp dụng Dither Opacity Mask lên tán cây/vách đá chắn camera. | Controller quản lý camera arm 1200 cm; Zone System kích hoạt Stream Grid và Dither Occlusion. |
| **`attributes-system.md`** | Chỉ số Max HP/Mana/Stamina hiện tại của nhân vật. | GAS GameplayEffect `GE_CampfireRestoration` hồi đầy 100% tài nguyên; xử lý chết và respawn. | Zone System phát trigger; Attributes System áp dụng GameplayEffect. |
| **`skill-progression-system.md`** | Danh sách các kỹ năng Active/Passive đã học của nhân vật. | Cập nhật Action Deck Loadout mới (4 Active + 3 Passive) khi người chơi cấu hình tại Lửa Trại. | Skill System quản lý dữ liệu kỹ năng; Zone System cung cấp ngữ cảnh cho phép đổi chiêu. |
| **`foundational-classes.md`** | Danh sách 12 Chức nghiệp đã được người chơi mở khóa. | Hoán đổi Class ID hiện tại của nhân vật tại Lửa Trại. | Class System cập nhật bộ thuộc tính cơ sở và passive nội tại; Zone System cung cấp trạm hoán đổi. |
| **`blacksmithing-system.md`** | Cấp độ Lò Rèn (Tier 1, Tier 2, Tier 3) gắn với tọa độ Lửa Trại tương ứng. | Kích hoạt menu rèn trang bị, đục lỗ ngọc hoặc đúc Thần Binh từ Linh Hồn Boss. | Blacksmithing System quản lý công thức và tỷ lệ; Zone System quản lý vị trí NPC dã ngoại. |
| **`inventory-system.md`** | Số lượng Vàng, Tàn Trang và Trang Bị hiện có trong kho đồ. | Trừ 50% Vàng / 100% Tàn Trang (PvE) tạo Vệt Tro Tàn; nạp đầy số lần dùng Bình Dược Phẩm khi nghỉ. | Inventory System trừ/cộng vật phẩm; Zone System sinh thực thể Vệt Tro Tàn ngoài thế giới. |

## Formulas

### 1. Công Thức Cân Bằng Máu Boss Động (Boss Dynamic HP Scaling)

The `boss_dynamic_hp_scaling` formula is defined as:

`scaled_max_hp = round(base_max_hp * (1.0 + hp_scale_coefficient * max(0, combatant_count - 1)))`

**Variables:**
| Variable | Symbol | Type | Range | Description |
| :--- | :---: | :---: | :---: | :--- |
| `base_max_hp` | $HP_{base}$ | int | $5,000$ – $100,000$ | Lượng máu gốc của Boss khi solo 1 người (định nghĩa trong DataTable). |
| `hp_scale_coefficient` | $K_{hp}$ | float | $0.30$ – $0.70$ (Chuẩn: $0.50$) | Hệ số gia tăng máu trên mỗi người chơi tăng thêm trong vùng chiến sự. |
| `combatant_count` | $N_{comb}$ | int | $1$ – $50$ | Số người chơi đang gây sát thương/nằm trong bán kính 3000 cm của Boss. |

**Output Range:** $5,000$ đến $1,325,000$ HP (với Boss 100k máu và 25 người cùng tham chiến).
**Example:** Với Boss mẫu Ironclad Warlord có $HP_{base} = 10,000$. Khi có $N_{comb} = 4$ người tham chiến:  
`scaled_max_hp = round(10000 * (1.0 + 0.50 * (4 - 1))) = round(10000 * 2.50) = 25,000 HP`.

---

### 2. Công Thức Cân Bằng Posture Boss Động (Boss Dynamic Posture Scaling)

The `boss_dynamic_posture_scaling` formula is defined as:

`scaled_max_posture = round(base_max_posture * (1.0 + posture_scale_coefficient * max(0, combatant_count - 1)))`

**Variables:**
| Variable | Symbol | Type | Range | Description |
| :--- | :---: | :---: | :---: | :--- |
| `base_max_posture` | $P_{base}$ | float | $400.0$ – $2,000.0$ | Lượng Posture gốc của Boss (Chuẩn: $800.0$ điểm từ `boss-ai.md`). |
| `posture_scale_coefficient` | $K_{posture}$ | float | $0.20$ – $0.50$ (Chuẩn: $0.35$) | Hệ số gia tăng Posture trên mỗi người chơi tăng thêm. |
| `combatant_count` | $N_{comb}$ | int | $1$ – $50$ | Số người chơi đang tham chiến trong bán kính 3000 cm của Boss. |

**Output Range:** $400.0$ đến $18,150.0$ điểm Posture.
**Example:** Với Boss có $P_{base} = 800.0$. Khi có $4$ người chơi cùng tấn công:  
`scaled_max_posture = round(800.0 * (1.0 + 0.35 * 3)) = round(800.0 * 2.05) = 1,640.0` điểm.

---

### 3. Công Thức Chia Điểm Kinh Nghiệm Tổ Đội (Party EXP Share Formula)

The `party_exp_share` formula is defined as:

`exp_per_member = round((base_monster_exp * (1.0 + morale_bonus)) / valid_party_member_count)`

**Variables:**
| Variable | Symbol | Type | Range | Description |
| :--- | :---: | :---: | :---: | :--- |
| `base_monster_exp` | $EXP_{base}$ | int | $50$ – $50,000$ | Điểm kinh nghiệm gốc của quái vật hoặc Boss khi bị tiêu diệt. |
| `morale_bonus` | $B_{morale}$ | float | $0.00$ – $0.15$ | Bùa lợi Ý Chí Đồng Đội (2 người: $+0.05$, 3 người: $+0.10$, 4 người: $+0.15$). |
| `valid_party_member_count` | $M_{valid}$ | int | $1$ – $4$ | Số thành viên tổ đội đứng trong bán kính hỗ trợ hợp lệ ($2500\text{ cm}$). |

**Output Range:** $13$ đến $57,500$ EXP mỗi người.
**Example:** Diệt Boss dã ngoại cho $EXP_{base} = 10,000$. Tổ đội 4 người đứng đủ trong bán kính ($B_{morale} = 0.15$, $M_{valid} = 4$):  
`exp_per_member = round((10000 * 1.15) / 4) = round(11500 / 4) = 2,875 EXP` cho mỗi thành viên (tổng EXP nhóm nhận là $11,500$ — có lợi hơn đi solo).

---

### 4. Công Thức Tổn Thất Vàng Khi Tử Trận PvE (PvE Death Gold Loss)

The `pve_death_gold_loss` formula is defined as:

`lost_gold = floor(current_gold * pve_gold_loss_ratio)`

**Variables:**
| Variable | Symbol | Type | Range | Description |
| :--- | :---: | :---: | :---: | :--- |
| `current_gold` | $G_{curr}$ | int | $0$ – $9,999,999$ | Số Vàng nhân vật đang mang trên người tại thời điểm tử trận. |
| `pve_gold_loss_ratio` | $R_{pve}$ | float | $0.50$ (Cố định) | Tỷ lệ Vàng đánh rơi vào Vệt Tro Tàn khi chết do Quái/Môi trường. |

**Output Range:** $0$ đến $4,999,999$ Vàng rơi vào Vệt Tro Tàn (chính chủ có thể nhặt lại trong 30 phút).
**Example:** Người chơi mang $1,500$ Vàng bị quái đánh chết:  
`lost_gold = floor(1500 * 0.50) = 750 Vàng`. Nhân vật giữ lại $750$ Vàng, $750$ Vàng nằm trong Vệt Tro Tàn.

---

### 5. Công Thức Chiến Lợi Phẩm Khi Nạn Nhân Bị PK (PvP Victim Gold Loss)

The `pvp_victim_gold_loss` formula is defined as:

`pk_bounty_gold = floor(victim_current_gold * pvp_gold_steal_ratio)`

**Variables:**
| Variable | Symbol | Type | Range | Description |
| :--- | :---: | :---: | :---: | :--- |
| `victim_current_gold` | $G_{victim}$ | int | $0$ – $9,999,999$ | Số Vàng nạn nhân mang theo tại thời điểm bị kẻ khác hạ sát. |
| `pvp_gold_steal_ratio` | $R_{pvp}$ | float | $0.25$ (Cố định) | Tỷ lệ Vàng nạn nhân bị mất và chuyển thẳng vào túi kẻ PK. |

**Output Range:** $0$ đến $2,499,999$ Vàng chuyển cho kẻ PK.
**Example:** Nạn nhân mang $2,000$ Vàng bị PK:  
`pk_bounty_gold = floor(2000 * 0.25) = 500 Vàng`. Nạn nhân giữ lại $1,500$ Vàng, kẻ PK nhận $500$ Vàng. Nạn nhân không mất bất kỳ Tàn Trang hay Trang Bị nào.

## Edge Cases

- **Nếu người chơi ngắt kết nối mạng hoặc cố tình tắt game (Disconnect / Combat Alt-F4) khi đang giao tranh ngoài hoang dã**: Nhân vật được máy chủ duy trì dưới dạng "Hình nhân thế mạng" (`Server_CombatGhost`) tại vị trí hiện tại trong **15.0 giây** trước khi biến mất hoàn toàn. Nếu bị quái vật hoặc người chơi khác tiêu diệt trong 15.0s này, xử lý tử trận bình thường theo quy tắc PvE hoặc PvP tương ứng. Nếu sống sót sau 15.0s, nhân vật tự động được lưu tọa độ an toàn và đưa về Lửa Trại gần nhất khi đăng nhập lại (triệt tiêu hoàn toàn thủ thuật Alt-F4 trốn chết hoặc trốn thợ săn tiền thưởng).
- **Nếu người chơi cố tình kéo Boss thế giới hoặc quái tinh anh vượt cấp về cổng Thị Trấn Tiền Trạm**: Ngay khi quái vật chạm vào ranh giới Sanctuary (1000 cm) của Thị Trấn, dàn Vệ Binh NPC Cấp 50 lập tức kích hoạt kỹ năng *Thần Uy Trấn Áp* gây $50\%$ Max HP sát thương chuẩn mỗi đòn lên quái vật; đồng thời quái vật bị cưỡng chế nhận thẻ `State.Invulnerable`, xóa sạch bảng Aggro và bị dịch chuyển tức thời (teleport) về lại Spawn Origin với 100% HP.
- **Nếu người chơi tử trận lần thứ hai khi Vệt Tro Tàn cũ (Ash Remnant) chưa được thu hồi**: Vệt Tro Tàn cũ lập tức tan biến vĩnh viễn vào không gian (toàn bộ 50% Vàng và 100% Tàn Trang cũ bị mất vĩnh viễn); máy chủ sinh ra một Vệt Tro Tàn mới tại vị trí vừa chết chứa lượng Vàng và Tàn Trang đánh rơi của lần chết thứ hai.
- **Nếu người chơi bị quái vật tấn công khi đang cố gắng thắp sáng Lửa Trại (Dormant -> Active)**: Quá trình tương tác thắp lửa yêu cầu giữ phím E trong $1.5\text{s}$. Bất kỳ sát thương hay hiệu ứng khống chế nào trúng phải nhân vật sẽ lập tức ngắt quá trình thắp lửa, buộc người chơi phải dọn sạch quái hoặc lướt né ra xa trước khi thử lại.
- **Nếu người chơi đang niệm Dịch Chuyển Nhanh (Fast Travel 2.0s) mà bị tấn công**: Thanh niệm chú lập tức bị hủy bỏ, đưa nhân vật vào trạng thái Giao Tranh (`State.InCombat`), đồng thời khóa hoàn toàn chức năng Dịch Chuyển Nhanh trong **10.0 giây** tính từ đòn đánh cuối cùng.
- **Nếu Kẻ Bị Truy Nã (Wanted Outlaw / Red Name) liều lĩnh xông vào phạm vi Thánh Địa / Thị Trấn**: Tháp Canh và Vệ Binh Thánh Địa tự động phóng ra các tia sét Thần Phạt tầm xa $1500\text{ cm}$ gây $20\%$ Max HP sát thương chuẩn mỗi giây, bỏ qua giáp và cấm hồi phục, tiêu diệt kẻ thủ ác tại chỗ và áp giải vào Ngục Tối Khổ Sai 5 phút.
- **Nếu Boss thế giới bị bẻ gãy Posture (Stagger) nhưng người tung đòn Finisher bị tử trận hoặc bị khống chế cứng trong 1.5s đầu**: Cửa sổ ưu tiên 1.5s lập tức kết thúc sớm; nút bấm Execution rút 25% Max HP được kích hoạt mở tự do ngay lập tức cho bất kỳ người chơi tham chiến nào còn sống trong bán kính 1000 cm.
- **Nếu nhiều tổ đội cùng tham gia hạ sát Boss và tất cả đều đạt ngưỡng đóng góp $\ge 5\%$**: Máy chủ ghi nhận thành tích cho từng tổ đội độc lập; toàn bộ thành viên hợp lệ của mọi tổ đội đều nhận hòm đồ cá nhân riêng biệt (Instanced Loot) tại màn hình tổng kết mà không có bất kỳ xung đột tài nguyên nào.

## Dependencies

### 1. Hệ Thống Thượng Nguồn (Upstream Dependencies)

| Hệ Thống Thượng Nguồn | Bản Tả Thiết Kế (GDD) | Bản Chất Phụ Thuộc | Dữ Liệu & Giao Diện Tiếp Nhận |
| :--- | :--- | :---: | :--- |
| **Input & Isometric Controller** | [`isometric-controller.md`](file:///mnt/Data/Projects/project-games/design/gdd/isometric-controller.md) | **Bắt Buộc (Hard)** | Nhận tọa độ vị trí nhân vật, góc quay camera SpringArm 1200 cm, độ mở rộng tầm nhìn Lookahead 250 cm; cung cấp tín hiệu kích hoạt Dither Opacity Mask lên các vách đá/tán cây che khuất camera. |
| **Attributes & Stats (GAS)** | [`attributes-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/attributes-system.md) | **Bắt Buộc (Hard)** | Nhận các thuộc tính sinh tồn (HP, Mana, Stamina) để thực thi hồi phục trọn vẹn qua `GE_CampfireRestoration` khi nghỉ tại Lửa Trại; tiếp nhận sự kiện tử trận để kích hoạt quy trình rơi Vàng/Tàn Trang và hồi sinh. |
| **Kiến Trúc Mạng & Combat MMO** | [`ADR-0001`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md) | **Bắt Buộc (Hard)** | Ràng buộc 100% logic máy chủ (Server Authority); cung cấp công thức Dynamic Difficulty Scaling, quy tắc Instanced Loot ngưỡng 5%, cửa sổ ưu tiên Stagger Execution 1.5s, và cờ trạng thái Karma/Wanted. |
| **Core Combat & Combo** | [`combat-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/combat-system.md) | **Tương Thích (Soft)** | Xác định trạng thái Vào Giao Tranh (`State.InCombat`) để khóa các tính năng Dịch Chuyển Nhanh (Fast Travel) và chuyển đổi Chức nghiệp. |

---

### 2. Hệ Thống Hạ Nguồn (Downstream Dependents)

| Hệ Thống Hạ Nguồn | Bản Tả Thiết Kế (GDD) | Bản Chất Phụ Thuộc | Kỳ Vọng Zone System Cung Cấp |
| :--- | :--- | :---: | :--- |
| **Zone-tiered Blacksmithing** | [`blacksmithing-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/blacksmithing-system.md) | **Bắt Buộc (Hard)** | Cung cấp vị trí địa lý cố định và bối cảnh an toàn cho 3 cấp lò rèn dã ngoại: Thợ Rèn Tiền Trạm (Tier 1), Thợ Rèn Dã Ngoại (Tier 2), và Lò Rèn Cấm Địa Thần Tích (Tier 3). |
| **Merchant & Currency Economy** | `design/gdd/merchant-economy.md` | **Bắt Buộc (Hard)** | Cung cấp các điểm đỗ an toàn cho Thương Nhân Tiền Trạm, Thương Nhân Lang Thang dã ngoại và Chợ Đen bí mật tại Cấm Địa; áp dụng thuế suất hoặc danh mục hàng hóa theo từng bậc vùng đất. |
| **Open World MMO Netcode** | `design/gdd/multiplayer-coop.md` | **Bắt Buộc (Hard)** | Cung cấp lưới tọa độ World Partition, ranh giới Leash Range của quái vật dã ngoại, và dữ liệu lưu trữ trạng thái Lửa Trại của từng người chơi trên máy chủ. |
| **World Boss Raids** | `design/gdd/world-bosses.md` | **Bắt Buộc (Hard)** | Cung cấp đấu trường mở không rào chắn tại Vùng 3 (Cấm Địa Thần Tích) và tích hợp công thức Dynamic HP / Posture Scaling khi hàng chục người cùng tham chiến. |

## Tuning Knobs

Bảng tổng hợp các biến số thiết kế được cấu hình qua `DataAsset` hoặc `DataTable` trong Unreal Engine 5, cho phép Game Designer cân chỉnh trực tiếp nhịp độ thế giới và độ khó mà không cần lập trình viên biên dịch lại mã nguồn C++:

| Tên Biến Số (Tuning Knob) | Kiểu Dữ Liệu | Giá Trị Mặc Định | Dải An Toàn (Min – Max) | Hậu Quả Nếu Quá Thấp | Hậu Quả Nếu Quá Cao |
| :--- | :---: | :---: | :---: | :--- | :--- |
| `campfire_sanctuary_radius` | float | `1000.0 cm` | $600.0$ – $1500.0\text{ cm}$ | Quái vật đứng quá sát Lửa Trại, bắn tỉa người chơi đang nghỉ ngơi. | Khu vực an toàn quá rộng làm triệt tiêu sự căng thẳng khi bị rượt đuổi dã ngoại. |
| `campfire_ignite_interaction_time` | float | `1.5 s` | $1.0$ – $3.0\text{ s}$ | Thắp lửa quá nhanh làm mất tính hồi hộp khi đang bị quái truy đuổi. | Quá lâu khiến người chơi hầu như không thể thắp lửa nếu có quái tầm xa. |
| `fast_travel_cast_time` | float | `2.0 s` | $1.0$ – $5.0\text{ s}$ | Dễ bị lợi dụng để trốn chạy ngay trước mắt kẻ địch hoặc đối thủ PK. | Gây sốt ruột và cảm giác ì ạch khi dịch chuyển giữa các trạm an toàn. |
| `ai_leash_max_distance` | float | `2500.0 cm` | $1500.0$ – $4000.0\text{ cm}$ | Quái vừa đuổi 1 đoạn ngắn đã quay đầu, người chơi tầm xa dễ dàng cấu rỉa thả diều (kite). | Dễ bị người chơi kéo quái cấp cao qua nhiều vùng địa hình gây nhiễu loạn server. |
| `combat_disconnect_ghost_duration`| float | `15.0 s` | $10.0$ – $30.0\text{ s}$ | Người chơi lợi dụng rút dây mạng/Alt-F4 để thoát hiểm khi sắp chết. | Bất công cho người chơi bị rớt mạng thật sự do đường truyền Internet kém. |
| `boss_loot_contribution_hp` | float | `0.05` (5%) | $0.01$ – $0.15$ | Tạo điều kiện cho nạn bot/ăn hôi chỉ quẹt nhẹ rồi đứng ngoài nhận thưởng. | Quá khắt khe đối với người chơi hỗ trợ (Acolyte) hoặc người mới vào trận trễ. |
| `boss_loot_contribution_posture` | float | `0.10` (10%) | $0.05$ – $0.20$ | Class chuyên đỡ đòn/phản đòn (Vanguard) bị thiệt thòi nếu Boss chết nhanh. | Ép buộc mọi người phải dồn toàn lực vào Posture thay vì phối hợp đa dạng. |
| `boss_finisher_exclusive_window` | float | `1.5 s` | $1.0$ – $2.5\text{ s}$ | Finisher không kịp tiếp cận Boss nếu đang ở cự ly xa (bị hụt quyền kết liễu). | Thời gian chờ quá lâu làm ngắt quãng nhịp độ phối hợp dồn đòn đánh của đồng đội. |
| `wanted_prison_duration_seconds` | int | `300 s` (5 min) | $180$ – $600\text{ s}$ | Không đủ tính răn đe, kẻ xấu sẵn sàng bị bắt rồi ra quậy phá tiếp. | Gây ức chế tiêu cực cực đoan khiến người chơi phạm luật thoát game bỏ chơi. |
| `wanted_prison_mine_ore_target` | int | `20 khối` | $10$ – $50\text{ khối}$ | Lao động quá nhanh, coi thường án phạt ngục tù. | Biến nhiệm vụ khổ sai thành cực hình nhàm chán lặp đi lặp lại. |
| `ash_remnant_owner_lock_duration` | int | `1800 s` (30 min)| $600$ – $3600\text{ s}$ | Thời gian quá ngắn khiến người chơi ở xa không kịp chạy bộ về nhặt lại xác. | Dữ liệu Vệt Tro Tàn lưu lại quá lâu trên máy chủ làm tăng tải bộ nhớ RAM thế giới. |
| `party_share_exp_radius` | float | `2500.0 cm` | $1500.0$ – $4000.0\text{ cm}$ | Thành viên đứng cách nhau một góc màn hình đã bị mất quyền nhận EXP. | Khuyến khích thành viên cắm máy AFK ở góc xa ăn bám tổ đội cày cấp. |

## Visual/Audio Requirements

### 1. Hiệu Ứng Hình Ảnh (VFX — Niagara)
* **Cột Khói Lửa Trại (Smoke Beacon — Niagara System):**
  * Mỗi Lửa Trại chưa mở (Dormant) liên tục phóng một cột khói lam nhạt mảnh mai vươn cao $3000\text{ cm}$ lên bầu trời, xuyên qua lớp sương mù chiến tranh (Fog of War) để người chơi có thể định hướng tìm kiếm từ góc nhìn Isometric bao quát.
  * Khi người chơi thắp lửa thành công: Hiệu ứng tia lửa bùng nổ (`NS_CampfireIgnite_Burst`), chuyển ngọn lửa sang sắc vàng cam ấm áp và trải một vòng hào quang ánh sáng mềm mại trên mặt đất bao phủ trọn vẹn bán kính Thánh Địa $1000\text{ cm}$.
* **Vệt Tro Tàn (Ash Remnant VFX):**
  * Điểm chết PvE sinh ra một đốm tro tàn phát ánh sáng vàng hổ phách nhấp nháy, được bao quanh bởi một kết giới mờ biểu thị trạng thái "Khóa Chủ Sở Hữu" (Owner Lock).
  * Điểm chết của Kẻ Bị Truy Nã (Wanted Red Name) phát ra làn khói đen tà ác sủi bọt, không có kết giới bao quanh (báo hiệu cho phép nhặt tự do).
* **Dither Occlusion Mask (Xử Lý Che Khuất Tầm Nhìn):**
  * Khi nhân vật hoặc Lửa Trại nằm dưới tán cây cổ thụ hoặc sau vách núi đá chắn tầm nhìn camera Isometric, vật liệu môi trường tự động áp dụng hiệu ứng mờ bán phần (Dither Transparency Mask) trong bán kính $300\text{ cm}$ xung quanh nhân vật, bảo đảm tầm quan sát không bao giờ bị điểm mù.

### 2. Thiết Kế Âm Thanh (SFX & Audio Stems)
* **Âm Thanh Lửa Trại (Campfire Audio):**
  * Tiếng quẹt đá lửa 'XOẸT! BÙNG!' đanh gọn khi tương tác thắp sáng; tiếng củi nổ lách tách ấm áp và tiếng gió rít nhẹ bên ngoài ranh giới an toàn.
  * Ngay khi bước vào kết giới Thánh Địa, nhạc nền chiến đấu căng thẳng tự động fade-out (0.8s) nhường chỗ cho giai điệu vĩ cầm và sáo trúc mộc mạc, bình yên của chủ đề "Ốc Đảo Sinh Tồn".
* **Âm Thanh Cảnh Báo Wanted & Vệ Binh:**
  * Khi một Kẻ Bị Truy Nã (Red Name) tiến vào phạm vi $2000\text{ cm}$ của người chơi, một âm thanh chuông ngân rùng rợn (`SFX_Wanted_Proximity_Warning`) vang lên cảnh báo nguy hiểm.
  * Tiếng gươm giáo va đập và tiếng tù và thị uy đanh thép của Vệ Binh Trấn Thủ khi quái vật hoặc tội phạm áp sát cổng trấn.

---

## UI Requirements

* **Hệ Thống Bản Đồ Thế Giới & Bản Đồ Mini (World Map & Minimap):**
  * Sương mù chiến tranh (Fog of War) che phủ địa hình, tự động giải phóng trong bán kính $1500\text{ cm}$ khi người chơi thám hiểm qua.
  * Biểu tượng Lửa Trại hiển thị trực quan: Đốm xám tro (chưa thắp sáng), Ngọn lửa vàng rực (đã kích hoạt), Ngọn lửa xanh (Lửa Trại hiện đang tĩnh dưỡng).
  * Cho phép bấm trực tiếp vào biểu tượng Lửa Trại đã kích hoạt trên bản đồ để bắt đầu niệm Dịch Chuyển Nhanh.
* **Thanh Trạng Thái Danh Dự & Tên Nhân Vật (Karma HUD):**
  * Tên người chơi đổi màu động theo 5 bậc: Xanh Lá (Chính Nghĩa) $\rightarrow$ Trắng (Trung Lập) $\rightarrow$ Xám (Kẻ Tấn Công) $\rightarrow$ Cam (Tội Đồ) $\rightarrow$ Đỏ Thẫm (Kẻ Bị Truy Nã).
  * Kẻ Bị Truy Nã (Red Name) hiển thị biểu tượng Đầu Lâu Đỏ bốc cháy bên cạnh thanh máu; vị trí tọa độ vùng của họ được phát sóng (ping) định kỳ 30s một lần lên Minimap toàn server.
  * Thông Báo Toàn Server (Global Banner): Khi một Kẻ Bị Truy Nã khét tiếng bị Thợ Săn Tiền Thưởng hạ sát, một thông báo vinh danh xuất hiện trên đầu màn hình kèm số Vàng thưởng được chi trả.
* **Cửa Sổ Kết Liễu Stagger (Stagger Execution UI):**
  * Khi Boss bị bẻ gãy Posture: Hiển thị icon phím bấm tương tác kết liễu khổng lồ trên đỉnh đầu Boss.
  * Trong $1.5\text{s}$ đầu: Icon viền vàng rực rỡ kèm dòng chữ *"Quyền Kết Liễu Của [Tên Finisher]"* chỉ hiển thị với Finisher.
  * Từ giây thứ $1.5$ đến $3.0\text{s}$: Icon viền bạc nhấp nháy công khai cho toàn bộ người chơi tham chiến.

---

## Acceptance Criteria

Mọi tiêu chí kiểm thử đều được cấu trúc theo định dạng **Given-When-Then** độc lập, cho phép đội ngũ QA kiểm thử và xác thực cơ chế mà không cần phụ thuộc vào mã nguồn:

- **AC-01 (Thế Giới Mở Liền Mạch Không Rào Cản - Non-gated Streaming):**
  - **GIVEN** nhân vật vừa tạo Cấp 1 tại Thị Trấn Tiền Trạm,
  - **WHEN** người chơi điều khiển nhân vật chạy bộ liên tục xuyên qua ranh giới Rừng Sơ Khai sang Hoang Dã Tàn Tích và tiến thẳng vào Cấm Địa Thần Tích,
  - **THEN** nhân vật di chuyển liền mạch không gặp bất kỳ tường vô hình (Invisible Wall) hay màn hình tải cắt cảnh nào; World Partition tải dữ liệu địa hình mượt mà giữ vững $\ge 60\text{ FPS}$.

- **AC-02 (Ranh Giới Thánh Địa & Xóa Aggro - Sanctuary Boundary):**
  - **GIVEN** người chơi đang bị đàn quái cấp 25 rượt đuổi ngoài hoang dã,
  - **WHEN** nhân vật lướt né bước chân qua bán kính $1000\text{ cm}$ của Lửa Trại đã kích hoạt,
  - **THEN** toàn bộ quái vật lập tức dừng truy đuổi tại mép kết giới, nhận thẻ `State.Invulnerable` và quay đầu chạy về vị trí ban đầu; người chơi không nhận thêm bất kỳ sát thương nào.

- **AC-03 (Tĩnh Dưỡng & Tái Sinh Quái - Campfire Rest):**
  - **GIVEN** nhân vật đang còn $20\%$ HP, $0$ Mana, và đã dùng hết 3 bình dược phẩm,
  - **WHEN** người chơi tiếp cận Lửa Trại và bấm chọn "Tĩnh Dưỡng",
  - **THEN** HP, Mana, Stamina và toàn bộ số lần dùng của bình dược phẩm hồi phục $100\%$ ngay lập tức; đồng thời toàn bộ quái vật thường đã tiêu diệt trong vùng được tái tạo (respawn) lại hoàn toàn.

- **AC-04 (Dịch Chuyển Nhanh - Fast Travel):**
  - **GIVEN** người chơi đã thắp sáng Lửa Trại Tiền Trạm và Lửa Trại Hoang Dã,
  - **WHEN** người chơi đứng tại Lửa Trại Tiền Trạm, chọn Lửa Trại Hoang Dã trên bản đồ và niệm chú thành công trong $2.0\text{s}$ mà không bị quái/người chơi khác tấn công,
  - **THEN** nhân vật được dịch chuyển tức thời tới đứng cạnh Lửa Trại Hoang Dã mà không bị trừ bất kỳ chi phí Vàng hay tài nguyên nào.

- **AC-05 (Ngưỡng Đóng Góp Nhận Instanced Loot - Contested Drop):**
  - **GIVEN** một Boss thế giới có $10,000$ HP đang bị 3 người chơi xa lạ cùng tấn công,
  - **WHEN** Người Chơi A gây được $600$ sát thương ($6\%$ HP $\ge 5\%$) và Người Chơi B chỉ đánh 1 đòn gây $100$ sát thương ($1\%$ HP $< 5\%$) trước khi Boss chết,
  - **THEN** máy chủ chỉ sinh hòm Instanced Loot cá nhân riêng biệt cho Người Chơi A; Người Chơi B hoàn toàn không nhận được hòm đồ rơi của Boss.

- **AC-06 (Cân Bằng Posture Boss Động - Dynamic Posture Scaling):**
  - **GIVEN** Boss thế giới có Posture cơ sở $800.0$ điểm,
  - **WHEN** có $4$ người chơi cùng tham chiến trong bán kính $3000\text{ cm}$ của Boss,
  - **THEN** thanh Posture tối đa của Boss tự động tăng lên chính xác $1,640.0$ điểm theo công thức `boss_dynamic_posture_scaling`.

- **AC-07 (Cửa Sổ Ưu Tiên Kết Liễu Cho Finisher - Priority Execution):**
  - **GIVEN** Boss thế giới bị đánh bẻ gãy $100\%$ thanh Posture bởi Người Chơi A (Finisher),
  - **WHEN** cửa sổ Stagger $3.0\text{s}$ bắt đầu và Người Chơi B đứng cạnh bấm tương tác kết liễu tại giây thứ $0.8$,
  - **THEN** lệnh của Người Chơi B bị máy chủ từ chối; chỉ duy nhất Người Chơi A có thể kích hoạt đòn chém Execution rút $25\%$ Max HP của Boss trong $1.5\text{s}$ đầu tiên.

- **AC-08 (Mở Tự Do Quyền Kết Liễu Sau 1.5s - Fallback Execution):**
  - **GIVEN** Boss thế giới bị Stagger nhưng Người Chơi A (Finisher) không bấm kết liễu trong $1.5\text{s}$ đầu,
  - **WHEN** Người Chơi B bấm nút tương tác kết liễu tại giây thứ $1.8$,
  - **THEN** máy chủ chấp thuận lệnh của Người Chơi B, cho phép Người Chơi B thực hiện hoạt ảnh kết liễu rút $25\%$ Max HP của Boss.

- **AC-09 (Tử Trận PvE & Khóa Vệt Tro Tàn - Ash Remnant Owner Lock):**
  - **GIVEN** người chơi mang $1,000$ Vàng và $50$ Tàn Trang (`item_skill_shard`) bị quái vật cắn chết,
  - **WHEN** nhân vật hồi sinh tại Lửa Trại,
  - **THEN** nhân vật giữ lại $500$ Vàng; tại vị trí chết xuất hiện Vệt Tro Tàn chứa $500$ Vàng và $50$ Tàn Trang được khóa quyền nhặt độc quyền cho chính chủ trong $30$ phút; người chơi khác đi ngang qua không thể nhặt được.

- **AC-10 (Bảo Vệ Nạn Nhân Bị PK - Victim PvP Protection):**
  - **GIVEN** người chơi mang tên Trắng (Karma $0$) sở hữu $2,000$ Vàng, $100$ Tàn Trang và set trang bị +5 bị một người chơi khác đồ sát (PK),
  - **WHEN** nhân vật tử trận,
  - **THEN** nạn nhân chỉ bị trừ $500$ Vàng ($25\%$), bảo toàn $100$ Tàn Trang và toàn bộ trang bị +5; không sinh ra Vệt Tro Tàn ngoài thế giới; kẻ PK nhận $500$ Vàng và bị trừ $-30$ Karma.

- **AC-11 (Trừng Phạt Kẻ Bị Truy Nã - Wanted Outlaw Penalties):**
  - **GIVEN** người chơi có Karma $-60$ (Tên Đỏ Thẫm / Wanted Outlaw) bị Thợ Săn Tiền Thưởng hạ sát,
  - **WHEN** nhân vật tử trận,
  - **THEN** kẻ thủ ác đánh rơi $100\%$ Vàng và $100\%$ Tàn Trang công khai ra đất, có $15\%$ xác suất đánh rơi $1$ món đồ trong ba lô; đồng thời nhân vật bị cưỡng chế chuyển sinh vào Ngục Tối Khổ Sai trong $5$ phút hoặc cho đến khi đập đủ $20$ quặng ngục.

- **AC-12 (Ranh Giới Truy Đuổi Của Quái - AI Leash Limit):**
  - **GIVEN** quái vật dã ngoại có điểm sinh tại tọa độ $(X_0, Y_0)$,
  - **WHEN** người chơi dẫn dụ (kite) quái vật chạy xa quá $2500\text{ cm}$ tính từ điểm sinh,
  - **THEN** quái vật lập tức nhận trạng thái `State.Invulnerable`, hủy mọi đòn đánh đang thi triển và quay đầu chạy thẳng về $(X_0, Y_0)$ với tốc độ tăng thêm $50\%$, hồi đầy $100\%$ máu khi về tới vị trí cũ.

## Open Questions

| STT | Câu Hỏi Mở Thiết Kế | Bộ Phận Phụ Trách | Mục Tiêu Giải Quyết |
| :---: | :--- | :---: | :---: |
| **Q1** | Có nên bổ sung cơ chế Đặt Bẫy Dã Ngoại (Traps & Barricades) bên ngoài Thánh Địa để tổ đội người chơi lập chốt phòng thủ trước khi vào săn World Boss không? | Systems Designer | Alpha Phase |
| **Q2** | Có nên thiết kế một Đường Hầm Vượt Ngục Bí Mật (Secret Escape Tunnel) trong Ngục Tối Khổ Sai với độ khó bẫy nhảy cực hạn dành riêng cho người chơi tay to muốn trốn ngục sớm không? | Level Designer | Alpha Phase |
