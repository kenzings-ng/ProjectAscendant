// Copyright Project Ascendant. All Rights Reserved.

#include "Inventory/GE_Item_IronSword.h"
#include "Combat/AscendantAttributeSet.h"

UGE_Item_IronSword::UGE_Item_IronSword()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	FGameplayModifierInfo ModInfo;
	ModInfo.Attribute = UAscendantAttributeSet::GetAttackPowerAttribute();
	ModInfo.ModifierOp = EGameplayModOp::Additive;
	ModInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(15.0f));
	Modifiers.Add(ModInfo);
}
