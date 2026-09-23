// Copyright Project Ascendant. All Rights Reserved.

#include "Combat/PAPartBreakingComponent.h"

UPAPartBreakingComponent::UPAPartBreakingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPAPartBreakingComponent::BeginPlay()
{
	Super::BeginPlay();
	Model.Initialize();
}

void UPAPartBreakingComponent::InitializeParts(float InBossMaxHP)
{
	Model.Initialize(InBossMaxHP);
}

FPAPartBreakResult UPAPartBreakingComponent::ApplyPartDamage(EPABossPartType PartType, float RawDamage)
{
	const FPAPartBreakResult Result = Model.ApplyPartDamage(PartType, RawDamage);

	if (Result.bPartBroken)
	{
		OnPartBroken.Broadcast(Result.BrokenPart, Result.DropItemId);

		if (Result.bStunnedBoss)
		{
			OnBossStunnedByBreak.Broadcast(Result.StunDuration);
		}

		if (Result.bWeakpointRevealed)
		{
			OnChestWeakpointRevealed.Broadcast();
		}
	}

	return Result;
}
