# Epic: Core Items & DataAssets

> **Layer**: Core  
> **GDD**: [`design/gdd/inventory-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/inventory-system.md)  
> **Architecture Module**: Inventory & Item Database ([`docs/architecture/architecture.md`](file:///mnt/Data/Projects/project-games/docs/architecture/architecture.md) Chapter 3.2 Module 3)  
> **Status**: Ready  
> **Stories**: 1 Story Created  

## Overview

This epic creates the baseline item DataAsset instances (`.uasset`) in `Content/Items/` utilizing the `UItemStaticDataAsset` hierarchy established in Foundation layer (`inv-001` and `inv-003`).

## Governing ADRs

| ADR | Decision Summary | Engine Risk |
|---|---|---|
| [`ADR-0003: Item DataAsset Architecture & Rarity Matrix`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0003-item-dataasset-architecture-rarity.md) | Immutable static data in `UItemStaticDataAsset` instances with dynamic GAS GameplayEffect and Ability class bindings. | 🟢 LOW |

## Stories

| # | Story | Type | Status | Governing ADR |
|---|---|---|---|---|
| 001 | [`Item DataAssets & Quickbar Assets Setup`](story-001-item-data-assets.md) | Asset | Ready | ADR-0003 |
