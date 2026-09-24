// Copyright Project Ascendant. All Rights Reserved.

#include "Crafting/PABlacksmithComponent.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "Economy/PACurrencyComponent.h"
#include "Crafting/PABlacksmithSubsystem.h"
#include "Itemization/PAServerItemGeneratorSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"

UPABlacksmithComponent::UPABlacksmithComponent()
	: ForgeTier(EPABlacksmithTier::Tier1_Outpost)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPABlacksmithComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UPABlacksmithComponent::ValidateInteraction(const AActor* InteractingActor, bool bInCombat, EPACraftingError& OutError) const
{
	if (bInCombat)
	{
		OutError = EPACraftingError::InCombat;
		return false;
	}

	if (InteractingActor && GetOwner())
	{
		const float DistSq = FVector::DistSquared(InteractingActor->GetActorLocation(), GetOwner()->GetActorLocation());
		if (DistSq > FMath::Square(kMaxInteractionDistance))
		{
			OutError = EPACraftingError::DistanceExceeded;
			return false;
		}
	}

	OutError = EPACraftingError::None;
	return true;
}

bool UPABlacksmithComponent::RepairItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, EPACraftingError& OutError)
{
	if (!Inventory || !Wallet)
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const FPAInventoryItemEntry* Entry = Inventory->GetItemAtSlot(SlotIndex);
	if (!Entry || !Entry->StaticData)
	{
		OutError = EPACraftingError::ItemNotFound;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (Entry->DynamicData.CurrentDurability >= 100.0f)
	{
		OutError = EPACraftingError::MaxDurabilityAlready;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const int32 BasePrice = Entry->StaticData->BaseSellPrice;
	const int32 RepairCost = FPABlacksmithFormulas::CalculateRepairCost(BasePrice, Entry->DynamicData.CurrentDurability, 100.0f);

	if (Wallet->GetGold() < RepairCost)
	{
		OutError = EPACraftingError::InsufficientGold;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Atomic gold deduction
	EPACurrencyTransactionError CurrErr = EPACurrencyTransactionError::None;
	if (RepairCost > 0 && !Wallet->DeductCurrency(EPACurrencyType::Gold, RepairCost, CurrErr))
	{
		OutError = EPACraftingError::InsufficientGold;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Restore durability to 100%
	FPAItemInstanceData RepairedData = Entry->DynamicData;
	RepairedData.CurrentDurability = 100.0f;
	Inventory->UpdateItemDynamicData(SlotIndex, RepairedData);

	OutError = EPACraftingError::None;
	OnItemRepaired.Broadcast(SlotIndex, RepairCost, 100.0f);
	return true;
}

bool UPABlacksmithComponent::SalvageItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32& OutShardsGained, EPACraftingError& OutError)
{
	OutShardsGained = 0;

	if (!Inventory || !Wallet)
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const FPAInventoryItemEntry* Entry = Inventory->GetItemAtSlot(SlotIndex);
	if (!Entry || !Entry->StaticData)
	{
		OutError = EPACraftingError::ItemNotFound;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// AC-2: Protected locked items
	if (Entry->DynamicData.bIsLocked)
	{
		OutError = EPACraftingError::ItemLocked;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const int32 Shards = FPABlacksmithFormulas::GetSalvageAshShards(Entry->StaticData->RarityTier, Entry->StaticData->Category);

	// Credit Ash Shards to wallet
	EPACurrencyTransactionError CurrErr = EPACurrencyTransactionError::None;
	if (Shards > 0 && !Wallet->AddCurrency(EPACurrencyType::AshShards, Shards, CurrErr))
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const FName ItemDefId = Entry->ItemDefId;
	Inventory->RemoveItemFromSlot(SlotIndex, Entry->StackCount);

	OutShardsGained = Shards;
	OutError = EPACraftingError::None;
	OnItemSalvaged.Broadcast(SlotIndex, ItemDefId, Shards);
	return true;
}

bool UPABlacksmithComponent::EnhanceItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, bool bUseWard, EPACraftingError& OutError)
{
	if (!Inventory || !Wallet)
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const FPAInventoryItemEntry* Entry = Inventory->GetItemAtSlot(SlotIndex);
	if (!Entry || !Entry->StaticData)
	{
		OutError = EPACraftingError::ItemNotFound;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const int32 CurrentLevel = Entry->DynamicData.EnhancementLevel;
	const int32 TargetLevel = CurrentLevel + 1;

	// AC-3 (crft-001) / AC-1 & AC-2 (crft-002): Check Tier enhancement ceiling
	const int32 MaxTierLevel = FPABlacksmithFormulas::GetMaxEnhancementForTier(ForgeTier);
	if (TargetLevel > MaxTierLevel)
	{
		OutError = EPACraftingError::MaxTierLevelReached;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// AC-2: If ward is requested for high-tier enhancement (+7 to +10), verify player has ward item
	if (TargetLevel >= 7 && bUseWard)
	{
		if (Inventory->GetItemCount(FName("item_blacksmith_ward")) < 1)
		{
			OutError = EPACraftingError::NoWardItem;
			OnCraftingFailed.Broadcast(OutError);
			return false;
		}
	}

	int32 GoldCost = 0;
	int32 IronOreCost = 0;
	if (!FPABlacksmithFormulas::GetEnhancementCost(CurrentLevel, GoldCost, IronOreCost))
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Check resources
	if (Wallet->GetGold() < GoldCost)
	{
		OutError = EPACraftingError::InsufficientGold;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (Inventory->GetItemCount(FName("iron_ore")) < IronOreCost)
	{
		OutError = EPACraftingError::InsufficientMaterials;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Atomic deduction: Gold & Iron Ore
	EPACurrencyTransactionError CurrErr = EPACurrencyTransactionError::None;
	if (GoldCost > 0 && !Wallet->DeductCurrency(EPACurrencyType::Gold, GoldCost, CurrErr))
	{
		OutError = EPACraftingError::InsufficientGold;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (IronOreCost > 0 && !Inventory->ConsumeItemQuantity(FName("iron_ore"), IronOreCost))
	{
		// Rollback gold if material consumption fails
		Wallet->AddCurrency(EPACurrencyType::Gold, GoldCost, CurrErr);
		OutError = EPACraftingError::InsufficientMaterials;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// If ward was used, consume 1 ward item from inventory
	if (TargetLevel >= 7 && bUseWard)
	{
		Inventory->ConsumeItemQuantity(FName("item_blacksmith_ward"), 1);
	}

	// AC-1 & AC-2: RNG Resolution
	const float SuccessRate = FPABlacksmithFormulas::GetEnhancementSuccessRate(TargetLevel);
	const float Roll = (TestRollOverride >= 0.0f) ? TestRollOverride : FMath::FRand();
	const bool bSuccess = (Roll < SuccessRate);

	FPAItemInstanceData NewDynamicData = Entry->DynamicData;

	if (bSuccess)
	{
		NewDynamicData.EnhancementLevel = TargetLevel;
		Inventory->UpdateItemDynamicData(SlotIndex, NewDynamicData);

		OutError = EPACraftingError::None;
		OnItemEnhanced.Broadcast(SlotIndex, TargetLevel, true);
		return true;
	}
	else
	{
		// AC-1: Failure at +4 to +6 keeps current level
		// AC-2: Failure at +7 to +10 drops 1 level unless protected by ward
		if (TargetLevel <= 6 || bUseWard)
		{
			// Preserve current level
			OutError = EPACraftingError::None;
			OnItemEnhanced.Broadcast(SlotIndex, CurrentLevel, false);
			return false;
		}
		else
		{
			// Drop 1 level (cannot drop below 0)
			const int32 NewDowngradedLevel = FMath::Max(0, CurrentLevel - 1);
			NewDynamicData.EnhancementLevel = NewDowngradedLevel;
			Inventory->UpdateItemDynamicData(SlotIndex, NewDynamicData);

			OutError = EPACraftingError::None;
			OnItemEnhanced.Broadcast(SlotIndex, NewDowngradedLevel, false);
			return false;
		}
	}
}

bool UPABlacksmithComponent::UnlockSocket(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, EPACraftingError& OutError)
{
	if (!Inventory)
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Socketing requires Tier 2 Forge or higher
	if (ForgeTier < EPABlacksmithTier::Tier2_Wilderness)
	{
		OutError = EPACraftingError::MaxTierLevelReached;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const FPAInventoryItemEntry* Entry = Inventory->GetItemAtSlot(SlotIndex);
	if (!Entry || !Entry->StaticData)
	{
		OutError = EPACraftingError::ItemNotFound;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Only Equipment of Rare rarity or higher can have sockets
	if (Entry->StaticData->Category != EPAItemCategory::Equipment || Entry->StaticData->RarityTier < EPAItemRarity::Rare)
	{
		OutError = EPACraftingError::ItemCannotBeSocketed;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const int32 MaxSockets = FPABlacksmithFormulas::GetMaxGemSockets(ForgeTier, Entry->StaticData->RarityTier);
	if (Entry->DynamicData.SocketedGemIds.Num() >= MaxSockets)
	{
		OutError = EPACraftingError::MaxSocketsReached;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Add empty socket
	FPAItemInstanceData NewDynamicData = Entry->DynamicData;
	NewDynamicData.SocketedGemIds.Add(NAME_None);
	Inventory->UpdateItemDynamicData(SlotIndex, NewDynamicData);

	OutError = EPACraftingError::None;
	OnSocketUnlocked.Broadcast(SlotIndex, NewDynamicData.SocketedGemIds.Num(), MaxSockets);
	return true;
}

bool UPABlacksmithComponent::SocketGem(UPAInventoryComponent* Inventory, int32 EquipmentSlotIndex, int32 SocketIndex, FName GemItemId, EPACraftingError& OutError)
{
	if (!Inventory)
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (ForgeTier < EPABlacksmithTier::Tier2_Wilderness)
	{
		OutError = EPACraftingError::MaxTierLevelReached;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (!FPABlacksmithFormulas::IsGemItem(GemItemId))
	{
		OutError = EPACraftingError::InvalidGemItem;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (Inventory->GetItemCount(GemItemId) < 1)
	{
		OutError = EPACraftingError::InsufficientMaterials;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const FPAInventoryItemEntry* Entry = Inventory->GetItemAtSlot(EquipmentSlotIndex);
	if (!Entry || !Entry->StaticData)
	{
		OutError = EPACraftingError::ItemNotFound;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (!Entry->DynamicData.SocketedGemIds.IsValidIndex(SocketIndex))
	{
		OutError = EPACraftingError::InvalidSocketIndex;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (Entry->DynamicData.SocketedGemIds[SocketIndex] != NAME_None)
	{
		OutError = EPACraftingError::SocketNotEmpty;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Consume gem from inventory
	if (!Inventory->ConsumeItemQuantity(GemItemId, 1))
	{
		OutError = EPACraftingError::InsufficientMaterials;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Socket gem
	FPAItemInstanceData NewDynamicData = Entry->DynamicData;
	NewDynamicData.SocketedGemIds[SocketIndex] = GemItemId;
	Inventory->UpdateItemDynamicData(EquipmentSlotIndex, NewDynamicData);

	OutError = EPACraftingError::None;
	OnGemSocketed.Broadcast(EquipmentSlotIndex, SocketIndex, GemItemId);
	return true;
}

bool UPABlacksmithComponent::UnsocketGem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 EquipmentSlotIndex, int32 SocketIndex, EPACraftingError& OutError)
{
	if (!Inventory || !Wallet)
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (ForgeTier < EPABlacksmithTier::Tier2_Wilderness)
	{
		OutError = EPACraftingError::MaxTierLevelReached;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const FPAInventoryItemEntry* Entry = Inventory->GetItemAtSlot(EquipmentSlotIndex);
	if (!Entry || !Entry->StaticData)
	{
		OutError = EPACraftingError::ItemNotFound;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (!Entry->DynamicData.SocketedGemIds.IsValidIndex(SocketIndex))
	{
		OutError = EPACraftingError::InvalidSocketIndex;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const FName GemId = Entry->DynamicData.SocketedGemIds[SocketIndex];
	if (GemId == NAME_None)
	{
		OutError = EPACraftingError::SocketEmpty;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const int32 UnsocketFee = FPABlacksmithFormulas::GetUnsocketGoldFee();
	if (Wallet->GetGold() < UnsocketFee)
	{
		OutError = EPACraftingError::InsufficientGold;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Find or create asset for returning gem
	UItemStaticDataAsset* GemAsset = nullptr;
	for (int32 i = 0; i < Inventory->GetCurrentMaxSlots(); ++i)
	{
		const FPAInventoryItemEntry* Existing = Inventory->GetItemAtSlot(i);
		if (Existing && Existing->ItemDefId == GemId && Existing->StaticData)
		{
			GemAsset = Existing->StaticData;
			break;
		}
	}

	if (!GemAsset)
	{
		GemAsset = NewObject<UItemStaticDataAsset>(Inventory);
		GemAsset->ItemId = GemId;
		GemAsset->Category = EPAItemCategory::Material;
		GemAsset->MaxStackSize = 999;
		GemAsset->ItemName = FText::FromName(GemId);
	}

	// Check if inventory has room
	int32 RemainingQuantity = 0;
	if (!Inventory->TryAddItem(GemAsset, 1, RemainingQuantity) || RemainingQuantity > 0)
	{
		OutError = EPACraftingError::InventoryFull;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Deduct fee
	EPACurrencyTransactionError CurrErr = EPACurrencyTransactionError::None;
	Wallet->DeductCurrency(EPACurrencyType::Gold, UnsocketFee, CurrErr);

	// Clear socket to NAME_None
	FPAItemInstanceData NewDynamicData = Entry->DynamicData;
	NewDynamicData.SocketedGemIds[SocketIndex] = NAME_None;
	Inventory->UpdateItemDynamicData(EquipmentSlotIndex, NewDynamicData);

	OutError = EPACraftingError::None;
	OnGemUnsocketed.Broadcast(EquipmentSlotIndex, SocketIndex, GemId);
	return true;
}

// -----------------------------------------------------------------------------
// Server RPCs
// -----------------------------------------------------------------------------

bool UPABlacksmithComponent::Server_RequestRepair_Validate(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	return Inventory != nullptr && Wallet != nullptr && SlotIndex >= 0;
}

void UPABlacksmithComponent::Server_RequestRepair_Implementation(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	EPACraftingError Err = EPACraftingError::None;
	RepairItem(Inventory, Wallet, SlotIndex, Err);
}

bool UPABlacksmithComponent::Server_RequestSalvage_Validate(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	return Inventory != nullptr && Wallet != nullptr && SlotIndex >= 0;
}

void UPABlacksmithComponent::Server_RequestSalvage_Implementation(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	int32 ShardsGained = 0;
	EPACraftingError Err = EPACraftingError::None;
	SalvageItem(Inventory, Wallet, SlotIndex, ShardsGained, Err);
}

bool UPABlacksmithComponent::Server_RequestEnhance_Validate(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	return Inventory != nullptr && Wallet != nullptr && SlotIndex >= 0;
}

void UPABlacksmithComponent::Server_RequestEnhance_Implementation(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	EPACraftingError Err = EPACraftingError::None;
	EnhanceItem(Inventory, Wallet, SlotIndex, false, Err);
}

bool UPABlacksmithComponent::Server_RequestEnhanceWithWard_Validate(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, bool bUseWard)
{
	return Inventory != nullptr && Wallet != nullptr && SlotIndex >= 0;
}

void UPABlacksmithComponent::Server_RequestEnhanceWithWard_Implementation(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, bool bUseWard)
{
	EPACraftingError Err = EPACraftingError::None;
	EnhanceItem(Inventory, Wallet, SlotIndex, bUseWard, Err);
}

bool UPABlacksmithComponent::Server_RequestUnlockSocket_Validate(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	return Inventory != nullptr && Wallet != nullptr && SlotIndex >= 0;
}

void UPABlacksmithComponent::Server_RequestUnlockSocket_Implementation(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	EPACraftingError Err = EPACraftingError::None;
	UnlockSocket(Inventory, Wallet, SlotIndex, Err);
}

bool UPABlacksmithComponent::Server_RequestSocketGem_Validate(UPAInventoryComponent* Inventory, int32 EquipmentSlotIndex, int32 SocketIndex, FName GemItemId)
{
	return Inventory != nullptr && EquipmentSlotIndex >= 0 && SocketIndex >= 0;
}

void UPABlacksmithComponent::Server_RequestSocketGem_Implementation(UPAInventoryComponent* Inventory, int32 EquipmentSlotIndex, int32 SocketIndex, FName GemItemId)
{
	EPACraftingError Err = EPACraftingError::None;
	SocketGem(Inventory, EquipmentSlotIndex, SocketIndex, GemItemId, Err);
}

bool UPABlacksmithComponent::Server_RequestUnsocketGem_Validate(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 EquipmentSlotIndex, int32 SocketIndex)
{
	return Inventory != nullptr && Wallet != nullptr && EquipmentSlotIndex >= 0 && SocketIndex >= 0;
}

void UPABlacksmithComponent::Server_RequestUnsocketGem_Implementation(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 EquipmentSlotIndex, int32 SocketIndex)
{
	EPACraftingError Err = EPACraftingError::None;
	UnsocketGem(Inventory, Wallet, EquipmentSlotIndex, SocketIndex, Err);
}

bool UPABlacksmithComponent::ForgeBossSoulEquipment(
	UPAInventoryComponent* Inventory,
	UPACurrencyComponent* Wallet,
	UItemStaticDataAsset* DivineItemData,
	FName BossSoulItemId,
	FName BossPartItemId,
	FName VoidOreItemId,
	EPACraftingError& OutError)
{
	if (!Inventory || !Wallet || !DivineItemData)
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// AC-1: Độc quyền tại Tier 3 Ancient Sanctuary Forge
	if (ForgeTier < EPABlacksmithTier::Tier3_Sanctuary)
	{
		OutError = EPACraftingError::MaxTierLevelReached;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Empty slot check
	const int32 EmptySlot = Inventory->FindFirstEmptySlot();
	if (EmptySlot == INDEX_NONE)
	{
		OutError = EPACraftingError::InventoryFull;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	int32 GoldCost = 0;
	int32 VoidOreCount = 0;
	int32 BossPartCount = 0;
	int32 BossSoulCount = 0;
	FPABlacksmithFormulas::GetBossSoulForgingCost(GoldCost, VoidOreCount, BossPartCount, BossSoulCount);

	// Gold check
	if (Wallet->GetGold() < GoldCost)
	{
		OutError = EPACraftingError::InsufficientGold;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Material checks
	if (Inventory->GetItemCount(BossSoulItemId) < BossSoulCount ||
		Inventory->GetItemCount(BossPartItemId) < BossPartCount ||
		Inventory->GetItemCount(VoidOreItemId) < VoidOreCount)
	{
		OutError = EPACraftingError::InsufficientMaterials;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Deduct Gold
	EPACurrencyTransactionError CurrErr = EPACurrencyTransactionError::None;
	if (GoldCost > 0 && !Wallet->DeductCurrency(EPACurrencyType::Gold, GoldCost, CurrErr))
	{
		OutError = EPACraftingError::InsufficientGold;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Consume materials
	Inventory->ConsumeItemQuantity(BossSoulItemId, BossSoulCount);
	Inventory->ConsumeItemQuantity(BossPartItemId, BossPartCount);
	Inventory->ConsumeItemQuantity(VoidOreItemId, VoidOreCount);

	// Create and add Divine equipment
	FPAItemInstanceData DynamicData;
	DynamicData.CurrentDurability = 100.0f;
	DynamicData.EnhancementLevel = 0;
	Inventory->AddItemToSlot(EmptySlot, DivineItemData, 1, DynamicData);

	OutError = EPACraftingError::None;
	OnBossSoulForged.Broadcast(DivineItemData->ItemId, DivineItemData->RarityTier);
	return true;
}

bool UPABlacksmithComponent::ExpandBackpackCapacity(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, EPACraftingError& OutError)
{
	if (!Inventory || !Wallet)
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		OutError = EPACraftingError::ServerRejected;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	const int32 CurrentSlots = Inventory->GetCurrentMaxSlots();
	if (CurrentSlots >= 60)
	{
		OutError = EPACraftingError::MaxBackpackCapacity;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	FPABackpackExpansionRequirements Req;
	if (!FPABlacksmithFormulas::GetBackpackExpansionRequirement(CurrentSlots, Req))
	{
		OutError = EPACraftingError::MaxBackpackCapacity;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (ForgeTier < Req.RequiredForgeTier)
	{
		OutError = EPACraftingError::MaxTierLevelReached;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	if (Wallet->GetGold() < Req.GoldCost)
	{
		OutError = EPACraftingError::InsufficientGold;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	auto ResolveItemWithCount = [&](FName PrimaryId, const TArray<FName>& Fallbacks, int32 Needed) -> FName
	{
		if (Inventory->GetItemCount(PrimaryId) >= Needed) return PrimaryId;
		for (const FName& Alt : Fallbacks)
		{
			if (Inventory->GetItemCount(Alt) >= Needed) return Alt;
		}
		return PrimaryId;
	};

	TArray<FName> FallbacksA;
	TArray<FName> FallbacksB;

	if (Req.TargetCapacity == 40)
	{
		FallbacksA = { FName("item_leather"), FName("item_animal_leather") };
		FallbacksB = { FName("item_copper_ore"), FName("item_ore_copper") };
	}
	else if (Req.TargetCapacity == 50)
	{
		FallbacksA = { FName("item_hardened_leather") };
		FallbacksB = { FName("item_black_iron_ore"), FName("item_ore_black_iron") };
	}
	else if (Req.TargetCapacity == 60)
	{
		FallbacksA = { FName("item_boss_scale"), FName("item_boss_part_tail") };
		FallbacksB = { FName("item_void_ore"), FName("item_ore_void") };
	}

	const FName ItemA = ResolveItemWithCount(Req.MaterialItemA, FallbacksA, Req.MaterialCountA);
	const FName ItemB = ResolveItemWithCount(Req.MaterialItemB, FallbacksB, Req.MaterialCountB);

	if (Inventory->GetItemCount(ItemA) < Req.MaterialCountA || Inventory->GetItemCount(ItemB) < Req.MaterialCountB)
	{
		OutError = EPACraftingError::InsufficientMaterials;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Deduct Gold
	EPACurrencyTransactionError CurrErr = EPACurrencyTransactionError::None;
	if (!Wallet->DeductCurrency(EPACurrencyType::Gold, Req.GoldCost, CurrErr))
	{
		OutError = EPACraftingError::InsufficientGold;
		OnCraftingFailed.Broadcast(OutError);
		return false;
	}

	// Consume Materials
	Inventory->ConsumeItemQuantity(ItemA, Req.MaterialCountA);
	Inventory->ConsumeItemQuantity(ItemB, Req.MaterialCountB);

	// Unlock 10 slots
	Inventory->UnlockMoreSlots(10);

	OutError = EPACraftingError::None;
	OnBackpackExpanded.Broadcast(CurrentSlots, Req.TargetCapacity, Req.GoldCost);
	return true;
}

bool UPABlacksmithComponent::Server_RequestForgeBossSoul_Validate(
	UPAInventoryComponent* Inventory,
	UPACurrencyComponent* Wallet,
	UItemStaticDataAsset* DivineItemData,
	FName BossSoulItemId,
	FName BossPartItemId,
	FName VoidOreItemId)
{
	return Inventory != nullptr && Wallet != nullptr && DivineItemData != nullptr;
}

void UPABlacksmithComponent::Server_RequestForgeBossSoul_Implementation(
	UPAInventoryComponent* Inventory,
	UPACurrencyComponent* Wallet,
	UItemStaticDataAsset* DivineItemData,
	FName BossSoulItemId,
	FName BossPartItemId,
	FName VoidOreItemId)
{
	EPACraftingError Err = EPACraftingError::None;
	ForgeBossSoulEquipment(Inventory, Wallet, DivineItemData, BossSoulItemId, BossPartItemId, VoidOreItemId, Err);
}

bool UPABlacksmithComponent::Server_RequestExpandBackpack_Validate(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet)
{
	return Inventory != nullptr && Wallet != nullptr;
}

void UPABlacksmithComponent::Server_RequestExpandBackpack_Implementation(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet)
{
	EPACraftingError Err = EPACraftingError::None;
	ExpandBackpackCapacity(Inventory, Wallet, Err);
}

void UPABlacksmithComponent::BindSavedItemInventory(TArray<FPASavedItemInstance>* InItems, UPACurrencyComponent* InWallet)
{
	BoundSavedItems = InItems;
	BoundWallet = InWallet;
}

bool UPABlacksmithComponent::Server_RepairItem_Validate(const FGuid& ItemInstanceUID, int32 CostGold)
{
	return ItemInstanceUID.IsValid() && CostGold >= 0;
}

void UPABlacksmithComponent::Server_RepairItem_Implementation(const FGuid& ItemInstanceUID, int32 CostGold)
{
	if (!BoundSavedItems || !BoundWallet.IsValid())
	{
		OnCraftingFailed.Broadcast(EPACraftingError::ServerRejected);
		return;
	}

	UPABlacksmithSubsystem* Subsystem = nullptr;
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			Subsystem = GI->GetSubsystem<UPABlacksmithSubsystem>();
		}
	}

	EPACraftingError Err = EPACraftingError::None;
	if (Subsystem)
	{
		Subsystem->ServerRepairItemByUID(*BoundSavedItems, ItemInstanceUID, BoundWallet.Get(), CostGold, Err);
	}
	else
	{
		for (FPASavedItemInstance& Item : *BoundSavedItems)
		{
			if (Item.ItemInstanceUID == ItemInstanceUID)
			{
				if (Item.CurrentDurability >= Item.MaxDurability)
				{
					Err = EPACraftingError::MaxDurabilityAlready;
					break;
				}
				if (BoundWallet->GetGold() < CostGold)
				{
					Err = EPACraftingError::InsufficientGold;
					break;
				}
				EPACurrencyTransactionError CurrErr;
				if (BoundWallet->DeductCurrency(EPACurrencyType::Gold, CostGold, CurrErr))
				{
					Item.CurrentDurability = Item.MaxDurability;
				}
				else
				{
					Err = EPACraftingError::InsufficientGold;
				}
				break;
			}
		}
	}

	if (Err != EPACraftingError::None)
	{
		OnCraftingFailed.Broadcast(Err);
	}
}

bool UPABlacksmithComponent::Server_ReforgeAffix_Validate(const FGuid& ItemInstanceUID, int32 AffixIndex, int32 CostGold, int32 CostShards)
{
	return ItemInstanceUID.IsValid() && AffixIndex >= 0 && CostGold >= 0 && CostShards >= 0;
}

void UPABlacksmithComponent::Server_ReforgeAffix_Implementation(const FGuid& ItemInstanceUID, int32 AffixIndex, int32 CostGold, int32 CostShards)
{
	if (!BoundSavedItems || !BoundWallet.IsValid())
	{
		OnCraftingFailed.Broadcast(EPACraftingError::ServerRejected);
		return;
	}

	UPABlacksmithSubsystem* BlacksmithSubsystem = nullptr;
	UPAServerItemGeneratorSubsystem* GeneratorSubsystem = nullptr;
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			BlacksmithSubsystem = GI->GetSubsystem<UPABlacksmithSubsystem>();
			GeneratorSubsystem = GI->GetSubsystem<UPAServerItemGeneratorSubsystem>();
		}
	}

	EPACraftingError Err = EPACraftingError::None;
	if (BlacksmithSubsystem && GeneratorSubsystem)
	{
		EPAForgeTier ForgeTierConverted = EPAForgeTier::Tier1_Outpost;
		switch (ForgeTier)
		{
		case EPABlacksmithTier::Tier2_Wilderness:
			ForgeTierConverted = EPAForgeTier::Tier2_Field;
			break;
		case EPABlacksmithTier::Tier3_Sanctuary:
			ForgeTierConverted = EPAForgeTier::Tier3_Forbidden;
			break;
		default:
			ForgeTierConverted = EPAForgeTier::Tier1_Outpost;
			break;
		}

		BlacksmithSubsystem->ServerReforgeAffixByUID(*BoundSavedItems, ItemInstanceUID, AffixIndex, CostGold, CostShards, ForgeTierConverted, BoundWallet.Get(), GeneratorSubsystem, Err);
	}
	else
	{
		Err = EPACraftingError::ServerRejected;
	}

	if (Err != EPACraftingError::None)
	{
		OnCraftingFailed.Broadcast(Err);
	}
}

bool UPABlacksmithComponent::Server_AddSocket_Validate(const FGuid& ItemInstanceUID, int32 CostGold, int32 CostShards, EPAForgeTier InForgeTier)
{
	return ItemInstanceUID.IsValid() && CostGold >= 0 && CostShards >= 0;
}

void UPABlacksmithComponent::Server_AddSocket_Implementation(const FGuid& ItemInstanceUID, int32 CostGold, int32 CostShards, EPAForgeTier InForgeTier)
{
	if (!BoundSavedItems || !BoundWallet.IsValid())
	{
		OnCraftingFailed.Broadcast(EPACraftingError::ServerRejected);
		return;
	}

	UPABlacksmithSubsystem* BlacksmithSubsystem = nullptr;
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			BlacksmithSubsystem = GI->GetSubsystem<UPABlacksmithSubsystem>();
		}
	}

	EPACraftingError Err = EPACraftingError::None;
	if (BlacksmithSubsystem)
	{
		BlacksmithSubsystem->ServerAddSocketByUID(*BoundSavedItems, ItemInstanceUID, CostGold, CostShards, InForgeTier, BoundWallet.Get(), Err);
	}
	else
	{
		Err = EPACraftingError::ServerRejected;
	}

	if (Err != EPACraftingError::None)
	{
		OnCraftingFailed.Broadcast(Err);
	}
}

