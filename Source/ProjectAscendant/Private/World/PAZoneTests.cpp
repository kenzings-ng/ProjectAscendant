// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "World/PAZoneTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAZoneTests
 *
 * Kiểm thử tự động cho Story zone-001 (3-Tier Seamless Zones, Campfire Sanctuaries & AI Leash):
 *  - AC-1: Campfire Sanctuary Boundary (1000cm bán kính, chỉ kích hoạt khi đã thắp sáng).
 *  - AC-2: Rest & Restoration (Hồi 100% HP, MP, Stamina, đầy 5/5 Flask, ghim SpawnAnchor).
 *  - AC-3: Fast Travel Teleportation (Niệm chú 2.0s, hủy khi di chuyển hoặc nhận sát thương).
 *  - AC-4: AI Leash Boundary (2500cm cự ly xích quái khỏi vị trí sinh).
 *
 * Kiểm thử chạy trên FPAZoneModel thuần túy (không phụ thuộc World runtime).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAZoneTests,
	"ProjectAscendant.World.SeamlessZonesCampfires",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
	constexpr float kTolerance = 0.02f;
}

// ---------------------------------------------------------------------------
// RunTest
// ---------------------------------------------------------------------------

bool FPAZoneTests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: Campfire Sanctuary Boundary (1000cm)
	// ===========================================================
	{
		FPAZoneModel Model;

		// Đăng ký Campfire 1 tại (0, 0, 0) - Chưa thắp sáng
		Model.RegisterCampfire(FName("Campfire_Outpost"), EPAZoneTier::Tier1_VerdantFrontier, FVector(0.0f, 0.0f, 0.0f), false);

		FName OutId = NAME_None;
		// Đứng cách 500cm nhưng chưa thắp sáng -> Không có Thánh địa
		bool bInsideUnlit = Model.IsInsideSanctuary(FVector(500.0f, 0.0f, 0.0f), OutId);
		TestFalse(TEXT("AC1: Unlit campfire does not grant sanctuary"), bInsideUnlit);

		// Thắp sáng Campfire
		bool bLit = Model.LightCampfire(FName("Campfire_Outpost"));
		TestTrue(TEXT("AC1: LightCampfire succeeds"), bLit);

		// Đứng cách 800cm (< 1000cm) -> Trong Thánh địa
		bool bInside = Model.IsInsideSanctuary(FVector(800.0f, 0.0f, 0.0f), OutId);
		TestTrue(TEXT("AC1: Position at 800cm is inside sanctuary"), bInside);
		TestEqual(TEXT("AC1: OutCampfireId is Campfire_Outpost"), OutId, FName("Campfire_Outpost"));

		// Đứng cách 1200cm (> 1000cm) -> Ngoài Thánh địa
		bool bOutside = Model.IsInsideSanctuary(FVector(1200.0f, 0.0f, 0.0f), OutId);
		TestFalse(TEXT("AC1: Position at 1200cm is outside sanctuary"), bOutside);
	}

	// ===========================================================
	// AC-2: Rest & Restoration
	// ===========================================================
	{
		FPAZoneModel Model;
		Model.RegisterCampfire(FName("Campfire_Outpost"), EPAZoneTier::Tier1_VerdantFrontier, FVector::ZeroVector, true);

		float HP = 0.20f;
		float MP = 0.10f;
		float Stamina = 0.0f;
		int32 Flasks = 1;

		bool bRestSuccess = Model.RestAtCampfire(FName("Campfire_Outpost"), HP, MP, Stamina, Flasks);
		TestTrue(TEXT("AC2: RestAtCampfire succeeds"), bRestSuccess);
		TestNearlyEqual(TEXT("AC2: HP restored to 100% (1.0)"), HP, 1.0f, kTolerance);
		TestNearlyEqual(TEXT("AC2: MP restored to 100% (1.0)"), MP, 1.0f, kTolerance);
		TestNearlyEqual(TEXT("AC2: Stamina restored to 100% (1.0)"), Stamina, 1.0f, kTolerance);
		TestEqual(TEXT("AC2: Flasks refilled to 5/5"), Flasks, 5);
		TestEqual(TEXT("AC2: SpawnAnchor set to Campfire_Outpost"), Model.CurrentSpawnAnchorId, FName("Campfire_Outpost"));
	}

	// ===========================================================
	// AC-3: Fast Travel Teleportation (2.0s Cast)
	// ===========================================================
	{
		FPAZoneModel Model;
		Model.RegisterCampfire(FName("CF_1"), EPAZoneTier::Tier1_VerdantFrontier, FVector(0, 0, 0), true);
		Model.RegisterCampfire(FName("CF_2"), EPAZoneTier::Tier2_AshenWilderness, FVector(50000, 0, 0), true);
		Model.RegisterCampfire(FName("CF_Unlit"), EPAZoneTier::Tier3_ForbiddenSanctum, FVector(100000, 0, 0), false);

		// Không thể dịch chuyển đến điểm chưa thắp sáng
		bool bFailUnlit = Model.StartFastTravel(FName("CF_1"), FName("CF_Unlit"), false);
		TestFalse(TEXT("AC3: Cannot fast travel to unlit campfire"), bFailUnlit);

		// Không thể dịch chuyển khi đang trong chiến đấu
		bool bFailCombat = Model.StartFastTravel(FName("CF_1"), FName("CF_2"), true);
		TestFalse(TEXT("AC3: Cannot fast travel while in combat"), bFailCombat);

		// Bắt đầu niệm chú hợp lệ (CF_1 -> CF_2)
		bool bStarted = Model.StartFastTravel(FName("CF_1"), FName("CF_2"), false);
		TestTrue(TEXT("AC3: Fast travel channel started"), bStarted);
		TestTrue(TEXT("AC3: bIsChannelingFastTravel is true"), Model.bIsChannelingFastTravel);
		TestEqual(TEXT("AC3: State is FastTraveling"), Model.CurrentState, EPACampfireState::FastTraveling);

		// Tiến 1.0s (chưa đủ 2.0s)
		bool bFinishedMidway = Model.UpdateFastTravel(1.0f, false, false);
		TestFalse(TEXT("AC3: Fast travel not complete at 1.0s"), bFinishedMidway);

		// Thử nghiệm ngắt niệm chú khi bị đánh (Damage = true)
		Model.UpdateFastTravel(0.1f, false, true);
		TestFalse(TEXT("AC3: Fast travel cancelled on damage"), Model.bIsChannelingFastTravel);

		// Khởi động lại và niệm chú hoàn tất đủ 2.0s
		Model.StartFastTravel(FName("CF_1"), FName("CF_2"), false);
		Model.UpdateFastTravel(1.0f, false, false);
		bool bCompleted = Model.UpdateFastTravel(1.05f, false, false); // Tổng 2.05s > 2.0s
		TestTrue(TEXT("AC3: Fast travel completed after 2.0s"), bCompleted);
		TestFalse(TEXT("AC3: No longer channeling after completion"), Model.bIsChannelingFastTravel);
		TestEqual(TEXT("AC3: Spawn anchor updated to destination CF_2"), Model.CurrentSpawnAnchorId, FName("CF_2"));
	}

	// ===========================================================
	// AC-4: AI Leash Boundary (2500cm)
	// ===========================================================
	{
		FPAZoneModel Model;
		const FVector SpawnOrigin(1000.0f, 1000.0f, 0.0f);

		// Quái vật ở khoảng cách 1500cm so với gốc (< 2500cm)
		const FVector PosInside(2500.0f, 1000.0f, 0.0f);
		TestFalse(TEXT("AC4: 1500cm is within leash"), Model.IsBeyondLeash(PosInside, SpawnOrigin));

		// Quái vật bị kéo đi 2600cm (> 2500cm)
		const FVector PosBeyond(3600.0f, 1000.0f, 0.0f);
		TestTrue(TEXT("AC4: 2600cm is beyond leash boundary"), Model.IsBeyondLeash(PosBeyond, SpawnOrigin));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
