// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Network/PADifficultyScalingTypes.h"
#include "PADifficultyScalingComponent.generated.h"

/**
 * UPADifficultyScalingComponent
 *
 * ActorComponent quản lý Co giãn độ khó động lực (DDS) cho Boss thế giới mở.
 * Tự động tính toán Scaled HP, Scaled Posture, Kháng khống chế Anti-Zerg,
 * và xét duyệt tư cách nhận Instanced Loot.
 *
 * Tham chiếu GDD: design/gdd/zone-system.md §Dynamic Difficulty Scaling
 * ADR-0001: Server-authoritative calculations
 */
UCLASS(ClassGroup = (Network), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPADifficultyScalingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPADifficultyScalingComponent();

	// ===========================================================
	// Public API
	// ===========================================================

	UFUNCTION(BlueprintCallable, Category = "DDS")
	void InitializeScaling(float InBaseHP = 10000.0f, float InBasePosture = 800.0f);

	UFUNCTION(BlueprintCallable, Category = "DDS")
	void RegisterCombatant(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "DDS")
	void RecordCombatantDamage(const FString& PlayerId, float HPDamage, float PostureDamage);

	// ===========================================================
	// Getters
	// ===========================================================

	UFUNCTION(BlueprintPure, Category = "DDS")
	float GetScaledMaxHP() const { return Model.ComputeScaledMaxHP(); }

	UFUNCTION(BlueprintPure, Category = "DDS")
	float GetScaledMaxPosture() const { return Model.ComputeScaledMaxPosture(); }

	UFUNCTION(BlueprintPure, Category = "DDS")
	float GetCCReduction() const { return Model.ComputeCCReduction(); }

	UFUNCTION(BlueprintPure, Category = "DDS")
	float GetTurnRateBonus() const { return Model.ComputeTurnRateBonus(); }

	UFUNCTION(BlueprintPure, Category = "DDS")
	int32 GetCombatantCount() const { return Model.GetCombatantCount(); }

	UFUNCTION(BlueprintPure, Category = "DDS")
	bool IsQualifiedForLoot(const FString& PlayerId) const { return Model.IsQualifiedForLoot(PlayerId); }

	UFUNCTION(BlueprintPure, Category = "DDS")
	TArray<FString> GetQualifiedPlayers() const { return Model.GetQualifiedPlayers(); }

	UFUNCTION(BlueprintPure, Category = "DDS")
	const FPADifficultyScalingModel& GetModel() const { return Model; }

	// ===========================================================
	// Delegates
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScalingUpdated, int32, CombatantCount, float, ScaledHP, float, ScaledPosture);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAntiZergActivated, float, CCReductionPct);

	UPROPERTY(BlueprintAssignable, Category = "DDS|Events")
	FOnScalingUpdated OnScalingUpdated;

	UPROPERTY(BlueprintAssignable, Category = "DDS|Events")
	FOnAntiZergActivated OnAntiZergActivated;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DDS")
	FPADifficultyScalingModel Model;

	int32 PreviousCombatantCount = 0;
	float PreviousCCReduction = 0.0f;
};
