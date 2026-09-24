// Copyright Project Ascendant. All Rights Reserved.

#include "UI/PABlacksmithForgeWidget.h"
#include "Crafting/PABlacksmithComponent.h"
#include "Economy/PACurrencyComponent.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"

// ===========================================================
// Lifecycle
// ===========================================================

void UPABlacksmithForgeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CachedDeltaTime = 0.0f;
}

void UPABlacksmithForgeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	CachedDeltaTime = InDeltaTime;
}

// ===========================================================
// Initialization
// ===========================================================

void UPABlacksmithForgeWidget::InitializeForge(
	UPABlacksmithComponent* ForgeComp,
	UPAInventoryComponent* PlayerInv,
	UPACurrencyComponent* PlayerWallet)
{
	ForgeRef = ForgeComp;
	InventoryRef = PlayerInv;
	CurrencyRef = PlayerWallet;

	Model.ResetHold();
	Model.TargetEquipmentId = NAME_None;
	Model.MaterialSlots.Reset();
	Model.StatPreviews.Reset();
}

// ===========================================================
// Equipment & Materials
// ===========================================================

void UPABlacksmithForgeWidget::SetTargetEquipmentSlot(int32 SlotIndex)
{
	if (!InventoryRef.IsValid() || !ForgeRef.IsValid())
	{
		return;
	}

	TargetSlotIndex = SlotIndex;
	const FPAInventoryItemEntry* Slot = InventoryRef->GetItemAtSlot(SlotIndex);
	if (!Slot)
	{
		return;
	}

	const FName ItemId = Slot->ItemDefId;
	const FString DisplayName = Slot->StaticData ? Slot->StaticData->ItemName.ToString() : ItemId.ToString();
	Model.SetTargetEquipment(ItemId, DisplayName);

	Model.MaterialSlots.Reset();
	Model.GoldCost = 100;

	if (CurrencyRef.IsValid())
	{
		Model.PlayerGold = CurrencyRef->GetGold();
	}
}

void UPABlacksmithForgeWidget::ToggleWardStone(bool bUseWard)
{
	Model.ToggleWard(bUseWard);
}

// ===========================================================
// Hold-to-Craft
// ===========================================================

void UPABlacksmithForgeWidget::UpdateHoldInput(bool bIsHolding)
{
	const bool bJustCompleted = Model.UpdateHoldProgress(CachedDeltaTime, bIsHolding);
	if (bJustCompleted)
	{
		OnForgeHoldCompleted.Broadcast();

		// Dispatch Server RPC
		if (ForgeRef.IsValid() && InventoryRef.IsValid() && CurrencyRef.IsValid())
		{
			if (Model.bUsingWard)
			{
				ForgeRef->Server_RequestEnhanceWithWard(InventoryRef.Get(), CurrencyRef.Get(), TargetSlotIndex, true);
			}
			else
			{
				ForgeRef->Server_RequestEnhance(InventoryRef.Get(), CurrencyRef.Get(), TargetSlotIndex);
			}
		}
	}
}

// ===========================================================
// Proximity & Auto-Close
// ===========================================================

void UPABlacksmithForgeWidget::UpdateProximityState(float Distance, bool bCombatActive)
{
	const bool bShouldClose = Model.UpdateProximity(Distance, bCombatActive);
	if (bShouldClose)
	{
		OnForgeAutoClose.Broadcast();
	}
}
