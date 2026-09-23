// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "PABasePlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class APABaseCharacter;
class UPrimitiveComponent;
class UPALoginWidget;

/**
 * APABasePlayerController
 *
 * Player Controller cơ sở cho Project Ascendant, tích hợp Unreal Engine 5.7 Enhanced Input.
 * Quản lý Input Mapping Context (IMC_DefaultLocomotion), điều khiển di chuyển 8 hướng trực giao màn hình,
 * chiếu tia con trỏ chuột (Mouse Cursor De-projection) và điều khiển nhắm bắn độc lập (Decoupled Aiming)
 * qua Chuột hoặc Cần xoay Gamepad (Twin-Stick Aiming với deadzone 0.2).
 */
UCLASS()
class PROJECTASCENDANT_API APABasePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APABasePlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PlayerTick(float DeltaTime) override;

	// -------------------------------------------------------------------------
	// Camera Look-Ahead & Occlusion Dither (Story 003)
	// -------------------------------------------------------------------------

	/** Trả về vị trí đón đầu Look-Ahead hiện tại của camera */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Camera")
	FVector GetCurrentLookAheadOffset() const { return CurrentLookAheadOffset; }

	/** Bật/tắt tính năng Camera Look-Ahead */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Camera")
	void SetLookAheadEnabled(bool bEnabled) { bEnableLookAhead = bEnabled; }

	/** Kiểm tra trạng thái bật/tắt của Look-Ahead */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Camera")
	bool IsLookAheadEnabled() const { return bEnableLookAhead; }

	/** Trả về số lượng thành phần hiện đang bị làm mờ do che khuất tầm nhìn */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Camera")
	int32 GetCurrentlyOccludedComponentsCount() const { return CurrentlyOccludedComponents.Num(); }

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Xử lý input di chuyển 2D (WASD / Left Stick) khi đang kích hoạt */
	void HandleMoveTriggered(const FInputActionValue& Value);

	/** Xử lý khi buông phím di chuyển */
	void HandleMoveCompleted(const FInputActionValue& Value);

	/** Xử lý input nhắm bắn từ analog phải của Gamepad (Twin-stick Aiming) */
	void HandleGamepadAimTriggered(const FInputActionValue& Value);

	/** Xử lý tấn công cận chiến trực tiếp khi bấm chuột trái hoặc phím tấn công */
	void HandleDirectAttack();

	/** Xử lý lướt né I-frame khi bấm phím Spacebar */
	void HandleDirectDash();

	/** Xử lý kết liễu Finisher khi bấm phím E hoặc chuột phải */
	void HandleDirectFinisher();

	/** Cập nhật hướng ngắm từ vị trí con trỏ chuột chiếu xuống mặt đất */
	void UpdateMouseAiming();

	/** Cập nhật độ trôi đón đầu của Camera (Story 003 / AC-2) */
	void UpdateCameraLookAhead(float DeltaTime);

	/** Quét tia kiểm tra vật cản che khuất tầm nhìn và áp dụng Dither Opacity Mask (Story 003 / AC-3) */
	void UpdateCameraOcclusion(float DeltaTime);

	/** Xóa bỏ hiệu ứng dither trên các vật thể không còn che khuất */
	void ResetOccludedComponents();

protected:
	/** Lớp Widget giao diện khởi đầu / Đăng nhập / Fast Playtest (Client Viewport) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|UI")
	TSubclassOf<UPALoginWidget> LoginWidgetClass;

	/** Instance widget giao diện đang hiển thị trên viewport cục bộ */
	UPROPERTY(BlueprintReadOnly, Category = "ProjectAscendant|UI")
	TObjectPtr<UPALoginWidget> ActiveLoginWidget = nullptr;

	/** Input Mapping Context mặc định cho việc điều khiển nhân vật */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Input Action di chuyển 2 trục (2D Axis Vector) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Input Action ngắm bắn bằng cần phải Gamepad (Right Stick 2D Axis Vector) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Input")
	TObjectPtr<UInputAction> GamepadAimAction;

	/** Vùng chết cho analog stick ngắm bắn (Gamepad Deadzone = 0.20) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Input", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float GamepadAimDeadzone = 0.20f;

	/** Cho phép kích hoạt cơ chế Look-Ahead (mặc định tắt để camera cố định vào nhân vật, không trôi theo chuột) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Camera")
	bool bEnableLookAhead = false;

	/** Khoảng cách trôi tối đa của camera (250 cm - AC-2) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Camera", meta = (ClampMin = "0.0"))
	float MaxLookAheadDistance = 250.0f;

	/** Tốc độ hoàn trả tâm camera về nhân vật (8.0 - AC-2) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Camera", meta = (ClampMin = "0.1"))
	float LookAheadReturnSpeed = 8.0f;

	/** Bán kính quét vật cản che khuất tầm nhìn (200 cm - AC-3) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Camera", meta = (ClampMin = "50.0"))
	float OcclusionFadeRadius = 200.0f;

	/** Độ mờ dither áp dụng cho vật thể chắn tầm nhìn (0.25 - AC-3) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Camera", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OcclusionDitherOpacity = 0.25f;

	/** Tên thông số vật liệu điều khiển độ trong suốt Dither */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Camera")
	FName DitherMaterialParameterName = FName("DitherOpacity");

	/** Cờ đánh dấu người chơi đang tích cực sử dụng cần analog để ngắm (ưu tiên hơn chuột) */
	bool bIsActivelyUsingGamepadAim = false;

	/** Tọa độ giao cắt cuối cùng của con trỏ chuột trên mặt đất */
	FVector LastGroundHitLocation = FVector::ZeroVector;

	/** Tọa độ offset đón đầu hiện tại */
	FVector CurrentLookAheadOffset = FVector::ZeroVector;

	/** Danh sách các PrimitiveComponent đang bị làm mờ dither do che khuất tầm nhìn */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UPrimitiveComponent>> CurrentlyOccludedComponents;
};
