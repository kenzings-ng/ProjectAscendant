// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Combat/PAPartBreakingTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAPartBreakingTests
 *
 * Kiểm thử tự động cho Story stgr-002 (Anatomical Part Breaking & Skill Disabling Matrix):
 *  - AC-1: Anatomical Part Health Tracking (Horn 20%, Tail 15%, Chest Armor 25% - Sát thương kép vào Part và Boss).
 *  - AC-2: Behavior Tree Skill Disabling (Gãy sừng choáng 1.5s & cấm Horn Charge; Đứt đuôi cấm Tail Sweep).
 *  - AC-3: Chest Weakpoint Vulnerability (Vỡ giáp ngực nhân hệ số +50% sát thương vào ngực).
 *  - AC-4: Crafting Reagent Drop Notification (Rơi đúng ID nguyên liệu đặc thù theo từng bộ phận).
 *
 * Kiểm thử chạy trên FPAPartBreakingModel thuần túy (không phụ thuộc World runtime).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAPartBreakingTests,
	"ProjectAscendant.Combat.PartBreakingMatrix",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
	constexpr float kTolerance = 0.05f;
}

// ---------------------------------------------------------------------------
// RunTest
// ---------------------------------------------------------------------------

bool FPAPartBreakingTests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: Anatomical Part Health Tracking & Dual Damage
	// ===========================================================
	{
		FPAPartBreakingModel Model;
		Model.Initialize(10000.0f);

		// Boss 10,000 HP:
		// Sừng: 20% = 2,000 HP
		// Đuôi: 15% = 1,500 HP
		// Giáp ngực: 25% = 2,500 HP
		TestNearlyEqual(TEXT("AC1: Horn Max HP is 2,000"), Model.GetPartHealth(EPABossPartType::Horn), 2000.0f, kTolerance);
		TestNearlyEqual(TEXT("AC1: Tail Max HP is 1,500"), Model.GetPartHealth(EPABossPartType::Tail), 1500.0f, kTolerance);
		TestNearlyEqual(TEXT("AC1: Chest Armor Max HP is 2,500"), Model.GetPartHealth(EPABossPartType::ChestArmor), 2500.0f, kTolerance);

		// Đánh 500 sát thương vào Sừng
		FPAPartBreakResult HitResult = Model.ApplyPartDamage(EPABossPartType::Horn, 500.0f);
		TestFalse(TEXT("AC1: Horn not broken at 1500/2000 HP"), HitResult.bPartBroken);

		// Sát thương kép: Máu Sừng giảm 500 (còn 1500), Tổng máu Boss cũng giảm 500 (còn 9500)
		TestNearlyEqual(TEXT("AC1: Horn HP reduced to 1500"), Model.GetPartHealth(EPABossPartType::Horn), 1500.0f, kTolerance);
		TestNearlyEqual(TEXT("AC1: Boss Total HP reduced to 9500"), Model.BossCurrentHealth, 9500.0f, kTolerance);
		TestNearlyEqual(TEXT("AC1: Horn HP percent is 75%"), Model.GetPartHealthPercent(EPABossPartType::Horn), 0.75f, kTolerance);
	}

	// ===========================================================
	// AC-2: Behavior Tree Skill Disabling & Stun
	// ===========================================================
	{
		FPAPartBreakingModel Model;
		Model.Initialize(10000.0f);

		// Đánh bồi thêm 2,000 sát thương vào Sừng -> Gãy sừng hoàn toàn
		FPAPartBreakResult BreakResult = Model.ApplyPartDamage(EPABossPartType::Horn, 2000.0f);
		TestTrue(TEXT("AC2: Horn break triggered"), BreakResult.bPartBroken);
		TestEqual(TEXT("AC2: Broken part is Horn"), BreakResult.BrokenPart, EPABossPartType::Horn);
		TestTrue(TEXT("AC2: IsPartBroken returns true for Horn"), Model.IsPartBroken(EPABossPartType::Horn));

		// Choáng trùm 1.5s khi gãy sừng
		TestTrue(TEXT("AC2: Boss is stunned on horn break"), BreakResult.bStunnedBoss);
		TestNearlyEqual(TEXT("AC2: Stun duration is 1.50s"), BreakResult.StunDuration, 1.50f, kTolerance);

		// Gãy đuôi: Đánh 1,500 sát thương vào Đuôi
		FPAPartBreakResult TailBreakResult = Model.ApplyPartDamage(EPABossPartType::Tail, 1500.0f);
		TestTrue(TEXT("AC2: Tail break triggered"), TailBreakResult.bPartBroken);
		TestEqual(TEXT("AC2: Broken part is Tail"), TailBreakResult.BrokenPart, EPABossPartType::Tail);
		TestTrue(TEXT("AC2: IsPartBroken returns true for Tail"), Model.IsPartBroken(EPABossPartType::Tail));
		TestFalse(TEXT("AC2: Tail break does not stun boss"), TailBreakResult.bStunnedBoss);
	}

	// ===========================================================
	// AC-3: Chest Weakpoint Vulnerability (+50% Damage)
	// ===========================================================
	{
		FPAPartBreakingModel Model;
		Model.Initialize(10000.0f);

		TestFalse(TEXT("AC3: Weakpoint inactive initially"), Model.bChestWeakpointActive);
		TestNearlyEqual(TEXT("AC3: Initial chest damage multiplier is 1.0"), Model.GetDamageMultiplier(EPABossPartType::ChestArmor), 1.0f, kTolerance);

		// Đánh vỡ Giáp ngực (2,500 HP)
		FPAPartBreakResult ArmorBreakResult = Model.ApplyPartDamage(EPABossPartType::ChestArmor, 2500.0f);
		TestTrue(TEXT("AC3: Chest Armor break triggered"), ArmorBreakResult.bPartBroken);
		TestTrue(TEXT("AC3: Weakpoint revealed"), ArmorBreakResult.bWeakpointRevealed);
		TestTrue(TEXT("AC3: bChestWeakpointActive is now true"), Model.bChestWeakpointActive);
		TestNearlyEqual(TEXT("AC3: Multiplier is now 1.50x (+50%)"), Model.GetDamageMultiplier(EPABossPartType::ChestArmor), 1.50f, kTolerance);

		// Đòn đánh tiếp theo có RawDamage = 100 vào ngực
		const float BossHPBefore = Model.BossCurrentHealth;
		FPAPartBreakResult WeakpointHit = Model.ApplyPartDamage(EPABossPartType::ChestArmor, 100.0f);

		// Sát thương thực tế phải là 100 * 1.50 = 150
		TestNearlyEqual(TEXT("AC3: Weakpoint hit deals 150 damage (+50%)"), WeakpointHit.DamageDealtToBoss, 150.0f, kTolerance);
		TestNearlyEqual(TEXT("AC3: Boss HP reduced by exactly 150"), BossHPBefore - Model.BossCurrentHealth, 150.0f, kTolerance);
	}

	// ===========================================================
	// AC-4: Crafting Reagent Drop Notification
	// ===========================================================
	{
		FPAPartBreakingModel Model;
		Model.Initialize(10000.0f);

		// Kiểm tra vật phẩm rơi của Sừng
		FPAPartBreakResult HornDrop = Model.ApplyPartDamage(EPABossPartType::Horn, 2000.0f);
		TestEqual(TEXT("AC4: Horn drops Item_Beast_Horn_Shard"), HornDrop.DropItemId, FName("Item_Beast_Horn_Shard"));

		// Kiểm tra vật phẩm rơi của Đuôi
		FPAPartBreakResult TailDrop = Model.ApplyPartDamage(EPABossPartType::Tail, 1500.0f);
		TestEqual(TEXT("AC4: Tail drops Item_Dragon_Tail_Sinew"), TailDrop.DropItemId, FName("Item_Dragon_Tail_Sinew"));

		// Kiểm tra vật phẩm rơi của Giáp ngực
		FPAPartBreakResult ChestDrop = Model.ApplyPartDamage(EPABossPartType::ChestArmor, 2500.0f);
		TestEqual(TEXT("AC4: Chest drops Item_Hardened_Carapace"), ChestDrop.DropItemId, FName("Item_Hardened_Carapace"));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
