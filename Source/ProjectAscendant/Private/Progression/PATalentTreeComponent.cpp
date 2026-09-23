// Copyright Project Ascendant. All Rights Reserved.

#include "Progression/PATalentTreeComponent.h"
#include "Progression/PAProgressionComponent.h"
#include "Combat/AscendantAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UPATalentTreeComponent::UPATalentTreeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPATalentTreeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPATalentTreeComponent, CharacterClass);
	DOREPLIFETIME(UPATalentTreeComponent, UnlockedNodeIds);
}

// -------------------------------------------------------------------------
// Public API — Server-Authoritative
// -------------------------------------------------------------------------

bool UPATalentTreeComponent::UnlockNode(FName NodeId, EPATalentTreeError& OutError)
{
	AActor* Owner = GetOwner();
	if (Owner && !Owner->HasAuthority())
	{
		OutError = EPATalentTreeError::NotAuthoritative;
		OnTalentFailed.Broadcast(OutError);
		return false;
	}

	if (IsNodeUnlocked(NodeId))
	{
		OutError = EPATalentTreeError::NodeAlreadyUnlocked;
		OnTalentFailed.Broadcast(OutError);
		return false;
	}

	const FPATalentNode* Node = FPATalentTreeDatabase::FindNode(CharacterClass, NodeId);
	if (!Node)
	{
		OutError = EPATalentTreeError::InvalidNodeId;
		OnTalentFailed.Broadcast(OutError);
		return false;
	}

	// Kiểm tra điều kiện tiên quyết (Prerequisite)
	if (Node->PrerequisiteNodeId != NAME_None && !IsNodeUnlocked(Node->PrerequisiteNodeId))
	{
		OutError = EPATalentTreeError::PrerequisiteNotMet;
		OnTalentFailed.Broadcast(OutError);
		return false;
	}

	// Kiểm tra cấp độ và điểm kỹ năng qua UPAProgressionComponent nếu có
	UPAProgressionComponent* ProgressionComp = Owner ? Owner->FindComponentByClass<UPAProgressionComponent>() : nullptr;
	if (ProgressionComp)
	{
		if (ProgressionComp->GetCurrentLevel() < Node->RequiredLevel)
		{
			OutError = EPATalentTreeError::InsufficientLevel;
			OnTalentFailed.Broadcast(OutError);
			return false;
		}

		if (ProgressionComp->GetAvailableSkillPoints() < Node->SkillPointCost)
		{
			OutError = EPATalentTreeError::InsufficientSkillPoints;
			OnTalentFailed.Broadcast(OutError);
			return false;
		}

		// Khấu trừ điểm kỹ năng
		for (int32 i = 0; i < Node->SkillPointCost; ++i)
		{
			ProgressionComp->SpendSkillPoint();
		}
	}

	// Ghi nhận node đã mở khóa
	UnlockedNodeIds.Add(NodeId);

	// Áp dụng các chỉ số nội tại vào UAscendantAttributeSet
	ModifyAttributeSetStats(*Node, true);

	OutError = EPATalentTreeError::None;
	OnTalentNodeUnlocked.Broadcast(NodeId, *Node);
	return true;
}

bool UPATalentTreeComponent::ResetTalents(EPATalentTreeError& OutError)
{
	AActor* Owner = GetOwner();
	if (Owner && !Owner->HasAuthority())
	{
		OutError = EPATalentTreeError::NotAuthoritative;
		OnTalentFailed.Broadcast(OutError);
		return false;
	}

	if (UnlockedNodeIds.IsEmpty())
	{
		OutError = EPATalentTreeError::NoNodesToReset;
		OnTalentFailed.Broadcast(OutError);
		return false;
	}

	int32 TotalPointsRefunded = 0;

	// Gỡ bỏ toàn bộ chỉ số cộng thêm khỏi AttributeSet và tính tổng điểm hoàn trả
	for (const FName& NodeId : UnlockedNodeIds)
	{
		const FPATalentNode* Node = FPATalentTreeDatabase::FindNode(CharacterClass, NodeId);
		if (Node)
		{
			ModifyAttributeSetStats(*Node, false);
			TotalPointsRefunded += Node->SkillPointCost;
		}
	}

	// Hoàn trả điểm vào ProgressionComponent
	UPAProgressionComponent* ProgressionComp = Owner ? Owner->FindComponentByClass<UPAProgressionComponent>() : nullptr;
	if (ProgressionComp && TotalPointsRefunded > 0)
	{
		ProgressionComp->RefundSkillPoints(TotalPointsRefunded);
	}

	UnlockedNodeIds.Empty();

	OutError = EPATalentTreeError::None;
	OnTalentsReset.Broadcast(TotalPointsRefunded);
	return true;
}

