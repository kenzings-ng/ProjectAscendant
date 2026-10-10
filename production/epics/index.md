# Epics Index: Project Ascendant

> **Last Updated**: 2026-10-10 (X14: đối chiếu trạng thái; bản trước 2026-09-23)  
> **X14 (2026-10-10)**: Các nhãn "100% Complete" và "Done" trước đây không có bằng chứng (rà soát R4, M19). Trạng thái dưới đây suy ra từ story file, theo bảng [`x14-status-reconciliation.md`](../qa/x14-status-reconciliation.md). Từ vựng trạng thái: Not Started / Ready / In Progress / Review / Complete (theo `production/sprint-status.yaml`) và Removed.  
> **Engine**: Unreal Engine 5.8 (C++, GAS, Iris, PaperZD)  
> **Current Stage**: Production  

---

## Foundation Layer Epics (X14 2026-10-10: In Progress — trước đây ghi 100% Complete)

| Epic | Layer | System | Design Document (GDD) | Governing ADRs | Stories Status | Epic Status |
|---|---|---|---|---|---|---|
| [`foundation-attributes`](foundation-attributes/EPIC.md) | Foundation | Character Attributes & Stats Engine (GAS) | [`attributes-system.md`](../../design/gdd/attributes-system.md) | ADR-0001, ADR-0002 | 3 stories (In Progress 3) | **In Progress** *(trước đây **Done**)* |
| [`foundation-controller`](foundation-controller/EPIC.md) | Foundation | Input & Isometric Camera Controller | [`isometric-controller.md`](../../design/gdd/isometric-controller.md) | ADR-0001, ADR-0002 | 3 stories (In Progress 3) | **In Progress** *(trước đây **Done**)* |
| [`foundation-inventory`](foundation-inventory/EPIC.md) | Foundation | Inventory & 5-Tier Item Database | [`inventory-system.md`](../../design/gdd/inventory-system.md) | ADR-0001, ADR-0003 | 3 stories (In Progress 3) | **In Progress** *(trước đây **Done**)* |
| [`foundation-netcode`](foundation-netcode/EPIC.md) | Foundation | Open World MMO Netcode & Contested Aggro Sync | [`multiplayer-coop.md`](../../design/gdd/multiplayer-coop.md) | ADR-0001 | 4 stories (In Progress 4) | **In Progress** *(trước đây **Done**)* |

---

## Core Layer Epics (Sprint 2 — X14 2026-10-10: In Progress — trước đây ghi 100% Complete)

| Epic | Layer | System | Design Document (GDD) | Governing ADRs | Stories Status | Epic Status |
|---|---|---|---|---|---|---|
| [`core-world`](core-world/EPIC.md) | Core | Raw Map Blockout & Zone Volumes | [`isometric-controller.md`](../../design/gdd/isometric-controller.md), [`multiplayer-coop.md`](../../design/gdd/multiplayer-coop.md) | ADR-0001, ADR-0002 | 2 stories (In Progress 2) | **In Progress** *(trước đây **Done**)* |
| [`core-character`](core-character/EPIC.md) | Core | Paper2D Slicing & PaperZD AnimBP State Machine | [`game-concept.md`](../../design/gdd/game-concept.md), [`isometric-controller.md`](../../design/gdd/isometric-controller.md) | ADR-0002 | 3 stories (In Progress 2, Review 1) | **In Progress** *(trước đây **Done**)* |
| [`core-combat`](core-combat/EPIC.md) | Core | GAS Dash I-Frame & 3-Hit Combo / Posture Finisher | [`combat-system.md`](../../design/gdd/combat-system.md), [`attributes-system.md`](../../design/gdd/attributes-system.md) | ADR-0001, ADR-0002 | 2 stories (In Progress 2) | **In Progress** *(trước đây **Done**)* |
| [`core-items`](core-items/EPIC.md) | Core | Item DataAssets & Quickbar Consumables | [`inventory-system.md`](../../design/gdd/inventory-system.md) | ADR-0003 | 1 story (Complete 1) | **Complete** *(trước đây **Done**)* |

---

## Expansion Layer Epics (Sprint 3 — X14 2026-10-10: In Progress — trước đây ghi 100% Complete)

