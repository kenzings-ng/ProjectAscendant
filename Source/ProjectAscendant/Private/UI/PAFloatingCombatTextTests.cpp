// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UI/PACombatTextTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAFloatingCombatTextTests
 *
 * Kiểm thử tự động cho Story hud-003 (Floating Combat Text & Action Feedback):
 *  - AC-1: Damage Number Categories & Styling — scale đúng theo loại (1.0, 1.5, 0.85, 1.2).
 *  - AC-2: Ballistic Radial Scatter Arc — vị trí đúng công thức P(t) = P0 + V0*t + 0.5*g*t².
 *  - AC-3: "PERFECT!" Callout — text đúng, lifetime 0.50s.
 *  - AC-4: Object Pool Recycling — cap 50 instance, tái sử dụng, zero leak.
 *
 * Tất cả test chạy trên FPACombatTextPool thuần túy (không UMG).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAFloatingCombatTextTests,
	"ProjectAscendant.UI.FloatingCombatText",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
	constexpr float kCombatTextTolerance = 1.0f; // Sai số vị trí cho phép (cm)
	constexpr float kCombatTextSmallTolerance = 0.01f;

	void SimulatePoolTicks(FPACombatTextPool& Pool, float TotalTime, float StepSize = 0.016f)
	{
		float Elapsed = 0.0f;
		while (Elapsed < TotalTime)
		{
			const float Dt = FMath::Min(StepSize, TotalTime - Elapsed);
			Pool.Update(Dt);
			Elapsed += Dt;
		}
	}
}

// ---------------------------------------------------------------------------
// RunTest
// ---------------------------------------------------------------------------

