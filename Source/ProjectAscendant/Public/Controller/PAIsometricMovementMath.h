// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PAIsometricMovementMath.generated.h"

/**
 * EPAAimDirection8Way
 *
 * 8 hướng nhắm bắn / định hướng trực giao theo hệ quy chiếu 2.5D Isometric.
 * Được ánh xạ trực tiếp sang các state flipbook trong PaperZD Animation Blueprint.
 */
UENUM(BlueprintType)
enum class EPAAimDirection8Way : uint8
{
	East        UMETA(DisplayName = "Đông (Phải)"),
	NorthEast   UMETA(DisplayName = "Đông Bắc (Tiến Phải)"),
	North       UMETA(DisplayName = "Bắc (Tiến Thẳng)"),
	NorthWest   UMETA(DisplayName = "Tây Bắc (Tiến Trái)"),
	West        UMETA(DisplayName = "Tây (Trái)"),
	SouthWest   UMETA(DisplayName = "Tây Nam (Lùi Trái)"),
	South       UMETA(DisplayName = "Nam (Lùi Thẳng)"),
	SouthEast   UMETA(DisplayName = "Đông Nam (Lùi Phải)")
};

/**
 * UPAIsometricMovementMath
 * 
 * Thư viện tính toán hình học không gian, chuyển đổi hệ quy chiếu 2.5D Isometric,
 * chuẩn hóa vận tốc di chuyển, chiếu tia con trỏ chuột (De-projection) và
 * ngắm bắn độc lập (Decoupled Aiming) theo GDD isometric-controller.md & ADR-0002.
 */
