// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PABossAITypes.generated.h"

/**
 * EPABossAttackPhase
 *
 * 4 pha chuẩn hóa của mọi đòn tấn công của Trùm theo GDD boss-ai.md §Telegraph System:
 * - Idle: Trùm đang quan sát, chọn đòn (EQS) hoặc di chuyển tiếp cận.
 * - Telegraph: Vùng báo động (Ground Decal) lấp đầy màu đỏ từ 0% -> 100%.
 * - FlashCue: 0.10s cuối của Telegraph (FillRatio >= 0.85), phát tia chớp ánh mắt và âm Ping!
 * - ActiveHitbox: Vũ khí/thân thể quét qua vùng decal gây sát thương thực tế.
 * - Recovery: Trùm hồi thế, mở ra cửa sổ cho người chơi trừng phạt.
 * - WallStunned: Trùm bị choáng 1.8s sau khi húc sừng vào tường.
 * - Staggered: Trùm vỡ thế Posture quỳ gối 3.0s.
 */
UENUM(BlueprintType)
enum class EPABossAttackPhase : uint8
{
	Idle          = 0 UMETA(DisplayName = "Idle / Moving"),
	Telegraph     = 1 UMETA(DisplayName = "Telegraph (Decal Fill 0%->100%)"),
	FlashCue      = 2 UMETA(DisplayName = "Flash Cue (Last 0.10s - Ping!)"),
	ActiveHitbox  = 3 UMETA(DisplayName = "Active Hitbox (Damage Window)"),
	Recovery      = 4 UMETA(DisplayName = "Recovery (Punish Window)"),
	WallStunned   = 5 UMETA(DisplayName = "Wall Stunned (1.8s Crash)"),
	Staggered     = 6 UMETA(DisplayName = "Staggered Kneeling (3.0s)")
};

/**
 * EPABossAttackType
 *
 * 5 kỹ năng chiến đấu của Boss Ironclad Warlord theo GDD boss-ai.md §Moveset:
 */
UENUM(BlueprintType)
enum class EPABossAttackType : uint8
{
	None              = 0 UMETA(DisplayName = "None"),
	CleaveStrike      = 1 UMETA(DisplayName = "Cleave Strike (Cone 120°, 350cm)"),
	OverheadSmash     = 2 UMETA(DisplayName = "Overhead Smash (Rect 150x450cm)"),
	IronHornCharge    = 3 UMETA(DisplayName = "Iron Horn Charge (Rect 250x900cm)"),
	IronTailSweep     = 4 UMETA(DisplayName = "Iron Tail Sweep (Arc 160°, 300cm behind)"),
	EarthquakeStomp   = 5 UMETA(DisplayName = "Earthquake Stomp (Phase 3 Circle 500cm)")
};

/**
 * EPABossCombatPhase
 *
 * 3 giai đoạn máu của Boss theo GDD boss-ai.md §3-Phase Behavior:
 * - Phase1: 100% -> 75% HP (đòn đơn, hồi phục chậm 0.9s).
 * - Phase2: 75% -> 25% HP (combo 2-3 đòn, hồi phục 0.7s).
 * - Phase3: <25% HP Enrage (tăng tốc độ chạy 480cm/s, mở khóa Earthquake Stomp).
 */
UENUM(BlueprintType)
enum class EPABossCombatPhase : uint8
{
	None              = 0 UMETA(DisplayName = "None / Inactive"),
	Phase1_Normal     = 1 UMETA(DisplayName = "Phase 1: Normal (100%->75%)"),
	Phase2_Combos     = 2 UMETA(DisplayName = "Phase 2: Combos (75%->25%)"),
	Phase3_Enraged    = 3 UMETA(DisplayName = "Phase 3: Enraged (<25%)")
};

