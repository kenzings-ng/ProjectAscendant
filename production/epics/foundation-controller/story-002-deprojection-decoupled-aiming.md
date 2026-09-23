# Story 002: Screen-to-World De-projection & Decoupled Twin-Stick Aiming

> **Epic**: Input & Isometric Camera Controller  
> **Status**: Complete  
> **Layer**: Foundation  
> **Type**: Logic  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-09-16  

## Context

**GDD**: [`design/gdd/isometric-controller.md`](file:///mnt/Data/Projects/project-games/design/gdd/isometric-controller.md)  
**Requirement**: `TR-ctrl-001`  
*(Requirement text lives in `docs/architecture/tr-registry.yaml` — Screen-to-world de-projection from cursor coordinates onto an isometric ground collision plane at fixed angle)*  

**ADR Governing Implementation**: [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md)  
**ADR Decision Summary**: Establishes independent aiming and movement vectors, de-projecting mouse screen coordinates onto the 3D ground plane and driving PaperZD flipbook sprite facing angles independently of character velocity.

**Engine**: Unreal Engine 5.7 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Implements ray-plane intersection via `FMath::LinePlaneIntersection` and twin-stick analog deadzone filtering.

**Control Manifest Rules (Foundation Layer)**:
- Required: Aim vector must be decoupled from movement direction; PaperZD character orientation tracks aim vector.
- Forbidden: Never lock aim direction to movement direction unless no aim input is present.
- Guardrail: Aiming calculation overhead $\le 0.05\text{ms}$ per tick on player controller.

---

## Acceptance Criteria

*From GDD `design/gdd/isometric-controller.md`, scoped to this story:*

- [x] **AC-1 (Mouse Ground De-projection)**: Cursor screen coordinates are de-projected into world space via raycast intersecting a horizontal plane at character feet level ($Z = \text{CharacterLocation.Z}$), yielding accurate 3D world target coordinates regardless of camera zoom or aspect ratio.
- [x] **AC-2 (Decoupled Aiming & Backpedaling)**: The character's visual orientation and combat aim vector point directly towards the de-projected target, allowing the player to move backwards (away from the target) while continuously aiming and attacking forwards.
- [x] **AC-3 (Gamepad Twin-Stick Support)**: Right analog stick input evaluates directional aiming with a 0.2 deadzone (`GamepadDeadzone = 0.2`); releasing the right stick preserves the last aim orientation.

---

## Implementation Notes

*Derived from ADR-0002 Implementation Guidelines:*

1. **Ray-Plane De-projection Math (`UPAAimComponent`)**:
   - Get mouse position via `APlayerController::GetMousePosition`.
   - Call `UGameplayStatics::DeprojectScreenToWorld` to obtain `RayOrigin` and `RayDirection`.
   - Compute intersection point with ground plane:
     ```cpp
     const FPlane GroundPlane(FVector(0, 0, CharacterLocation.Z), FVector::UpVector);
     FVector HitLocation = FMath::LinePlaneIntersection(RayOrigin, RayOrigin + RayDirection * 10000.0f, GroundPlane);
     ```
2. **Aim Vector & PaperZD Directional Mapping**:
   - `FVector AimDirection = (HitLocation - CharacterLocation).GetSafeNormal2D();`
   - Compute aim angle in degrees: `float AimAngle = FMath::RadiansToDegrees(FMath::Atan2(AimDirection.Y, AimDirection.X));`
   - Map `AimAngle` to 8-way directional PaperZD animation states (East, North-East, North, North-West, West, South-West, South, South-East).
   - Character movement velocity does not override `AimDirection`.

---

## Out of Scope

*Handled by neighbouring stories — do not implement here:*

- Story 001: 8-way screen-relative movement and diagonal speed clamping.
- Story 003: Camera spring-arm look-ahead and line-of-sight occlusion dither.

---

## QA Test Cases

*Written by qa-lead at story creation:*

- **AC-1 Test: Ground Plane De-projection**:
  - Given: Isometric camera positioned at $(0, -1200, 1200)$ with $-45^\circ$ pitch. Character at $(0, 0, 0)$.
  - When: Mouse cursor points at screen center.
  - Then: Intersection point $Z$ equals $0.0f$, and $X, Y$ coordinates align with character position within $0.5\text{ cm}$.

- **AC-2 Test: Decoupled Backpedaling**:
  - Given: Character aiming forward towards $(500, 500, 0)$.
  - When: Player inputs movement backwards towards $(-500, -500, 0)$ at $550\text{ cm/s}$.
  - Then: Character velocity vector is $(-0.707, -0.707, 0)$ while aim vector remains $(+0.707, +0.707, 0)$, and PaperZD flipbook renders North-East facing sprite.

- **AC-3 Test: Gamepad Deadzone Filtering**:
  - Given: Gamepad right stick displaced by $(0.1, 0.1)$ (magnitude $0.141 < 0.2$).
  - When: Aim input is processed.
  - Then: Input is filtered out as deadzone; character retains previous aim orientation without jitter.

---

## Test Evidence

**Story Type**: Logic  
**Required evidence**: `tests/unit/controller/cursor_deprojection_aim_test.cpp` — must exist and pass automated CI  
**Status**: [x] Passed (4 test suites, 24 assertions)  

---

## Dependencies

- Depends on: Story 001 (Locomotion Baseline)
- Unlocks: Story 003 (Camera Look-Ahead & Occlusion) & Core Combat Stories

---

## Completion Notes
**Completed**: 2026-09-16  
**Criteria**: 3/3 passing (AC-1, AC-2, AC-3)  
**Deviations**: None  
**Test Evidence**: Logic unit test at `tests/unit/controller/cursor_deprojection_aim_test.cpp` (24 assertions passing)  
**Code Review**: Approved  
