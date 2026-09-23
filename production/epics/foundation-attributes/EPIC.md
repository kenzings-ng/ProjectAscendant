# Epic: Character Attributes & Stats Engine (GAS)

> **Layer**: Foundation  
> **GDD**: [`design/gdd/attributes-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/attributes-system.md)  
> **Architecture Module**: Character, Attributes & Gameplay Ability System ([`docs/architecture/architecture.md`](file:///mnt/Data/Projects/project-games/docs/architecture/architecture.md) Chapter 2)  
> **Status**: Ready  
> **Stories**: 3 Stories Created  

## Overview

This epic implements the foundational character attributes and statistics engine for *Project Ascendant* using Unreal Engine 5.7's Gameplay Ability System (GAS). It establishes the core `UAttributeSet` classes governing Health, Mana, Stamina, and Posture, non-linear defense calculation pipelines (`UGameplayEffectExecutionCalculation`), and client-side optimistic resource prediction with server-authoritative reconciliation over Iris network replication.

## Governing ADRs

| ADR | Decision Summary | Engine Risk |
|---|---|---|
| [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) | Standardizes GAS AbilitySystemComponent attachment to 2.5D PaperZD character pawns, GameplayEffect execution calculations, and attribute replication tags. | 🟡 MEDIUM |
| [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md) | Enforces Dedicated Server authority for all attribute mutations and network replication channels. | 🟡 MEDIUM |

## GDD Requirements

| TR-ID | Requirement | ADR Coverage |
|---|---|---|
| **TR-attr-001** | Character stats (Health, Mana, Stamina, Posture) implemented via GAS `UAttributeSet` with Iris network replication | ADR-0002, Arch Ch. 2 ✅ |
| **TR-attr-002** | Stat calculation pipelines using `UGameplayEffectExecutionCalculation` for non-linear defense and resistance scaling | ADR-0002, Arch Ch. 2 ✅ |
| **TR-attr-003** | Client-side optimistic resource prediction with server-authoritative reconciliation on mispredicted depletion | ADR-0001, ADR-0002 ✅ |

## Stories

| # | Story | Type | Status | Governing ADR |
|---|---|---|---|---|
| 001 | [`Core AttributeSet Definition & Iris Network Replication`](story-001-attributeset-replication.md) | Logic | Ready | ADR-0002, ADR-0001 |
| 002 | [`Non-Linear Damage & Posture Execution Calculations`](story-002-damage-posture-calculations.md) | Logic | Ready | ADR-0002 |
| 003 | [`Stamina Regeneration, Depletion & Exhaustion State Pipeline`](story-003-stamina-exhaustion-pipeline.md) | Logic | Ready | ADR-0002, ADR-0001 |

## Definition of Done

This epic is complete when:
- All stories created from this epic are implemented, reviewed, and closed via `/story-done`
- All acceptance criteria from [`design/gdd/attributes-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/attributes-system.md) are verified
- Core attribute formula calculations (damage mitigation, posture break thresholds) pass automated unit tests in `tests/unit/combat/CombatFormulasTest.cpp`
- Replication of attributes over headless dedicated server confirms zero desyncs during network latency simulation
- All implementation code adheres to [`docs/architecture/control-manifest.md`](file:///mnt/Data/Projects/project-games/docs/architecture/control-manifest.md)

## Next Step

Validate readiness for implementation using `/story-readiness production/epics/foundation-attributes/story-001-attributeset-replication.md` then `/dev-story`.
