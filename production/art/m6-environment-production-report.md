# Package M6 — Environment & Vegetation Production Foundation Report

## 1. Executive Summary

Package M6 establishes the **new production-quality Environment, Foliage, Props, and Terrain Tileset art foundation** for `ProjectAscendant`. Following the successful character foundation established in Package M5, this package delivers the foundational environmental elements required to construct vibrant, readable 2.5D isometric zones, wild landscapes, and dungeon paths.

All assets are authored strictly to the project's **32-Color 4-Tone Ramp Palette** (SPEC-ART-2026-09-23-V2), with locked **10 o'clock directional lighting**, **strictly binary alpha** (`A ∈ {0, 255}`), and standardized **bottom-center anchor pivots** for seamless Y-sorting in Unreal Engine 5.8 Paper2D.

A total of **43 assets** across five categories were produced, packaged into master `.aseprite` binaries, exported as individual production PNGs and composite atlases, and validated 100% through the automated Environment Art Gate.

---

## 2. Asset Inventory & Catalog

### A. Foliage & Wild Flora (`Content/Art/Environment/Sprites/Foliage/`)

| Asset Name | Dimensions | Pivot (X, Y) | Category | Description | Art Gate |
| :--- | :---: | :---: | :--- | :--- | :---: |
| `FOL_Grass_Tuft_01.png` | 16×16 | (8, 14) | Foliage/Grass | Single wild grass clump with root base | **PASS** |
| `FOL_Grass_Patch_02.png` | 32×32 | (16, 28) | Foliage/Grass | Dense layered turf patch with 10 o'clock specular glints | **PASS** |
| `FOL_Grass_Tall_03.png` | 32×32 | (16, 29) | Foliage/Grass | Tall wind-swayed reeds/grass arching rightward | **PASS** |
| `FOL_Flower_Poppies_01.png` | 32×32 | (16, 28) | Foliage/Flowers | Crimson poppies clustered on grass | **PASS** |
| `FOL_Flower_Bellflowers_02.png` | 32×32 | (16, 28) | Foliage/Flowers | Cobalt luminescent wild bellflowers | **PASS** |
| `FOL_Flower_Daisies_03.png` | 32×32 | (16, 28) | Foliage/Flowers | Golden sun-daisies with white petals | **PASS** |
| `FOL_Bush_Small_01.png` | 32×32 | (16, 28) | Foliage/Bushes | Low undergrowth shrub with red berries | **PASS** |
| `FOL_Bush_Medium_02.png` | 48×48 | (24, 42) | Foliage/Bushes | Dense leafy mound with shaded undercoat | **PASS** |
| `FOL_Bush_Bramble_03.png` | 48×48 | (24, 43) | Foliage/Bushes | Thorny bramble hedge with gnarled dark branches | **PASS** |

### B. Trees & Woodland (`Content/Art/Environment/Sprites/Trees/`)

| Asset Name | Dimensions | Pivot (X, Y) | Category | Description | Art Gate |
| :--- | :---: | :---: | :--- | :--- | :---: |
| `TREE_Sapling_01.png` | 32×48 | (16, 45) | Trees/Woodland | Slender woodland sapling with leafy crown | **PASS** |
| `TREE_Oak_Canopy_02.png` | 64×96 | (32, 90) | Trees/Woodland | Mature broadleaf oak with sprawling canopy & flared roots | **PASS** |
| `TREE_Pine_Evergreen_03.png` | 48×96 | (24, 90) | Trees/Conifer | Highland conifer pine with tiered needle clusters | **PASS** |

### C. Rocks & Geology (`Content/Art/Environment/Sprites/Props/`)

| Asset Name | Dimensions | Pivot (X, Y) | Category | Description | Art Gate |
| :--- | :---: | :---: | :--- | :--- | :---: |
| `ROCK_Pebbles_01.png` | 16×16 | (8, 14) | Rocks | Scattered trail stones and debris | **PASS** |
| `ROCK_Boulder_Mossy_02.png` | 32×32 | (16, 27) | Rocks | Faceted granite boulder with moss top layer | **PASS** |
| `ROCK_Formation_Crag_03.png` | 48×48 | (24, 42) | Rocks | Jagged stratified rock outcrop with deep fracture crevices | **PASS** |

### D. Props & Waymarkers (`Content/Art/Environment/Sprites/Props/`)

| Asset Name | Dimensions | Pivot (X, Y) | Category | Description | Art Gate |
| :--- | :---: | :---: | :--- | :--- | :---: |
| `PROP_Crate_Oak_01.png` | 32×32 | (16, 28) | Props/Containers | Wooden cargo crate with metal corner brackets & bevels | **PASS** |
| `PROP_Barrel_Keg_02.png` | 32×32 | (16, 28) | Props/Containers | Bulging oak keg with iron hoops and top lid | **PASS** |
| `PROP_Signpost_Waymarker_03.png` | 32×32 | (16, 29) | Props/Waymarkers | Weathered wooden directional pointer on mounted stake | **PASS** |
| `PROP_Brazier_Torch_04` | 32×32 (4f) | (16, 28) | Props/Light | Iron tripod floor brazier with 4-frame animated flame | **PASS** |

### E. Modular Terrain Tileset 32×32 (`Content/Art/Environment/Sprites/Tiles/`)

