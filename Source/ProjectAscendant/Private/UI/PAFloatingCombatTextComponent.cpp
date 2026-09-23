// Copyright Project Ascendant. All Rights Reserved.

#include "UI/PAFloatingCombatTextComponent.h"

// ===========================================================
// Constructor
// ===========================================================

UPAFloatingCombatTextComponent::UPAFloatingCombatTextComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

// ===========================================================
// Lifecycle
// ===========================================================

void UPAFloatingCombatTextComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPAFloatingCombatTextComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Pool.Update(DeltaTime);
}

// ===========================================================
// Spawn API
// ===========================================================

void UPAFloatingCombatTextComponent::SpawnCombatText(const FVector& WorldLocation, float Amount, EPACombatTextType Type)
{
	const int32 Index = Pool.SpawnCombatText(WorldLocation, Amount, Type);
	if (Index != INDEX_NONE)
	{
		OnCombatTextSpawned.Broadcast(Index, Type);
	}
}

void UPAFloatingCombatTextComponent::SpawnPerfectDodgeCallout(const FVector& CharacterLocation)
{
	const int32 Index = Pool.SpawnPerfectDodgeCallout(CharacterLocation);
	if (Index != INDEX_NONE)
	{
		OnCombatTextSpawned.Broadcast(Index, EPACombatTextType::PerfectDodgeCallout);
	}
}
