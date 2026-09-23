// Copyright Project Ascendant. All Rights Reserved.

#include "Progression/PAProgressionComponent.h"
#include "Combat/AscendantAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UPAProgressionComponent::UPAProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPAProgressionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPAProgressionComponent, CurrentLevel);
	DOREPLIFETIME(UPAProgressionComponent, CurrentXP);
	DOREPLIFETIME(UPAProgressionComponent, AvailableSkillPoints);
}

// -------------------------------------------------------------------------
// Public API
// -------------------------------------------------------------------------

EPAProgressionError UPAProgressionComponent::GrantXP(int32 XPAmount)
{
	// Server Authority check
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return EPAProgressionError::NotAuthoritative;
	}

	if (XPAmount <= 0)
	{
		return EPAProgressionError::InvalidXPAmount;
	}

	if (IsMaxLevel())
	{
		return EPAProgressionError::AlreadyMaxLevel;
	}

	CurrentXP += XPAmount;
	OnXPGained.Broadcast(XPAmount, CurrentXP);

	// Multi-level-up loop: liên tục lên cấp nếu XP đủ
	int32 XPRequired = FPAProgressionFormulas::GetXPRequiredForLevel(CurrentLevel);
	while (XPRequired > 0 && CurrentXP >= XPRequired && CurrentLevel < FPAProgressionFormulas::MaxLevel)
	{
		// Trừ XP cho level này
		CurrentXP -= XPRequired;

		// Lên cấp
		CurrentLevel++;

		// Tính và áp dụng phần thưởng
		FPALevelUpReward Reward = FPAProgressionFormulas::GetLevelUpReward();
		AvailableSkillPoints += Reward.SkillPointsAwarded;

		// Áp dụng stat bonus lên AttributeSet
		ApplyLevelUpStatBonus(Reward);

		// Broadcast
		OnLevelUp.Broadcast(CurrentLevel, Reward);

		// Tính XP cần cho level tiếp theo
		XPRequired = FPAProgressionFormulas::GetXPRequiredForLevel(CurrentLevel);
	}

	// Nếu đã max level, xóa XP dư
	if (IsMaxLevel())
	{
		CurrentXP = 0;
	}

	return EPAProgressionError::None;
}

EPAProgressionError UPAProgressionComponent::SpendSkillPoint()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return EPAProgressionError::NotAuthoritative;
	}

	if (AvailableSkillPoints <= 0)
	{
		return EPAProgressionError::InsufficientSkillPoints;
	}

	AvailableSkillPoints--;
	return EPAProgressionError::None;
}

void UPAProgressionComponent::RefundSkillPoints(int32 Amount)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	if (Amount > 0)
	{
		AvailableSkillPoints += Amount;
	}
}

int32 UPAProgressionComponent::GetXPToNextLevel() const
{
	return FPAProgressionFormulas::GetXPRequiredForLevel(CurrentLevel);
}

// -------------------------------------------------------------------------
// Server RPC
// -------------------------------------------------------------------------

bool UPAProgressionComponent::Server_SpendSkillPoint_Validate()
{
	return true;
}

void UPAProgressionComponent::Server_SpendSkillPoint_Implementation()
{
	SpendSkillPoint();
}

// -------------------------------------------------------------------------
// Rep Notifies
// -------------------------------------------------------------------------

void UPAProgressionComponent::OnRep_CurrentLevel()
{
	// Client-side visual/audio reaction khi nhận level mới từ server
	// (VFX, SFX cấp lên sẽ được trigger ở đây trong tương lai)
}

void UPAProgressionComponent::OnRep_CurrentXP()
{
	// Client-side XP bar update
}

// -------------------------------------------------------------------------
// Internal
// -------------------------------------------------------------------------

void UPAProgressionComponent::ApplyLevelUpStatBonus(const FPALevelUpReward& Reward)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Tìm AbilitySystemComponent trên Owner
	UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC)
	{
		return;
	}

	// Tìm AttributeSet
	const UAscendantAttributeSet* AttrSetConst = ASC->GetSet<UAscendantAttributeSet>();
	if (!AttrSetConst)
	{
		return;
	}

	// Non-const cast để modify (chúng ta đang trên server với authority)
	UAscendantAttributeSet* AttrSet = const_cast<UAscendantAttributeSet*>(AttrSetConst);

	// Áp dụng bonus — tăng cả Base và Current cho Max attributes
	AttrSet->SetMaxHealth(AttrSet->GetMaxHealth() + Reward.BonusMaxHealth);
	AttrSet->SetHealth(AttrSet->GetHealth() + Reward.BonusMaxHealth); // Heal khi lên cấp

	AttrSet->SetMaxMana(AttrSet->GetMaxMana() + Reward.BonusMaxMana);
	AttrSet->SetMana(AttrSet->GetMana() + Reward.BonusMaxMana); // Restore mana khi lên cấp

	AttrSet->SetMaxStamina(AttrSet->GetMaxStamina() + Reward.BonusMaxStamina);

	AttrSet->SetAttackPower(AttrSet->GetAttackPower() + Reward.BonusAttackPower);
	AttrSet->SetArmor(AttrSet->GetArmor() + Reward.BonusArmor);
}
