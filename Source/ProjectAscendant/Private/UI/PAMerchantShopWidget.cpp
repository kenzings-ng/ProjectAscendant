// Copyright Project Ascendant. All Rights Reserved.

#include "UI/PAMerchantShopWidget.h"
#include "Economy/PAMerchantComponent.h"
#include "Economy/PAMerchantTypes.h"
#include "Economy/PACurrencyComponent.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "Network/PAServiceRequestComponent.h"
#include "GameFramework/PlayerController.h"

// ===========================================================
// Lifecycle
// ===========================================================

void UPAMerchantShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Model.SwitchTab(EPAShopTab::Buy);
}

void UPAMerchantShopWidget::NativeDestruct()
{
	UnbindRouter();
	Super::NativeDestruct();
}

// ===========================================================
// X11b: server confirmation routing
// ===========================================================

void UPAMerchantShopWidget::BindRouter(UPAServiceRequestComponent* Router)
{
	UnbindRouter();
	RouterRef = Router;
	if (Router)
	{
		RouterConfirmHandle = Router->OnMerchantRequestConfirmedNative.AddUObject(this, &UPAMerchantShopWidget::HandleMerchantRequestConfirmed);
	}
}

void UPAMerchantShopWidget::UnbindRouter()
{
	if (UPAServiceRequestComponent* Router = RouterRef.Get())
	{
		Router->OnMerchantRequestConfirmedNative.Remove(RouterConfirmHandle);
	}
	RouterConfirmHandle.Reset();
	RouterRef.Reset();
	PendingRequestId = INDEX_NONE;
	PendingAction = EPendingShopAction::None;
}

void UPAMerchantShopWidget::HandleMerchantRequestConfirmed(int32 RequestId, bool bSuccess, EPATransactionError ErrorCode)
{
	if (RequestId != PendingRequestId || PendingRequestId == INDEX_NONE)
	{
		return; // not ours (another widget / stale request)
	}

	const EPendingShopAction Action = PendingAction;
	PendingRequestId = INDEX_NONE;
	PendingAction = EPendingShopAction::None;

	if (!bSuccess)
	{
		OnTransactionRejected.Broadcast(ErrorCode);
		return;
	}

	if (Action == EPendingShopAction::Sell)
	{
		Model.AddBuybackEntry(PendingSoldItem);
	}
	else if (Action == EPendingShopAction::Buyback)
	{
		Model.RemoveBuybackEntry(PendingBuybackIndex);
	}

	if (CurrencyRef.IsValid())
	{
		Model.UpdateAffordability(static_cast<int32>(CurrencyRef->GetGold()));
	}

	OnTransactionCompleted.Broadcast();
}

// ===========================================================
// Initialization
// ===========================================================

void UPAMerchantShopWidget::InitializeShop(
	UPAMerchantComponent* MerchantComp,
	UPAInventoryComponent* PlayerInv,
	UPACurrencyComponent* PlayerWallet,
	int32 PlayerKarma,
	UPAServiceRequestComponent* RequestRouter)
{
	MerchantRef = MerchantComp;
	InventoryRef = PlayerInv;
	CurrencyRef = PlayerWallet;

	if (!RequestRouter)
	{
		if (const APlayerController* OwningPC = GetOwningPlayer())
		{
			RequestRouter = OwningPC->FindComponentByClass<UPAServiceRequestComponent>();
		}
	}
	BindRouter(RequestRouter);

	if (!MerchantComp || !PlayerWallet)
	{
		return;
	}

	// Populate catalog từ MerchantComponent
	Model.CatalogItems.Reset();
	const TArray<FPAMerchantCatalogEntry>& Catalog = MerchantComp->GetCatalog();
	for (const FPAMerchantCatalogEntry& Entry : Catalog)
	{
		FPAShopItemEntry ShopEntry;
		if (Entry.ItemData)
		{
			ShopEntry.ItemId = Entry.ItemData->ItemId;
			ShopEntry.DisplayName = Entry.ItemData->ItemName.ToString();
		}
		ShopEntry.Quantity = Entry.AvailableStock;
		ShopEntry.PriceGold = Entry.PriceGold;
		ShopEntry.FinalPrice = Entry.PriceGold;
		Model.CatalogItems.Add(ShopEntry);
	}

	// Áp dụng Karma surcharge nếu Karma < -50
	const bool bSurcharge = PlayerKarma < -50;
	Model.ApplyKarmaSurcharge(bSurcharge);

	// Cập nhật affordability
	const int32 Gold = PlayerWallet->GetGold();
	Model.UpdateAffordability(Gold);

	Model.SwitchTab(EPAShopTab::Buy);
}

