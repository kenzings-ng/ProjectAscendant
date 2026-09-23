// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/PAShopForgeUITypes.h"
#include "PABlacksmithForgeWidget.generated.h"

class UPABlacksmithComponent;
class UPAInventoryComponent;
class UPACurrencyComponent;

/**
 * UPABlacksmithForgeWidget
 *
 * Widget giao diện Blacksmith Forge Anvil: ô trang bị trung tâm, vệ tinh nguyên liệu,
 * xem trước thay đổi chỉ số, và cơ chế giữ nút 0.80s (Hold-to-Craft).
 *
 * Tham chiếu GDD: design/gdd/blacksmithing-system.md
 * ADR-0001: Server-authoritative transactions
 * ADR-0003: FastArray inventory syncing
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTASCENDANT_API UPABlacksmithForgeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/**
	 * Khởi tạo Forge UI với các component tham chiếu.
	 */
	UFUNCTION(BlueprintCallable, Category = "ForgeUI")
	void InitializeForge(UPABlacksmithComponent* ForgeComp, UPAInventoryComponent* PlayerInv,
		UPACurrencyComponent* PlayerWallet);

	/** Đặt trang bị lên bệ rèn */
	UFUNCTION(BlueprintCallable, Category = "ForgeUI")
	void SetTargetEquipmentSlot(int32 SlotIndex);

	/** Bật/tắt Ward Stone */
	UFUNCTION(BlueprintCallable, Category = "ForgeUI")
	void ToggleWardStone(bool bUseWard);

	/** Cập nhật tiến độ giữ nút rèn (gọi mỗi frame khi input active) */
	UFUNCTION(BlueprintCallable, Category = "ForgeUI")
	void UpdateHoldInput(bool bIsHolding);

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

	// ===========================================================
	// Events
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnForgeHoldCompleted);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnForgeAutoClose);

	UPROPERTY(BlueprintAssignable, Category = "ForgeUI|Events")
	FOnForgeHoldCompleted OnForgeHoldCompleted;

	UPROPERTY(BlueprintAssignable, Category = "ForgeUI|Events")
	FOnForgeAutoClose OnForgeAutoClose;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI")
	FPAForgeUIModel Model;

	UPROPERTY()
	TWeakObjectPtr<UPABlacksmithComponent> ForgeRef;

	UPROPERTY()
	TWeakObjectPtr<UPAInventoryComponent> InventoryRef;

	UPROPERTY()
	TWeakObjectPtr<UPACurrencyComponent> CurrencyRef;

	/** Cache DeltaTime cho hold update */
	float CachedDeltaTime = 0.0f;
};
