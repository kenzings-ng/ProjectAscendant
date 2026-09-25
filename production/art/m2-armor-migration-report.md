# Package M2 — Legacy Armor Migration Report

## 1. Executive Summary

- **Expected legacy assets**: 8
- **Inspected**: 8
- **Production candidates**: 0
- **Art Gate PASS**: 0
- **REWORK_REQUIRED**: 8
- **REFERENCE_ONLY**: 0
- **MANUAL_REVIEW**: 0
- **ARCHIVE**: 0
- **Production outputs**: 0

All 8 legacy armor assets located in `Art_Gallery/legacy/armor/` were thoroughly inspected, verified, and audited against the official ProjectAscendant Art Gate specification (`production/qa/art-gate-criteria.md`). Every asset was determined to be a high-resolution, continuous-alpha illustration (256×256 pixels, 3,399 to 9,724 colors, continuous anti-aliased alpha gradients) rather than a native pixel art sprite. In strict accordance with the Art Gate mandate ("GATE, NOT PRODUCTION — Do NOT lower the gate to achieve a specific number of migrated assets"), all 8 assets have been classified as `REWORK_REQUIRED` and kept in legacy storage. No unapproved assets were promoted to `Content/Art/Armor/`.

---

## 2. Complete Inventory

| Source | Dimensions | Format | Classification | Production Output | Art Gate | Notes |
| :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| `Armor_Arcanist_Scholar_Tunic.png` | 256×256 | RGBA | **REWORK_REQUIRED** | None | **FAIL** | Non-pixel art canvas (256×256), 181 semi-trans pixels, 9,724 colors. Concept/rework source for Arcanist robe. |
| `Armor_Leather_Scout_Vest.png` | 256×256 | RGBA | **REWORK_REQUIRED** | None | **FAIL** | Non-pixel art canvas (256×256), 250 semi-trans pixels, 6,214 colors. Concept/rework source for Ranger scout vest. |
| `Armor_Steel_Knight_Cuirass.png` | 256×256 | RGBA | **REWORK_REQUIRED** | None | **FAIL** | Non-pixel art canvas (256×256), 340 semi-trans pixels, 6,999 colors. Concept/rework source for Vanguard knight cuirass. |
| `Boots_Leather_Traveler_Boots.png` | 256×256 | RGBA | **REWORK_REQUIRED** | None | **FAIL** | Non-pixel art canvas (256×256), 355 semi-trans pixels, 4,583 colors. Concept/rework source for leather boots. |
| `Greaves_Steel_Knight_Legguards.png` | 256×256 | RGBA | **REWORK_REQUIRED** | None | **FAIL** | Non-pixel art canvas (256×256), 420 semi-trans pixels, 4,725 colors. Concept/rework source for plate legguards. |
| `Helmet_Leather_Ranger_Hood.png` | 256×256 | RGBA | **REWORK_REQUIRED** | None | **FAIL** | Non-pixel art canvas (256×256), 271 semi-trans pixels, 3,399 colors. Concept/rework source for ranger hood. |
| `Helmet_Steel_Knight_Closed.png` | 256×256 | RGBA | **REWORK_REQUIRED** | None | **FAIL** | Non-pixel art canvas (256×256), 288 semi-trans pixels, 5,392 colors. Concept/rework source for closed knight helm. |
| `Helmet_Steel_Knight_Open.png` | 256×256 | RGBA | **REWORK_REQUIRED** | None | **FAIL** | Non-pixel art canvas (256×256), 236 semi-trans pixels, 5,333 colors. Concept/rework source for open knight helm. |

---

## 3. Production Candidates

- **Promoted Assets**: 0
- **Rationale**: None of the 8 candidates satisfied the baseline technical constraints required by `SPEC-ART-2026-09-23-V2` and `production/qa/art-gate-criteria.md`:
  1. **Canvas Size**: 256×256 px (production standard requires 32×32 for inventory icons, 128×128 for Paperdoll equipment rigs).
  2. **Pixel Purity (Criterion 1)**: Smooth anti-aliased line art with non-integer scaling / mixels.
  3. **Binary Alpha Policy**: All files possess continuous alpha channels with hundreds of semi-transparent boundary pixels ($a \in [1, 254]$), violating binary alpha ($a \in \{0, 255\}$).
  4. **Palette Quantization (Criterion 2)**: Over 3,000 distinct colors per asset, lacking the required 4-Tone Ramp Hue-Shifting palette.

---

## 4. Rejected / Deferred Assets

### 1. `Armor_Arcanist_Scholar_Tunic.png`
- **Classification**: `REWORK_REQUIRED`
- **Exact Reasons**: 256×256 px; 181 semi-transparent pixels; 9,724 continuous colors.
- **Recommended Action**: Recreate as a 32×32 inventory icon (`ARM_Arcanist_Scholar_Tunic.png`) and/or integrate into the Caster Master Rig (`master_rig_03.aseprite`) at 128×128 on layer `Chest_Armor`.

### 2. `Armor_Leather_Scout_Vest.png`
- **Classification**: `REWORK_REQUIRED`
- **Exact Reasons**: 256×256 px; 250 semi-transparent pixels; 6,214 continuous colors.
- **Recommended Action**: Recreate as a 32×32 inventory icon (`ARM_Leather_Scout_Vest.png`) and/or integrate into the Agility Master Rig (`master_rig_02.aseprite`) on layer `Chest_Armor`.