| Tile Name | Dimensions | Role | Function | Art Gate |
| :--- | :---: | :--- | :--- | :---: |
| `TILE_Dirt_Base.png` | 32×32 | Ground Base | Rich dark soil / earthen undercoat | **PASS** |
| `TILE_Grass_Base.png` | 32×32 | Ground Base | Lush green turf with blade highlights | **PASS** |
| `TILE_Stone_Cobble_Path.png` | 32×32 | Pathway | Mortared cobblestone road with worn bevels | **PASS** |
| `TILE_Cliff_Ledge_Wall.png` | 32×32 | Boundary | Vertical striated rock cliff face | **PASS** |
| `TILE_Grass_Edge_N.png` | 32×32 | Transition | Scalloped grass top fringe over dirt | **PASS** |
| `TILE_Grass_Edge_S.png` | 32×32 | Transition | Grass bottom fringe over dirt | **PASS** |
| `TILE_Grass_Edge_W.png` | 32×32 | Transition | Grass left fringe over dirt | **PASS** |
| `TILE_Grass_Edge_E.png` | 32×32 | Transition | Grass right fringe over dirt | **PASS** |
| `TILE_Grass_Corner_NW.png` | 32×32 | Outer Corner | Top-left grass outer corner | **PASS** |
| `TILE_Grass_Corner_NE.png` | 32×32 | Outer Corner | Top-right grass outer corner | **PASS** |
| `TILE_Grass_Corner_SW.png` | 32×32 | Outer Corner | Bottom-left grass outer corner | **PASS** |
| `TILE_Grass_Corner_SE.png` | 32×32 | Outer Corner | Bottom-right grass outer corner | **PASS** |
| `TILE_Grass_Inner_NW.png` | 32×32 | Inner Corner | Top-left grass inner notch | **PASS** |
| `TILE_Grass_Inner_NE.png` | 32×32 | Inner Corner | Top-right grass inner notch | **PASS** |
| `TILE_Grass_Inner_SW.png` | 32×32 | Inner Corner | Bottom-left grass inner notch | **PASS** |
| `TILE_Grass_Inner_SE.png` | 32×32 | Inner Corner | Bottom-right grass inner notch | **PASS** |

---

## 3. Palette & Material Rules

All assets strictly respect the project's **32-Color 4-Tone Ramp Palette**:
* **Foliage Greens**: `#0E2818` (Deep Shadow), `#1B5226` (Foliage Shadow), `#3D8B37` (Midtone), `#7EC850` (Sunlit Highlight), `#C8F080` (Specular Glint).
* **Earth & Wood Browns**: `#2C1808` (Loam Deep), `#5C3A1E` (Bark Shadow), `#9A6A40` (Timber Midtone), `#D0A878` (Dry Earth / Weathered Wood).
* **Granite & Stone Grays**: `#121214` (Outline), `#303644` (Slate Shadow), `#4A5868` (Granite Midtone), `#94A4B4` (Stone Light), `#DCE4EC` (Quartz Highlight).
* **Floral & Element Accents**: Crimson (`#D02020`), Bellflower Blue (`#2860D0`), Sun Daisy Gold (`#E0A830`), Torch Fire (`#B43C14` / `#FFB428`).

### Lighting Rules
* **10 o'clock key lighting**: Upper-left illumination casts consistent shadows toward 4 o'clock (lower-right).
* **Zero Pillow Shading**: All volumes are modeled with planar light-and-shade steps, not radial edge blurs.
* **Strict Binary Alpha**: Zero pixels with `0 < A < 255`. Clean edges eliminate halos against any background terrain.

---

## 4. Master Aseprite Binaries & Tooling

```text
Content/Art/Environment/Source/
├── environment_foliage.aseprite          (Foliage layers & animation tags)
├── environment_trees.aseprite            (Trees & canopy hierarchy)
├── environment_props.aseprite            (Props, rocks & brazier flame frames)
└── environment_terrain_tileset.aseprite  (Full 16-tile atlas master)

Tools/Aseprite/
├── generate_environment_assets.py        (Automated artisan generator & exporter)
└── environment_art_gate.py               (Automated deterministic environment validator)
```

---

## 5. Art Gate Validation Results

Validation was executed via `Tools/Aseprite/environment_art_gate.py`:

```text
================================================================================
Environment Art Gate Validation — Package M6
Target Directory: Content/Art/Environment
================================================================================

Inspection Summary:
  Total Assets Inspected: 43
  Passed:                 43
  Failed:                 0

Art Gate: PASS
  All environment assets satisfy binary alpha, palette budget, and grid constraints!
```

---

## 6. Unreal Engine Paper2D Integration

* **Tile Grid**: Native 32×32 pixels per tile. Compatible with Unreal Engine Paper2D TileMap component (PPU = 1.0 or 32 PPU standard).
* **Pivot Coordinates**: Documented per asset in `Content/Art/Environment/metadata/environment_metadata.json`. All trees and standing props use bottom-center pivot coordinates to ensure correct Paper2D Y-sorting relative to the Vanguard character (`(64, 114)`).
* **Regression Safety**: Legacy files under `Art_Gallery/legacy/environment/` were completely untouched. Zero engine runtime C++ or PaperZD files were modified.
