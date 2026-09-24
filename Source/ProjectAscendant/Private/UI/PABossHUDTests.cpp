// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UI/PABossHUDTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPABossHUDTests
 *
 * Kiểm thử tự động cho Story hud-002 (Boss Health, Stagger Posture & Execution Reticle):
 *  - AC-1: Boss Health Bar & 3-Phase Thresholds — vạch khấc 75%/25%, phase detection.
 *  - AC-2: Posture Stagger Meter & 4.0 Hz Flashing — nhấp nháy đỏ 3.0s, toggle đúng tần số.
 *  - AC-3: Part Status Cross-out — đánh dấu bộ phận gãy.
 *  - AC-4: Execution Reticle Visibility — chỉ hiện khi Staggered, ẩn khi Recovered.
 *
 * Tất cả test chạy trên FPABossHUDModel thuần túy (không UMG).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPABossHUDTests,
	"ProjectAscendant.UI.BossHUD",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
	constexpr float kTolerance = 0.01f;

	/** Giả lập nhiều tick liên tục cho Model.Update() */
	void SimulateBossTicks(FPABossHUDModel& Model, float TotalTime, float StepSize = 0.016f)
	{
		float Elapsed = 0.0f;
		while (Elapsed < TotalTime)
		{
			const float Dt = FMath::Min(StepSize, TotalTime - Elapsed);
			Model.Update(Dt);
			Elapsed += Dt;
		}
	}
}

// ---------------------------------------------------------------------------
// RunTest
// ---------------------------------------------------------------------------

