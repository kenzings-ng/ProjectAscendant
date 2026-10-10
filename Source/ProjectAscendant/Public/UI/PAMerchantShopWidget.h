// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/PAShopForgeUITypes.h"
#include "Economy/PAMerchantTypes.h"
#include "Economy/PACurrencyTypes.h"
#include "PAMerchantShopWidget.generated.h"

class UPAMerchantComponent;
class UPAInventoryComponent;
class UPACurrencyComponent;
class UPAServiceRequestComponent;

/**
 * UPAMerchantShopWidget
 *
 * Widget giao dịch Merchant Shop 2 cột: Catalog NPC (trái) | Inventory người chơi (phải).
 * Tab điều hướng: Buy / Sell / Buyback.
 *
 * Tham chiếu GDD: design/gdd/merchant-economy.md
 * ADR-0001: Server-authoritative transactions
 * ADR-0003: FastArray inventory syncing
 *
 * X11b: requests go through the player's UPAServiceRequestComponent; the model is updated and
 * OnTransactionCompleted / OnTransactionRejected fire only when the server confirmation arrives
 * (Client_ConfirmMerchantRequest). One request in flight at a time.
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTASCENDANT_API UPAMerchantShopWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * Khởi tạo Shop với các component tham chiếu.
	 * @param MerchantComp   Component catalog NPC
	 * @param PlayerInv      Inventory người chơi
	 * @param PlayerWallet   Currency người chơi
	 * @param PlayerKarma    Karma hiện tại (< -50 → surcharge 20%)
	 * @param RequestRouter  X11b: router của người chơi; nullptr = lấy từ owning PlayerController
	 */
	UFUNCTION(BlueprintCallable, Category = "ShopUI")
	void InitializeShop(UPAMerchantComponent* MerchantComp, UPAInventoryComponent* PlayerInv,
		UPACurrencyComponent* PlayerWallet, int32 PlayerKarma, UPAServiceRequestComponent* RequestRouter = nullptr);

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

	/** X11b: true while a request awaits server confirmation. */
	UFUNCTION(BlueprintPure, Category = "ShopUI")
	bool IsRequestPending() const { return PendingRequestId != INDEX_NONE; }

	// ===========================================================
	// Events
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabChanged, EPAShopTab, NewTab);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransactionCompleted);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransactionRejected, EPATransactionError, ErrorCode);

	UPROPERTY(BlueprintAssignable, Category = "ShopUI|Events")
	FOnTabChanged OnTabChanged;

	/** Fires only after the server confirmed a successful transaction (X11b). */
	UPROPERTY(BlueprintAssignable, Category = "ShopUI|Events")
	FOnTransactionCompleted OnTransactionCompleted;

	/** Fires when the server rejected the pending request (X11b). */
	UPROPERTY(BlueprintAssignable, Category = "ShopUI|Events")
	FOnTransactionRejected OnTransactionRejected;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "ShopUI")
	FPAShopUIModel Model;

	UPROPERTY()
	TWeakObjectPtr<UPAMerchantComponent> MerchantRef;

	UPROPERTY()
	TWeakObjectPtr<UPAInventoryComponent> InventoryRef;

	UPROPERTY()
	TWeakObjectPtr<UPACurrencyComponent> CurrencyRef;

	UPROPERTY()
	TWeakObjectPtr<UPAServiceRequestComponent> RouterRef;

	/** Automation tests (PAServiceRoutingTests.cpp) seed model rows the UI has no public setter for. */
	friend struct FPAMerchantShopWidgetTestAccess;

private:
	enum class EPendingShopAction : uint8 { None, Buy, Sell, Buyback };

	void BindRouter(UPAServiceRequestComponent* Router);
	void UnbindRouter();
	void BindWallet(UPACurrencyComponent* Wallet);
	void UnbindWallet();
	void HandleMerchantRequestConfirmed(int32 RequestId, bool bSuccess, EPATransactionError ErrorCode);

	/** X11b: gold shown follows the (replicated) wallet, not only the confirmation (dedicated-client ordering). */
	UFUNCTION()
	void HandleCurrencyBalanceChanged(EPACurrencyType Type, int64 NewBalance, int64 Delta);

	FDelegateHandle RouterConfirmHandle;
	int32 PendingRequestId = INDEX_NONE;
	EPendingShopAction PendingAction = EPendingShopAction::None;
	FPAShopItemEntry PendingSoldItem;
	int32 PendingBuybackIndex = INDEX_NONE;
};