bool FPAFloatingCombatTextTests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: Damage Number Categories & Styling
	// ===========================================================
	{
		FPACombatTextPool Pool;

		// QA Test 1: Normal damage → scale 1.0, text = số nguyên
		const FVector SpawnPos(100.0f, 200.0f, 300.0f);
		int32 NormalIdx = Pool.SpawnCombatText(SpawnPos, 150.0f, EPACombatTextType::NormalDamage);
		TestNotEqual(TEXT("AC1: Normal spawned"), NormalIdx, (int32)INDEX_NONE);
		TestNearlyEqual(TEXT("AC1: Normal scale = 1.0"), Pool.Instances[NormalIdx].Scale, 1.0f, kCombatTextSmallTolerance);
		TestEqual(TEXT("AC1: Normal text = '150'"), Pool.Instances[NormalIdx].Text, FString(TEXT("150")));
		TestEqual(TEXT("AC1: Normal type"), Pool.Instances[NormalIdx].Type, EPACombatTextType::NormalDamage);
		TestNearlyEqual(TEXT("AC1: Normal lifetime = 0.60s"), Pool.Instances[NormalIdx].MaxLifetime, 0.60f, kCombatTextSmallTolerance);

		// QA Test 1: Critical damage → scale 1.5, orange
		int32 CritIdx = Pool.SpawnCombatText(SpawnPos, 300.0f, EPACombatTextType::CriticalDamage);
		TestNearlyEqual(TEXT("AC1: Crit scale = 1.5"), Pool.Instances[CritIdx].Scale, 1.5f, kCombatTextSmallTolerance);
		TestEqual(TEXT("AC1: Crit text = '300'"), Pool.Instances[CritIdx].Text, FString(TEXT("300")));
		TestEqual(TEXT("AC1: Crit type"), Pool.Instances[CritIdx].Type, EPACombatTextType::CriticalDamage);

		// Posture damage → scale 0.85
		int32 PostureIdx = Pool.SpawnCombatText(SpawnPos, 45.0f, EPACombatTextType::PostureDamage);
		TestNearlyEqual(TEXT("AC1: Posture scale = 0.85"), Pool.Instances[PostureIdx].Scale, 0.85f, kCombatTextSmallTolerance);
		TestEqual(TEXT("AC1: Posture text = '45'"), Pool.Instances[PostureIdx].Text, FString(TEXT("45")));
	}

	// ===========================================================
	// AC-2: Ballistic Radial Scatter Arc
	// ===========================================================
	{
		FPACombatTextPool Pool;
		const FVector SpawnPos(0.0f, 0.0f, 100.0f);

		int32 Idx = Pool.SpawnCombatText(SpawnPos, 100.0f, EPACombatTextType::NormalDamage);
		const FPACombatTextInstance& Inst = Pool.Instances[Idx];

		// Vận tốc ban đầu Z = 180 cm/s (GDD)
		TestNearlyEqual(TEXT("AC2: InitialVelocity.Z = 180"), (float)Inst.InitialVelocity.Z, 180.0f, kCombatTextSmallTolerance);

		// QA Test 2: Vị trí tại t = 0.3s theo công thức đạn đạo
		// P(0.3) = P0 + V0 * 0.3 + 0.5 * g * 0.3²
		// Z: 100 + 180*0.3 + 0.5*(-300)*0.09 = 100 + 54 - 13.5 = 140.5
		const float t = 0.3f;
		const float ExpectedZ = (float)(SpawnPos.Z + Inst.InitialVelocity.Z * t + 0.5f * Pool.Config.GravityZ * t * t);

		// Giả lập chính xác 0.3s (dùng step nhỏ)
		SimulatePoolTicks(Pool, 0.3f, 0.001f);

		TestNearlyEqual(TEXT("AC2: Z at t=0.3s follows ballistic"), (float)Pool.Instances[Idx].WorldPosition.Z, ExpectedZ, kCombatTextTolerance);

		// Kiểm tra vận tốc hiện tại: V(0.3) = V0 + g*0.3
		// Vz: 180 + (-300)*0.3 = 180 - 90 = 90 cm/s (vẫn đang bay lên)
		const float ExpectedVz = (float)(Inst.InitialVelocity.Z + Pool.Config.GravityZ * t);
		TestNearlyEqual(TEXT("AC2: Vz at t=0.3s"), (float)Pool.Instances[Idx].Velocity.Z, ExpectedVz, kCombatTextTolerance);

		// QA Test 3: Radial offset — spawn 3 số tại cùng vị trí → vận tốc X,Y khác nhau
		FPACombatTextPool Pool2;
		TArray<FVector> InitialVelocities;
		for (int32 i = 0; i < 3; ++i)
		{
			int32 Idx2 = Pool2.SpawnCombatText(SpawnPos, 100.0f, EPACombatTextType::NormalDamage);
			InitialVelocities.Add(Pool2.Instances[Idx2].InitialVelocity);
		}

		// Kiểm tra các vận tốc XY không giống nhau (radial scatter)
		bool bAllDifferent = true;
		for (int32 i = 0; i < InitialVelocities.Num(); ++i)
		{
			for (int32 j = i + 1; j < InitialVelocities.Num(); ++j)
			{
				if (FVector2D(InitialVelocities[i].X, InitialVelocities[i].Y).Equals(
					FVector2D(InitialVelocities[j].X, InitialVelocities[j].Y), 0.01f))
				{
					bAllDifferent = false;
				}
			}
		}
		TestTrue(TEXT("AC2: Radial offsets produce different XY velocities"), bAllDifferent);

		// Tất cả vận tốc Z đều = 180 (chỉ XY khác nhau)
		for (int32 i = 0; i < InitialVelocities.Num(); ++i)
		{
			TestNearlyEqual(
				*FString::Printf(TEXT("AC2: Vel[%d].Z = 180"), i),
				(float)InitialVelocities[i].Z, 180.0f, kCombatTextSmallTolerance);
		}
	}

	// ===========================================================
	// AC-3: "PERFECT!" Callout
	// ===========================================================
	{
		FPACombatTextPool Pool;
		const FVector CharPos(500.0f, 500.0f, 0.0f);

		int32 Idx = Pool.SpawnPerfectDodgeCallout(CharPos);
		TestNotEqual(TEXT("AC3: PERFECT! spawned"), Idx, (int32)INDEX_NONE);

		const FPACombatTextInstance& Inst = Pool.Instances[Idx];
		TestEqual(TEXT("AC3: Text = 'PERFECT!'"), Inst.Text, FString(TEXT("PERFECT!")));
		TestEqual(TEXT("AC3: Type = PerfectDodgeCallout"), Inst.Type, EPACombatTextType::PerfectDodgeCallout);
		TestNearlyEqual(TEXT("AC3: Lifetime = 0.50s"), Inst.MaxLifetime, 0.50f, kCombatTextSmallTolerance);
		TestNearlyEqual(TEXT("AC3: Scale = 1.2"), Inst.Scale, 1.2f, kCombatTextSmallTolerance);

		// Spawn position nổi lên 60cm phía trên nhân vật
		TestNearlyEqual(TEXT("AC3: SpawnPos.Z = CharPos.Z + 60"), (float)Inst.SpawnPosition.Z, 60.0f, kCombatTextSmallTolerance);

		// Sau 0.50s+ callout hết hạn
		SimulatePoolTicks(Pool, 0.51f);
		TestFalse(TEXT("AC3: PERFECT! expired after 0.50s"), Pool.Instances[Idx].bActive);
		TestEqual(TEXT("AC3: Active count = 0"), Pool.GetActiveCount(), 0);
	}

	// ===========================================================
	// AC-4: Object Pool Recycling
	// ===========================================================
	{
		FPACombatTextPool Pool;
		const FVector SpawnPos(0.0f, 0.0f, 0.0f);

		// QA Test 4: Spawn 50 numbers → pool capped at 50
		for (int32 i = 0; i < 50; ++i)
		{
			int32 Idx = Pool.SpawnCombatText(SpawnPos, (float)(i * 10), EPACombatTextType::NormalDamage);
			TestNotEqual(*FString::Printf(TEXT("AC4: Spawn %d succeeded"), i), Idx, (int32)INDEX_NONE);
		}
		TestEqual(TEXT("AC4: Pool size = 50"), Pool.GetPoolSize(), 50);
		TestEqual(TEXT("AC4: Active count = 50"), Pool.GetActiveCount(), 50);

		// Spawn thêm 10 → recycle cũ nhất (pool không vượt quá 50)
		for (int32 i = 0; i < 10; ++i)
		{
			int32 Idx = Pool.SpawnCombatText(SpawnPos, 999.0f, EPACombatTextType::CriticalDamage);
			TestNotEqual(*FString::Printf(TEXT("AC4: Overflow spawn %d succeeded"), i), Idx, (int32)INDEX_NONE);
		}
		TestEqual(TEXT("AC4: Pool size still 50 (capped)"), Pool.GetPoolSize(), 50);

		// Tất cả hết hạn sau 0.60s+ → active = 0
		SimulatePoolTicks(Pool, 0.70f);
		TestEqual(TEXT("AC4: All expired → active 0"), Pool.GetActiveCount(), 0);

		// Tái sử dụng slot cũ → không tăng pool size
		int32 ReuseIdx = Pool.SpawnCombatText(SpawnPos, 42.0f, EPACombatTextType::NormalDamage);
		TestNotEqual(TEXT("AC4: Reuse slot succeeded"), ReuseIdx, (int32)INDEX_NONE);
		TestEqual(TEXT("AC4: Pool size still 50 (reused)"), Pool.GetPoolSize(), 50);
		TestEqual(TEXT("AC4: Active count = 1"), Pool.GetActiveCount(), 1);
		TestEqual(TEXT("AC4: Reused text = '42'"), Pool.Instances[ReuseIdx].Text, FString(TEXT("42")));
	}

	// ===========================================================
	// Edge: Opacity fade-out
	// ===========================================================
	{
		FPACombatTextPool Pool;
		int32 Idx = Pool.SpawnCombatText(FVector::ZeroVector, 100.0f, EPACombatTextType::NormalDamage);

		// Opacity bắt đầu = 1.0
		TestNearlyEqual(TEXT("Edge: Initial opacity = 1.0"), Pool.Instances[Idx].Opacity, 1.0f, kCombatTextSmallTolerance);

		// Tại nửa đời (0.30s / 0.60s) → opacity ~ 0.50
		SimulatePoolTicks(Pool, 0.30f, 0.001f);
		TestNearlyEqual(TEXT("Edge: Opacity at 50% lifetime ~ 0.50"), Pool.Instances[Idx].Opacity, 0.50f, 0.05f);

		// Hết đời → opacity = 0
		SimulatePoolTicks(Pool, 0.31f);
		TestNearlyEqual(TEXT("Edge: Opacity at expiry = 0"), Pool.Instances[Idx].Opacity, 0.0f, kCombatTextSmallTolerance);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
