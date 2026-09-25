// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Character/PACivilianNPCComponent.h"
#include "Character/PACivilianNPCTypes.h"
#include "Character/PABaseCharacter.h"
#include "Character/PAPaperdollComponent.h"
#include "PaperFlipbookComponent.h"
#include "World/PACitadelComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPACivilianNPCTest
 *
 * Kiểm thử tự động cho Story visual-003 (EPIC-CHARACTER-VISUAL-001, Sprint 7):
 * - AC-1: 5 Vai trò NPC Dân sự & Rig Mapping (Blacksmith/TownGuard -> HeavyTank, Merchant/Villager -> Agility, QuestGiver -> Caster).
 * - AC-2: Dynamic Palette Swap Material (Verdant Bastion, Ashen Keep, Sanctum Fortress - 3 bảng màu riêng).
 * - AC-3: 10 Đạo cụ tĩnh gắn tay (HandSocket_R/L) & Callout Quest Giver ('!' vàng và vẫy tay).
 * - AC-4: Town Guard AI Dual-State (Passive -> Lethal Strike khi phát hiện Outlaw Karma < 0 trong 1000cm, tái dùng 100% FB_Upper_2H_Polearm_Combo).
 * - AC-5: Headless Safe 100%, tích hợp hoàn hảo với zone-001 Citadel và regression suite.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPACivilianNPCTest,
	"ProjectAscendant.CharacterVisual.CivilianNPCAndTownGuardAI",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPACivilianNPCTest::RunTest(const FString& Parameters)
{
	// =========================================================================
	// AC-1: 5 Vai Trò NPC Dân Sự & Master Rig Mapping
	// =========================================================================
	{
		UPACivilianNPCComponent* Comp = NewObject<UPACivilianNPCComponent>();
		TestNotNull(TEXT("AC-1: UPACivilianNPCComponent instantiated"), Comp);

		// 1. Thợ Rèn (Blacksmith) -> HeavyTank
		Comp->SetCivilianRole(EPACivilianRole::Blacksmith);
		TestEqual(TEXT("AC-1: Blacksmith role set"), Comp->GetCivilianRole(), EPACivilianRole::Blacksmith);
		TestEqual(TEXT("AC-1: Blacksmith uses HeavyTank Rig"), Comp->GetMasterRig(), EPAMasterRig::HeavyTank);
		TestEqual(TEXT("AC-1: Blacksmith LowerBody asset is HeavyTank"),
			Comp->GetActiveLowerBodyVisualAssetId(), FName(TEXT("FB_Lower_HeavyTank_Set")));

		// 2. Lính Gác (TownGuard) -> HeavyTank
		Comp->SetCivilianRole(EPACivilianRole::TownGuard);
		TestEqual(TEXT("AC-1: TownGuard role set"), Comp->GetCivilianRole(), EPACivilianRole::TownGuard);
		TestEqual(TEXT("AC-1: TownGuard uses HeavyTank Rig"), Comp->GetMasterRig(), EPAMasterRig::HeavyTank);
		TestEqual(TEXT("AC-1: TownGuard LowerBody asset is HeavyTank"),
			Comp->GetActiveLowerBodyVisualAssetId(), FName(TEXT("FB_Lower_HeavyTank_Set")));

		// 3. Thương Nhân (Merchant) -> Agility
		Comp->SetCivilianRole(EPACivilianRole::Merchant);
		TestEqual(TEXT("AC-1: Merchant role set"), Comp->GetCivilianRole(), EPACivilianRole::Merchant);
		TestEqual(TEXT("AC-1: Merchant uses Agility Rig"), Comp->GetMasterRig(), EPAMasterRig::Agility);
		TestEqual(TEXT("AC-1: Merchant LowerBody asset is Agility"),
			Comp->GetActiveLowerBodyVisualAssetId(), FName(TEXT("FB_Lower_Agility_Set")));

		// 4. Dân Làng Nền (Villager) -> Agility
		Comp->SetCivilianRole(EPACivilianRole::Villager);
		TestEqual(TEXT("AC-1: Villager role set"), Comp->GetCivilianRole(), EPACivilianRole::Villager);
		TestEqual(TEXT("AC-1: Villager uses Agility Rig"), Comp->GetMasterRig(), EPAMasterRig::Agility);
		TestEqual(TEXT("AC-1: Villager LowerBody asset is Agility"),
			Comp->GetActiveLowerBodyVisualAssetId(), FName(TEXT("FB_Lower_Agility_Set")));

		// 5. Quest Giver -> Caster
		Comp->SetCivilianRole(EPACivilianRole::QuestGiver);
		TestEqual(TEXT("AC-1: QuestGiver role set"), Comp->GetCivilianRole(), EPACivilianRole::QuestGiver);
		TestEqual(TEXT("AC-1: QuestGiver uses Caster Rig"), Comp->GetMasterRig(), EPAMasterRig::Caster);
		TestEqual(TEXT("AC-1: QuestGiver LowerBody asset is Caster"),
			Comp->GetActiveLowerBodyVisualAssetId(), FName(TEXT("FB_Lower_Caster_Set")));
	}

	// =========================================================================
	// AC-2: Dynamic Palette Swap Material (3 Vùng Bản Đồ)
	// =========================================================================
	{
		UPACivilianNPCComponent* Comp = NewObject<UPACivilianNPCComponent>();

		// 1. Verdant Bastion (Vùng 1: Tiền Trạm - Nâu da bò & Xanh rêu)
		Comp->SetZonePalette(FName(TEXT("Citadel_VerdantBastion")));
		TestEqual(TEXT("AC-2: Verdant Zone set"), Comp->GetCurrentZoneId(), FName(TEXT("Citadel_VerdantBastion")));
		const FPACivilianPalettePreset& VerdantPalette = Comp->GetCurrentPalettePreset();
		TestEqual(TEXT("AC-2: Verdant PresetName"), VerdantPalette.PresetName, FName(TEXT("Verdant_Bastion")));
		TestEqual(TEXT("AC-2: Verdant Primary Color is #8D6E63"), VerdantPalette.PrimaryColor, FLinearColor(0.553f, 0.431f, 0.388f, 1.0f));
		TestEqual(TEXT("AC-2: Verdant Secondary Color is #4E6E58"), VerdantPalette.SecondaryColor, FLinearColor(0.306f, 0.431f, 0.345f, 1.0f));

		// 2. Ashen Keep (Vùng 2: Hoang Dã Tro Tàn - Xám tro & Đỏ gạch)
		Comp->SetZonePalette(FName(TEXT("Citadel_AshenKeep")));
		TestEqual(TEXT("AC-2: Ashen Zone set"), Comp->GetCurrentZoneId(), FName(TEXT("Citadel_AshenKeep")));
		const FPACivilianPalettePreset& AshenPalette = Comp->GetCurrentPalettePreset();
		TestEqual(TEXT("AC-2: Ashen PresetName"), AshenPalette.PresetName, FName(TEXT("Ashen_Keep")));
		TestEqual(TEXT("AC-2: Ashen Primary Color is #424242"), AshenPalette.PrimaryColor, FLinearColor(0.259f, 0.259f, 0.259f, 1.0f));
		TestEqual(TEXT("AC-2: Ashen Secondary Color is #8D2B2B"), AshenPalette.SecondaryColor, FLinearColor(0.553f, 0.169f, 0.169f, 1.0f));

		// 3. Sanctum Fortress (Vùng 3: Cấm Địa - Trắng ngà & Lam ngọc)
		Comp->SetZonePalette(FName(TEXT("Citadel_SanctumFortress")));
		TestEqual(TEXT("AC-2: Sanctum Zone set"), Comp->GetCurrentZoneId(), FName(TEXT("Citadel_SanctumFortress")));
		const FPACivilianPalettePreset& SanctumPalette = Comp->GetCurrentPalettePreset();
		TestEqual(TEXT("AC-2: Sanctum PresetName"), SanctumPalette.PresetName, FName(TEXT("Sanctum_Fortress")));
		TestEqual(TEXT("AC-2: Sanctum Primary Color is #F5F5F5"), SanctumPalette.PrimaryColor, FLinearColor(0.961f, 0.961f, 0.961f, 1.0f));
		TestEqual(TEXT("AC-2: Sanctum Secondary Color is #1A237E"), SanctumPalette.SecondaryColor, FLinearColor(0.102f, 0.137f, 0.494f, 1.0f));

		// 4. Kiểm tra đồng bộ từ UPACitadelComponent (Dependency zone-001)
		UPACitadelComponent* CitadelComp = NewObject<UPACitadelComponent>();
		CitadelComp->InitializeCitadel(FName(TEXT("Citadel_SanctumFortress")), TEXT("Sanctum Fortress"), EPAZoneTier::Tier3_ForbiddenSanctum);
		Comp->SetZonePaletteFromCitadel(CitadelComp);
		TestEqual(TEXT("AC-2: Synced palette from UPACitadelComponent"), Comp->GetCurrentZoneId(), FName(TEXT("Citadel_SanctumFortress")));
	}

	// =========================================================================
	// AC-3: 10 Đạo Cụ Tĩnh Gắn Tay & Callout Quest Giver
	// =========================================================================
	{
		UPACivilianNPCComponent* Comp = NewObject<UPACivilianNPCComponent>();

		// 1. Thợ Rèn: Búa rèn (phải) & Kẹp rèn (trái)
		Comp->SetCivilianRole(EPACivilianRole::Blacksmith);
		TestEqual(TEXT("AC-3: Blacksmith Right Prop is Hammer"),
			Comp->GetAttachedProp(false), FPACivilianConstants::Prop_Blacksmith_Hammer);
		TestEqual(TEXT("AC-3: Blacksmith Left Prop is Tongs"),
			Comp->GetAttachedProp(true), FPACivilianConstants::Prop_Tongs);

		// 2. Thương Nhân: Túi tiền (phải) & Cân tiểu ly (trái)
		Comp->SetCivilianRole(EPACivilianRole::Merchant);
		TestEqual(TEXT("AC-3: Merchant Right Prop is GoldPouch"),
			Comp->GetAttachedProp(false), FPACivilianConstants::Prop_Merchant_GoldPouch);
		TestEqual(TEXT("AC-3: Merchant Left Prop is Scale"),
			Comp->GetAttachedProp(true), FPACivilianConstants::Prop_Merchant_Scale);

		// 3. Dân Làng: Giỏ lương thực (phải)
		Comp->SetCivilianRole(EPACivilianRole::Villager);
		TestEqual(TEXT("AC-3: Villager Right Prop is Basket"),
			Comp->GetAttachedProp(false), FPACivilianConstants::Prop_Villager_Basket);
		// Gắn thử chổi quét
		Comp->AttachCivilianProp(FPACivilianConstants::Prop_Villager_Broom, false);
		TestEqual(TEXT("AC-3: Villager Right Prop swapped to Broom"),
			Comp->GetAttachedProp(false), FPACivilianConstants::Prop_Villager_Broom);

		// 4. Lính Gác: Giáo thành (phải) & Khiên thành (trái)
		Comp->SetCivilianRole(EPACivilianRole::TownGuard);
		TestEqual(TEXT("AC-3: Guard Right Prop is CitySpear"),
			Comp->GetAttachedProp(false), FPACivilianConstants::Prop_Guard_CitySpear);
		TestEqual(TEXT("AC-3: Guard Left Prop is CityShield"),
			Comp->GetAttachedProp(true), FPACivilianConstants::Prop_Guard_CityShield);

		// 5. Quest Giver: Cuộn thư cổ (phải) & Đèn lồng (trái)
		Comp->SetCivilianRole(EPACivilianRole::QuestGiver);
		TestEqual(TEXT("AC-3: QuestGiver Right Prop is Scroll"),
			Comp->GetAttachedProp(false), FPACivilianConstants::Prop_Quest_Scroll);
		TestEqual(TEXT("AC-3: QuestGiver Left Prop is Lantern"),
			Comp->GetAttachedProp(true), FPACivilianConstants::Prop_Quest_Lantern);

		// 6. Test Callout Animation & Icon '!'
		TestFalse(TEXT("AC-3: Callout inactive initially"), Comp->IsCalloutActive());
		Comp->PlayCalloutAnimation();
		TestTrue(TEXT("AC-3: Callout active"), Comp->IsCalloutActive());
		TestEqual(TEXT("AC-3: Callout VFX is Exclamation mark"),
			Comp->GetActiveCalloutVisualId(), FPACivilianConstants::VFX_Quest_Exclamation);
		TestEqual(TEXT("AC-3: Upper Body animation is QuestGiver Wave"),
			Comp->GetActiveUpperBodyVisualAssetId(), FPACivilianConstants::FB_QuestGiver_Wave);

		Comp->StopCalloutAnimation();
		TestFalse(TEXT("AC-3: Callout stopped"), Comp->IsCalloutActive());
		TestTrue(TEXT("AC-3: Callout VFX cleared"), Comp->GetActiveCalloutVisualId().IsNone());
	}

	// =========================================================================
	// AC-4: Town Guard AI Dual-State & Outlaw Deterrence
	// =========================================================================
	{
		UPACivilianNPCComponent* GuardComp = NewObject<UPACivilianNPCComponent>();
		GuardComp->SetCivilianRole(EPACivilianRole::TownGuard);

		TestEqual(TEXT("AC-4: Initial Guard State is Passive"), GuardComp->GetGuardAIState(), EPAGuardAIState::Passive);
		TestFalse(TEXT("AC-4: Lethal strike not triggered initially"), GuardComp->IsLethalStrikeTriggered());

		// Giả lập Target Player 1: Người chơi lương thiện (Karma = +50, không phải Outlaw)
		APABaseCharacter* InnocentPlayer = NewObject<APABaseCharacter>();
		InnocentPlayer->SetKarma(50.0f);
		TestFalse(TEXT("AC-4: Innocent player is not Outlaw"), InnocentPlayer->IsOutlaw());

		// Đặt vị trí trong bán kính 500cm
		const FVector GuardLoc(0.0f, 0.0f, 0.0f);
		const FVector NearbyLoc(500.0f, 0.0f, 0.0f);

		// Đánh giá đe dọa với người chơi lương thiện
		FPACivilianNPCModel& GuardModel = GuardComp->GetMutableModel();
		const bool bInnocentThreat = GuardModel.EvaluateTargetThreat(GuardLoc, NearbyLoc, InnocentPlayer->IsOutlaw(), 1000.0f);
		TestFalse(TEXT("AC-4: Guard ignores innocent player"), bInnocentThreat);
		TestEqual(TEXT("AC-4: Guard remains Passive"), GuardComp->GetGuardAIState(), EPAGuardAIState::Passive);

		// Giả lập Target Player 2: Kẻ ngoài vòng pháp luật (Outlaw Karma = -50)
		APABaseCharacter* OutlawPlayer = NewObject<APABaseCharacter>();
		OutlawPlayer->SetKarma(-50.0f);
		TestTrue(TEXT("AC-4: Outlaw player identified"), OutlawPlayer->IsOutlaw());

		// Bước 1: Outlaw ở khoảng cách 1500cm (ngoài bán kính 1000cm)
		const FVector DistantLoc(1500.0f, 0.0f, 0.0f);
		const bool bDistantThreat = GuardModel.EvaluateTargetThreat(GuardLoc, DistantLoc, OutlawPlayer->IsOutlaw(), 1000.0f);
		TestFalse(TEXT("AC-4: Outlaw outside 1000cm not engaged"), bDistantThreat);
		TestEqual(TEXT("AC-4: Guard still Passive"), GuardComp->GetGuardAIState(), EPAGuardAIState::Passive);

		// Bước 2: Outlaw tiến vào phạm vi 800cm (trong bán kính 1000cm)
		const FVector CloseLoc(800.0f, 0.0f, 0.0f);
		const bool bCloseThreat = GuardModel.EvaluateTargetThreat(GuardLoc, CloseLoc, OutlawPlayer->IsOutlaw(), 1000.0f);
		TestTrue(TEXT("AC-4: Outlaw in 800cm triggers threat"), bCloseThreat);

		// Xác nhận Town Guard chuyển sang CombatActive
		TestEqual(TEXT("AC-4: Guard State switches to CombatActive"), GuardComp->GetGuardAIState(), EPAGuardAIState::CombatActive);
		TestTrue(TEXT("AC-4: Lethal Strike triggered"), GuardComp->IsLethalStrikeTriggered());

		// Xác nhận TÁI DÙNG 100% FB_Upper_2H_Polearm_Combo (Weapon Family 3)
		TestEqual(TEXT("AC-4: Upper Body animation reuses FB_Upper_2H_Polearm_Combo"),
			GuardComp->GetActiveUpperBodyVisualAssetId(), FName(TEXT("FB_Upper_2H_Polearm_Combo")));
		TestEqual(TEXT("AC-4: Right prop remains CitySpear"),
			GuardComp->GetAttachedProp(false), FPACivilianConstants::Prop_Guard_CitySpear);

		// Thực hiện đòn đánh Knockback lên Outlaw
		const bool bStrikeExecuted = GuardComp->ExecuteLethalGatekeeperStrike(OutlawPlayer);
		TestTrue(TEXT("AC-4: ExecuteLethalGatekeeperStrike succeeded"), bStrikeExecuted);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
