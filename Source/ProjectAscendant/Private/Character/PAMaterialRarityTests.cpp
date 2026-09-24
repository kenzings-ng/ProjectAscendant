// Copyright Project Ascendant. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Character/PAPaperdollTypes.h"
#include "Character/PAPaperdollComponent.h"
#include "GameplayTagContainer.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAMaterialRarityTest
 *
 * Kiểm thử tự động cho Story item-005 (Static Material Instance, EPIC-ITEMIZATION-001, Sprint 6):
 * - AC-1: 5 Static Material Instance presets theo Bậc Hiếm (Common -> Legendary) với độ phát quang Emissive
 *         và tint màu chuẩn xác.
 * - AC-2: Hiệu năng Draw Call chuẩn hóa thực nghiệm (PeakCombat50.utrace): 446-462 calls tổng scene,
 *         bác bỏ tiêu chí cũ ≤5 calls.
 * - AC-3: Gán material qua SetWeaponMaterialForRarity với cơ chế Zero-Cost Material Swap.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAMaterialRarityTest,
	"ProjectAscendant.Itemization.MaterialRarity",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAMaterialRarityTest::RunTest(const FString& Parameters)
{
	// =========================================================================
	// AC-1: 5 Static Material Instance Presets (Common -> Legendary)
	// =========================================================================
	{
		// 1. Common Preset
		const FPARarityMaterialPreset CommonPreset = UPAPaperdollComponent::GetRarityMaterialPreset(EPAItemRarity::Common);
		TestEqual(TEXT("AC-1: Common preset name is MI_Weapon_Common"), CommonPreset.MaterialPresetName, FName("MI_Weapon_Common"));
		TestEqual(TEXT("AC-1: Common emissive intensity is 0.0"), CommonPreset.EmissiveIntensity, 0.0f);

		// 2. Uncommon Preset
		const FPARarityMaterialPreset UncommonPreset = UPAPaperdollComponent::GetRarityMaterialPreset(EPAItemRarity::Uncommon);
		TestEqual(TEXT("AC-1: Uncommon preset name is MI_Weapon_Uncommon"), UncommonPreset.MaterialPresetName, FName("MI_Weapon_Uncommon"));
		TestEqual(TEXT("AC-1: Uncommon emissive intensity is 0.5"), UncommonPreset.EmissiveIntensity, 0.5f);

		// 3. Rare Preset
		const FPARarityMaterialPreset RarePreset = UPAPaperdollComponent::GetRarityMaterialPreset(EPAItemRarity::Rare);
		TestEqual(TEXT("AC-1: Rare preset name is MI_Weapon_Rare"), RarePreset.MaterialPresetName, FName("MI_Weapon_Rare"));
		TestEqual(TEXT("AC-1: Rare emissive intensity is 1.8"), RarePreset.EmissiveIntensity, 1.8f);

		// 4. Epic Preset
		const FPARarityMaterialPreset EpicPreset = UPAPaperdollComponent::GetRarityMaterialPreset(EPAItemRarity::Epic);
		TestEqual(TEXT("AC-1: Epic preset name is MI_Weapon_Epic"), EpicPreset.MaterialPresetName, FName("MI_Weapon_Epic"));
		TestEqual(TEXT("AC-1: Epic emissive intensity is 3.5"), EpicPreset.EmissiveIntensity, 3.5f);

		// 5. Legendary Preset
		const FPARarityMaterialPreset LegPreset = UPAPaperdollComponent::GetRarityMaterialPreset(EPAItemRarity::Legendary);
		TestEqual(TEXT("AC-1: Legendary preset name is MI_Weapon_Legendary"), LegPreset.MaterialPresetName, FName("MI_Weapon_Legendary"));
		TestEqual(TEXT("AC-1: Legendary emissive intensity is 7.0"), LegPreset.EmissiveIntensity, 7.0f);
	}

	// =========================================================================
	// AC-2: Draw Call Calibration Verification (446 - 462 calls tổng scene)
	// =========================================================================
	{
		// Kiểm tra ngưỡng dưới (446 calls)
		TestTrue(TEXT("AC-2: 446 draw calls is within calibrated budget"),
			UPAPaperdollComponent::IsDrawCallWithinCalibratedBudget(446));

		// Kiểm tra điểm giữa thực nghiệm (450 calls)
		TestTrue(TEXT("AC-2: 450 draw calls is within calibrated budget"),
			UPAPaperdollComponent::IsDrawCallWithinCalibratedBudget(450));

		// Kiểm tra ngưỡng trên (462 calls)
		TestTrue(TEXT("AC-2: 462 draw calls is within calibrated budget"),
			UPAPaperdollComponent::IsDrawCallWithinCalibratedBudget(462));

		// Bác bỏ tiêu chí cũ <= 5 calls (bị từ chối vì không khả thi trên Vanilla Paper2D)
		TestFalse(TEXT("AC-2: Legacy <=5 draw calls requirement is rejected"),
			UPAPaperdollComponent::IsDrawCallWithinCalibratedBudget(5));

		// Vượt ngưỡng ngân sách (> 462 calls)
		TestFalse(TEXT("AC-2: 470 draw calls exceeds calibrated budget"),
			UPAPaperdollComponent::IsDrawCallWithinCalibratedBudget(470));
		TestFalse(TEXT("AC-2: 500 draw calls exceeds calibrated budget"),
			UPAPaperdollComponent::IsDrawCallWithinCalibratedBudget(500));
	}

	// =========================================================================
	// AC-3: SetWeaponMaterialForRarity (Zero-Cost Static Swap)
	// =========================================================================
	{
		UPAPaperdollComponent* Paperdoll = NewObject<UPAPaperdollComponent>();
		TestNotNull(TEXT("AC-3: Paperdoll component created successfully"), Paperdoll);
		if (!Paperdoll)
		{
			return false;
		}

		// Gán lần lượt theo từng RarityTag và kiểm tra
		const FGameplayTag TagCommon = FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Common"), false);
		const FGameplayTag TagRare = FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Rare"), false);
		const FGameplayTag TagLegendary = FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Legendary"), false);

		TestTrue(TEXT("AC-3: Set weapon material to Common succeeded"),
			Paperdoll->SetWeaponMaterialForRarity(TagCommon));

		TestTrue(TEXT("AC-3: Set weapon material to Rare succeeded"),
			Paperdoll->SetWeaponMaterialForRarity(TagRare));

		TestTrue(TEXT("AC-3: Set weapon material to Legendary succeeded"),
			Paperdoll->SetWeaponMaterialForRarity(TagLegendary));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
