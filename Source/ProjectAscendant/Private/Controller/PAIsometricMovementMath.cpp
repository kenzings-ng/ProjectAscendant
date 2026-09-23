// Copyright Project Ascendant. All Rights Reserved.

#include "Controller/PAIsometricMovementMath.h"

// -----------------------------------------------------------------------------
// Locomotion & Movement Math (Story 001)
// -----------------------------------------------------------------------------

FVector UPAIsometricMovementMath::CalculateWorldDirection(const FVector2D& RawInput, float CameraYaw)
{
	const FVector2D ClampedInput = NormalizeInputVector(RawInput);

	if (ClampedInput.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	const FRotator YawRotation(0.0f, CameraYaw, 0.0f);
	const FRotationMatrix RotMatrix(YawRotation);

	const FVector ForwardDirection = RotMatrix.GetUnitAxis(EAxis::X);
	const FVector RightDirection = RotMatrix.GetUnitAxis(EAxis::Y);

	FVector WorldDirection = (ForwardDirection * ClampedInput.Y) + (RightDirection * ClampedInput.X);

	WorldDirection.Z = 0.0f;
	return WorldDirection.GetClampedToMaxSize(1.0f);
}

FVector2D UPAIsometricMovementMath::NormalizeInputVector(const FVector2D& RawInput)
{
	const float SizeSquared = RawInput.SizeSquared();

	if (SizeSquared <= KINDA_SMALL_NUMBER)
	{
		return FVector2D::ZeroVector;
	}

	if (SizeSquared > 1.0f)
	{
		return RawInput.GetSafeNormal();
	}

	return RawInput;
}

FVector2D UPAIsometricMovementMath::ResolveOpposingInputs(bool ForwardPositive, bool ForwardNegative, bool RightPositive, bool RightNegative)
{
	FVector2D ResolvedInput = FVector2D::ZeroVector;

	if (ForwardPositive && !ForwardNegative)
	{
		ResolvedInput.Y = 1.0f;
	}
	else if (!ForwardPositive && ForwardNegative)
	{
		ResolvedInput.Y = -1.0f;
	}

	if (RightPositive && !RightNegative)
	{
		ResolvedInput.X = 1.0f;
	}
	else if (!RightPositive && RightNegative)
	{
		ResolvedInput.X = -1.0f;
	}

	return NormalizeInputVector(ResolvedInput);
}

float UPAIsometricMovementMath::CalculateEffectiveMoveSpeed(float BaseSpeed, bool bIsExhausted)
{
	if (bIsExhausted)
	{
		return BaseSpeed * (1.0f - kExhaustedSpeedPenalty);
	}

	return BaseSpeed;
}

// -----------------------------------------------------------------------------
// Cursor De-projection & Decoupled Aiming (Story 002)
// -----------------------------------------------------------------------------

bool UPAIsometricMovementMath::DeprojectRayToGroundPlane(const FVector& RayOrigin, const FVector& RayDirection, float GroundZ, FVector& OutIntersection)
{
	// Nếu hướng tia song song với mặt phẳng ngang (RayDirection.Z gần bằng 0), không thể xác định giao điểm
	if (FMath::IsNearlyZero(RayDirection.Z, KINDA_SMALL_NUMBER))
	{
		OutIntersection = RayOrigin;
		OutIntersection.Z = GroundZ;
		return false;
	}

	// Định nghĩa mặt phẳng ngang tại cao độ GroundZ với vector pháp tuyến hướng lên (Z = +1)
	const FPlane GroundPlane(FVector(0.0f, 0.0f, GroundZ), FVector::UpVector);

	// Tính giao điểm của tia với mặt phẳng
	OutIntersection = FMath::LinePlaneIntersection(RayOrigin, RayOrigin + (RayDirection * 50000.0f), GroundPlane);
	OutIntersection.Z = GroundZ; // Đảm bảo làm tròn chính xác cao độ mặt phẳng
	return true;
}

FVector UPAIsometricMovementMath::CalculateAimDirection2D(const FVector& OriginLocation, const FVector& TargetLocation)
{
	FVector AimDelta = TargetLocation - OriginLocation;
	AimDelta.Z = 0.0f; // Bỏ qua trục Z để tính toán hướng phẳng ngang 2D

	if (AimDelta.IsNearlyZero(KINDA_SMALL_NUMBER))
	{
		// Nếu con trỏ chuột nằm ngay chính xác vị trí nhân vật, mặc định hướng về phía trước
		return FVector(1.0f, 0.0f, 0.0f);
	}

	return AimDelta.GetSafeNormal();
}

float UPAIsometricMovementMath::CalculateAimAngleDegrees(const FVector& AimDirection2D)
{
	// Tính góc bằng Atan2(Y, X) và chuyển sang độ (-180 đến +180)
	const float Radians = FMath::Atan2(AimDirection2D.Y, AimDirection2D.X);
	return FMath::RadiansToDegrees(Radians);
}

EPAAimDirection8Way UPAIsometricMovementMath::Get8WayDirectionFromAngle(float AimAngleDegrees)
{
	// Chuẩn hóa góc về khoảng [-180, +180]
	float NormalizedAngle = FRotator::NormalizeAxis(AimAngleDegrees);

	// Phân đoạn 8 hướng trực giao, mỗi hướng chiếm 45 độ (tâm góc +/- 22.5 độ):
	// East (Đông):        [-22.5, +22.5]
	// NorthEast (Đông Bắc): [+22.5, +67.5]
	// North (Bắc):        [+67.5, +112.5]
	// NorthWest (Tây Bắc): [+112.5, +157.5]
	// West (Tây):         [+157.5, 180] hoặc [-180, -157.5]
	// SouthWest (Tây Nam): [-157.5, -112.5]
	// South (Nam):        [-112.5, -67.5]
	// SouthEast (Đông Nam): [-67.5, -22.5]

	if (NormalizedAngle >= -22.5f && NormalizedAngle < 22.5f)
	{
		return EPAAimDirection8Way::East;
	}
	else if (NormalizedAngle >= 22.5f && NormalizedAngle < 67.5f)
	{
		return EPAAimDirection8Way::NorthEast;
	}
	else if (NormalizedAngle >= 67.5f && NormalizedAngle < 112.5f)
	{
		return EPAAimDirection8Way::North;
	}
	else if (NormalizedAngle >= 112.5f && NormalizedAngle < 157.5f)
	{
		return EPAAimDirection8Way::NorthWest;
	}
	else if (NormalizedAngle >= 157.5f || NormalizedAngle < -157.5f)
	{
		return EPAAimDirection8Way::West;
	}
	else if (NormalizedAngle >= -157.5f && NormalizedAngle < -112.5f)
	{
		return EPAAimDirection8Way::SouthWest;
	}
	else if (NormalizedAngle >= -112.5f && NormalizedAngle < -67.5f)
	{
		return EPAAimDirection8Way::South;
	}
	else // [-67.5f, -22.5f)
	{
		return EPAAimDirection8Way::SouthEast;
	}
}

FVector2D UPAIsometricMovementMath::FilterGamepadAimInput(const FVector2D& RawRightStick, float Deadzone)
{
	const float Magnitude = RawRightStick.Size();

	// Nếu cần analog chưa vượt quá ngưỡng deadzone, trả về ZeroVector để giữ nguyên hướng ngắm cũ
	if (Magnitude <= Deadzone)
	{
		return FVector2D::ZeroVector;
	}

	// Tái chuẩn hóa thang giá trị tuyến tính ngoài vùng deadzone: (Magnitude - Deadzone) / (1.0 - Deadzone)
	const float NormalizedMagnitude = FMath::Clamp((Magnitude - Deadzone) / (1.0f - Deadzone), 0.0f, 1.0f);
	return (RawRightStick / Magnitude) * NormalizedMagnitude;
}

// -----------------------------------------------------------------------------
// Camera Boom & Occlusion Line-of-Sight Math (Story 003)
// -----------------------------------------------------------------------------

FVector UPAIsometricMovementMath::CalculateLookAheadTargetOffset(
	const FVector& CharacterLocation,
	const FVector& CursorWorldLocation,
	float MaxDistance,
	float DistanceScale)
{
	const float CursorDist = FVector::Dist2D(CharacterLocation, CursorWorldLocation);
	const float ClampedDist = FMath::Clamp(CursorDist * DistanceScale, 0.0f, MaxDistance);

	const FVector AimDirection = CalculateAimDirection2D(CharacterLocation, CursorWorldLocation);
	return AimDirection * ClampedDist;
}

FVector UPAIsometricMovementMath::InterpolateLookAheadOffset(
	const FVector& CurrentOffset,
	const FVector& TargetOffset,
	float DeltaTime,
	float InterpSpeed)
{
	return FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaTime, InterpSpeed);
}

bool UPAIsometricMovementMath::IsPointWithinOcclusionCylinder(
	const FVector& PointLocation,
	const FVector& CameraLocation,
	const FVector& CharacterLocation,
	float FadeRadius)
{
	const FVector LineSegment = CharacterLocation - CameraLocation;
	const float LineLengthSq = LineSegment.SizeSquared();

	if (LineLengthSq <= KINDA_SMALL_NUMBER)
	{
		return FVector::DistSquared(PointLocation, CharacterLocation) <= FMath::Square(FadeRadius);
	}

	// Chiếu điểm PointLocation lên đoạn thẳng nối giữa Camera và Nhân vật: T = Clamp(dot(AP, AB) / |AB|^2, 0, 1)
	const float T = FMath::Clamp(FVector::DotProduct(PointLocation - CameraLocation, LineSegment) / LineLengthSq, 0.0f, 1.0f);
	const FVector ClosestPointOnSegment = CameraLocation + (LineSegment * T);

	const float DistSqToSegment = FVector::DistSquared(PointLocation, ClosestPointOnSegment);
	return DistSqToSegment <= FMath::Square(FadeRadius);
}