// -------------------------------------------------------------------------
// Queries
// -------------------------------------------------------------------------

bool UPATalentTreeComponent::IsNodeUnlocked(FName NodeId) const
{
	return UnlockedNodeIds.Contains(NodeId);
}

float UPATalentTreeComponent::GetTotalDashCooldownReduction() const
{
	float Total = 0.0f;
	for (const FName& NodeId : UnlockedNodeIds)
	{
		const FPATalentNode* Node = FPATalentTreeDatabase::FindNode(CharacterClass, NodeId);
		if (Node)
		{
			Total += Node->DashCooldownReduction;
		}
	}
	return Total;
}

float UPATalentTreeComponent::GetTotalBonusAttackPower() const
{
	float Total = 0.0f;
	for (const FName& NodeId : UnlockedNodeIds)
	{
		const FPATalentNode* Node = FPATalentTreeDatabase::FindNode(CharacterClass, NodeId);
		if (Node)
		{
			Total += Node->BonusAttackPower;
		}
	}
	return Total;
}

float UPATalentTreeComponent::GetTotalBonusMaxPosture() const
{
	float Total = 0.0f;
	for (const FName& NodeId : UnlockedNodeIds)
	{
		const FPATalentNode* Node = FPATalentTreeDatabase::FindNode(CharacterClass, NodeId);
		if (Node)
		{
			Total += Node->BonusMaxPosture;
		}
	}
	return Total;
}

float UPATalentTreeComponent::GetTotalBonusArmor() const
{
	float Total = 0.0f;
	for (const FName& NodeId : UnlockedNodeIds)
	{
		const FPATalentNode* Node = FPATalentTreeDatabase::FindNode(CharacterClass, NodeId);
		if (Node)
		{
			Total += Node->BonusArmor;
		}
	}
	return Total;
}

float UPATalentTreeComponent::GetTotalBonusMaxHealth() const
{
	float Total = 0.0f;
	for (const FName& NodeId : UnlockedNodeIds)
	{
		const FPATalentNode* Node = FPATalentTreeDatabase::FindNode(CharacterClass, NodeId);
		if (Node)
		{
			Total += Node->BonusMaxHealth;
		}
	}
	return Total;
}

// -------------------------------------------------------------------------
// Internal
// -------------------------------------------------------------------------

void UPATalentTreeComponent::ModifyAttributeSetStats(const FPATalentNode& Node, bool bAdd)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC)
	{
		return;
	}

	const UAscendantAttributeSet* AttrSetConst = ASC->GetSet<UAscendantAttributeSet>();
	if (!AttrSetConst)
	{
		return;
	}

	UAscendantAttributeSet* AttrSet = const_cast<UAscendantAttributeSet*>(AttrSetConst);
	const float Multiplier = bAdd ? 1.0f : -1.0f;

	if (Node.BonusAttackPower != 0.0f)
	{
		AttrSet->SetAttackPower(AttrSet->GetAttackPower() + Multiplier * Node.BonusAttackPower);
	}

	if (Node.BonusMaxPosture != 0.0f)
	{
		AttrSet->SetMaxPosture(AttrSet->GetMaxPosture() + Multiplier * Node.BonusMaxPosture);
	}

	if (Node.BonusArmor != 0.0f)
	{
		AttrSet->SetArmor(AttrSet->GetArmor() + Multiplier * Node.BonusArmor);
	}

	if (Node.BonusMaxHealth != 0.0f)
	{
		AttrSet->SetMaxHealth(AttrSet->GetMaxHealth() + Multiplier * Node.BonusMaxHealth);
		if (bAdd)
		{
			AttrSet->SetHealth(AttrSet->GetHealth() + Node.BonusMaxHealth);
		}
		else
		{
			AttrSet->SetHealth(FMath::Min(AttrSet->GetHealth(), AttrSet->GetMaxHealth()));
		}
	}
}

// -------------------------------------------------------------------------
// Server RPCs
// -------------------------------------------------------------------------

bool UPATalentTreeComponent::Server_UnlockNode_Validate(FName NodeId)
{
	return NodeId != NAME_None;
}

void UPATalentTreeComponent::Server_UnlockNode_Implementation(FName NodeId)
{
	EPATalentTreeError Err = EPATalentTreeError::None;
	UnlockNode(NodeId, Err);
}

bool UPATalentTreeComponent::Server_ResetTalents_Validate()
{
	return true;
}

void UPATalentTreeComponent::Server_ResetTalents_Implementation()
{
	EPATalentTreeError Err = EPATalentTreeError::None;
	ResetTalents(Err);
}
