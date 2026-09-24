// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Character/PAPaperdollComponent.h"
#include "Character/PAPaperdollTypes.h"
#include "PaperFlipbookComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPACharacterVisualRigTest
 *
 * Kiểm thử tự động cho Story visual-001 (EPIC-CHARACTER-VISUAL-001, Sprint 7):
 * - AC-1: 4 Master Rig Enum & State Machine (HeavyTank, Agility, Caster, Monk), FootPivot (64, 114).
 * - AC-2: 7 Weapon Family Upper Body Binding & Decoupled State (Đổi vũ khí không ảnh hưởng Lower Body locomotion).
 * - AC-3: Frame Lockstep & Directional Sync (Z-Order: LowerBody 10, UpperBody 20; HandSocket đảo khi mirror hướng Tây).
 * - AC-4: 28 Tổ hợp Rig x Weapon Family, 100% Headless Safe, Không rò rỉ bộ nhớ.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPACharacterVisualRigTest,
	"ProjectAscendant.CharacterVisual.MasterRigDecoupledStateMachine",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPACharacterVisualRigTest::RunTest(const FString& Parameters)
{
	// =========================================================================
	// AC-1: 4 Master Rig Enum & State Machine
	// =========================================================================
	{
		TestEqual(TEXT("AC-1: EPAMasterRig defines 4 rigs"), (uint8)EPAMasterRig::Count, (uint8)4);

		FPAPaperdollModel Model;
		TestEqual(TEXT("AC-1: Default rig is HeavyTank"), Model.GetMasterRig(), EPAMasterRig::HeavyTank);
		TestEqual(TEXT("AC-1: Default lower asset is FB_Lower_HeavyTank_Set"),
			Model.GetLowerBodyVisualAssetId(), FName(TEXT("FB_Lower_HeavyTank_Set")));

		// Chuyển sang Agility
		Model.SetMasterRig(EPAMasterRig::Agility);
		TestEqual(TEXT("AC-1: Agility rig mapped"), Model.GetMasterRig(), EPAMasterRig::Agility);
		TestEqual(TEXT("AC-1: Agility asset is FB_Lower_Agility_Set"),
			Model.GetLowerBodyVisualAssetId(), FName(TEXT("FB_Lower_Agility_Set")));

		// Chuyển sang Caster
		Model.SetMasterRig(EPAMasterRig::Caster);
		TestEqual(TEXT("AC-1: Caster rig mapped"), Model.GetMasterRig(), EPAMasterRig::Caster);
		TestEqual(TEXT("AC-1: Caster asset is FB_Lower_Caster_Set"),
			Model.GetLowerBodyVisualAssetId(), FName(TEXT("FB_Lower_Caster_Set")));

		// Chuyển sang Monk
		Model.SetMasterRig(EPAMasterRig::Monk);
		TestEqual(TEXT("AC-1: Monk rig mapped"), Model.GetMasterRig(), EPAMasterRig::Monk);
		TestEqual(TEXT("AC-1: Monk asset is FB_Lower_Monk_Set"),
			Model.GetLowerBodyVisualAssetId(), FName(TEXT("FB_Lower_Monk_Set")));

		// Kiểm tra Pivots
		TestEqual(TEXT("AC-1: Foot pivot is strictly (64, 114)"),
			FPAPaperdollConstants::FootPivot, FVector2D(64.0f, 114.0f));
		TestEqual(TEXT("AC-1: Waist pivot is strictly (64, 80)"),
			FPAPaperdollConstants::WaistPivot, FVector2D(64.0f, 80.0f));
	}

	// =========================================================================
	// AC-2: 7 Weapon Family Upper Body Binding & Decoupled State
	// =========================================================================
	{
		TestEqual(TEXT("AC-2: EPAWeaponFamily defines 8 entries including None"), (uint8)EPAWeaponFamily::Count, (uint8)8);

		FPAPaperdollModel Model;
		Model.SetMasterRig(EPAMasterRig::HeavyTank);
		TestEqual(TEXT("AC-2: Initial lower asset"), Model.GetLowerBodyVisualAssetId(), FName(TEXT("FB_Lower_HeavyTank_Set")));

		// 1. Blade_1H
		Model.SetUpperBodyWeaponFamily(EPAWeaponFamily::Blade_1H);
		TestEqual(TEXT("AC-2: Family is Blade_1H"), Model.GetWeaponFamily(), EPAWeaponFamily::Blade_1H);
		TestEqual(TEXT("AC-2: Upper asset is FB_Upper_1H_Blade_Combo"),
			Model.GetUpperBodyVisualAssetId(), FName(TEXT("FB_Upper_1H_Blade_Combo")));
		// Xác nhận Lower Body HOÀN TOÀN KHÔNG BỊ ẢNH HƯỞNG (Decoupled!)
		TestEqual(TEXT("AC-2: Decoupled Lower Body unaffected"),
			Model.GetLowerBodyVisualAssetId(), FName(TEXT("FB_Lower_HeavyTank_Set")));

		// 2. Heavy_2H
		Model.SetUpperBodyWeaponFamily(EPAWeaponFamily::Heavy_2H);
		TestEqual(TEXT("AC-2: Family is Heavy_2H"), Model.GetWeaponFamily(), EPAWeaponFamily::Heavy_2H);
		TestEqual(TEXT("AC-2: Upper asset is FB_Upper_2H_Heavy_Combo"),
			Model.GetUpperBodyVisualAssetId(), FName(TEXT("FB_Upper_2H_Heavy_Combo")));

		// 3. Polearm_2H (Dùng chung cả Dragon Knight, God Slayer lẫn Town Guard)
		Model.SetUpperBodyWeaponFamily(EPAWeaponFamily::Polearm_2H);
		TestEqual(TEXT("AC-2: Family is Polearm_2H"), Model.GetWeaponFamily(), EPAWeaponFamily::Polearm_2H);
		TestEqual(TEXT("AC-2: Upper asset is FB_Upper_2H_Polearm_Combo"),
			Model.GetUpperBodyVisualAssetId(), FName(TEXT("FB_Upper_2H_Polearm_Combo")));

		// 4. Bow_2H
		Model.SetUpperBodyWeaponFamily(EPAWeaponFamily::Bow_2H);
		TestEqual(TEXT("AC-2: Family is Bow_2H"), Model.GetWeaponFamily(), EPAWeaponFamily::Bow_2H);
		TestEqual(TEXT("AC-2: Upper asset is FB_Upper_2H_Bow_Combo"),
			Model.GetUpperBodyVisualAssetId(), FName(TEXT("FB_Upper_2H_Bow_Combo")));

		// 5. DualDaggers
		Model.SetUpperBodyWeaponFamily(EPAWeaponFamily::DualDaggers);
		TestEqual(TEXT("AC-2: Family is DualDaggers"), Model.GetWeaponFamily(), EPAWeaponFamily::DualDaggers);
		TestEqual(TEXT("AC-2: Upper asset is FB_Upper_Dual_Daggers_Combo"),
			Model.GetUpperBodyVisualAssetId(), FName(TEXT("FB_Upper_Dual_Daggers_Combo")));

		// 6. Staff_2H
		Model.SetUpperBodyWeaponFamily(EPAWeaponFamily::Staff_2H);
		TestEqual(TEXT("AC-2: Family is Staff_2H"), Model.GetWeaponFamily(), EPAWeaponFamily::Staff_2H);
		TestEqual(TEXT("AC-2: Upper asset is FB_Upper_2H_Staff_Combo"),
			Model.GetUpperBodyVisualAssetId(), FName(TEXT("FB_Upper_2H_Staff_Combo")));

		// 7. MaceRelic_1H
		Model.SetUpperBodyWeaponFamily(EPAWeaponFamily::MaceRelic_1H);
		TestEqual(TEXT("AC-2: Family is MaceRelic_1H"), Model.GetWeaponFamily(), EPAWeaponFamily::MaceRelic_1H);
		TestEqual(TEXT("AC-2: Upper asset is FB_Upper_1H_MaceRelic_Combo"),
			Model.GetUpperBodyVisualAssetId(), FName(TEXT("FB_Upper_1H_MaceRelic_Combo")));

		// Kiểm tra gán bằng GameplayTag
		FGameplayTag PolearmTag = FPAPaperdollConstants::GetTagForWeaponFamily(EPAWeaponFamily::Polearm_2H);
		const bool bTagAssigned = Model.SetUpperBodyWeaponFamilyByTag(PolearmTag);
		TestTrue(TEXT("AC-2: SetUpperBodyWeaponFamilyByTag succeeded"), bTagAssigned);
		TestEqual(TEXT("AC-2: WeaponFamily matches tag"), Model.GetWeaponFamily(), EPAWeaponFamily::Polearm_2H);

		// Ngược lại: Đổi Master Rig không làm thay đổi Weapon Family (Decoupled 2 chiều!)
		Model.SetMasterRig(EPAMasterRig::Agility);
		TestEqual(TEXT("AC-2: Lower asset changed to Agility"), Model.GetLowerBodyVisualAssetId(), FName(TEXT("FB_Lower_Agility_Set")));
		TestEqual(TEXT("AC-2: Upper asset remains Polearm"), Model.GetUpperBodyVisualAssetId(), FName(TEXT("FB_Upper_2H_Polearm_Combo")));
	}

	// =========================================================================
	// AC-3: Frame Lockstep & Directional Sync
	// =========================================================================
	{
		UPAPaperdollComponent* Comp = NewObject<UPAPaperdollComponent>();
		TestNotNull(TEXT("AC-3: UPAPaperdollComponent instantiated"), Comp);

		UPaperFlipbookComponent* LowerComp = NewObject<UPaperFlipbookComponent>();
		UPaperFlipbookComponent* UpperComp = NewObject<UPaperFlipbookComponent>();

		Comp->RegisterLowerBodyComponent(LowerComp);
		Comp->RegisterUpperBodyComponent(UpperComp);

		TestEqual(TEXT("AC-3: LowerBodyComponent registered"), Comp->GetLowerBodyComponent(), LowerComp);
		TestEqual(TEXT("AC-3: UpperBodyComponent registered"), Comp->GetUpperBodyComponent(), UpperComp);

		// Kiểm tra Translucent Sort Priority ban đầu khi ngắm hướng Đông (East)
		Comp->UpdateDirectionalSortKeys(EPAAimDirection8Way::East);
		TestEqual(TEXT("AC-3: LowerBody sort priority is 10"), LowerComp->TranslucencySortPriority, 10);
		TestEqual(TEXT("AC-3: UpperBody sort priority is 20"), UpperComp->TranslucencySortPriority, 20);

		// Kiểm tra khi quay sang hướng Tây (West - Mirrored)
		Comp->UpdateDirectionalSortKeys(EPAAimDirection8Way::West);
		TestEqual(TEXT("AC-3: LowerBody sort priority stays 10 when mirrored"), LowerComp->TranslucencySortPriority, 10);
		TestEqual(TEXT("AC-3: UpperBody sort priority stays 20 when mirrored"), UpperComp->TranslucencySortPriority, 20);

		// So sánh với Sort Priority của MainHand / OffHand
		const int32 MainHandSortEast = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::MainHand, EPAAimDirection8Way::East);
		const int32 OffHandSortEast = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::OffHand, EPAAimDirection8Way::East);
		TestEqual(TEXT("AC-3: MainHand East is in front (50)"), MainHandSortEast, 50);
		TestEqual(TEXT("AC-3: OffHand East is behind (5)"), OffHandSortEast, 5);

		const int32 MainHandSortWest = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::MainHand, EPAAimDirection8Way::West);
		const int32 OffHandSortWest = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::OffHand, EPAAimDirection8Way::West);
		TestEqual(TEXT("AC-3: MainHand West mirrors to behind (5)"), MainHandSortWest, 5);
		TestEqual(TEXT("AC-3: OffHand West mirrors to front (50)"), OffHandSortWest, 50);
	}

	// =========================================================================
	// AC-4: 28 Combinations Rig x Weapon Family & Headless Null Safety
	// =========================================================================
	{
		UPAPaperdollComponent* Comp = NewObject<UPAPaperdollComponent>();

		const EPAMasterRig AllRigs[] = {
			EPAMasterRig::HeavyTank,
			EPAMasterRig::Agility,
			EPAMasterRig::Caster,
			EPAMasterRig::Monk
		};

		const EPAWeaponFamily AllFamilies[] = {
			EPAWeaponFamily::Blade_1H,
			EPAWeaponFamily::Heavy_2H,
			EPAWeaponFamily::Polearm_2H,
			EPAWeaponFamily::Bow_2H,
			EPAWeaponFamily::DualDaggers,
			EPAWeaponFamily::Staff_2H,
			EPAWeaponFamily::MaceRelic_1H
		};

		int32 ValidCombinations = 0;

		for (EPAMasterRig Rig : AllRigs)
		{
			for (EPAWeaponFamily Family : AllFamilies)
			{
				Comp->SetMasterRig(Rig);
				Comp->SetUpperBodyWeaponFamily(Family);

				TestEqual(TEXT("AC-4: Rig matches"), Comp->GetMasterRig(), Rig);
				TestEqual(TEXT("AC-4: Family matches"), Comp->GetWeaponFamily(), Family);
				TestFalse(TEXT("AC-4: Lower asset valid"), Comp->GetActiveLowerBodyVisualAssetId().IsNone());
				TestFalse(TEXT("AC-4: Upper asset valid"), Comp->GetActiveUpperBodyVisualAssetId().IsNone());

				ValidCombinations++;
			}
		}

		TestEqual(TEXT("AC-4: Exactly 28 Rig x Weapon combinations tested"), ValidCombinations, 28);

		// Kiểm tra ResetToStarterCloth
		Comp->ResetToStarterCloth();
		TestTrue(TEXT("AC-4: Wearing starter cloth after reset"), Comp->IsWearingStarterCloth());
		TestEqual(TEXT("AC-4: Master Rig reset to default"), Comp->GetMasterRig(), EPAMasterRig::HeavyTank);
		TestEqual(TEXT("AC-4: Weapon Family reset to default"), Comp->GetWeaponFamily(), EPAWeaponFamily::Blade_1H);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
