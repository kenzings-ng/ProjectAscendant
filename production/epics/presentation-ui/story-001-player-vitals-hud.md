# Story 001: Player Vitals & Status HUD Engine

> **Epic**: Presentation & Interface Layer (Combat HUD & Interactive UI)  
> **Status**: ✅ Done  
> **Layer**: Presentation  
> **Type**: UI  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-19  
> **Last Updated**: 2026-09-22  

## Context

**GDD**: [`design/gdd/combat-hud.md`](../../design/gdd/combat-hud.md)  
**Requirement**: `TR-hud-001` (Player Cluster: Health, Stamina, Mana, Catch-up Ghost Bar, Golden Flash, Low Health/Exhausted Vignette)

**ADR Governing Implementation**: 
- [`ADR-0002: GAS Integration & PaperZD Pixel Sprites`](../../docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) (Event-driven GAS attribute delegates, zero polling tick overhead)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `UPAPlayerVitalsWidget` (`UUserWidget`) with `FPAVitalsModel` and `FPAVitalsConfig`.

**Control Manifest Rules (Presentation Layer)**:
- Required: Event-driven attribute updates triggered by `OnAttributeChanged` delegates from `UAscendantAttributeSet`.
- Required: Ghost bar interpolation: 0.40s delay before interpolating toward target health at speed 3.5.
- Required: Perfect Dodge triggers 0.20s Golden Flash on stamina bar with +15 stamina refund visual feedback.
- Guardrail: Low-HP heartbeat pulse triggers when Health < 20% of MaxHealth (60 to 100 BPM scaling); Exhausted graying triggers on `State.Exhausted` tag.

---

## Acceptance Criteria

- [x] **AC-1 (GAS Attribute Binding)**: `UPAPlayerVitalsWidget` binds to `UAscendantAttributeSet` delegates on initialization; Health, Stamina, and Mana percentages update instantaneously on attribute change without tick polling.
- [x] **AC-2 (Catch-Up Ghost Bar)**: When Health decreases, the primary health bar drops immediately while the ghost bar maintains its position for 0.40s (`CatchUpDelay`) before smoothly interpolating at speed 3.5 (`CatchUpInterpSpeed`). When Health increases (healing), ghost bar matches immediately without delay.
- [x] **AC-3 (Stamina State Feedback & Golden Flash)**: Normal stamina consumption updates the bar. Triggering Perfect Dodge activates a 0.20s Golden Flash (`bIsGoldenFlashing == true`) and visually reflects the +15 stamina refund. When `State.Exhausted` is applied, the stamina bar state switches to `Exhausted` (grayed out with lock indicator).
- [x] **AC-4 (Vignette & Screen Effects)**: When Health drops below 20% of MaxHealth, `bLowHealthVignetteActive` becomes true with heartbeat frequency scaling between 60 and 100 BPM. When `State.Exhausted` is active, `bExhaustionVignetteActive` becomes true (50% desaturation flag).

---

## Implementation Notes

1. **`PAPlayerVitalsTypes.h`**:
   - `EPAStaminaBarState`: `Normal`, `GoldenFlash`, `Exhausted`.
   - `FPAVitalsConfig`: `CatchUpDelay = 0.40f`, `CatchUpInterpSpeed = 3.5f`, `GoldenFlashDuration = 0.20f`, `LowHealthThreshold = 0.20f`.
   - `FPAVitalsModel`: Data model tracking current/max values, ghost percentages, timers, and vignette states.
2. **`PAPlayerVitalsWidget.h` / `PAPlayerVitalsWidget.cpp`**:
   - `BindToAttributeSet(const UAscendantAttributeSet* AttributeSet)`
   - `UpdateVitals(float DeltaTime)`
   - `HandlePerfectDodge()`
   - `HandleExhaustionChanged(bool bIsExhausted)`
   - Getters for UI elements: `GetHealthPercent()`, `GetGhostHealthPercent()`, `GetStaminaPercent()`, `GetManaPercent()`, `GetHeartbeatBPM()`.

---

## QA Test Cases

- **Test 1: Attribute Value Mapping**: MaxHealth 100, Health 80 -> HealthPercent = 0.80.
- **Test 2: Ghost Bar Delay**: Take 30 damage -> Health drops to 0.50 immediately; at t = 0.20s Ghost remains 0.80; at t = 0.50s Ghost starts interpolating toward 0.50.
- **Test 3: Perfect Dodge Golden Flash**: Call `HandlePerfectDodge()` -> State becomes `GoldenFlash`; after 0.20s timer State reverts to `Normal`.
- **Test 4: Low Health Heartbeat**: Health = 10 / 100 (10% < 20%) -> `bLowHealthVignetteActive == true`, BPM = 80.
- **Test 5: Exhaustion Tag**: Apply exhaustion -> `bExhaustionVignetteActive == true`, StaminaBarState = `Exhausted`.
