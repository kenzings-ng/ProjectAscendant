# Third-Party Assets Provenance & Intake Ledger

> **Document**: `THIRD_PARTY_ASSETS.md`  
> **Project**: Project Ascendant (2.5D Isometric Hardcore ARPG MMO on Unreal Engine 5.8)  
> **Standard**: Provenance Intake Ledger (Modeled after Voidclad Provenance Protocol)  
> **Governing Policy**: Zero-Trust License Verification. Never infer license from repository name. UNKNOWN license is strictly blocked from production. Only copy assets after individual pack provenance is verified.

---

## 1. Audit Summary

| Metric | Count | Note |
| :--- | :---: | :--- |
| **Packs Audited** | **7** | Rà soát chi tiết từng pack/thư mục độc lập |
| **Packs Approved** | **4** | Đầy đủ bằng chứng CC0-1.0, không xung đột lore/kỹ thuật |
| **Packs Rejected** | **2** | 1 do xung đột đề tài (súng hiện đại), 1 do thiếu chứng chỉ pháp lý (UNKNOWN) |
| **Packs Pending** | **1** | Đang chờ thẩm định tương thích góc chiếu Isometric trước khi nạp |
| **Assets Harvested** | **30** | Lưu tại `Content/Art/ThirdParty/CC0_Harvested/` kèm SHA-256 |
| **Art Gate Clearance** | **23 Approved / 7 Rejected** | Thẩm định độc lập: [`art-gate-report-batch-01.md`](production/qa/art-gate-report-batch-01.md) |

---

## 2. Quy Tắc Thẩm Định & Nhập Liệu (Intake Rules)

1. **Intake Ledger, Not License Replacement**: Tập tin này là sổ cái ghi nhận nguồn gốc và bằng chứng tại thời điểm kéo dữ liệu (Pull Time). Mọi asset phải lưu kèm thông tin giấy phép gốc.
2. **Pack-by-Pack Audit**: Không mặc định toàn bộ nội dung trong một repo là CC0. Phải kiểm tra file `LICENSE`, `pack.json` và header cụ thể của từng pack.
3. **Phân Biệt Code vs Media**: Giấy phép mã nguồn (như MIT/Apache-2.0) không áp dụng cho đồ họa (Media). Chỉ chấp nhận Media có giấy phép rõ ràng (ưu tiên tuyệt đối CC0-1.0 Universal).
4. **Quy Tắc UNKNOWN**: Bất kỳ tài nguyên nào có trạng thái bản quyền không rõ ràng (**UNKNOWN**) đều bị coi là **REJECTED** đối với môi trường production.
5. **SHA-256 Checksum**: Mỗi file tải về phải có mã băm SHA-256 đối soát để đảm bảo tính toàn vẹn dữ liệu.

---

## 3. Nhật Ký Thẩm Định Từng Gói (Pack-by-Pack Ledger)

### Pack 01: Dungeon Crawl Stone Soup — Items & Equipment
- **Asset/Pack Name**: `dcss-items-and-equipment`
- **Original URL**: https://crawl.develz.org/ / https://github.com/Papyszoo/CC0-Public-Domain-Sprites/tree/main/packs/dcss-items-and-equipment
- **Source Repository**: `Papyszoo/CC0-Public-Domain-Sprites`
- **Commit / Version**: `e27131731f89187b9842e9c6c95381a2e954aafd` (2026-08-16)
- **License**: **CC0 1.0 Universal (Public Domain Dedication)** — https://creativecommons.org/publicdomain/zero/1.0/
  - *Bằng chứng thẩm định*: File `pack.json` trong thư mục pack ghi rõ `"license": "CC0"`, `"creator": "Dungeon Crawl Stone Soup Team"`. Bản thân dự án DCSS phát hành toàn bộ tileset dưới phạm vi công cộng (CC0).
