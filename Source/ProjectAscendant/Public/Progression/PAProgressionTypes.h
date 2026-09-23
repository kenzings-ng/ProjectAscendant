// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PAProgressionTypes.generated.h"

/**
 * EPAProgressionError
 * Mã lỗi cho các thao tác progression (cấp XP, lên level, phân bổ skill point).
 */
UENUM(BlueprintType)
enum class EPAProgressionError : uint8
{
	None = 0,
	/** Lượng XP truyền vào <= 0 */
	InvalidXPAmount,
	/** Đã đạt cấp độ tối đa (MaxLevel) */
	AlreadyMaxLevel,
	/** Không đủ Skill Points để phân bổ */
	InsufficientSkillPoints,
	/** Server authority check thất bại */
	NotAuthoritative
};

/**
 * FPALevelUpReward
 * Phần thưởng mỗi khi lên cấp: tăng chỉ số cơ sở + 1 Skill Point.
 */
USTRUCT(BlueprintType)
struct FPALevelUpReward
{
	GENERATED_BODY()

	/** Lượng MaxHealth tăng thêm mỗi cấp */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	float BonusMaxHealth = 0.f;

	/** Lượng MaxMana tăng thêm mỗi cấp */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	float BonusMaxMana = 0.f;

	/** Lượng MaxStamina tăng thêm mỗi cấp */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	float BonusMaxStamina = 0.f;

	/** Lượng AttackPower tăng thêm mỗi cấp */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	float BonusAttackPower = 0.f;

	/** Lượng Armor tăng thêm mỗi cấp */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	float BonusArmor = 0.f;

	/** Số Skill Points thưởng (luôn = 1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 SkillPointsAwarded = 1;
};

/**
 * FPAProgressionFormulas
 * Công thức XP phi tuyến cho Level 1→50.
 *
 * XP Required to reach next level:
 *   XPToNextLevel(Level) = BaseXP * Level^ExponentFactor
 *
 * Mặc định: BaseXP = 100, ExponentFactor = 1.8
 *   Level 1→2: 100 XP
 *   Level 10→11: 100 * 10^1.8 = 6,310 XP
 *   Level 49→50: 100 * 49^1.8 = 143,640 XP
 *
 * Per-Level Stat Growth:
 *   MaxHealth  += 25 per level
 *   MaxMana    += 5  per level
 *   MaxStamina += 3  per level
 *   AttackPower += 3 per level
 *   Armor      += 2  per level
 *   SkillPoints += 1 per level
 */
struct FPAProgressionFormulas
{
	// --- XP Curve ---
	static constexpr int32 MaxLevel = 50;
	static constexpr double BaseXP = 100.0;
	static constexpr double ExponentFactor = 1.8;

	/**
	 * XP cần thiết để lên từ Level hiện tại sang Level+1.
	 * Trả về 0 nếu đã max level.
	 */
	static int32 GetXPRequiredForLevel(int32 CurrentLevel)
	{
		if (CurrentLevel <= 0 || CurrentLevel >= MaxLevel)
		{
			return 0;
		}
		// XP = BaseXP * Level^ExponentFactor
		const double RawXP = BaseXP * FMath::Pow(static_cast<double>(CurrentLevel), ExponentFactor);
		return FMath::CeilToInt(RawXP);
	}

	/**
	 * Tổng XP tích lũy cần để đạt Level nhất định (từ Level 1).
	 */
	static int64 GetTotalXPForLevel(int32 TargetLevel)
	{
		if (TargetLevel <= 1)
		{
			return 0;
		}
		int64 TotalXP = 0;
		for (int32 Lv = 1; Lv < TargetLevel && Lv < MaxLevel; ++Lv)
		{
			TotalXP += GetXPRequiredForLevel(Lv);
		}
		return TotalXP;
	}

	// --- Per-Level Stat Growth ---
	static constexpr float HealthPerLevel = 25.f;
	static constexpr float ManaPerLevel = 5.f;
	static constexpr float StaminaPerLevel = 3.f;
	static constexpr float AttackPowerPerLevel = 3.f;
	static constexpr float ArmorPerLevel = 2.f;
	static constexpr int32 SkillPointsPerLevel = 1;

	/**
	 * Tính phần thưởng stat khi lên 1 cấp.
	 */
	static FPALevelUpReward GetLevelUpReward()
	{
		FPALevelUpReward Reward;
		Reward.BonusMaxHealth = HealthPerLevel;
		Reward.BonusMaxMana = ManaPerLevel;
		Reward.BonusMaxStamina = StaminaPerLevel;
		Reward.BonusAttackPower = AttackPowerPerLevel;
		Reward.BonusArmor = ArmorPerLevel;
		Reward.SkillPointsAwarded = SkillPointsPerLevel;
		return Reward;
	}
};
