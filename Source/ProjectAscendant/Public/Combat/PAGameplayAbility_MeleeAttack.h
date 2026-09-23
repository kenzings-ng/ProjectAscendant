// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "PAGameplayAbility_MeleeAttack.generated.h"

class APABaseCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnComboAttackExecuted, int32, ComboStep, float, DamageMultiplier, float, PostureDamage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnSectorHitboxHit, AActor*, HitActor, float, DamageDealt, float, PostureDamageDealt);

/**
 * FPAComboFinisherPipeline
 *
 * Bộ công thức toán học và quy tắc kinh doanh thuần túy (Pure Logic & Formulas)
 * cho chuỗi 3 đòn liên hoàn (Combo Chain), Quét va chạm hình quạt (Sector Trace)
 * và Đòn kết liễu phá thế (Posture Finisher Execution).
 *
 * Tuân thủ:
 *  - GDD combat-system.md, attributes-system.md
 *  - ADR-0001: Open World MMO Combat Networking
 *  - ADR-0002: GAS Integration Strategy for PaperZD
 *  - Story cmbt-001 Acceptance Criteria (AC-1, AC-2, AC-3)
 */
struct PROJECTASCENDANT_API FPAComboStepConfig
{
	float DamageMultiplier = 1.0f;
	float PostureDamage = 10.0f;
};

struct PROJECTASCENDANT_API FPAComboFinisherPipeline
{
	static constexpr float kComboResetDelay = 1.2f;               // AC-1: Quá 1.2s tự động reset về Nhịp 1
	static constexpr float kSectorTraceRadius = 180.0f;           // AC-2: Bán kính quét 180cm
	static constexpr float kSectorTraceHalfAngleDeg = 45.0f;      // AC-2: Hình nón 90 độ (+/- 45 độ quanh hướng ngắm)
	static constexpr float kFinisherMaxDistance = 250.0f;         // AC-3: Cự ly kết liễu tối đa 250cm
	static constexpr float kFinisherDuration = 1.5f;              // AC-3: Thời lượng choáng và bất tử 1.5s
	static constexpr float kFinisherDamageRatio = 0.25f;          // AC-3: 25% Max HP sát thương chuẩn

	/**
	 * AC-1: Lấy cấu hình nhịp combo (Hệ số sát thương & Sát thương thế đứng)
	 */
	static FPAComboStepConfig GetComboStepConfig(int32 ComboStep);

	/**
	 * AC-1: Tính toán nhịp combo kế tiếp dựa trên mốc thời gian.
	 * Nếu ElapsedTime > 1.2s -> Reset về Step 1.
	 * Nếu trong 1.2s -> Tăng tiến: 1 -> 2 -> 3 -> 1.
	 */
	static int32 AdvanceComboStep(int32 CurrentStep, float TimeSinceLastAttack, float ResetDelay = kComboResetDelay);

	/**
	 * AC-1: Lấy hệ số sát thương của từng nhịp combo:
	 * Nhịp 1: 1.0x, Nhịp 2: 1.2x, Nhịp 3: 1.6x.
	 */
	static float GetComboDamageMultiplier(int32 ComboStep);

	/**
	 * AC-1: Lấy lượng sát thương thế đứng (Posture Damage) của từng nhịp combo:
	 * Nhịp 1: 10 điểm, Nhịp 2: 15 điểm, Nhịp 3: 25 điểm.
	 */
	static float GetComboPostureDamage(int32 ComboStep);

	/**
	 * AC-2: Thẩm định xem một vị trí mục tiêu có nằm trong hình nón quét (Sector Cone) hay không.
	 * Bán kính <= 180cm VÀ Góc lệch so với hướng mặt <= 45 độ (tổng nón 90 độ).
	 */
	static bool IsTargetInAttackSector(
		const FVector& AttackerPos,
		const FVector& AttackerFacingDirection,
		const FVector& TargetPos,
		float Radius = kSectorTraceRadius,
		float HalfAngleDeg = kSectorTraceHalfAngleDeg);

	/**
	 * AC-3: Kiểm tra điều kiện kích hoạt đòn kết liễu Finisher.
	 * Cự ly <= 250cm VÀ Mục tiêu mang thẻ State.Broken (hoặc State.Staggered).
	 */
	static bool CanExecuteFinisher(float DistanceToTarget, bool bTargetHasBrokenTag, float MaxDistance = kFinisherMaxDistance);

	/**
	 * AC-3: Tính sát thương chuẩn đòn kết liễu (25% Max HP của mục tiêu).
	 */
	static float CalculateFinisherDamage(float TargetMaxHP, float Ratio = kFinisherDamageRatio);
};