bool FPABossHUDTests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: Boss Health Bar & 3-Phase Thresholds
	// ===========================================================
	{
		FPABossHUDModel Model;
		Model.SetBossIdentity(TEXT("Dragon Lord"), 50);
		Model.SetHP(1000.0f, 1000.0f);

		// QA Test 1: Phase notches at 75% and 25%
		TArray<float> Notches;
		Model.GetPhaseNotchPercents(Notches);
		TestEqual(TEXT("AC1: 2 phase notches"), Notches.Num(), 2);
		TestNearlyEqual(TEXT("AC1: Notch 1 at 75%"), Notches[0], 0.75f, kTolerance);
		TestNearlyEqual(TEXT("AC1: Notch 2 at 25%"), Notches[1], 0.25f, kTolerance);

		// Phase detection
		TestEqual(TEXT("AC1: Phase 1 at 100% HP"), Model.GetCurrentPhase(), 1);

		Model.SetHP(800.0f, 1000.0f);
		TestEqual(TEXT("AC1: Phase 1 at 80% HP"), Model.GetCurrentPhase(), 1);

		Model.SetHP(750.0f, 1000.0f);
		TestEqual(TEXT("AC1: Phase 2 at 75% HP (boundary)"), Model.GetCurrentPhase(), 2);

		Model.SetHP(500.0f, 1000.0f);
		TestEqual(TEXT("AC1: Phase 2 at 50% HP"), Model.GetCurrentPhase(), 2);

		Model.SetHP(250.0f, 1000.0f);
		TestEqual(TEXT("AC1: Phase 3 at 25% HP (boundary)"), Model.GetCurrentPhase(), 3);

		Model.SetHP(100.0f, 1000.0f);
		TestEqual(TEXT("AC1: Phase 3 at 10% HP"), Model.GetCurrentPhase(), 3);

		Model.SetHP(0.0f, 1000.0f);
		TestEqual(TEXT("AC1: Phase 3 at 0% HP (dead)"), Model.GetCurrentPhase(), 3);

		// HP percent accuracy
		Model.SetHP(600.0f, 1000.0f);
		TestNearlyEqual(TEXT("AC1: HP% = 0.60"), Model.GetHPPercent(), 0.60f, kTolerance);

		// Boss identity
		TestEqual(TEXT("AC1: Boss name"), Model.BossName, FString(TEXT("Dragon Lord")));
		TestEqual(TEXT("AC1: Boss level"), Model.BossLevel, 50);

		// Edge: MaxHP = 0 guard
		Model.SetHP(500.0f, 0.0f);
		// MaxHP clamped to 1.0, CurrentHP clamped to 1.0
		TestNearlyEqual(TEXT("AC1: MaxHP=0 guard"), Model.GetHPPercent(), 1.0f, kTolerance);
	}

	// ===========================================================
	// AC-2: Posture Stagger Meter & 4.0 Hz Flashing
	// ===========================================================
	{
		FPABossHUDModel Model;
		Model.SetPosture(0.0f, 100.0f);

		// Posture accumulating
		TestEqual(TEXT("AC2: Initial state = Accumulating"), Model.PostureState, EPABossPostureState::Accumulating);
		TestNearlyEqual(TEXT("AC2: Posture 0%"), Model.GetPosturePercent(), 0.0f, kTolerance);

		Model.SetPosture(50.0f, 100.0f);
		TestNearlyEqual(TEXT("AC2: Posture 50%"), Model.GetPosturePercent(), 0.50f, kTolerance);

		// QA Test 2: Posture 100% → BrokenFlashing
		Model.TriggerPostureBroken();
		TestEqual(TEXT("AC2: PostureBroken → BrokenFlashing"), Model.PostureState, EPABossPostureState::BrokenFlashing);
		TestTrue(TEXT("AC2: Execution reticle visible"), Model.bExecutionReticleVisible);
		TestNearlyEqual(TEXT("AC2: Stagger timer = 3.0s"), Model.StaggerTimer, 3.0f, kTolerance);

		// Nhấp nháy 4.0 Hz → toggle mỗi 0.125s (nửa chu kỳ)
		// Bắt đầu bBlinkVisible = true
		TestTrue(TEXT("AC2: Blink starts visible"), Model.bBlinkVisible);

		// Giả lập 0.125s → toggle lần 1 (visible → invisible)
		SimulateBossTicks(Model, 0.125f);
		TestFalse(TEXT("AC2: After 0.125s → blink invisible"), Model.bBlinkVisible);

		// Thêm 0.125s → toggle lần 2 (invisible → visible) = 1 chu kỳ đầy đủ
		SimulateBossTicks(Model, 0.125f);
		TestTrue(TEXT("AC2: After 0.250s → blink visible (1 full cycle)"), Model.bBlinkVisible);

		// Vẫn đang flashing
		TestTrue(TEXT("AC2: Still flashing during stagger"), Model.IsStaggerFlashing());

		// Stagger active for 3.0s then expires
		// Đã chạy 0.25s, chạy thêm 2.75s+
		SimulateBossTicks(Model, 2.76f);
		TestEqual(TEXT("AC2: After 3.0s → Recovered"), Model.PostureState, EPABossPostureState::Recovered);
		TestFalse(TEXT("AC2: Execution reticle hidden after recovery"), Model.bExecutionReticleVisible);
		TestTrue(TEXT("AC2: Blink reset to visible"), Model.bBlinkVisible);
	}

	// ===========================================================
	// AC-2 (Bổ sung): Đếm số lần toggle trong 1 giây
	// ===========================================================
	{
		FPABossHUDModel Model;
		Model.TriggerPostureBroken();

		// 4.0 Hz = 8 toggles/sec (mỗi nửa chu kỳ toggle 1 lần)
		int32 ToggleCount = 0;
		bool PrevBlink = Model.bBlinkVisible;
		const float StepSize = 0.001f; // 1ms step cho chính xác cao

		for (float t = 0.0f; t < 1.0f; t += StepSize)
		{
			Model.Update(StepSize);
			if (Model.bBlinkVisible != PrevBlink)
			{
				ToggleCount++;
				PrevBlink = Model.bBlinkVisible;
			}
		}

		// 4.0 Hz → 8 toggles/sec (cho phép sai lệch ±1 do rounding)
		TestTrue(TEXT("AC2: ~8 toggles/sec (4.0 Hz)"), FMath::Abs(ToggleCount - 8) <= 1);
	}

	// ===========================================================
	// AC-3: Part Status Cross-out
	// ===========================================================
	{
		FPABossHUDModel Model;

		// Đăng ký bộ phận
		TArray<FName> PartIds;
		PartIds.Add(FName("Horn"));
		PartIds.Add(FName("Tail"));
		Model.RegisterParts(PartIds);

		TestEqual(TEXT("AC3: 2 parts registered"), Model.Parts.Num(), 2);
		TestFalse(TEXT("AC3: Horn not broken"), Model.IsPartBroken(FName("Horn")));
		TestFalse(TEXT("AC3: Tail not broken"), Model.IsPartBroken(FName("Tail")));

		// QA Test 3: Break horn
		Model.BreakPart(FName("Horn"));
		TestTrue(TEXT("AC3: Horn broken"), Model.IsPartBroken(FName("Horn")));
		TestFalse(TEXT("AC3: Tail still intact"), Model.IsPartBroken(FName("Tail")));

		// Break tail
		Model.BreakPart(FName("Tail"));
		TestTrue(TEXT("AC3: Tail broken"), Model.IsPartBroken(FName("Tail")));

		// Bộ phận không tồn tại → false
		TestFalse(TEXT("AC3: Unknown part → false"), Model.IsPartBroken(FName("Wing")));

		// Break bộ phận chưa đăng ký → tự thêm mới
		Model.BreakPart(FName("Wing"));
		TestTrue(TEXT("AC3: Wing auto-registered & broken"), Model.IsPartBroken(FName("Wing")));
		TestEqual(TEXT("AC3: Now 3 parts"), Model.Parts.Num(), 3);
	}

	// ===========================================================
	// AC-4: Execution Reticle Visibility
	// ===========================================================
	{
		FPABossHUDModel Model;

		// QA Test 4: Reticle not visible by default
		TestFalse(TEXT("AC4: Reticle hidden initially"), Model.bExecutionReticleVisible);

		// Stagger → reticle visible
		Model.TriggerPostureBroken();
		TestTrue(TEXT("AC4: Reticle visible during stagger"), Model.bExecutionReticleVisible);

		// Cập nhật vị trí reticle
		Model.UpdateReticleScreenPosition(FVector2D(960.0, 540.0));
		TestNearlyEqual(TEXT("AC4: Reticle X = 960"), (double)Model.ReticleScreenPosition.X, 960.0, (double)kTolerance);
		TestNearlyEqual(TEXT("AC4: Reticle Y = 540"), (double)Model.ReticleScreenPosition.Y, 540.0, (double)kTolerance);

		// Stagger hết → reticle ẩn
		SimulateBossTicks(Model, 3.1f);
		TestFalse(TEXT("AC4: Reticle hidden after recovery"), Model.bExecutionReticleVisible);
		TestEqual(TEXT("AC4: State = Recovered"), Model.PostureState, EPABossPostureState::Recovered);
	}

	// ===========================================================
	// Edge: Trigger stagger lần 2 sau recovery
	// ===========================================================
	{
		FPABossHUDModel Model;

		// Lần 1
		Model.TriggerPostureBroken();
		SimulateBossTicks(Model, 3.1f);
		TestEqual(TEXT("Edge: Recovered after 1st stagger"), Model.PostureState, EPABossPostureState::Recovered);

		// Lần 2
		Model.TriggerPostureBroken();
		TestEqual(TEXT("Edge: 2nd stagger → BrokenFlashing"), Model.PostureState, EPABossPostureState::BrokenFlashing);
		TestTrue(TEXT("Edge: Reticle visible again"), Model.bExecutionReticleVisible);
		TestNearlyEqual(TEXT("Edge: Timer reset to 3.0s"), Model.StaggerTimer, 3.0f, kTolerance);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