- **Commercial Redistribution**: **Allowed** (Miễn phí thương mại, không bắt buộc ghi công, được phép chỉnh sửa).
- **Date Audited**: 2026-09-25
- **Intended Use trong Project Ascendant**: Cung cấp sprite $32 \times 32\text{ px}$ chuẩn cho 21 Base Weapons (Kiếm, Rìu, Đao, Giáo, Trượng, Chùy) và Giáp (Bản giáp, Giáp xích, Giáp da, Ủng, Găng) trong hệ thống Itemization (`item-005`, `item-004`).
- **Status**: **APPROVED**
- **Harvested Files (18 assets)**:
  | File Name | Relpath | Size | SHA-256 Checksum |
  | :--- | :--- | :---: | :--- |
  | `dcss_broad_axe.png` | `Content/Art/ThirdParty/CC0_Harvested/Weapons/dcss_broad_axe.png` | 413 B | `087a3c96567b2cbfc99dd86743404841131f81ba551abd690b660391743f7a4d` |
  | `dcss_blessed_blade.png` | `Content/Art/ThirdParty/CC0_Harvested/Weapons/dcss_blessed_blade.png` | 733 B | `c34b93dbac5b5d813bd50628fd1d744deb4ce8688d1969788d4e298f4b670a39` |
  | `dcss_long_sword.png` | `Content/Art/ThirdParty/CC0_Harvested/Weapons/dcss_long_sword.png` | 430 B | `1d2ee0b5e314dafe8f5f385207bf63110bfdc6d39fdf2cae8dc36f78a014e7ab` |
  | `dcss_greatsword.png` | `Content/Art/ThirdParty/CC0_Harvested/Weapons/dcss_greatsword.png` | 568 B | `f280f6f55c8879b57d751448bfb58e177634e04d9ee42addad7e33c54b7467bf` |
  | `dcss_dagger.png` | `Content/Art/ThirdParty/CC0_Harvested/Weapons/dcss_dagger.png` | 338 B | `da6fdcf11df898ab0780b97e8cfc34d2ae0a793c5973ada7b7749d6b711783f4` |
  | `dcss_spear.png` | `Content/Art/ThirdParty/CC0_Harvested/Weapons/dcss_spear.png` | 456 B | `71fda4ac1741b0bdbc1e5a27ad35744b0e444866e685e5bbdba3159bbde990b1` |
  | `dcss_halberd.png` | `Content/Art/ThirdParty/CC0_Harvested/Weapons/dcss_halberd.png` | 354 B | `cc96611f75b8d5d3d246c23da2a949029c76a643ab5d945e25c1aa26c95c1564` |
  | `dcss_quarterstaff.png` | `Content/Art/ThirdParty/CC0_Harvested/Weapons/dcss_quarterstaff.png` | 422 B | `b539ff5b9605a1d83a93a13babb24bdfa2783d9710331c93595181a8b6a722cc` |
  | `dcss_mace.png` | `Content/Art/ThirdParty/CC0_Harvested/Weapons/dcss_mace.png` | 444 B | `43771f02fd1e1a2fba23c29885cc0312c4e824a02acaf062f7549525c3c40ad9` |
  | `dcss_morningstar.png` | `Content/Art/ThirdParty/CC0_Harvested/Weapons/dcss_morningstar.png` | 546 B | `7a5ebb7ead0f681c9866aca312475566417891e6ae0e4187c8b039423bd216bc` |
  | `dcss_plate_mail.png` | `Content/Art/ThirdParty/CC0_Harvested/Armor/dcss_plate_mail.png` | 826 B | `31b23d7433683e0587f47a2ea3bee9405f235e08e7731715c580153ddaa0e188` |
  | `dcss_banded_mail.png` | `Content/Art/ThirdParty/CC0_Harvested/Armor/dcss_banded_mail.png` | 752 B | `cc6c9d35af2162a46f2ff613f99b16e035332391d50f4f92f735c9addd5eb08e` |
  | `dcss_chain_mail.png` | `Content/Art/ThirdParty/CC0_Harvested/Armor/dcss_chain_mail.png` | 1202 B | `3bc0f26408fe0ec0dff50659c1dba795553ef4d4064a842a1971aabc0f52741d` |
  | `dcss_leather_armour.png` | `Content/Art/ThirdParty/CC0_Harvested/Armor/dcss_leather_armour.png` | 656 B | `cab2595c6ab1df239601c02ec79cb8248b46830f831b6cf20e7b76594ce93eae` |
  | `dcss_robe.png` | `Content/Art/ThirdParty/CC0_Harvested/Armor/dcss_robe.png` | 451 B | `7989e17d11e035498bc0b671e0202efa94fe948908554cf2a8fa6a755ba83c34` |
  | `dcss_boots.png` | `Content/Art/ThirdParty/CC0_Harvested/Armor/dcss_boots.png` | 670 B | `aa5551a1eaf0ccdc6a1eae7df75b66ab3909826669a655e3e6497fad9e36e343` |
  | `dcss_gauntlets.png` | `Content/Art/ThirdParty/CC0_Harvested/Armor/dcss_gauntlets.png` | 1045 B | `9dccf44c56426f30192a6f9f701ae2c3417393ad02b8029bf6e76b73a4862ce1` |
  | `dcss_cloak.png` | `Content/Art/ThirdParty/CC0_Harvested/Armor/dcss_cloak.png` | 543 B | `57c505dcd93506c3709038964cae1e8286e00d32f5a09be5b1935d82ebf8b3f8` |

