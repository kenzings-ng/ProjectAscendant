// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PAShopForgeUITypes.generated.h"

/**
 * EPAShopTab
 *
 * Tab điều hướng trong cửa sổ Merchant Shop.
 * - Buy: Mua vật phẩm từ catalog NPC.
 * - Sell: Bán vật phẩm từ inventory người chơi.
 * - Buyback: Chuộc lại vật phẩm đã bán (FIFO, tối đa 10 entries).
 */
UENUM(BlueprintType)
enum class EPAShopTab : uint8
{
	Buy      = 0 UMETA(DisplayName = "Buy"),
	Sell     = 1 UMETA(DisplayName = "Sell"),
	Buyback  = 2 UMETA(DisplayName = "Buyback")
};

/**
 * FPAShopItemEntry
 *
 * Một dòng hiển thị vật phẩm trong cửa sổ Shop (cả catalog NPC lẫn inventory người chơi).
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAShopItemEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ShopUI")
	FName ItemId;

	UPROPERTY(BlueprintReadOnly, Category = "ShopUI")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "ShopUI")
	int32 Quantity = 1;

	UPROPERTY(BlueprintReadOnly, Category = "ShopUI")
	int32 PriceGold = 0;

	/** Có áp dụng phụ phí Karma (+20%)? */
	UPROPERTY(BlueprintReadOnly, Category = "ShopUI")
	bool bHasSurcharge = false;

	/** Giá sau phụ phí (nếu có) */
	UPROPERTY(BlueprintReadOnly, Category = "ShopUI")
	int32 FinalPrice = 0;

	/** Người chơi đủ tiền mua? */
	UPROPERTY(BlueprintReadOnly, Category = "ShopUI")
	bool bCanAfford = false;

	FPAShopItemEntry() = default;
};

/**
 * FPAForgeSlotEntry
 *
 * Một ô nguyên liệu hiển thị trên giao diện Blacksmith Forge Anvil.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAForgeSlotEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI")
	FName MaterialId;

	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI")
	FString DisplayName;

	/** Số lượng yêu cầu */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI")
	int32 RequiredAmount = 0;

	/** Số lượng hiện có trong inventory */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI")
	int32 OwnedAmount = 0;

	/** Đủ nguyên liệu? (xanh nếu true, đỏ nếu false) */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI")
	bool bIsSufficient = false;

	FPAForgeSlotEntry() = default;

	void UpdateSufficiency()
	{
		bIsSufficient = OwnedAmount >= RequiredAmount;
	}
};

