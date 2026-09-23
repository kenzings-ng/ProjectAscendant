# Story 003: Stone Golem Boss PaperZD AnimBP & Aggro Integration

> **Epic**: Core Character & PaperZD Integration  
> **Status**: Complete  
> **Layer**: Core  
> **Type**: Integration  
> **Estimate**: 8 hours (L)  
> **Manifest Version**: 2026-09-17  
> **Last Updated**: 2026-09-17  

## Context

**GDD**: [`design/gdd/combat-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/combat-system.md), [`design/gdd/game-concept.md`](file:///mnt/Data/Projects/project-games/design/gdd/game-concept.md), [`design/gdd/multiplayer-coop.md`](file:///mnt/Data/Projects/project-games/design/gdd/multiplayer-coop.md)  
**Requirement**: `TR-art-004`, `TR-boss-001`  
**ADR Governing Implementation**: [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md)  

**Engine**: Unreal Engine 5.8 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Implements `APAStoneGolemBoss` character class with `UPAMonsterLeashComponent`, `UPAPaperZDAnimInstance` state machine (`ABP_StoneGolem` / `AS_StoneGolem`) linked to boss combat flipbooks (Idle, Walk, Slam, Stagger, Death).

---

## Acceptance Criteria

- [x] **AC-1 (Stone Golem Boss PaperZD State Machine)**:
  - Animation Source `AS_StoneGolem` and AnimBP `ABP_StoneGolem` in `/Game/art/characters/boss/anim/`:
    - `Idle` state: Plays `FB_Golem_Idle` @ 10.0 FPS when `Speed <= 10.0f`.
    - `Walk` state: Plays `FB_Golem_Walk` @ 10.0 FPS when `Speed > 10.0f`.
    - `Slam` state: Plays `FB_Golem_Slam` @ 10.0 FPS when `State.Attacking` or Slam ability is triggered.
    - `Stagger` state: Plays `FB_Golem_Stagger` @ 10.0 FPS when `State.Broken` or `State.Stunned`.
    - `Death` state: Plays `FB_Golem_Death` @ 10.0 FPS when `State.Dead`.
- [x] **AC-2 (Boss AI Aggro & Leash Integration)**:
  - `APAStoneGolemBoss` initializes `UPAMonsterLeashComponent` with `LeashRadius = 2500.0f`, `MaxWalkSpeed = 220.0f`, and `LeashReturnSpeed = 330.0f`.
  - Detects nearest player within `AggroRadius = 800.0f`, acquiring threat and moving towards target.
  - Drops aggro and executes Leash reset if pulled > 2500.0f from spawn origin.
- [x] **AC-3 (Ground Slam Hitbox Window & Damage Execution)**:
  - Boss executes Ground Slam attack when target distance <= 250.0f with 3.0s cooldown.
  - Deals 35 base physical damage and 30 Posture damage in a 300cm AoE radius during active impact frame.

---

## Completion Notes
**Completed**: 2026-09-18  
**Criteria**: 3/3 passing  
**Deviations**: None  
**Test Evidence**: Integration tests at `ProjectAscendant/Source/ProjectAscendant/Private/Character/boss_paperzd_aggro_test.cpp` and `ProjectAscendant/Source/ProjectAscendant/Private/Combat/code_review_fixes_regression_test.cpp` (Unreal Engine Automation Testing: 100% Pass)  
**Code Review**: Complete (All 6 review findings resolved with zero warnings)  