// ===========================================================
// Tab Navigation
// ===========================================================

void UPAMerchantShopWidget::SwitchTab(EPAShopTab NewTab)
{
	Model.SwitchTab(NewTab);
	OnTabChanged.Broadcast(NewTab);
}

// ===========================================================
// Selection
// ===========================================================

void UPAMerchantShopWidget::SelectCatalogItem(int32 Index)
{
	Model.SelectCatalogItem(Index);
}

void UPAMerchantShopWidget::SelectInventoryItem(int32 SlotIndex)
{
	Model.SelectInventoryItem(SlotIndex);
}

// ===========================================================
// Transactions
// ===========================================================

void UPAMerchantShopWidget::ExecuteBuy()
{
	if (IsRequestPending() || !RouterRef.IsValid() || !MerchantRef.IsValid() || !CurrencyRef.IsValid() || !InventoryRef.IsValid())
	{
		return;
	}

	if (Model.SelectedCatalogIndex < 0 || Model.SelectedCatalogIndex >= Model.CatalogItems.Num())
	{
		return;
	}

	const FPAShopItemEntry& Item = Model.CatalogItems[Model.SelectedCatalogIndex];
	if (!Item.bCanAfford)
	{
		return;
	}

	// X11b: request via the player's router; OnTransactionCompleted fires on server confirmation.
	PendingAction = EPendingShopAction::Buy;
	PendingRequestId = RouterRef->AllocateRequestId();
	RouterRef->Server_MerchantBuyItem(PendingRequestId, MerchantRef->GetOwner(), InventoryRef.Get(), CurrencyRef.Get(), Model.SelectedCatalogIndex, 1);
}

void UPAMerchantShopWidget::ExecuteSell()
{
	if (IsRequestPending() || !RouterRef.IsValid() || !MerchantRef.IsValid() || !InventoryRef.IsValid() || !CurrencyRef.IsValid())
	{
		return;
	}

	if (Model.SelectedInventoryIndex < 0 || Model.SelectedInventoryIndex >= Model.InventoryItems.Num())
	{
		return;
	}

	// Buyback entry is added only after the server confirms the sale.
	PendingSoldItem = Model.InventoryItems[Model.SelectedInventoryIndex];
	PendingAction = EPendingShopAction::Sell;
	PendingRequestId = RouterRef->AllocateRequestId();
	RouterRef->Server_MerchantSellItem(PendingRequestId, MerchantRef->GetOwner(), InventoryRef.Get(), CurrencyRef.Get(), Model.SelectedInventoryIndex, 1);
}

void UPAMerchantShopWidget::ExecuteBuyback()
{
	if (IsRequestPending() || !RouterRef.IsValid() || !MerchantRef.IsValid() || !InventoryRef.IsValid() || !CurrencyRef.IsValid())
	{
		return;
	}

	if (Model.BuybackItems.Num() == 0)
	{
		return;
	}

	// Buyback entry cuối cùng (LIFO cho buyback); server BuybackList cũng nối đuôi theo thứ tự bán.
	const int32 LastIndex = Model.BuybackItems.Num() - 1;
	const FPAShopItemEntry& Item = Model.BuybackItems[LastIndex];

	if (Model.PlayerGold < Item.FinalPrice)
	{
		return;
	}

	// X11b: previously only removed the local entry (no server call). Now the server performs the buyback and the
	// local entry is removed on confirmation.
	PendingBuybackIndex = LastIndex;
	PendingAction = EPendingShopAction::Buyback;
	PendingRequestId = RouterRef->AllocateRequestId();
	RouterRef->Server_MerchantBuybackItem(PendingRequestId, MerchantRef->GetOwner(), InventoryRef.Get(), CurrencyRef.Get(), LastIndex);
}