/**
 * FPAStatDeltaPreview
 *
 * Hiển thị xem trước thay đổi chỉ số: "Attack: 50 -> 58 (+8)"
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAStatDeltaPreview
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI")
	FString StatName;

	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI")
	float CurrentValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI")
	float NewValue = 0.0f;

	float GetDelta() const { return NewValue - CurrentValue; }

	FString GetPreviewText() const
	{
		const float Delta = GetDelta();
		const FString Sign = Delta >= 0.0f ? TEXT("+") : TEXT("");
		return FString::Printf(TEXT("%s: %d -> %d (%s%d)"),
			*StatName,
			FMath::RoundToInt(CurrentValue),
			FMath::RoundToInt(NewValue),
			*Sign,
			FMath::RoundToInt(Delta));
	}

	FPAStatDeltaPreview() = default;
};

/**
 * FPAShopUIModel
 *
 * Data model thuần túy cho cửa sổ Merchant Shop.
 * Kiểm thử tự động 100% không cần UMG.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAShopUIModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ShopUI|Model")
	EPAShopTab CurrentTab = EPAShopTab::Buy;

	/** Catalog NPC (cột trái) */
	UPROPERTY(BlueprintReadOnly, Category = "ShopUI|Model")
	TArray<FPAShopItemEntry> CatalogItems;

	/** Inventory người chơi (cột phải) */
	UPROPERTY(BlueprintReadOnly, Category = "ShopUI|Model")
	TArray<FPAShopItemEntry> InventoryItems;

	/** Danh sách Buyback FIFO (tối đa 10) */
	UPROPERTY(BlueprintReadOnly, Category = "ShopUI|Model")
	TArray<FPAShopItemEntry> BuybackItems;

	/** Index item đang chọn trong catalog (-1 = chưa chọn) */
	UPROPERTY(BlueprintReadOnly, Category = "ShopUI|Model")
	int32 SelectedCatalogIndex = -1;

	/** Index item đang chọn trong inventory (-1 = chưa chọn) */
	UPROPERTY(BlueprintReadOnly, Category = "ShopUI|Model")
	int32 SelectedInventoryIndex = -1;

	/** Người chơi đang bị Karma surcharge? */
	UPROPERTY(BlueprintReadOnly, Category = "ShopUI|Model")
	bool bKarmaSurchargeActive = false;

	/** Số vàng hiện tại */
	UPROPERTY(BlueprintReadOnly, Category = "ShopUI|Model")
	int32 PlayerGold = 0;

	/** Giới hạn Buyback FIFO */
	static constexpr int32 MaxBuybackEntries = 10;

	// ===========================================================
	// Actions
	// ===========================================================

	void SwitchTab(EPAShopTab NewTab)
	{
		CurrentTab = NewTab;
		SelectedCatalogIndex = -1;
		SelectedInventoryIndex = -1;
	}

	void SelectCatalogItem(int32 Index)
	{
		SelectedCatalogIndex = (Index >= 0 && Index < CatalogItems.Num()) ? Index : -1;
	}

	void SelectInventoryItem(int32 Index)
	{
		SelectedInventoryIndex = (Index >= 0 && Index < InventoryItems.Num()) ? Index : -1;
	}

	/**
	 * Thêm vật phẩm vào buyback queue (FIFO). 
	 * Nếu đầy 10 slot, loại bỏ cái cũ nhất (đầu mảng).
	 */
	void AddBuybackEntry(const FPAShopItemEntry& Entry)
	{
		if (BuybackItems.Num() >= MaxBuybackEntries)
		{
			BuybackItems.RemoveAt(0);
		}
		BuybackItems.Add(Entry);
	}

	/** Xóa buyback entry sau khi chuộc lại thành công */
	void RemoveBuybackEntry(int32 Index)
	{
		if (BuybackItems.IsValidIndex(Index))
		{
			BuybackItems.RemoveAt(Index);
		}
	}

	/** Áp dụng surcharge +20% cho tất cả catalog items khi Karma < -50 */
	void ApplyKarmaSurcharge(bool bActive)
	{
		bKarmaSurchargeActive = bActive;
		for (FPAShopItemEntry& Item : CatalogItems)
		{
			Item.bHasSurcharge = bActive;
			Item.FinalPrice = bActive
				? FMath::CeilToInt(Item.PriceGold * 1.20f)
				: Item.PriceGold;
		}
	}

	/** Cập nhật trạng thái CanAfford cho catalog items */
	void UpdateAffordability(int32 Gold)
	{
		PlayerGold = Gold;
		for (FPAShopItemEntry& Item : CatalogItems)
		{
			Item.bCanAfford = Gold >= Item.FinalPrice;
		}
	}
};

