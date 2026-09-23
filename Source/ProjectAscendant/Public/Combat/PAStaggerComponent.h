// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/PAStaggerTypes.h"
#include "PAStaggerComponent.generated.h"

/**
 * UPAStaggerComponent
 *
 * ActorComponent quản lý Posture, Vỡ thế (Stagger 3.0s), Đòn kết liễu 25% True Damage,
 * và Suy giảm Posture sau 4.0s.
 *
 * Tham chiếu GDD: design/gdd/stagger-system.md
 * ADR-0001: Server-authoritative state
 * ADR-0002: GAS integration & Motion Warping execution
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAStaggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAStaggerComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===========================================================
	// Public API
	// ===========================================================

	UFUNCTION(BlueprintCallable, Category = "Combat|Stagger")
	void InitializeStagger(float InMaxPosture = 800.0f);

	/**
	 * Gây sát thương Posture lên mục tiêu.
	 * @return true nếu đòn đánh này làm mục tiêu vỡ thế.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stagger")
	bool ApplyPostureDamage(float Amount);

	/** Thông báo mục tiêu nhận sát thương để reset delay suy giảm */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stagger")
	void NotifyDamageReceived();

	/**
	 * Người chơi kích hoạt đòn kết liễu tại điểm yếu.
	 * @param PlayerDistance   Khoảng cách từ người chơi đến Socket_Execution
	 * @param TargetMaxHP      Tổng máu tối đa của Trùm
	 * @return Kết quả kết liễu
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stagger")
	FPAExecutionResult RequestExecution(float PlayerDistance, float TargetMaxHP);

	// ===========================================================
	// Getters
	// ===========================================================

	UFUNCTION(BlueprintPure, Category = "Combat|Stagger")
	EPAStaggerState GetState() const { return Model.State; }

	UFUNCTION(BlueprintPure, Category = "Combat|Stagger")
	float GetCurrentPosture() const { return Model.CurrentPosture; }

	UFUNCTION(BlueprintPure, Category = "Combat|Stagger")
	float GetMaxPosture() const { return Model.MaxPosture; }

	UFUNCTION(BlueprintPure, Category = "Combat|Stagger")
	float GetPosturePercent() const { return Model.GetPosturePercent(); }

	UFUNCTION(BlueprintPure, Category = "Combat|Stagger")
	bool IsStaggered() const { return Model.IsStaggered(); }

	UFUNCTION(BlueprintPure, Category = "Combat|Stagger")
	bool IsPostureImmune() const { return Model.IsPostureImmune(); }

	UFUNCTION(BlueprintPure, Category = "Combat|Stagger")
	bool IsExecutionSocketActive() const { return Model.bSocketExecutionActive; }

	UFUNCTION(BlueprintPure, Category = "Combat|Stagger")
	const FPAStaggerModel& GetModel() const { return Model; }

	// ===========================================================
	// Delegates
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPostureBroken);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExecutionCompleted, float, DamageDealt, float, InvulnDuration);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMissedRecoveryShockwave);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaggerStateChanged, EPAStaggerState, NewState);

	UPROPERTY(BlueprintAssignable, Category = "Combat|Stagger|Events")
	FOnPostureBroken OnPostureBroken;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Stagger|Events")
	FOnExecutionCompleted OnExecutionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Stagger|Events")
	FOnMissedRecoveryShockwave OnMissedRecoveryShockwave;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Stagger|Events")
	FOnStaggerStateChanged OnStaggerStateChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Stagger")
	FPAStaggerModel Model;

	EPAStaggerState PreviousState = EPAStaggerState::Normal;
};
