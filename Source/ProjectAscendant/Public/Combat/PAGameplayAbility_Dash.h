// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "PAGameplayAbility_Dash.generated.h"

class APABaseCharacter;
class ACharacter;
class UCurveFloat;
class UAbilityTask_ApplyRootMotionConstantForce;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnDashExecuted, const FVector&, DashDirection, float, DashDistance);

/**
 * FPADashPipeline
 *
 * Bộ công thức toán học và logic trạng thái thuần túy (Pure Logic & Formulas) cho kỹ năng Dash.
 * Cho phép kiểm thử tự động tốc độ cao trong Unit Test không cần phụ thuộc vào Engine World.
 *
 * Tuân thủ:
 *  - GDD dash-evasion.md, combat-system.md & TR-cmbt-003
 *  - ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites
 *  - Story dash-001 Acceptance Criteria (AC-1, AC-2, AC-3, AC-4)
 */
struct PROJECTASCENDANT_API FPADashPipeline
{
	static constexpr float kDefaultStaminaCost = 25.0f;
	static constexpr float kDefaultDuration = 0.35f;
	static constexpr float kDefaultDistance = 450.0f;
	static constexpr float kIFrameStartTime = 0.05f;
	static constexpr float kIFrameDuration = 0.20f;
	static constexpr float kIFrameEndTime = 0.25f; // 0.05s + 0.20s
	static constexpr float kCooldownDuration = 0.50f;

	/**
	 * AC-1: Kiểm tra điều kiện kích hoạt Dash.
	 * Chặn hoàn toàn nếu Stamina < 25.0f hoặc mang thẻ State.Exhausted.
	 */
	static bool CanActivateDash(float CurrentStamina, bool bIsExhausted, float Cost = kDefaultStaminaCost);

	/**
	 * AC-2: Kiểm tra xem tại thời điểm ElapsedTime có nằm trong cửa sổ I-Frame bất tử (0.05s - 0.25s) hay không.
	 */
	static bool IsWithinIFrameWindow(float ElapsedTime, float StartTime = kIFrameStartTime, float EndTime = kIFrameEndTime);

	/**
	 * AC-3: Tính toán vận tốc đỉnh ban đầu theo quy luật giảm dần tuyến tính (Linear Decay).
	 * V_peak = (2 * Distance) / Duration = (2 * 450) / 0.35 ~= 2571.43 cm/s
	 */
	static float CalculatePeakVelocity(float Distance = kDefaultDistance, float Duration = kDefaultDuration);

	/**
	 * AC-3: Tính toán vận tốc tức thời tại thời điểm ElapsedTime theo hàm giảm tuyến tính:
	 * V(t) = V_peak * (1 - t / Duration)
	 */
	static float CalculateDecayingVelocity(float ElapsedTime, float Distance = kDefaultDistance, float Duration = kDefaultDuration);

	/**
	 * AC-3: Tính quãng đường di chuyển tích lũy từ t = 0 đến ElapsedTime.
	 * S(t) = V_peak * (t - t^2 / (2 * Duration))
	 */
	static float CalculateDisplacementAtTime(float ElapsedTime, float Distance = kDefaultDistance, float Duration = kDefaultDuration);

	/**
	 * AC-3: Xác định hướng lướt chuẩn hóa:
	 * Ưu tiên 1: MovementVector (khi đang di chuyển)
	 * Ưu tiên 2: AimDirection (khi đứng yên có ngắm)
	 * Ưu tiên 3: ForwardVector (hướng mặt nhân vật)
	 */
	static FVector ResolveDashDirection(const FVector& MovementVector, const FVector& AimDirection, const FVector& ForwardVector);
};

/**
 * UPAGameplayAbility_Dash
 *
 * Kỹ năng lướt né đòn (Dash Evasion) xây dựng trên Gameplay Ability System (GAS).
 * Tích hợp cửa sổ bất tử I-Frame 0.20s (State.Invulnerable) từ t = 0.05s đến 0.25s qua GameplayEffect,
 * lực đẩy 450 units trong 0.35s với linear decay qua Root Motion Constant Force Task,
 * tiêu hao 25 Stamina qua GameplayEffect và Cooldown 0.5s (Cooldown.Dash) khi kết thúc chiêu.
 */
