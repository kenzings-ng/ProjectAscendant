// Copyright Project Ascendant. All Rights Reserved.

#include "Network/PANetLagCompensation.h"

// -----------------------------------------------------------------------------
// FPAPositionHistoryBuffer Implementation
// -----------------------------------------------------------------------------

FPAPositionHistoryBuffer::FPAPositionHistoryBuffer()
	: HeadIndex(0)
	, Count(0)
{
}

void FPAPositionHistoryBuffer::Clear()
{
	HeadIndex = 0;
	Count = 0;
}

void FPAPositionHistoryBuffer::RecordSnapshot(float InTimestamp, const FVector& InLocation, const FRotator& InRotation, const FVector& InExtents)
{
	Buffer[HeadIndex] = FPAPastTransform(InTimestamp, InLocation, InRotation, InExtents);
	HeadIndex = (HeadIndex + 1) % kMaxHistoryCapacity;
	Count = FMath::Min(Count + 1, kMaxHistoryCapacity);
}

float FPAPositionHistoryBuffer::GetOldestTimestamp() const
{
	if (Count == 0)
	{
		return 0.0f;
	}

	if (Count < kMaxHistoryCapacity)
	{
		return Buffer[0].Timestamp;
	}

	// Khi buffer đầy, HeadIndex trỏ đúng vào phần tử cũ nhất chuẩn bị bị ghi đè
	return Buffer[HeadIndex].Timestamp;
}

float FPAPositionHistoryBuffer::GetNewestTimestamp() const
{
	if (Count == 0)
	{
		return 0.0f;
	}

	const int32 NewestIdx = (HeadIndex - 1 + kMaxHistoryCapacity) % kMaxHistoryCapacity;
	return Buffer[NewestIdx].Timestamp;
}

bool FPAPositionHistoryBuffer::GetInterpolatedSnapshot(float TargetTimestamp, FPAPastTransform& OutSnapshot) const
{
	if (Count == 0)
	{
		return false;
	}

	const float NewestTime = GetNewestTimestamp();
	// Từ chối tuyệt đối timestamp trong tương lai (TargetTimestamp > NewestTime)
	if (TargetTimestamp > NewestTime + 0.0001f)
	{
		return false;
	}

	const float OldestTime = GetOldestTimestamp();
	// Kẹp về mốc lịch sử cũ nhất (tối đa 200ms) nếu timestamp quá cũ
	const float ClampedTargetTime = FMath::Max(TargetTimestamp, OldestTime);

	if (Count == 1)
	{
		OutSnapshot = Buffer[0];
		OutSnapshot.Timestamp = ClampedTargetTime;
		return true;
	}

	// Lập danh sách chỉ số theo thứ tự thời gian tăng dần từ cũ nhất tới mới nhất
	const int32 StartIdx = (Count < kMaxHistoryCapacity) ? 0 : HeadIndex;

	for (int32 i = 0; i < Count - 1; ++i)
	{
		const int32 CurrIdx = (StartIdx + i) % kMaxHistoryCapacity;
		const int32 NextIdx = (StartIdx + i + 1) % kMaxHistoryCapacity;

		const FPAPastTransform& SnapA = Buffer[CurrIdx];
		const FPAPastTransform& SnapB = Buffer[NextIdx];

		if (ClampedTargetTime >= SnapA.Timestamp && ClampedTargetTime <= SnapB.Timestamp)
		{
			const float TimeDiff = SnapB.Timestamp - SnapA.Timestamp;
			const float Alpha = (TimeDiff > 0.00001f) ? FMath::Clamp((ClampedTargetTime - SnapA.Timestamp) / TimeDiff, 0.0f, 1.0f) : 0.0f;

			OutSnapshot.Timestamp = ClampedTargetTime;
			OutSnapshot.Location = FMath::Lerp(SnapA.Location, SnapB.Location, Alpha);
			OutSnapshot.Rotation = FMath::Lerp(SnapA.Rotation, SnapB.Rotation, Alpha);
			OutSnapshot.CapsuleExtents = FMath::Lerp(SnapA.CapsuleExtents, SnapB.CapsuleExtents, Alpha);
			return true;
		}
	}

	// Trường hợp làm tròn biên, trả về snapshot mới nhất
	const int32 LatestIdx = (HeadIndex - 1 + kMaxHistoryCapacity) % kMaxHistoryCapacity;
	OutSnapshot = Buffer[LatestIdx];
	OutSnapshot.Timestamp = ClampedTargetTime;
	return true;
}

