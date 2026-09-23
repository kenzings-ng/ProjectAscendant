// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PAStaggerTypes.generated.h"

/**
 * EPAStaggerState
 *
 * Trạng thái Posture & Choáng của mục tiêu theo GDD stagger-system.md:
 * - Normal: Bình thường, tích lũy Posture khi nhận đòn, tự suy giảm sau 4s không bị đánh.
 * - StaggeredKneel: Vỡ thế ở 100% Posture, quỳ gối trong 3.0s, mở cửa sổ kết liễu tử huyệt.
 * - PostureImmune: Miễn nhiễm tích lũy Posture trong 2.0s sau khi đứng dậy để tránh lặp vô tận.
 */
UENUM(BlueprintType)
enum class EPAStaggerState : uint8
{
	Normal          = 0 UMETA(DisplayName = "Normal"),
	StaggeredKneel  = 1 UMETA(DisplayName = "Staggered Kneel (3.0s Execution Window)"),
	PostureImmune   = 2 UMETA(DisplayName = "Posture Immune (2.0s Anti-Loop Window)")
};

/**
 * FPAStaggerConfig
 *
 * Bảng tham số cân chỉnh Stagger & Execution theo GDD stagger-system.md §Tuning Knobs.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAStaggerConfig
{
	GENERATED_BODY()

	/** Thời gian quỳ gối khi vỡ thế (giây) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stagger|Config")
	float StaggerDuration = 3.0f;

	/** Thời gian chờ trước khi Posture bắt đầu suy giảm (giây) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stagger|Config")
	float PostureDecayDelay = 4.0f;

	/** Tốc độ suy giảm Posture mỗi giây (điểm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stagger|Config")
	float PostureDecayRate = 20.0f;

	/** Khoảng cách tối đa cho phép thực thi đòn kết liễu (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stagger|Config")
	float ExecutionRange = 250.0f;

	/** Tỷ lệ sát thương kết liễu theo Max HP của Trùm (25% True Damage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stagger|Config")
	float ExecutionHPPct = 0.25f;

	/** Thời gian bất tử (I-Frame) cấp cho người chơi khi kết liễu (giây) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stagger|Config")
	float ExecutionInvulnDuration = 1.20f;

	/** Thời gian miễn nhiễm Posture sau khi đứng dậy (giây) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stagger|Config")
	float PostureImmunityDuration = 2.0f;

	/** Tỷ lệ Posture hoàn lại nếu người chơi bỏ lỡ cửa sổ 3.0s (50%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stagger|Config")
	float MissedRefundPct = 0.50f;

	/** Bán kính sóng xung kích hất văng khi trùm tự hồi phục (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stagger|Config")
	float RecoveryShockwaveRadius = 300.0f;
};

/**
 * FPAExecutionResult
 *
 * Kết quả thực thi đòn kết liễu hoặc kết thúc chu kỳ quỳ gối.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAExecutionResult
{
	GENERATED_BODY()

	/** Thực hiện kết liễu thành công? */
	UPROPERTY(BlueprintReadOnly, Category = "Stagger|Result")
	bool bSuccess = false;

	/** Lượng True Damage gây ra cho Trùm */
	UPROPERTY(BlueprintReadOnly, Category = "Stagger|Result")
	float DamageDealt = 0.0f;

	/** Thời gian I-Frame người chơi nhận được */
	UPROPERTY(BlueprintReadOnly, Category = "Stagger|Result")
	float PlayerInvulnDuration = 0.0f;

	/** Trùm tự hồi phục kích hoạt sóng xung kích? */
	UPROPERTY(BlueprintReadOnly, Category = "Stagger|Result")
	bool bShockwaveTriggered = false;

	FPAExecutionResult() = default;
};