/**
 * FPAForgeUIModel
 *
 * Data model thuần túy cho cửa sổ Blacksmith Forge Anvil.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAForgeUIModel
{
	GENERATED_BODY()

	/** Trang bị đặt trên bệ rèn (central anvil slot) */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI|Model")
	FName TargetEquipmentId;

	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI|Model")
	FString TargetEquipmentName;

	/** Danh sách nguyên liệu hiển thị (satellite slots) */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI|Model")
	TArray<FPAForgeSlotEntry> MaterialSlots;

	/** Chi phí vàng */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI|Model")
	int32 GoldCost = 0;

	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI|Model")
	int32 PlayerGold = 0;

	/** Ward Stone đang được sử dụng? */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI|Model")
	bool bUsingWard = false;

	/** Xem trước thay đổi chỉ số */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI|Model")
	TArray<FPAStatDeltaPreview> StatPreviews;

	// ---- Hold-to-Craft ----

	/** Thời gian cần giữ nút để rèn (0.80s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForgeUI|Config")
	float HoldDuration = 0.80f;

	/** Tiến độ giữ nút hiện tại (0.0 → 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI|Model")
	float HoldProgress = 0.0f;

	/** Đã hoàn thành hold → dispatch RPC? */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI|Model")
	bool bHoldCompleted = false;

	/** Đang giữ nút? */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI|Model")
	bool bIsHolding = false;

	// ---- Proximity / Combat Auto-Close ----

	/** Khoảng cách mở tối đa (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForgeUI|Config")
	float OpenDistance = 300.0f;

	/** Khoảng cách tự động đóng (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForgeUI|Config")
	float AutoCloseDistance = 500.0f;

	/** Khoảng cách hiện tại đến NPC/Anvil */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI|Model")
	float CurrentDistance = 0.0f;

	/** Đang trong chiến đấu? */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI|Model")
	bool bInCombat = false;

	/** Widget có đang hiển thị? */
	UPROPERTY(BlueprintReadOnly, Category = "ForgeUI|Model")
	bool bIsOpen = false;

	// ===========================================================
	// Actions
	// ===========================================================

	FPAForgeUIModel()
	{
		TargetEquipmentId = NAME_None;
		HoldDuration = 0.80f;
		HoldProgress = 0.0f;
		bHoldCompleted = false;
		bIsHolding = false;
		OpenDistance = 300.0f;
		AutoCloseDistance = 500.0f;
		CurrentDistance = 0.0f;
		bInCombat = false;
		bIsOpen = false;
	}

	void SetTargetEquipment(FName InId, const FString& InName)
	{
		TargetEquipmentId = InId;
		TargetEquipmentName = InName;
		ResetHold();
	}

	void ToggleWard(bool bUse)
	{
		bUsingWard = bUse;
	}

	bool CanAffordGold() const
	{
		return PlayerGold >= GoldCost;
	}

	bool AllMaterialsSufficient() const
	{
		for (const FPAForgeSlotEntry& Slot : MaterialSlots)
		{
			if (!Slot.bIsSufficient)
			{
				return false;
			}
		}
		return true;
	}

	bool CanForge() const
	{
		return TargetEquipmentId != NAME_None
			&& CanAffordGold()
			&& AllMaterialsSufficient();
	}

	// ===========================================================
	// Hold-to-Craft
	// ===========================================================

	/**
	 * Cập nhật tiến độ hold-to-craft.
	 * @return true nếu vừa hoàn thành hold (trigger RPC).
	 */
	bool UpdateHoldProgress(float DeltaTime, bool bHolding)
	{
		bIsHolding = bHolding;

		if (!bHolding)
		{
			// Thả nút → reset tiến độ
			HoldProgress = 0.0f;
			bHoldCompleted = false;
			return false;
		}

		if (bHoldCompleted)
		{
			// Đã hoàn thành rồi, không trigger lại
			return false;
		}

		HoldProgress = FMath::Clamp(HoldProgress + DeltaTime / HoldDuration, 0.0f, 1.0f);

		if (HoldProgress >= 1.0f)
		{
			bHoldCompleted = true;
			return true; // → dispatch Server RPC
		}

		return false;
	}

	void ResetHold()
	{
		HoldProgress = 0.0f;
		bHoldCompleted = false;
		bIsHolding = false;
	}

	// ===========================================================
	// Proximity & Auto-Close
	// ===========================================================

	/**
	 * Cập nhật khoảng cách và trạng thái chiến đấu.
	 * @return true nếu cần auto-close widget.
	 */
	bool UpdateProximity(float Distance, bool bCombat)
	{
		CurrentDistance = Distance;
		bInCombat = bCombat;

		if (bIsOpen)
		{
			if (Distance > AutoCloseDistance || bCombat)
			{
				bIsOpen = false;
				ResetHold();
				return true; // Cần close
			}
		}

		return false;
	}

	bool CanOpen() const
	{
		return CurrentDistance <= OpenDistance && !bInCombat;
	}

	void Open()
	{
		if (CanOpen())
		{
			bIsOpen = true;
			ResetHold();
		}
	}

	void Close()
	{
		bIsOpen = false;
		ResetHold();
	}
};
