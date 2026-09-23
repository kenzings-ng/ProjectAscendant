// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Economy/PAMerchantTypes.h"
#include "PAMerchantComponent.generated.h"

class UPAInventoryComponent;
class UPACurrencyComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnItemPurchased, FName, ItemId, int32, Quantity, int32, TotalCost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnItemSold, FName, ItemId, int32, Quantity, int32, TotalReceived);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnItemBuybacked, FName, ItemId, int32, Quantity, int32, GoldSpent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnTransactionFailed, EPATransactionError, ErrorCode);

/**
 * UPAMerchantComponent
 *
 * Component quản lý dịch vụ Thương Nhân (Merchant & Economy Loop, Story econ-002, ADR-0001).
 * - AC-1: Danh mục hàng hóa & giao dịch mua hàng nguyên tử.
 * - AC-2: Thu mua đồ từ túi đồ (chiết khấu 30%), bảo vệ đồ khóa, bán nhanh đồ rác (bIsJunk).
 * - AC-3: Cửa sổ mua lại (Buyback Window) 10 ô xử lý theo FIFO, hoàn tiền đúng mức đã nhận.
 * - AC-4: Khoảng cách tương tác (<= 300cm), cấm giao dịch khi đang giao chiến (In-Combat).
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAMerchantComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAMerchantComponent();

	virtual void BeginPlay() override;

	// -------------------------------------------------------------------------
	// Catalog Management
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Merchant")
	void AddCatalogEntry(const FPAMerchantCatalogEntry& Entry);

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Merchant")
	int32 GetCatalogCount() const { return Catalog.Num(); }

	const FPAMerchantCatalogEntry* GetCatalogEntry(int32 Index) const;

	// -------------------------------------------------------------------------
	// Buyback Management
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Merchant")
	int32 GetBuybackCount() const { return BuybackList.Num(); }

	const FPABuybackItemEntry* GetBuybackEntry(int32 Index) const;

	// -------------------------------------------------------------------------
	// Core Trading Operations (Server-Authoritative)
	// -------------------------------------------------------------------------

	/**
	 * AC-1: Mua vật phẩm từ danh mục thương nhân.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Merchant")
	bool BuyItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 CatalogIndex, int32 Quantity, EPATransactionError& OutError);

	/**
	 * AC-2: Bán vật phẩm từ túi đồ cho thương nhân.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Merchant")
	bool SellItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32 Quantity, EPATransactionError& OutError);

	/**
	 * AC-2: Bán nhanh toàn bộ đồ đánh dấu rác (bIsJunk == true).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Merchant")
	bool SellAllJunk(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32& OutTotalGoldReceived, int32& OutItemsSold, EPATransactionError& OutError);

	/**
	 * AC-3: Mua lại vật phẩm đã bán từ cửa sổ Buyback.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Merchant")
	bool BuybackItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 BuybackIndex, EPATransactionError& OutError);

	/**
	 * AC-4: Kiểm tra điều kiện tương tác (khoảng cách <= 300cm, không ở trạng thái giao chiến).
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Merchant")
	bool ValidateInteraction(const AActor* InteractingActor, bool bInCombat, EPATransactionError& OutError) const;

	// -------------------------------------------------------------------------
	// Server RPCs
	// -------------------------------------------------------------------------

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Merchant")
	void Server_RequestBuyItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 CatalogIndex, int32 Quantity);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Merchant")
	void Server_RequestSellItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32 Quantity);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Merchant")
	void Server_RequestSellAllJunk(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Merchant")
	void Server_RequestBuybackItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 BuybackIndex);

public:
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Merchant")
	FPAOnItemPurchased OnItemPurchased;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Merchant")
	FPAOnItemSold OnItemSold;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Merchant")
	FPAOnItemBuybacked OnItemBuybacked;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Merchant")
	FPAOnTransactionFailed OnTransactionFailed;

	static constexpr float kMaxInteractionDistance = 300.0f; // 300cm theo AC-4
	static constexpr int32 kMaxBuybackSlots = 10;          // 10 ô Buyback theo AC-3

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Merchant")
	EPAMerchantTier MerchantTier = EPAMerchantTier::Tier1_Outpost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Merchant", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VendorSellPenalty = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Merchant")
	TArray<FPAMerchantCatalogEntry> Catalog;

	UPROPERTY(BlueprintReadOnly, Category = "ProjectAscendant|Merchant")
	TArray<FPABuybackItemEntry> BuybackList;
};
