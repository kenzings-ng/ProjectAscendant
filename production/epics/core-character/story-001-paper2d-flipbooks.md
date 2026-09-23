# Story 001: Paper2D Sprite Extraction & Flipbooks

> **Epic**: Core Character & PaperZD Integration  
> **Status**: Complete  
> **Layer**: Core  
> **Type**: Asset  
> **Estimate**: 8 hours (L)  
> **Manifest Version**: 2026-09-17  
> **Last Updated**: 2026-09-17  

## Context

**GDD**: [`design/gdd/game-concept.md`](file:///mnt/Data/Projects/project-games/design/gdd/game-concept.md)  
**Requirement**: `TR-art-001`, `TR-art-002`  
**ADR Governing Implementation**: [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md)  

**Engine**: Unreal Engine 5.8 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Extracts frames from `vanguard_pixel_spritesheet.png` and `stone_golem_boss_pixel_spritesheet.png` into discrete frames, enforces `TEXTUREGROUP_Pixels`, `Filter: Nearest` and `TMGS_NO_MIPMAPS` to avoid bilinear blurring, and builds `UPaperFlipbook` assets at 12 FPS (Vanguard) and 10 FPS (Boss).

---

## Acceptance Criteria

- [x] **AC-1 (Texture & Pixel Filtering)**: Imported textures `T_Vanguard_Spritesheet` and `T_Boss_Spritesheet` in `Content/art/characters/` configured with `Filter: Nearest`, `MipGenSettings: TMGS_NO_MIPMAPS`, and `TextureGroup: TEXTUREGROUP_Pixels` to prevent bilinear blurring.
- [x] **AC-2 (Vanguard Flipbooks)**: Discrete flipbooks created in `Content/art/characters/vanguard/flipbooks/`: `FB_Vanguard_Idle` (8 frames), `FB_Vanguard_Run` (3 frames), `FB_Vanguard_Attack1` (8 frames), `FB_Vanguard_Dash` (6 frames), `FB_Vanguard_Hurt` (6 frames), `FB_Vanguard_Death` (6 frames) playing at 12.0 FPS with individual PNG frames in `Content/art/characters/vanguard/` and animated GIF previews.
- [x] **AC-3 (Stone Golem Boss Flipbooks)**: Flipbooks created in `Content/art/characters/boss/flipbooks/`: `FB_Golem_Idle` (6 frames), `FB_Golem_Walk` (6 frames), `FB_Golem_Slam` (6 frames), `FB_Golem_Stagger` (5 frames), `FB_Golem_Death` (6 frames) playing at 10.0 FPS with individual PNG frames in `Content/art/characters/boss/` and animated GIF previews.

---

## Two-Axis Review Verdict
- **Standards Axis**: PASS (Mipmaps disabled `TMGS_NO_MIPMAPS`, project-relative path resolution via `unreal.Paths.project_content_dir()`, Nearest-neighbor pixel filter enforced).
- **Spec Axis**: PASS (All 11 flipbooks created with exact playback FPS: 12.0 FPS for Vanguard and 10.0 FPS for Boss).