UCLASS()
class PROJECTASCENDANT_API UPAIsometricMovementMath : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Hằng số vận tốc đi bộ cơ bản quy chuẩn (550 cm/s) */
	static constexpr float kDefaultBaseMoveSpeed = 550.0f;

	/** Góc xoay Yaw cố định của camera isometric (45 độ) */
	static constexpr float kDefaultCameraYaw = 45.0f;

	/** Góc nghiêng Pitch cố định của camera isometric (-45 độ) */
	static constexpr float kDefaultCameraPitch = -45.0f;

	/** Cự ly tầm nhìn cố định của camera (1400 cm) */
	static constexpr float kDefaultTargetArmLength = 1400.0f;

	/** Tốc độ trễ camera làm mượt chuyển động (CameraLagSpeed = 12.0) */
	static constexpr float kDefaultCameraLagSpeed = 12.0f;

	/** Cự ly trôi đón đầu tối đa của camera theo hướng ngắm (MaxLookAheadDistance = 250 cm) */
	static constexpr float kDefaultMaxLookAheadDistance = 250.0f;

	/** Tốc độ hồi vị trí camera về tâm nhân vật (LookAheadReturnSpeed = 8.0) */
	static constexpr float kDefaultLookAheadReturnSpeed = 8.0f;

	/** Tỷ lệ khoảng cách đón đầu theo con trỏ chuột (35%) */
	static constexpr float kDefaultLookAheadDistanceScale = 0.35f;

	/** Bán kính hình trụ quét làm mờ vật cản che khuất tầm nhìn (200 cm) */
	static constexpr float kDefaultOcclusionFadeRadius = 200.0f;

	/** Độ mờ dither khi vật cản che khuất tầm nhìn (DitherOpacity = 0.25) */
	static constexpr float kDefaultOcclusionDitherOpacity = 0.25f;

	/** Hệ số suy giảm vận tốc khi ở trạng thái Kiệt Sức State.Exhausted (25%) */
	static constexpr float kExhaustedSpeedPenalty = 0.25f;

	/** Vùng chết mặc định cho analog stick ngắm bắn (Gamepad Deadzone = 0.2) */
	static constexpr float kDefaultGamepadAimDeadzone = 0.20f;

	// -------------------------------------------------------------------------
	// Locomotion & Movement Math (Story 001)
	// -------------------------------------------------------------------------

	/**
	 * Chuyển đổi vector input 2D từ hệ quy chiếu màn hình sang hướng thế giới 3D (Camera-Relative).
	 * Áp dụng ma trận xoay Yaw 45 độ:
	 *   V_world = R_yaw(CameraYaw) * [Input.Y, Input.X, 0]^T
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|MovementMath")
	static FVector CalculateWorldDirection(const FVector2D& RawInput, float CameraYaw = 45.0f);

	/**
	 * Chuẩn hóa vector input 2D, đảm bảo độ dài không vượt quá 1.0.
	 * Ngăn chặn lỗi di chuyển chéo nhanh hơn sqrt(2) ~ 1.414x.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|MovementMath")
	static FVector2D NormalizeInputVector(const FVector2D& RawInput);

	/**
	 * Kiểm tra và triệt tiêu các phím bấm đối nghịch (W + S hoặc A + D).
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|MovementMath")
	static FVector2D ResolveOpposingInputs(bool ForwardPositive, bool ForwardNegative, bool RightPositive, bool RightNegative);

	/**
	 * Tính toán vận tốc di chuyển thực tế theo trạng thái nhân vật.
	 * V_target = BaseSpeed * (1 - ExhaustedPenalty).
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|MovementMath")
	static float CalculateEffectiveMoveSpeed(float BaseSpeed = 550.0f, bool bIsExhausted = false);

	// -------------------------------------------------------------------------
	// Cursor De-projection & Decoupled Aiming (Story 002)
	// -------------------------------------------------------------------------

	/**
	 * Chiếu tia raycast từ màn hình xuống mặt phẳng nằm ngang tại cao độ chân nhân vật (GroundPlane Z).
	 * Sử dụng thuật toán giao cắt tia - mặt phẳng FMath::LinePlaneIntersection.
	 *
	 * @param RayOrigin Tọa độ gốc của tia trong thế giới (World Origin từ DeprojectScreenToWorld).
	 * @param RayDirection Vector đơn vị hướng tia (World Direction).
	 * @param GroundZ Cao độ mặt phẳng va chạm mặt đất (mặc định 0.0f hoặc Z của nhân vật).
	 * @param OutIntersection Tọa độ điểm giao cắt trên mặt phẳng Z.
	 * @return True nếu tia giao cắt thành công với mặt phẳng, False nếu tia song song với mặt đất.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|AimingMath")
	static bool DeprojectRayToGroundPlane(const FVector& RayOrigin, const FVector& RayDirection, float GroundZ, FVector& OutIntersection);

	/**
	 * Tính toán vector hướng ngắm bắn 2D chuẩn hóa trên mặt phẳng ngang (XY) từ điểm gốc đến mục tiêu.
	 *
	 * @param OriginLocation Tọa độ nhân vật.
	 * @param TargetLocation Tọa độ mục tiêu (ví dụ: điểm giao cắt của con trỏ chuột).
	 * @return Vector hướng ngắm chuẩn hóa 2D (Z = 0).
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|AimingMath")
	static FVector CalculateAimDirection2D(const FVector& OriginLocation, const FVector& TargetLocation);

	/**
	 * Tính toán góc ngắm bắn theo độ (-180 đến +180 độ) từ vector hướng 2D.
	 * Góc 0 độ hướng về phía Đông (+X), 90 độ hướng về phía Bắc (+Y).
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|AimingMath")
	static float CalculateAimAngleDegrees(const FVector& AimDirection2D);

	/**
	 * Ánh xạ góc ngắm độ (-180 đến +180) sang 8 hướng trực giao EPAAimDirection8Way
	 * phục vụ kích hoạt hoạt ảnh PaperZD flipbook độc lập với hướng di chuyển.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|AimingMath")
	static EPAAimDirection8Way Get8WayDirectionFromAngle(float AimAngleDegrees);

	/**
	 * Bộ lọc deadzone cho analog stick ngắm bắn của Gamepad (Right Stick).
	 * Nếu độ lệch analog nhỏ hơn Deadzone (0.2), trả về ZeroVector để giữ nguyên hướng ngắm cũ.
	 *
	 * @param RawRightStick Vector analog stick 2D thô từ controller.
	 * @param Deadzone Ngưỡng vùng chết (mặc định 0.20f).
	 * @return Vector đã lọc qua vùng chết, chuẩn hóa độ dài [0.0, 1.0].
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|AimingMath")
	static FVector2D FilterGamepadAimInput(const FVector2D& RawRightStick, float Deadzone = 0.20f);

	// -------------------------------------------------------------------------
	// Camera Boom & Occlusion Line-of-Sight Math (Story 003)
	// -------------------------------------------------------------------------

	/**
	 * Tính toán vector độ lệch trôi đón đầu mục tiêu (Target Look-Ahead Offset)
	 * dựa trên khoảng cách giữa nhân vật và con trỏ chuột trong không gian 2D.
	 * TargetOffset = AimDirection * Clamp(CursorDistance * DistanceScale, 0.0, MaxDistance)
	 *
	 * @param CharacterLocation Tọa độ thế giới của nhân vật.
	 * @param CursorWorldLocation Tọa độ giao cắt của con trỏ chuột trên mặt đất.
	 * @param MaxDistance Giới hạn khoảng cách trôi tối đa (mặc định 250 cm).
	 * @param DistanceScale Hệ số tỷ lệ khoảng cách (mặc định 0.35).
	 * @return Vector độ lệch không gian thế giới (Z = 0) dùng cho SocketOffset của SpringArm.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|CameraMath")
	static FVector CalculateLookAheadTargetOffset(
		const FVector& CharacterLocation,
		const FVector& CursorWorldLocation,
		float MaxDistance = 250.0f,
		float DistanceScale = 0.35f);

	/**
	 * Nội suy mượt mà độ lệch camera đón đầu hiện tại về phía mục tiêu.
	 * Sử dụng FMath::VInterpTo với tốc độ nội suy InterpSpeed.
	 *
	 * @param CurrentOffset Độ lệch hiện tại của camera.
	 * @param TargetOffset Độ lệch mục tiêu.
	 * @param DeltaTime Bước thời gian của frame.
	 * @param InterpSpeed Tốc độ nội suy (mặc định 8.0).
	 * @return Vector độ lệch đã được làm mượt.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|CameraMath")
	static FVector InterpolateLookAheadOffset(
		const FVector& CurrentOffset,
		const FVector& TargetOffset,
		float DeltaTime,
		float InterpSpeed = 8.0f);

	/**
	 * Kiểm tra xem một điểm hoặc vật cản có nằm trong hình trụ che khuất tầm nhìn
	 * giữa Camera và Nhân vật hay không (với bán kính FadeRadius).
	 *
	 * @param PointLocation Tọa độ điểm va chạm của vật cản.
	 * @param CameraLocation Vị trí đặt camera.
	 * @param CharacterLocation Vị trí nhân vật.
	 * @param FadeRadius Bán kính làm mờ hình trụ quanh đường ngắm (mặc định 200 cm).
	 * @return True nếu điểm nằm trong vùng hình trụ che tầm nhìn.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|CameraMath")
	static bool IsPointWithinOcclusionCylinder(
		const FVector& PointLocation,
		const FVector& CameraLocation,
		const FVector& CharacterLocation,
		float FadeRadius = 200.0f);
};
