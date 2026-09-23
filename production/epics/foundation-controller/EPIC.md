# Epic: Input & Isometric Camera Controller

> **Layer**: Foundation  
> **GDD**: [`design/gdd/isometric-controller.md`](file:///mnt/Data/Projects/project-games/design/gdd/isometric-controller.md)  
> **Architecture Module**: 2.5D Isometric Controller & Camera ([`docs/architecture/architecture.md`](file:///mnt/Data/Projects/project-games/docs/architecture/architecture.md) Chapter 3)  
> **Status**: Ready  
> **Stories**: 3 Stories Created  

## Overview

This epic implements the core 2.5D isometric player controller and camera system for *Project Ascendant*. It establishes the Unreal Engine 5.7 Enhanced Input mapping contexts for dual-input parity (Keyboard/Mouse and Gamepads), precise 8-way directional locomotion, deterministic screen-to-world cursor raycasting onto an isometric ground plane, and an asynchronous spring-arm camera system with smooth interpolation and player occlusion detection.

## Governing ADRs

| ADR | Decision Summary | Engine Risk |
|---|---|---|
| [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md) | Standardizes Enhanced Input binding to GAS Ability activation and PaperZD character orientation locking. | 🟡 MEDIUM |
| [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md) | Enforces client-side movement prediction with server replay reconciliation to eliminate movement rubberbanding. | 🟡 MEDIUM |

## GDD Requirements

| TR-ID | Requirement | ADR Coverage |
|---|---|---|
| **TR-ctrl-001** | Screen-to-world de-projection from cursor coordinates onto an isometric ground collision plane at fixed angle | Arch Ch. 3, ADR-0002 ✅ |
| **TR-ctrl-002** | 8-way directional character locomotion using Enhanced Input IMC with smooth acceleration and decel curves | Arch Ch. 3, ADR-0002 ✅ |

## Stories

| # | Story | Type | Status | Governing ADR |
|---|---|---|---|---|
| 001 | [`Screen-Relative 8-Way Locomotion & Diagonal Vector Normalization`](story-001-screen-relative-locomotion.md) | Logic | Ready | ADR-0002, ADR-0001 |
| 002 | [`Screen-to-World De-projection & Decoupled Twin-Stick Aiming`](story-002-deprojection-decoupled-aiming.md) | Logic | Ready | ADR-0002 |
| 003 | [`Dynamic Look-Ahead SpringArm Camera & Line-of-Sight Occlusion Dither`](story-003-camera-lookahead-occlusion.md) | Integration | Ready | ADR-0002 |

## Definition of Done

This epic is complete when:
- All stories created from this epic are implemented, reviewed, and closed via `/story-done`
- 8-way movement and aiming respond cleanly within 1 frame (16.67ms) under both Keyboard/Mouse and Gamepad
- Cursor raycast de-projection onto the ground plane maintains perfect accuracy across varying camera zoom levels
- Movement prediction and server reconciliation pass network latency simulation tests (100ms RTT) without visible position jitter
- All acceptance criteria from [`design/gdd/isometric-controller.md`](file:///mnt/Data/Projects/project-games/design/gdd/isometric-controller.md) are verified
- Implementation conforms strictly to [`docs/architecture/control-manifest.md`](file:///mnt/Data/Projects/project-games/docs/architecture/control-manifest.md)

## Next Step

Validate readiness for implementation using `/story-readiness production/epics/foundation-controller/story-001-screen-relative-locomotion.md` then `/dev-story`.
