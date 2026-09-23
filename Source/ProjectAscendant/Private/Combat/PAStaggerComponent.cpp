// Copyright Project Ascendant. All Rights Reserved.

#include "Combat/PAStaggerComponent.h"

UPAStaggerComponent::UPAStaggerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UPAStaggerComponent::BeginPlay()
{
	Super::BeginPlay();
	Model.Initialize();
	PreviousState = Model.State;
}

void UPAStaggerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const FPAExecutionResult Result = Model.Update(DeltaTime);

	// Kiểm tra nếu có sóng xung kích do bỏ lỡ cửa sổ kết liễu
	if (Result.bShockwaveTriggered)
	{
		OnMissedRecoveryShockwave.Broadcast();
	}

	// Phát sự kiện thay đổi trạng thái
	if (Model.State != PreviousState)
	{
		OnStaggerStateChanged.Broadcast(Model.State);

		if (Model.State == EPAStaggerState::StaggeredKneel)
		{
			OnPostureBroken.Broadcast();
		}

		PreviousState = Model.State;
	}
}

void UPAStaggerComponent::InitializeStagger(float InMaxPosture)
{
	Model.Initialize(InMaxPosture);
	PreviousState = Model.State;
}

bool UPAStaggerComponent::ApplyPostureDamage(float Amount)
{
	const bool bBroken = Model.ApplyPostureDamage(Amount);
	if (bBroken)
	{
		OnPostureBroken.Broadcast();
		PreviousState = Model.State;
	}
	return bBroken;
}

void UPAStaggerComponent::NotifyDamageReceived()
{
	Model.OnDamageReceived();
}

FPAExecutionResult UPAStaggerComponent::RequestExecution(float PlayerDistance, float TargetMaxHP)
{
	const FPAExecutionResult Result = Model.TryExecute(PlayerDistance, TargetMaxHP);

	if (Result.bSuccess)
	{
		OnExecutionCompleted.Broadcast(Result.DamageDealt, Result.PlayerInvulnDuration);
		PreviousState = Model.State;
	}

	return Result;
}