UCLASS()
class PROJECTASCENDANT_API UPAGameplayAbility_Dash : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPAGameplayAbility_Dash();

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

	/** AC-4: Delegate phát khi kích hoạt cú lướt né đòn (cho phép VFX spawn ghost afterimages) */
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Combat|Dash")
	FPAOnDashExecuted OnDashExecuted;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Dash")
	float GetDashStaminaCost() const { return DashStaminaCost; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Dash")
	float GetDashDuration() const { return DashDuration; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Dash")
	float GetDashDistance() const { return DashDistance; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Dash")
	float GetCooldownDuration() const { return CooldownDuration; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Dash")
	bool IsInIFrameWindow() const { return bAppliedInvulnerability; }

	/** AC-2: Bắt đầu cửa sổ I-Frame (t = 0.05s) - Áp dụng GE_DashInvulnerability */
	UFUNCTION()
	void OnIFrameWindowOpened();

	/** AC-2: Đóng cửa sổ I-Frame (t = 0.25s) */
	UFUNCTION()
	void OnIFrameWindowClosed();

	/** AC-3: Hoàn thành cú lướt (t = 0.35s) */
	UFUNCTION()
	void OnDashCompleted();

protected:
	/** Chi phí thể lực của Dash (mặc định 25.0) - AC-1 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Dash|Config")
	float DashStaminaCost = 25.0f;

	/** Tổng thời lượng lướt (mặc định 0.35s) - AC-2/AC-3 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Dash|Config")
	float DashDuration = 0.35f;

	/** Quãng đường lướt (mặc định 450 units) - AC-3 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Dash|Config")
	float DashDistance = 450.0f;

	/** Thời điểm bắt đầu I-Frame (mặc định 0.05s) - AC-2 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Dash|Config")
	float IFrameStartTime = 0.05f;

	/** Thời lượng I-Frame (mặc định 0.20s -> kết thúc tại 0.25s) - AC-2 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Dash|Config")
	float IFrameDuration = 0.20f;

	/** Thời lượng hồi chiêu sau khi kết thúc lướt (mặc định 0.5s) - AC-3 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Dash|Config")
	float CooldownDuration = 0.50f;

	/** Thẻ kích hoạt lướt (Ability.Skill.Dash) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Dash|Tags")
	FGameplayTag TagAbilityDash;

	/** Thẻ trạng thái đang lướt (State.Dashing) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Dash|Tags")
	FGameplayTag TagStateDashing;

	/** Thẻ bất tử trong I-frame (State.Invulnerable) - AC-2 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Dash|Tags")
	FGameplayTag TagStateInvulnerable;

	/** Thẻ hồi chiêu (Cooldown.Dash) - AC-3 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Dash|Tags")
	FGameplayTag TagCooldownDash;

	/** Thẻ kiệt sức chặn lướt (State.Exhausted) - AC-1 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Dash|Tags")
	FGameplayTag TagStateExhausted;

	/** Áp dụng GameplayEffect tiêu hao thể lực (AC-1) */
	void ApplyCostEffect();

	/** Áp dụng GameplayEffect bất tử I-frame (AC-2) */
	void ApplyInvulnerabilityEffect();

	/** Áp dụng GameplayEffect hồi chiêu (AC-3) */
	void ApplyCooldownEffect();

private:
	FTimerHandle IFrameStartTimerHandle;
	FTimerHandle IFrameEndTimerHandle;
	FTimerHandle DashDurationTimerHandle;

	FActiveGameplayEffectHandle ActiveInvulnerabilityHandle;

	FVector ResolvedDashDirection = FVector::ForwardVector;
	bool bAppliedInvulnerability = false;

	void ClearActiveTimers();
};

/** Alias tương thích theo spec GDD */
using UGA_Dash = UPAGameplayAbility_Dash;