---

### Pack 02: Buch — Animated Potions
- **Asset/Pack Name**: `buch-animated-potions`
- **Original URL**: https://opengameart.org/content/colorful-potions-animated / https://github.com/Papyszoo/CC0-Public-Domain-Sprites/tree/main/packs/buch-animated-potions
- **Source Repository**: `Papyszoo/CC0-Public-Domain-Sprites`
- **Commit / Version**: `e27131731f89187b9842e9c6c95381a2e954aafd` (2026-08-16)
- **License**: **CC0 1.0 Universal**
  - *Bằng chứng thẩm định*: Tác giả Buch đăng tải trực tiếp trên OpenGameArt dưới giấy phép CC0. File `pack.json` xác nhận bản quyền.
- **Commercial Redistribution**: **Allowed**
- **Date Audited**: 2026-09-25
- **Intended Use trong Project Ascendant**: Làm icon bình hồi phục (Máu, Mana, Thể lực, Tẩy điểm) cho khay Quickbar Consumables (`item-001`).
- **Status**: **APPROVED**
- **Harvested Files (4 assets)**:
  | File Name | Relpath | Size | SHA-256 Checksum |
  | :--- | :--- | :---: | :--- |
  | `buch_health_potion.png` | `Content/Art/ThirdParty/CC0_Harvested/Consumables/buch_health_potion.png` | 3471 B | `1850d2ac674b46d867fc0542d023038343b9bf29da5df49b82eb55fc811757af` |
  | `buch_mana_potion.png` | `Content/Art/ThirdParty/CC0_Harvested/Consumables/buch_mana_potion.png` | 3565 B | `b3ef50cadfa65f1d3db4bd91dfa67f4e8e7cb2876741c2affa915dcfea40038c` |
  | `buch_stamina_potion.png` | `Content/Art/ThirdParty/CC0_Harvested/Consumables/buch_stamina_potion.png` | 3488 B | `cc89a30437dad834d1f6e3505f909c0808b3477216ae66dcd457b1a199fccd23` |
  | `buch_flask_round.png` | `Content/Art/ThirdParty/CC0_Harvested/Consumables/buch_flask_round.png` | 3321 B | `78a42c5c182bedd9430e25ea75334fa52f103d3d0f58e5fd6ca5bcb1fdaf0d29` |

---

