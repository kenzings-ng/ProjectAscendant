// Copyright Project Ascendant. All Rights Reserved.

#include "Economy/PAMerchantComponent.h"
#include "Economy/PACurrencyComponent.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "GameFramework/Actor.h"

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

	if (BuybackList.Num() >= kMaxBuybackSlots)
	{
		BuybackList.RemoveAt(0);
	}
	BuybackList.Add(BuybackEntry);

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

			if (BuybackList.Num() >= kMaxBuybackSlots)
			{
				BuybackList.RemoveAt(0);
			}
			BuybackList.Add(BuybackEntry);

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

	// Restore item with dynamic data intact
	Inventory->AddItemToSlot(EmptySlot, BuybackEntry.ItemData, BuybackEntry.Quantity, BuybackEntry.DynamicData, BuybackEntry.ItemInstanceUID);
	BuybackList.RemoveAt(BuybackIndex);

	OutError = EPATransactionError::None;
	OnItemBuybacked.Broadcast(BuybackEntry.ItemData->ItemId, BuybackEntry.Quantity, BuybackEntry.BuybackPriceGold);
	return true;
}

// -----------------------------------------------------------------------------
// Server RPCs
// -----------------------------------------------------------------------------

bool UPAMerchantComponent::Server_RequestBuyItem_Validate(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 CatalogIndex, int32 Quantity)
{
	return Inventory != nullptr && Wallet != nullptr && CatalogIndex >= 0 && Quantity > 0;
}

void UPAMerchantComponent::Server_RequestBuyItem_Implementation(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 CatalogIndex, int32 Quantity)
{
	EPATransactionError Err = EPATransactionError::None;
	BuyItem(Inventory, Wallet, CatalogIndex, Quantity, Err);
}

bool UPAMerchantComponent::Server_RequestSellItem_Validate(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32 Quantity)
{
	return Inventory != nullptr && Wallet != nullptr && SlotIndex >= 0 && Quantity > 0;
}

void UPAMerchantComponent::Server_RequestSellItem_Implementation(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32 Quantity)
{
	EPATransactionError Err = EPATransactionError::None;
	SellItem(Inventory, Wallet, SlotIndex, Quantity, Err);
}

bool UPAMerchantComponent::Server_RequestSellAllJunk_Validate(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet)
{
	return Inventory != nullptr && Wallet != nullptr;
}

void UPAMerchantComponent::Server_RequestSellAllJunk_Implementation(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet)
{
	int32 TotalGold = 0;
	int32 ItemsSold = 0;
	EPATransactionError Err = EPATransactionError::None;
	SellAllJunk(Inventory, Wallet, TotalGold, ItemsSold, Err);
}

bool UPAMerchantComponent::Server_RequestBuybackItem_Validate(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 BuybackIndex)
{
	return Inventory != nullptr && Wallet != nullptr && BuybackIndex >= 0;
}

void UPAMerchantComponent::Server_RequestBuybackItem_Implementation(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 BuybackIndex)
{
	EPATransactionError Err = EPATransactionError::None;
	BuybackItem(Inventory, Wallet, BuybackIndex, Err);
}
