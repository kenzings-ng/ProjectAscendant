// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PADashTypes.generated.h"

/**
 * EPADashPhase
 *
 * Các pha chuyển động của cú lướt theo GDD dash-evasion.md §The 3 Dash Phases:
 * - None: Nhân vật đứng yên / di chuyển bình thường.
 * - IFramePeak: 0.00s -> 0.28s, Invulnerable = true, tốc độ đỉnh 1200 -> 900 cm/s, xuyên qua thân quái.
 * - Recovery: 0.28s -> 0.35s, Invulnerable = false, giảm tốc 900 -> 550 cm/s, hồi va chạm vật lý.
 * - AttackCancelable: 0.35s -> 0.45s, người chơi có thể nhấn Đánh để hủy hồi phục và tung Dash Attack.
 */
UENUM(BlueprintType)
enum class EPADashPhase : uint8
{
	None              = 0 UMETA(DisplayName = "None"),
	IFramePeak        = 1 UMETA(DisplayName = "I-Frame Peak (0.00s-0.28s Invulnerable)"),
	Recovery          = 2 UMETA(DisplayName = "Recovery (0.28s-0.35s Vulnerable)"),
	AttackCancelable  = 3 UMETA(DisplayName = "Attack Cancelable (0.35s-0.45s Cancel Window)"),
	Completed         = 4 UMETA(DisplayName = "Completed")
};

/**
 * FPADashConfig
 *
 * Tham số cân chỉnh kỹ thuật né đòn theo GDD dash-evasion.md §Tuning Knobs.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPADashConfig
{
	GENERATED_BODY()

	/** Tổng thời gian lướt (0.45s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Config")
	float TotalDuration = 0.45f;

	/** Thời gian bất tử tuyệt đối I-Frame (0.28s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Config")
	float IFrameDuration = 0.28f;

	/** Thời điểm bắt đầu khung thời gian vàng Perfect Dodge (0.05s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Config")
	float SweetSpotStart = 0.05f;

	/** Thời điểm kết thúc khung thời gian vàng Perfect Dodge (0.15s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Config")
	float SweetSpotEnd = 0.15f;

	/** Thời điểm mở cửa sổ hủy hồi phục sang Dash Attack (0.35s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Config")
	float CancelWindowStart = 0.35f;

	/** Chi phí thể lực cơ bản (25 Stamina) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Config")
	float StaminaCost = 25.0f;

	/** Thể lực hoàn lại khi kích hoạt Perfect Dodge (+15 Stamina) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Config")
	float StaminaRefund = 15.0f;

	/** Thời gian Hitstop toàn cục (0.08s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Config")
	float HitstopDuration = 0.08f;

	/** Time Dilation môi trường khi Hitstop (0.1x) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Config")
	float HitstopDilation = 0.10f;

	/** Vận tốc đỉnh ban đầu (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Config")
	float PeakVelocity = 1200.0f;

	/** Vận tốc cuối khi kết thúc lướt (550 cm/s = tốc độ chạy thường) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Config")
	float EndVelocity = 550.0f;

	/** Thời gian kéo dài cờ PerfectDodgeTriggered (2.0s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Config")
	float PerfectDodgeBuffDuration = 2.0f;
};

/**
 * FPAIncomingHitResult
 *
 * Kết quả phân giải khi người chơi bị đòn đánh quét trúng trong lúc lướt.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAIncomingHitResult
{
	GENERATED_BODY()

	/** Đã né thành công nhờ I-Frame? */
	UPROPERTY(BlueprintReadOnly, Category = "Dash|Result")
	bool bAvoidedWithIFrame = false;

	/** Kích hoạt Perfect Dodge (trong sweet-spot 0.05-0.15s)? */
	UPROPERTY(BlueprintReadOnly, Category = "Dash|Result")
	bool bTriggeredPerfectDodge = false;

	/** Số Stamina được hoàn lại */
	UPROPERTY(BlueprintReadOnly, Category = "Dash|Result")
	float StaminaRefunded = 0.0f;

	/** Thời gian Hitstop kích hoạt (giây) */
	UPROPERTY(BlueprintReadOnly, Category = "Dash|Result")
	float HitstopDuration = 0.0f;

	/** Sát thương thực tế phải nhận (0 nếu trong I-Frame) */
	UPROPERTY(BlueprintReadOnly, Category = "Dash|Result")
	float DamageTaken = 0.0f;

	FPAIncomingHitResult() = default;
};

