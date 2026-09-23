// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Combat/PAStaggerTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAStaggerTests
 *
 * Kiểm thử tự động cho Story stgr-001 (Posture Stagger, Kneel Window & True Damage Execution):
 *  - AC-1: Posture Break Trigger & Kneel State (100% Posture -> quỳ gối 3.0s, Socket_Execution sáng).
 *  - AC-2: 25% True Damage Execution (ở cự ly <= 250cm, trừ đúng 25% Max HP, cấp 1.2s I-frame, 2.0s PostureImmune).
 *  - AC-3: Posture Decay Logic (Sau 4.0s không bị đánh -> suy giảm 20/s; bị đánh reset delay).
 *  - AC-4: Missed Window Recovery Shockwave (Quá 3.0s không ai kết liễu -> phát shockwave, hoàn 50% Posture).
 *
 * Kiểm thử chạy trên FPAStaggerModel thuần túy (không phụ thuộc UMG/GAS runtime).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAStaggerTests,
	"ProjectAscendant.Combat.StaggerExecution",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
	constexpr float kTolerance = 0.05f;

	void AdvanceStaggerTime(FPAStaggerModel& Model, float Duration, float StepSize = 0.016f)
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

bool FPAStaggerTests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: Posture Break Trigger & Kneel State
	// ===========================================================
	{
		FPAStaggerModel Model;
		Model.Initialize(800.0f);

		TestEqual(TEXT("AC1: Initial state is Normal"), Model.State, EPAStaggerState::Normal);
		TestNearlyEqual(TEXT("AC1: Initial Posture is 0"), Model.CurrentPosture, 0.0f, kTolerance);
		TestFalse(TEXT("AC1: Not staggered initially"), Model.IsStaggered());
		TestFalse(TEXT("AC1: Socket Execution not active"), Model.bSocketExecutionActive);

		// Tích lũy 500 Posture (chưa vỡ thế)
		bool bBroken = Model.ApplyPostureDamage(500.0f);
		TestFalse(TEXT("AC1: 500/800 does not break Posture"), bBroken);
		TestEqual(TEXT("AC1: Still Normal state"), Model.State, EPAStaggerState::Normal);
		TestNearlyEqual(TEXT("AC1: Posture is 500"), Model.CurrentPosture, 500.0f, kTolerance);

		// Đánh thêm 300 Posture (tổng 800/800 = 100%) -> Vỡ thế
		bBroken = Model.ApplyPostureDamage(300.0f);
		TestTrue(TEXT("AC1: 800/800 triggers Posture break"), bBroken);
		TestEqual(TEXT("AC1: State transitions to StaggeredKneel"), Model.State, EPAStaggerState::StaggeredKneel);
		TestTrue(TEXT("AC1: IsStaggered returns true"), Model.IsStaggered());
		TestTrue(TEXT("AC1: Socket Execution is now active"), Model.bSocketExecutionActive);
		TestNearlyEqual(TEXT("AC1: Stagger time remaining is 3.0s"), Model.StaggerTimeRemaining, 3.0f, kTolerance);
	}

	// ===========================================================
	// AC-2: 25% True Damage Execution
	// ===========================================================
	{
		FPAStaggerModel Model;
		Model.Initialize(800.0f);
		Model.ApplyPostureDamage(800.0f); // Làm vỡ thế

		const float BossMaxHP = 10000.0f;

		// Thử kết liễu từ quá xa (300cm > 250cm) -> Thất bại
		FPAExecutionResult FailResult = Model.TryExecute(300.0f, BossMaxHP);
		TestFalse(TEXT("AC2: Execution fails when beyond 250cm"), FailResult.bSuccess);
		TestEqual(TEXT("AC2: Still in StaggeredKneel after failed attempt"), Model.State, EPAStaggerState::StaggeredKneel);

		// Người chơi tiếp cận cự ly 150cm (<= 250cm) -> Thành công
		FPAExecutionResult SuccessResult = Model.TryExecute(150.0f, BossMaxHP);
		TestTrue(TEXT("AC2: Execution succeeds at 150cm"), SuccessResult.bSuccess);
		TestNearlyEqual(TEXT("AC2: Deals exactly 25% True Damage (2500 HP)"), SuccessResult.DamageDealt, 2500.0f, kTolerance);
		TestNearlyEqual(TEXT("AC2: Grants 1.20s I-Frame"), SuccessResult.PlayerInvulnDuration, 1.20f, kTolerance);

		// Sau khi kết liễu: Boss chuyển sang PostureImmune 2.0s
		TestEqual(TEXT("AC2: Boss enters PostureImmune state"), Model.State, EPAStaggerState::PostureImmune);
		TestTrue(TEXT("AC2: IsPostureImmune returns true"), Model.IsPostureImmune());
		TestFalse(TEXT("AC2: Socket Execution turned off"), Model.bSocketExecutionActive);
		TestNearlyEqual(TEXT("AC2: Posture reset to 0"), Model.CurrentPosture, 0.0f, kTolerance);
		TestNearlyEqual(TEXT("AC2: Immunity time is 2.0s"), Model.ImmunityTimeRemaining, 2.0f, kTolerance);

		// Trong thời gian miễn nhiễm -> Đòn đánh Posture bị bỏ qua
		bool bIgnored = Model.ApplyPostureDamage(100.0f);
		TestFalse(TEXT("AC2: Posture damage ignored during immunity"), bIgnored);
		TestNearlyEqual(TEXT("AC2: Posture remains 0 during immunity"), Model.CurrentPosture, 0.0f, kTolerance);

		// Hết 2.0s miễn nhiễm -> Trở về trạng thái Normal
		AdvanceStaggerTime(Model, 2.05f);
		TestEqual(TEXT("AC2: Returns to Normal state after immunity expires"), Model.State, EPAStaggerState::Normal);
		TestFalse(TEXT("AC2: PostureImmune is now false"), Model.IsPostureImmune());
	}

	// ===========================================================
	// AC-3: Posture Decay Logic
	// ===========================================================
	{
		FPAStaggerModel Model;
		Model.Initialize(800.0f);
		Model.ApplyPostureDamage(400.0f); // 50% Posture

		// Tiến thời gian 3.90s (< 4.0s delay) -> Posture không đổi (vẫn 400)
		AdvanceStaggerTime(Model, 3.90f);
		TestNearlyEqual(TEXT("AC3: Posture remains 400 before 4.0s delay"), Model.CurrentPosture, 400.0f, kTolerance);

		// Tiến thêm 0.50s (tổng 4.40s, vượt quá 4.0s được 0.40s)
		// Lượng suy giảm: 0.40s * 20.0/s = 8.0 điểm -> Còn lại 392.0 điểm
		AdvanceStaggerTime(Model, 0.50f);
		TestNearlyEqual(TEXT("AC3: Posture decays at 20/s after delay (~392)"), Model.CurrentPosture, 392.0f, 1.0f);

		// Bị đánh 1 đòn (nhận sát thương) -> Reset delay về 0
		Model.OnDamageReceived();
		TestNearlyEqual(TEXT("AC3: TimeSinceLastDamage reset to 0"), Model.TimeSinceLastDamage, 0.0f, kTolerance);

		// Tiến tiếp 2.0s sau đòn đánh -> Chưa đủ 4.0s -> Posture không suy giảm tiếp
		const float PostureBefore = Model.CurrentPosture;
		AdvanceStaggerTime(Model, 2.0f);
		TestNearlyEqual(TEXT("AC3: Posture frozen during new 4s delay"), Model.CurrentPosture, PostureBefore, kTolerance);
	}

	// ===========================================================
	// AC-4: Missed Window Recovery Shockwave
	// ===========================================================
	{
		FPAStaggerModel Model;
		Model.Initialize(800.0f);
		Model.ApplyPostureDamage(800.0f); // Làm vỡ thế

		TestTrue(TEXT("AC4: Boss is staggered"), Model.IsStaggered());

		// Chờ 2.90s (< 3.0s) -> Vẫn quỳ
		AdvanceStaggerTime(Model, 2.90f);
		TestTrue(TEXT("AC4: Boss still kneeling at 2.90s"), Model.IsStaggered());

		// Chờ thêm 0.15s (tổng 3.05s > 3.0s) -> Hết cửa sổ
		FPAExecutionResult Result = Model.Update(0.15f);
		TestTrue(TEXT("AC4: Recovery shockwave triggered upon window expiry"), Result.bShockwaveTriggered);
		TestFalse(TEXT("AC4: No longer staggered"), Model.IsStaggered());

		// Hoàn lại 50% Posture (400/800)
		TestNearlyEqual(TEXT("AC4: Refunds exactly 50% Posture (400)"), Model.CurrentPosture, 400.0f, kTolerance);

		// Boss vào trạng thái miễn nhiễm 2.0s
		TestEqual(TEXT("AC4: Boss enters PostureImmune after missed recovery"), Model.State, EPAStaggerState::PostureImmune);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
