# Story 003: Dynamic Look-Ahead SpringArm Camera & Line-of-Sight Occlusion Dither

> **Epic**: Input & Isometric Camera Controller  
> **Status**: Complete  
> **Layer**: Foundation  
> **Type**: Integration  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-09-16  

## Context

**GDD**: [`design/gdd/isometric-controller.md`](file:///mnt/Data/Projects/project-games/design/gdd/isometric-controller.md)  
**Requirement**: `TR-ctrl-003`  
*(Requirement text lives in `docs/architecture/tr-registry.yaml` — Occluding 3D geometry dither-fade transparency shader when blocking character or enemy line of sight)*  

**ADR Governing Implementation**: [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) (Primary), [`docs/architecture/architecture.md`](file:///mnt/Data/Projects/project-games/docs/architecture/architecture.md) Chapter 3  
**ADR Decision Summary**: Defines custom 2.5D SpringArm parameters with smooth camera lag, disabling spring-arm clipping in favor of material dithered opacity masks on blocking geometry, and interpolating a dynamic look-ahead offset.

**Engine**: Unreal Engine 5.7 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Utilizes Substrate/Dithered Opacity material parameter collections and `FMath::VInterpTo` for camera offset smoothing.

**Control Manifest Rules (Foundation Layer)**:
- Required: SpringArm must disable spring collision test (`bDoCollisionTest = false`); Occlusion must use dithered material masks instead of zooming.
- Forbidden: Never allow camera clipping into character face when obstructed by walls.
- Guardrail: Camera interpolation tick $\le 0.2\text{ms}$ on render/game thread.

---

## Acceptance Criteria

*From GDD `design/gdd/isometric-controller.md`, scoped to this story:*

- [x] **AC-1 (Isometric SpringArm Specifications)**: `USpringArmComponent` is configured with `TargetArmLength = 1200.0f`, fixed rotation `Pitch = -45.0f`, `Yaw = 45.0f`, `Roll = 0.0f`, `bDoCollisionTest = false`, and `CameraLagSpeed = 12.0f`.
- [x] **AC-2 (Dynamic Look-Ahead Offset)**: As the player aims away from the character, the camera shifts up to $250\text{ cm}$ (`MaxLookAheadDistance = 250.0f`) along the aim direction; when aim returns to center, the camera smoothly re-centers with interpolation speed $8.0$ (`LookAheadReturnSpeed = 8.0f`).
- [x] **AC-3 (Line-of-Sight Occlusion Dither)**: Line-traces between the camera and character detect any blocking 3D meshes (pillars, archways, trees), setting their dynamic material scalar parameter `DitherOpacity` to $0.25f$ within a $200\text{ cm}$ cylinder to ensure the player and boss telegraphs are never occluded.

---

## Implementation Notes

*Derived from Master Architecture Chapter 3 & GDD Formulas:*

1. **SpringArm & Camera Setup (`APACameraActor` / `APABaseCharacter`)**:
   - `SpringArm->SetRelativeRotation(FRotator(-45.0f, 45.0f, 0.0f));`
   - `SpringArm->TargetArmLength = 1200.0f;`
   - `SpringArm->bEnableCameraLag = true;`
   - `SpringArm->CameraLagSpeed = 12.0f;`
   - `SpringArm->bDoCollisionTest = false;`
2. **Look-Ahead Offset Math**:
   ```cpp
   float CursorDist = FVector::Dist2D(CharacterLocation, HitLocation);
   float TargetDist = FMath::Min(CursorDist * 0.35f, 250.0f);
   FVector TargetLookAhead = AimDirection * TargetDist;
   CurrentLookAhead = FMath::VInterpTo(CurrentLookAhead, TargetLookAhead, DeltaTime, 8.0f);
   SpringArm->SocketOffset = CurrentLookAhead;
   ```
3. **Occlusion Trace**:
   - Sphere sweep ($15\text{ cm}$ radius) from Camera location to Character capsule location.
   - For hit actors: query primitive component, set scalar parameter `DitheredAlpha = 0.25f`.
   - Revert back to `1.0f` when no longer hit.

---

## Out of Scope

*Handled by neighbouring stories — do not implement here:*

- Story 001: Screen-relative WASD input and diagonal normalization.
- Story 002: Mouse cursor ground de-projection and decoupled aiming.

---

## QA Test Cases

*Written by qa-lead at story creation:*

- **AC-1 Test: SpringArm Transform Invariance**:
  - Given: Character rotating through 360 degrees and moving across terrain.
  - When: Camera rotation is evaluated.
  - Then: Asserts Pitch is strictly $-45.0^\circ$, Yaw is strictly $+45.0^\circ$, and `TargetArmLength` remains $1200.0\text{ cm}$.

- **AC-2 Test: Look-Ahead Offset Bounds**:
  - Given: Player aims cursor $1500\text{ cm}$ away along the positive X axis.
  - When: Camera interpolation settles.
  - Then: Asserts `SocketOffset.X` clamps at exactly $250.0\text{ cm} \pm 1.0\text{ cm}$.
  - When 2: Aim cursor is brought back to $(0, 0)$.
  - Then 2: `SocketOffset` smoothly interpolates back to $(0, 0, 0)$ within $0.5\text{s}$.

- **AC-3 Test: Geometry Occlusion Dither Transparency**:
  - Given: A stone pillar mesh placed between camera and character.
  - When: Camera line-trace intersects pillar.
  - Then: Pillar primitive material `DitherOpacity` parameter asserts $0.25f$; character capsule remains visible.
  - When 2: Character moves out from behind pillar.
  - Then 2: `DitherOpacity` parameter restores to $1.0f$.

---

## Test Evidence

**Story Type**: Integration  
**Required evidence**: `tests/integration/controller/camera_lookahead_occlusion_test.cpp` OR screenshot evidence in `production/qa/evidence/camera-lookahead-evidence.md`  
**Status**: [x] Passed (`tests/integration/controller/camera_lookahead_occlusion_test.cpp` — 3 test suites, 22 assertions)  

---

## Dependencies

- Depends on: Story 002 (Decoupled Aiming)
- Unlocks: Epic Completion (Foundation Controller fully ready for implementation)

---

## Completion Notes
**Completed**: 2026-09-16  
**Criteria**: 3/3 passing (AC-1, AC-2, AC-3)  
**Deviations**: None  
**Test Evidence**: Integration test at `tests/integration/controller/camera_lookahead_occlusion_test.cpp` (22 assertions passing)  
**Code Review**: Approved  
