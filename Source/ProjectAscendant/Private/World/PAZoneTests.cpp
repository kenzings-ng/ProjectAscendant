// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "World/PAZoneTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAZoneTests
 *
 * Kiểm thử tự động cho Tầng Phân Vùng & Tòa Thành An Toàn (Citadel Safe Zones, Auto-Save & Relog):
 *  - AC-1: Citadel Safe Zone Boundary (Bán kính vùng an toàn của tòa thành, cấm PvP, drop aggro).
 *  - AC-2: Citadel Enter Auto-Save (Bước vào thành lập tức kích hoạt Auto-Save, hồi 100% tài nguyên, cập nhật LastVisitedCitadel).
 *  - AC-3: Relog Return to Last Visited Citadel (Thoát game ở hoang dã, khi đăng nhập lại luôn đưa về Tòa thành gần nhất).
 *  - AC-4: Citadel Fast Travel (Niệm chú 2.0s giữa các thành trì đã khám phá, ngắt khi di chuyển/chịu sát thương).
 *  - AC-5: AI Leash Boundary (2500cm cự ly xích quái khỏi vị trí sinh).
 *
 * Kiểm thử chạy trên FPAZoneModel thuần túy (100% headless testable).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAZoneTests,
	"ProjectAscendant.World.CitadelSafeZonesAndAutoSave",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
	constexpr float kZoneTolerance = 0.02f;
}

// ---------------------------------------------------------------------------
// RunTest
// ---------------------------------------------------------------------------

