// Copyright Project Ascendant. All Rights Reserved.

#include "Economy/PAMerchantComponent.h"
#include "Economy/PACurrencyComponent.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Network/PAServerRequestValidation.h"

UPAMerchantComponent::UPAMerchantComponent()
	: MerchantTier(EPAMerchantTier::Tier1_Outpost)
	, VendorSellPenalty(0.30f)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPAMerchantComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPAMerchantComponent::AddCatalogEntry(const FPAMerchantCatalogEntry& Entry)
{
	Catalog.Add(Entry);
}

const FPAMerchantCatalogEntry* UPAMerchantComponent::GetCatalogEntry(int32 Index) const
{
	if (Catalog.IsValidIndex(Index))
	{
		return &Catalog[Index];
	}
	return nullptr;
}

const FPABuybackItemEntry* UPAMerchantComponent::GetBuybackEntry(int32 Index) const
{
	if (BuybackList.IsValidIndex(Index))
	{
		return &BuybackList[Index];
	}
	return nullptr;
}

bool UPAMerchantComponent::IsEntryFromSeller(const FPABuybackItemEntry& Entry, const APlayerController* Seller)
{
	return Seller ? (Entry.Seller.Get() == Seller) : Entry.Seller.IsExplicitlyNull();
}

TArray<FPABuybackItemEntry> UPAMerchantComponent::GetBuybackEntriesForSeller(const APlayerController* Seller) const
{
	TArray<FPABuybackItemEntry> Result;
	for (const FPABuybackItemEntry& Entry : BuybackList)
	{
		if (IsEntryFromSeller(Entry, Seller))
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

void UPAMerchantComponent::PurgeLoggedOutBuybackEntries()
{
	// GDD merchant-economy.md §4: buyback lasts for the player's session. A set-but-invalid Seller means the
	// seller's PlayerController was destroyed (logout / disconnect) -> its entries are dropped.
	BuybackList.RemoveAll([](const FPABuybackItemEntry& Entry)
	{
		return !Entry.Seller.IsExplicitlyNull() && !Entry.Seller.IsValid();
	});
}

void UPAMerchantComponent::AddBuybackEntry(const FPABuybackItemEntry& Entry)
{
	PurgeLoggedOutBuybackEntries();

	const APlayerController* Seller = Entry.Seller.Get();
	int32 SellerCount = 0;
	int32 OldestIndex = INDEX_NONE;
	for (int32 Index = 0; Index < BuybackList.Num(); ++Index)
	{
		if (IsEntryFromSeller(BuybackList[Index], Seller))
		{
			if (OldestIndex == INDEX_NONE)
			{
				OldestIndex = Index;
			}
			++SellerCount;
		}
	}

	// FIFO per seller: other players' sales never evict this seller's entries.
	if (SellerCount >= kMaxBuybackSlots && OldestIndex != INDEX_NONE)
	{
		BuybackList.RemoveAt(OldestIndex);
	}
	BuybackList.Add(Entry);
}

bool UPAMerchantComponent::ValidateInteraction(const AActor* InteractingActor, bool bInCombat, EPATransactionError& OutError) const
{
	if (bInCombat)
	{
		OutError = EPATransactionError::InCombat;
		return false;
	}

	if (InteractingActor && GetOwner())
	{
		const float DistSq = FVector::DistSquared(InteractingActor->GetActorLocation(), GetOwner()->GetActorLocation());
		if (DistSq > FMath::Square(kMaxInteractionDistance))
		{
			OutError = EPATransactionError::DistanceExceeded;
			return false;
		}
	}

	OutError = EPATransactionError::None;
	return true;
}

bool UPAMerchantComponent::ValidateServerRequest(const APlayerController* Requester, const UActorComponent* Inventory, const UActorComponent* Wallet, EPATransactionError& OutError) const
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Requester)
	{
		OutError = EPATransactionError::ServerRejected;
		return false;
	}

	// Client-supplied components must belong to the requesting player (no trading with another player's inventory/wallet).
	if (Inventory && !PAServerRequestValidation::IsComponentOwnedBy(Inventory, Requester))
	{
		OutError = EPATransactionError::ServerRejected;
		return false;
	}

	if (Wallet && !PAServerRequestValidation::IsComponentOwnedBy(Wallet, Requester))
	{
		OutError = EPATransactionError::ServerRejected;
		return false;
	}

	const APawn* InteractingPawn = Requester->GetPawn();
	if (!InteractingPawn)
	{
		OutError = EPATransactionError::ServerRejected;
		return false;
	}

	return ValidateInteraction(InteractingPawn, PAServerRequestValidation::IsActorInCombat(InteractingPawn), OutError);
}

