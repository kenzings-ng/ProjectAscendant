# Story 002: PaperZD AnimBP & State Machine Setup

> **Epic**: Core Character & PaperZD Integration  
> **Status**: Complete  
> **Layer**: Core  
> **Type**: Integration  
> **Estimate**: 8 hours (L)  
> **Manifest Version**: 2026-09-17  
> **Last Updated**: 2026-09-17  

## Context

**GDD**: [`design/gdd/isometric-controller.md`](file:///mnt/Data/Projects/project-games/design/gdd/isometric-controller.md), [`design/gdd/game-concept.md`](file:///mnt/Data/Projects/project-games/design/gdd/game-concept.md)  
**Requirement**: `TR-art-003`, `TR-ctrl-001`  
**ADR Governing Implementation**: [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md)  

**Engine**: Unreal Engine 5.8 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Builds `UPaperZDAnimInstance` subclass or blueprint (`ABP_Vanguard`) driven by character speed and gameplay tags (`State.Dashing`, `State.Attacking`, `State.Stunned`, `State.Dead`).

---

## Acceptance Criteria

- [x] **AC-1 (State Machine Architecture)**: `ABP_Vanguard` contains states: `Idle`, `Locomotion`, `Dash`, `LightAttack_Combo`, `Hurt`, `Stunned`, `Dead`. Transitions evaluate `Speed > 10.0f` and GAS tags.
- [x] **AC-2 (Directional Sprite Flipping)**: Horizontal facing flips character sprite component rotation (`Yaw 0` for right, `Yaw 180` for left) based on aim cursor angle or movement direction.
- [x] **AC-3 (Combat Anim Notifies)**: `FB_Vanguard_Attack1` triggers custom PaperZD AnimNotify `AnimNotify_ActiveHitbox` at frame 2 to start collision trace, and `AnimNotify_EndHitbox` at frame 3.