/**
 * UPAGameplayAbility_MeleeAttack
 *
 * GameplayAbility thực thi chuỗi 3 đòn đánh liên hoàn (3-Hit Combo) và quét va chạm hình quạt (Sector Trace).
 * - AC-1: Nhịp 1 (1.0x, 10 Posture), Nhịp 2 (1.2x, 15 Posture), Nhịp 3 (1.6x, 25 Posture). Delay > 1.2s reset về Nhịp 1.
 * - AC-2: Kích hoạt quét va chạm nón 90 độ bán kính 180cm trên Server Authority, thẩm định qua Lag Compensation.
 */
UCLASS()
class PROJECTASCENDANT_API UPAGameplayAbility_MeleeAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPAGameplayAbility_MeleeAttack();

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

	/** Delegate phát khi nhịp combo được thực thi (AC-1) */
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Combat|Combo")
	FPAOnComboAttackExecuted OnComboAttackExecuted;

	/** Delegate phát khi quét trúng mục tiêu trong hình nón (AC-2) */
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Combat|Combo")
	FPAOnSectorHitboxHit OnSectorHitboxHit;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Combo")
	int32 GetCurrentComboStep() const { return CurrentComboStep; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Combo")
	float GetLastAttackTimestamp() const { return LastAttackTimestamp; }

	/** Đặt lại combo về Nhịp 1 thủ công */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Combat|Combo")
	void ResetCombo();

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat|Combo")
	const FGameplayTagContainer& GetActivationBlockedTagContainer() const { return ActivationBlockedTags; }

	/**
	 * AC-2: Thực hiện quét va chạm hình nón (Sector Trace: R = 180cm, Góc = 90 độ)
	 * Có thể gọi từ PaperZD AnimNotify hoặc trực tiếp trên Server.
	 *
	 * @param ClientTimestamp Mốc thời gian phía Client gửi lên để Server tua lại lag compensation.
	 * @return Danh sách các Actor trúng đòn hợp lệ.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Combat|Combo")
	TArray<AActor*> PerformSectorHitboxTrace(float ClientTimestamp = -1.0f);

protected:
	/** Nhịp combo hiện tại (1, 2, 3) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Combo|State")
	int32 CurrentComboStep = 0;

	/** Mốc thời gian của đòn tấn công trước đó (giây) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Combo|State")
	float LastAttackTimestamp = -100.0f;

	/** Thời gian chờ tối đa giữa các đòn trước khi reset (mặc định 1.2s) - AC-1 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Combo|Config")
	float ComboResetDelay = 1.2f;

	/** Bán kính quét va chạm (mặc định 180cm) - AC-2 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Combo|Config")
	float SectorTraceRadius = 180.0f;

	/** Nửa góc mở hình nón quét (mặc định 45 độ -> nón 90 độ) - AC-2 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Combo|Config")
	float SectorTraceHalfAngleDeg = 45.0f;

	/** Sát thương cơ sở khi chưa nhân hệ số combo (mặc định 20.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Combo|Config")
	float BaseAttackDamage = 20.0f;

	/** Thẻ kích hoạt kỹ năng chém (Ability.Skill.Attack) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Combo|Tags")
	FGameplayTag TagAbilityAttack;

	/** Thẻ trạng thái đang tấn công (State.Attacking) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Combo|Tags")
	FGameplayTag TagStateAttacking;

	/** Thẻ báo vỡ thế đứng (State.Broken) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Combo|Tags")
	FGameplayTag TagStateBroken;

	/** Thẻ choáng vỡ thế đứng (State.Staggered) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Combo|Tags")
	FGameplayTag TagStateStaggered;

	/** Thời lượng của một đòn tấn công trước khi dứt chiêu (mặc định 0.4s) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Combat|Combo|Config")
	float AttackDuration = 0.4f;

	/** Handler lắng nghe sự kiện Event.Combat.Hitbox.Active */
	void OnHitboxEventReceived(FGameplayTag MatchingTag, const FGameplayEventData* Payload);

	/** Callback khi nhận TargetData từ Client (chứa Client Activation Timestamp) */
	void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ApplicationTag);

	/** Kết thúc hoạt ảnh đòn chém */
	UFUNCTION()
	void OnAttackCompleted();

private:
	FDelegateHandle HitboxEventDelegateHandle;
	FDelegateHandle TargetDataDelegateHandle;
	FTimerHandle AttackDurationTimerHandle;

	/** Nhãn thời gian kích hoạt đòn đánh phía client được đồng bộ lên server */
	float ReplicatedClientTimestamp = -1.0f;
};
