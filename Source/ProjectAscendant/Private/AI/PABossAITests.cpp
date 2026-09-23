// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "AI/PABossAITypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPABossAITests
 *
 * Kiểm thử tự động cho Story boss-001 (Prototype Boss AI & 4-Phase Telegraph System):
 *  - AC-1: 4-Phase Attack Timing Lifecycle (Telegraph, Flash Cue 0.10s, Hitbox, Recovery).
 *  - AC-2: Moveset & Action Selection Score (EQS scoring theo góc và khoảng cách).
 *  - AC-3: 3-Phase Health Progression (100% -> Normal, 75% -> Combos, <25% -> Enraged 480cm/s).
 *  - AC-4: Wall Crash Stun (Húc tường choáng 1.80s).
 *
 * Tất cả test chạy trên FPABossAIModel thuần túy (không phụ thuộc World/BT).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPABossAITests,
	"ProjectAscendant.AI.BossAITelegraphs",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
	constexpr float kTolerance = 0.02f;

	void AdvanceModelTime(FPABossAIModel& Model, float Duration, float StepSize = 0.016f)
	{
		float Elapsed = 0.0f;
		while (Elapsed < Duration)
		{
			const float Dt = FMath::Min(StepSize, Duration - Elapsed);
			Model.Update(Dt);
			Elapsed += Dt;
		}
	}
}

// ---------------------------------------------------------------------------
// RunTest
// ---------------------------------------------------------------------------

