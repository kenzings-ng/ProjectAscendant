// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDAnimInstance.h"
#include "Controller/PAIsometricMovementMath.h"
#include "PAPaperZDAnimInstance.generated.h"

class APABaseCharacter;
class UAbilitySystemComponent;

/**
 * UPAPaperZDAnimInstance
 *
 * Lớp cơ sở Animation Instance PaperZD cho Project Ascendant.
 * Quản lý các biến chuyển trạng thái State Machine:
 *  - Idle <-> Locomotion (dựa trên vận tốc di chuyển Speed > 10.0f)
 *  - Dash (dựa trên Gameplay Tag: State.Dashing)
 *  - LightAttack / Combo (dựa trên Gameplay Tag: State.Attacking)
 *  - Hurt (dựa trên Gameplay Tag: State.Hurt)
 *  - Stunned / Posture Broken (dựa trên Gameplay Tag: State.Broken hoặc State.Stunned)
 *  - Dead (dựa trên Gameplay Tag: State.Dead)
 * Đồng thời tự động lật mặt sprite 2D (Horizontal Sprite Flipping) theo góc ngắm độc lập (Decoupled Aiming).
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTASCENDANT_API UPAPaperZDAnimInstance : public UPaperZDAnimInstance
{
	GENERATED_BODY()

public:
	UPAPaperZDAnimInstance();

	virtual void OnInit_Implementation() override;
	virtual void OnTick_Implementation(float DeltaTime) override;

	/** Cập nhật toàn bộ các biến trạng thái Animation từ Owning Character và GAS */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Animation")
	void UpdateAnimationVariables(float DeltaTime);

	/** Xử lý lật mặt sprite (bFacingRight = false khi ngắm hoặc di chuyển sang bên trái) */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Animation")
	void UpdateDirectionalFacing();

public:
	// -------------------------------------------------------------------------
	// State Machine Animation Variables
	// -------------------------------------------------------------------------

	/** Vận tốc mặt đất hiện tại của nhân vật (cm/s) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Animation")
	float CurrentSpeed;

	/** Cờ đánh dấu nhân vật đang di chuyển (CurrentSpeed > 10.0f) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Animation")
	bool bIsMoving;

	/** Cờ đánh dấu đang lướt né I-frame (State.Dashing) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Animation")
	bool bIsDashing;

	/** Cờ đánh dấu đang tung đòn đánh (State.Attacking) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Animation")
	bool bIsAttacking;

	/** Cờ đánh dấu đang trúng đòn khựng lại (State.Hurt) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Animation")
	bool bIsHurt;

	/** Cờ đánh dấu bị phá vỡ thể khí hoặc choáng (State.Broken / State.Stunned) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Animation")
	bool bIsStunned;

	/** Cờ đánh dấu đã tử trận (State.Dead) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Animation")
	bool bIsDead;

	/** Hướng quay mặt sprite: true = Phải (Yaw 0), false = Trái (Yaw 180 / Scale -1) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Animation")
	bool bFacingRight;

	/** Hướng 8 chiều hiện tại theo con trỏ ngắm */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Animation")
	EPAAimDirection8Way CurrentAimDirection8Way;

protected:
	/** Khử trùng lặp logic cache tham chiếu tới APABaseCharacter và AbilitySystemComponent */
	void CacheOwnerReferencesIfNeeded();

	/** Tham chiếu cache tới APABaseCharacter sở hữu */
	UPROPERTY(Transient)
	TWeakObjectPtr<APABaseCharacter> OwningPABaseCharacter;

	/** Tham chiếu cache tới AbilitySystemComponent */
	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;
};
