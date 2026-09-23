// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PAKarmaTypes.generated.h"

/**
 * EPAKarmaTier
 *
 * 5 bậc danh dự Karma theo GDD zone-system.md §Karma System:
 * - Righteous (+50 đến +100): Tên xanh lục, giảm 10% phí sửa đồ.
 * - Neutral (0 đến +49): Tên trắng, được pháp luật bảo hộ.
 * - Aggressor: Tên xám tạm thời trong 120s khi gây hấn trước, bị cấm vào thị trấn.
 * - Criminal (-1 đến -49): Tên cam, lính gác từ chối phục vụ.
 * - WantedOutlaw (-50 đến -100): Tên đỏ + đầu lâu, bị lính gác giết ngay khi gặp, bị săn tiền thưởng.
 */
UENUM(BlueprintType)
enum class EPAKarmaTier : uint8
{
	Righteous     = 0 UMETA(DisplayName = "Righteous (+50 to +100)"),
	Neutral       = 1 UMETA(DisplayName = "Neutral (0 to +49)"),
	Aggressor     = 2 UMETA(DisplayName = "Aggressor (120s Countdown)"),
	Criminal      = 3 UMETA(DisplayName = "Criminal (-1 to -49)"),
	WantedOutlaw  = 4 UMETA(DisplayName = "Wanted Outlaw (-50 to -100)")
};

/**
 * EPADeathScenario
 *
 * Kịch bản tử vong của nhân vật:
 * - PvE: Chết do quái vật hoặc bẫy môi trường.
 * - PvPVictim: Người vô tội (Karma >= 0) bị người chơi khác PK.
 * - WantedOutlaw: Kẻ có lệnh truy nã (Karma < -50) bị tiêu diệt.
 */
UENUM(BlueprintType)
enum class EPADeathScenario : uint8
{
	PvE           = 0 UMETA(DisplayName = "PvE Death (Monster / Environment)"),
	PvPVictim     = 1 UMETA(DisplayName = "PvP Victim (Innocent killed by PK)"),
	WantedOutlaw  = 2 UMETA(DisplayName = "Wanted Outlaw Death (Killed by Guards / Bounty)")
};

/**
 * FPADeathPenaltyResult
 *
 * Kết quả phân giải hình phạt khi nhân vật tử vong.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPADeathPenaltyResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Karma|Penalty")
	float GoldLost = 0.0f;

	/** Số vàng chuyển thẳng cho kẻ sát nhân (nếu là nạn nhân PvP) */
	UPROPERTY(BlueprintReadOnly, Category = "Karma|Penalty")
	float GoldToKiller = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Karma|Penalty")
	float ShardsLost = 0.0f;

	/** Tạo Tàn Tích Tro (Ash Remnant) tại vị trí chết? */
	UPROPERTY(BlueprintReadOnly, Category = "Karma|Penalty")
	bool bAshRemnantCreated = false;

	/** Bị tống vào Nhà tù Lao Động (Labor Prison)? */
	UPROPERTY(BlueprintReadOnly, Category = "Karma|Penalty")
	bool bSentToPrison = false;

	UPROPERTY(BlueprintReadOnly, Category = "Karma|Penalty")
	float PrisonDurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Karma|Penalty")
	int32 OreRequiredForRelease = 0;

	/** Rơi 1 vật phẩm ngẫu nhiên trong túi đồ? */
	UPROPERTY(BlueprintReadOnly, Category = "Karma|Penalty")
	bool bDropInventoryItem = false;

	FPADeathPenaltyResult() = default;
};

