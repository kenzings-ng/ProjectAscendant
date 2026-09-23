# Story 003: Floating Combat Text & Action Feedback

> **Epic**: Presentation & Interface Layer (Combat HUD & Interactive UI)  
> **Status**: ✅ Done  
> **Layer**: Presentation  
> **Type**: UI  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-19  
> **Last Updated**: 2026-09-23  

## Context

**GDD**: [`design/gdd/combat-hud.md`](../../design/gdd/combat-hud.md), [`design/gdd/combat-system.md`](../../design/gdd/combat-system.md)  
**Requirement**: `TR-hud-003` (Floating Damage Numbers, Crit bounce, Posture damage text, "PERFECT!" dodge banner)

**ADR Governing Implementation**: 
- [`ADR-0002: GAS Integration & PaperZD Pixel Sprites`](../../docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) (Combat event delegates for hit/damage notifications)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `UPAFloatingCombatTextComponent` with pooling and ballistic trajectory interpolation.

---

## Acceptance Criteria

- [x] **AC-1 (Damage Number Categories & Styling)**:
  - Normal Damage: Crisp white text with black border.
  - Critical Damage: Radiant orange-yellow text, scaled 1.5x with bounce animation.
  - Posture Damage: Golden-yellow text targeting posture bar.
- [x] **AC-2 (Ballistic Radial Scatter Arc)**: Numbers spawn at hit location and follow ballistic arc equation: $\vec{P}(t) = \vec{P}_0 + \vec{V}_0 \cdot t + \frac{1}{2} \vec{g} \cdot t^2$ with $\vec{V}_0 = (v_x, v_y, 180\text{ cm/s})$ and gravity $\vec{g} = (0, 0, -300\text{ cm/s}^2)$. Radial offset $\pm 25\text{px}$ prevents overlapping during cleaves. Lifetime = 0.60s with fade-out.
- [x] **AC-3 ("PERFECT!" Callout)**: When a Perfect Dodge is executed, spawns a golden-silver callout text ("PERFECT!") directly above the player character for 0.50s.
- [x] **AC-4 (Performance & Object Pooling)**: Recycles text instances via an in-memory pool (up to 50 active instances) to prevent garbage collection spikes during high-density combat.

---

## Implementation Notes

1. **`PACombatTextTypes.h`**:
   - `EPACombatTextType`: `NormalDamage`, `CriticalDamage`, `PostureDamage`, `PerfectDodgeCallout`.
   - `FPACombatTextInstance`: `WorldPosition`, `Velocity`, `Lifetime`, `MaxLifetime`, `Scale`, `Opacity`, `Text`, `Type`.
2. **`PAFloatingCombatTextComponent.h` / `PAFloatingCombatTextComponent.cpp`**:
   - `SpawnCombatText(const FVector& WorldLocation, float Amount, EPACombatTextType Type)`
   - `SpawnPerfectDodgeCallout(const FVector& CharacterLocation)`
   - `TickComponent(float DeltaTime)` updates physics arc and opacity fade.
   - `GetActiveInstances()` for UMG canvas renderer.

---

## QA Test Cases

- **Test 1: Normal vs Critical Styling**: Verify type enum produces correct scale (1.0 vs 1.5) and text format.
- **Test 2: Arc Physics**: Verify position at t = 0.3s follows ballistic formula.
- **Test 3: Radial Offset**: Spawn 3 numbers at same location -> radial offsets ensure non-identical start velocities.
- **Test 4: Pool Recycling**: Spawn 60 numbers -> pool caps active count and reuses finished instances with zero memory leaks.