### Pack 03: Project Cordon — Crafting Materials
- **Asset/Pack Name**: `doficia-project-cordon-materials`
- **Original URL**: https://github.com/doficia/project-cordon-sprites/tree/master/sprites/materials
- **Source Repository**: `doficia/project-cordon-sprites`
- **Commit / Version**: `aa879085c1cd3aea647bf4acb85f7953bc9373cd` (2018-10-14)
- **License**: **CC0 1.0 Universal**
  - *Bằng chứng thẩm định*: Tệp `LICENSE.md` tại root repository xác nhận đóng góp toàn bộ nội dung vào Public Domain.
- **Commercial Redistribution**: **Allowed**
- **Date Audited**: 2026-09-25
- **Intended Use trong Project Ascendant**: Làm icon phôi kim loại (Thỏi sắt, Quặng thô, Mảnh hắc diện thạch) cho Hệ thống Thợ rèn Blacksmithing (`item-003`).
- **Status**: **APPROVED**
- **Harvested Files (3 assets)**:
  | File Name | Relpath | Size | SHA-256 Checksum |
  | :--- | :--- | :---: | :--- |
  | `doficia_iron_ingot.png` | `Content/Art/ThirdParty/CC0_Harvested/Materials/doficia_iron_ingot.png` | 543 B | `abd32a026d6fa7582704d0c21ad350b096ed332c6aaf0afdfc5b28ce4dcea752` |
  | `doficia_iron_ore.png` | `Content/Art/ThirdParty/CC0_Harvested/Materials/doficia_iron_ore.png` | 1151 B | `279ade54ad8205fe822a56e93df0e9fdce7093b818f260904204b062014dea04` |
  | `doficia_obsidian_shard.png` | `Content/Art/ThirdParty/CC0_Harvested/Materials/doficia_obsidian_shard.png` | 4044 B | `93fe61cd5a63b25908b8ee9f72d7f740000b28d36027ced2c6745efa65e761b0` |

---

### Pack 04: Kenney — Game & UI Icons
- **Asset/Pack Name**: `kenney-game-icons`
- **Original URL**: https://kenney.nl/assets/game-icons / https://github.com/Papyszoo/CC0-Public-Domain-Sprites/tree/main/packs/kenney-game-icons
- **Source Repository**: `Papyszoo/CC0-Public-Domain-Sprites`
- **Commit / Version**: `e27131731f89187b9842e9c6c95381a2e954aafd` (2026-08-16)
- **License**: **CC0 1.0 Universal**
  - *Bằng chứng thẩm định*: Tệp `License.txt` trong pack xác nhận cấp phép bởi Kenney (kenney.nl) dưới chuẩn CC0 1.0.
- **Commercial Redistribution**: **Allowed**
- **Date Audited**: 2026-09-25
- **Intended Use trong Project Ascendant**: Nút bấm UI và thanh điều hướng cho Combat HUD (`presentation-ui`).
- **Status**: **APPROVED**
- **Harvested Files (5 assets)**:
  | File Name | Relpath | Size | SHA-256 Checksum |
  | :--- | :--- | :---: | :--- |
  | `kenney_button_A.png` | `Content/Art/ThirdParty/CC0_Harvested/UI_Icons/kenney_button_A.png` | 15943 B | `1e000a513313651db35920569af754431fa3961b42099c27adfffdcc9b2a81dd` |
  | `kenney_button_B.png` | `Content/Art/ThirdParty/CC0_Harvested/UI_Icons/kenney_button_B.png` | 15907 B | `231e4efa8445dcb0a70c28fc75f09590671f921262964d5b28765b750d46f236` |
  | `kenney_bars_horizontal.png` | `Content/Art/ThirdParty/CC0_Harvested/UI_Icons/kenney_bars_horizontal.png` | 15220 B | `3b8529604d7c8a3a28aa39709b8838031c87062a8e5a5ee0087c7e9d0357a14d` |
  | `kenney_arrow_down.png` | `Content/Art/ThirdParty/CC0_Harvested/UI_Icons/kenney_arrow_down.png` | 15328 B | `187df6603922dbde3db15f0f28033e2f7e74435a017095d56cf617bf4eee6c59` |
  | `kenney_audio_on.png` | `Content/Art/ThirdParty/CC0_Harvested/UI_Icons/kenney_audio_on.png` | 15628 B | `aee49ff7b9151214178079eb860a341958ba54627c93d0b2636b05670a37a585` |