// -----------------------------------------------------------------------------
// UPANetcodeMath Implementation
// -----------------------------------------------------------------------------

bool UPANetcodeMath::ValidateClientTimestamp(float ClientTime, float ServerTime, float MaxLagCompensation, float& OutClampedTime)
{
	// Guardrail 1: Từ chối timestamp trong tương lai
	if (ClientTime > ServerTime + 0.001f)
	{
		OutClampedTime = ServerTime;
		return false;
	}

	// Guardrail 2: Giới hạn trần tua lại tối đa (mặc định 200ms)
	const float MinAllowedTime = ServerTime - MaxLagCompensation;
	if (ClientTime < MinAllowedTime)
	{
		OutClampedTime = MinAllowedTime;
		return true; // Chấp nhận nhưng kẹp về 200ms
	}

	OutClampedTime = ClientTime;
	return true;
}

bool UPANetcodeMath::ValidateMeleeSweep(const FVector& AttackerPos, const FVector& TargetRewoundPos, float AttackRange, float TargetRadius)
{
	const float EffectiveReach = AttackRange + TargetRadius;
	const float Dist2DSquared = FVector::DistSquared2D(AttackerPos, TargetRewoundPos);
	const float ReachSquared = EffectiveReach * EffectiveReach;

	// Kiểm tra cự ly mặt phẳng 2D và chênh lệch cao độ Z
	const float VerticalDist = FMath::Abs(AttackerPos.Z - TargetRewoundPos.Z);
	constexpr float kMaxVerticalTolerance = 120.0f;

	return (Dist2DSquared <= ReachSquared) && (VerticalDist <= kMaxVerticalTolerance);
}

FPASoftReconciliationResult UPANetcodeMath::CalculateSoftReconciliation(
	const FVector& ClientPredictedPos,
	const FVector& ServerAuthoritativePos,
	float ElapsedTime,
	float TotalBlendDuration,
	float Threshold)
{
	FPASoftReconciliationResult Result;
	Result.DivergenceDistance = FVector::Dist(ClientPredictedPos, ServerAuthoritativePos);

	// AC-12: Nếu sai lệch <= 15cm, bỏ qua hoàn toàn để chống jitter camera
	if (Result.DivergenceDistance <= Threshold)
	{
		Result.bNeedsCorrection = false;
		Result.SmoothedPosition = ClientPredictedPos;
		return Result;
	}

	// Nếu sai lệch > 15cm: kích hoạt làm mịn nội suy hàm mũ qua TotalBlendDuration
	Result.bNeedsCorrection = true;
	const float Alpha = (TotalBlendDuration > 0.0f) ? FMath::Clamp(ElapsedTime / TotalBlendDuration, 0.0f, 1.0f) : 1.0f;
	Result.SmoothedPosition = FMath::Lerp(ClientPredictedPos, ServerAuthoritativePos, Alpha);

	return Result;
}

bool UPANetcodeMath::EvaluateAllyCollisionPassThrough(bool bIsSameFactionOrParty, bool bIsPvEArea)
{
	// AC-9: Trong khu vực PvE, người chơi cùng phe/nhóm được phép đi xuyên thân thể (Zero Body Blocking)
	return (bIsPvEArea && bIsSameFactionOrParty);
}