/**
 * FPADashModel
 *
 * Pure data model quản lý tiến trình lướt, tính toán vận tốc phi tuyến tính,
 * kiểm tra sweet-spot né hoàn hảo, chống rơi mép vực, và hủy chiêu.
 * 100% headless testable.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPADashModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dash|Model")
	EPADashPhase CurrentPhase = EPADashPhase::None;

	UPROPERTY(BlueprintReadOnly, Category = "Dash|Model")
	bool bIsDashing = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dash|Model")
	bool bInvulnerable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dash|Model")
	bool bCanCancelIntoAttack = false;

	/** Chống rơi mép vực trong suốt cú lướt (bCanWalkOffLedges = false) */
	UPROPERTY(BlueprintReadOnly, Category = "Dash|Model")
	bool bCanWalkOffLedges = true;

	UPROPERTY(BlueprintReadOnly, Category = "Dash|Model")
	float Elapsed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Dash|Model")
	float CurrentVelocity = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Dash|Model")
	float TotalDistanceTraveled = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Dash|Model")
	bool bPerfectDodgeTriggered = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dash|Model")
	float PerfectDodgeBuffRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Dash|Model")
	float HitstopRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Model")
	FPADashConfig Config;

	// ===========================================================
	// Lifecycle
	// ===========================================================

	FPADashModel() = default;

	/**
	 * Khởi động cú lướt.
	 * @param CurrentStamina Thể lực hiện tại của người chơi
	 * @param OutStaminaRemaining Thể lực còn lại sau khi trừ chi phí
	 * @return true nếu bắt đầu lướt thành công (đủ 25 stamina)
	 */
	bool StartDash(float CurrentStamina, float& OutStaminaRemaining)
	{
		if (bIsDashing || CurrentStamina < Config.StaminaCost)
		{
			OutStaminaRemaining = CurrentStamina;
			return false;
		}

		bIsDashing = true;
		CurrentPhase = EPADashPhase::IFramePeak;
		bInvulnerable = true;
		bCanCancelIntoAttack = false;
		bCanWalkOffLedges = false; // AC-3: Chống rơi mép vực
		Elapsed = 0.0f;
		CurrentVelocity = Config.PeakVelocity;
		TotalDistanceTraveled = 0.0f;
		bPerfectDodgeTriggered = false;

		OutStaminaRemaining = CurrentStamina - Config.StaminaCost;
		return true;
	}

	// ===========================================================
	// Update Tick
	// ===========================================================

	/**
	 * Cập nhật tiến trình lướt qua từng frame.
	 * @return true nếu cú lướt vừa hoàn tất trong frame này.
	 */
	bool Update(float DeltaTime)
	{
		// Cập nhật bộ đếm Hitstop
		if (HitstopRemaining > 0.0f)
		{
			HitstopRemaining = FMath::Max(0.0f, HitstopRemaining - DeltaTime);
		}

		// Cập nhật buff phản công Perfect Dodge
		if (PerfectDodgeBuffRemaining > 0.0f)
		{
			PerfectDodgeBuffRemaining = FMath::Max(0.0f, PerfectDodgeBuffRemaining - DeltaTime);
			if (PerfectDodgeBuffRemaining <= 0.0f)
			{
				bPerfectDodgeTriggered = false;
			}
		}

		if (!bIsDashing)
		{
			return false;
		}

		Elapsed += DeltaTime;

		// Tính toán vận tốc phi tuyến tính theo GDD:
		// t in [0.00, 0.20]: V = 1200 cm/s
		// t in [0.20, 0.45]: V(t) = 550 + (1200 - 550) * (1 - (t - 0.20)/0.25)^2
		CurrentVelocity = ComputeVelocity(Elapsed);
		TotalDistanceTraveled += CurrentVelocity * DeltaTime;

		// Phân chia pha chuyển động:
		if (Elapsed < Config.IFrameDuration) // 0.00s -> 0.28s
		{
			CurrentPhase = EPADashPhase::IFramePeak;
			bInvulnerable = true;
			bCanCancelIntoAttack = false;
		}
		else if (Elapsed < Config.CancelWindowStart) // 0.28s -> 0.35s
		{
			CurrentPhase = EPADashPhase::Recovery;
			bInvulnerable = false; // Mất I-Frame
			bCanCancelIntoAttack = false;
		}
		else if (Elapsed < Config.TotalDuration) // 0.35s -> 0.45s
		{
			CurrentPhase = EPADashPhase::AttackCancelable;
			bInvulnerable = false;
			bCanCancelIntoAttack = true; // Mở cửa sổ cancel sang Dash Attack
		}
		else // >= 0.45s
		{
			CompleteDash();
			return true;
		}

		return false;
	}

	void CompleteDash()
	{
		bIsDashing = false;
		CurrentPhase = EPADashPhase::Completed;
		bInvulnerable = false;
		bCanCancelIntoAttack = false;
		bCanWalkOffLedges = true; // Hồi phục khả năng rơi mép vực bình thường
		CurrentVelocity = Config.EndVelocity;
	}

	/**
	 * Hủy phục hồi để chuyển sang đòn Dash Attack.
	 * Chỉ thành công nếu đang trong pha AttackCancelable (t >= 0.35s).
	 */
	bool TryCancelIntoAttack()
	{
		if (bIsDashing && bCanCancelIntoAttack)
		{
			CompleteDash();
			return true;
		}
		return false;
	}

	// ===========================================================
	// Hit Processing & Perfect Dodge
	// ===========================================================

	/**
	 * Xử lý khi bị đòn đánh quét trúng trong lúc lướt.
	 */
	FPAIncomingHitResult ProcessIncomingHit(float RawDamage)
	{
		FPAIncomingHitResult Result;

		if (!bIsDashing)
		{
			Result.bAvoidedWithIFrame = false;
			Result.DamageTaken = RawDamage;
			return Result;
		}

		// Nếu đang trong thời gian I-Frame (0.00s -> 0.28s)
		if (bInvulnerable)
		{
			Result.bAvoidedWithIFrame = true;
			Result.DamageTaken = 0.0f;

			// Kiểm tra cửa sổ vàng Perfect Dodge (0.05s -> 0.15s)
			if (Elapsed >= Config.SweetSpotStart && Elapsed <= Config.SweetSpotEnd)
			{
				Result.bTriggeredPerfectDodge = true;
				Result.StaminaRefunded = Config.StaminaRefund; // +15 Stamina
				Result.HitstopDuration = Config.HitstopDuration; // 0.08s Hitstop

				bPerfectDodgeTriggered = true;
				PerfectDodgeBuffRemaining = Config.PerfectDodgeBuffDuration;
				HitstopRemaining = Config.HitstopDuration;
			}
		}
		else
		{
			// Đã hết I-Frame (t > 0.28s) -> Nhận sát thương bình thường
			Result.bAvoidedWithIFrame = false;
			Result.DamageTaken = RawDamage;
		}

		return Result;
	}

	// ===========================================================
	// Math Helpers
	// ===========================================================

	/**
	 * Đường cong vận tốc phi tuyến tính theo GDD:
	 * t in [0.00, 0.20]: 1200 cm/s
	 * t in [0.20, 0.45]: 550 + 650 * (1 - (t - 0.20)/0.25)^2
	 */
	float ComputeVelocity(float t) const
	{
		if (t <= 0.20f)
		{
			return Config.PeakVelocity;
		}

		if (t >= Config.TotalDuration)
		{
			return Config.EndVelocity;
		}

		const float NormT = FMath::Clamp((t - 0.20f) / 0.25f, 0.0f, 1.0f);
		const float Factor = 1.0f - NormT;
		return Config.EndVelocity + (Config.PeakVelocity - Config.EndVelocity) * (Factor * Factor);
	}
};
