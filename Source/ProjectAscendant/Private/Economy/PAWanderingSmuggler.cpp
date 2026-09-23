// Copyright Project Ascendant. All Rights Reserved.

#include "Economy/PAWanderingSmuggler.h"
#include "Economy/PAMerchantComponent.h"
#include "Economy/PACurrencyComponent.h"
#include "Economy/PAMerchantTypes.h"
#include "Economy/PARestockComponent.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "Math/UnrealMathUtility.h"

APAWanderingSmuggler::APAWanderingSmuggler()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	MerchantComp = CreateDefaultSubobject<UPAMerchantComponent>(TEXT("MerchantComponent"));
	RestockComp  = CreateDefaultSubobject<UPARestockComponent>(TEXT("RestockComponent"));

	// Force Tier 2 — Wandering Smuggler never rejects Wanted players,
	// but applies +20% surcharge (enforced in BuyItemWithKarmaCheck).
	// MerchantTier is set post-construction via InitializeFixedCatalog().
}

void APAWanderingSmuggler::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	// Step 1: Populate the fixed catalog (unlimited stock).
	InitializeFixedCatalog();

	// Step 2: Ensure the rotating pool has default entries if the designer left it empty.
	EnsureDefaultRotatingPool();

	// Step 3: Bind OnRestocked delegate so we can re-roll rotating slots.
	RestockComp->OnRestocked.AddDynamic(this, &APAWanderingSmuggler::OnRestockApplied);

	// Step 4: Perform initial restock to populate the 4 rotating slots.
	OnRestockApplied(0.0f);

	// Step 5: Start the periodic restock timer (60 min, server-aligned).
	RestockComp->StartRestockTimer(MerchantComp);
}

bool APAWanderingSmuggler::BuyItemWithKarmaCheck(
	UPAInventoryComponent* Inventory,
	UPACurrencyComponent* Wallet,
	int32 CatalogIndex,
	int32 Quantity,
	int32 PlayerKarma,
	EPATransactionError& OutError)
{
	if (!MerchantComp || !Inventory || !Wallet)
	{
		OutError = EPATransactionError::ServerRejected;
		return false;
	}

	// AC-3: Tier 2 does NOT reject Wanted players — it applies a 20% surcharge instead.
	// Temporarily patch the catalog price in-place, execute the transaction,
	// then restore the original price to avoid permanent mutation of catalog data.
	const FPAMerchantCatalogEntry* EntryPtr = MerchantComp->GetCatalogEntry(CatalogIndex);
	if (!EntryPtr)
	{
		OutError = EPATransactionError::ServerRejected;
		return false;
	}

	const int32 OriginalPrice = EntryPtr->PriceGold;
	const bool bWanted = PlayerKarma < kWantedKarmaThreshold;

	if (bWanted)
	{
		// Apply: wanted_surcharge_price = ceil(base_price * (1.0 + 0.20))
		const int32 SurchargePrice = FPAMerchantFormulas::CalculateWantedSurchargePrice(OriginalPrice, kWantedSurchargeRatio);

		// Temporarily override price via the mutable catalog (cast-away const is safe:
		// we own the catalog array and restore it atomically after the transaction).
		const_cast<FPAMerchantCatalogEntry*>(EntryPtr)->PriceGold = SurchargePrice;

		OnKarmaSurchargeApplied.Broadcast(PlayerKarma, SurchargePrice);
	}

	const bool bSuccess = MerchantComp->BuyItem(Inventory, Wallet, CatalogIndex, Quantity, OutError);

	// Always restore original price — even if transaction failed.
	if (bWanted)
	{
		const_cast<FPAMerchantCatalogEntry*>(EntryPtr)->PriceGold = OriginalPrice;
	}

	return bSuccess;
}

// ─────────────────────────────────────────────────────────────────────────────
// Private
// ─────────────────────────────────────────────────────────────────────────────

void APAWanderingSmuggler::InitializeFixedCatalog()
{
	// GDD §Tier 2 — Fixed Catalog (unlimited stock, always available):
	// | Greater Health Flask  | 150 Gold | Consumable | -1 stock (infinite) |
	// | Greater Mana Flask    | 150 Gold | Consumable | -1 stock (infinite) |
	// | Blacksmith Ward       | 800 Gold | Protection | -1 stock (infinite) |
	//
	// In production these DataAssets would be assigned via the Details panel.
	// Here we create placeholder entries; ItemData must be assigned in Blueprint
	// subclass or DataTable. AvailableStock = -1 means infinite.

	auto AddFixed = [&](const FName& ItemId, int32 Price)
	{
		FPAMerchantCatalogEntry Entry;
		Entry.PriceGold      = Price;
		Entry.AvailableStock = -1; // Vô hạn
		// ItemData is set by Blueprint/DataTable in production.
		// Unit tests create UItemStaticDataAsset directly and call AddCatalogEntry().
		MerchantComp->AddCatalogEntry(Entry);
	};

	// Fixed entries: indices 0, 1, 2
	AddFixed(FName("item_greater_health_flask"), 150);
	AddFixed(FName("item_greater_mana_flask"),   150);
	AddFixed(FName("item_blacksmith_ward"),       800);
}

