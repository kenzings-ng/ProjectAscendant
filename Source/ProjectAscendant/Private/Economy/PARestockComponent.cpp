// Copyright Project Ascendant. All Rights Reserved.

#include "Economy/PARestockComponent.h"
#include "Economy/PAMerchantComponent.h"
#include "Economy/PAMerchantTypes.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UPARestockComponent::UPARestockComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPARestockComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPARestockComponent, NextRestockServerTime);
}

void UPARestockComponent::BeginPlay()
{
	Super::BeginPlay();
	// Timer is started explicitly by APAWanderingSmuggler::BeginPlay()
	// after the MerchantComponent catalog is fully populated.
}

void UPARestockComponent::StartRestockTimer(UPAMerchantComponent* MerchantComp)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!MerchantComp)
	{
		return;
	}

	CachedMerchantComp = MerchantComp;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Calculate the next restock time aligned to global server boundary.
	// All Tier 2 smugglers restock simultaneously (GDD: "Restock đồng bộ toàn server").
	NextRestockServerTime = CalculateAlignedNextRestockTime();

	const float DelayUntilFirstRestock = NextRestockServerTime - World->GetTimeSeconds();
	const float SafeDelay = FMath::Max(DelayUntilFirstRestock, 1.0f);

	World->GetTimerManager().SetTimer(
		RestockTimerHandle,
		this,
		&UPARestockComponent::OnRestockTimerFired,
		RestockIntervalSeconds,
		/*bLoop=*/true,
		/*FirstDelay=*/SafeDelay
	);
}

void UPARestockComponent::ExecuteRestock(UPAMerchantComponent* MerchantComp)
{
	if (!MerchantComp)
	{
		return;
	}

	// Only rotating slots (indices >= FixedCatalogCount) are refreshed.
	// We rely on the catalog having been pre-populated up to FixedCatalogCount + RotatingSlotCount.
	// ExecuteRestock updates AvailableStock on each rotating entry in-place.
	// The caller (APAWanderingSmuggler) is responsible for maintaining the correct pool mapping.

	// Notify owning actor so it can re-roll rotating entries from its pool.
	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Update next restock time for client countdown.
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		NextRestockServerTime = CurrentTime + RestockIntervalSeconds;
	}

	OnRestocked.Broadcast(CurrentTime);
}

float UPARestockComponent::GetSecondsUntilNextRestock() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	const float Remaining = NextRestockServerTime - World->GetTimeSeconds();
	return FMath::Max(Remaining, 0.0f);
}

float UPARestockComponent::CalculateAlignedNextRestockTime() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return RestockIntervalSeconds;
	}

	// Align to global server boundaries: floor(CurrentTime / Interval) * Interval + Interval
	// This ensures all same-Tier merchants restock simultaneously regardless of spawn time.
	const float CurrentTime = World->GetTimeSeconds();
	const float Boundary = FMath::Floor(CurrentTime / RestockIntervalSeconds) * RestockIntervalSeconds;
	return Boundary + RestockIntervalSeconds;
}

void UPARestockComponent::OnRestockTimerFired()
{
	UPAMerchantComponent* MerchantComp = CachedMerchantComp.Get();
	if (!MerchantComp)
	{
		return;
	}

	ExecuteRestock(MerchantComp);
}
