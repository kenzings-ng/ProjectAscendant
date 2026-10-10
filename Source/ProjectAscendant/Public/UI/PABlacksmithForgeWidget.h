// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/PAShopForgeUITypes.h"
#include "Crafting/PABlacksmithTypes.h"
#include "Economy/PACurrencyTypes.h"
#include "PABlacksmithForgeWidget.generated.h"

class UPABlacksmithComponent;
class UPAInventoryComponent;
class UPACurrencyComponent;
class UPAServiceRequestComponent;

/**
 * UPABlacksmithForgeWidget
 *
 * Widget giao diện Blacksmith Forge Anvil: ô trang bị trung tâm, vệ tinh nguyên liệu,
 * xem trước thay đổi chỉ số, và cơ chế giữ nút 0.80s (Hold-to-Craft).
 *
 * Tham chiếu GDD: design/gdd/blacksmithing-system.md
 * ADR-0001: Server-authoritative transactions
 * ADR-0003: FastArray inventory syncing
 *
 * X11b: the enhancement request goes through the player's UPAServiceRequestComponent; OnTransactionCompleted
 * fires only when the server confirmation arrives (Client_ConfirmForgeRequest). One request in flight at a time.
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTASCENDANT_API UPABlacksmithForgeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

	/**
	 * Khởi tạo Forge UI với các component tham chiếu.
	 * @param RequestRouter  X11b: router của người chơi; nullptr = lấy từ owning PlayerController
	 */
	UFUNCTION(BlueprintCallable, Category = "ForgeUI")
	void InitializeForge(UPABlacksmithComponent* ForgeComp, UPAInventoryComponent* PlayerInv,
		UPACurrencyComponent* PlayerWallet, UPAServiceRequestComponent* RequestRouter = nullptr);

	/** Đặt trang bị lên bệ rèn */
	UFUNCTION(BlueprintCallable, Category = "ForgeUI")
	void SetTargetEquipmentSlot(int32 SlotIndex);

	/** Bật/tắt Ward Stone */
	UFUNCTION(BlueprintCallable, Category = "ForgeUI")
	void ToggleWardStone(bool bUseWard);

	/** Cập nhật tiến độ giữ nút rèn (gọi mỗi frame khi input active) */
	UFUNCTION(BlueprintCallable, Category = "ForgeUI")
	void UpdateHoldInput(bool bIsHolding);

	/** Same as UpdateHoldInput with an explicit frame delta (UpdateHoldInput uses the delta cached in NativeTick). */
	void UpdateHoldInputWithDelta(float DeltaTime, bool bIsHolding);

	/** Cập nhật proximity/combat state (gọi mỗi frame) */
	UFUNCTION(BlueprintCallable, Category = "ForgeUI")
	void UpdateProximityState(float Distance, bool bCombatActive);

	// ===========================================================
	// Blueprint Getters
	// ===========================================================

	UFUNCTION(BlueprintPure, Category = "ForgeUI")
	float GetHoldProgress() const { return Model.HoldProgress; }

	UFUNCTION(BlueprintPure, Category = "ForgeUI")
	bool IsHoldCompleted() const { return Model.bHoldCompleted; }

	UFUNCTION(BlueprintPure, Category = "ForgeUI")
	bool CanForge() const { return Model.CanForge(); }

	UFUNCTION(BlueprintPure, Category = "ForgeUI")
	bool IsWidgetOpen() const { return Model.bIsOpen; }

	UFUNCTION(BlueprintPure, Category = "ForgeUI")
	const FPAForgeUIModel& GetModel() const { return Model; }

	/** X11b: true while a request awaits server confirmation. */
	UFUNCTION(BlueprintPure, Category = "ForgeUI")
	bool IsRequestPending() const { return PendingRequestId != INDEX_NONE; }

	// ===========================================================
	// Events
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnForgeHoldCompleted);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnForgeAutoClose);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnForgeTransactionCompleted, bool, bSuccess, EPACraftingError, ErrorCode);

	UPROPERTY(BlueprintAssignable, Category = "ForgeUI|Events")
	FOnForgeHoldCompleted OnForgeHoldCompleted;

	UPROPERTY(BlueprintAssignable, Category = "ForgeUI|Events")
	FOnForgeAutoClose OnForgeAutoClose;

	/**
	 * X11b: fires only on server confirmation. bSuccess=false with ErrorCode=None means the enhancement was
	 * processed but the roll failed (crft-002); any other ErrorCode means the server rejected the request.
	 */
	UPROPERTY(BlueprintAssignable, Category = "ForgeUI|Events")
	FOnForgeTransactionCompleted OnTransactionCompleted;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI")
	FPAForgeUIModel Model;

	UPROPERTY()
	TWeakObjectPtr<UPABlacksmithComponent> ForgeRef;

	UPROPERTY()
	TWeakObjectPtr<UPAInventoryComponent> InventoryRef;

	UPROPERTY()
	TWeakObjectPtr<UPACurrencyComponent> CurrencyRef;

	UPROPERTY()
	TWeakObjectPtr<UPAServiceRequestComponent> RouterRef;

	/** Cache DeltaTime cho hold update */
	float CachedDeltaTime = 0.0f;

	int32 TargetSlotIndex = INDEX_NONE;

private:
	void BindRouter(UPAServiceRequestComponent* Router);
	void UnbindRouter();
	void BindWallet(UPACurrencyComponent* Wallet);
	void UnbindWallet();
	void HandleForgeRequestConfirmed(int32 RequestId, bool bSuccess, EPACraftingError ErrorCode);

	/** X11b: gold shown follows the (replicated) wallet, not only the confirmation (dedicated-client ordering). */
	UFUNCTION()
	void HandleCurrencyBalanceChanged(EPACurrencyType Type, int64 NewBalance, int64 Delta);

	FDelegateHandle RouterConfirmHandle;
	int32 PendingRequestId = INDEX_NONE;
};
