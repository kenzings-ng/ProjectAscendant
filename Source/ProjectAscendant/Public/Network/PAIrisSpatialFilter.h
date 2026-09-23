// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PAIrisSpatialFilter.generated.h"

/**
 * EPAIrisSpatialTier
 *
 * 3 cấp độ phân tầng tần số đồng bộ mạng Iris theo cự ly không gian (AC-10 & ADR-0001).
 */
UENUM(BlueprintType)
enum class EPAIrisSpatialTier : uint8
{
	Tier1_HighFrequency   UMETA(DisplayName = "Cấp 1: Cận Chiến Cao Tần (60Hz, <= 15m)"),
	Tier2_MidFrequency    UMETA(DisplayName = "Cấp 2: Tầm Trung Giảm Tải (30Hz, 15m - 35m)"),
	Tier3_Dormant         UMETA(DisplayName = "Cấp 3: Ngủ Đông / Triệt Tiêu (0Hz, > 35m)")
};

/**
 * FPAIrisSpatialConfig
 *
 * Cấu hình tham số ngưỡng khoảng cách và tần số đồng bộ của hệ thống Iris Replication.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAIrisSpatialConfig
{
	GENERATED_BODY()

	/** Bán kính Cấp 1 (15m = 1500 cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Iris|Spatial")
	float Tier1Radius = 1500.0f;

	/** Bán kính Cấp 2 (35m = 3500 cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Iris|Spatial")
	float Tier2Radius = 3500.0f;

	/** Tần số đồng bộ Cấp 1 (60Hz) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Iris|Spatial")
	float Tier1Frequency = 60.0f;

	/** Tần số đồng bộ Cấp 2 (30Hz) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Iris|Spatial")
	float Tier2Frequency = 30.0f;

	/** Tần số đồng bộ Cấp 3 (0Hz - Dormant/Cull) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Iris|Spatial")
	float Tier3Frequency = 0.0f;

	/** Giới hạn trần băng thông tải xuống (Ingress): 50 KB/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Iris|Bandwidth")
	float MaxClientBandwidthIngressKbps = 50.0f;

	/** Giới hạn trần băng thông tải lên (Egress): 20 KB/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Iris|Bandwidth")
	float MaxClientBandwidthEgressKbps = 20.0f;

	FPAIrisSpatialConfig() = default;
};

/**
 * UPAIrisSpatialFilter
 *
 * Thư viện tính toán phân tầng không gian và tần số đồng bộ mạng Iris
 * cho thế giới mở MMO theo GDD multiplayer-coop.md và ADR-0001.
 */
UCLASS()
class PROJECTASCENDANT_API UPAIrisSpatialFilter : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * AC-10: Đánh giá cấp độ phân tầng Iris giữa vị trí Người quan sát và Mục tiêu.
	 * - D <= 1500 cm  => Tier 1 (60Hz)
	 * - 1500 < D <= 3500 cm => Tier 2 (30Hz)
	 * - D > 3500 cm   => Tier 3 (Dormant / Cull)
	 */
	UFUNCTION(BlueprintPure, Category = "Iris|Spatial")
	static EPAIrisSpatialTier EvaluateSpatialTier(
		const FVector& ObserverLocation,
		const FVector& TargetLocation,
		const FPAIrisSpatialConfig& Config = FPAIrisSpatialConfig());

	/**
	 * Lấy tần số đồng bộ (Hz) tương ứng với khoảng cách thực tế (cm).
	 */
	UFUNCTION(BlueprintPure, Category = "Iris|Spatial")
	static float GetReplicationFrequencyForDistance(
		float DistanceCm,
		const FPAIrisSpatialConfig& Config = FPAIrisSpatialConfig());

	/**
	 * Kiểm tra xem Actor có vượt quá ngưỡng cự ly 35m và cần bị đưa vào trạng thái ngủ đông hay không.
	 */
	UFUNCTION(BlueprintPure, Category = "Iris|Spatial")
	static bool ShouldCullReplication(
		float DistanceCm,
		const FPAIrisSpatialConfig& Config = FPAIrisSpatialConfig());

	/**
	 * Tính chu kỳ gửi gói tin (Interval tính bằng giây = 1.0 / Frequency).
	 */
	UFUNCTION(BlueprintPure, Category = "Iris|Spatial")
	static float CalculateReplicationInterval(float FrequencyHz);
};
