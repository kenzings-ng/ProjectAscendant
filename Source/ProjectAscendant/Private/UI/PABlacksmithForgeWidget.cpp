// Copyright Project Ascendant. All Rights Reserved.

#include "UI/PABlacksmithForgeWidget.h"
#include "Crafting/PABlacksmithComponent.h"
#include "Economy/PACurrencyComponent.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "Network/PAServiceRequestComponent.h"
#include "GameFramework/PlayerController.h"

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

void UPABlacksmithForgeWidget::NativeDestruct()
{
	UnbindRouter();
	UnbindWallet();
	Super::NativeDestruct();
}

// ===========================================================
// X11b: server confirmation routing
// ===========================================================

void UPABlacksmithForgeWidget::BindRouter(UPAServiceRequestComponent* Router)
{
	UnbindRouter();
	RouterRef = Router;
	if (Router)
	{
		RouterConfirmHandle = Router->OnForgeRequestConfirmedNative.AddUObject(this, &UPABlacksmithForgeWidget::HandleForgeRequestConfirmed);
	}
}

void UPABlacksmithForgeWidget::UnbindRouter()
{
	if (UPAServiceRequestComponent* Router = RouterRef.Get())
	{
		Router->OnForgeRequestConfirmedNative.Remove(RouterConfirmHandle);
	}
	RouterConfirmHandle.Reset();
	RouterRef.Reset();
	PendingRequestId = INDEX_NONE;
}


void UPABlacksmithForgeWidget::BindWallet(UPACurrencyComponent* Wallet)
{
	UnbindWallet();
	if (Wallet)
	{
		Wallet->OnCurrencyBalanceChanged.AddUniqueDynamic(this, &UPABlacksmithForgeWidget::HandleCurrencyBalanceChanged);
	}
}

void UPABlacksmithForgeWidget::UnbindWallet()
{
	if (UPACurrencyComponent* Wallet = CurrencyRef.Get())
	{
		Wallet->OnCurrencyBalanceChanged.RemoveDynamic(this, &UPABlacksmithForgeWidget::HandleCurrencyBalanceChanged);
	}
}

void UPABlacksmithForgeWidget::HandleCurrencyBalanceChanged(EPACurrencyType Type, int64 NewBalance, int64 Delta)
{
	if (Type == EPACurrencyType::Gold)
	{
		Model.PlayerGold = static_cast<int32>(NewBalance);
	}
}

void UPABlacksmithForgeWidget::HandleForgeRequestConfirmed(int32 RequestId, bool bSuccess, EPACraftingError ErrorCode)
{
	if (RequestId != PendingRequestId || PendingRequestId == INDEX_NONE)
	{
		return; // not ours (another widget / stale request)
	}
	PendingRequestId = INDEX_NONE;

	if (CurrencyRef.IsValid())
	{
		Model.PlayerGold = static_cast<int32>(CurrencyRef->GetGold());
	}

	OnTransactionCompleted.Broadcast(bSuccess, ErrorCode);
}

// ===========================================================
// Initialization
// ===========================================================

void UPABlacksmithForgeWidget::InitializeForge(
	UPABlacksmithComponent* ForgeComp,
	UPAInventoryComponent* PlayerInv,
	UPACurrencyComponent* PlayerWallet,
	UPAServiceRequestComponent* RequestRouter)
{
	UnbindWallet();
	ForgeRef = ForgeComp;
	InventoryRef = PlayerInv;
	CurrencyRef = PlayerWallet;
	BindWallet(PlayerWallet);

	if (!RequestRouter)
	{
		if (const APlayerController* OwningPC = GetOwningPlayer())
		{
			RequestRouter = OwningPC->FindComponentByClass<UPAServiceRequestComponent>();
		}
	}
	BindRouter(RequestRouter);

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
	UpdateHoldInputWithDelta(CachedDeltaTime, bIsHolding);
}

void UPABlacksmithForgeWidget::UpdateHoldInputWithDelta(float DeltaTime, bool bIsHolding)
{
	const bool bJustCompleted = Model.UpdateHoldProgress(DeltaTime, bIsHolding);
	if (bJustCompleted)
	{
		OnForgeHoldCompleted.Broadcast();

		// X11b: request via the player's router; OnTransactionCompleted fires on server confirmation.
		if (!IsRequestPending() && RouterRef.IsValid() && ForgeRef.IsValid() && InventoryRef.IsValid() && CurrencyRef.IsValid())
		{
			PendingRequestId = RouterRef->AllocateRequestId();
			RouterRef->Server_RequestForgeEnhance(PendingRequestId, ForgeRef->GetOwner(), InventoryRef.Get(), CurrencyRef.Get(), TargetSlotIndex, Model.bUsingWard);
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
