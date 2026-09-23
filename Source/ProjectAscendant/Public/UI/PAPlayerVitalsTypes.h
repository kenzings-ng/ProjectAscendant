// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PAPlayerVitalsTypes.generated.h"

/**
 * EPAStaminaBarState
 *
 * Trạng thái phản hồi trực quan của thanh Thể Lực (Stamina Bar) theo GDD combat-hud.md:
 * - Normal: Màu ngọc bích (Cyan-Green), hiển thị thể lực thông thường.
 * - GoldenFlash: Lóe sáng màu vàng kim trong 0.20s khi né chuẩn (Perfect Dodge) + hoàn lại 15 Thể lực.
 * - Exhausted: Màu xám tro với biểu tượng chiếc khóa đỏ nhấp nháy khi dính State.Exhausted.
 */
UENUM(BlueprintType)
enum class EPAStaminaBarState : uint8
{
	Normal       = 0 UMETA(DisplayName = "Normal (Cyan-Green)"),
	GoldenFlash  = 1 UMETA(DisplayName = "Golden Flash (Perfect Dodge)"),
	Exhausted    = 2 UMETA(DisplayName = "Exhausted (Grayed Lock)")
};

/**
 * FPAVitalsConfig
 *
 * Tham số cân chỉnh giao diện sinh mệnh người chơi theo GDD combat-hud.md §Tuning Knobs.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAVitalsConfig
{
	GENERATED_BODY()

	/** Thời gian bóng mờ giữ nguyên vị trí trước khi co lại (mặc định 0.40s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals|Config")
	float CatchUpDelay = 0.40f;

	/** Tốc độ co dần của bóng mờ máu (FInterpTo speed, mặc định 3.5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals|Config")
	float CatchUpInterpSpeed = 3.5f;

	/** Thời gian lóe sáng màu vàng kim khi né hoàn hảo (mặc định 0.20s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals|Config")
	float GoldenFlashDuration = 0.20f;

	/** Ngưỡng máu kích hoạt nhịp tim cảnh báo nguy cấp (< 20% Max HP) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals|Config")
	float LowHealthThreshold = 0.20f;

	/** Tần số nhịp tim tối thiểu (tại mốc 20% máu: 60 BPM) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals|Config")
	float MinHeartbeatBPM = 60.0f;

	/** Tần số nhịp tim tối đa (khi máu sát 0%: 100 BPM) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals|Config")
	float MaxHeartbeatBPM = 100.0f;

	/** Lượng thể lực hoàn trả khi kích hoạt Perfect Dodge (mặc định +15) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals|Config")
	float PerfectDodgeStaminaRefund = 15.0f;
};

/**
 * FPAVitalsModel
 *
 * Data model tính toán thuần túy cho HUD sinh mệnh người chơi (Health, Stamina, Mana).
 * Cho phép kiểm thử tự động 100% không cần phụ thuộc vào Slate/UMG Viewport.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAVitalsModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	float CurrentHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	float GhostHealthPercent = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	float GhostDelayTimer = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	float CurrentStamina = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	float MaxStamina = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	float CurrentMana = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	float MaxMana = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	EPAStaminaBarState StaminaState = EPAStaminaBarState::Normal;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	float GoldenFlashTimer = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	bool bIsExhausted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	bool bLowHealthVignetteActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	bool bExhaustionVignetteActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals|Model")
	float CurrentHeartbeatBPM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals|Model")
	FPAVitalsConfig Config;

	FPAVitalsModel()
	{
		CurrentHealth = 100.0f;
		MaxHealth = 100.0f;
		GhostHealthPercent = 1.0f;
		GhostDelayTimer = 0.0f;
		CurrentStamina = 100.0f;
		MaxStamina = 100.0f;
		CurrentMana = 100.0f;
		MaxMana = 100.0f;
		StaminaState = EPAStaminaBarState::Normal;
		GoldenFlashTimer = 0.0f;
		bIsExhausted = false;
		bLowHealthVignetteActive = false;
		bExhaustionVignetteActive = false;
		CurrentHeartbeatBPM = 0.0f;
	}

	float GetHealthPercent() const
	{
		return MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
	}

	float GetGhostHealthPercent() const
	{
		return FMath::Clamp(GhostHealthPercent, 0.0f, 1.0f);
	}

	float GetStaminaPercent() const
	{
		return MaxStamina > 0.0f ? FMath::Clamp(CurrentStamina / MaxStamina, 0.0f, 1.0f) : 0.0f;
	}

	float GetManaPercent() const
	{
		return MaxMana > 0.0f ? FMath::Clamp(CurrentMana / MaxMana, 0.0f, 1.0f) : 0.0f;
	}

	void SetHealth(float InCurrent, float InMax)
	{
		const float OldHealthPercent = GetHealthPercent();
		MaxHealth = FMath::Max(1.0f, InMax);
		CurrentHealth = FMath::Clamp(InCurrent, 0.0f, MaxHealth);
		const float NewHealthPercent = GetHealthPercent();

		if (NewHealthPercent < OldHealthPercent)
		{
			// Mất máu: kích hoạt bóng mờ trễ 0.40s
			GhostDelayTimer = Config.CatchUpDelay;
			// GhostHealthPercent giữ nguyên vị trí cũ
		}
		else
		{
			// Hồi máu: bóng mờ lập tức đi cùng máu thực tế
			GhostHealthPercent = NewHealthPercent;
			GhostDelayTimer = 0.0f;
		}

		UpdateLowHealthStatus();
	}

	void SetStamina(float InCurrent, float InMax)
	{
		MaxStamina = FMath::Max(1.0f, InMax);
		CurrentStamina = FMath::Clamp(InCurrent, 0.0f, MaxStamina);
	}

	void SetMana(float InCurrent, float InMax)
	{
		MaxMana = FMath::Max(1.0f, InMax);
		CurrentMana = FMath::Clamp(InCurrent, 0.0f, MaxMana);
	}

	void TriggerPerfectDodge()
	{
		if (!bIsExhausted)
		{
			StaminaState = EPAStaminaBarState::GoldenFlash;
			GoldenFlashTimer = Config.GoldenFlashDuration;
			// Cập nhật thể lực hoàn lại
			CurrentStamina = FMath::Clamp(CurrentStamina + Config.PerfectDodgeStaminaRefund, 0.0f, MaxStamina);
		}
	}

	void SetExhausted(bool bInExhausted)
	{
		bIsExhausted = bInExhausted;
		bExhaustionVignetteActive = bInExhausted;

		if (bInExhausted)
		{
			StaminaState = EPAStaminaBarState::Exhausted;
			GoldenFlashTimer = 0.0f;
		}
		else
		{
			StaminaState = EPAStaminaBarState::Normal;
		}
	}

	void Update(float DeltaTime)
	{
		// Cập nhật bóng mờ trượt (Catch-up Ghost Bar)
		const float TargetPercent = GetHealthPercent();
		if (GhostDelayTimer > 0.0f)
		{
			GhostDelayTimer = FMath::Max(0.0f, GhostDelayTimer - DeltaTime);
		}
		else
		{
			GhostHealthPercent = FMath::FInterpTo(GhostHealthPercent, TargetPercent, DeltaTime, Config.CatchUpInterpSpeed);
		}

		// Cập nhật tia chớp vàng Perfect Dodge
		if (StaminaState == EPAStaminaBarState::GoldenFlash)
		{
			GoldenFlashTimer -= DeltaTime;
			if (GoldenFlashTimer <= 0.0f)
			{
				StaminaState = bIsExhausted ? EPAStaminaBarState::Exhausted : EPAStaminaBarState::Normal;
			}
		}

		UpdateLowHealthStatus();
	}

	void UpdateLowHealthStatus()
	{
		const float HPPercent = GetHealthPercent();
		if (HPPercent < Config.LowHealthThreshold && HPPercent > 0.0f)
		{
			bLowHealthVignetteActive = true;
			// Công thức nhịp tim dồn dập (GDD §Formulas 2):
			// BPM(HP) = 60 + (100 - 60) * (1 - HP / (0.20 * MaxHP))
			const float Ratio = FMath::Clamp(CurrentHealth / (Config.LowHealthThreshold * MaxHealth), 0.0f, 1.0f);
			CurrentHeartbeatBPM = Config.MinHeartbeatBPM + (Config.MaxHeartbeatBPM - Config.MinHeartbeatBPM) * (1.0f - Ratio);
		}
		else
		{
			bLowHealthVignetteActive = false;
			CurrentHeartbeatBPM = 0.0f;
		}
	}
};
