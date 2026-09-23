// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PABossHUDTypes.generated.h"

/**
 * EPABossPostureState
 *
 * Trạng thái thanh Thế Đứng (Posture Bar) của Boss theo GDD combat-hud.md:
 * - Accumulating: Thanh posture đang tích lũy sát thương phá thế (0% → 100%).
 * - BrokenFlashing: Posture đạt 100%, thanh nhấp nháy đỏ liên tục 4.0 Hz trong 3.0s (State.Staggered).
 * - Recovered: Boss hồi phục sau stagger, posture reset về 0%.
 */
UENUM(BlueprintType)
enum class EPABossPostureState : uint8
{
	Accumulating    = 0 UMETA(DisplayName = "Accumulating (Building Posture)"),
	BrokenFlashing  = 1 UMETA(DisplayName = "Broken Flashing (4.0 Hz Red)"),
	Recovered       = 2 UMETA(DisplayName = "Recovered (Reset)")
};

/**
 * FPABossPartStatus
 *
 * Trạng thái bộ phận có thể gãy của Boss (Sừng, Đuôi).
 * Khi bị đánh gãy, hiển thị gạch chéo đỏ đanh thép trên icon.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPABossPartStatus
{
	GENERATED_BODY()

	/** Tên bộ phận (VD: "Horn", "Tail") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossHUD|Part")
	FName PartId;

	/** Đã bị đánh gãy? (Hiển thị gạch chéo đỏ khi true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossHUD|Part")
	bool bIsBroken = false;

	FPABossPartStatus() : PartId(NAME_None), bIsBroken(false) {}
	FPABossPartStatus(FName InPartId) : PartId(InPartId), bIsBroken(false) {}
};

/**
 * FPABossHUDConfig
 *
 * Tham số cân chỉnh giao diện Boss theo GDD combat-hud.md §Tuning Knobs.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPABossHUDConfig
{
	GENERATED_BODY()

	/** Mốc chuyển pha chiến đấu thứ nhất (75% HP) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossHUD|Config")
	float PhaseNotch1 = 0.75f;

	/** Mốc chuyển pha chiến đấu thứ hai (25% HP) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossHUD|Config")
	float PhaseNotch2 = 0.25f;

	/** Thời gian cửa sổ stagger (Boss choáng, mặc định 3.0s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossHUD|Config")
	float StaggerWindowDuration = 3.0f;

	/** Tần số nhấp nháy đỏ thanh Posture khi vỡ thế (mặc định 4.0 Hz) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossHUD|Config")
	float StaggerBlinkRate = 4.0f;
};

/**
 * FPABossHUDModel
 *
 * Data model tính toán thuần túy cho HUD Boss Encounter.
 * Cho phép kiểm thử tự động 100% không cần Slate/UMG Viewport.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPABossHUDModel
{
	GENERATED_BODY()

	// ---- Danh tính Boss ----
	UPROPERTY(BlueprintReadOnly, Category = "BossHUD|Model")
	FString BossName;

	UPROPERTY(BlueprintReadOnly, Category = "BossHUD|Model")
	int32 BossLevel = 1;

	// ---- Thanh Máu ----
	UPROPERTY(BlueprintReadOnly, Category = "BossHUD|Model")
	float CurrentHP = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "BossHUD|Model")
	float MaxHP = 100.0f;

	// ---- Thanh Posture ----
	UPROPERTY(BlueprintReadOnly, Category = "BossHUD|Model")
	float CurrentPosture = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "BossHUD|Model")
	float MaxPosture = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "BossHUD|Model")
	EPABossPostureState PostureState = EPABossPostureState::Accumulating;

	/** Bộ đếm thời gian stagger (3.0s → 0.0s) */
	UPROPERTY(BlueprintReadOnly, Category = "BossHUD|Model")
	float StaggerTimer = 0.0f;

	/** Bộ đếm nháy dùng cho 4.0 Hz flash (toggle mỗi nửa chu kỳ) */
	UPROPERTY(BlueprintReadOnly, Category = "BossHUD|Model")
	float BlinkAccumulator = 0.0f;

	/** Trạng thái bật/tắt hiển thị hiện tại của thanh posture (nhấp nháy) */
	UPROPERTY(BlueprintReadOnly, Category = "BossHUD|Model")
	bool bBlinkVisible = true;

	// ---- Bộ phận ----
	UPROPERTY(BlueprintReadOnly, Category = "BossHUD|Model")
	TArray<FPABossPartStatus> Parts;

	// ---- Execution Reticle ----
	UPROPERTY(BlueprintReadOnly, Category = "BossHUD|Model")
	bool bExecutionReticleVisible = false;

	UPROPERTY(BlueprintReadOnly, Category = "BossHUD|Model")
	FVector2D ReticleScreenPosition = FVector2D::ZeroVector;

	// ---- Config ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossHUD|Model")
	FPABossHUDConfig Config;

	// ===========================================================
	// Khởi tạo
	// ===========================================================

	FPABossHUDModel()
	{
		BossName = TEXT("");
		BossLevel = 1;
		CurrentHP = 100.0f;
		MaxHP = 100.0f;
		CurrentPosture = 0.0f;
		MaxPosture = 100.0f;
		PostureState = EPABossPostureState::Accumulating;
		StaggerTimer = 0.0f;
		BlinkAccumulator = 0.0f;
		bBlinkVisible = true;
		bExecutionReticleVisible = false;
		ReticleScreenPosition = FVector2D::ZeroVector;
	}

	// ===========================================================
	// Getters
	// ===========================================================

	float GetHPPercent() const
	{
		return MaxHP > 0.0f ? FMath::Clamp(CurrentHP / MaxHP, 0.0f, 1.0f) : 0.0f;
	}

	float GetPosturePercent() const
	{
		return MaxPosture > 0.0f ? FMath::Clamp(CurrentPosture / MaxPosture, 0.0f, 1.0f) : 0.0f;
	}

	/**
	 * Trả về danh sách phần trăm vạch khấc chia pha trên thanh máu Boss.
	 * GDD: 75% và 25%.
	 */
	void GetPhaseNotchPercents(TArray<float>& OutNotches) const
	{
		OutNotches.Reset();
		OutNotches.Add(Config.PhaseNotch1);
		OutNotches.Add(Config.PhaseNotch2);
	}

	/**
	 * Xác định pha chiến đấu hiện tại dựa trên HP%:
	 * Phase 1: HP > 75%, Phase 2: 25% < HP ≤ 75%, Phase 3: HP ≤ 25%.
	 */
	int32 GetCurrentPhase() const
	{
		const float HPPercent = GetHPPercent();
		if (HPPercent > Config.PhaseNotch1) return 1;
		if (HPPercent > Config.PhaseNotch2) return 2;
		return 3;
	}

	bool IsStaggerFlashing() const
	{
		return PostureState == EPABossPostureState::BrokenFlashing;
	}

	bool IsPartBroken(FName PartId) const
	{
		for (const FPABossPartStatus& Part : Parts)
		{
			if (Part.PartId == PartId)
			{
				return Part.bIsBroken;
			}
		}
		return false;
	}

	// ===========================================================
	// Setters / Mutations
	// ===========================================================

	void SetBossIdentity(const FString& InName, int32 InLevel)
	{
		BossName = InName;
		BossLevel = FMath::Max(1, InLevel);
	}

	void SetHP(float InCurrent, float InMax)
	{
		MaxHP = FMath::Max(1.0f, InMax);
		CurrentHP = FMath::Clamp(InCurrent, 0.0f, MaxHP);
	}

	void SetPosture(float InCurrent, float InMax)
	{
		MaxPosture = FMath::Max(1.0f, InMax);
		CurrentPosture = FMath::Clamp(InCurrent, 0.0f, MaxPosture);
	}

	/**
	 * Kích hoạt trạng thái Stagger (Posture đạt 100%).
	 * Bắt đầu cửa sổ nhấp nháy đỏ 3.0s và hiện Execution Reticle.
	 */
	void TriggerPostureBroken()
	{
		PostureState = EPABossPostureState::BrokenFlashing;
		StaggerTimer = Config.StaggerWindowDuration;
		BlinkAccumulator = 0.0f;
		bBlinkVisible = true;
		bExecutionReticleVisible = true;
	}

	/**
	 * Đánh dấu bộ phận Boss bị gãy (Part Break).
	 * Nếu PartId chưa tồn tại trong danh sách, sẽ được thêm mới và đánh gãy luôn.
	 */
	void BreakPart(FName PartId)
	{
		for (FPABossPartStatus& Part : Parts)
		{
			if (Part.PartId == PartId)
			{
				Part.bIsBroken = true;
				return;
			}
		}
		// Nếu chưa có, thêm mới với trạng thái gãy
		FPABossPartStatus NewPart(PartId);
		NewPart.bIsBroken = true;
		Parts.Add(NewPart);
	}

	/**
	 * Đăng ký danh sách bộ phận Boss (trước khi trận đấu bắt đầu).
	 */
	void RegisterParts(const TArray<FName>& PartIds)
	{
		Parts.Reset();
		for (const FName& Id : PartIds)
		{
			Parts.Add(FPABossPartStatus(Id));
		}
	}

	/**
	 * Cập nhật vị trí Execution Reticle trên màn hình (World-to-Screen projection).
	 * Chỉ hiển thị khi Boss đang Staggered.
	 */
	void UpdateReticleScreenPosition(const FVector2D& InScreenPos)
	{
		ReticleScreenPosition = InScreenPos;
	}

	// ===========================================================
	// Tick Update
	// ===========================================================

	void Update(float DeltaTime)
	{
		if (PostureState == EPABossPostureState::BrokenFlashing)
		{
			// Đếm ngược cửa sổ stagger
			StaggerTimer = FMath::Max(0.0f, StaggerTimer - DeltaTime);

			// Nhấp nháy 4.0 Hz = chu kỳ 0.25s → toggle mỗi 0.125s (nửa chu kỳ)
			const float HalfPeriod = 1.0f / (Config.StaggerBlinkRate * 2.0f);
			BlinkAccumulator += DeltaTime;
			while (BlinkAccumulator >= HalfPeriod)
			{
				BlinkAccumulator -= HalfPeriod;
				bBlinkVisible = !bBlinkVisible;
			}

			// Hết thời gian stagger → hồi phục
			if (StaggerTimer <= 0.0f)
			{
				PostureState = EPABossPostureState::Recovered;
				bBlinkVisible = true;
				bExecutionReticleVisible = false;
				BlinkAccumulator = 0.0f;
			}
		}
	}
};
