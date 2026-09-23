// Copyright Project Ascendant. All Rights Reserved.

#include "World/PACampfireComponent.h"
#include "GameFramework/Actor.h"

UPACampfireComponent::UPACampfireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SanctuaryRadius = 1000.0f;
}

void UPACampfireComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		CampfireData.WorldLocation = GetOwner()->GetActorLocation();
	}
}

void UPACampfireComponent::InitializeCampfire(FName InId, EPAZoneTier InTier, bool bInitiallyLit)
{
	CampfireData.CampfireId = InId;
	CampfireData.ZoneTier = InTier;
	CampfireData.bIsLit = bInitiallyLit;

	if (GetOwner())
	{
		CampfireData.WorldLocation = GetOwner()->GetActorLocation();
	}
}

bool UPACampfireComponent::LightCampfire()
{
	if (CampfireData.bIsLit)
	{
		return false;
	}

	CampfireData.bIsLit = true;
	OnCampfireLit.Broadcast(CampfireData.CampfireId);
	return true;
}

bool UPACampfireComponent::RestAtCampfire(float& OutHP, float& OutMP, float& OutStamina, int32& OutFlasks)
{
	if (!CampfireData.bIsLit)
	{
		return false;
	}

	OutHP = 1.0f;
	OutMP = 1.0f;
	OutStamina = 1.0f;
	OutFlasks = 5;

	OnCampfireRestCompleted.Broadcast();
	return true;
}

bool UPACampfireComponent::IsInsideSanctuary(const FVector& Pos) const
{
	if (!CampfireData.bIsLit)
	{
		return false;
	}

	const float DistSq = FVector::DistSquared2D(Pos, CampfireData.WorldLocation);
	return DistSq <= FMath::Square(SanctuaryRadius);
}
