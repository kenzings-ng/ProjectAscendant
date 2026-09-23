// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/PAShopForgeUITypes.h"
#include "PAMerchantShopWidget.generated.h"

class UPAMerchantComponent;
class UPAInventoryComponent;
class UPACurrencyComponent;

/**
 * UPAMerchantShopWidget
 *
 * Widget giao dịch Merchant Shop 2 cột: Catalog NPC (trái) | Inventory người chơi (phải).
 * Tab điều hướng: Buy / Sell / Buyback.
 *
 * Tham chiếu GDD: design/gdd/merchant-economy.md
 * ADR-0001: Server-authoritative transactions
 * ADR-0003: FastArray inventory syncing
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTASCENDANT_API UPAMerchantShopWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/**
	 * Khởi tạo Shop với các component tham chiếu.
	 * @param MerchantComp   Component catalog NPC
	 * @param PlayerInv      Inventory người chơi
	 * @param PlayerWallet   Currency người chơi
	 * @param PlayerKarma    Karma hiện tại (< -50 → surcharge 20%)
	 */
	UFUNCTION(BlueprintCallable, Category = "ShopUI")
	void InitializeShop(UPAMerchantComponent* MerchantComp, UPAInventoryComponent* PlayerInv,
		UPACurrencyComponent* PlayerWallet, int32 PlayerKarma);

	UFUNCTION(BlueprintCallable, Category = "ShopUI")
	void SwitchTab(EPAShopTab NewTab);

	UFUNCTION(BlueprintCallable, Category = "ShopUI")
	void SelectCatalogItem(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "ShopUI")
	void SelectInventoryItem(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "ShopUI")
	void ExecuteBuy();

	UFUNCTION(BlueprintCallable, Category = "ShopUI")
	void ExecuteSell();

	UFUNCTION(BlueprintCallable, Category = "ShopUI")
	void ExecuteBuyback();

	// ===========================================================
	// Blueprint Getters
	// ===========================================================

	UFUNCTION(BlueprintPure, Category = "ShopUI")
	EPAShopTab GetCurrentTab() const { return Model.CurrentTab; }

	UFUNCTION(BlueprintPure, Category = "ShopUI")
	int32 GetCatalogCount() const { return Model.CatalogItems.Num(); }

	UFUNCTION(BlueprintPure, Category = "ShopUI")
	int32 GetBuybackCount() const { return Model.BuybackItems.Num(); }

	UFUNCTION(BlueprintPure, Category = "ShopUI")
	bool IsKarmaSurchargeActive() const { return Model.bKarmaSurchargeActive; }

	UFUNCTION(BlueprintPure, Category = "ShopUI")
	int32 GetPlayerGold() const { return Model.PlayerGold; }

	UFUNCTION(BlueprintPure, Category = "ShopUI")
	const FPAShopUIModel& GetModel() const { return Model; }

	// ===========================================================
	// Events
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabChanged, EPAShopTab, NewTab);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransactionCompleted);

	UPROPERTY(BlueprintAssignable, Category = "ShopUI|Events")
	FOnTabChanged OnTabChanged;

	UPROPERTY(BlueprintAssignable, Category = "ShopUI|Events")
	FOnTransactionCompleted OnTransactionCompleted;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "ShopUI")
	FPAShopUIModel Model;

	UPROPERTY()
	TWeakObjectPtr<UPAMerchantComponent> MerchantRef;

	UPROPERTY()
	TWeakObjectPtr<UPAInventoryComponent> InventoryRef;

	UPROPERTY()
	TWeakObjectPtr<UPACurrencyComponent> CurrencyRef;
};