bool FPABossAITests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: 4-Phase Attack Timing Lifecycle
	// ===========================================================
	{
		FPABossAIModel Model;
		Model.Initialize(10000.0f);

		// Chọn Cleave Strike: Telegraph 0.50s, FlashCue 0.10s, Hitbox 0.20s, Recovery 0.70s (+0.10s ở P1 = 0.80s)
		bool bStarted = Model.StartAttack(EPABossAttackType::CleaveStrike);
		TestTrue(TEXT("AC1: Cleave Strike started"), bStarted);
		TestEqual(TEXT("AC1: Starts in Telegraph phase"), Model.CurrentPhase, EPABossAttackPhase::Telegraph);
		TestFalse(TEXT("AC1: Flash Cue initially false"), Model.bFlashCueActive);
		TestFalse(TEXT("AC1: Hitbox initially false"), Model.bHitboxActive);

		// Tiến 0.20s (40% tiến độ Telegraph)
		AdvanceModelTime(Model, 0.20f);
		TestEqual(TEXT("AC1: At 0.20s still in Telegraph"), Model.CurrentPhase, EPABossAttackPhase::Telegraph);
		TestNearlyEqual(TEXT("AC1: Telegraph ratio ~ 0.40"), Model.TelegraphFillRatio, 0.40f, kTolerance);
		TestFalse(TEXT("AC1: Flash cue not active at 0.20s"), Model.bFlashCueActive);

		// Tiến tiếp đến 0.42s (còn 0.08s < 0.10s cuối, FillRatio = 0.84 -> 0.85+)
		AdvanceModelTime(Model, 0.22f);
		TestEqual(TEXT("AC1: At 0.42s in Flash Cue phase"), Model.CurrentPhase, EPABossAttackPhase::FlashCue);
		TestTrue(TEXT("AC1: Flash Cue active in final 0.10s window"), Model.bFlashCueActive);
		TestFalse(TEXT("AC1: Hitbox not active yet during Flash Cue"), Model.bHitboxActive);

		// Tiến tiếp 0.09s (tổng 0.51s > 0.50s Telegraph) -> Chuyển sang Active Hitbox
		AdvanceModelTime(Model, 0.09f);
		TestEqual(TEXT("AC1: At 0.51s in Active Hitbox phase"), Model.CurrentPhase, EPABossAttackPhase::ActiveHitbox);
		TestTrue(TEXT("AC1: Hitbox active"), Model.bHitboxActive);
		TestFalse(TEXT("AC1: Flash cue turned off"), Model.bFlashCueActive);

		// Hitbox duy trì 0.20s -> tiến 0.21s -> Chuyển sang Recovery
		AdvanceModelTime(Model, 0.21f);
		TestEqual(TEXT("AC1: After Hitbox duration, in Recovery phase"), Model.CurrentPhase, EPABossAttackPhase::Recovery);
		TestFalse(TEXT("AC1: Hitbox turned off"), Model.bHitboxActive);

		// Recovery kéo dài 0.80s (0.70s + 0.10s ở P1) -> tiến 0.82s -> Quay lại Idle
		AdvanceModelTime(Model, 0.82f);
		TestEqual(TEXT("AC1: After Recovery, returned to Idle"), Model.CurrentPhase, EPABossAttackPhase::Idle);
		TestEqual(TEXT("AC1: Attack cleared to None"), Model.CurrentAttack, EPABossAttackType::None);
	}

	// ===========================================================
	// AC-2: Moveset & Action Selection Score (EQS Logic)
	// ===========================================================
	{
		FPABossAIModel Model;
		Model.Initialize(10000.0f);

		// QA Test 2a: Người chơi ở sau lưng (>120°) và cự ly gần (200cm <= 300cm)
		// -> Tail Sweep phải đạt điểm cao nhất (95.0f)
		EPABossAttackType BestBehind = Model.SelectBestAction(200.0f, 150.0f);
		TestEqual(TEXT("AC2: Tail Sweep prioritized when target is behind"), BestBehind, EPABossAttackType::IronTailSweep);

		// QA Test 2b: Người chơi ở xa (> 500cm, góc 0°)
		// -> Horn Charge phải đạt điểm cao nhất (85.0f)
		EPABossAttackType BestFar = Model.SelectBestAction(600.0f, 0.0f);
		TestEqual(TEXT("AC2: Horn Charge prioritized at distance >500cm"), BestFar, EPABossAttackType::IronHornCharge);

		// QA Test 2c: Người chơi ở cự ly cận chiến phía trước (250cm, góc 20°)
		// -> Cleave Strike ưu tiên phía trước
		EPABossAttackType BestFrontMelee = Model.SelectBestAction(250.0f, 20.0f);
		TestEqual(TEXT("AC2: Cleave Strike prioritized at front melee range"), BestFrontMelee, EPABossAttackType::CleaveStrike);

		// QA Test 2d: Khi sừng bị gãy -> Horn Charge bị loại bỏ khỏi danh sách chọn
		Model.BreakHorn();
		TestFalse(TEXT("AC2: Horn broken flag set"), Model.bCanUseHornCharge);
		EPABossAttackType BestFarAfterBrokenHorn = Model.SelectBestAction(600.0f, 0.0f);
		TestNotEqual(TEXT("AC2: Cannot select Horn Charge when horn is broken"), BestFarAfterBrokenHorn, EPABossAttackType::IronHornCharge);

		// QA Test 2e: Khi đuôi bị đứt -> Tail Sweep bị loại bỏ
		Model.BreakTail();
		TestFalse(TEXT("AC2: Tail broken flag set"), Model.bCanUseTailSweep);
		EPABossAttackType BestBehindAfterBrokenTail = Model.SelectBestAction(200.0f, 150.0f);
		TestNotEqual(TEXT("AC2: Cannot select Tail Sweep when tail is broken"), BestBehindAfterBrokenTail, EPABossAttackType::IronTailSweep);
	}

	// ===========================================================
	// AC-3: 3-Phase Health Progression
	// ===========================================================
	{
		FPABossAIModel Model;
		Model.Initialize(10000.0f);

		// Ban đầu: 100% HP -> Phase 1 Normal, MoveSpeed = 420
		TestEqual(TEXT("AC3: Initial Combat Phase is Phase 1"), Model.CombatPhase, EPABossCombatPhase::Phase1_Normal);
		TestNearlyEqual(TEXT("AC3: Initial Move Speed = 420"), Model.CurrentMoveSpeed, 420.0f, kTolerance);

		// Trừ máu xuống 70% (7000 HP) -> Chuyển sang Phase 2 Combos
		Model.SetHealth(7000.0f);
		TestEqual(TEXT("AC3: At 70% HP, in Phase 2 Combos"), Model.CombatPhase, EPABossCombatPhase::Phase2_Combos);
		TestNearlyEqual(TEXT("AC3: Move Speed still 420 in Phase 2"), Model.CurrentMoveSpeed, 420.0f, kTolerance);

		// Trừ máu xuống 20% (2000 HP) -> Chuyển sang Phase 3 Enraged
		Model.SetHealth(2000.0f);
		TestEqual(TEXT("AC3: At 20% HP, in Phase 3 Enraged"), Model.CombatPhase, EPABossCombatPhase::Phase3_Enraged);
		TestNearlyEqual(TEXT("AC3: Move Speed boosted to 480 in Phase 3"), Model.CurrentMoveSpeed, 480.0f, kTolerance);

		// Kiểm tra mở khóa Earthquake Stomp chỉ trong Phase 3
		const FPABossAttackData* Stomp = Model.FindAttack(EPABossAttackType::EarthquakeStomp);
		TestNotNull(TEXT("AC3: Earthquake Stomp exists"), Stomp);
		TestEqual(TEXT("AC3: Earthquake Stomp requires Phase 3"), Stomp->MinCombatPhase, EPABossCombatPhase::Phase3_Enraged);

		// Ở Phase 3, điểm số của Earthquake Stomp hợp lệ và được ưu tiên
		float StompScore = Model.EvaluateActionScore(*Stomp, 300.0f, 0.0f);
		TestEqual(TEXT("AC3: Earthquake Stomp score = 80 in Phase 3"), StompScore, 80.0f);

		// Nếu đổi lại HP về 90% (Phase 1) -> Điểm Earthquake Stomp bị cấm (-1.0f)
		Model.SetHealth(9000.0f);
		float StompScoreInP1 = Model.EvaluateActionScore(*Stomp, 300.0f, 0.0f);
		TestEqual(TEXT("AC3: Earthquake Stomp disabled in Phase 1"), StompScoreInP1, -1.0f);
	}

	// ===========================================================
	// AC-4: Wall Crash Stun
	// ===========================================================
	{
		FPABossAIModel Model;
		Model.Initialize(10000.0f);

		// Bắt đầu Horn Charge
		bool bStarted = Model.StartAttack(EPABossAttackType::IronHornCharge);
		TestTrue(TEXT("AC4: Horn Charge started"), bStarted);

		// Đang trong đòn Charge -> Bị va chạm vào tường
		Model.TriggerWallCrash();
		TestEqual(TEXT("AC4: Immediately enters WallStunned phase"), Model.CurrentPhase, EPABossAttackPhase::WallStunned);
		TestTrue(TEXT("AC4: IsWallStunned flag is true"), Model.bIsWallStunned);
		TestFalse(TEXT("AC4: Hitbox cancelled on wall crash"), Model.bHitboxActive);
		TestFalse(TEXT("AC4: Flash cue cancelled on wall crash"), Model.bFlashCueActive);

		// Giữ nguyên trạng thái choáng trong 1.70s (< 1.80s)
		AdvanceModelTime(Model, 1.70f);
		TestEqual(TEXT("AC4: Still WallStunned at 1.70s"), Model.CurrentPhase, EPABossAttackPhase::WallStunned);
		TestTrue(TEXT("AC4: Still stunned"), Model.bIsWallStunned);

		// Sau 1.80s -> Hết choáng và hồi phục về Idle
		AdvanceModelTime(Model, 0.15f); // Tổng 1.85s > 1.80s
		TestEqual(TEXT("AC4: Recovers to Idle after 1.80s stun"), Model.CurrentPhase, EPABossAttackPhase::Idle);
		TestFalse(TEXT("AC4: IsWallStunned is false"), Model.bIsWallStunned);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
