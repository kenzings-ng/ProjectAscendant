# Epic: Core Combat & Abilities

> **Layer**: Core  
> **GDD**: [`design/gdd/combat-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/combat-system.md), [`design/gdd/attributes-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/attributes-system.md)  
> **Architecture Module**: Combat & Evasion ([`docs/architecture/architecture.md`](file:///mnt/Data/Projects/project-games/docs/architecture/architecture.md) Chapter 3.3 Module 4 & 5)  
> **Status**: Ready  
> **Stories**: 2 Stories Created  

## Overview

This epic implements the player's core combat abilities using the Gameplay Ability System (GAS): `GA_Dash` (0.20s Invulnerability I-frame window, 25 Stamina cost, Ghost Trail delegate) and `GA_LightAttack` (3-hit combo chain, PaperZD notify hitbox detection, 1.2s reset window, and 1.5s Posture Finisher execution).

## Governing ADRs

| ADR | Decision Summary | Engine Risk |
|---|---|---|
| [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) | Standardizes GameplayAbility activations linked to Enhanced Input actions and PaperZD animation play montage tasks. | 🟡 MEDIUM |
| [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md) | Enforces 200ms lag compensation rewind buffer validation for attack hit detection on dedicated server. | 🟡 MEDIUM |

## Stories

| # | Story | Type | Status | Governing ADR |
|---|---|---|---|---|
| 001 | [`GAS Dash Ability & Invulnerability Window`](story-001-gas-dash-ability.md) | Logic | Ready | ADR-0002 |
| 002 | [`GAS 3-Hit Combo & Posture Finisher Execution`](story-002-gas-combo-finisher.md) | Logic | Ready | ADR-0001, ADR-0002 |
