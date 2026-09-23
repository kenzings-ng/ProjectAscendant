// Copyright Project Ascendant. All Rights Reserved.

#include "World/PACitadelComponent.h"
#include "GameFramework/Actor.h"

UPACitadelComponent::UPACitadelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CitadelData.SafeZoneRadius = 5000.0f;
}

void UPACitadelComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		CitadelData.WorldLocation = Owner->GetActorLocation();
		InternalZoneModel.RegisterCitadel(
			CitadelData.CitadelId,
			CitadelData.CitadelDisplayName,
			CitadelData.ZoneTier,
			CitadelData.WorldLocation,
			CitadelData.SafeZoneRadius
		);
	}
}

void UPACitadelComponent::InitializeCitadel(FName InId, const FString& InDisplayName, EPAZoneTier InTier, float InSafeRadius)
{
	CitadelData.CitadelId = InId;
	CitadelData.CitadelDisplayName = InDisplayName;
	CitadelData.ZoneTier = InTier;
	CitadelData.SafeZoneRadius = InSafeRadius;

	if (AActor* Owner = GetOwner())
	{
		CitadelData.WorldLocation = Owner->GetActorLocation();
	}

	InternalZoneModel.Citadels.Empty();
	InternalZoneModel.RegisterCitadel(InId, InDisplayName, InTier, CitadelData.WorldLocation, InSafeRadius);
}

bool UPACitadelComponent::OnPlayerEnterCitadel(
	const FString& PlayerId,
	const FVector& PlayerLocation,
	FPAAutoSaveRecord& OutSaveRecord,
	float& OutHP,
	float& OutMP,
	float& OutStamina,
	int32& OutFlasks)
{
	const float CurrentSimTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	const bool bEntered = InternalZoneModel.EnterCitadel(
		PlayerId,
		PlayerLocation,
		CurrentSimTime,
		OutSaveRecord,
		OutHP,
		OutMP,
		OutStamina,
		OutFlasks
	);

	if (bEntered)
	{
		CitadelData.bIsDiscovered = true;
		CitadelData.bIsLastVisited = true;
		OnCitadelEnteredAndSaved.Broadcast(CitadelData.CitadelId, PlayerId, OutSaveRecord);
	}

	return bEntered;
}

bool UPACitadelComponent::IsInsideSafeZone(const FVector& Pos) const
{
	FName OutId = NAME_None;
	return InternalZoneModel.IsInsideSafeZone(Pos, OutId);
}
