# Epic: Core World & Level Blockout

> **Layer**: Core  
> **GDD**: [`design/gdd/isometric-controller.md`](file:///mnt/Data/Projects/project-games/design/gdd/isometric-controller.md), [`design/gdd/game-concept.md`](file:///mnt/Data/Projects/project-games/design/gdd/game-concept.md)  
> **Architecture Module**: World Partition & Map ([`docs/architecture/architecture.md`](file:///mnt/Data/Projects/project-games/docs/architecture/architecture.md) Chapter 3.3 Module 8)  
> **Status**: Ready  
> **Stories**: 2 Stories Created  

## Overview

This epic delivers the primary 3D Graybox testing arena (`L_Ruins_Blockout.umap`) for *Project Ascendant*. It establishes the fixed isometric perspective (Pitch $-45^\circ$, Yaw $+45^\circ$, SpringArm 1400 units), ground plane geometry, occlusion dither test pillars, NavMeshBounds, player start locations, and zone volumes (`APASanctuaryVolume` for safezones and Leash thresholds for monster tethering).

## Governing ADRs

| ADR | Decision Summary | Engine Risk |
|---|---|---|
| [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) | Standardizes 2.5D camera angle (-45° pitch, +45° yaw) and sprite billboard alignment with 3D environment geometry. | 🟡 MEDIUM |
| [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md) | Enforces server-authoritative leash validation (2500cm radius) and zone boundary synchronization. | 🟡 MEDIUM |

## Stories

| # | Story | Type | Status | Governing ADR |
|---|---|---|---|---|
| 001 | [`Raw Map Blockout & Isometric Camera Setup`](story-001-map-blockout-camera.md) | Integration | Ready | ADR-0002 |
| 002 | [`Sanctuary & Combat Zone Volumes`](story-002-sanctuary-leash-volumes.md) | Logic | Ready | ADR-0001 |

## Definition of Done

- Map `L_Ruins_Blockout.umap` exists in `ProjectAscendant/Content/Maps/` and loads without errors.
- Player spawns at `PlayerStart`, camera locks to $-45^\circ / +45^\circ$, and movement responds smoothly.
- Tall pillars trigger Line-of-Sight occlusion dither fading when obstructing the character.
- Sanctuary and Leash volumes correctly trigger state changes on character and AI.
