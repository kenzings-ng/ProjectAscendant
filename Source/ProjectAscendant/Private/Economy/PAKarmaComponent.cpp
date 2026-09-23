// Copyright Project Ascendant. All Rights Reserved.

#include "Economy/PAKarmaComponent.h"

UPAKarmaComponent::UPAKarmaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UPAKarmaComponent::BeginPlay()
{
	Super::BeginPlay();
	Model.Initialize();
	PreviousTier = Model.GetKarmaTier();
	bPreviousInPrison = Model.bInLaborPrison;
}

void UPAKarmaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Model.Update(DeltaTime);

	const EPAKarmaTier CurrentTier = Model.GetKarmaTier();
	if (CurrentTier != PreviousTier)
	{
		OnKarmaTierChanged.Broadcast(CurrentTier);
		PreviousTier = CurrentTier;
	}

	if (!Model.bInLaborPrison && bPreviousInPrison)
	{
		OnReleasedFromPrison.Broadcast();
	}
	bPreviousInPrison = Model.bInLaborPrison;
}

void UPAKarmaComponent::InitializeKarma(float InInitialKarma)
{
	Model.Initialize(InInitialKarma);
	PreviousTier = Model.GetKarmaTier();
	bPreviousInPrison = Model.bInLaborPrison;
}

void UPAKarmaComponent::AddKarma(float Delta)
{
	Model.AddKarma(Delta);

	const EPAKarmaTier CurrentTier = Model.GetKarmaTier();
	if (CurrentTier != PreviousTier)
	{
		OnKarmaTierChanged.Broadcast(CurrentTier);
		PreviousTier = CurrentTier;
	}
}

void UPAKarmaComponent::TriggerAggressor()
{
	Model.TriggerAggressor();

	const EPAKarmaTier CurrentTier = Model.GetKarmaTier();
	if (CurrentTier != PreviousTier)
	{
		OnKarmaTierChanged.Broadcast(CurrentTier);
		PreviousTier = CurrentTier;
	}
}

FPADeathPenaltyResult UPAKarmaComponent::HandleDeath(EPADeathScenario Scenario, float CurrentGold, float CurrentShards, float RandomRoll)
{
	const FPADeathPenaltyResult Result = Model.ResolveDeath(Scenario, CurrentGold, CurrentShards, RandomRoll);

	if (Result.bSentToPrison)
	{
		OnSentToPrison.Broadcast(Result.PrisonDurationSeconds, Result.OreRequiredForRelease);
		bPreviousInPrison = true;
	}

	return Result;
}

bool UPAKarmaComponent::MineOreInPrison(int32 Amount)
{
	const bool bReleased = Model.MineOreInPrison(Amount);
	if (bReleased)
	{
		OnReleasedFromPrison.Broadcast();
		bPreviousInPrison = false;
	}
	return bReleased;
}
