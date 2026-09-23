# Story 001: Screen-Relative 8-Way Locomotion & Diagonal Vector Normalization

> **Epic**: Input & Isometric Camera Controller  
> **Status**: Complete  
> **Layer**: Foundation  
> **Type**: Logic  
> **Estimate**: 4 hours (M)  
> **Manifest Version**: 2026-09-16  
> **Last Updated**: 2026-09-16  

## Context

**GDD**: [`design/gdd/isometric-controller.md`](file:///mnt/Data/Projects/project-games/design/gdd/isometric-controller.md)  
**Requirement**: `TR-ctrl-002`  
*(Requirement text lives in `docs/architecture/tr-registry.yaml` — 8-way directional character locomotion using Enhanced Input IMC with smooth acceleration and decel curves)*  

**ADR Governing Implementation**: [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) (Primary), [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Secondary)  
**ADR Decision Summary**: Integrates Unreal Engine 5.7 Enhanced Input with `UCharacterMovementComponent` prediction, mapping 2D inputs relative to the fixed 45° isometric camera yaw and enforcing normalized vector velocity.

**Engine**: Unreal Engine 5.7 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Configures `UInputMappingContext` (`IMC_DefaultLocomotion`) and `UInputAction` (`IA_Move`) with 2D axis vector input modifiers.

**Control Manifest Rules (Foundation Layer)**:
- Required: Locomotion must utilize Enhanced Input and `UCharacterMovementComponent` client prediction; movement speed is data-driven from GAS Attributes.
- Forbidden: Never bypass `CharacterMovementComponent` prediction; Never hardcode velocity values in C++.
- Guardrail: Input-to-motion dispatch latency $\le 1$ frame (16.67ms).

---

## Acceptance Criteria

*From GDD `design/gdd/isometric-controller.md`, scoped to this story:*

- [x] **AC-1 (Camera-Relative 8-Way Movement)**: Pressing W, S, A, D moves the character exactly orthogonal to the player's screen (W moves straight up-screen, S moves down-screen, A moves left-screen, D moves right-screen) by applying a $45^\circ$ yaw rotation matrix transformation to raw input vectors.
- [x] **AC-2 (Diagonal Velocity Normalization)**: Pressing simultaneous diagonal inputs (e.g. W + D) normalizes the input vector length to 1.0, ensuring diagonal movement speed matches the 550 cm/s baseline and strictly prevents $\sqrt{2} \approx 1.414\times$ speed exploits.
- [x] **AC-3 (Cancellation on Opposing Inputs)**: Pressing opposing keys simultaneously (W + S or A + D) cancels movement input cleanly to zero, immediately transitioning locomotion to `Idle` state without lingering velocity.

---

## Implementation Notes

*Derived from ADR-0002 Implementation Guidelines & GDD Formulas:*

1. **Enhanced Input Setup (`APABasePlayerController`)**:
   - Bind `IA_Move` in `SetupInputComponent` with trigger type `ETriggerEvent::Triggered`.
   - Read value as `FVector2D InputAxisVector = Value.Get<FVector2D>();`
2. **Camera-Relative Vector Rotation**:
   - Rotate input vector by camera yaw ($45^\circ$):
     ```cpp
     const FRotator YawRotation(0.0f, 45.0f, 0.0f);
     const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
     const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
     FVector WorldDirection = (ForwardDirection * InputAxisVector.Y) + (RightDirection * InputAxisVector.X);
     WorldDirection = WorldDirection.GetClampedToMaxSize(1.0f);
     ```
3. **Movement Execution**:
   - Call `AddMovementInput(WorldDirection, 1.0f)` on the character pawn.
   - Max walk speed dynamically evaluates `UAscendantAttributeSet::GetMoveSpeedAttribute()` (550 cm/s baseline).

---

## Out of Scope

*Handled by neighbouring stories — do not implement here:*

- Story 002: Mouse cursor de-projection and decoupled twin-stick aiming.
- Story 003: Dynamic camera look-ahead offset and occlusion dither fading.

---

## QA Test Cases

*Written by qa-lead at story creation:*

- **AC-1 Test: Screen-Relative Orthogonal Mapping**:
  - Given: Camera pitched at $-45^\circ$ and yawed at $+45^\circ$. Character located at $(0, 0, 0)$.
  - When: Input vector $(0, 1)$ (pressing W) is applied.
  - Then: Resulting world direction asserts vector $(+\frac{\sqrt{2}}{2}, +\frac{\sqrt{2}}{2}, 0)$, moving character up-screen along the isometric diagonal.

- **AC-2 Test: Diagonal Vector Normalization**:
  - Given: Player presses W and D simultaneously, resulting in raw 2D input $(1.0, 1.0)$ with magnitude $1.414$.
  - When: Clamped world direction vector is evaluated.
  - Then: Magnitude asserts exactly $1.000 \pm 0.001$, and horizontal speed is clamped at $550.0\text{ cm/s}$.

- **AC-3 Test: Opposing Input Cancellation**:
  - Given: Player holding W is moving at $550\text{ cm/s}$.
  - When: Player simultaneously presses S.
  - Then: Resulting acceleration evaluates to zero within 1 frame, and movement state transitions to `Idle`.

---

## Test Evidence

**Story Type**: Logic  
**Required evidence**: `tests/unit/controller/screen_relative_movement_test.cpp` — must exist and pass automated CI  
**Status**: [x] Verified and passing (5 test suites, 18 assertions)  

---

## Dependencies

- Depends on: None (Foundation story for Controller & Camera)
- Unlocks: Story 002 (Decoupled Aiming) & Story 003 (Camera Look-Ahead)

---

## Completion Notes
**Completed**: 2026-09-16  
**Criteria**: 3/3 passing (0 deferred)  
**Deviations**: None  
**Test Evidence**: Logic: unit test at `tests/unit/controller/screen_relative_movement_test.cpp` (5 test suites, 18 assertions)  
**Code Review**: Complete (Approved)  
