# Story 002: Sanctuary & Combat Zone Volumes

> **Epic**: Core World & Level Blockout  
> **Status**: Complete  
> **Layer**: Core  
> **Type**: Logic  
> **Estimate**: 8 hours (L)  
> **Manifest Version**: 2026-09-17  
> **Last Updated**: 2026-09-17  

## Context

**GDD**: [`design/gdd/multiplayer-coop.md`](file:///mnt/Data/Projects/project-games/design/gdd/multiplayer-coop.md), [`design/gdd/game-concept.md`](file:///mnt/Data/Projects/project-games/design/gdd/game-concept.md)  
**Requirement**: `TR-world-001`, `TR-world-002`  
**ADR Governing Implementation**: [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md)  

**Engine**: Unreal Engine 5.8 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Implements `APASanctuaryVolume` and Leash boundary subsystem checking actor distance relative to spawn origin.

---

## Acceptance Criteria

- [x] **AC-1 (Sanctuary Volume Safezone)**: `APASanctuaryVolume` actor with 1000cm collision radius applies `State.InSanctuary` tag on overlapping players. Inside Sanctuary, players are immune to damage and offensive ability activation is blocked.
- [x] **AC-2 (Monster Leash Boundary)**: Any hostile monster/boss pulled beyond 2500cm from its spawn origin enters `LeashReset` state, drops aggro, becomes immune to damage, and runs back to origin at 1.5x walk speed, regenerating 100% Posture and Health.
- [x] **AC-3 (Zone Transition Replication)**: Entering or exiting a zone volume triggers Iris-replicated tag notifications to client HUD (`Zone.Sanctuary`, `Zone.Wilderness`).
