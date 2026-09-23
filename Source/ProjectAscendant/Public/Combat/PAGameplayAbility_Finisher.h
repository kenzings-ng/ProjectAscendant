// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "PAGameplayAbility_Finisher.generated.h"

class APABaseCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnFinisherExecuted, AActor*, TargetActor, float, DamageDealt);

/**
 * UPAGameplayAbility_Finisher
 *
 * GameplayAbility thực thi Đòn Kết Liễu Phá Thế (Posture Finisher Execution) - AC-3.
 * Điều kiện kích hoạt: Mục tiêu mang thẻ State.Broken và cự ly <= 250cm.
 * Hiệu ứng:
 *  - Mục tiêu bị khóa choáng 1.5s (State.Stunned).
 *  - Người tung chiêu nhận bất tử 1.5s (State.Invulnerable).
 *  - Gây đúng 25% Max HP sát thương chuẩn (Pure Damage, bỏ qua giáp).
 *  - Hóa giải và reset thanh thế đứng (Posture) của mục tiêu về 0.
 */
UCLASS()
class PROJECTASCENDANT_API UPAGameplayAbility_Finisher : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPAGameplayAbility_Finisher();

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	/** Delegate phát khi đòn kết liễu thực thi thành công (AC-3) */
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Combat|Finisher")
	FPAOnFinisherExecuted OnFinisherExecuted;

	/** Thiết lập mục tiêu kết liễu trước khi kích hoạt chiêu */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Combat|Finisher")
	void SetExecutionTarget(AActor* InTarget) { ExecutionTarget = InTarget; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Finisher")
	const FGameplayTagContainer& GetActivationBlockedTagContainer() const { return ActivationBlockedTags; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Finisher")
	AActor* GetExecutionTarget() const { return ExecutionTarget.Get(); }

	/** Tìm kiếm mục tiêu hợp lệ gần nhất thỏa mãn điều kiện kết liễu (State.Broken, <= 250cm) */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Combat|Finisher")
	AActor* FindBestExecutionTarget(const AActor* Attacker) const;

	/** Thẩm định mục tiêu kết liễu hợp lệ theo khoảng cách và thẻ trạng thái State.Broken */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Finisher")
	bool ValidateExecutionTarget(const AActor* Attacker, const AActor* Target) const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Finisher")
	float GetFinisherDuration() const { return FinisherDuration; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Finisher")
	float GetMaxExecutionDistance() const { return MaxExecutionDistance; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Finisher")
	float GetFinisherDamageRatio() const { return FinisherDamageRatio; }

protected:
	/** Cự ly tối đa cho phép kích hoạt đòn kết liễu (mặc định 250cm) - AC-3 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Finisher|Config")
	float MaxExecutionDistance = 250.0f;

	/** Thời lượng khóa hoạt ảnh và bất tử (mặc định 1.5s) - AC-3 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Finisher|Config")
	float FinisherDuration = 1.5f;

	/** Tỷ lệ phần trăm Max HP sát thương chuẩn (mặc định 0.25 = 25%) - AC-3 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Finisher|Config")
	float FinisherDamageRatio = 0.25f;

	/** Thẻ kỹ năng kết liễu (Ability.Skill.Finisher) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Finisher|Tags")
	FGameplayTag TagAbilityFinisher;

	/** Thẻ bất tử của người tung chiêu (State.Invulnerable) - AC-3 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Finisher|Tags")
	FGameplayTag TagStateInvulnerable;

	/** Thẻ choáng của nạn nhân (State.Stunned) - AC-3 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Finisher|Tags")
	FGameplayTag TagStateStunned;

	/** Thẻ điều kiện mục tiêu bị phá vỡ thế (State.Broken) - AC-3 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Finisher|Tags")
	FGameplayTag TagStateBroken;

	/** Thẻ choáng vỡ thế (State.Staggered) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Finisher|Tags")
	FGameplayTag TagStateStaggered;

	/** Mục tiêu đang hướng tới để thực hiện kết liễu */
	UPROPERTY(BlueprintReadWrite, Category = "ProjectAscendant|Combat|Finisher|State")
	TWeakObjectPtr<AActor> ExecutionTarget = nullptr;

	/** Áp dụng hiệu ứng bất tử 1.5s cho bản thân người tung chiêu */
	void ApplyAttackerInvulnerabilityEffect();

	/** Áp dụng hiệu ứng choáng 1.5s cho mục tiêu bị kết liễu */
	void ApplyTargetStunEffect(AActor* TargetActor);

	/** Thực thi kết liễu và sát thương lên mục tiêu đã được xác thực (Server Authority) */
	void ExecuteFinisherOnTarget(AActor* ValidatedTarget);

	/** Callback khi nhận TargetData từ Client */
	void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ApplicationTag);

	UFUNCTION()
	void OnFinisherSequenceCompleted();

private:
	FTimerHandle FinisherTimerHandle;
	FDelegateHandle TargetDataDelegateHandle;
	FActiveGameplayEffectHandle ActiveInvulnerabilityHandle;
	FActiveGameplayEffectHandle ActiveTargetStunHandle;
	bool bFinisherExecuted = false;
};
