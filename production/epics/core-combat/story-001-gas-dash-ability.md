# Story 001: GAS Dash Ability & Invulnerability Window

> **Epic**: Core Combat & Abilities  
> **Status**: Complete  
> **Layer**: Core  
> **Type**: Logic  
> **Estimate**: 8 hours (L)  
> **Manifest Version**: 2026-09-17  
> **Last Updated**: 2026-09-17  

## Context

**GDD**: [`design/gdd/combat-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/combat-system.md), [`design/gdd/attributes-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/attributes-system.md)  
**Requirement**: `TR-cmbt-003`  
**ADR Governing Implementation**: [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md)  

**Engine**: Unreal Engine 5.8 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Implements `UPAGameplayAbility_Dash` inheriting from `UGameplayAbility`. Coordinates with `UCharacterMovementComponent` root motion or velocity impulse.

---

## Acceptance Criteria

- [x] **AC-1 (Stamina Cost & Validation)**: Dash costs 25 Stamina. If current Stamina < 25 or character possesses `State.Exhausted`, activation is strictly blocked on both client and server.
- [x] **AC-2 (I-Frame Invulnerability Window)**: Applies `State.Invulnerable` tag at $t = 0.05\text{s}$ through $t = 0.25\text{s}$ (0.20s window) during the total 0.35s dash duration. Any incoming damage during this window is mitigated 100%.
- [x] **AC-3 (Impulse & Cooldown)**: Launches character 450 units in the current movement or aim direction over 0.35s with linear decay. Applies 0.5s cooldown tag `Cooldown.Dash` upon ability end.
- [x] **AC-4 (Ghost Trail Delegate)**: Fires delegate `OnDashExecuted` allowing visual components to spawn pixel ghost afterimages along the dash vector.
