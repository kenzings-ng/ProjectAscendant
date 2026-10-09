// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Combat/PAPostureSyncComponent.h"
#include "Network/PALootDistributionSubsystem.h"
#include "Network/PALootDropletActor.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAContestedLootFinisherTest
 *
 * Kiểm thử tự động tích hợp cho Cửa sổ ưu tiên kết liễu 1.5s, Túi chiến lợi phẩm cá nhân
 * và Chia sẻ EXP tổ đội theo cự ly không gian (Story 004 / net-004, ADR-0001).
 *
 * Tiêu chí nghiệm thu:
 *  - AC-7: 1.5s Finisher Exclusive Window (Player 2 bị chặn tại T=0.8s; được phép tại T=1.6s; gây 25% Max HP; vinh danh đúng Executor).
 *  - AC-6: Ngưỡng đóng góp Instanced Loot (Player A 6% HP -> Đạt; Player B 4.5% HP -> Trượt; Player C 12% Posture -> Đạt; cô lập hiển thị mạng IsNetRelevantFor).
 *  - AC-8: Chia sẻ EXP tổ đội bán kính 3000 cm với Morale Bonus (Đội 4 người: 3 người gần nhận +35% bonus; người ở 3500 cm nhận 0 EXP).
 *  - Guardrail: Thời gian tính toán và phân phối loot <= 1.0ms.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAContestedLootFinisherTest,
	"ProjectAscendant.Foundation.Netcode.ContestedLootFinisherIntegration",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAContestedLootFinisherTest::RunTest(const FString& Parameters)
{
	// =========================================================================
	// Test 1: AC-7 (1.5s Finisher Exclusive Window & Stagger Execution)
	// =========================================================================
	{
		UPAPostureSyncComponent* PostureComp = NewObject<UPAPostureSyncComponent>();
		const float BossMaxHP = 10000.0f;
		const FGuid FinisherUID = FGuid(1, 2, 3, 4);
		const FGuid OtherPlayerUID = FGuid(5, 6, 7, 8);
		const FString FinisherId = TEXT("Player_1");
		const FString OtherPlayerId = TEXT("Player_2");

		// Boss bị bẻ gãy thế đứng tại T = 100.0s bởi Player 1
		const float BreakTime = 100.0f;
		PostureComp->NotifyPostureBrokenWithId(FinisherId, BossMaxHP, BreakTime);

		TestTrue(TEXT("AC-7: Boss phải rơi vào trạng thái Choáng vỡ thế (Staggered)"), PostureComp->IsStaggered());
		TestEqual(TEXT("AC-7: FinisherPlayerId phải được gán chính xác là Player 1"), PostureComp->GetFinisherPlayerId(), FinisherId);
		TestEqual(TEXT("AC-7: BreakTimestamp phải ghi nhận mốc 100.0s"), PostureComp->GetBreakTimestamp(), BreakTime);

		// 1.1: Tại T = 100.8s (Elapsed = 0.8s, nằm trong cửa sổ 1.5s độc quyền)
		const float TimeWithinWindow = 100.8f;
		TestTrue(TEXT("AC-7: Tại T=0.8s phải đang trong cửa sổ độc quyền 1.5s"), PostureComp->IsInFinisherExclusiveWindow(TimeWithinWindow));

		// Player 2 cố tình gửi yêu cầu kết liễu -> Máy chủ PHẢI TỪ CHỐI
		const bool bCanPlayer2ExecuteEarly = PostureComp->CanExecuteById(OtherPlayerId, TimeWithinWindow);
		TestFalse(TEXT("AC-7: Player 2 KHÔNG ĐƯỢC PHÉP kết liễu trong cửa sổ 1.5s độc quyền của Player 1"), bCanPlayer2ExecuteEarly);

		const bool bExecution2Rejected = PostureComp->TryExecuteStaggerById(OtherPlayerId, BossMaxHP, TimeWithinWindow);
		TestFalse(TEXT("AC-7: Máy chủ phải từ chối yêu cầu kết liễu từ Player 2 tại T=0.8s"), bExecution2Rejected);
		TestTrue(TEXT("AC-7: Boss vẫn phải duy trì trạng thái Stagger sau khi từ chối Player 2"), PostureComp->IsStaggered());

		// Finisher (Player 1) có quyền kết liễu trong thời điểm này
		const bool bCanPlayer1Execute = PostureComp->CanExecuteById(FinisherId, TimeWithinWindow);
		TestTrue(TEXT("AC-7: Player 1 (Finisher) PHẢI ĐƯỢC PHÉP kết liễu tại T=0.8s"), bCanPlayer1Execute);

		// 1.2: Tại T = 101.6s (Elapsed = 1.6s > 1.5s, cửa sổ độc quyền đã hết hạn, mở tự do)
		const float TimeAfterWindow = 101.6f;
		TestFalse(TEXT("AC-7: Tại T=1.6s cửa sổ độc quyền 1.5s phải kết thúc"), PostureComp->IsInFinisherExclusiveWindow(TimeAfterWindow));

		const bool bCanPlayer2ExecuteLate = PostureComp->CanExecuteById(OtherPlayerId, TimeAfterWindow);
		TestTrue(TEXT("AC-7: Sau 1.5s, Player 2 được phép kết liễu tự do"), bCanPlayer2ExecuteLate);

		// Thực thi kết liễu bởi Player 2 tại T=1.6s
		const bool bExecutionAccepted = PostureComp->TryExecuteStaggerById(OtherPlayerId, BossMaxHP, TimeAfterWindow);
		TestTrue(TEXT("AC-7: Máy chủ chấp thuận đòn kết liễu của Player 2 sau 1.5s"), bExecutionAccepted);
		TestFalse(TEXT("AC-7: Boss thoát khỏi trạng thái Choáng vỡ thế sau khi bị kết liễu"), PostureComp->IsStaggered());
	}

	// =========================================================================
	// Test 2: AC-6 (Instanced Loot Contribution Threshold & Droplet Isolation)
	// =========================================================================
	{
		UPALootDistributionSubsystem* LootSubsystem = NewObject<UPALootDistributionSubsystem>();
		const FString EncounterId = TEXT("Boss_StoneGolem_01");
		const float BossMaxHP = 10000.0f;
		const float BossMaxPosture = 1000.0f;

		const FGuid UID_A = FGuid(10, 0, 0, 1);
		const FGuid UID_B = FGuid(10, 0, 0, 2);
		const FGuid UID_C = FGuid(10, 0, 0, 3);

		// Kịch bản QA test case:
		// - Player A: 600 HP dmg (6% >= 5% HP) -> Đủ điều kiện
		// - Player B: 450 HP dmg (4.5% < 5% HP, 0 Posture) -> Không đủ điều kiện
		// - Player C: 0 HP dmg, 120 Posture dmg (12% >= 10% Posture) -> Đủ điều kiện
		LootSubsystem->RecordCombatContributionWithUID(EncounterId, UID_A, TEXT("Player_A"), nullptr, 600.0f, 0.0f);
		LootSubsystem->RecordCombatContributionWithUID(EncounterId, UID_B, TEXT("Player_B"), nullptr, 450.0f, 0.0f);
		LootSubsystem->RecordCombatContributionWithUID(EncounterId, UID_C, TEXT("Player_C"), nullptr, 0.0f, 120.0f);

		// Kiểm tra tính đủ điều kiện theo FGuid
		const bool bEligibleA = LootSubsystem->IsPlayerEligibleForLootByUID(EncounterId, UID_A, BossMaxHP, BossMaxPosture);
		const bool bEligibleB = LootSubsystem->IsPlayerEligibleForLootByUID(EncounterId, UID_B, BossMaxHP, BossMaxPosture);
		const bool bEligibleC = LootSubsystem->IsPlayerEligibleForLootByUID(EncounterId, UID_C, BossMaxHP, BossMaxPosture);

		TestTrue(TEXT("AC-6: Player A đạt 6% HP (>= 5%) PHẢI đủ điều kiện nhận Loot"), bEligibleA);
		TestFalse(TEXT("AC-6: Player B chỉ đạt 4.5% HP (< 5%) KHÔNG ĐƯỢC nhận Loot"), bEligibleB);
		TestTrue(TEXT("AC-6: Player C đạt 12% Posture (>= 10%) PHẢI đủ điều kiện nhận Loot"), bEligibleC);

		const TArray<FGuid> EligibleList = LootSubsystem->GetEligiblePlayerUIDsForLoot(EncounterId, BossMaxHP, BossMaxPosture);
		TestEqual(TEXT("AC-6: Tổng cộng chính xác 2 người chơi đủ điều kiện nhận túi đồ"), EligibleList.Num(), 2);
		TestTrue(TEXT("AC-6: Danh sách đủ điều kiện phải chứa UID_A"), EligibleList.Contains(UID_A));
		TestTrue(TEXT("AC-6: Danh sách đủ điều kiện phải chứa UID_C"), EligibleList.Contains(UID_C));
		TestFalse(TEXT("AC-6: Danh sách đủ điều kiện KHÔNG ĐƯỢC chứa UID_B"), EligibleList.Contains(UID_B));

		// 2.2: Kiểm tra tính bảo mật và cách ly tầm nhìn của APALootDropletActor (Anti-Ninja Looting)
		APALootDropletActor* DropletForA = NewObject<APALootDropletActor>();
		DropletForA->InitializeDropletWithUID(UID_A, TEXT("Player_A"), nullptr, EncounterId, 500, { FName("Item_GolemCore") });

		// Player A có quyền nhìn thấy và nhặt theo FGuid
		TestTrue(TEXT("AC-6: Player A có quyền nhìn thấy và nhặt túi đồ của mình"), DropletForA->CanPlayerSeeOrInteractByUID(UID_A));

		// Player C (hoặc người lạ) hoàn toàn không có quyền tương tác với túi đồ của Player A
		TestFalse(TEXT("AC-6: Player C KHÔNG THỂ nhìn thấy hay nhặt túi đồ của Player A"), DropletForA->CanPlayerSeeOrInteractByUID(UID_C));
		TestFalse(TEXT("AC-6: Player B KHÔNG THỂ nhìn thấy hay nhặt túi đồ của Player A"), DropletForA->CanPlayerSeeOrInteractByUID(UID_B));

		// Nhặt đồ thành công bởi Player A
		int32 ClaimedGold = 0;
		TArray<FName> ClaimedItems;
		const bool bClaimSuccess = DropletForA->TryClaimLootByUID(UID_A, ClaimedGold, ClaimedItems);
		TestTrue(TEXT("AC-6: Nhặt chiến lợi phẩm thành công"), bClaimSuccess);
		TestEqual(TEXT("AC-6: Nhận đúng 500 vàng thưởng"), ClaimedGold, 500);
		TestEqual(TEXT("AC-6: Nhận đúng 1 vật phẩm Golem Core"), ClaimedItems.Num(), 1);

		// Nhặt lại lần 2 bị từ chối
		const bool bClaimAgain = DropletForA->TryClaimLootByUID(UID_A, ClaimedGold, ClaimedItems);
		TestFalse(TEXT("AC-6: Không thể nhặt lại túi đồ đã được nhận"), bClaimAgain);
	}

	// =========================================================================
	// Test 3: AC-8 (Party EXP Sharing with Morale Bonus & 3000 cm Radius Culling)
	// =========================================================================
	{
		UPALootDistributionSubsystem* LootSubsystem = NewObject<UPALootDistributionSubsystem>();
		const int32 BaseMonsterExp = 1000;
		const FVector MobDeathLocation = FVector(0.0f, 0.0f, 0.0f);

		// Kịch bản QA test case:
		// Tổ đội 4 người:
		// - Player 1: cự ly 500 cm (<= 3000 cm) -> Hợp lệ
		// - Player 2: cự ly 800 cm (<= 3000 cm) -> Hợp lệ
		// - Player 3: cự ly 1000 cm (<= 3000 cm) -> Hợp lệ
		// - Player 4: cự ly 3500 cm (> 3000 cm) -> Ngoài cự ly!
		TArray<FPAPartyMemberInfo> PartyMembers;
		PartyMembers.Add(FPAPartyMemberInfo(TEXT("Player_1"), FVector(500.0f, 0.0f, 0.0f)));
		PartyMembers.Add(FPAPartyMemberInfo(TEXT("Player_2"), FVector(0.0f, 800.0f, 0.0f)));
		PartyMembers.Add(FPAPartyMemberInfo(TEXT("Player_3"), FVector(707.0f, 707.0f, 0.0f))); // ~1000 cm
		PartyMembers.Add(FPAPartyMemberInfo(TEXT("Player_4"), FVector(3500.0f, 0.0f, 0.0f))); // 3500 cm (> 3000 cm)

		// Kiểm tra hệ số Morale Bonus của đội 4 người: +35% (1.35x)
		const float MoraleBonus4P = UPALootDistributionSubsystem::GetMoraleBonus(4);
		TestNearlyEqual(TEXT("AC-8: Đội 4 người phải có Morale Bonus = +35% (0.35f)"), MoraleBonus4P, 0.35f, 0.001f);

		// Phân phối EXP
		const TMap<FString, int32> ExpDistribution = LootSubsystem->DistributePartyExp(BaseMonsterExp, PartyMembers, MobDeathLocation);

		// Tính toán lý thuyết:
		// Tổng EXP = 1000 * (1 + 0.35) = 1350 EXP.
		// Số người hợp lệ trong cự ly = 3.
		// Mỗi người hợp lệ nhận: round(1350 / 3) = 450 EXP.
		// Player 4 (ngoài cự ly) nhận: 0 EXP.
		TestEqual(TEXT("AC-8: Player 1 nhận chính xác 450 EXP (kèm +35% bonus)"), ExpDistribution.FindRef(TEXT("Player_1")), 450);
		TestEqual(TEXT("AC-8: Player 2 nhận chính xác 450 EXP (kèm +35% bonus)"), ExpDistribution.FindRef(TEXT("Player_2")), 450);
		TestEqual(TEXT("AC-8: Player 3 nhận chính xác 450 EXP (kèm +35% bonus)"), ExpDistribution.FindRef(TEXT("Player_3")), 450);
		TestEqual(TEXT("AC-8: Player 4 đứng ở 3500 cm (> 3000 cm) PHẢI nhận đúng 0 EXP"), ExpDistribution.FindRef(TEXT("Player_4")), 0);

		// 3.2: Kiểm tra khi toàn bộ 4 thành viên đều nằm trong bán kính:
		PartyMembers[3].Location = FVector(2000.0f, 0.0f, 0.0f); // di chuyển vào trong 3000 cm
		const TMap<FString, int32> ExpDistributionAllIn = LootSubsystem->DistributePartyExp(BaseMonsterExp, PartyMembers, MobDeathLocation);
		// 1350 / 4 = 337.5 -> làm tròn 338 EXP
		TestEqual(TEXT("AC-8: Khi cả 4 người trong cự ly, mỗi người nhận 338 EXP"), ExpDistributionAllIn.FindRef(TEXT("Player_4")), 338);
	}

	// =========================================================================
	// Test 4: Performance & Guardrails (<= 1.0ms server game thread budget)
	// =========================================================================
	{
		UPALootDistributionSubsystem* LootSubsystem = NewObject<UPALootDistributionSubsystem>();
		const FString BenchmarkEncounter = TEXT("Encounter_RaidBoss_32P");
		const float BossMaxHP = 100000.0f;
		const float BossMaxPosture = 10000.0f;

		// Mô phỏng 32 người chơi cùng tham gia đánh boss với FGuid
		for (int32 i = 0; i < 32; ++i)
		{
			const FGuid PUID = FGuid(100, 200, 300, i);
			const FString PId = FString::Printf(TEXT("Player_%02d"), i);
			const float Dmg = 2000.0f + (i * 300.0f);
			const float PostureDmg = 100.0f + (i * 20.0f);
			LootSubsystem->RecordCombatContributionWithUID(BenchmarkEncounter, PUID, PId, nullptr, Dmg, PostureDmg);
		}

		const double StartTime = FPlatformTime::Seconds();
		const TArray<FGuid> Eligible = LootSubsystem->GetEligiblePlayerUIDsForLoot(BenchmarkEncounter, BossMaxHP, BossMaxPosture);
		const double ElapsedMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;

		TestTrue(TEXT("Guardrail: Tính toán điều kiện nhận Loot 32 người phải <= 1.0ms"), ElapsedMs <= 1.0);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
