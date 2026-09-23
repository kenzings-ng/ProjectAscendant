# Story 003: Wilderness Wandering Smuggler & Limited Stock

> **Epic**: Expansion Economy & Merchant Network  
> **Status**: In Progress  
> **Layer**: Expansion  
> **Type**: World  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-19  
> **Last Updated**: 2026-09-19  

## Context

**GDD**: [`design/gdd/merchant-economy.md`](../../design/gdd/merchant-economy.md)  
**Requirement**: `TR-econ-003` (GDD §Tier 2: Thương Nhân Lang Thang)

**ADR Governing Implementation**:
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Server-Authoritative transactions, distance check ≤ 300 cm, out-of-combat enforcement, rate-limiting)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `APAWanderingSmuggler` (`AActor` + `UPAMerchantComponent` + `UPARestockComponent`). Restock timer driven by server-side `FTimerHandle` via `GetWorldTimerManager()`. All inventory mutations remain on `UPAMerchantComponent` from econ-002.

**Control Manifest Rules (Expansion Layer)**:
- Required: Server-authoritative validation for all purchase/sell operations (delegated to `UPAMerchantComponent`).
- Required: Restock executes exclusively on Dedicated Server and replicates stock counts to clients via existing `FPAMerchantCatalogEntry.AvailableStock` replication.
- Required: Karma surcharge (`wanted_surcharge_ratio = 0.20`) applied server-side for buyers with `Karma < -50`.
- Forbidden: Client-side stock prediction or speculative restock animations without server confirmation.
- Guardrail: Rate-limit (2 transactions/second/player) enforced at `UPAMerchantComponent` layer (econ-002).

---

## Acceptance Criteria

- [ ] **AC-1 (Fixed Catalog — Unlimited Stock)**: `APAWanderingSmuggler` exposes a fixed catalog via `UPAMerchantComponent` containing: Greater Health Flask (150 Gold, unlimited), Greater Mana Flask (150 Gold, unlimited), Blacksmith Ward (`item_blacksmith_ward`, 800 Gold, unlimited). Buying any fixed item deducts Gold atomically.
- [ ] **AC-2 (Rotating Catalog — Limited Stock & Restock Timer)**: `UPARestockComponent` manages 4 rotating slots populated from a configurable pool (`RotatingStockPool`). Each slot has an initial stock of 1–3 units. When a slot reaches 0, it displays `OutOfStock` until the next restock cycle (60 minutes real-time, `restock_timer_tier2 = 3600s`). On restock, all 4 slots are re-rolled from the pool with fresh stock counts (server-wide synchronization — all instances restock at the same world-time interval, not per-player).
- [ ] **AC-3 (Karma Surcharge for Wanted Players)**: When a player with `Karma < -50` buys from the Wandering Smuggler, the effective price is `ceil(base_price * 1.20)` (20% surcharge). Non-wanted players pay base price. The check is performed server-side on every purchase; no UI change on client until confirmed.
- [ ] **AC-4 (Interaction & Proximity Guardrails)**: All trading enforces ≤ 300 cm interaction distance and `In-Combat == false` via `UPAMerchantComponent::ValidateInteraction` (econ-002 contract). Shop auto-closes when player moves > 500 cm away or enters combat. Tier 2 never rejects based on Karma — only applies surcharge.

---

## Implementation Notes

1. **`PARestockComponent.h` / `PARestockComponent.cpp`** (new):
   - `UPARestockComponent : public UActorComponent`
   - `float RestockIntervalSeconds = 3600.0f` — configurable per-Tier.
   - `FTimerHandle RestockTimerHandle` — started in `BeginPlay()` if `HasAuthority()`.
   - `void ExecuteRestock(UPAMerchantComponent* MerchantComp)` — re-rolls rotating slots, resets `AvailableStock`, broadcasts `OnRestocked`.
   - `float GetTimeUntilNextRestock() const` — for client UI countdown (replicated via property).
   - `UPROPERTY(Replicated) float NextRestockServerTime` — world time of next restock, replicated to clients.
   - `DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPAOnRestocked)` — broadcast to owning actor on restock.

2. **`PAWanderingSmuggler.h` / `PAWanderingSmuggler.cpp`** (new):
   - `APAWanderingSmuggler : public AActor`
   - Subobjects: `UPAMerchantComponent* MerchantComp`, `UPARestockComponent* RestockComp`.
   - `int32 RotatingSlotCount = 4` — matches `rotating_slot_count_tier2 = 4`.
   - `TArray<FPAMerchantCatalogEntry> RotatingStockPool` — designer-configured pool of items.
   - `BuyItemWithKarmaCheck(UPAInventoryComponent*, UPACurrencyComponent*, int32 CatalogIndex, int32 Quantity, int32 PlayerKarma, EPATransactionError& OutError)` — wraps `MerchantComp->BuyItem`, first patching `PriceGold` in-flight when `PlayerKarma < -50`.
   - `MerchantTier` forced to `EPAMerchantTier::Tier2_Wilderness` in constructor.

3. **Rotating Stock Pool (initial seeding)**:
   - Populated via `BeginPlay()` calling `RestockComp->ExecuteRestock(MerchantComp)` on server.
   - Fixed catalog entries (indices 0–2) remain unchanged across restocks.
   - Rotating catalog entries (indices 3–6) are replaced each restock cycle.

4. **Server-wide restock synchronization**:
   - `NextRestockServerTime` calculated from `GetWorld()->GetTimeSeconds()` rounded to the nearest 3600s boundary — ensures all Tier 2 smugglers restock simultaneously regardless of individual spawn time.

---

## Out of Scope

- Tier 3 Black Market Broker (Karma Bailout service).
- Player-to-Player trading.
- Auction House.
- NPC patrol/movement AI (smuggler is stationary at campfire anchor).

---

## QA Test Cases

- **Test AC-1 — Fixed catalog unlimited purchase**: Wallet has 1,000 Gold. Buy 1× Blacksmith Ward (800 Gold). Verify Gold = 200, item in inventory, `AvailableStock` on fixed entry remains -1.
- **Test AC-2 — Limited stock exhaustion**: Rotating slot seeded with 2 units. Buy 2 units. Verify `AvailableStock = 0`. Attempt buy → `OutOfStock`. Trigger `ExecuteRestock` manually. Verify stock > 0 again.
- **Test AC-2 — Restock timer interval**: `RestockIntervalSeconds = 3600.0f`. Advance world timer. Verify `ExecuteRestock` is called and delegate fires.
- **Test AC-3 — Karma surcharge applied**: Player Karma = -60. Buy Blacksmith Ward (base 800 Gold). Verify price = `ceil(800 * 1.20)` = 960 Gold deducted. Player Karma = 0, same item → 800 Gold.
- **Test AC-4 — Tier 2 never rejects Wanted**: Player Karma = -80. `ValidateInteraction` returns `true` (not rejected). Surcharge is applied but shop opens.
- **Test AC-4 — Distance/combat guardrails**: In-combat → `InCombat` error. No rejection from Karma alone.
