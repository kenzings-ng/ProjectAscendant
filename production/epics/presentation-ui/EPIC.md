# Epic: Presentation & Interface Layer (Combat HUD & Interactive UI)

> **Layer**: Presentation  
> **GDD**: [`design/gdd/combat-hud.md`](../../design/gdd/combat-hud.md), [`design/gdd/merchant-economy.md`](../../design/gdd/merchant-economy.md), [`design/gdd/blacksmithing-system.md`](../../design/gdd/blacksmithing-system.md)  
> **Architecture Module**: UI & Player Experience Architecture ([`docs/architecture/architecture.md`](../../docs/architecture/architecture.md))  
> **Status**: ✅ Complete  
> **Stories**: 4/4 Stories Done  

## Overview

This epic implements the minimal diegetic combat HUD, Boss health/posture encounter interface, floating combat text system, and interactive transaction windows (Merchant Shop & Blacksmith Forge) for *Project Ascendant*. Built with Unreal Engine 5 UMG, Slate, and CommonUI, it guarantees crisp 2.5D isometric visibility, zero input lag, and responsive player feedback.

## Governing ADRs

| ADR | Decision Summary | Engine Risk |
|---|---|---|
| [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md) | HUD binds to replicated state without client-side speculation on currency or inventory transactions. | 🟢 LOW |
| [`ADR-0002: GAS Integration & PaperZD Pixel Sprites`](../../docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) | Direct event-driven bindings from `UAscendantAttributeSet` and Gameplay Tag listeners to avoid polling ticks. | 🟢 LOW |
| [`ADR-0003: Server-Authoritative Grid Inventory via FFastArraySerializer`](../../docs/architecture/adr-0003-server-authoritative-grid-inventory-fast-array.md) | Shop & Forge UI reflect FastArray delta updates directly. | 🟢 LOW |

## GDD Requirements

| TR-ID | Requirement | ADR Coverage |
|---|---|---|
| **TR-hud-001** | Player Vitals HUD: HP, Stamina, Mana with Catch-up Ghost Bar, Golden Flash on Perfect Dodge, Low-HP/Exhaustion vignette | ADR-0002 ✅ |
| **TR-hud-002** | Boss Encounter HUD: 3-phase HP bar, Posture break flashing meter, broken parts status, World-to-Screen Execution Reticle | ADR-0001, ADR-0002 ✅ |
| **TR-hud-003** | Floating Combat Text: Damage numbers (normal, crit 1.5x bounce, posture gold) with radial scatter arc and "PERFECT!" dodge text | ADR-0002 ✅ |
| **TR-hud-004** | Interactive Windows: 2-column Merchant Shop with Buyback & Wanted surcharge; Blacksmith Forge Anvil with 0.8s hold-to-craft | ADR-0001, ADR-0003 ✅ |

## Stories

| # | Story | Type | Status | Governing ADR |
|---|---|---|---|---|
| 001 | [`Player Vitals & Status HUD Engine`](story-001-player-vitals-hud.md) | UI | ✅ Done | ADR-0002 |
| 002 | [`Boss Health, Stagger Posture & Execution Reticle`](story-002-boss-stagger-reticle.md) | UI | ✅ Done | ADR-0001, ADR-0002 |
| 003 | [`Floating Combat Text & Action Feedback`](story-003-floating-combat-text.md) | UI | ✅ Done | ADR-0002 |
| 004 | [`Merchant Shop & Blacksmith Forge Interactive Windows`](story-004-shop-forge-windows.md) | UI | ✅ Done | ADR-0001, ADR-0003 |

## Definition of Done

This epic is complete when:
- Player vitals reflect GAS attributes instantly with ghost bar smoothing and vignette effects.
- Boss health bar clearly indicates 75% and 25% phase changes, posture break at 100%, and execution prompt.
- Damage numbers scatter naturally without overlapping or tanking frame rate (> 60 FPS target).
- Merchant and Forge windows correctly interface with `UPAMerchantComponent` and `UPABlacksmithComponent`.
- 100% automated tests for HUD models and event handlers pass.
