// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PANetLagCompensation.generated.h"

/**
 * FPAPastTransform
 *
 * Lưu trữ snapshot vị trí, góc xoay và kích thước capsule của nhân vật tại một mốc thời gian.
 * Phục vụ cơ chế tua lại (Rewind) và thẩm định va chạm đòn đánh trên Dedicated Server.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAPastTransform
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Netcode|LagCompensation")
	float Timestamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Netcode|LagCompensation")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Netcode|LagCompensation")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Netcode|LagCompensation")
	FVector CapsuleExtents = FVector(34.0f, 34.0f, 88.0f);

	FPAPastTransform() = default;

	FPAPastTransform(float InTimestamp, const FVector& InLocation, const FRotator& InRotation, const FVector& InExtents = FVector(34.0f, 34.0f, 88.0f))
		: Timestamp(InTimestamp)
		, Location(InLocation)
		, Rotation(InRotation)
		, CapsuleExtents(InExtents)
	{
	}
};

/**
 * FPAPositionHistoryBuffer
 *
 * Circular Ring Buffer lưu trữ lịch sử vị trí capsule nhân vật trong quá khứ.
 * Dung lượng: 20 mẫu tại tần số 100Hz (chu kỳ 10ms), tương đương 200ms (0.20s).
 * Theo tiêu chuẩn AC-1 và ADR-0001: server_max_lag_compensation = 0.20s.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAPositionHistoryBuffer
{
	GENERATED_BODY()

public:
	/** Dung lượng mẫu tối đa (20 frames @ 100Hz = 200ms) */
	static constexpr int32 kMaxHistoryCapacity = 20;

	/** Khoảng thời gian tối đa lưu trữ lịch sử (200ms = 0.20s) */
	static constexpr float kMaxHistoryDuration = 0.20f;

	FPAPositionHistoryBuffer();

	/** Ghi một snapshot mới vào circular buffer */
	void RecordSnapshot(float InTimestamp, const FVector& InLocation, const FRotator& InRotation, const FVector& InExtents = FVector(34.0f, 34.0f, 88.0f));

	/**
	 * Truy vấn snapshot được nội suy tuyến tính (Lerp) tại TargetTimestamp.
	 * Trả về false nếu buffer rỗng hoặc TargetTimestamp lớn hơn mốc thời gian mới nhất (tương lai).
	 */
	bool GetInterpolatedSnapshot(float TargetTimestamp, FPAPastTransform& OutSnapshot) const;

	/** Xóa sạch buffer */
	void Clear();

	/** Số lượng mẫu hiện có trong buffer */
	int32 GetSnapshotCount() const { return Count; }

	/** Lấy timestamp của mẫu cũ nhất */
	float GetOldestTimestamp() const;

	/** Lấy timestamp của mẫu mới nhất */
	float GetNewestTimestamp() const;

private:
	/** Mảng tĩnh lưu trữ circular buffer */
	FPAPastTransform Buffer[kMaxHistoryCapacity];

	/** Chỉ số vị trí ghi kế tiếp */
	int32 HeadIndex = 0;

	/** Số lượng phần tử hiện đang lưu trữ (tối đa 20) */
	int32 Count = 0;
};

/**
 * FPASoftReconciliationResult
 *
 * Kết quả thẩm định sai lệch vị trí giữa Client và Dedicated Server (AC-12).
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPASoftReconciliationResult
{
	GENERATED_BODY()

	/** Cờ báo hiệu client có cần thực hiện hiệu chỉnh vị trí hay không */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Netcode|Reconciliation")
	bool bNeedsCorrection = false;

	/** Khoảng cách sai lệch giữa Client và Server (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Netcode|Reconciliation")
	float DivergenceDistance = 0.0f;

	/** Vị trí đã được làm mịn (interpolated) sau một khoảng delta time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Netcode|Reconciliation")
	FVector SmoothedPosition = FVector::ZeroVector;
};

/**
 * UPANetcodeMath
 *
 * Thư viện tính toán toán học mạng, Lag Compensation Rewind, Soft Reconciliation
 * và quy tắc va chạm đồng minh PvE theo GDD multiplayer-coop.md & ADR-0001.
 */
UCLASS()
class PROJECTASCENDANT_API UPANetcodeMath : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Thời gian lag compensation tối đa cho phép (0.20s = 200ms) */
	static constexpr float kDefaultMaxLagCompensation = 0.20f;

	/** Ngưỡng sai lệch vị trí kích hoạt soft reconciliation (15 cm) */
	static constexpr float kDefaultReconciliationThreshold = 15.0f;

	/** Thời lượng làm mịn hiệu chỉnh vị trí chống rung giật (0.15s) */
	static constexpr float kDefaultSoftReconciliationDuration = 0.15f;

	/**
	 * AC-1: Thẩm tra timestamp gửi từ Client.
	 * - Nếu ClientTime > ServerTime: Từ chối (timestamp trong tương lai).
	 * - Nếu ClientTime < ServerTime - MaxLagCompensation: Kẹp về mốc trần 200ms.
	 * - Ngược lại: Chấp nhận timestamp hợp lệ.
	 */
	UFUNCTION(BlueprintPure, Category = "Netcode|LagCompensation")
	static bool ValidateClientTimestamp(float ClientTime, float ServerTime, float MaxLagCompensation, float& OutClampedTime);

	/**
	 * AC-1: Thẩm định quét va chạm đòn đánh cận chiến (Melee Sweep Validation).
	 * Kiểm tra xem vị trí Attacker có nằm trong tầm đánh (AttackRange + TargetRadius) của vị trí mục tiêu đã tua lại hay không.
	 */
	UFUNCTION(BlueprintPure, Category = "Netcode|LagCompensation")
	static bool ValidateMeleeSweep(const FVector& AttackerPos, const FVector& TargetRewoundPos, float AttackRange, float TargetRadius = 34.0f);

	/**
	 * AC-12: Tính toán Soft Position Reconciliation.
	 * Nếu sai lệch > 15 cm: kích hoạt hiệu chỉnh nội suy mượt mà qua TotalBlendDuration (0.15s).
	 * Nếu sai lệch <= 15 cm: bỏ qua để chống camera giật (jitter).
	 */
	UFUNCTION(BlueprintPure, Category = "Netcode|Reconciliation")
	static FPASoftReconciliationResult CalculateSoftReconciliation(
		const FVector& ClientPredictedPos,
		const FVector& ServerAuthoritativePos,
		float ElapsedTime,
		float TotalBlendDuration = 0.15f,
		float Threshold = 15.0f);

	/**
	 * AC-9: Đánh giá va chạm giữa 2 nhân vật trong khu vực PvE (Zero Body Blocking).
	 * Trả về true nếu được phép đi xuyên qua nhau (bỏ qua Pawn collision channel).
	 * Trả về false nếu vẫn giữ va chạm cản trở (Blocking collision cho Quái vật/Boss hoặc trong PvP).
	 */
	UFUNCTION(BlueprintPure, Category = "Netcode|Collision")
	static bool EvaluateAllyCollisionPassThrough(bool bIsSameFactionOrParty, bool bIsPvEArea);
};