### 3. `Armor_Steel_Knight_Cuirass.png`
- **Classification**: `REWORK_REQUIRED`
- **Exact Reasons**: 256×256 px; 340 semi-transparent pixels; 6,999 continuous colors.
- **Recommended Action**: Recreate as a 32×32 inventory icon (`ARM_Steel_Knight_Cuirass.png`) and/or integrate into the HeavyTank Master Rig (`master_rig_01.aseprite`) on layer `Chest_Armor`.

### 4. `Boots_Leather_Traveler_Boots.png`
- **Classification**: `REWORK_REQUIRED`
- **Exact Reasons**: 256×256 px; 355 semi-transparent pixels; 4,583 continuous colors.
- **Recommended Action**: Recreate as a 32×32 inventory icon (`ARM_Leather_Traveler_Boots.png`) and/or integrate into Master Rigs on layer `Boots`.

### 5. `Greaves_Steel_Knight_Legguards.png`
- **Classification**: `REWORK_REQUIRED`
- **Exact Reasons**: 256×256 px; 420 semi-transparent pixels; 4,725 continuous colors.
- **Recommended Action**: Recreate as a 32×32 inventory icon (`ARM_Steel_Knight_Legguards.png`) and/or integrate into Master Rigs on layer `Leg_Armor`.

### 6. `Helmet_Leather_Ranger_Hood.png`
- **Classification**: `REWORK_REQUIRED`
- **Exact Reasons**: 256×256 px; 271 semi-transparent pixels; 3,399 continuous colors.
- **Recommended Action**: Recreate as a 32×32 inventory icon (`ARM_Leather_Ranger_Hood.png`) and/or integrate into Master Rigs at socket `Helm Crest Socket (64, 40)`.

### 7. `Helmet_Steel_Knight_Closed.png`
- **Classification**: `REWORK_REQUIRED`
- **Exact Reasons**: 256×256 px; 288 semi-transparent pixels; 5,392 continuous colors.
- **Recommended Action**: Recreate as a 32×32 inventory icon (`ARM_Steel_Knight_Closed.png`) and/or integrate into Master Rigs at socket `Helm Crest Socket (64, 40)`.

### 8. `Helmet_Steel_Knight_Open.png`
- **Classification**: `REWORK_REQUIRED`
- **Exact Reasons**: 256×256 px; 236 semi-transparent pixels; 5,333 continuous colors.
- **Recommended Action**: Recreate as a 32×32 inventory icon (`ARM_Steel_Knight_Open.png`) and/or integrate into Master Rigs at socket `Helm Crest Socket (64, 40)`.

---

## 5. Dependencies

- **Codebase Dependency Audit**: Searched `Source/`, `Content/`, `Config/`, and `Plugins/` for references to each of the 8 filenames.
- **Result**: Zero (0) references discovered in C++, Blueprints, DataAssets, Paper2D, or PaperZD.
- **Runtime Safety**: No `.uasset` files or existing Paperdoll configurations (`Content/art/characters/paperdoll/`) were moved, renamed, or modified.

---

## 6. Tooling Changes

- **Modified Tool**: `Tools/Aseprite/aseprite_export_pipeline.py`
- **Rationale**: Provide automated Art Gate validation for armor equipment assets, adhering to the same programmatic standards as static weapons.
- **Modifications**:
  - Added function `validate_and_process_armor(input_path, output_dir)`.
  - Added CLI flag `--armor` (`-a`) to batch-validate armor candidates against dimensions (32×32 or 128×128), binary alpha, palette bounds ($\le 32$ colors), and thumbnail silhouette legibility.
- **Backward Compatibility**: Fully verified; existing character rig export and `--weapons` export functionality remain 100% operational.

---

## 7. Validation & Verification

1. **Source Preservation**:
   All 8 source files in `Art_Gallery/legacy/armor/` remain byte-for-byte untouched:
   - `Armor_Arcanist_Scholar_Tunic.png`: `776f24d134e29e80b46c7569cabd6859ae273163b43e6258edfb917511c12b05`
   - `Armor_Leather_Scout_Vest.png`: `0bc14310b1e4bf6b74ec696ecdb6140f87e08c3a878e7ab378b975ea6771235d`
   - `Armor_Steel_Knight_Cuirass.png`: `3909984e95301f6467c85c6b6295c31b8ce7e7c22e94f0c7607d0bb8809085d6`
   - `Boots_Leather_Traveler_Boots.png`: `b34453c66cc72bb4bca74f1f2877a46d0278871863ff995f361949f0e0a478a0`
   - `Greaves_Steel_Knight_Legguards.png`: `c081cf19318d2eeb964674005fdc7aac0d11844307d54f016c6281d5fe1c0933`
   - `Helmet_Leather_Ranger_Hood.png`: `326f0a75fb925ae85d82d48577707fa24cfea79c02ab8d255d16011fc28ce35a`
   - `Helmet_Steel_Knight_Closed.png`: `df1dcc5ed5f3b191b46de4c1e774eed68ced3960281c2fdb972b48ad2b8bd5f7`
   - `Helmet_Steel_Knight_Open.png`: `93fee396ea194e1e6d0f6bd0be4b0e5fb7039e5ff1b4fbd930d43306e02c2458`

2. **Art Gate Result**:
   - `aseprite_export_pipeline.py --armor`: 0 PASS, 8 REWORK_REQUIRED.

3. **Broken References**:
   - Total broken references introduced: 0.

4. **UE5.8 Automation Regression**:
   - Suite: `ProjectAscendant.Combat.RegressionHardening`
   - Exit Code: 0 (PASS).
