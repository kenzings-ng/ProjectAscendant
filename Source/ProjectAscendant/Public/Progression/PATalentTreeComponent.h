// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Progression/PATalentTreeTypes.h"
#include "PATalentTreeComponent.generated.h"

class UPAProgressionComponent;
class UAbilitySystemComponent;
class UAscendantAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnTalentNodeUnlocked, FName, NodeId, const FPATalentNode&, NodeData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnTalentsReset, int32, PointsRefunded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnTalentFailed, EPATalentTreeError, ErrorCode);

/**
 * UPATalentTreeComponent
 *
 * Component quản lý Cây Kỹ Năng và Mở Khóa Node Nội Tại (Story prog-002).
 * - AC-1: Cây kỹ năng 3 nhánh chuyên môn hóa cho Vanguard, Ranger, Arcanist.
 * - AC-2: Mở khóa node tiêu hao Skill Points, áp dụng trực tiếp lên UAscendantAttributeSet (Attack, Posture, Dash CD, Armor, Health).
 * - AC-3: Hỗ trợ tẩy điểm kỹ năng (ResetTalents), hoàn trả toàn bộ Skill Points và gỡ bỏ chỉ số.
 */
UCLASS(ClassGroup = (ProjectAscendant), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPATalentTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPATalentTreeComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// -------------------------------------------------------------------------
	// Class Configuration
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Talents")
	EPACharacterClass GetCharacterClass() const { return CharacterClass; }

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Talents")
	void SetCharacterClass(EPACharacterClass InClass) { CharacterClass = InClass; }

	// -------------------------------------------------------------------------
	// Public API — Server-Authoritative
	// -------------------------------------------------------------------------

	/**
	 * AC-2: Mở khóa một Node trên Cây Kỹ Năng.
	 * Tiêu hao 1 Skill Point từ UPAProgressionComponent và cộng chỉ số vào UAscendantAttributeSet.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Talents")
	bool UnlockNode(FName NodeId, EPATalentTreeError& OutError);

	/**
	 * AC-3: Tẩy toàn bộ điểm kỹ năng đã phân bổ, hoàn trả lại UPAProgressionComponent.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Talents")
	bool ResetTalents(EPATalentTreeError& OutError);

	// -------------------------------------------------------------------------
	// Queries
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Talents")
	bool IsNodeUnlocked(FName NodeId) const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Talents")
	const TArray<FName>& GetUnlockedNodeIds() const { return UnlockedNodeIds; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Talents")
	int32 GetUnlockedNodeCount() const { return UnlockedNodeIds.Num(); }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Talents")
	float GetTotalDashCooldownReduction() const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Talents")
	float GetTotalBonusAttackPower() const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Talents")
	float GetTotalBonusMaxPosture() const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Talents")
	float GetTotalBonusArmor() const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Talents")
	float GetTotalBonusMaxHealth() const;

	// -------------------------------------------------------------------------
	// Server RPCs
	// -------------------------------------------------------------------------

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Talents")
	void Server_UnlockNode(FName NodeId);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Talents")
	void Server_ResetTalents();

	// -------------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Talents")
	FPAOnTalentNodeUnlocked OnTalentNodeUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Talents")
	FPAOnTalentsReset OnTalentsReset;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Talents")
	FPAOnTalentFailed OnTalentFailed;

protected:
	UPROPERTY(Replicated)
	EPACharacterClass CharacterClass = EPACharacterClass::Vanguard;

	UPROPERTY(Replicated)
	TArray<FName> UnlockedNodeIds;

	/** Điều chỉnh thuộc tính trên UAscendantAttributeSet (+ nếu bAdd, - nếu !bAdd) */
	void ModifyAttributeSetStats(const FPATalentNode& Node, bool bAdd);
};