bool FPAZoneTests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: Citadel Safe Zone Boundary
	// ===========================================================
	{
		FPAZoneModel Model;

		// Đăng ký Thành Tân Thủ (Tier 1: Verdant Bastion) tại (0, 0, 0), bán kính Safe Zone 5000cm
		Model.RegisterCitadel(
			FName("Citadel_VerdantBastion"),
			TEXT("Verdant Bastion"),
			EPAZoneTier::Tier1_VerdantFrontier,
			FVector(0.0f, 0.0f, 0.0f),
			5000.0f
		);

		FName OutCitadelId = NAME_None;

		// Đứng tại khoảng cách 2500cm (< 5000cm) -> Trong Vùng An Toàn
		bool bInside = Model.IsInsideSafeZone(FVector(2500.0f, 0.0f, 0.0f), OutCitadelId);
		TestTrue(TEXT("AC1: Position at 2500cm is inside Citadel Safe Zone"), bInside);
		TestEqual(TEXT("AC1: OutCitadelId is Citadel_VerdantBastion"), OutCitadelId, FName("Citadel_VerdantBastion"));

		// Đứng tại khoảng cách 6000cm (> 5000cm) -> Ngoài Vùng An Toàn (ngoài hoang dã)
		bool bOutside = Model.IsInsideSafeZone(FVector(6000.0f, 0.0f, 0.0f), OutCitadelId);
		TestFalse(TEXT("AC1: Position at 6000cm is outside Safe Zone"), bOutside);
		TestTrue(TEXT("AC1: OutCitadelId is None outside Safe Zone"), OutCitadelId.IsNone());
	}

	// ===========================================================
	// AC-2: Citadel Enter Auto-Save & Resource Restoration
	// ===========================================================
	{
		FPAZoneModel Model;
		Model.RegisterCitadel(
			FName("Citadel_VerdantBastion"),
			TEXT("Verdant Bastion"),
			EPAZoneTier::Tier1_VerdantFrontier,
			FVector(0.0f, 0.0f, 0.0f),
			5000.0f
		);

		const FString PlayerId = TEXT("Player_Hero_01");
		const FVector PlayerPosEntering(1000.0f, 0.0f, 0.0f); // Bước vào cổng thành
		const float CurrentTime = 120.5f;

		FPAAutoSaveRecord SaveRecord;
		float HP = 0.15f;      // Đang thoi thóp 15% HP
		float MP = 0.05f;      // Cạn kiệt 5% MP
		float Stamina = 0.0f;  // Hết thể lực
		int32 Flasks = 0;      // Hết bình thuốc

		bool bEnteredAndSaved = Model.EnterCitadel(
			PlayerId,
			PlayerPosEntering,
			CurrentTime,
			SaveRecord,
			HP,
			MP,
			Stamina,
			Flasks
		);

		TestTrue(TEXT("AC2: EnterCitadel triggers Auto-Save successfully"), bEnteredAndSaved);
		TestTrue(TEXT("AC2: SaveRecord marked bSavedSuccessfully"), SaveRecord.bSavedSuccessfully);
		TestEqual(TEXT("AC2: SaveRecord PlayerId matches"), SaveRecord.PlayerId, PlayerId);
		TestEqual(TEXT("AC2: SaveRecord LastVisitedCitadelId is VerdantBastion"), SaveRecord.LastVisitedCitadelId, FName("Citadel_VerdantBastion"));
		TestNearlyEqual(TEXT("AC2: SaveRecord Timestamp matches"), SaveRecord.AutoSaveTimestamp, 120.5f, kZoneTolerance);

		// Kiểm tra hồi phục trọn vẹn 100% tài nguyên
		TestNearlyEqual(TEXT("AC2: HP restored to 100%"), HP, 1.0f, kZoneTolerance);
		TestNearlyEqual(TEXT("AC2: MP restored to 100%"), MP, 1.0f, kZoneTolerance);
		TestNearlyEqual(TEXT("AC2: Stamina restored to 100%"), Stamina, 1.0f, kZoneTolerance);
		TestEqual(TEXT("AC2: Flasks refilled to 5/5"), Flasks, 5);

		// Kiểm tra trạng thái Model
		TestEqual(TEXT("AC2: Model LastVisitedCitadelId updated"), Model.LastVisitedCitadelId, FName("Citadel_VerdantBastion"));
		const FPACitadelNode* Node = Model.FindCitadel(FName("Citadel_VerdantBastion"));
		TestNotNull(TEXT("AC2: Citadel node found"), Node);
		if (Node)
		{
			TestTrue(TEXT("AC2: Citadel marked as discovered"), Node->bIsDiscovered);
			TestTrue(TEXT("AC2: Citadel marked as last visited"), Node->bIsLastVisited);
		}
	}

	// ===========================================================
	// AC-3: Relog Always Returns To Last Visited Citadel
	// ===========================================================
	{
		FPAZoneModel Model;
		const FVector BastionLoc(0.0f, 0.0f, 0.0f);
		const FVector AshenKeepLoc(50000.0f, 50000.0f, 0.0f);

		Model.RegisterCitadel(FName("Citadel_VerdantBastion"), TEXT("Verdant Bastion"), EPAZoneTier::Tier1_VerdantFrontier, BastionLoc, 5000.0f);
		Model.RegisterCitadel(FName("Citadel_AshenKeep"), TEXT("Ashen Keep"), EPAZoneTier::Tier2_AshenWilderness, AshenKeepLoc, 5000.0f);

		const FString PlayerId = TEXT("Player_Hero_01");

		// Bước 1: Người chơi ban đầu ở Verdant Bastion
		FPAAutoSaveRecord Save1;
		float HP, MP, Stamina;
		int32 Flasks;
		Model.EnterCitadel(PlayerId, FVector(500.0f, 0.0f, 0.0f), 10.0f, Save1, HP, MP, Stamina, Flasks);

		// Bước 2: Người chơi di chuyển sang Vùng 2 và bước vào Ashen Keep -> Auto-Save Ashen Keep
		FPAAutoSaveRecord Save2;
		Model.EnterCitadel(PlayerId, AshenKeepLoc + FVector(200.0f, 0.0f, 0.0f), 350.0f, Save2, HP, MP, Stamina, Flasks);
		TestEqual(TEXT("AC3: Last visited updated to AshenKeep"), Model.LastVisitedCitadelId, FName("Citadel_AshenKeep"));

		// Bước 3: Người chơi đi ra ngoài hoang dã đánh quái tại (75000, 75000, 0) và THOÁT GAME (Quit / Disconnect)
		// Khi đăng nhập lại (Relog): Máy chủ truy vấn vị trí spawn của người chơi
		FVector RelogSpawnLoc = FVector::ZeroVector;
		FName SpawnCitadelId = NAME_None;
		bool bGotSpawn = Model.GetRelogSpawnTransform(PlayerId, RelogSpawnLoc, SpawnCitadelId);

		TestTrue(TEXT("AC3: GetRelogSpawnTransform returns true"), bGotSpawn);
		TestEqual(TEXT("AC3: Spawn Citadel is Citadel_AshenKeep (most recently entered)"), SpawnCitadelId, FName("Citadel_AshenKeep"));
		TestNearlyEqual(TEXT("AC3: Spawn location X matches Ashen Keep"), (float)RelogSpawnLoc.X, (float)AshenKeepLoc.X, kZoneTolerance);
		TestNearlyEqual(TEXT("AC3: Spawn location Y matches Ashen Keep"), (float)RelogSpawnLoc.Y, (float)AshenKeepLoc.Y, kZoneTolerance);
	}

	// ===========================================================
	// AC-4: Fast Travel Between Citadels (2.0s Cast)
	// ===========================================================
	{
		FPAZoneModel Model;
		const FVector BastionLoc(0.0f, 0.0f, 0.0f);
		const FVector AshenKeepLoc(50000.0f, 50000.0f, 0.0f);
		const FVector SanctumLoc(120000.0f, 120000.0f, 0.0f);

		Model.RegisterCitadel(FName("Citadel_1"), TEXT("Bastion"), EPAZoneTier::Tier1_VerdantFrontier, BastionLoc, 5000.0f);
		Model.RegisterCitadel(FName("Citadel_2"), TEXT("Keep"), EPAZoneTier::Tier2_AshenWilderness, AshenKeepLoc, 5000.0f);
		Model.RegisterCitadel(FName("Citadel_Undiscovered"), TEXT("Sanctum"), EPAZoneTier::Tier3_ForbiddenSanctum, SanctumLoc, 5000.0f);

		// Khám phá Citadel 1 & 2
		FPACitadelNode* C1 = Model.FindCitadel(FName("Citadel_1"));
		FPACitadelNode* C2 = Model.FindCitadel(FName("Citadel_2"));
		if (C1) C1->bIsDiscovered = true;
		if (C2) C2->bIsDiscovered = true;

		// Không thể dịch chuyển tới thành chưa từng khám phá
		bool bFailUndiscovered = Model.StartCitadelFastTravel(FName("Citadel_1"), FName("Citadel_Undiscovered"), false);
		TestFalse(TEXT("AC4: Cannot fast travel to undiscovered Citadel"), bFailUndiscovered);

		// Không thể dịch chuyển khi đang giao tranh (In Combat)
		bool bFailCombat = Model.StartCitadelFastTravel(FName("Citadel_1"), FName("Citadel_2"), true);
		TestFalse(TEXT("AC4: Cannot fast travel while in combat"), bFailCombat);

		// Bắt đầu niệm chú dịch chuyển hợp lệ (2.0s)
		bool bStarted = Model.StartCitadelFastTravel(FName("Citadel_1"), FName("Citadel_2"), false);
		TestTrue(TEXT("AC4: Fast travel channeling started"), bStarted);
		TestTrue(TEXT("AC4: bIsChannelingFastTravel is true"), Model.bIsChannelingFastTravel);

		// Bị ngắt khi nhận sát thương
		Model.UpdateFastTravel(0.5f, false, true);
		TestFalse(TEXT("AC4: Channel interrupted on damage"), Model.bIsChannelingFastTravel);

		// Khởi động lại và niệm chú hoàn tất đủ 2.0s
		Model.StartCitadelFastTravel(FName("Citadel_1"), FName("Citadel_2"), false);
		Model.UpdateFastTravel(1.0f, false, false);
		bool bCompleted = Model.UpdateFastTravel(1.05f, false, false); // Tổng 2.05s >= 2.0s
		TestTrue(TEXT("AC4: Fast travel completed after 2.0s"), bCompleted);
		TestFalse(TEXT("AC4: No longer channeling after completion"), Model.bIsChannelingFastTravel);
		TestEqual(TEXT("AC4: Last visited updated to Citadel_2"), Model.LastVisitedCitadelId, FName("Citadel_2"));
	}

	// ===========================================================
	// AC-5: AI Leash Boundary (2500cm)
	// ===========================================================
	{
		FPAZoneModel Model;
		const FVector SpawnOrigin(1000.0f, 1000.0f, 0.0f);

		// Quái vật ở khoảng cách 1500cm so với gốc (< 2500cm)
		const FVector PosInside(2500.0f, 1000.0f, 0.0f);
		TestFalse(TEXT("AC5: 1500cm is within leash"), Model.IsBeyondLeash(PosInside, SpawnOrigin));

		// Quái vật bị kéo đi 2600cm (> 2500cm)
		const FVector PosBeyond(3600.0f, 1000.0f, 0.0f);
		TestTrue(TEXT("AC5: 2600cm is beyond leash boundary"), Model.IsBeyondLeash(PosBeyond, SpawnOrigin));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
