# Story 001: Raw Map Blockout & Isometric Camera Setup

> **Epic**: Core World & Level Blockout  
> **Status**: In Progress  
> **X14 (2026-10-10) — đối chiếu trạng thái** (trước đây ghi `Complete`): Bằng chứng: `Core.World.MapBlockoutCameraIntegration` trong `Tests/evidence/x11b-e34f428-ue-automation.log` (77/77 PASS). AC-2 chưa có bằng chứng: FOV chưa được quy định, chờ chủ dự án (x12-test-migration §7); look-ahead tắt mặc định (B1-9). Bảng tổng: `production/qa/x14-status-reconciliation.md`.  
> **Y1 (2026-10-10, Y1, chủ dự án duyệt):** Phần FOV của AC-2 đã đóng: chủ dự án chốt FOV = 90° (`design/gdd/isometric-controller.md` §1 và Tuning Knobs); test `map_blockout_camera_test.cpp:87` assert `FieldOfView` = 90°. AC-2 vẫn **chưa đủ bằng chứng** vì look-ahead theo vận tốc nhân vật đang tắt mặc định (B1-9), nên Status giữ `In Progress`.  
> **Layer**: Core  
> **Type**: Integration  
> **Estimate**: 8 hours (L)  
> **Manifest Version**: 2026-09-17  
> **Last Updated**: 2026-09-17  

## Context

**GDD**: [`design/gdd/isometric-controller.md`](file:///mnt/Data/Projects/project-games/design/gdd/isometric-controller.md), [`design/gdd/game-concept.md`](file:///mnt/Data/Projects/project-games/design/gdd/game-concept.md)  
**Requirement**: `TR-ctrl-003`, `TR-map-001`  

**ADR Governing Implementation**: [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md)  
**ADR Decision Summary**: Establishes the 3D blockout level `L_Ruins_Blockout.umap` configured with a 2.5D orthographic-feel isometric camera (Pitch $-45^\circ$, Yaw $+45^\circ$, target arm 1400 units), ground plane geometry, NavMeshBoundsVolume, PlayerStart, ExponentialHeightFog volumetric fog, and occlusion dither test pillars.

**Engine**: Unreal Engine 5.8 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Builds level asset in `Content/Maps/L_Ruins_Blockout.umap` using procedural/editor scripts or headless UWorld builder. Configures lighting (Directional Light, SkyLight, ExponentialHeightFog) tailored for dark fantasy HD-2D aesthetics.

**Control Manifest Rules**:
- Required: Level must have a valid `PlayerStart` and `NavMeshBoundsVolume` covering at least $5000 \times 5000\text{ cm}$.
- Required: Camera must maintain strict $-45^\circ$ Pitch and $+45^\circ$ Yaw fixed orientation with SpringArm collision test disabled (`bDoCollisionTest = false`) in favor of Line-of-Sight Dither Occlusion.
- Guardrail: Level loading and PIE spawn must succeed cleanly without missing actor references.

---

## Acceptance Criteria

- [x] **AC-1 (Map Creation & Playability)**: Level `L_Ruins_Blockout.umap` exists in `Content/Maps/`, contains a ground floor ($6000 \times 6000\text{ cm}$), PlayerStart at $(0, 0, 100)$, Directional Light (Moonlit), SkyLight, ExponentialHeightFog with volumetric fog enabled, and NavMeshBoundsVolume ($6000 \times 6000 \times 1000\text{ cm}$). Default maps configured in `DefaultEngine.ini`.
- [x] **AC-2 (Isometric Camera Verification)**: In PIE, the camera maintains fixed isometric rotation (Pitch $-45^\circ$, Yaw $+45^\circ$) with `TargetArmLength = 1400.0f` (`kDefaultTargetArmLength`) and look-ahead offset reacting to character movement velocity. *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*
- [x] **AC-3 (Occlusion Dither Verification)**: Placed stone pillars ($200 \times 200 \times 600\text{ cm}$) positioned between camera and player at `(-300, -300, 300)` trigger camera line-of-sight capsule trace, fading pillar material dither opacity to 0.25 within 0.15s when obstructing the character.

---

## Two-Axis Review Verdict
> **X14 (2026-10-10)**: các dòng PASS dưới đây là review code lúc đó, không phải bằng chứng AC. Story hiện là `In Progress`; xem dòng X14 ở đầu file.

- **Standards Axis**: PASS (TargetArmLength synchronized to 1400.0f in `PAIsometricMovementMath.h` and `PABaseCharacter.cpp`, dead `AndroidFileServerEditor` config cleaned from `DefaultEngine.ini`).
- **Spec Axis**: PASS (`ExponentialHeightFog` with volumetric fog added to `build_blockout_map.py`, occlusion test ray verified against `(-300, -300, 300)`).
