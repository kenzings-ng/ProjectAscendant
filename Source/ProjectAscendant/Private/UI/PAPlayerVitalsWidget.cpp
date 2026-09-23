// Copyright Project Ascendant. All Rights Reserved.

#include "UI/PAPlayerVitalsWidget.h"
#include "Combat/AscendantAttributeSet.h"
#include "AbilitySystemComponent.h"

UPAPlayerVitalsWidget::UPAPlayerVitalsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPAPlayerVitalsWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPAPlayerVitalsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateModel(InDeltaTime);
}

void UPAPlayerVitalsWidget::BindToAttributeSet(UAscendantAttributeSet* InAttributeSet, UAbilitySystemComponent* InASC)
{
	BoundAttributeSet = InAttributeSet;
	BoundASC = InASC;

	if (InAttributeSet)
	{
		Model.SetHealth(InAttributeSet->GetHealth(), InAttributeSet->GetMaxHealth());
		Model.SetStamina(InAttributeSet->GetStamina(), InAttributeSet->GetMaxStamina());
		Model.SetMana(InAttributeSet->GetMana(), InAttributeSet->GetMaxMana());

		OnHealthPercentChanged.Broadcast(Model.GetHealthPercent());
		OnGhostHealthPercentChanged.Broadcast(Model.GetGhostHealthPercent());
		OnStaminaPercentChanged.Broadcast(Model.GetStaminaPercent());
		OnManaPercentChanged.Broadcast(Model.GetManaPercent());
	}

	if (InASC)
	{
		// AC-1: Event-driven bindings (không dùng tick polling)
		InASC->GetGameplayAttributeValueChangeDelegate(UAscendantAttributeSet::GetHealthAttribute())
			.AddUObject(this, &UPAPlayerVitalsWidget::OnHealthChanged);

		InASC->GetGameplayAttributeValueChangeDelegate(UAscendantAttributeSet::GetMaxHealthAttribute())
			.AddUObject(this, &UPAPlayerVitalsWidget::OnMaxHealthChanged);

		InASC->GetGameplayAttributeValueChangeDelegate(UAscendantAttributeSet::GetStaminaAttribute())
			.AddUObject(this, &UPAPlayerVitalsWidget::OnStaminaChanged);

		InASC->GetGameplayAttributeValueChangeDelegate(UAscendantAttributeSet::GetMaxStaminaAttribute())
			.AddUObject(this, &UPAPlayerVitalsWidget::OnMaxStaminaChanged);

		InASC->GetGameplayAttributeValueChangeDelegate(UAscendantAttributeSet::GetManaAttribute())
			.AddUObject(this, &UPAPlayerVitalsWidget::OnManaChanged);

		InASC->GetGameplayAttributeValueChangeDelegate(UAscendantAttributeSet::GetMaxManaAttribute())
			.AddUObject(this, &UPAPlayerVitalsWidget::OnMaxManaChanged);

		// Lắng nghe thẻ State.Exhausted
		const FGameplayTag ExhaustedTag = FGameplayTag::RequestGameplayTag(FName("State.Exhausted"), false);
		if (ExhaustedTag.IsValid())
		{
			InASC->RegisterGameplayTagEvent(ExhaustedTag, EGameplayTagEventType::NewOrRemoved)
				.AddUObject(this, &UPAPlayerVitalsWidget::OnExhaustionTagChanged);
		}
	}
}

void UPAPlayerVitalsWidget::SetManualVitals(float Health, float MaxHealth, float Stamina, float MaxStamina, float Mana, float MaxMana)
{
	Model.SetHealth(Health, MaxHealth);
	Model.SetStamina(Stamina, MaxStamina);
	Model.SetMana(Mana, MaxMana);

	OnHealthPercentChanged.Broadcast(Model.GetHealthPercent());
	OnGhostHealthPercentChanged.Broadcast(Model.GetGhostHealthPercent());
	OnStaminaPercentChanged.Broadcast(Model.GetStaminaPercent());
	OnManaPercentChanged.Broadcast(Model.GetManaPercent());
}

void UPAPlayerVitalsWidget::TriggerPerfectDodge()
{
	Model.TriggerPerfectDodge();
	OnStaminaStateChanged.Broadcast(Model.StaminaState);
	OnStaminaPercentChanged.Broadcast(Model.GetStaminaPercent());
}

void UPAPlayerVitalsWidget::SetExhausted(bool bExhausted)
{
	Model.SetExhausted(bExhausted);
	OnStaminaStateChanged.Broadcast(Model.StaminaState);
	OnExhaustionVignetteChanged.Broadcast(Model.bExhaustionVignetteActive);
}

void UPAPlayerVitalsWidget::UpdateModel(float DeltaTime)
{
	const float PrevGhost = Model.GhostHealthPercent;
	const EPAStaminaBarState PrevState = Model.StaminaState;
	const bool PrevLowHP = Model.bLowHealthVignetteActive;
	const float PrevBPM = Model.CurrentHeartbeatBPM;
	const bool PrevExhausted = Model.bExhaustionVignetteActive;

	Model.Update(DeltaTime);

	if (!FMath::IsNearlyEqual(PrevGhost, Model.GhostHealthPercent, 0.0001f))
	{
		OnGhostHealthPercentChanged.Broadcast(Model.GhostHealthPercent);
	}

	if (PrevState != Model.StaminaState)
	{
		OnStaminaStateChanged.Broadcast(Model.StaminaState);
	}

	if (PrevLowHP != Model.bLowHealthVignetteActive || !FMath::IsNearlyEqual(PrevBPM, Model.CurrentHeartbeatBPM, 0.01f))
	{
		OnLowHealthVignetteChanged.Broadcast(Model.bLowHealthVignetteActive, Model.CurrentHeartbeatBPM);
	}

	if (PrevExhausted != Model.bExhaustionVignetteActive)
	{
		OnExhaustionVignetteChanged.Broadcast(Model.bExhaustionVignetteActive);
	}
}

void UPAPlayerVitalsWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	Model.SetHealth(Data.NewValue, Model.MaxHealth);
	OnHealthPercentChanged.Broadcast(Model.GetHealthPercent());
}

void UPAPlayerVitalsWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	Model.SetHealth(Model.CurrentHealth, Data.NewValue);
	OnHealthPercentChanged.Broadcast(Model.GetHealthPercent());
}

void UPAPlayerVitalsWidget::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	Model.SetStamina(Data.NewValue, Model.MaxStamina);
	OnStaminaPercentChanged.Broadcast(Model.GetStaminaPercent());
}

void UPAPlayerVitalsWidget::OnMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	Model.SetStamina(Model.CurrentStamina, Data.NewValue);
	OnStaminaPercentChanged.Broadcast(Model.GetStaminaPercent());
}

void UPAPlayerVitalsWidget::OnManaChanged(const FOnAttributeChangeData& Data)
{
	Model.SetMana(Data.NewValue, Model.MaxMana);
	OnManaPercentChanged.Broadcast(Model.GetManaPercent());
}

void UPAPlayerVitalsWidget::OnMaxManaChanged(const FOnAttributeChangeData& Data)
{
	Model.SetMana(Model.CurrentMana, Data.NewValue);
	OnManaPercentChanged.Broadcast(Model.GetManaPercent());
}

void UPAPlayerVitalsWidget::OnExhaustionTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	SetExhausted(NewCount > 0);
}
