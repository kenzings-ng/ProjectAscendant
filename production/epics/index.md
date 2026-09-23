# Epics Index: Project Ascendant

> **Last Updated**: 2026-09-17  
> **Engine**: Unreal Engine 5.8 (C++, GAS, Iris, PaperZD)  
> **Current Stage**: Pre-Production  

---

## Foundation Layer Epics (100% Complete)

| Epic | Layer | System | Design Document (GDD) | Governing ADRs | Stories Status | Epic Status |
|---|---|---|---|---|---|---|
| [`foundation-attributes`](foundation-attributes/EPIC.md) | Foundation | Character Attributes & Stats Engine (GAS) | [`attributes-system.md`](../../design/gdd/attributes-system.md) | ADR-0001, ADR-0002 | 3 stories | **Done** |
| [`foundation-controller`](foundation-controller/EPIC.md) | Foundation | Input & Isometric Camera Controller | [`isometric-controller.md`](../../design/gdd/isometric-controller.md) | ADR-0001, ADR-0002 | 3 stories | **Done** |
| [`foundation-inventory`](foundation-inventory/EPIC.md) | Foundation | Inventory & 5-Tier Item Database | [`inventory-system.md`](../../design/gdd/inventory-system.md) | ADR-0001, ADR-0003 | 3 stories | **Done** |
| [`foundation-netcode`](foundation-netcode/EPIC.md) | Foundation | Open World MMO Netcode & Contested Aggro Sync | [`multiplayer-coop.md`](../../design/gdd/multiplayer-coop.md) | ADR-0001 | 4 stories | **Done** |

---

## Core Layer Epics (Sprint 2 - 100% Complete)

| Epic | Layer | System | Design Document (GDD) | Governing ADRs | Stories Status | Epic Status |
|---|---|---|---|---|---|---|
| [`core-world`](core-world/EPIC.md) | Core | Raw Map Blockout & Zone Volumes | [`isometric-controller.md`](../../design/gdd/isometric-controller.md), [`multiplayer-coop.md`](../../design/gdd/multiplayer-coop.md) | ADR-0001, ADR-0002 | 2 stories | **Done** |
| [`core-character`](core-character/EPIC.md) | Core | Paper2D Slicing & PaperZD AnimBP State Machine | [`game-concept.md`](../../design/gdd/game-concept.md), [`isometric-controller.md`](../../design/gdd/isometric-controller.md) | ADR-0002 | 3 stories | **Done** |
| [`core-combat`](core-combat/EPIC.md) | Core | GAS Dash I-Frame & 3-Hit Combo / Posture Finisher | [`combat-system.md`](../../design/gdd/combat-system.md), [`attributes-system.md`](../../design/gdd/attributes-system.md) | ADR-0001, ADR-0002 | 2 stories | **Done** |
| [`core-items`](core-items/EPIC.md) | Core | Item DataAssets & Quickbar Consumables | [`inventory-system.md`](../../design/gdd/inventory-system.md) | ADR-0003 | 1 story | **Done** |

---

## Expansion Layer Epics (Sprint 3 - 100% Complete)

| Epic | Layer | System | Design Document (GDD) | Governing ADRs | Stories Status | Epic Status |
|---|---|---|---|---|---|---|
| [`expansion-economy`](expansion-economy/EPIC.md) | Expansion | Dual Currency Economy & Merchant Network | [`merchant-economy.md`](../../design/gdd/merchant-economy.md) | ADR-0001, ADR-0003 | 3 stories | **Done** |
| [`expansion-crafting`](expansion-crafting/EPIC.md) | Expansion | 3-Tier Blacksmithing & Enhancement Forge | [`blacksmithing-system.md`](../../design/gdd/blacksmithing-system.md) | ADR-0001, ADR-0003 | 3 stories | **Done** |
| [`expansion-progression`](expansion-progression/EPIC.md) | Expansion | Character XP, Leveling & Class Mastery | [`skill-progression-system.md`](../../design/gdd/skill-progression-system.md) | ADR-0001, ADR-0002 | 2 stories | **Done** |

---

## Presentation & Interface Layer Epics (Sprint 4 - Production Active)

| Epic | Layer | System | Design Document (GDD) | Governing ADRs | Stories Status | Epic Status |
|---|---|---|---|---|---|---|
| [`presentation-ui`](presentation-ui/EPIC.md) | Presentation | Combat HUD, Boss Vitals & Interactive Windows | [`combat-hud.md`](../../design/gdd/combat-hud.md) | ADR-0001, ADR-0002, ADR-0003 | 4 stories | **Done** |

---

## Encounter Layer Epics (Sprint 5 - Production Active)

| Epic | Layer | System | Design Document (GDD) | Governing ADRs | Stories Status | Epic Status |
|---|---|---|---|---|---|---|
| [`encounter-boss`](encounter-boss/EPIC.md) | Encounter | Boss AI, Stagger & Execution, Part Breaking & Perfect Evasion | [`boss-ai.md`](../../design/gdd/boss-ai.md), [`stagger-system.md`](../../design/gdd/stagger-system.md), [`dash-evasion.md`](../../design/gdd/dash-evasion.md) | ADR-0001, ADR-0002 | 4 stories | **Done** |

---

## World Integration & Auth Layer Epics (Sprint 6 - Production Active)

| Epic | Layer | System | Design Document (GDD) | Governing ADRs | Stories Status | Epic Status |
|---|---|---|---|---|---|---|
| [`world-zone-auth`](world-zone-auth/EPIC.md) | World / Auth | Seamless Zones, Campfire Sanctuaries, Karma, DDS & Account Auth | [`zone-system.md`](../../design/gdd/zone-system.md), [`authentication-account-system.md`](../../design/gdd/authentication-account-system.md) | ADR-0001, ADR-0003 | 4 stories | **Ready** |