| Epic | Layer | System | Design Document (GDD) | Governing ADRs | Stories Status | Epic Status |
|---|---|---|---|---|---|---|
| [`expansion-economy`](expansion-economy/EPIC.md) | Expansion | Dual Currency Economy & Merchant Network | [`merchant-economy.md`](../../design/gdd/merchant-economy.md) | ADR-0001, ADR-0003 | 3 stories (In Progress 2, Review 1) | **In Progress** *(trước đây **Done**)* |
| [`expansion-crafting`](expansion-crafting/EPIC.md) | Expansion | 3-Tier Blacksmithing & Enhancement Forge | [`blacksmithing-system.md`](../../design/gdd/blacksmithing-system.md) | ADR-0001, ADR-0003 | 3 stories (In Progress 3) | **In Progress** *(trước đây **Done**)* |
| [`expansion-progression`](expansion-progression/EPIC.md) | Expansion | Character XP, Leveling & Class Mastery | [`skill-progression-system.md`](../../design/gdd/skill-progression-system.md) | ADR-0001, ADR-0002 | 2 stories (In Progress 1; prog-002 Talent Tree: Removed — DECISIONS §12, 2026-10-10) | **In Progress** *(trước đây **Done**)* |

---

## Presentation & Interface Layer Epics (Sprint 4 — X14 2026-10-10: In Progress — trước đây ghi 100% Complete)

| Epic | Layer | System | Design Document (GDD) | Governing ADRs | Stories Status | Epic Status |
|---|---|---|---|---|---|---|
| [`presentation-ui`](presentation-ui/EPIC.md) | Presentation | Combat HUD, Boss Vitals & Interactive Windows | [`combat-hud.md`](../../design/gdd/combat-hud.md) | ADR-0001, ADR-0002, ADR-0003 | 4 stories (In Progress 2, Review 2) | **In Progress** *(trước đây **Done**)* |

---

## Encounter Layer Epics (Sprint 5 — X14 2026-10-10: In Progress — trước đây ghi 100% Complete)

| Epic | Layer | System | Design Document (GDD) | Governing ADRs | Stories Status | Epic Status |
|---|---|---|---|---|---|---|
| [`encounter-boss`](encounter-boss/EPIC.md) | Encounter | Boss AI, Stagger & Execution, Part Breaking & Perfect Evasion | [`boss-ai.md`](../../design/gdd/boss-ai.md), [`stagger-system.md`](../../design/gdd/stagger-system.md), [`dash-evasion.md`](../../design/gdd/dash-evasion.md) | ADR-0001, ADR-0002 | 4 stories (In Progress 4) | **In Progress** *(trước đây **Done**)* |

---

## World Integration & Auth Layer Epics (Sprint 6 — X14 2026-10-10: In Progress — trước đây ghi 100% Complete)

| Epic | Layer | System | Design Document (GDD) | Governing ADRs | Stories Status | Epic Status |
|---|---|---|---|---|---|---|
| [`world-zone-auth`](world-zone-auth/EPIC.md) | World / Auth | Seamless Zones, Citadel Safe Zones, Karma, DDS & Account Auth | [`zone-system.md`](../../design/gdd/zone-system.md), [`authentication-account-system.md`](../../design/gdd/authentication-account-system.md) | ADR-0001, ADR-0003 | 4 stories (In Progress 4) | **In Progress** *(trước đây **Done**)* |

---

## Character & Visual Presentation Layer Epics (Sprint 7 — X14 2026-10-10: In Progress — trước đây ghi Code Complete, Art in Placeholder-Tier)

| Epic | Layer | System | Design Document (GDD) | Governing ADRs | Stories Status | Epic Status |
|---|---|---|---|---|---|---|
| [`presentation-character-visual`](presentation-character-visual/epic-overview.md) | Presentation | Character & NPC Visual Identity, 4 Master Rigs, 7 Weapon Families, 16 Class Idles & Civilian NPCs | [`character-visual-system.md`](../../design/gdd/character-visual-system.md) | SPEC-ART-2026-09-23-V2 | 7 stories (In Progress 3, Review 4); art proxy chờ thay art thật | **In Progress** *(trước đây **In Progress (Placeholder Art)**)* |


