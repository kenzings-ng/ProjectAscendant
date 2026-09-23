// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PAPartBreakingTypes.generated.h"

/**
 * EPABossPartType
 *
 * Các bộ phận giải phẫu có thể bị phá hủy của Boss theo GDD stagger-system.md §Part Breaking System:
 * - Horn: Chiếm 20% Max HP. Khi gãy làm choáng 1.5s, cấm vĩnh viễn Iron Horn Charge, rơi Item_Beast_Horn_Shard.
 * - Tail: Chiếm 15% Max HP. Khi đứt cấm vĩnh viễn Iron Tail Sweep, rơi Item_Dragon_Tail_Sinew.
 * - ChestArmor: Chiếm 25% Max HP. Khi vỡ biến ngực thành Tử huyệt (+50% sát thương), rơi Item_Hardened_Carapace.
 */
UENUM(BlueprintType)
enum class EPABossPartType : uint8
{
	Horn          = 0 UMETA(DisplayName = "Horn (20% Max HP)"),
	Tail          = 1 UMETA(DisplayName = "Tail (15% Max HP)"),
	ChestArmor    = 2 UMETA(DisplayName = "Chest Armor (25% Max HP)")
};

/**
 * FPABossPartStatusData
 *
 * Dữ liệu trạng thái của từng bộ phận.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPABossPartStatusData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking")
	EPABossPartType PartType = EPABossPartType::Horn;

	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking")
	FString PartName;

	/** Tỷ lệ máu theo Max HP của Trùm (0.20, 0.15, 0.25) */
	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking")
	float HPRatio = 0.20f;

	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking")
	float MaxHealth = 2000.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking")
	float CurrentHealth = 2000.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking")
	bool bIsBroken = false;

	/** ID vật phẩm rơi đặc thù để rèn */
	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking")
	FName DropItemId;

	/** Tên chiêu thức bị cấm trong Behavior Tree */
	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking")
	FString DisabledAttackName;

	FPABossPartStatusData() = default;
};

/**
 * FPAPartBreakResult
 *
 * Kết quả sau một đòn tấn công vào bộ phận.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAPartBreakResult
{
	GENERATED_BODY()

	/** Bộ phận vừa bị gãy trong đòn đánh này? */
	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking|Result")
	bool bPartBroken = false;

	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking|Result")
	EPABossPartType BrokenPart = EPABossPartType::Horn;

	/** ID vật phẩm rơi */
	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking|Result")
	FName DropItemId;

	/** Sát thương thực tế đã trừ vào Tổng máu Trùm (kèm hệ số yếu điểm nếu có) */
	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking|Result")
	float DamageDealtToBoss = 0.0f;

	/** Gây choáng Boss 1.5s (dành riêng khi gãy Sừng)? */
	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking|Result")
	bool bStunnedBoss = false;

	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking|Result")
	float StunDuration = 0.0f;

	/** Vừa mở khóa tử huyệt ngực (+50% damage)? */
	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking|Result")
	bool bWeakpointRevealed = false;

	FPAPartBreakResult() = default;
};

