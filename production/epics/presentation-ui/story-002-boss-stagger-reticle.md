# Story 002: Boss Health, Stagger Posture & Execution Reticle

> **Epic**: Presentation & Interface Layer (Combat HUD & Interactive UI)  
> **Status**: ✅ Done  
> **Layer**: Presentation  
> **Type**: UI  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-19  
> **Last Updated**: 2026-09-22  

## Context

**GDD**: [`design/gdd/combat-hud.md`](../../design/gdd/combat-hud.md), [`design/gdd/stagger-system.md`](../../design/gdd/stagger-system.md), [`design/gdd/boss-ai.md`](../../design/gdd/boss-ai.md)  
**Requirement**: `TR-hud-002` (Boss Cluster: 3-phase HP bar, Posture Break flashing meter, Part breaking indicators, Execution Reticle)

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Server-Authoritative boss stats and exclusive execution window RPC)
- [`ADR-0002: GAS Integration & PaperZD Pixel Sprites`](../../docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) (Boss attribute & tag listeners)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `UPABossHealthWidget` (`UUserWidget`) and `UPAExecutionReticleWidget`.

---

## Acceptance Criteria

- [x] **AC-1 (Boss Health Bar & 3-Phase Thresholds)**: `UPABossHealthWidget` displays boss name, level, and health bar. Features distinct threshold notches at 75% and 25% Max HP indicating combat phase transitions.
- [x] **AC-2 (Posture Stagger Meter & 4.0 Hz Flashing)**: Posture bar tracks accumulated posture damage (0% to 100%). When Posture reaches 100% (Boss enters `State.Staggered`), the bar triggers continuous 4.0 Hz flashing in bright red for the duration of the 3.0s stagger window.
- [x] **AC-3 (Part Status Cross-out)**: Displays status indicators for breakable boss parts (Horn, Tail). When a part is broken (via gameplay tag or part break delegate), its icon displays an authoritative red cross-out overlay.
- [x] **AC-4 (World-to-Screen Execution Reticle)**: When Boss is staggered, `UPAExecutionReticleWidget` projects the boss execution socket (`Socket_Execution`) from 3D world space to 2D screen coordinates, displaying the animated execution reticle and contextual finisher prompt (`Attack` / `Interact`).

---

## Implementation Notes

1. **`PABossHUDTypes.h`**:
   - `EPABossPostureState`: `Accumulating`, `BrokenFlashing`, `Recovered`.
   - `FPABossPartStatus`: `PartId`, `bIsBroken`.
   - `FPABossHUDModel`: Tracks Boss HP, Posture percent, Stagger timer (3.0s), and part status map.
2. **`PABossHealthWidget.h` / `PABossHealthWidget.cpp`**:
   - `SetBossTarget(AActor* BossActor)`
   - `UpdateBossVitals(float CurrentHP, float MaxHP, float CurrentPosture, float MaxPosture)`
   - `HandlePartBroken(FName PartId)`
   - `GetPhaseNotchPercents(TArray<float>& OutNotches)` (returns 0.75, 0.25)
   - `IsStaggerFlashing() const`
3. **`PAExecutionReticleWidget.h` / `PAExecutionReticleWidget.cpp`**:
   - `UpdateReticlePosition(APlayerController* PC, FVector WorldLocation)`
   - `SetExecutionAvailable(bool bAvailable, float RemainingDuration)`

---

## QA Test Cases

- **Test 1: Phase Notches**: Notches are positioned exactly at 75% and 25%.
- **Test 2: Posture Stagger Flash**: Posture = 100% -> PostureState becomes `BrokenFlashing`, blink rate = 4.0 Hz, active for 3.0s.
- **Test 3: Part Break Cross-out**: Break horn -> `Horn.bIsBroken == true`.
- **Test 4: Execution Reticle Projection**: Reticle becomes visible only when Staggered == true, projects to screen within viewport bounds.
