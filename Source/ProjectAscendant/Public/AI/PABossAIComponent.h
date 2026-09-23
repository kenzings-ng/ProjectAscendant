// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/PABossAITypes.h"
#include "PABossAIComponent.generated.h"

/**
 * UPABossAIComponent
 *
 * ActorComponent quản lý Trí tuệ nhân tạo và Vòng đời đòn đánh 4 pha cho Boss Ironclad Warlord.
 *
 * Tham chiếu GDD: design/gdd/boss-ai.md
 * ADR-0001: Server-authoritative combat state
 * ADR-0002: PaperZD & GAS integration
 *
 * Tính năng:
 * - 4 Pha đòn đánh chuẩn mực: Telegraph (decal lấp đầy), Flash Cue (Ping! 0.10s), Hitbox, Recovery.
 * - Đánh giá hành động theo cự ly và góc quay (EQS scoring).
 * - 3 Giai đoạn máu (Normal -> Combos -> Enraged 480cm/s).
 * - Trạng thái choáng húc tường 1.8s (Wall Crash Stun).
 * - Đồng bộ cờ gãy sừng và gãy đuôi để khóa vĩnh viễn chiêu tương ứng.
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPABossAIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPABossAIComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===========================================================
	// Public API
	// ===========================================================

	UFUNCTION(BlueprintCallable, Category = "BossAI")
	void InitializeBoss(float InMaxHP = 10000.0f);

	UFUNCTION(BlueprintCallable, Category = "BossAI")
	bool StartAttack(EPABossAttackType AttackType);

	UFUNCTION(BlueprintCallable, Category = "BossAI")
	void TriggerWallCrash();

	UFUNCTION(BlueprintCallable, Category = "BossAI")
	void SetBossHealth(float NewHP);

	UFUNCTION(BlueprintCallable, Category = "BossAI")
	void ApplyBossDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category = "BossAI")
	void BreakHorn();

	UFUNCTION(BlueprintCallable, Category = "BossAI")
	void BreakTail();

	// ===========================================================
	// Blueprint Getters
	// ===========================================================

	UFUNCTION(BlueprintPure, Category = "BossAI")
	EPABossAttackPhase GetCurrentPhase() const { return Model.CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "BossAI")
	EPABossAttackType GetCurrentAttack() const { return Model.CurrentAttack; }

	UFUNCTION(BlueprintPure, Category = "BossAI")
	EPABossCombatPhase GetCombatPhase() const { return Model.CombatPhase; }

	UFUNCTION(BlueprintPure, Category = "BossAI")
	float GetTelegraphFillRatio() const { return Model.TelegraphFillRatio; }

	UFUNCTION(BlueprintPure, Category = "BossAI")
	bool IsFlashCueActive() const { return Model.bFlashCueActive; }

	UFUNCTION(BlueprintPure, Category = "BossAI")
	bool IsHitboxActive() const { return Model.bHitboxActive; }

	UFUNCTION(BlueprintPure, Category = "BossAI")
	bool IsWallStunned() const { return Model.bIsWallStunned; }

	UFUNCTION(BlueprintPure, Category = "BossAI")
	float GetCurrentMoveSpeed() const { return Model.CurrentMoveSpeed; }

	UFUNCTION(BlueprintPure, Category = "BossAI")
	bool CanUseHornCharge() const { return Model.bCanUseHornCharge; }

	UFUNCTION(BlueprintPure, Category = "BossAI")
	bool CanUseTailSweep() const { return Model.bCanUseTailSweep; }

	UFUNCTION(BlueprintPure, Category = "BossAI")
	const FPABossAIModel& GetModel() const { return Model; }

	// ===========================================================
	// Target Tracking (được gán từ Controller / Perception)
	// ===========================================================

	UFUNCTION(BlueprintCallable, Category = "BossAI")
	void SetTargetInfo(float Distance, float AngleDegrees);

	// ===========================================================
	// Delegates cho Blueprint Event Binding
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossAttackPhaseChanged, EPABossAttackPhase, NewPhase, EPABossAttackType, Attack);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossFlashCue, EPABossAttackType, Attack);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossCombatPhaseChanged, EPABossCombatPhase, NewCombatPhase);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossWallStunned);

	UPROPERTY(BlueprintAssignable, Category = "BossAI|Events")
	FOnBossAttackPhaseChanged OnAttackPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "BossAI|Events")
	FOnBossFlashCue OnFlashCueTriggered;

	UPROPERTY(BlueprintAssignable, Category = "BossAI|Events")
	FOnBossCombatPhaseChanged OnCombatPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "BossAI|Events")
	FOnBossWallStunned OnWallStunTriggered;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BossAI")
	FPABossAIModel Model;

	float TargetDistance = 300.0f;
	float TargetAngleDegrees = 0.0f;

	EPABossAttackPhase PreviousPhase = EPABossAttackPhase::Idle;
	EPABossCombatPhase PreviousCombatPhase = EPABossCombatPhase::Phase1_Normal;
	bool bPreviousFlashCue = false;
};