---

### Pack 05: Project Cordon — Modern Firearms & Ballistics
- **Asset/Pack Name**: `doficia-project-cordon-weapons`
- **Original URL**: https://github.com/doficia/project-cordon-sprites/tree/master/sprites/weapons
- **Source Repository**: `doficia/project-cordon-sprites`
- **Commit / Version**: `aa879085c1cd3aea647bf4acb85f7953bc9373cd` (2018-10-14)
- **License**: CC0 1.0 Universal (Hợp lệ về mặt pháp lý)
- **Commercial Redistribution**: Allowed
- **Date Audited**: 2026-09-25
- **Intended Use**: Được rà soát cho kho vũ khí.
- **Status**: **REJECTED**
- **Lý do từ chối (Rejection Rationale)**:
  - *Xung đột cốt truyện & thẩm mỹ (Lore & Theme Clash)*: Pack chứa 112 sprites chủ yếu là súng đạn hiện đại / hậu tận thế (`ak47.png`, `awp.png`, `bazooka.png`, `bfg9000.png`), vi phạm nghiêm trọng định hướng Dark Fantasy RPG của `SPEC-ART-2026-09-23-V2`.
  - Không copy bất kỳ file nào từ pack này vào dự án.

---

### Pack 06: External Community Rip / Unverified Submission Sample
- **Asset/Pack Name**: `unverified-community-rip-sample`
- **Original URL**: N/A (Third-party forum / Reddit mirror)
- **Source Repository**: External Mirror
- **Commit / Version**: Unversioned
- **License**: **UNKNOWN**
- **Commercial Redistribution**: **Restricted / Unknown**
- **Date Audited**: 2026-09-25
- **Intended Use**: Kiểm thử quy tắc an toàn pháp lý.
- **Status**: **REJECTED**
- **Lý do từ chối (Rejection Rationale)**:
  - Vi phạm nguyên tắc Zero-Trust: Trạng thái bản quyền không được chứng minh rõ ràng bằng file license hoặc tuyên bố công quyền của tác giả gốc. Theo nguyên tắc, UNKNOWN bị từ chối triệt để khỏi pipeline sản xuất.

---

### Pack 07: Dungeon Crawl Stone Soup — Dungeon Environments
- **Asset/Pack Name**: `dcss-dungeon-environments`
- **Original URL**: https://github.com/Papyszoo/CC0-Public-Domain-Sprites/tree/main/packs/dcss-dungeon-environments
- **Source Repository**: `Papyszoo/CC0-Public-Domain-Sprites`
- **Commit / Version**: `e27131731f89187b9842e9c6c95381a2e954aafd` (2026-08-16)
- **License**: **CC0 1.0 Universal** (Xác thực hợp lệ)
- **Commercial Redistribution**: Allowed
- **Date Audited**: 2026-09-25
- **Intended Use**: Nghiên cứu làm gạch nền hầm ngục phế tích (Catacombs).
- **Status**: **PENDING**
- **Lý do tạm hoãn (Pending Rationale)**:
  - Cần thẩm định thêm về góc chiếu (Top-down $90^\circ$ của DCSS vs Isometric $2.5\text{D}$ của Project Ascendant) trước khi thu hoạch để tránh lãng phí dung lượng repo. Chuyển sang Agent 5 thẩm định góc nhìn trước khi duyệt nạp.
