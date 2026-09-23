// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/PAInventoryTypes.h"
#include "PAMerchantTypes.generated.h"

class UItemStaticDataAsset;

/**
 * EPAMerchantTier
 *
 * 3 cấp độ thương nhân theo GDD merchant-economy.md:
 * - Tier 1: Thương nhân Tiền Trạm (Outpost Provisioner) - Hàng cơ bản, từ chối người chơi Wanted.
 * - Tier 2: Thương nhân Lang Thang (Wandering Smuggler) - Hàng bảo hộ, phụ phí 20% cho Wanted.
 * - Tier 3: Chợ Đen Cấm Địa (Sanctum Black Market Broker) - Đồ cấm, nộp phạt chuộc tội Karma Bailout.
 */
UENUM(BlueprintType)
enum class EPAMerchantTier : uint8
{
	None              = 0 UMETA(DisplayName = "None / Unassigned"),
	Tier1_Outpost     = 1 UMETA(DisplayName = "Tier 1: Outpost Provisioner"),
	Tier2_Wilderness  = 2 UMETA(DisplayName = "Tier 2: Wandering Smuggler"),
	Tier3_Sanctuary   = 3 UMETA(DisplayName = "Tier 3: Sanctum Black Market")
};

/**
 * EPATransactionError
 *
 * Mã lỗi phản hồi khi giao dịch mua/bán với thương nhân bị từ chối.
 */
UENUM(BlueprintType)
enum class EPATransactionError : uint8
{
	None                  = 0 UMETA(DisplayName = "None"),
	InsufficientGold      = 1 UMETA(DisplayName = "Insufficient Gold"),
	InventoryFull         = 2 UMETA(DisplayName = "Inventory Full"),
	OutOfStock            = 3 UMETA(DisplayName = "Item Out Of Stock"),
	ItemNotFound          = 4 UMETA(DisplayName = "Item Not Found In Slot"),
	ItemLocked            = 5 UMETA(DisplayName = "Item Is Locked"),
	DistanceExceeded      = 6 UMETA(DisplayName = "Interaction Distance Exceeded (>300cm)"),
	InCombat              = 7 UMETA(DisplayName = "Cannot Trade In Combat"),
	BuybackEmpty          = 8 UMETA(DisplayName = "Buyback Slot Is Empty"),
	ServerRejected        = 9 UMETA(DisplayName = "Server Rejected Transaction")
};

/**
 * FPAMerchantCatalogEntry
 *
 * Định nghĩa một món hàng trong danh mục bày bán của Thương Nhân.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAMerchantCatalogEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant|Catalog")
	TObjectPtr<UItemStaticDataAsset> ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant|Catalog", meta = (ClampMin = "0"))
	int32 PriceGold = 0;

	/** Số lượng tồn kho hiện tại (-1 là vô hạn) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant|Catalog")
	int32 AvailableStock = -1;

	FPAMerchantCatalogEntry()
		: ItemData(nullptr)
		, PriceGold(0)
		, AvailableStock(-1)
	{
	}
};

/**
 * FPABuybackItemEntry
 *
 * Ô lưu trữ vật phẩm đã bán trong phiên giao dịch để hỗ trợ chuộc lại (Buyback Window).
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPABuybackItemEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Merchant|Buyback")
	TObjectPtr<UItemStaticDataAsset> ItemData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Merchant|Buyback")
	int32 Quantity = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Merchant|Buyback")
	FPAItemInstanceData DynamicData;

	UPROPERTY(BlueprintReadOnly, Category = "Merchant|Buyback")
	int32 BuybackPriceGold = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Merchant|Buyback")
	FGuid ItemInstanceUID;

	FPABuybackItemEntry()
		: ItemData(nullptr)
		, Quantity(1)
		, BuybackPriceGold(0)
		, ItemInstanceUID(FGuid())
	{
	}
};

/**
 * FPAMerchantFormulas
 *
 * Công thức kinh tế mua bán và tỷ lệ chiết khấu thương nhân.
 */
struct PROJECTASCENDANT_API FPAMerchantFormulas
{
	/**
	 * AC-2: Giá thu mua vật phẩm từ người chơi:
	 * vendor_sell_price = floor(base_price * vendor_sell_penalty)
	 */
	static int32 CalculateSellPrice(int32 BaseSellPrice, float VendorPenalty = 0.30f)
	{
		if (BaseSellPrice <= 0)
		{
			return 0;
		}

		const float RawPrice = static_cast<float>(BaseSellPrice) * FMath::Clamp(VendorPenalty, 0.0f, 1.0f);
		return FMath::Max(1, FMath::FloorToInt(RawPrice));
	}

	/**
	 * Phụ phí bán cho người chơi Wanted tại Tier 2 (+20%):
	 * wanted_surcharge_price = ceil(base_price * (1.0 + surcharge_ratio))
	 */
	static int32 CalculateWantedSurchargePrice(int32 BasePrice, float SurchargeRatio = 0.20f)
	{
		if (BasePrice <= 0)
		{
			return 0;
		}

		const float Raw = static_cast<float>(BasePrice) * (1.0f + SurchargeRatio);
		return FMath::CeilToInt(Raw);
	}

	/**
	 * Chi phí nộp phạt chuộc tội Karma Bailout tại Chợ Đen Tier 3:
	 * karma_bailout_cost = abs(karma) * 50 Gold
	 */
	static int32 CalculateKarmaBailoutCost(int32 Karma, int32 GoldPerPoint = 50)
	{
		return FMath::Abs(Karma) * GoldPerPoint;
	}
};