/**
 * FPAStaggerModel
 *
 * Pure data model quản lý toàn bộ logic Posture, vỡ thế, suy giảm, và kết liễu.
 * 100% headless testable.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAStaggerModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Stagger|Model")
	EPAStaggerState State = EPAStaggerState::Normal;

	UPROPERTY(BlueprintReadOnly, Category = "Stagger|Model")
	float CurrentPosture = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Stagger|Model")
	float MaxPosture = 800.0f;

	/** Thời gian kể từ lần nhận sát thương gần nhất (giây) */
	UPROPERTY(BlueprintReadOnly, Category = "Stagger|Model")
	float TimeSinceLastDamage = 0.0f;

	/** Thời gian quỳ gối còn lại (giây) */
	UPROPERTY(BlueprintReadOnly, Category = "Stagger|Model")
	float StaggerTimeRemaining = 0.0f;

	/** Thời gian miễn nhiễm Posture còn lại (giây) */
	UPROPERTY(BlueprintReadOnly, Category = "Stagger|Model")
	float ImmunityTimeRemaining = 0.0f;

	/** Điểm yếu kết liễu Socket_Execution đang sáng? */
	UPROPERTY(BlueprintReadOnly, Category = "Stagger|Model")
	bool bSocketExecutionActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stagger|Model")
	FPAStaggerConfig Config;

	// ===========================================================
	// Lifecycle & Init
	// ===========================================================

	FPAStaggerModel() = default;

	void Initialize(float InMaxPosture = 800.0f)
	{
		MaxPosture = FMath::Max(1.0f, InMaxPosture);
		CurrentPosture = 0.0f;
		State = EPAStaggerState::Normal;
		TimeSinceLastDamage = 0.0f;
		StaggerTimeRemaining = 0.0f;
		ImmunityTimeRemaining = 0.0f;
		bSocketExecutionActive = false;
	}

	// ===========================================================
	// Posture Damage & Hits
	// ===========================================================

	/**
	 * Nhận sát thương Posture.
	 * @return true nếu đòn này làm vỡ thế (kích hoạt StaggeredKneel).
	 */
	bool ApplyPostureDamage(float Amount)
	{
		// Nếu đang quỳ hoặc đang miễn nhiễm -> bỏ qua
		if (State != EPAStaggerState::Normal)
		{
			return false;
		}

		TimeSinceLastDamage = 0.0f;
		CurrentPosture = FMath::Clamp(CurrentPosture + Amount, 0.0f, MaxPosture);

		// Kiểm tra ngưỡng 100% Posture Break
		if (CurrentPosture >= MaxPosture)
		{
			TriggerStagger();
			return true;
		}

		return false;
	}

	/**
	 * Gọi khi mục tiêu nhận bất kỳ sát thương nào (để reset bộ đếm suy giảm 4.0s).
	 */
	void OnDamageReceived()
	{
		TimeSinceLastDamage = 0.0f;
	}

	// ===========================================================
	// Execution & Stagger Actions
	// ===========================================================

	void TriggerStagger()
	{
		State = EPAStaggerState::StaggeredKneel;
		StaggerTimeRemaining = Config.StaggerDuration;
		bSocketExecutionActive = true;
	}

	/**
	 * Người chơi cố gắng tung đòn kết liễu (Execution).
	 * Yêu cầu: Đang trong StaggeredKneel và khoảng cách <= ExecutionRange (250cm).
	 */
	FPAExecutionResult TryExecute(float PlayerDistance, float TargetMaxHP)
	{
		FPAExecutionResult Result;

		if (State != EPAStaggerState::StaggeredKneel)
		{
			Result.bSuccess = false;
			return Result;
		}

		if (PlayerDistance > Config.ExecutionRange)
		{
			Result.bSuccess = false;
			return Result;
		}

		// Kết liễu thành công!
		Result.bSuccess = true;
		Result.DamageDealt = TargetMaxHP * Config.ExecutionHPPct; // 25% True Damage
		Result.PlayerInvulnDuration = Config.ExecutionInvulnDuration; // 1.20s I-Frame
		Result.bShockwaveTriggered = false;

		// Thoát Stagger -> Chuyển sang Miễn nhiễm Posture 2.0s
		State = EPAStaggerState::PostureImmune;
		ImmunityTimeRemaining = Config.PostureImmunityDuration;
		CurrentPosture = 0.0f;
		bSocketExecutionActive = false;
		StaggerTimeRemaining = 0.0f;

		return Result;
	}

	// ===========================================================
	// Update Tick
	// ===========================================================

	/**
	 * Cập nhật thời gian từng frame.
	 * Trả về FPAExecutionResult nếu trùm tự đứng dậy phát shockwave.
	 */
	FPAExecutionResult Update(float DeltaTime)
	{
		FPAExecutionResult Result;

		switch (State)
		{
		case EPAStaggerState::Normal:
			{
				TimeSinceLastDamage += DeltaTime;

				// Nếu quá 4.0s không nhận sát thương -> suy giảm 20/s
				if (TimeSinceLastDamage >= Config.PostureDecayDelay && CurrentPosture > 0.0f)
				{
					CurrentPosture = FMath::Max(0.0f, CurrentPosture - Config.PostureDecayRate * DeltaTime);
				}
			}
			break;

		case EPAStaggerState::StaggeredKneel:
			{
				StaggerTimeRemaining -= DeltaTime;

				// Hết 3.0s mà không ai kết liễu -> Tự hồi phục và phát sóng xung kích
				if (StaggerTimeRemaining <= 0.0f)
				{
					State = EPAStaggerState::PostureImmune;
					ImmunityTimeRemaining = Config.PostureImmunityDuration;
					bSocketExecutionActive = false;
					StaggerTimeRemaining = 0.0f;

					// Hoàn lại 50% Posture
					CurrentPosture = MaxPosture * Config.MissedRefundPct;

					Result.bShockwaveTriggered = true;
				}
			}
			break;

		case EPAStaggerState::PostureImmune:
			{
				ImmunityTimeRemaining -= DeltaTime;

				if (ImmunityTimeRemaining <= 0.0f)
				{
					State = EPAStaggerState::Normal;
					ImmunityTimeRemaining = 0.0f;
					TimeSinceLastDamage = 0.0f;
				}
			}
			break;
		}

		return Result;
	}

	// ===========================================================
	// Query Getters
	// ===========================================================

	bool IsStaggered() const
	{
		return State == EPAStaggerState::StaggeredKneel;
	}

	bool IsPostureImmune() const
	{
		return State == EPAStaggerState::PostureImmune;
	}

	float GetPosturePercent() const
	{
		return MaxPosture > 0.0f ? FMath::Clamp(CurrentPosture / MaxPosture, 0.0f, 1.0f) : 0.0f;
	}
};
