// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Network/PADifficultyScalingTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPADifficultyScalingTests
 *
 * Kiểm thử tự động cho Story zone-002 (Dynamic Difficulty Scaling & Contested Instanced Loot):
 *  - AC-1: Dynamic HP Scaling Formula (BaseMaxHP * (1.0 + 0.50 * (N-1))).
 *  - AC-2: Dynamic Posture Scaling Formula (BaseMaxPosture * (1.0 + 0.35 * (N-1))).
 *  - AC-3: Anti-Zerg Crowd Control Reduction (N >= 4: 30% CC; N >= 8: 50% CC + 25% Turn rate).
 *  - AC-4: Instanced Loot Contribution Threshold (>= 5% HP HOẶC >= 10% Posture).
 *
 * Kiểm thử chạy trên FPADifficultyScalingModel thuần túy (không phụ thuộc NetDriver).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPADifficultyScalingTests,
	"ProjectAscendant.Network.DifficultyScalingLoot",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
	constexpr float kDiffTolerance = 0.5f;
}

// ---------------------------------------------------------------------------
// RunTest
// ---------------------------------------------------------------------------

bool FPADifficultyScalingTests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: Dynamic HP Scaling Formula
	// ===========================================================
	{
		FPADifficultyScalingModel Model;
		Model.Initialize(10000.0f, 800.0f);

		// 1 người chơi: 10,000 HP
		Model.RegisterCombatant(TEXT("Player_1"));
		TestNearlyEqual(TEXT("AC1: 1 combatant -> 10,000 HP"), Model.ComputeScaledMaxHP(), 10000.0f, kDiffTolerance);

		// Thêm người chơi 2, 3, 4 (Tổng 4 người)
		Model.RegisterCombatant(TEXT("Player_2"));
		Model.RegisterCombatant(TEXT("Player_3"));
		Model.RegisterCombatant(TEXT("Player_4"));

		// 4 người chơi: 10,000 * (1.0 + 0.50 * 3) = 10,000 * 2.5 = 25,000 HP
		TestEqual(TEXT("AC1: 4 combatants registered"), Model.GetCombatantCount(), 4);
		TestNearlyEqual(TEXT("AC1: 4 combatants -> 25,000 HP"), Model.ComputeScaledMaxHP(), 25000.0f, kDiffTolerance);
	}

	// ===========================================================
	// AC-2: Dynamic Posture Scaling Formula
	// ===========================================================
	{
		FPADifficultyScalingModel Model;
		Model.Initialize(10000.0f, 800.0f);

		// 1 người chơi: 800 Posture
		Model.RegisterCombatant(TEXT("Player_1"));
		TestNearlyEqual(TEXT("AC2: 1 combatant -> 800 Posture"), Model.ComputeScaledMaxPosture(), 800.0f, kDiffTolerance);

		// 4 người chơi: 800 * (1.0 + 0.35 * 3) = 800 * 2.05 = 1,640 Posture
		Model.RegisterCombatant(TEXT("Player_2"));
		Model.RegisterCombatant(TEXT("Player_3"));
		Model.RegisterCombatant(TEXT("Player_4"));
		TestNearlyEqual(TEXT("AC2: 4 combatants -> 1,640 Posture"), Model.ComputeScaledMaxPosture(), 1640.0f, kDiffTolerance);
	}

	// ===========================================================
	// AC-3: Anti-Zerg Crowd Control Reduction
	// ===========================================================
	{
		FPADifficultyScalingModel Model;
		Model.Initialize(10000.0f, 800.0f);

		// 3 người chơi: 0% kháng khống chế, 0% tốc độ xoay
		Model.RegisterCombatant(TEXT("P1"));
		Model.RegisterCombatant(TEXT("P2"));
		Model.RegisterCombatant(TEXT("P3"));
		TestNearlyEqual(TEXT("AC3: 3 combatants -> 0% CC reduction"), Model.ComputeCCReduction(), 0.0f, 0.01f);
		TestNearlyEqual(TEXT("AC3: 3 combatants -> 0% turn rate bonus"), Model.ComputeTurnRateBonus(), 0.0f, 0.01f);

		// 4 người chơi: Bật Anti-Zerg Tier 1 (+30% CC reduction)
		Model.RegisterCombatant(TEXT("P4"));
		TestNearlyEqual(TEXT("AC3: 4 combatants -> 30% CC reduction"), Model.ComputeCCReduction(), 0.30f, 0.01f);
		TestNearlyEqual(TEXT("AC3: 4 combatants -> 0% turn rate bonus"), Model.ComputeTurnRateBonus(), 0.0f, 0.01f);

		// Thêm tới 8 người chơi: Bật Anti-Zerg Tier 2 (+50% CC reduction + 25% Turn rate)
		Model.RegisterCombatant(TEXT("P5"));
		Model.RegisterCombatant(TEXT("P6"));
		Model.RegisterCombatant(TEXT("P7"));
		Model.RegisterCombatant(TEXT("P8"));
		TestEqual(TEXT("AC3: 8 combatants registered"), Model.GetCombatantCount(), 8);
		TestNearlyEqual(TEXT("AC3: 8 combatants -> 50% CC reduction"), Model.ComputeCCReduction(), 0.50f, 0.01f);
		TestNearlyEqual(TEXT("AC3: 8 combatants -> +25% turn rate bonus"), Model.ComputeTurnRateBonus(), 0.25f, 0.01f);
	}

	// ===========================================================
	// AC-4: Instanced Loot Contribution Threshold
	// ===========================================================
	{
		FPADifficultyScalingModel Model;
		Model.Initialize(10000.0f, 800.0f);

		// Đăng ký 4 người chơi -> ScaledHP = 25,000; ScaledPosture = 1,640
		// Ngưỡng tối thiểu nhận Loot:
		// 5% HP = 1,250 HP damage
		// 10% Posture = 164 Posture damage
		Model.RegisterCombatant(TEXT("Player_Attacker"));
		Model.RegisterCombatant(TEXT("Player_Staggerer"));
		Model.RegisterCombatant(TEXT("Player_Leecher"));
		Model.RegisterCombatant(TEXT("Player_Solo"));

		// 1. Player_Attacker: Gây 1,500 HP damage (6% > 5%) -> ĐỦ ĐIỀU KIỆN
		Model.RecordDamage(TEXT("Player_Attacker"), 1500.0f, 0.0f);
		TestTrue(TEXT("AC4: Attacker with 6% HP damage qualifies for loot"), Model.IsQualifiedForLoot(TEXT("Player_Attacker")));

		// 2. Player_Staggerer: Gây 200 HP damage (ít) nhưng 200 Posture damage (> 164 = 10%) -> ĐỦ ĐIỀU KIỆN
		Model.RecordDamage(TEXT("Player_Staggerer"), 200.0f, 200.0f);
		TestTrue(TEXT("AC4: Staggerer with >10% Posture damage qualifies for loot"), Model.IsQualifiedForLoot(TEXT("Player_Staggerer")));

		// 3. Player_Leecher: Đánh hôi chỉ 500 HP damage (2% < 5%) và 50 Posture (< 10%) -> BỊ LOẠI
		Model.RecordDamage(TEXT("Player_Leecher"), 500.0f, 50.0f);
		TestFalse(TEXT("AC4: Leecher below both thresholds does NOT qualify for loot"), Model.IsQualifiedForLoot(TEXT("Player_Leecher")));

		// 4. Player_Solo: Chưa đánh đòn nào (0 HP, 0 Posture) -> BỊ LOẠI
		TestFalse(TEXT("AC4: Inactive player does not qualify"), Model.IsQualifiedForLoot(TEXT("Player_Solo")));

		// Tổng danh sách người đủ điều kiện
		TArray<FString> Qualified = Model.GetQualifiedPlayers();
		TestEqual(TEXT("AC4: Exactly 2 players qualified"), Qualified.Num(), 2);
		TestTrue(TEXT("AC4: Qualified contains Attacker"), Qualified.Contains(TEXT("Player_Attacker")));
		TestTrue(TEXT("AC4: Qualified contains Staggerer"), Qualified.Contains(TEXT("Player_Staggerer")));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
