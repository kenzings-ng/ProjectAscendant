// Copyright Project Ascendant. All Rights Reserved.

#include "AI/PABossAIComponent.h"

UPABossAIComponent::UPABossAIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UPABossAIComponent::BeginPlay()
{
	Super::BeginPlay();
	Model.Initialize();
	PreviousPhase = Model.CurrentPhase;
	PreviousCombatPhase = Model.CombatPhase;
	bPreviousFlashCue = false;
}

void UPABossAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Model.Update(DeltaTime, TargetDistance, TargetAngleDegrees);

	// Phát sự kiện chuyển pha đòn đánh
	if (Model.CurrentPhase != PreviousPhase)
	{
		OnAttackPhaseChanged.Broadcast(Model.CurrentPhase, Model.CurrentAttack);

		if (Model.CurrentPhase == EPABossAttackPhase::WallStunned)
		{
			OnWallStunTriggered.Broadcast();
		}

		PreviousPhase = Model.CurrentPhase;
	}

	// Phát sự kiện Flash Cue (Ping!)
	if (Model.bFlashCueActive && !bPreviousFlashCue)
	{
		OnFlashCueTriggered.Broadcast(Model.CurrentAttack);
	}
	bPreviousFlashCue = Model.bFlashCueActive;

	// Phát sự kiện chuyển giai đoạn máu Boss
	if (Model.CombatPhase != PreviousCombatPhase)
	{
		OnCombatPhaseChanged.Broadcast(Model.CombatPhase);
		PreviousCombatPhase = Model.CombatPhase;
	}
}

void UPABossAIComponent::InitializeBoss(float InMaxHP)
{
	Model.Initialize(InMaxHP);
	PreviousPhase = Model.CurrentPhase;
	PreviousCombatPhase = Model.CombatPhase;
	bPreviousFlashCue = false;
}

bool UPABossAIComponent::StartAttack(EPABossAttackType AttackType)
{
	return Model.StartAttack(AttackType);
}

void UPABossAIComponent::TriggerWallCrash()
{
	Model.TriggerWallCrash();
}

void UPABossAIComponent::SetBossHealth(float NewHP)
{
	Model.SetHealth(NewHP);
}

void UPABossAIComponent::ApplyBossDamage(float Amount)
{
	Model.ApplyDamage(Amount);
}

void UPABossAIComponent::BreakHorn()
{
	Model.BreakHorn();
}

void UPABossAIComponent::BreakTail()
{
	Model.BreakTail();
}

void UPABossAIComponent::SetTargetInfo(float Distance, float AngleDegrees)
{
	TargetDistance = Distance;
	TargetAngleDegrees = AngleDegrees;
}