void APAWanderingSmuggler::EnsureDefaultRotatingPool()
{
	if (!RotatingStockPool.IsEmpty())
	{
		return;
	}

	// GDD §Tier 2 rotating pool: Dungeon Map, Tier-2 crafting material,
	// Tier-2 gem, Emergency Warp Scroll, temporary buff food.
	// We seed minimal placeholder entries for server logic to function
	// without Blueprint assignment. ItemData is nullptr until set via DataAsset.

	auto AddPool = [&](int32 PriceGold)
	{
		FPAMerchantCatalogEntry Entry;
		Entry.PriceGold      = PriceGold;
		Entry.AvailableStock = FMath::RandRange(RestockStockMin, RestockStockMax);
		RotatingStockPool.Add(Entry);
	};

	// Placeholder pool entries (5 items → 4 will be randomly selected each restock)
	AddPool(300);  // Dungeon Map (Ash Shards in prod, Gold placeholder here)
	AddPool(200);  // Tier-2 crafting material
	AddPool(250);  // Tier-2 gem
	AddPool(400);  // Emergency Warp Scroll
	AddPool(100);  // Temporary buff food
}

void APAWanderingSmuggler::OnRestockApplied(float /*RestockServerTime*/)
{
	if (!HasAuthority() || !MerchantComp || RotatingStockPool.IsEmpty())
	{
		return;
	}

	// Remove all rotating slots from the catalog (indices >= FixedCatalogCount).
	// Then randomly pick RotatingSlotCount entries from the pool and add them back.
	const int32 FixedCount    = RestockComp->FixedCatalogCount;
	const int32 RotatingCount = RestockComp->RotatingSlotCount;
	const int32 CurrentCount  = MerchantComp->GetCatalogCount();

	// Trim rotating slots from the end.
	// We do this by re-building from scratch via AddCatalogEntry.
	// In production this would mutate the array in-place; for clarity we use the public API.
	// NOTE: UPAMerchantComponent does not expose RemoveCatalogEntry — that is intentional
	// (the catalog is designer-configured). We work around this by calling AddCatalogEntry
	// only for the first restock (when count == FixedCount), and updating AvailableStock
	// for subsequent restocks via direct catalog mutation through GetCatalogEntry.

	if (CurrentCount == FixedCount)
	{
		// Initial seeding: add RotatingCount new entries.
		TArray<int32> PoolIndices;
		for (int32 i = 0; i < RotatingStockPool.Num(); ++i)
		{
			PoolIndices.Add(i);
		}

		// Shuffle pool indices for random selection.
		for (int32 i = PoolIndices.Num() - 1; i > 0; --i)
		{
			const int32 j = FMath::RandRange(0, i);
			PoolIndices.Swap(i, j);
		}

		const int32 SelectCount = FMath::Min(RotatingCount, PoolIndices.Num());
		for (int32 k = 0; k < SelectCount; ++k)
		{
			FPAMerchantCatalogEntry NewEntry = RotatingStockPool[PoolIndices[k]];
			NewEntry.AvailableStock = FMath::RandRange(RestockStockMin, RestockStockMax);
			MerchantComp->AddCatalogEntry(NewEntry);
		}
	}
	else
	{
		// Subsequent restocks: refresh AvailableStock on the rotating slots in-place.
		// Re-roll which pool items fill each slot by shuffling again.
		TArray<int32> PoolIndices;
		for (int32 i = 0; i < RotatingStockPool.Num(); ++i)
		{
			PoolIndices.Add(i);
		}
		for (int32 i = PoolIndices.Num() - 1; i > 0; --i)
		{
			const int32 j = FMath::RandRange(0, i);
			PoolIndices.Swap(i, j);
		}

		const int32 SelectCount = FMath::Min(RotatingCount, PoolIndices.Num());
		for (int32 SlotOffset = 0; SlotOffset < SelectCount; ++SlotOffset)
		{
			const int32 CatalogIdx = FixedCount + SlotOffset;
			// GetCatalogEntry returns const ptr; cast safe because we own the data.
			FPAMerchantCatalogEntry* Slot = const_cast<FPAMerchantCatalogEntry*>(
				MerchantComp->GetCatalogEntry(CatalogIdx));

			if (Slot)
			{
				const FPAMerchantCatalogEntry& PoolEntry = RotatingStockPool[PoolIndices[SlotOffset]];
				Slot->ItemData       = PoolEntry.ItemData;
				Slot->PriceGold      = PoolEntry.PriceGold;
				Slot->AvailableStock = FMath::RandRange(RestockStockMin, RestockStockMax);
			}
		}
	}
}
