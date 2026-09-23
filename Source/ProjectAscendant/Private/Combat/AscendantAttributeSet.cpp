// Copyright Project Ascendant. All Rights Reserved.

#include "Combat/AscendantAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UAscendantAttributeSet::UAscendantAttributeSet()
{
	// Khởi tạo các giá trị cơ sở chuẩn hoá cho nhân vật Level 1 theo GDD attributes-system.md
	InitHealth(500.0f);
	InitMaxHealth(500.0f);

	InitMana(100.0f);
	InitMaxMana(100.0f);

	InitStamina(100.0f);
	InitMaxStamina(100.0f);
	InitStaminaRegenRate(45.0f); // 45 điểm/giây, đầy bình sau ~2.2s

	InitPosture(0.0f);
	InitMaxPosture(100.0f);
	InitPostureDecayRate(20.0f); // 20 điểm/giây sau 4.0s ngưng giao tranh

	InitIFrameDuration(0.28f);  // 0.28 giây bất tử khi lướt
	InitMoveSpeed(550.0f);       // 550 cm/s

	InitArmor(0.0f);             // 0 giáp cơ bản
	InitAttackPower(100.0f);     // 100 lực công cơ bản
	InitBaseStagger(10.0f);      // 10 lực phá thế cơ bản
	InitStaggerBonus(0.0f);      // 0% thưởng phá thế cơ bản
}

void UAscendantAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Ràng buộc giá trị chặn biên (Clamping - AC-2)
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
		if (NewValue > 0.0f)
		{
			bOutOfHealthBroadcasted = false;
		}
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetPostureAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxPosture());
		if (NewValue < GetMaxPosture())
		{
			bPostureBrokenBroadcasted = false;
		}
	}
	else if (Attribute == GetMaxPostureAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetIFrameDurationAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetArmorAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetAttackPowerAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetBaseStaggerAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetStaggerBonusAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UAscendantAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	AActor* InstigatorActor = nullptr;
	if (Data.EffectSpec.GetContext().GetInstigator())
	{
		InstigatorActor = Data.EffectSpec.GetContext().GetInstigator();
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
		EvaluateOutOfHealthBroadcast(InstigatorActor);
	}
	else if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetPostureAttribute())
	{
		SetPosture(FMath::Clamp(GetPosture(), 0.0f, GetMaxPosture()));
		EvaluatePostureBrokenBroadcast(InstigatorActor);
	}
}

void UAscendantAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Đồng bộ mạng Dedicated Server với Iris Replication Descriptor (AC-3)
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, StaminaRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, Posture, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, MaxPosture, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, PostureDecayRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, IFrameDuration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, BaseStagger, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAscendantAttributeSet, StaggerBonus, COND_None, REPNOTIFY_Always);
}

void UAscendantAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, Health, OldHealth);
}

void UAscendantAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, MaxHealth, OldMaxHealth);
}

void UAscendantAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, Mana, OldMana);
}

void UAscendantAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, MaxMana, OldMaxMana);
}

void UAscendantAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, Stamina, OldStamina);
}

void UAscendantAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, MaxStamina, OldMaxStamina);
}

void UAscendantAttributeSet::OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, StaminaRegenRate, OldStaminaRegenRate);
}

void UAscendantAttributeSet::OnRep_Posture(const FGameplayAttributeData& OldPosture)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, Posture, OldPosture);
}

void UAscendantAttributeSet::OnRep_MaxPosture(const FGameplayAttributeData& OldMaxPosture)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, MaxPosture, OldMaxPosture);
}

void UAscendantAttributeSet::OnRep_PostureDecayRate(const FGameplayAttributeData& OldPostureDecayRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, PostureDecayRate, OldPostureDecayRate);
}

void UAscendantAttributeSet::OnRep_IFrameDuration(const FGameplayAttributeData& OldIFrameDuration)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, IFrameDuration, OldIFrameDuration);
}

void UAscendantAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, MoveSpeed, OldMoveSpeed);
}

void UAscendantAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, Armor, OldArmor);
}

void UAscendantAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, AttackPower, OldAttackPower);
}

void UAscendantAttributeSet::OnRep_BaseStagger(const FGameplayAttributeData& OldBaseStagger)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, BaseStagger, OldBaseStagger);
}

void UAscendantAttributeSet::OnRep_StaggerBonus(const FGameplayAttributeData& OldStaggerBonus)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAscendantAttributeSet, StaggerBonus, OldStaggerBonus);
}

