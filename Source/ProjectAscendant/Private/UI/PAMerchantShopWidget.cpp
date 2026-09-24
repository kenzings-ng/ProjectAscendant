// Copyright Project Ascendant. All Rights Reserved.

#include "UI/PAMerchantShopWidget.h"
#include "Economy/PAMerchantComponent.h"
#include "Economy/PAMerchantTypes.h"
#include "Economy/PACurrencyComponent.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"

// ===========================================================
// Lifecycle
// ===========================================================

void UPAMerchantShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Model.SwitchTab(EPAShopTab::Buy);
}

// ===========================================================
// Initialization
// ===========================================================

void UPAMerchantShopWidget::InitializeShop(
	UPAMerchantComponent* MerchantComp,
	UPAInventoryComponent* PlayerInv,
	UPACurrencyComponent* PlayerWallet,
	int32 PlayerKarma)
{
	MerchantRef = MerchantComp;
	InventoryRef = PlayerInv;
	CurrencyRef = PlayerWallet;

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
	if (!MerchantRef.IsValid() || !CurrencyRef.IsValid() || !InventoryRef.IsValid())
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

	// Delegate mua hàng đến MerchantComponent (Server RPC)
	MerchantRef->Server_RequestBuyItem(InventoryRef.Get(), CurrencyRef.Get(), Model.SelectedCatalogIndex, 1);
	OnTransactionCompleted.Broadcast();
}

void UPAMerchantShopWidget::ExecuteSell()
{
	if (!MerchantRef.IsValid() || !InventoryRef.IsValid() || !CurrencyRef.IsValid())
	{
		return;
	}

	if (Model.SelectedInventoryIndex < 0 || Model.SelectedInventoryIndex >= Model.InventoryItems.Num())
	{
		return;
	}

	const FPAShopItemEntry& Item = Model.InventoryItems[Model.SelectedInventoryIndex];

	// Thêm vào buyback queue trước khi bán
	Model.AddBuybackEntry(Item);

	// Delegate bán hàng đến MerchantComponent (Server RPC)
	MerchantRef->Server_RequestSellItem(InventoryRef.Get(), CurrencyRef.Get(), Model.SelectedInventoryIndex, 1);
	OnTransactionCompleted.Broadcast();
}

void UPAMerchantShopWidget::ExecuteBuyback()
{
	if (!MerchantRef.IsValid() || !CurrencyRef.IsValid())
	{
		return;
	}

	if (Model.BuybackItems.Num() == 0)
	{
		return;
	}

	// Buyback entry cuối cùng (LIFO cho buyback)
	const int32 LastIndex = Model.BuybackItems.Num() - 1;
	const FPAShopItemEntry& Item = Model.BuybackItems[LastIndex];

	if (Model.PlayerGold < Item.FinalPrice)
	{
		return;
	}

	Model.RemoveBuybackEntry(LastIndex);
	OnTransactionCompleted.Broadcast();
}
