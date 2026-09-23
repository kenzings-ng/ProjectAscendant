// Copyright Project Ascendant. All Rights Reserved.

#include "Inventory/GE_Item_IronPlate.h"
#include "Combat/AscendantAttributeSet.h"

UGE_Item_IronPlate::UGE_Item_IronPlate()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	FGameplayModifierInfo ModInfo;
	ModInfo.Attribute = UAscendantAttributeSet::GetArmorAttribute();
	ModInfo.ModifierOp = EGameplayModOp::Additive;
	ModInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(40.0f));
	Modifiers.Add(ModInfo);
}
