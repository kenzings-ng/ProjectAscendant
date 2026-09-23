// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/PADashTypes.h"
#include "PADashEvasionComponent.generated.h"

/**
 * UPADashEvasionComponent
 *
 * ActorComponent quản lý Kỹ thuật Né đòn (Dash), Bất tử tuyệt đối I-Frame 0.28s,
 * Cửa sổ vàng Perfect Dodge 0.05s-0.15s, Hitstop 0.08s, và chống rơi mép vực.
 *
 * Tham chiếu GDD: design/gdd/dash-evasion.md
 * ADR-0001: Server-authoritative validation
 * ADR-0002: GAS State.Invulnerable & PaperZD integration
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPADashEvasionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPADashEvasionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===========================================================
	// Public API
	// ===========================================================

	/**
	 * Khởi động cú lướt né đòn.
	 * @param CurrentStamina        Thể lực người chơi hiện tại
	 * @param OutStaminaRemaining   Thể lực còn lại sau khi trừ chi phí (25)
	 * @return true nếu bắt đầu thành công
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Dash")
	bool StartDash(float CurrentStamina, float& OutStaminaRemaining);

	/**
	 * Cố gắng hủy phục hồi để tung đòn Dash Attack (t >= 0.35s).
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Dash")
	bool TryCancelIntoAttack();

	/**
	 * Xử lý khi bị kẻ địch đánh trúng trong cú lướt.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Dash")
	FPAIncomingHitResult ProcessIncomingHit(float RawDamage);

	// ===========================================================
	// Getters
	// ===========================================================

	UFUNCTION(BlueprintPure, Category = "Combat|Dash")
	bool IsDashing() const { return Model.bIsDashing; }

	UFUNCTION(BlueprintPure, Category = "Combat|Dash")
	bool IsInvulnerable() const { return Model.bInvulnerable; }

	UFUNCTION(BlueprintPure, Category = "Combat|Dash")
	bool CanCancelIntoAttack() const { return Model.bCanCancelIntoAttack; }

	UFUNCTION(BlueprintPure, Category = "Combat|Dash")
	bool CanWalkOffLedges() const { return Model.bCanWalkOffLedges; }

	UFUNCTION(BlueprintPure, Category = "Combat|Dash")
	EPADashPhase GetDashPhase() const { return Model.CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "Combat|Dash")
	float GetCurrentVelocity() const { return Model.CurrentVelocity; }

	UFUNCTION(BlueprintPure, Category = "Combat|Dash")
	bool IsPerfectDodgeTriggered() const { return Model.bPerfectDodgeTriggered; }

	UFUNCTION(BlueprintPure, Category = "Combat|Dash")
	const FPADashModel& GetModel() const { return Model; }

	// ===========================================================
	// Delegates
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDashStarted);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDashCompleted);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerfectDodgeTriggered, float, StaminaRefund, float, HitstopDuration);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDashAttackCancelled);

	UPROPERTY(BlueprintAssignable, Category = "Combat|Dash|Events")
	FOnDashStarted OnDashStarted;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Dash|Events")
	FOnDashCompleted OnDashCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Dash|Events")
	FOnPerfectDodgeTriggered OnPerfectDodgeTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Dash|Events")
	FOnDashAttackCancelled OnDashAttackCancelled;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Dash")
	FPADashModel Model;
};
