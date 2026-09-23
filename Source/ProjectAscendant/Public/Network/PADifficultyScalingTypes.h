// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PADifficultyScalingTypes.generated.h"

/**
 * FPADDSConfig
 *
 * Tham số co giãn độ khó động lực (DDS) và ngưỡng nhận thưởng Instanced Loot
 * theo GDD zone-system.md §Contested Combat & Open Tagging.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPADDSConfig
{
	GENERATED_BODY()

	/** Hệ số co giãn máu Boss theo người chơi tham chiến (+50%/người) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDS|Config")
	float HPScaleCoeff = 0.50f;

	/** Hệ số co giãn Posture Boss theo người chơi tham chiến (+35%/người) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDS|Config")
	float PostureScaleCoeff = 0.35f;

	/** Bán kính tính người tham chiến (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDS|Config")
	float CombatantRadius = 3000.0f;

	/** Ngưỡng sát thương Máu tối thiểu để nhận loot (5% = 0.05) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDS|Config")
	float MinContributionHPPct = 0.05f;

	/** Ngưỡng sát thương Posture tối thiểu để nhận loot (10% = 0.10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDS|Config")
	float MinContributionPosturePct = 0.10f;

	/** Ngưỡng Anti-Zerg Bậc 1 (4 người) -> Giảm 30% CC */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDS|Config")
	int32 AntiZergTier1Count = 4;

	/** Ngưỡng Anti-Zerg Bậc 2 (8 người) -> Giảm 50% CC + 25% Pivot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDS|Config")
	int32 AntiZergTier2Count = 8;
};

/**
 * FPACombatantContribution
 *
 * Bản ghi đóng góp sát thương của một người chơi vào Boss.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPACombatantContribution
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DDS|Contribution")
	FString PlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "DDS|Contribution")
	float TotalHPDamage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DDS|Contribution")
	float TotalPostureDamage = 0.0f;

	FPACombatantContribution() = default;
};

/**
 * FPADifficultyScalingModel
 *
 * Pure data model tính toán Co giãn Máu & Posture của Boss theo thời gian thực,
 * tỷ lệ kháng khống chế Anti-Zerg, và xét duyệt tư cách nhận Instanced Loot.
 * 100% headless testable.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPADifficultyScalingModel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDS|Model")
	float BaseMaxHP = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDS|Model")
	float BaseMaxPosture = 800.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DDS|Model")
	TArray<FPACombatantContribution> Combatants;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDS|Model")
	FPADDSConfig Config;

	// ===========================================================
	// Init
	// ===========================================================

	FPADifficultyScalingModel() = default;

	void Initialize(float InBaseHP = 10000.0f, float InBasePosture = 800.0f)
	{
		BaseMaxHP = FMath::Max(1.0f, InBaseHP);
		BaseMaxPosture = FMath::Max(1.0f, InBasePosture);
		Combatants.Reset();
	}

	// ===========================================================
	// Combatant Management
	// ===========================================================

	void RegisterCombatant(const FString& PlayerId)
	{
		if (!FindCombatant(PlayerId))
		{
			FPACombatantContribution Entry;
			Entry.PlayerId = PlayerId;
			Entry.TotalHPDamage = 0.0f;
			Entry.TotalPostureDamage = 0.0f;
			Combatants.Add(Entry);
		}
	}

	void RecordDamage(const FString& PlayerId, float HPDamage, float PostureDamage)
	{
		FPACombatantContribution* Entry = FindCombatant(PlayerId);
		if (!Entry)
		{
			RegisterCombatant(PlayerId);
			Entry = FindCombatant(PlayerId);
		}

		if (Entry)
		{
			Entry->TotalHPDamage += HPDamage;
			Entry->TotalPostureDamage += PostureDamage;
		}
	}

	int32 GetCombatantCount() const
	{
		return Combatants.Num();
	}

	// ===========================================================
	// Scaling Formulas
	// ===========================================================

	/**
	 * Máu tối đa co giãn theo công thức GDD:
	 * ScaledMaxHP = BaseMaxHP * (1.0 + 0.50 * max(0, N - 1))
	 */
	float ComputeScaledMaxHP() const
	{
		const int32 N = FMath::Max(1, Combatants.Num());
		const float ScaleFactor = 1.0f + Config.HPScaleCoeff * static_cast<float>(N - 1);
		return FMath::RoundToFloat(BaseMaxHP * ScaleFactor);
	}

	/**
	 * Posture tối đa co giãn theo công thức GDD:
	 * ScaledMaxPosture = BaseMaxPosture * (1.0 + 0.35 * max(0, N - 1))
	 */
	float ComputeScaledMaxPosture() const
	{
		const int32 N = FMath::Max(1, Combatants.Num());
		const float ScaleFactor = 1.0f + Config.PostureScaleCoeff * static_cast<float>(N - 1);
		return FMath::RoundToFloat(BaseMaxPosture * ScaleFactor);
	}

	/**
	 * Tỷ lệ kháng khống chế Anti-Zerg (CC Reduction):
	 * N >= 8: 50% CC reduction
	 * N >= 4: 30% CC reduction
	 * N < 4: 0%
	 */
	float ComputeCCReduction() const
	{
		const int32 N = Combatants.Num();
		if (N >= Config.AntiZergTier2Count)
		{
			return 0.50f;
		}
		if (N >= Config.AntiZergTier1Count)
		{
			return 0.30f;
		}
		return 0.0f;
	}

	/**
	 * Tốc độ xoay chuyển hướng đòn đánh (+25% khi N >= 8)
	 */
	float ComputeTurnRateBonus() const
	{
		return (Combatants.Num() >= Config.AntiZergTier2Count) ? 0.25f : 0.0f;
	}

	// ===========================================================
	// Instanced Loot Qualification
	// ===========================================================

	/**
	 * Kiểm tra tư cách nhận Instanced Loot:
	 * Đạt >= 5% ScaledMaxHP HOẶC >= 10% ScaledMaxPosture
	 */
	bool IsQualifiedForLoot(const FString& PlayerId) const
	{
		const FPACombatantContribution* Entry = FindCombatant(PlayerId);
		if (!Entry)
		{
			return false;
		}

		const float ScaledHP = ComputeScaledMaxHP();
		const float ScaledPosture = ComputeScaledMaxPosture();

		const float MinHPRequired = ScaledHP * Config.MinContributionHPPct;
		const float MinPostureRequired = ScaledPosture * Config.MinContributionPosturePct;

		return (Entry->TotalHPDamage >= MinHPRequired) || (Entry->TotalPostureDamage >= MinPostureRequired);
	}

	TArray<FString> GetQualifiedPlayers() const
	{
		TArray<FString> Qualified;
		for (const FPACombatantContribution& Entry : Combatants)
		{
			if (IsQualifiedForLoot(Entry.PlayerId))
			{
				Qualified.Add(Entry.PlayerId);
			}
		}
		return Qualified;
	}

	// ===========================================================
	// Helpers
	// ===========================================================

	FPACombatantContribution* FindCombatant(const FString& PlayerId)
	{
		for (FPACombatantContribution& Entry : Combatants)
		{
			if (Entry.PlayerId == PlayerId)
			{
				return &Entry;
			}
		}
		return nullptr;
	}

	const FPACombatantContribution* FindCombatant(const FString& PlayerId) const
	{
		for (const FPACombatantContribution& Entry : Combatants)
		{
			if (Entry.PlayerId == PlayerId)
			{
				return &Entry;
			}
		}
		return nullptr;
	}
};
