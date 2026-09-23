// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Economy/PAKarmaTypes.h"
#include "PAKarmaComponent.generated.h"

/**
 * UPAKarmaComponent
 *
 * ActorComponent quản lý điểm Danh dự Karma, trạng thái Gây hấn Aggressor,
 * phân giải hình phạt tử vong và chấp hành án phạt Nhà tù Lao Động.
 *
 * Tham chiếu GDD: design/gdd/zone-system.md §Karma System & Death Penalty
 * ADR-0001: Server-authoritative logic
 */
UCLASS(ClassGroup = (Economy), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAKarmaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAKarmaComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===========================================================
	// Public API
	// ===========================================================

	UFUNCTION(BlueprintCallable, Category = "Karma")
	void InitializeKarma(float InInitialKarma = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Karma")
	void AddKarma(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Karma")
	void TriggerAggressor();

	UFUNCTION(BlueprintCallable, Category = "Karma")
	FPADeathPenaltyResult HandleDeath(EPADeathScenario Scenario, float CurrentGold, float CurrentShards, float RandomRoll = 0.5f);

	UFUNCTION(BlueprintCallable, Category = "Karma")
	bool MineOreInPrison(int32 Amount = 1);

	// ===========================================================
	// Getters
	// ===========================================================

	UFUNCTION(BlueprintPure, Category = "Karma")
	EPAKarmaTier GetKarmaTier() const { return Model.GetKarmaTier(); }

	UFUNCTION(BlueprintPure, Category = "Karma")
	float GetCurrentKarma() const { return Model.CurrentKarma; }

	UFUNCTION(BlueprintPure, Category = "Karma")
	bool IsAggressor() const { return Model.AggressorTimeRemaining > 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Karma")
	bool IsInPrison() const { return Model.bInLaborPrison; }

	UFUNCTION(BlueprintPure, Category = "Karma")
	int32 GetMinedOresCount() const { return Model.MinedOresCount; }

	UFUNCTION(BlueprintPure, Category = "Karma")
	const FPAKarmaModel& GetModel() const { return Model; }

	// ===========================================================
	// Delegates
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKarmaTierChanged, EPAKarmaTier, NewTier);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSentToLaborPrison, float, Duration, int32, OresRequired);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReleasedFromPrison);

	UPROPERTY(BlueprintAssignable, Category = "Karma|Events")
	FOnKarmaTierChanged OnKarmaTierChanged;

	UPROPERTY(BlueprintAssignable, Category = "Karma|Events")
	FOnSentToLaborPrison OnSentToPrison;

	UPROPERTY(BlueprintAssignable, Category = "Karma|Events")
	FOnReleasedFromPrison OnReleasedFromPrison;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Karma")
	FPAKarmaModel Model;

	EPAKarmaTier PreviousTier = EPAKarmaTier::Neutral;
	bool bPreviousInPrison = false;
};