/**
 * FPAPartBreakingModel
 *
 * Pure data model quản lý máu bộ phận, phân phối sát thương kép (Part HP & Boss Total HP),
 * và hệ số sát thương yếu điểm +50%.
 * 100% headless testable.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAPartBreakingModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking|Model")
	float BossMaxHealth = 10000.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking|Model")
	float BossCurrentHealth = 10000.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking|Model")
	TArray<FPABossPartStatusData> Parts;

	/** Tử huyệt ngực đang mở (+50% damage)? */
	UPROPERTY(BlueprintReadOnly, Category = "PartBreaking|Model")
	bool bChestWeakpointActive = false;

	// ===========================================================
	// Init
	// ===========================================================

	FPAPartBreakingModel()
	{
		Initialize(10000.0f);
	}

	void Initialize(float InBossMaxHP = 10000.0f)
	{
		BossMaxHealth = FMath::Max(1.0f, InBossMaxHP);
		BossCurrentHealth = BossMaxHealth;
		bChestWeakpointActive = false;
		Parts.Reset();

		// 1. Horn (20% Max HP = 2000 HP)
		{
			FPABossPartStatusData Part;
			Part.PartType = EPABossPartType::Horn;
			Part.PartName = TEXT("Horn");
			Part.HPRatio = 0.20f;
			Part.MaxHealth = BossMaxHealth * Part.HPRatio;
			Part.CurrentHealth = Part.MaxHealth;
			Part.bIsBroken = false;
			Part.DropItemId = FName("Item_Beast_Horn_Shard");
			Part.DisabledAttackName = TEXT("IronHornCharge");
			Parts.Add(Part);
		}

		// 2. Tail (15% Max HP = 1500 HP)
		{
			FPABossPartStatusData Part;
			Part.PartType = EPABossPartType::Tail;
			Part.PartName = TEXT("Tail");
			Part.HPRatio = 0.15f;
			Part.MaxHealth = BossMaxHealth * Part.HPRatio;
			Part.CurrentHealth = Part.MaxHealth;
			Part.bIsBroken = false;
			Part.DropItemId = FName("Item_Dragon_Tail_Sinew");
			Part.DisabledAttackName = TEXT("IronTailSweep");
			Parts.Add(Part);
		}

		// 3. Chest Armor (25% Max HP = 2500 HP)
		{
			FPABossPartStatusData Part;
			Part.PartType = EPABossPartType::ChestArmor;
			Part.PartName = TEXT("Chest Armor");
			Part.HPRatio = 0.25f;
			Part.MaxHealth = BossMaxHealth * Part.HPRatio;
			Part.CurrentHealth = Part.MaxHealth;
			Part.bIsBroken = false;
			Part.DropItemId = FName("Item_Hardened_Carapace");
			Part.DisabledAttackName = TEXT("");
			Parts.Add(Part);
		}
	}

	// ===========================================================
	// Damage & Breaking Logic
	// ===========================================================

	/**
	 * Gây sát thương vào một bộ phận cụ thể.
	 * Sát thương áp dụng đồng thời vào Part HP và Boss Total HP.
	 * Nếu ngực đã vỡ giáp -> sát thương vào ngực được nhân hệ số 1.5x (+50%).
	 */
	FPAPartBreakResult ApplyPartDamage(EPABossPartType PartType, float RawDamage)
	{
		FPAPartBreakResult Result;
		FPABossPartStatusData* Part = FindPart(PartType);
		if (!Part)
		{
			return Result;
		}

		// Tính toán hệ số sát thương (Yếu điểm ngực +50%)
		float EffectiveDamage = RawDamage;
		if (PartType == EPABossPartType::ChestArmor && bChestWeakpointActive)
		{
			EffectiveDamage = RawDamage * 1.50f; // +50% Weakpoint True Bonus
		}

		Result.DamageDealtToBoss = EffectiveDamage;

		// Trừ máu tổng của Boss
		BossCurrentHealth = FMath::Clamp(BossCurrentHealth - EffectiveDamage, 0.0f, BossMaxHealth);

		// Nếu bộ phận chưa gãy -> trừ máu bộ phận
		if (!Part->bIsBroken)
		{
			Part->CurrentHealth = FMath::Max(0.0f, Part->CurrentHealth - EffectiveDamage);

			// Kiểm tra gãy bộ phận (Part HP <= 0)
			if (Part->CurrentHealth <= 0.0f)
			{
				Part->bIsBroken = true;
				Result.bPartBroken = true;
				Result.BrokenPart = PartType;
				Result.DropItemId = Part->DropItemId;

				// Hiệu ứng riêng theo từng bộ phận
				switch (PartType)
				{
				case EPABossPartType::Horn:
					Result.bStunnedBoss = true;
					Result.StunDuration = 1.50f; // Choáng trùm 1.5s
					break;

				case EPABossPartType::ChestArmor:
					bChestWeakpointActive = true; // Mở khóa tử huyệt ngực
					Result.bWeakpointRevealed = true;
					break;

				case EPABossPartType::Tail:
					// Đứt đuôi mất thăng bằng
					break;
				}
			}
		}

		return Result;
	}

	// ===========================================================
	// Queries
	// ===========================================================

	bool IsPartBroken(EPABossPartType PartType) const
	{
		const FPABossPartStatusData* Part = FindPart(PartType);
		return Part ? Part->bIsBroken : false;
	}

	float GetPartHealth(EPABossPartType PartType) const
	{
		const FPABossPartStatusData* Part = FindPart(PartType);
		return Part ? Part->CurrentHealth : 0.0f;
	}

	float GetPartHealthPercent(EPABossPartType PartType) const
	{
		const FPABossPartStatusData* Part = FindPart(PartType);
		return (Part && Part->MaxHealth > 0.0f) ? (Part->CurrentHealth / Part->MaxHealth) : 0.0f;
	}

	float GetDamageMultiplier(EPABossPartType PartType) const
	{
		if (PartType == EPABossPartType::ChestArmor && bChestWeakpointActive)
		{
			return 1.50f; // +50%
		}
		return 1.0f;
	}

	FPABossPartStatusData* FindPart(EPABossPartType PartType)
	{
		for (FPABossPartStatusData& Part : Parts)
		{
			if (Part.PartType == PartType)
			{
				return &Part;
			}
		}
		return nullptr;
	}

	const FPABossPartStatusData* FindPart(EPABossPartType PartType) const
	{
		for (const FPABossPartStatusData& Part : Parts)
		{
			if (Part.PartType == PartType)
			{
				return &Part;
			}
		}
		return nullptr;
	}
};
