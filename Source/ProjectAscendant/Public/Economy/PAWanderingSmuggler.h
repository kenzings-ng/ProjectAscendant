// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Economy/PAMerchantTypes.h"
#include "Economy/PARestockComponent.h"
#include "PAWanderingSmuggler.generated.h"

class UPAMerchantComponent;
class UPAInventoryComponent;
class UPACurrencyComponent;
enum class EPATransactionError : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnSmugglerKarmaSurcharge, int32, PlayerKarma, int32 , EffectivePrice);

/**
 * APAWanderingSmuggler
 *
 * NPC Thương Nhân Lang Thang (Tier 2 — Wandering Smuggler) xuất hiện tại các Lửa Trại
 * hoang dã (Ashen Wilderness) theo GDD merchant-economy.md §Tier 2 (Story econ-003).
 *
 * Đặc điểm:
 * - Danh mục cố định (FixedCatalog): Greater Health Flask, Greater Mana Flask, Blacksmith Ward
 *   với `AvailableStock = -1` (vô hạn).
 * - Danh mục xoay vòng (4 ô): Làm mới theo chu kỳ 3600s qua UPARestockComponent.
 * - Karma Surcharge: Người chơi Karma < -50 mua với giá +20% (ceil), vẫn được giao dịch
 *   (Tier 2 không từ chối Wanted — chỉ tính phí rủi ro).
 * - Mọi giao dịch ủy quyền cho UPAMerchantComponent (econ-002): distance ≤ 300cm, out-of-combat.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTASCENDANT_API APAWanderingSmuggler : public AActor
{
	GENERATED_BODY()

public:
	APAWanderingSmuggler();

	virtual void BeginPlay() override;

	// -------------------------------------------------------------------------
	// Purchase Entry Point (Karma-Aware)
	// -------------------------------------------------------------------------

	/**
	 * AC-1/AC-3: Mua vật phẩm từ catalog thương nhân. Áp dụng Karma surcharge 20%
	 * nếu PlayerKarma < kWantedKarmaThreshold (-50).
	 *
	 * Không từ chối giao dịch dựa trên Karma — chỉ điều chỉnh giá.
	 *
	 * @param Inventory      Túi đồ người chơi (Server-Authoritative).
	 * @param Wallet         Ví tiền người chơi.
	 * @param CatalogIndex   Chỉ số trong danh mục hàng.
	 * @param Quantity       Số lượng mua.
	 * @param PlayerKarma    Điểm Karma hiện tại của người chơi.
	 * @param OutError       Mã lỗi nếu giao dịch thất bại.
	 * @return true nếu giao dịch thành công.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Smuggler")
	bool BuyItemWithKarmaCheck(
		UPAInventoryComponent* Inventory,
		UPACurrencyComponent* Wallet,
		int32 CatalogIndex,
		int32 Quantity,
		int32 PlayerKarma,
		EPATransactionError& OutError);

	// -------------------------------------------------------------------------
	// Components
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Smuggler")
	UPAMerchantComponent* GetMerchantComponent() const { return MerchantComp; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Smuggler")
	UPARestockComponent* GetRestockComponent() const { return RestockComp; }

	// -------------------------------------------------------------------------
	// Configuration — Rotating Stock Pool
	// -------------------------------------------------------------------------

	/**
	 * Pool các món hàng xoay vòng. Khi Restock, 4 ô được chọn ngẫu nhiên từ pool này.
	 * Mỗi entry trong pool: ItemData, PriceGold, và AvailableStock ≥ 1.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Smuggler")
	TArray<FPAMerchantCatalogEntry> RotatingStockPool;

	/** Số lượng tồn kho tối thiểu khi restock ô xoay vòng (GDD: 1–3 units). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Smuggler", meta = (ClampMin = "1"))
	int32 RestockStockMin = 1;

	/** Số lượng tồn kho tối đa khi restock ô xoay vòng (GDD: 1–3 units). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Smuggler", meta = (ClampMin = "1"))
	int32 RestockStockMax = 3;

	// -------------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------------

	/** Phát khi Karma surcharge được áp dụng lên một giao dịch. */
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Smuggler")
	FPAOnSmugglerKarmaSurcharge OnKarmaSurchargeApplied;

	// -------------------------------------------------------------------------
	// Constants (GDD §Tier 2)
	// -------------------------------------------------------------------------

	/** Ngưỡng Karma xác định trạng thái Wanted/Red Name (GDD: Karma < -50). */
	static constexpr int32 kWantedKarmaThreshold = -50;

	/** Tỷ lệ phụ phí Wanted tại Tier 2 (GDD: 20%). */
	static constexpr float kWantedSurchargeRatio = 0.20f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Smuggler")
	TObjectPtr<UPAMerchantComponent> MerchantComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Smuggler")
	TObjectPtr<UPARestockComponent> RestockComp;

private:
	/** Khởi tạo danh mục cố định (Fixed Catalog) — chỉ chạy 1 lần khi BeginPlay(). */
	void InitializeFixedCatalog();

	/** Khởi tạo pool xoay vòng mặc định nếu chưa có (default items từ GDD). */
	void EnsureDefaultRotatingPool();

	/** Callback nhận từ UPARestockComponent::OnRestocked — re-rolls rotating catalog slots. */
	UFUNCTION()
	void OnRestockApplied(float RestockServerTime);
};
