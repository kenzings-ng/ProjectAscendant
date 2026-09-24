// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PARestockComponent.generated.h"

class UPAMerchantComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnRestocked, float, RestockServerTime);
DECLARE_MULTICAST_DELEGATE_OneParam(FPAOnRestockedNative, float);

/**
 * UPARestockComponent
 *
 * Component quản lý chu kỳ làm mới hàng tồn kho (Restock Timer) của NPC Thương Nhân
 * theo GDD merchant-economy.md §Cơ Chế Hàng Tồn Kho & Restock (Story econ-003, ADR-0001).
 *
 * - Restock Timer chạy độc quyền trên Dedicated Server (HasAuthority()).
 * - Thời điểm restock được đồng bộ toàn server: tính từ boundary của khoảng thời gian
 *   (ví dụ: mỗi 3600s tính từ epoch) — toàn bộ NPC cùng Tier restock đồng loạt.
 * - NextRestockServerTime được replicate về Client phục vụ UI đếm ngược.
 * - Delegate OnRestocked broadcast khi hàng xoay vòng được làm mới.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPARestockComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPARestockComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// -------------------------------------------------------------------------
	// Configuration
	// -------------------------------------------------------------------------

	/** Khoảng thời gian giữa hai lần restock (giây). Tier 2 = 3600s (60 phút). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Restock", meta = (ClampMin = "60.0"))
	float RestockIntervalSeconds = 3600.0f;

	/** Số ô hàng xoay vòng cần làm mới mỗi chu kỳ. Tier 2 = 4. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Restock", meta = (ClampMin = "0"))
	int32 RotatingSlotCount = 4;

	/** Số lượng cổng đầu trong catalog dành cho hàng cố định (fixed, không bị thay thế khi restock). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Restock", meta = (ClampMin = "0"))
	int32 FixedCatalogCount = 3;

	// -------------------------------------------------------------------------
	// Restock Execution
	// -------------------------------------------------------------------------

	/**
	 * Khởi chạy Restock Timer. Gọi từ APAWanderingSmuggler::BeginPlay() sau khi
	 * MerchantComponent và RotatingStockPool đã được khởi tạo.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Restock")
	void StartRestockTimer(UPAMerchantComponent* MerchantComp);

	/**
	 * Thực thi ngay một lần Restock: làm mới toàn bộ RotatingSlotCount ô xoay vòng
	 * trong Catalog của MerchantComp, reset AvailableStock theo StockRange.
	 * Gọi trực tiếp trong tests hoặc khi server khởi động (initial seeding).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Restock")
	void ExecuteRestock(UPAMerchantComponent* MerchantComp);

	// -------------------------------------------------------------------------
	// Countdown Query (for UI)
	// -------------------------------------------------------------------------

	/** Thời gian server (GetWorld()->GetTimeSeconds()) của lần restock tiếp theo. Replicated. */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "ProjectAscendant|Restock")
	float NextRestockServerTime = 0.0f;

	/** Lấy số giây còn lại đến lần restock tiếp theo (client-side estimation). */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Restock")
	float GetSecondsUntilNextRestock() const;

	// -------------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------------

	/** Phát tín hiệu khi hàng xoay vòng được làm mới. */
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Restock")
	FPAOnRestocked OnRestocked;

	FPAOnRestockedNative OnRestockedNative;

protected:
	UPROPERTY()
	TWeakObjectPtr<UPAMerchantComponent> CachedMerchantComp;

private:
	FTimerHandle RestockTimerHandle;

	/** Tính thời điểm restock tiếp theo căn chỉnh theo boundary toàn server. */
	float CalculateAlignedNextRestockTime() const;

	void OnRestockTimerFired();
};
