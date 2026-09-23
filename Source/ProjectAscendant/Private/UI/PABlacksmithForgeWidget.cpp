// Copyright Project Ascendant. All Rights Reserved.

#include "UI/PABlacksmithForgeWidget.h"
#include "Crafting/PABlacksmithComponent.h"
#include "Economy/PACurrencyComponent.h"
#include "Inventory/PAInventoryComponent.h"

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

	// Lấy item từ inventory slot
	const auto& Slots = InventoryRef->GetSlots();
	if (!Slots.IsValidIndex(SlotIndex))
	{
		return;
	}

	const auto& Slot = Slots[SlotIndex];
	Model.SetTargetEquipment(Slot.ItemId, Slot.DisplayName);

	// Cập nhật danh sách nguyên liệu từ BlacksmithComponent
	Model.MaterialSlots.Reset();
	if (ForgeRef.IsValid())
	{
		const auto& Requirements = ForgeRef->GetEnhanceRequirements(Slot.ItemId);
		for (const auto& Req : Requirements)
		{
			FPAForgeSlotEntry SlotEntry;
			SlotEntry.MaterialId = Req.MaterialId;
			SlotEntry.DisplayName = Req.DisplayName;
			SlotEntry.RequiredAmount = Req.RequiredAmount;
			SlotEntry.OwnedAmount = InventoryRef->GetItemCount(Req.MaterialId);
			SlotEntry.UpdateSufficiency();
			Model.MaterialSlots.Add(SlotEntry);
		}

		Model.GoldCost = ForgeRef->GetEnhanceCostGold(Slot.ItemId);
	}

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
		if (ForgeRef.IsValid())
		{
			if (Model.bUsingWard)
			{
				ForgeRef->Server_RequestEnhanceWithWard(Model.TargetEquipmentId);
			}
			else
			{
				ForgeRef->Server_RequestEnhance(Model.TargetEquipmentId);
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
