// Copyright Project Ascendant. All Rights Reserved.

#include "Network/PADifficultyScalingComponent.h"

UPADifficultyScalingComponent::UPADifficultyScalingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPADifficultyScalingComponent::InitializeScaling(float InBaseHP, float InBasePosture)
{
	Model.Initialize(InBaseHP, InBasePosture);
	PreviousCombatantCount = 0;
	PreviousCCReduction = 0.0f;
}

void UPADifficultyScalingComponent::RegisterCombatant(const FString& PlayerId)
{
	Model.RegisterCombatant(PlayerId);

	const int32 NewCount = Model.GetCombatantCount();
	if (NewCount != PreviousCombatantCount)
	{
		OnScalingUpdated.Broadcast(NewCount, Model.ComputeScaledMaxHP(), Model.ComputeScaledMaxPosture());

		const float NewCC = Model.ComputeCCReduction();
		if (NewCC != PreviousCCReduction && NewCC > 0.0f)
		{
			OnAntiZergActivated.Broadcast(NewCC);
		}

		PreviousCombatantCount = NewCount;
		PreviousCCReduction = NewCC;
	}
}

void UPADifficultyScalingComponent::RecordCombatantDamage(const FString& PlayerId, float HPDamage, float PostureDamage)
{
	Model.RecordDamage(PlayerId, HPDamage, PostureDamage);
}
