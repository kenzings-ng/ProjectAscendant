// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Progression/PAProgressionTypes.h"
#include "PAProgressionComponent.generated.h"

class UAscendantAttributeSet;
class UAbilitySystemComponent;

/**
 * UPAProgressionComponent
 *
 * Server-Authoritative component quản lý XP, Leveling và Skill Points cho nhân vật.
 * Tuân thủ ADR-0001 (Server Authority) và ADR-0003 (Zero Client Trust).
 *
 * Đường cong XP phi tuyến Level 1→50:
 *   XPToNext(Lv) = 100 * Lv^1.8
 *
 * Mỗi lần lên cấp:
 *   - MaxHealth  += 25
 *   - MaxMana    += 5
 *   - MaxStamina += 3
 *   - AttackPower += 3
 *   - Armor      += 2
 *   - SkillPoints += 1
 *
 * Replicate CurrentLevel, CurrentXP, SkillPoints qua network.
 */
UCLASS(ClassGroup = (ProjectAscendant), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAProgressionComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// -------------------------------------------------------------------------
	// Public API — Server-Authoritative
	// -------------------------------------------------------------------------

	/**
	 * Cấp XP cho nhân vật. Tự động xử lý multi-level-up nếu XP đủ.
	 * @param XPAmount Lượng XP cấp (phải > 0)
	 * @return EPAProgressionError::None nếu thành công
	 */
	EPAProgressionError GrantXP(int32 XPAmount);

	/**
	 * Tiêu 1 Skill Point (khi phân bổ vào Skill Tree).
	 * @return EPAProgressionError::None nếu thành công
	 */
	EPAProgressionError SpendSkillPoint();

	/**
	 * Hoàn trả Skill Points khi tẩy điểm (Respec).
	 * @param Amount Số điểm hoàn trả
	 */
	void RefundSkillPoints(int32 Amount);

	// -------------------------------------------------------------------------
	// Getters
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetCurrentLevel() const { return CurrentLevel; }

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetCurrentXP() const { return CurrentXP; }

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetXPToNextLevel() const;

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetAvailableSkillPoints() const { return AvailableSkillPoints; }

	UFUNCTION(BlueprintPure, Category = "Progression")
	bool IsMaxLevel() const { return CurrentLevel >= FPAProgressionFormulas::MaxLevel; }

	// -------------------------------------------------------------------------
	// Server RPC
	// -------------------------------------------------------------------------

	/** Client yêu cầu tiêu Skill Point — Server validate và xử lý */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpendSkillPoint();

	// -------------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------------

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLevelUp, int32 /* NewLevel */, const FPALevelUpReward& /* Reward */);
	FOnLevelUp OnLevelUp;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnXPGained, int32 /* XPAmount */, int32 /* NewTotalXP */);
	FOnXPGained OnXPGained;

protected:
	// -------------------------------------------------------------------------
	// Replicated State
	// -------------------------------------------------------------------------

	UPROPERTY(ReplicatedUsing = OnRep_CurrentLevel)
	int32 CurrentLevel = 1;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentXP)
	int32 CurrentXP = 0;

	UPROPERTY(Replicated)
	int32 AvailableSkillPoints = 0;

	// -------------------------------------------------------------------------
	// Rep Notifies
	// -------------------------------------------------------------------------

	UFUNCTION()
	void OnRep_CurrentLevel();

	UFUNCTION()
	void OnRep_CurrentXP();

	// -------------------------------------------------------------------------
	// Internal
	// -------------------------------------------------------------------------

	/**
	 * Áp dụng stat bonus khi lên cấp lên AttributeSet.
	 * Tìm ASC trên OwnerActor, modify attributes trực tiếp.
	 */
	void ApplyLevelUpStatBonus(const FPALevelUpReward& Reward);
};
