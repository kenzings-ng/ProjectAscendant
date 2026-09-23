// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Economy/PAKarmaTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAKarmaTests
 *
 * Kiểm thử tự động cho Story zone-003 (5-Tier Karma State Machine & Death Penalties Matrix):
 *  - AC-1: 5-Tier Karma State Machine (Righteous, Neutral, Aggressor 120s, Criminal, Wanted Outlaw).
 *  - AC-2: PvE Death Penalty (Rơi 50% Vàng và 100% Shards vào Tàn tích Tro).
 *  - AC-3: Innocent PvP Victim Protection (Chỉ mất 25% Vàng cho sát thủ, không mất Shards).
 *  - AC-4: Wanted Outlaw Death & Labor Prison (Rơi 100% Vàng, đi tù 5 phút hoặc đào 20 quặng, reset về -49).
 *
 * Kiểm thử chạy trên FPAKarmaModel thuần túy (không phụ thuộc SaveGame/UI).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAKarmaTests,
	"ProjectAscendant.Economy.KarmaDeathPenalties",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
	constexpr float kTolerance = 0.5f;
}

// ---------------------------------------------------------------------------
// RunTest
// ---------------------------------------------------------------------------

bool FPAKarmaTests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: 5-Tier Karma State Machine
	// ===========================================================
	{
		FPAKarmaModel Model;
		Model.Initialize(0.0f); // Default Neutral
		TestEqual(TEXT("AC1: Karma 0 is Neutral"), Model.GetKarmaTier(), EPAKarmaTier::Neutral);

		// Tăng lên +75 -> Righteous
		Model.AddKarma(75.0f);
		TestEqual(TEXT("AC1: Karma +75 is Righteous"), Model.GetKarmaTier(), EPAKarmaTier::Righteous);

		// Bị chuyển sang -25 -> Criminal
		Model.AddKarma(-100.0f); // 75 - 100 = -25
		TestEqual(TEXT("AC1: Karma -25 is Criminal"), Model.GetKarmaTier(), EPAKarmaTier::Criminal);

		// Xuống -60 -> Wanted Outlaw
		Model.AddKarma(-35.0f); // -25 - 35 = -60
		TestEqual(TEXT("AC1: Karma -60 is WantedOutlaw"), Model.GetKarmaTier(), EPAKarmaTier::WantedOutlaw);

		// Trạng thái Gây hấn Aggressor tạm thời:
		Model.Initialize(20.0f); // Neutral
		TestEqual(TEXT("AC1: Reset to Neutral 20"), Model.GetKarmaTier(), EPAKarmaTier::Neutral);

		Model.TriggerAggressor();
		TestEqual(TEXT("AC1: Triggered Aggressor tier"), Model.GetKarmaTier(), EPAKarmaTier::Aggressor);
		TestNearlyEqual(TEXT("AC1: Aggressor countdown is 120s"), Model.AggressorTimeRemaining, 120.0f, kTolerance);

		// Trôi qua 120s -> Hết Aggressor, quay về bậc Karma gốc (Neutral)
		Model.Update(121.0f);
		TestEqual(TEXT("AC1: Returned to Neutral after 120s countdown"), Model.GetKarmaTier(), EPAKarmaTier::Neutral);
	}

	// ===========================================================
	// AC-2: PvE Death Penalty - Ash Remnant
	// ===========================================================
	{
		FPAKarmaModel Model;
		Model.Initialize(10.0f);

		// Người chơi có 1000 Vàng, 250 Shards bị quái đánh chết
		FPADeathPenaltyResult Result = Model.ResolveDeath(EPADeathScenario::PvE, 1000.0f, 250.0f);

		TestNearlyEqual(TEXT("AC2: Lost 50% Gold (500)"), Result.GoldLost, 500.0f, kTolerance);
		TestNearlyEqual(TEXT("AC2: Lost 100% Shards (250)"), Result.ShardsLost, 250.0f, kTolerance);
		TestTrue(TEXT("AC2: Ash Remnant created"), Result.bAshRemnantCreated);
		TestFalse(TEXT("AC2: Not sent to prison for PvE death"), Result.bSentToPrison);
		TestFalse(TEXT("AC2: Equipped gear not dropped"), Result.bDropInventoryItem);
	}

	// ===========================================================
	// AC-3: Innocent PvP Victim Protection
	// ===========================================================
	{
		FPAKarmaModel KillerModel;
		KillerModel.Initialize(0.0f);

		FPAKarmaModel VictimModel;
		VictimModel.Initialize(20.0f); // Người vô tội Karma >= 0

		// Nạn nhân có 1000 Vàng, 250 Shards bị PK chết
		FPADeathPenaltyResult VictimResult = VictimModel.ResolveDeath(EPADeathScenario::PvPVictim, 1000.0f, 250.0f);

		TestNearlyEqual(TEXT("AC3: Victim loses 25% Gold (250)"), VictimResult.GoldLost, 250.0f, kTolerance);
		TestNearlyEqual(TEXT("AC3: 250 Gold transferred to killer"), VictimResult.GoldToKiller, 250.0f, kTolerance);
		TestNearlyEqual(TEXT("AC3: Victim preserves 100% Shards (0 lost)"), VictimResult.ShardsLost, 0.0f, kTolerance);
		TestFalse(TEXT("AC3: No Ash Remnant created for PvP victim"), VictimResult.bAshRemnantCreated);

		// Sát nhân bị trừ -30 Karma
		KillerModel.AddKarma(-30.0f);
		TestNearlyEqual(TEXT("AC3: Killer Karma reduced by 30 (-30)"), KillerModel.CurrentKarma, -30.0f, kTolerance);
		TestEqual(TEXT("AC3: Killer becomes Criminal"), KillerModel.GetKarmaTier(), EPAKarmaTier::Criminal);
	}

	// ===========================================================
	// AC-4: Wanted Outlaw Death & Labor Prison
	// ===========================================================
	{
		FPAKarmaModel WantedModel;
		WantedModel.Initialize(-70.0f); // Wanted Outlaw
		TestEqual(TEXT("AC4: Initial tier is WantedOutlaw"), WantedModel.GetKarmaTier(), EPAKarmaTier::WantedOutlaw);

		// Kẻ Wanted bị tiêu diệt: có 2000 Gold, 500 Shards
		FPADeathPenaltyResult OutlawDeath = WantedModel.ResolveDeath(EPADeathScenario::WantedOutlaw, 2000.0f, 500.0f, 0.10f); // Roll 10% <= 15%

		TestNearlyEqual(TEXT("AC4: Drops 100% Gold (2000)"), OutlawDeath.GoldLost, 2000.0f, kTolerance);
		TestNearlyEqual(TEXT("AC4: Drops 100% Shards (500)"), OutlawDeath.ShardsLost, 500.0f, kTolerance);
		TestTrue(TEXT("AC4: Sent to Labor Prison"), OutlawDeath.bSentToPrison);
		TestTrue(TEXT("AC4: 15% drop item triggered at roll 0.10"), OutlawDeath.bDropInventoryItem);
		TestTrue(TEXT("AC4: Model flag bInLaborPrison is true"), WantedModel.bInLaborPrison);
		TestNearlyEqual(TEXT("AC4: Prison sentence is 300s (5m)"), WantedModel.PrisonTimeRemaining, 300.0f, kTolerance);

		// Đào 10 quặng (chưa đủ 20 quặng) -> Vẫn ở trong tù
		bool bReleasedEarly = WantedModel.MineOreInPrison(10);
		TestFalse(TEXT("AC4: 10/20 ores does not release early"), bReleasedEarly);
		TestTrue(TEXT("AC4: Still in prison"), WantedModel.bInLaborPrison);

		// Đào thêm 10 quặng (tổng 20 quặng) -> Được phóng thích
		bool bReleasedSuccess = WantedModel.MineOreInPrison(10);
		TestTrue(TEXT("AC4: 20/20 ores releases from prison"), bReleasedSuccess);
		TestFalse(TEXT("AC4: No longer in prison"), WantedModel.bInLaborPrison);

		// Sau khi ra tù: Karma được ấn định lại ở -49 (thoát khỏi mức Wanted)
		TestNearlyEqual(TEXT("AC4: Karma reset to -49 upon release"), WantedModel.CurrentKarma, -49.0f, kTolerance);
		TestEqual(TEXT("AC4: Tier transitions from WantedOutlaw to Criminal"), WantedModel.GetKarmaTier(), EPAKarmaTier::Criminal);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
