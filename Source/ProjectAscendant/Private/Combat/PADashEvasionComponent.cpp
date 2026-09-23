// Copyright Project Ascendant. All Rights Reserved.

#include "Combat/PADashEvasionComponent.h"

UPADashEvasionComponent::UPADashEvasionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UPADashEvasionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const bool bCompleted = Model.Update(DeltaTime);
	if (bCompleted)
	{
		OnDashCompleted.Broadcast();
	}
}

bool UPADashEvasionComponent::StartDash(float CurrentStamina, float& OutStaminaRemaining)
{
	const bool bStarted = Model.StartDash(CurrentStamina, OutStaminaRemaining);
	if (bStarted)
	{
		OnDashStarted.Broadcast();
	}
	return bStarted;
}

bool UPADashEvasionComponent::TryCancelIntoAttack()
{
	const bool bCancelled = Model.TryCancelIntoAttack();
	if (bCancelled)
	{
		OnDashAttackCancelled.Broadcast();
		OnDashCompleted.Broadcast();
	}
	return bCancelled;
}

FPAIncomingHitResult UPADashEvasionComponent::ProcessIncomingHit(float RawDamage)
{
	const FPAIncomingHitResult Result = Model.ProcessIncomingHit(RawDamage);

	if (Result.bTriggeredPerfectDodge)
	{
		OnPerfectDodgeTriggered.Broadcast(Result.StaminaRefunded, Result.HitstopDuration);
	}

	return Result;
}