/**
 * FPABossAttackData
 *
 * Thông số kỹ thuật của từng đòn đánh.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPABossAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Attack")
	EPABossAttackType AttackType = EPABossAttackType::None;

	/** Sát thương cơ bản */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Attack")
	float BaseDamage = 100.0f;

	/** Thời gian báo chiêu lấp đầy Decal (giây) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Attack")
	float TelegraphDuration = 0.60f;

	/** Cửa sổ Flash Cue cuối đòn (mặc định 0.10s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Attack")
	float FlashCueDuration = 0.10f;

	/** Thời gian duy trì Hitbox gây sát thương (giây) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Attack")
	float HitboxDuration = 0.20f;

	/** Thời gian hồi đòn / trừng phạt (giây) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Attack")
	float RecoveryDuration = 0.80f;

	/** Tầm hiệu quả tối đa (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Attack")
	float EffectiveRange = 400.0f;

	/** Góc quét tối đa tính từ phía trước (độ, 180 = mọi góc) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Attack")
	float MaxAngleDegrees = 60.0f;

	/** Yêu cầu góc phía sau? (dành cho Tail Sweep) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Attack")
	bool bRequiresBehind = false;

	/** Cooldown đòn đánh (giây) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Attack")
	float Cooldown = 3.0f;

	/** Thời gian cooldown còn lại */
	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Attack")
	float RemainingCooldown = 0.0f;

	/** Giai đoạn Boss tối thiểu để mở khóa */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Attack")
	EPABossCombatPhase MinCombatPhase = EPABossCombatPhase::Phase1_Normal;

	/** Yêu cầu Sừng còn nguyên vẹn? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Attack")
	bool bRequiresHorn = false;

	/** Yêu cầu Đuôi còn nguyên vẹn? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Attack")
	bool bRequiresTail = false;

	FPABossAttackData() = default;
};

/**
 * FPABossAIConfig
 *
 * Cấu hình chung cho Boss AI Warlord.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPABossAIConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Config")
	float BaseMoveSpeed = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Config")
	float EnragedMoveSpeed = 480.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Config")
	float WallStunDuration = 1.80f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Config")
	float FlashCueRatioThreshold = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Config")
	float Phase2HealthPct = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Config")
	float Phase3HealthPct = 0.25f;
};

/**
 * FPABossAIModel
 *
 * Pure data model cho Boss AI & Telegraphs.
 * Hoàn toàn headless, kiểm thử tự động 100% không phụ thuộc World hay BehaviorTree runtime.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPABossAIModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	EPABossAttackPhase CurrentPhase = EPABossAttackPhase::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	EPABossAttackType CurrentAttack = EPABossAttackType::None;

	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	EPABossCombatPhase CombatPhase = EPABossCombatPhase::Phase1_Normal;

	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	float CurrentHealth = 10000.0f;

	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	float MaxHealth = 10000.0f;

	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	float CurrentMoveSpeed = 420.0f;

	/** Thời gian đã trôi qua trong pha hiện tại */
	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	float PhaseElapsed = 0.0f;

	/** Tỷ lệ lấp đầy Decal Telegraph (0.0 -> 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	float TelegraphFillRatio = 0.0f;

	/** Đang trong cửa sổ Flash Cue (Ping!)? */
	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	bool bFlashCueActive = false;

	/** Đang trong pha Hitbox gây sát thương? */
	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	bool bHitboxActive = false;

	/** Đang bị choáng do húc tường? */
	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	bool bIsWallStunned = false;

	/** Sừng còn nguyên vẹn? */
	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	bool bCanUseHornCharge = true;

	/** Đuôi còn nguyên vẹn? */
	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	bool bCanUseTailSweep = true;

	/** Kho đòn đánh có sẵn */
	UPROPERTY(BlueprintReadOnly, Category = "BossAI|Model")
	TArray<FPABossAttackData> Attacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossAI|Model")
	FPABossAIConfig Config;

	// ===========================================================
	// Initialization
	// ===========================================================

	FPABossAIModel()
	{
		InitializeDefaultAttacks();
	}

	void Initialize(float InMaxHP = 10000.0f)
	{
		MaxHealth = InMaxHP;
		CurrentHealth = InMaxHP;
		CombatPhase = EPABossCombatPhase::Phase1_Normal;
		CurrentMoveSpeed = Config.BaseMoveSpeed;
		CurrentPhase = EPABossAttackPhase::Idle;
		CurrentAttack = EPABossAttackType::None;
		PhaseElapsed = 0.0f;
		TelegraphFillRatio = 0.0f;
		bFlashCueActive = false;
		bHitboxActive = false;
		bIsWallStunned = false;
		bCanUseHornCharge = true;
		bCanUseTailSweep = true;
		InitializeDefaultAttacks();
	}

	void InitializeDefaultAttacks()
	{
		Attacks.Reset();

		// 1. Cleave Strike (Cone 120°, Range <= 350cm, Telegraph 0.50s, Hitbox 0.20s, Recovery 0.70s, Dmg 120)
		{
			FPABossAttackData Atk;
			Atk.AttackType = EPABossAttackType::CleaveStrike;
			Atk.BaseDamage = 120.0f;
			Atk.TelegraphDuration = 0.50f;
			Atk.FlashCueDuration = 0.10f;
			Atk.HitboxDuration = 0.20f;
			Atk.RecoveryDuration = 0.70f;
			Atk.EffectiveRange = 350.0f;
			Atk.MaxAngleDegrees = 60.0f; // Cone 120° = +-60°
			Atk.bRequiresBehind = false;
			Atk.Cooldown = 2.5f;
			Atk.MinCombatPhase = EPABossCombatPhase::Phase1_Normal;
			Attacks.Add(Atk);
		}

		// 2. Overhead Smash (Rect 150x450cm, Range <= 450cm, Telegraph 0.65s, Hitbox 0.15s, Recovery 0.80s, Dmg 220)
		{
			FPABossAttackData Atk;
			Atk.AttackType = EPABossAttackType::OverheadSmash;
			Atk.BaseDamage = 220.0f;
			Atk.TelegraphDuration = 0.65f;
			Atk.FlashCueDuration = 0.10f;
			Atk.HitboxDuration = 0.15f;
			Atk.RecoveryDuration = 0.80f;
			Atk.EffectiveRange = 450.0f;
			Atk.MaxAngleDegrees = 30.0f;
			Atk.bRequiresBehind = false;
			Atk.Cooldown = 4.0f;
			Atk.MinCombatPhase = EPABossCombatPhase::Phase1_Normal;
			Attacks.Add(Atk);
		}

		// 3. Iron Horn Charge (Rect 250x900cm, Range > 500cm, Telegraph 0.80s, Hitbox 0.25s, Recovery 0.90s, Dmg 280)
		{
			FPABossAttackData Atk;
			Atk.AttackType = EPABossAttackType::IronHornCharge;
			Atk.BaseDamage = 280.0f;
			Atk.TelegraphDuration = 0.80f;
			Atk.FlashCueDuration = 0.10f;
			Atk.HitboxDuration = 0.25f;
			Atk.RecoveryDuration = 0.90f;
			Atk.EffectiveRange = 900.0f;
			Atk.MaxAngleDegrees = 45.0f;
			Atk.bRequiresBehind = false;
			Atk.Cooldown = 6.0f;
			Atk.MinCombatPhase = EPABossCombatPhase::Phase1_Normal;
			Atk.bRequiresHorn = true;
			Attacks.Add(Atk);
		}

		// 4. Iron Tail Sweep (Arc 160°, Range <= 300cm behind >120°, Telegraph 0.45s, Hitbox 0.20s, Recovery 0.60s, Dmg 150)
		{
			FPABossAttackData Atk;
			Atk.AttackType = EPABossAttackType::IronTailSweep;
			Atk.BaseDamage = 150.0f;
			Atk.TelegraphDuration = 0.45f;
			Atk.FlashCueDuration = 0.10f;
			Atk.HitboxDuration = 0.20f;
			Atk.RecoveryDuration = 0.60f;
			Atk.EffectiveRange = 300.0f;
			Atk.MaxAngleDegrees = 180.0f;
			Atk.bRequiresBehind = true; // Yêu cầu góc sau lưng (> 120°)
			Atk.Cooldown = 3.5f;
			Atk.MinCombatPhase = EPABossCombatPhase::Phase1_Normal;
			Atk.bRequiresTail = true;
			Attacks.Add(Atk);
		}

		// 5. Earthquake Stomp (Phase 3 Enraged, Circle 500cm, Telegraph 0.70s, Hitbox 0.25s, Recovery 0.80s, Dmg 200)
		{
			FPABossAttackData Atk;
			Atk.AttackType = EPABossAttackType::EarthquakeStomp;
			Atk.BaseDamage = 200.0f;
			Atk.TelegraphDuration = 0.70f;
			Atk.FlashCueDuration = 0.10f;
			Atk.HitboxDuration = 0.25f;
			Atk.RecoveryDuration = 0.80f;
			Atk.EffectiveRange = 500.0f;
			Atk.MaxAngleDegrees = 180.0f;
			Atk.bRequiresBehind = false;
			Atk.Cooldown = 5.0f;
			Atk.MinCombatPhase = EPABossCombatPhase::Phase3_Enraged;
			Attacks.Add(Atk);
		}
	}

	// ===========================================================
	// Health & Phase Updates
	// ===========================================================

	void SetHealth(float NewHP)
	{
		CurrentHealth = FMath::Clamp(NewHP, 0.0f, MaxHealth);
		const float Ratio = MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;

		if (Ratio <= Config.Phase3HealthPct)
		{
			CombatPhase = EPABossCombatPhase::Phase3_Enraged;
			CurrentMoveSpeed = Config.EnragedMoveSpeed;
		}
		else if (Ratio <= Config.Phase2HealthPct)
		{
			CombatPhase = EPABossCombatPhase::Phase2_Combos;
			CurrentMoveSpeed = Config.BaseMoveSpeed;
		}
		else
		{
			CombatPhase = EPABossCombatPhase::Phase1_Normal;
			CurrentMoveSpeed = Config.BaseMoveSpeed;
		}
	}

	void ApplyDamage(float Amount)
	{
		SetHealth(CurrentHealth - Amount);
	}

	// ===========================================================
	// Part Destruction Flags
	// ===========================================================

	void BreakHorn()
	{
		bCanUseHornCharge = false;
	}

	void BreakTail()
	{
		bCanUseTailSweep = false;
	}

	// ===========================================================
	// Action Scoring & Selection (EQS Logic)
	// ===========================================================

	/**
	 * Chấm điểm đòn đánh theo khoảng cách và góc quay người chơi.
	 * Trả về điểm số (càng cao càng ưu tiên). Nếu không đủ điều kiện -> trả về -1.0f.
	 */
	float EvaluateActionScore(const FPABossAttackData& Atk, float Distance, float AngleDegrees) const
	{
		// Kiểm tra giai đoạn mở khóa
		if (CombatPhase < Atk.MinCombatPhase)
		{
			return -1.0f;
		}

		// Kiểm tra cooldown
		if (Atk.RemainingCooldown > 0.0f)
		{
			return -1.0f;
		}

		// Kiểm tra bộ phận còn nguyên vẹn
		if (Atk.bRequiresHorn && !bCanUseHornCharge)
		{
			return -1.0f;
		}
		if (Atk.bRequiresTail && !bCanUseTailSweep)
		{
			return -1.0f;
		}

		// Kiểm tra góc độ
		const float AbsAngle = FMath::Abs(AngleDegrees);

		// Trường hợp đặc biệt: Tail Sweep (người chơi ở sau lưng > 120° và cự ly <= 300cm)
		if (Atk.bRequiresBehind)
		{
			if (AbsAngle >= 120.0f && Distance <= Atk.EffectiveRange)
			{
				return 95.0f; // 90%+ ưu tiên phản xạ quất đuôi
			}
			return -1.0f;
		}

		if (AbsAngle > Atk.MaxAngleDegrees && Atk.MaxAngleDegrees < 180.0f)
		{
			return -1.0f;
		}

		// Kiểm tra cự ly
		if (Atk.AttackType == EPABossAttackType::IronHornCharge)
		{
			// Horn Charge chỉ ưu tiên khi ở xa (> 500cm)
			if (Distance >= 500.0f && Distance <= Atk.EffectiveRange)
			{
				return 85.0f;
			}
			return -1.0f;
		}

		if (Distance > Atk.EffectiveRange)
		{
			return -1.0f;
		}

		// Cận chiến phía trước
		if (Atk.AttackType == EPABossAttackType::CleaveStrike && Distance <= 350.0f)
		{
			return 70.0f;
		}

		if (Atk.AttackType == EPABossAttackType::OverheadSmash && Distance <= 450.0f)
		{
			return 65.0f;
		}

		if (Atk.AttackType == EPABossAttackType::EarthquakeStomp && CombatPhase == EPABossCombatPhase::Phase3_Enraged)
		{
			return 80.0f; // Rất ưu tiên khi Enraged
		}

		return 50.0f;
	}

	/**
	 * Chọn đòn đánh tốt nhất dựa trên môi trường và mục tiêu.
	 */
	EPABossAttackType SelectBestAction(float Distance, float AngleDegrees) const
	{
		EPABossAttackType BestAttack = EPABossAttackType::None;
		float HighestScore = -1.0f;

		for (const FPABossAttackData& Atk : Attacks)
		{
			const float Score = EvaluateActionScore(Atk, Distance, AngleDegrees);
			if (Score > HighestScore)
			{
				HighestScore = Score;
				BestAttack = Atk.AttackType;
			}
		}

		return BestAttack;
	}

	// ===========================================================
	// Attack Execution & Lifecycle Tick
	// ===========================================================

	bool StartAttack(EPABossAttackType AttackType)
	{
		if (CurrentPhase != EPABossAttackPhase::Idle)
		{
			return false;
		}

		const FPABossAttackData* Atk = FindAttack(AttackType);
		if (!Atk)
		{
			return false;
		}

		CurrentAttack = AttackType;
		CurrentPhase = EPABossAttackPhase::Telegraph;
		PhaseElapsed = 0.0f;
		TelegraphFillRatio = 0.0f;
		bFlashCueActive = false;
		bHitboxActive = false;
		bIsWallStunned = false;

		return true;
	}

	/**
	 * Trùm húc phải tường trong khi Charge -> Choáng 1.8s
	 */
	void TriggerWallCrash()
	{
		if (CurrentAttack == EPABossAttackType::IronHornCharge &&
			(CurrentPhase == EPABossAttackPhase::Telegraph ||
			 CurrentPhase == EPABossAttackPhase::FlashCue ||
			 CurrentPhase == EPABossAttackPhase::ActiveHitbox))
		{
			CurrentPhase = EPABossAttackPhase::WallStunned;
			PhaseElapsed = 0.0f;
			bIsWallStunned = true;
			bHitboxActive = false;
			bFlashCueActive = false;
			TelegraphFillRatio = 0.0f;
		}
	}

	/**
	 * Cập nhật vòng đời đòn đánh qua từng frame.
	 */
	void Update(float DeltaTime, float DistanceToTarget = 300.0f, float AngleToTargetDegrees = 0.0f)
	{
		// Cập nhật cooldown của tất cả các đòn đánh
		for (FPABossAttackData& Atk : Attacks)
		{
			if (Atk.RemainingCooldown > 0.0f)
			{
				Atk.RemainingCooldown = FMath::Max(0.0f, Atk.RemainingCooldown - DeltaTime);
			}
		}

		PhaseElapsed += DeltaTime;

		// Xử lý theo từng pha
		switch (CurrentPhase)
		{
		case EPABossAttackPhase::Idle:
			// Đang rảnh rỗi -> tự động tìm đòn đánh tốt nhất nếu có mục tiêu
			{
				const EPABossAttackType Best = SelectBestAction(DistanceToTarget, AngleToTargetDegrees);
				if (Best != EPABossAttackType::None)
				{
					StartAttack(Best);
				}
			}
			break;

		case EPABossAttackPhase::Telegraph:
		case EPABossAttackPhase::FlashCue:
			{
				const FPABossAttackData* Atk = FindAttack(CurrentAttack);
				if (!Atk)
				{
					CurrentPhase = EPABossAttackPhase::Idle;
					break;
				}

				// Tỷ lệ lấp đầy Decal: 0.0 -> 1.0
				TelegraphFillRatio = FMath::Clamp(PhaseElapsed / Atk->TelegraphDuration, 0.0f, 1.0f);

				// Flash Cue kích hoạt trong 0.10s cuối (hoặc FillRatio >= 0.85)
				const float TimeRemaining = Atk->TelegraphDuration - PhaseElapsed;
				if (TimeRemaining <= Atk->FlashCueDuration || TelegraphFillRatio >= Config.FlashCueRatioThreshold)
				{
					CurrentPhase = EPABossAttackPhase::FlashCue;
					bFlashCueActive = true;
				}

				// Kết thúc Telegraph -> sang Active Hitbox
				if (PhaseElapsed >= Atk->TelegraphDuration)
				{
					CurrentPhase = EPABossAttackPhase::ActiveHitbox;
					PhaseElapsed = 0.0f;
					bFlashCueActive = false;
					bHitboxActive = true;
					TelegraphFillRatio = 1.0f;
				}
			}
			break;

		case EPABossAttackPhase::ActiveHitbox:
			{
				const FPABossAttackData* Atk = FindAttack(CurrentAttack);
				if (!Atk)
				{
					CurrentPhase = EPABossAttackPhase::Idle;
					break;
				}

				if (PhaseElapsed >= Atk->HitboxDuration)
				{
					CurrentPhase = EPABossAttackPhase::Recovery;
					PhaseElapsed = 0.0f;
					bHitboxActive = false;

					// Bắt đầu hồi chiêu
					FPABossAttackData* MutableAtk = FindMutableAttack(CurrentAttack);
					if (MutableAtk)
					{
						MutableAtk->RemainingCooldown = MutableAtk->Cooldown;
					}
				}
			}
			break;

		case EPABossAttackPhase::Recovery:
			{
				const FPABossAttackData* Atk = FindAttack(CurrentAttack);
				if (!Atk)
				{
					CurrentPhase = EPABossAttackPhase::Idle;
					break;
				}

				// Hồi đòn: Phase 1 chậm hơn (+0.2s), Phase 2 nhanh hơn (-0.1s)
				float EffectiveRecovery = Atk->RecoveryDuration;
				if (CombatPhase == EPABossCombatPhase::Phase1_Normal)
				{
					EffectiveRecovery += 0.10f; // Chậm rãi ở P1
				}
				else if (CombatPhase == EPABossCombatPhase::Phase2_Combos)
				{
					EffectiveRecovery -= 0.10f; // Dồn dập ở P2
				}

				if (PhaseElapsed >= EffectiveRecovery)
				{
					CurrentPhase = EPABossAttackPhase::Idle;
					CurrentAttack = EPABossAttackType::None;
					PhaseElapsed = 0.0f;
					TelegraphFillRatio = 0.0f;
				}
			}
			break;

		case EPABossAttackPhase::WallStunned:
			{
				bIsWallStunned = true;
				if (PhaseElapsed >= Config.WallStunDuration)
				{
					bIsWallStunned = false;
					CurrentPhase = EPABossAttackPhase::Idle;
					CurrentAttack = EPABossAttackType::None;
					PhaseElapsed = 0.0f;
				}
			}
			break;

		case EPABossAttackPhase::Staggered:
			// Do Stagger System điều khiển
			break;
		}
	}

	const FPABossAttackData* FindAttack(EPABossAttackType Type) const
	{
		for (const FPABossAttackData& Atk : Attacks)
		{
			if (Atk.AttackType == Type)
			{
				return &Atk;
			}
		}
		return nullptr;
	}

	FPABossAttackData* FindMutableAttack(EPABossAttackType Type)
	{
		for (FPABossAttackData& Atk : Attacks)
		{
			if (Atk.AttackType == Type)
			{
				return &Atk;
			}
		}
		return nullptr;
	}
};
