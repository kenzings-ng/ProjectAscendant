# Epic: Core Character & PaperZD Integration

> **Layer**: Core  
> **GDD**: [`design/gdd/game-concept.md`](file:///mnt/Data/Projects/project-games/design/gdd/game-concept.md), [`design/gdd/isometric-controller.md`](file:///mnt/Data/Projects/project-games/design/gdd/isometric-controller.md)  
> **Architecture Module**: PaperZD Animation & Character Visuals ([`docs/architecture/architecture.md`](file:///mnt/Data/Projects/project-games/docs/architecture/architecture.md) Chapter 3.3 Module 4)  
> **Status**: Ready  
> **Stories**: 3 Stories Created  

## Overview

This epic integrates 2D pixel art character and boss spritesheets into Unreal Engine 5.8 using Paper2D and PaperZD plugins. It extracts sprites into frame-accurate PaperFlipbooks (Idle, Run, Attack, Dash, Hurt, Death) and builds the PaperZD Animation Blueprint (`ABP_Vanguard`, `ABP_StoneGolem`) with complete state machine transitions, directional velocity driving, and animation notifies for combat hitboxes.

## Governing ADRs

| ADR | Decision Summary | Engine Risk |
|---|---|---|
| [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) | Uses PaperZD Animation Blueprints, PaperFlipbooks with nearest-neighbor pixel filtering, and AnimNotify-driven combat hitbox timing. | 🟡 MEDIUM |

## Stories

| # | Story | Type | Status | Governing ADR |
|---|---|---|---|---|
| 001 | [`Paper2D Sprite Extraction & Flipbooks`](story-001-paper2d-flipbooks.md) | Asset | Ready | ADR-0002 |
| 002 | [`PaperZD AnimBP & State Machine Setup`](story-002-paperzd-animbp.md) | Integration | Ready | ADR-0002 |
| 003 | [`Stone Golem Boss PaperZD AnimBP & Aggro Integration`](story-003-boss-paperzd-animbp.md) | Integration | Ready | ADR-0002 |
