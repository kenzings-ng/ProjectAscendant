// Copyright Project Ascendant. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Combat/PARaidCombatCalculationSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPARaidCombatCalculationTest
 *
 * Kiểm thử tự động cho Story item-006 (Raid Stacking Engine, EPIC-ITEMIZATION-001, Sprint 6):
 * Giả lập 50-Bot Spawner tấn công World Boss để thẩm định 4 quy tắc mạng Server-Authoritative:
 * - AC-1: Max Rule (Stagger Cap 3.5s) & Event DR (Parry 100% -> 50% -> 25%).
 * - AC-2: Instigator Only cho Execution đòn kết liễu và Stagger Leech.
 * - AC-3: Personal Outgoing & Bleed Manager (Max 10 nguồn, 2.0s Grace Period, Atomic Damage Ledger).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPARaidCombatCalculationTest,
	"ProjectAscendant.Itemization.RaidCombatStackingEngine",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPARaidCombatCalculationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance must be valid"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UPARaidCombatCalculationSubsystem* RaidEngine = NewObject<UPARaidCombatCalculationSubsystem>(GameInstance);
	TestNotNull(TEXT("RaidEngine subsystem must be valid"), RaidEngine);
	if (!RaidEngine)
	{
		return false;
	}

	// Tạo danh sách 50 Bots giả lập vây đánh World Boss
	constexpr int32 NUM_BOTS = 50;
	TArray<FGuid> BotIds;
	BotIds.Reserve(NUM_BOTS);
	for (int32 i = 0; i < NUM_BOTS; ++i)
	{
		BotIds.Add(FGuid::NewGuid());
	}
	TestEqual(TEXT("50 simulated bots created"), BotIds.Num(), 50);

	// =========================================================================
	// AC-1: Rule 1 (Max Rule Stagger Cap 3.5s) & Rule 4 (Event DR Parry)
	// =========================================================================
	{
		// 1. Max Rule Stagger Cap
		TArray<float> StaggerBonuses;
		for (int32 i = 0; i < NUM_BOTS; ++i)
		{
			// Giả lập các bonus từ 0.05s đến 1.5s
			StaggerBonuses.Add(0.05f * static_cast<float>(i + 1));
		}

		const float StaggerDuration = RaidEngine->CalculateMaxStaggerFromMultiple(3.0f, StaggerBonuses);
		TestEqual(TEXT("AC-1: Stagger duration is locked at hard cap 3.5s despite 50 bonuses"),
			StaggerDuration, 3.5f);

		// Thử với bonus nhỏ (0.2s) -> 3.2s
		TestEqual(TEXT("AC-1: Small bonus 0.2s yields 3.2s"),
			RaidEngine->CalculateStaggerDuration(3.0f, 0.2f), 3.2f);

		// 2. Event DR for Simultaneous Parries (Cửa sổ 1.0s)
		RaidEngine->ResetParryWindow();
		const float BaseParryPosture = 100.0f;
		const float WindowStartTime = 10.0f;

		// Bot 0 parry lần 1 -> 100% (100.0f)
		const float Parry1 = RaidEngine->CalculateParryPostureReflect(BotIds[0], BaseParryPosture, WindowStartTime);
		TestEqual(TEXT("AC-1: 1st parry gets 100% reflect"), Parry1, 100.0f);

		// Bot 1 parry lần 2 trong cùng cửa sổ -> 50% (50.0f)
		const float Parry2 = RaidEngine->CalculateParryPostureReflect(BotIds[1], BaseParryPosture, WindowStartTime + 0.2f);
		TestEqual(TEXT("AC-1: 2nd parry gets 50% reflect"), Parry2, 50.0f);

		// Bot 2 đến Bot 49 parry trong cùng cửa sổ -> 25% (25.0f)
		for (int32 i = 2; i < NUM_BOTS; ++i)
		{
			const float ParryN = RaidEngine->CalculateParryPostureReflect(BotIds[i], BaseParryPosture, WindowStartTime + 0.5f);
			TestEqual(FString::Printf(TEXT("AC-1: Bot %d parry gets 25%% reflect"), i), ParryN, 25.0f);
		}

		// Sau 1.0s (CurrentTime = 11.5s), mở cửa sổ mới -> Lại nhận 100%
		const float ParryNewWindow = RaidEngine->CalculateParryPostureReflect(BotIds[0], BaseParryPosture, 11.5f);
		TestEqual(TEXT("AC-1: New window resets parry back to 100%"), ParryNewWindow, 100.0f);
	}

	// =========================================================================
	// AC-2: Rule 2 (Instigator Only Execution & Leech)
	// =========================================================================
	{
		const float BossMaxHealth = 20000.0f;
		const float ExecTime = 20.0f;

		// 50 Bots cùng tranh giành tương tác Socket_Execution
		bool bBot0Started = RaidEngine->TryBeginExecution(BotIds[0], ExecTime, 1.2f);
		TestTrue(TEXT("AC-2: Bot 0 successfully claims execution"), bBot0Started);
		TestTrue(TEXT("AC-2: Execution is active on boss"), RaidEngine->IsExecutionActive(ExecTime));
		TestEqual(TEXT("AC-2: Active executor is Bot 0"), RaidEngine->GetActiveExecutorId(), BotIds[0]);

		// 49 Bots còn lại thử tranh cướp -> Phải bị từ chối 100%
		for (int32 i = 1; i < NUM_BOTS; ++i)
		{
			bool bOtherStarted = RaidEngine->TryBeginExecution(BotIds[i], ExecTime, 1.2f);
			TestFalse(FString::Printf(TEXT("AC-2: Bot %d rejected from execution"), i), bOtherStarted);
		}

		// Sát thương kết liễu: Chỉ tính cho Executor (Bot 0)
		// 25% Max HP = 5000, +20% bonus = 6000
		const float ExecDmgBot0 = RaidEngine->CalculateExecutionDamage(BossMaxHealth, 20.0f, BotIds[0]);
		TestEqual(TEXT("AC-2: Bot 0 deals 6000 execution damage"), ExecDmgBot0, 6000.0f);

		const float ExecDmgBot1 = RaidEngine->CalculateExecutionDamage(BossMaxHealth, 20.0f, BotIds[1]);
		TestEqual(TEXT("AC-2: Non-executor Bot 1 deals 0 execution damage"), ExecDmgBot1, 0.0f);

		// Hút máu (Leech on Stagger): Chỉ tính cho Executor (Bot 0)
		// 15% của 6000 = 900
		const float LeechBot0 = RaidEngine->CalculateStaggerLeech(ExecDmgBot0, 15.0f, BotIds[0]);
		TestEqual(TEXT("AC-2: Bot 0 receives 900 leech"), LeechBot0, 900.0f);

		const float LeechBot1 = RaidEngine->CalculateStaggerLeech(ExecDmgBot0, 15.0f, BotIds[1]);
		TestEqual(TEXT("AC-2: Non-executor Bot 1 receives 0 leech"), LeechBot1, 0.0f);

		RaidEngine->EndExecution();
		TestFalse(TEXT("AC-2: Execution ended"), RaidEngine->IsExecutionActive(ExecTime));
	}

	// =========================================================================
	// AC-3: Rule 3 (Bleed Manager: 10 Sources Cap, 2.0s Grace Period, Atomic Ledger)
	// =========================================================================
	{
		RaidEngine->ResetBleedState();

		// 1. 10 Bots đầu tiên (Bot 0 - Bot 9) gắn Bleed lên Boss -> Thành công cả 10
		for (int32 i = 0; i < 10; ++i)
		{
			bool bApplied = RaidEngine->ApplyBleed(BotIds[i], 20.0f, 30.0f);
			TestTrue(FString::Printf(TEXT("AC-3: Bot %d bleed applied"), i), bApplied);
		}
		TestEqual(TEXT("AC-3: Active bleed sources reached cap of 10"),
			RaidEngine->GetActiveBleedSourceCount(), 10);

		// 2. Bot 10 thử gắn Bleed khi cả 10 nguồn đang trong Grace Period (2.0s) -> BỊ TỪ CHỐI
		bool bBot10Rejected = RaidEngine->ApplyBleed(BotIds[10], 25.0f, 30.5f);
		TestFalse(TEXT("AC-3: Bot 10 rejected during active 2.0s Grace Period of existing sources"), bBot10Rejected);
		TestEqual(TEXT("AC-3: Active sources remains 10"), RaidEngine->GetActiveBleedSourceCount(), 10);

		// 3. Tiến hành Tick sát thương trong 2.1 giây (đủ để Grace Period 2.0s hết hạn)
		float TotalDamageDealt = 0.0f;
		RaidEngine->TickBleed(2.1f, TotalDamageDealt);
		TestTrue(TEXT("AC-3: Bleed ticked damage across 2.1 seconds"), TotalDamageDealt > 0.0f);

		// Kiểm tra Atomic Damage Ledger: Cả 10 Bots đều được lưu trữ sát thương vĩnh viễn
		for (int32 i = 0; i < 10; ++i)
		{
			const float RecordedDmg = RaidEngine->GetPlayerRecordedDamage(BotIds[i]);
			TestTrue(FString::Printf(TEXT("AC-3: Bot %d has recorded damage in Atomic Ledger"), i), RecordedDmg > 0.0f);
		}

		// 4. Giờ đây Grace Period đã hết hạn, Bot 10 với DPS cao hơn (30.0f > 20.0f) có thể evict nguồn yếu nhất
		bool bBot10Evicts = RaidEngine->ApplyBleed(BotIds[10], 30.0f, 32.5f);
		TestTrue(TEXT("AC-3: Bot 10 successfully evicts expired source after Grace Period"), bBot10Evicts);
		TestEqual(TEXT("AC-3: Active sources strictly maintains ceiling of 10"),
			RaidEngine->GetActiveBleedSourceCount(), 10);

		// 5. Kiểm tra giới hạn 3 Stacks của cá nhân
		// Bot 10 chém liên tiếp 4 lần
		RaidEngine->ApplyBleed(BotIds[10], 30.0f, 33.0f); // Stack 2
		RaidEngine->ApplyBleed(BotIds[10], 30.0f, 33.1f); // Stack 3
		RaidEngine->ApplyBleed(BotIds[10], 30.0f, 33.2f); // Vẫn Stack 3 (làm mới thời lượng)

		// Tick tiếp 1 giây
		float DmgTick2 = 0.0f;
		RaidEngine->TickBleed(1.0f, DmgTick2);

		// Kiểm tra Atomic Damage Ledger của Bot 10 đã được tích lũy
		TestTrue(TEXT("AC-3: Bot 10 has recorded damage in Atomic Ledger"),
			RaidEngine->GetPlayerRecordedDamage(BotIds[10]) > 0.0f);

		// Xác nhận trần DPS toàn raid không vượt quá giới hạn
		TestTrue(TEXT("AC-3: Total raid bleed DPS is within safe limits"),
			RaidEngine->GetTotalBleedDPS() <= UPARaidCombatCalculationSubsystem::GLOBAL_BLEED_MAX_DPS);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