bool UPAMerchantComponent::BuyItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 CatalogIndex, int32 Quantity, EPATransactionError& OutError)
{
	if (!Inventory || !Wallet)
	{
		OutError = EPATransactionError::ServerRejected;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPATransactionError::ServerRejected;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	if (!Catalog.IsValidIndex(CatalogIndex) || Quantity <= 0)
	{
		OutError = EPATransactionError::ServerRejected;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	FPAMerchantCatalogEntry& Entry = Catalog[CatalogIndex];
	if (!Entry.ItemData)
	{
		OutError = EPATransactionError::ServerRejected;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	// Stock check
	if (Entry.AvailableStock != -1 && Entry.AvailableStock < Quantity)
	{
		OutError = EPATransactionError::OutOfStock;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	// Gold check
	const int64 TotalCost = static_cast<int64>(Entry.PriceGold) * Quantity;
	if (Wallet->GetGold() < TotalCost)
	{
		OutError = EPATransactionError::InsufficientGold;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	// Space check & inventory addition
	int32 OutRemaining = 0;
	const bool bAdded = Inventory->TryAddItem(Entry.ItemData, Quantity, OutRemaining);
	if (!bAdded || OutRemaining > 0)
	{
		if (OutRemaining < Quantity)
		{
			const int32 AddedQty = Quantity - OutRemaining;
			Inventory->ConsumeItemQuantity(Entry.ItemData->ItemId, AddedQty);
		}
		OutError = EPATransactionError::InventoryFull;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	// Deduct Gold
	EPACurrencyTransactionError CurrErr = EPACurrencyTransactionError::None;
	if (TotalCost > 0 && !Wallet->DeductCurrency(EPACurrencyType::Gold, TotalCost, CurrErr))
	{
		Inventory->ConsumeItemQuantity(Entry.ItemData->ItemId, Quantity);
		OutError = EPATransactionError::InsufficientGold;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	// Decrement stock if limited
	if (Entry.AvailableStock != -1)
	{
		Entry.AvailableStock -= Quantity;
	}

	OutError = EPATransactionError::None;
	OnItemPurchased.Broadcast(Entry.ItemData->ItemId, Quantity, TotalCost);
	return true;
}

bool UPAMerchantComponent::SellItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32 Quantity, EPATransactionError& OutError)
{
	return SellItemFor(nullptr, Inventory, Wallet, SlotIndex, Quantity, OutError);
}

bool UPAMerchantComponent::SellItemFor(const APlayerController* Seller, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32 Quantity, EPATransactionError& OutError)
{
	if (!Inventory || !Wallet)
	{
		OutError = EPATransactionError::ServerRejected;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPATransactionError::ServerRejected;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	const FPAInventoryItemEntry* ItemEntry = Inventory->GetItemAtSlot(SlotIndex);
	if (!ItemEntry || !ItemEntry->StaticData || Quantity <= 0 || Quantity > ItemEntry->StackCount)
	{
		OutError = EPATransactionError::ItemNotFound;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	// AC-2: Locked item protection
	if (ItemEntry->DynamicData.bIsLocked)
	{
		OutError = EPATransactionError::ItemLocked;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	const int32 UnitPrice = FPAMerchantFormulas::CalculateSellPrice(ItemEntry->StaticData->BaseSellPrice, VendorSellPenalty);
	const int32 TotalGold = UnitPrice * Quantity;

	// Add to Buyback list (FIFO eviction if > 10 slots)
	FPABuybackItemEntry BuybackEntry;
	BuybackEntry.ItemData = ItemEntry->StaticData;
	BuybackEntry.Quantity = Quantity;
	BuybackEntry.DynamicData = ItemEntry->DynamicData;
	BuybackEntry.BuybackPriceGold = TotalGold;
	BuybackEntry.ItemInstanceUID = ItemEntry->ItemInstanceUID;
	BuybackEntry.Seller = Seller;
	AddBuybackEntry(BuybackEntry); // X11b: per-seller FIFO (10)

	const FName ItemDefId = ItemEntry->ItemDefId;
	Inventory->RemoveItemFromSlot(SlotIndex, Quantity);

	if (TotalGold > 0)
	{
		EPACurrencyTransactionError CurrErr = EPACurrencyTransactionError::None;
		Wallet->AddCurrency(EPACurrencyType::Gold, TotalGold, CurrErr);
	}

	OutError = EPATransactionError::None;
	OnItemSold.Broadcast(ItemDefId, Quantity, TotalGold);
	return true;
}

bool UPAMerchantComponent::SellAllJunk(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32& OutTotalGoldReceived, int32& OutItemsSold, EPATransactionError& OutError)
{
	return SellAllJunkFor(nullptr, Inventory, Wallet, OutTotalGoldReceived, OutItemsSold, OutError);
}

bool UPAMerchantComponent::SellAllJunkFor(const APlayerController* Seller, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32& OutTotalGoldReceived, int32& OutItemsSold, EPATransactionError& OutError)
{
	OutTotalGoldReceived = 0;
	OutItemsSold = 0;

	if (!Inventory || !Wallet)
	{
		OutError = EPATransactionError::ServerRejected;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPATransactionError::ServerRejected;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	for (int32 SlotIdx = 0; SlotIdx < Inventory->GetCurrentMaxSlots(); ++SlotIdx)
	{
		const FPAInventoryItemEntry* Entry = Inventory->GetItemAtSlot(SlotIdx);
		if (Entry && Entry->StaticData && Entry->DynamicData.bIsJunk && !Entry->DynamicData.bIsLocked)
		{
			const int32 Qty = Entry->StackCount;
			const int32 UnitPrice = FPAMerchantFormulas::CalculateSellPrice(Entry->StaticData->BaseSellPrice, VendorSellPenalty);
			const int32 Gold = UnitPrice * Qty;

			FPABuybackItemEntry BuybackEntry;
			BuybackEntry.ItemData = Entry->StaticData;
			BuybackEntry.Quantity = Qty;
			BuybackEntry.DynamicData = Entry->DynamicData;
			BuybackEntry.BuybackPriceGold = Gold;
			BuybackEntry.ItemInstanceUID = Entry->ItemInstanceUID;
			BuybackEntry.Seller = Seller;
			AddBuybackEntry(BuybackEntry); // X11b: per-seller FIFO (10)

			Inventory->RemoveItemFromSlot(SlotIdx, Qty);
			OutTotalGoldReceived += Gold;
			OutItemsSold += Qty;
		}
	}

	if (OutTotalGoldReceived > 0)
	{
		EPACurrencyTransactionError CurrErr = EPACurrencyTransactionError::None;
		Wallet->AddCurrency(EPACurrencyType::Gold, OutTotalGoldReceived, CurrErr);
	}

	OutError = EPATransactionError::None;
	return true;
}

bool UPAMerchantComponent::BuybackItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 BuybackIndex, EPATransactionError& OutError)
{
	return BuybackAtIndex(Inventory, Wallet, BuybackIndex, OutError);
}

bool UPAMerchantComponent::BuybackAtIndex(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 BuybackIndex, EPATransactionError& OutError)
{
	if (!Inventory || !Wallet)
	{
		OutError = EPATransactionError::ServerRejected;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPATransactionError::ServerRejected;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	if (!BuybackList.IsValidIndex(BuybackIndex))
	{
		OutError = EPATransactionError::BuybackEmpty;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	const FPABuybackItemEntry BuybackEntry = BuybackList[BuybackIndex];

	// Gold check
	if (Wallet->GetGold() < BuybackEntry.BuybackPriceGold)
	{
		OutError = EPATransactionError::InsufficientGold;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	// Space check
	const int32 EmptySlot = Inventory->FindFirstEmptySlot();
	if (EmptySlot == INDEX_NONE)
	{
		OutError = EPATransactionError::InventoryFull;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	// Deduct Gold
	EPACurrencyTransactionError CurrErr = EPACurrencyTransactionError::None;
	if (BuybackEntry.BuybackPriceGold > 0 && !Wallet->DeductCurrency(EPACurrencyType::Gold, BuybackEntry.BuybackPriceGold, CurrErr))
	{
		OutError = EPATransactionError::InsufficientGold;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	// Restore item with dynamic data intact. X11b: a partial-stack sale shares its UID with the stack still in the
	// inventory; restore such an entry under a fresh UID so instance UIDs stay unique.
	const bool bUidInUse = BuybackEntry.ItemInstanceUID.IsValid() && Inventory->FindSlotByItemUID(BuybackEntry.ItemInstanceUID) != INDEX_NONE;
	Inventory->AddItemToSlot(EmptySlot, BuybackEntry.ItemData, BuybackEntry.Quantity, BuybackEntry.DynamicData, bUidInUse ? FGuid() : BuybackEntry.ItemInstanceUID);
	BuybackList.RemoveAt(BuybackIndex);

	OutError = EPATransactionError::None;
	OnItemBuybacked.Broadcast(BuybackEntry.ItemData->ItemId, BuybackEntry.Quantity, BuybackEntry.BuybackPriceGold);
	return true;
}

// -----------------------------------------------------------------------------
// X11b: Authority-only request handlers (reached via UPAServiceRequestComponent)
// -----------------------------------------------------------------------------

bool UPAMerchantComponent::ServerHandleBuyItem(const APlayerController* Requester, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 CatalogIndex, int32 Quantity, EPATransactionError& OutError)
{
	if (!ValidateServerRequest(Requester, Inventory, Wallet, OutError))
	{
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}
	return BuyItem(Inventory, Wallet, CatalogIndex, Quantity, OutError);
}

bool UPAMerchantComponent::ServerHandleSellItem(const APlayerController* Requester, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32 Quantity, EPATransactionError& OutError)
{
	if (!ValidateServerRequest(Requester, Inventory, Wallet, OutError))
	{
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}
	return SellItemFor(Requester, Inventory, Wallet, SlotIndex, Quantity, OutError);
}

bool UPAMerchantComponent::ServerHandleSellAllJunk(const APlayerController* Requester, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, EPATransactionError& OutError)
{
	if (!ValidateServerRequest(Requester, Inventory, Wallet, OutError))
	{
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}
	int32 TotalGold = 0;
	int32 ItemsSold = 0;
	return SellAllJunkFor(Requester, Inventory, Wallet, TotalGold, ItemsSold, OutError);
}

bool UPAMerchantComponent::ServerHandleBuybackItem(const APlayerController* Requester, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, const FGuid& ItemInstanceUID, EPATransactionError& OutError)
{
	if (!ValidateServerRequest(Requester, Inventory, Wallet, OutError))
	{
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	PurgeLoggedOutBuybackEntries();

	// X11b: only the requester's own entries are addressable (by UID; newest match wins for repeated partial sales).
	int32 FoundIndex = INDEX_NONE;
	for (int32 Index = BuybackList.Num() - 1; Index >= 0; --Index)
	{
		const FPABuybackItemEntry& Entry = BuybackList[Index];
		if (Requester && IsEntryFromSeller(Entry, Requester) && Entry.ItemInstanceUID == ItemInstanceUID)
		{
			FoundIndex = Index;
			break;
		}
	}

	if (FoundIndex == INDEX_NONE)
	{
		OutError = EPATransactionError::BuybackEmpty;
		OnTransactionFailed.Broadcast(OutError);
		return false;
	}

	return BuybackAtIndex(Inventory, Wallet, FoundIndex, OutError);
}
