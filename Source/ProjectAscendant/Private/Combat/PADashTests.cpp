// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Combat/PADashTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPADashTests
 *
 * Kiểm thử tự động cho Story dash-001 (Dash I-Frame, Perfect Dodge Sweet Spot & Hitstop):
 *  - AC-1: Dash Timeline & Absolute I-Frame (0.45s tổng, tiêu 25 Stamina, 0.28s bất tử tuyệt đối).
 *  - AC-2: Perfect Dodge Sweet Spot & Rewards (Cửa sổ 0.05-0.15s: +15 Stamina refund, 0.08s hitstop).
 *  - AC-3: Ledge-Fall Prevention (bCanWalkOffLedges = false trong suốt cú lướt).
 *  - AC-4: Dash Attack Cancel (Hủy phục hồi chuyển sang Dash Attack từ 0.35s).
 *
 * Kiểm thử chạy trên FPADashModel thuần túy (không phụ thuộc World runtime).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPADashTests,
	"ProjectAscendant.Combat.DashIFramePerfectDodge",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
	constexpr float kDashTolerance = 0.02f;

	void AdvanceDashTime(FPADashModel& Model, float Duration, float StepSize = 0.016f)
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

bool FPADashTests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: Dash Timeline & Absolute I-Frame
	// ===========================================================
	{
		FPADashModel Model;
		float Stamina = 100.0f;
		float StaminaRemaining = 0.0f;

		// Khởi động lướt: tốn 25 thể lực (100 -> 75)
		bool bStarted = Model.StartDash(Stamina, StaminaRemaining);
		TestTrue(TEXT("AC1: Dash started successfully"), bStarted);
		TestNearlyEqual(TEXT("AC1: Stamina deducted by 25 (100 -> 75)"), StaminaRemaining, 75.0f, kDashTolerance);
		TestTrue(TEXT("AC1: IsDashing is true"), Model.bIsDashing);
		TestTrue(TEXT("AC1: Invulnerable is true at start"), Model.bInvulnerable);
		TestEqual(TEXT("AC1: Phase is IFramePeak"), Model.CurrentPhase, EPADashPhase::IFramePeak);

		// Tại t = 0.20s (vẫn trong I-Frame < 0.28s)
		AdvanceDashTime(Model, 0.20f);
		TestTrue(TEXT("AC1: At 0.20s still invulnerable"), Model.bInvulnerable);
		TestEqual(TEXT("AC1: At 0.20s still in IFramePeak phase"), Model.CurrentPhase, EPADashPhase::IFramePeak);

		// Tại t = 0.30s (vượt quá 0.28s I-Frame)
		AdvanceDashTime(Model, 0.10f); // Tổng 0.30s
		TestFalse(TEXT("AC1: At 0.30s I-Frame expired (Invulnerable = false)"), Model.bInvulnerable);
		TestEqual(TEXT("AC1: At 0.30s in Recovery phase"), Model.CurrentPhase, EPADashPhase::Recovery);

		// Đòn đánh trúng người tại t = 0.30s (ngoài I-Frame) -> Nhận sát thương đầy đủ
		FPAIncomingHitResult LateHit = Model.ProcessIncomingHit(100.0f);
		TestFalse(TEXT("AC1: Late hit is not avoided"), LateHit.bAvoidedWithIFrame);
		TestNearlyEqual(TEXT("AC1: Full 100 damage taken"), LateHit.DamageTaken, 100.0f, kDashTolerance);
		TestFalse(TEXT("AC1: No perfect dodge on late hit"), LateHit.bTriggeredPerfectDodge);

		// Tiến hết 0.45s -> Hoàn tất lướt
		AdvanceDashTime(Model, 0.16f); // Tổng 0.46s > 0.45s
		TestFalse(TEXT("AC1: Dash completed at 0.45s+"), Model.bIsDashing);
		TestEqual(TEXT("AC1: Phase is Completed"), Model.CurrentPhase, EPADashPhase::Completed);
	}

	// ===========================================================
	// AC-2: Perfect Dodge Sweet Spot & Rewards
	// ===========================================================
	{
		// QA Test 2a: Bị đánh trúng ở t = 0.10s (trong khung 0.05s -> 0.15s)
		{
			FPADashModel Model;
			float StaminaRem = 0.0f;
			Model.StartDash(100.0f, StaminaRem);

			// Tiến thời gian đến 0.10s
			AdvanceDashTime(Model, 0.10f);

			// Đòn đánh quét trúng
			FPAIncomingHitResult SweetSpotHit = Model.ProcessIncomingHit(150.0f);
			TestTrue(TEXT("AC2: Hit avoided with I-Frame"), SweetSpotHit.bAvoidedWithIFrame);
			TestNearlyEqual(TEXT("AC2: Zero damage taken"), SweetSpotHit.DamageTaken, 0.0f, kDashTolerance);
			TestTrue(TEXT("AC2: Perfect Dodge triggered in 0.05-0.15s sweet spot"), SweetSpotHit.bTriggeredPerfectDodge);
			TestNearlyEqual(TEXT("AC2: Refunds +15 Stamina"), SweetSpotHit.StaminaRefunded, 15.0f, kDashTolerance);
			TestNearlyEqual(TEXT("AC2: Grants 0.08s Hitstop"), SweetSpotHit.HitstopDuration, 0.08f, kDashTolerance);
			TestTrue(TEXT("AC2: Model flag bPerfectDodgeTriggered is true"), Model.bPerfectDodgeTriggered);
			TestNearlyEqual(TEXT("AC2: Hitstop remaining is 0.08s"), Model.HitstopRemaining, 0.08f, kDashTolerance);
		}

		// QA Test 2b: Bị đánh trúng ở t = 0.22s (trong I-Frame < 0.28s nhưng ngoài sweet-spot > 0.15s)
		{
			FPADashModel Model;
			float StaminaRem = 0.0f;
			Model.StartDash(100.0f, StaminaRem);

			AdvanceDashTime(Model, 0.22f);

			FPAIncomingHitResult NonSweetSpotHit = Model.ProcessIncomingHit(150.0f);
			TestTrue(TEXT("AC2: Hit avoided with I-Frame"), NonSweetSpotHit.bAvoidedWithIFrame);
			TestNearlyEqual(TEXT("AC2: Zero damage taken"), NonSweetSpotHit.DamageTaken, 0.0f, kDashTolerance);
			TestFalse(TEXT("AC2: No Perfect Dodge reward outside sweet-spot (t=0.22s)"), NonSweetSpotHit.bTriggeredPerfectDodge);
			TestNearlyEqual(TEXT("AC2: Zero stamina refund"), NonSweetSpotHit.StaminaRefunded, 0.0f, kDashTolerance);
			TestNearlyEqual(TEXT("AC2: Zero hitstop"), NonSweetSpotHit.HitstopDuration, 0.0f, kDashTolerance);
		}
	}

	// ===========================================================
	// AC-3: Ledge-Fall Prevention
	// ===========================================================
	{
		FPADashModel Model;
		TestTrue(TEXT("AC3: Can walk off ledges initially"), Model.bCanWalkOffLedges);

		float StaminaRem = 0.0f;
		Model.StartDash(100.0f, StaminaRem);

		// Trong suốt cú lướt (t = 0.10s, 0.25s, 0.40s) -> bCanWalkOffLedges luôn = false
		TestFalse(TEXT("AC3: Cannot walk off ledges at start"), Model.bCanWalkOffLedges);

		AdvanceDashTime(Model, 0.25f);
		TestFalse(TEXT("AC3: Cannot walk off ledges at mid-dash (0.25s)"), Model.bCanWalkOffLedges);

		AdvanceDashTime(Model, 0.15f); // t = 0.40s
		TestFalse(TEXT("AC3: Cannot walk off ledges at cancel window (0.40s)"), Model.bCanWalkOffLedges);

		// Kết thúc lướt (t > 0.45s) -> Hồi phục bCanWalkOffLedges = true
		AdvanceDashTime(Model, 0.06f); // t = 0.46s
		TestTrue(TEXT("AC3: Restored ability to walk off ledges after dash completes"), Model.bCanWalkOffLedges);
	}

	// ===========================================================
	// AC-4: Dash Attack Cancel
	// ===========================================================
	{
		FPADashModel Model;
		float StaminaRem = 0.0f;
		Model.StartDash(100.0f, StaminaRem);

		// Thử cancel ở t = 0.20s (< 0.35s) -> Thất bại
		AdvanceDashTime(Model, 0.20f);
		bool bCancelledEarly = Model.TryCancelIntoAttack();
		TestFalse(TEXT("AC4: Attack cancel fails before 0.35s"), bCancelledEarly);
		TestTrue(TEXT("AC4: Still dashing"), Model.bIsDashing);

		// Tiến đến t = 0.36s (>= 0.35s cancel window) -> Thành công
		AdvanceDashTime(Model, 0.16f); // Tổng 0.36s
		TestTrue(TEXT("AC4: Attack cancel available at 0.36s"), Model.bCanCancelIntoAttack);
		TestEqual(TEXT("AC4: In AttackCancelable phase"), Model.CurrentPhase, EPADashPhase::AttackCancelable);

		bool bCancelledSuccess = Model.TryCancelIntoAttack();
		TestTrue(TEXT("AC4: Attack cancel succeeds at 0.36s"), bCancelledSuccess);
		TestFalse(TEXT("AC4: Dash ends immediately on cancel"), Model.bIsDashing);
		TestEqual(TEXT("AC4: Phase transitions to Completed"), Model.CurrentPhase, EPADashPhase::Completed);
	}

	// ===========================================================
	// Edge: Insufficient Stamina
	// ===========================================================
	{
		FPADashModel Model;
		float StaminaRem = 0.0f;

		// Chỉ có 15 stamina (< 25)
		bool bFailed = Model.StartDash(15.0f, StaminaRem);
		TestFalse(TEXT("Edge: Cannot start dash with insufficient stamina (< 25)"), bFailed);
		TestNearlyEqual(TEXT("Edge: Stamina unchanged"), StaminaRem, 15.0f, kDashTolerance);
		TestFalse(TEXT("Edge: Not dashing"), Model.bIsDashing);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