/**
 * FPAKarmaConfig
 *
 * Tham số cấu hình Karma và hình phạt chết theo GDD zone-system.md.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAKarmaConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Karma|Config")
	float MinKarma = -100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Karma|Config")
	float MaxKarma = 100.0f;

	/** Thời gian đếm ngược trạng thái Gây hấn Aggressor (120s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Karma|Config")
	float AggressorDuration = 120.0f;

	/** Trừ Karma khi giết người vô tội (-30 điểm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Karma|Config")
	float KillInnocentPenalty = -30.0f;

	/** Tỷ lệ mất Vàng khi chết PvE (50%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Karma|Config")
	float PvEGoldLossPct = 0.50f;

	/** Tỷ lệ mất Vàng cho sát thủ khi bị PK (25%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Karma|Config")
	float PvPVictimGoldLossPct = 0.25f;

	/** Thời gian ngồi tù của kẻ Wanted (300s = 5 phút) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Karma|Config")
	float PrisonDuration = 300.0f;

	/** Số quặng cần đào để ra tù sớm (20 quặng) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Karma|Config")
	int32 PrisonOresRequired = 20;

	/** Điểm Karma sau khi được tha tù (-49) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Karma|Config")
	float PrisonReleaseKarma = -49.0f;
};

/**
 * FPAKarmaModel
 *
 * Pure data model quản lý điểm Karma, chuyển trạng thái 5 bậc,
 * kịch bản hình phạt chết, và chu kỳ thụ án Nhà tù Lao Động.
 * 100% headless testable.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAKarmaModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Karma|Model")
	float CurrentKarma = 0.0f;

	/** Thời gian đếm ngược trạng thái Gây hấn Aggressor còn lại (giây) */
	UPROPERTY(BlueprintReadOnly, Category = "Karma|Model")
	float AggressorTimeRemaining = 0.0f;

	/** Đang bị giam trong Nhà tù Lao Động? */
	UPROPERTY(BlueprintReadOnly, Category = "Karma|Model")
	bool bInLaborPrison = false;

	UPROPERTY(BlueprintReadOnly, Category = "Karma|Model")
	float PrisonTimeRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Karma|Model")
	int32 MinedOresCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Karma|Model")
	FPAKarmaConfig Config;

	// ===========================================================
	// Init
	// ===========================================================

	FPAKarmaModel() = default;

	void Initialize(float InitialKarma = 0.0f)
	{
		CurrentKarma = FMath::Clamp(InitialKarma, Config.MinKarma, Config.MaxKarma);
		AggressorTimeRemaining = 0.0f;
		bInLaborPrison = false;
		PrisonTimeRemaining = 0.0f;
		MinedOresCount = 0;
	}

	// ===========================================================
	// Karma Tier Evaluation
	// ===========================================================

	EPAKarmaTier GetKarmaTier() const
	{
		// Ưu tiên trạng thái Aggressor tạm thời nếu đang đếm ngược
		if (AggressorTimeRemaining > 0.0f)
		{
			return EPAKarmaTier::Aggressor;
		}

		if (CurrentKarma >= 50.0f)
		{
			return EPAKarmaTier::Righteous;
		}
		if (CurrentKarma >= 0.0f)
		{
			return EPAKarmaTier::Neutral;
		}
		if (CurrentKarma > -50.0f)
		{
			return EPAKarmaTier::Criminal;
		}

		return EPAKarmaTier::WantedOutlaw; // Karma <= -50.0f
	}

	void TriggerAggressor()
	{
		AggressorTimeRemaining = Config.AggressorDuration; // 120s
	}

	void AddKarma(float Delta)
	{
		CurrentKarma = FMath::Clamp(CurrentKarma + Delta, Config.MinKarma, Config.MaxKarma);
	}

	// ===========================================================
	// Tick Update
	// ===========================================================

	void Update(float DeltaTime)
	{
		if (AggressorTimeRemaining > 0.0f)
		{
			AggressorTimeRemaining = FMath::Max(0.0f, AggressorTimeRemaining - DeltaTime);
		}

		if (bInLaborPrison)
		{
			PrisonTimeRemaining = FMath::Max(0.0f, PrisonTimeRemaining - DeltaTime);
			if (PrisonTimeRemaining <= 0.0f)
			{
				ReleaseFromPrison();
			}
		}
	}

	// ===========================================================
	// Death Penalty Resolution
	// ===========================================================

	FPADeathPenaltyResult ResolveDeath(EPADeathScenario Scenario, float CurrentGold, float CurrentShards, float RandomRoll = 0.5f)
	{
		FPADeathPenaltyResult Result;

		switch (Scenario)
		{
		case EPADeathScenario::PvE:
			{
				// AC-2: Mất 50% Vàng và 100% Shards rơi vào Tàn tích Tro; trang bị bảo toàn
				Result.GoldLost = FMath::FloorToFloat(CurrentGold * Config.PvEGoldLossPct);
				Result.ShardsLost = CurrentShards;
				Result.bAshRemnantCreated = true;
				Result.bSentToPrison = false;
				Result.bDropInventoryItem = false;
			}
			break;

		case EPADeathScenario::PvPVictim:
			{
				// AC-3: Nạn nhân vô tội chỉ mất 25% Vàng cho sát thủ; bảo toàn 100% Shards; không tạo Ash Remnant
				Result.GoldLost = FMath::FloorToFloat(CurrentGold * Config.PvPVictimGoldLossPct);
				Result.GoldToKiller = Result.GoldLost;
				Result.ShardsLost = 0.0f;
				Result.bAshRemnantCreated = false;
				Result.bSentToPrison = false;
				Result.bDropInventoryItem = false;
			}
			break;

		case EPADeathScenario::WantedOutlaw:
			{
				// AC-4: Rơi 100% Vàng, 100% Shards; 15% rơi 1 item; tống vào tù 5 phút hoặc đào 20 quặng
				Result.GoldLost = CurrentGold;
				Result.ShardsLost = CurrentShards;
				Result.bAshRemnantCreated = false; // Rơi tự do nhặt ngay
				Result.bSentToPrison = true;
				Result.PrisonDurationSeconds = Config.PrisonDuration;
				Result.OreRequiredForRelease = Config.PrisonOresRequired;
				Result.bDropInventoryItem = (RandomRoll <= 0.15f); // 15% tỷ lệ rơi đồ

				// Tống vào tù
				SendToPrison();
			}
			break;
		}

		return Result;
	}

	// ===========================================================
	// Labor Prison System
	// ===========================================================

	void SendToPrison()
	{
		bInLaborPrison = true;
		PrisonTimeRemaining = Config.PrisonDuration;
		MinedOresCount = 0;
	}

	/**
	 * Đào quặng trong tù để giảm thời gian / được tha sớm.
	 * @return true nếu đã đào đủ 20 quặng và được thả tự do.
	 */
	bool MineOreInPrison(int32 Amount = 1)
	{
		if (!bInLaborPrison)
		{
			return false;
		}

		MinedOresCount += Amount;

		if (MinedOresCount >= Config.PrisonOresRequired)
		{
			ReleaseFromPrison();
			return true;
		}

		return false;
	}

	void ReleaseFromPrison()
	{
		bInLaborPrison = false;
		PrisonTimeRemaining = 0.0f;
		MinedOresCount = 0;
		// Ra tù: Karma ấn định lại ở -49 (thoát mức Wanted)
		CurrentKarma = FMath::Max(CurrentKarma, Config.PrisonReleaseKarma);
	}
};
