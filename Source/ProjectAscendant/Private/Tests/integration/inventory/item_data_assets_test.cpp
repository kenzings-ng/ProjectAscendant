// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "Inventory/GE_Item_IronSword.h"
#include "Inventory/GE_Item_IronPlate.h"
#include "Inventory/PAInventoryTypes.h"
#include "Inventory/PAEquipmentComponent.h"
#include "Combat/AscendantAttributeSet.h"
#include "Combat/PAGameplayAbility_Dash.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "UObject/UObjectGlobals.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPACoreItemDataAssetsIntegrationTest
 *
 * Automated integration test for Story item-001 (Core Items & Item DataAssets Setup):
 *  - AC-1 (Equipment DataAssets):
 *    * DA_Weapon_IronSword: Slot MainHand, Physical Damage +15, Rarity Uncommon, MaxStack 1, UGE_Item_IronSword.
 *    * DA_Armor_IronPlate: Slot Chest (BodyArmor), Armor +40, Rarity Rare, MaxStack 1, UGE_Item_IronPlate.
 *  - AC-2 (Consumable Quickbar DataAssets):
 *    * DA_Potion_Health: Slot Quickbar_1, ConsumableDuration 0.8s, SpeedDebuff 30%, Restores 100 HP, MaxStack 20.
 *  - AC-3 (Skill Book DataAsset):
 *    * DA_SkillBook_Dash: ItemType SkillBook, RequiredClassTag = Class.Vanguard, GrantedAbilityClass = UPAGameplayAbility_Dash (UGA_Dash), MaxStack 1.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPACoreItemDataAssetsIntegrationTest,
	"ProjectAscendant.Core.Items.ItemDataAssetsIntegration",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPACoreItemDataAssetsIntegrationTest::RunTest(const FString& Parameters)
{
	// =========================================================================
	// Test 1: AC-1 (Equipment DataAssets: Iron Sword & Iron Plate)
	// =========================================================================
	{
		// 1.1: DA_Weapon_IronSword Configuration
		UItemStaticDataAsset* SwordData = NewObject<UItemStaticDataAsset>();
		TestNotNull(TEXT("AC-1: UItemStaticDataAsset for Sword must be created"), SwordData);
		if (SwordData)
		{
			SwordData->SetupWeaponIronSword();

			TestEqual(TEXT("AC-1: Iron Sword ItemId"), SwordData->ItemId, FName(TEXT("Item_Weapon_IronSword")));
			TestEqual(TEXT("AC-1: Iron Sword Category must be Equipment"), SwordData->Category, EPAItemCategory::Equipment);
			TestEqual(TEXT("AC-1: Iron Sword Slot must be Mainhand"), SwordData->AllowedEquipmentSlot, EPAEquipmentSlot::Mainhand);
			TestEqual(TEXT("AC-1: Iron Sword Rarity must be Uncommon"), SwordData->RarityTier, EPAItemRarity::Uncommon);
			TestEqual(TEXT("AC-1: Iron Sword MaxStack must be 1"), SwordData->MaxStackSize, 1);
			TestNearlyEqual(TEXT("AC-1: Iron Sword Physical Damage Bonus must be +15.0f"), SwordData->BasePhysicalDamageBonus, 15.0f, 0.001f);
			TestEqual(TEXT("AC-1: Iron Sword EquipGameplayEffect class"), SwordData->EquipGameplayEffect, TSubclassOf<UGameplayEffect>(UGE_Item_IronSword::StaticClass()));

			// Validate slot compatibility with UPAEquipmentComponent
			TestTrue(TEXT("AC-1: Iron Sword must fit Mainhand slot"), UPAEquipmentComponent::CanItemFitInSlot(SwordData, EPAEquipmentSlot::Mainhand));
			TestFalse(TEXT("AC-1: Iron Sword must NOT fit BodyArmor slot"), UPAEquipmentComponent::CanItemFitInSlot(SwordData, EPAEquipmentSlot::BodyArmor));
		}

		// 1.2: UGE_Item_IronSword Modifier Validation (+15 AttackPower)
		const UGE_Item_IronSword* SwordGE = GetDefault<UGE_Item_IronSword>();
		TestNotNull(TEXT("AC-1: UGE_Item_IronSword CDO must exist"), SwordGE);
		if (SwordGE)
		{
			TestEqual(TEXT("AC-1: Sword GE DurationPolicy must be Infinite"), SwordGE->DurationPolicy, EGameplayEffectDurationType::Infinite);
			TestTrue(TEXT("AC-1: Sword GE must contain modifiers"), SwordGE->Modifiers.Num() > 0);

			bool bFoundAttackPowerMod = false;
			for (const FGameplayModifierInfo& Mod : SwordGE->Modifiers)
			{
				if (Mod.Attribute == UAscendantAttributeSet::GetAttackPowerAttribute())
				{
					bFoundAttackPowerMod = true;
					TestEqual(TEXT("AC-1: Sword GE ModifierOp must be Additive"), Mod.ModifierOp, EGameplayModOp::Additive);

					float OutMag = 0.0f;
					Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(1.0f, OutMag);
					TestNearlyEqual(TEXT("AC-1: Sword GE AttackPower magnitude must be +15.0f"), OutMag, 15.0f, 0.001f);
				}
			}
			TestTrue(TEXT("AC-1: Sword GE must modify AttackPower attribute"), bFoundAttackPowerMod);
		}

		// 1.3: DA_Armor_IronPlate Configuration
		UItemStaticDataAsset* PlateData = NewObject<UItemStaticDataAsset>();
		TestNotNull(TEXT("AC-1: UItemStaticDataAsset for Plate must be created"), PlateData);
		if (PlateData)
		{
			PlateData->SetupArmorIronPlate();

			TestEqual(TEXT("AC-1: Iron Plate ItemId"), PlateData->ItemId, FName(TEXT("Item_Armor_IronPlate")));
			TestEqual(TEXT("AC-1: Iron Plate Category must be Equipment"), PlateData->Category, EPAItemCategory::Equipment);
			TestEqual(TEXT("AC-1: Iron Plate Slot must be BodyArmor (Chest)"), PlateData->AllowedEquipmentSlot, EPAEquipmentSlot::BodyArmor);
			TestEqual(TEXT("AC-1: Iron Plate Rarity must be Rare"), PlateData->RarityTier, EPAItemRarity::Rare);
			TestEqual(TEXT("AC-1: Iron Plate MaxStack must be 1"), PlateData->MaxStackSize, 1);
			TestNearlyEqual(TEXT("AC-1: Iron Plate Armor Bonus must be +40.0f"), PlateData->BaseArmorBonus, 40.0f, 0.001f);
			TestEqual(TEXT("AC-1: Iron Plate EquipGameplayEffect class"), PlateData->EquipGameplayEffect, TSubclassOf<UGameplayEffect>(UGE_Item_IronPlate::StaticClass()));

			// Validate slot compatibility with UPAEquipmentComponent
			TestTrue(TEXT("AC-1: Iron Plate must fit BodyArmor slot"), UPAEquipmentComponent::CanItemFitInSlot(PlateData, EPAEquipmentSlot::BodyArmor));
			TestFalse(TEXT("AC-1: Iron Plate must NOT fit Mainhand slot"), UPAEquipmentComponent::CanItemFitInSlot(PlateData, EPAEquipmentSlot::Mainhand));
		}

		// 1.4: UGE_Item_IronPlate Modifier Validation (+40 Armor)
		const UGE_Item_IronPlate* PlateGE = GetDefault<UGE_Item_IronPlate>();
		TestNotNull(TEXT("AC-1: UGE_Item_IronPlate CDO must exist"), PlateGE);
		if (PlateGE)
		{
			TestEqual(TEXT("AC-1: Plate GE DurationPolicy must be Infinite"), PlateGE->DurationPolicy, EGameplayEffectDurationType::Infinite);
			TestTrue(TEXT("AC-1: Plate GE must contain modifiers"), PlateGE->Modifiers.Num() > 0);

			bool bFoundArmorMod = false;
			for (const FGameplayModifierInfo& Mod : PlateGE->Modifiers)
			{
				if (Mod.Attribute == UAscendantAttributeSet::GetArmorAttribute())
				{
					bFoundArmorMod = true;
					TestEqual(TEXT("AC-1: Plate GE ModifierOp must be Additive"), Mod.ModifierOp, EGameplayModOp::Additive);

					float OutMag = 0.0f;
					Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(1.0f, OutMag);
					TestNearlyEqual(TEXT("AC-1: Plate GE Armor magnitude must be +40.0f"), OutMag, 40.0f, 0.001f);
				}
			}
			TestTrue(TEXT("AC-1: Plate GE must modify Armor attribute"), bFoundArmorMod);
		}
	}

	// =========================================================================
	// Test 2: AC-2 (Consumable Quickbar DataAssets: Health Potion)
	// =========================================================================
	{
		UItemStaticDataAsset* PotionData = NewObject<UItemStaticDataAsset>();
		TestNotNull(TEXT("AC-2: UItemStaticDataAsset for Health Potion must be created"), PotionData);
		if (PotionData)
		{
			PotionData->SetupPotionHealth();

			TestEqual(TEXT("AC-2: Health Potion ItemId"), PotionData->ItemId, FName(TEXT("Item_Potion_Health")));
			TestEqual(TEXT("AC-2: Health Potion Category must be Consumable"), PotionData->Category, EPAItemCategory::Consumable);
			TestEqual(TEXT("AC-2: Health Potion PreferredQuickbarSlot must be Quickbar_1"), PotionData->PreferredQuickbarSlot, EPAQuickbarSlot::Quickbar_1);
			TestNearlyEqual(TEXT("AC-2: Health Potion ConsumableDuration must be 0.8s"), PotionData->ConsumableDuration, 0.8f, 0.001f);
			TestNearlyEqual(TEXT("AC-2: Health Potion SpeedDebuff must be 30% (0.30f)"), PotionData->ConsumableSpeedDebuffRatio, 0.30f, 0.001f);
			TestNearlyEqual(TEXT("AC-2: Health Potion Restores 100 HP"), PotionData->ConsumableHealthRestore, 100.0f, 0.001f);
			TestEqual(TEXT("AC-2: Health Potion MaxStack must be 20"), PotionData->MaxStackSize, 20);
		}
	}

	// =========================================================================
	// Test 3: AC-3 (Skill Book DataAsset: Vanguard Dash)
	// =========================================================================
	{
		UItemStaticDataAsset* DashBookData = NewObject<UItemStaticDataAsset>();
		TestNotNull(TEXT("AC-3: UItemStaticDataAsset for Dash SkillBook must be created"), DashBookData);
		if (DashBookData)
		{
			DashBookData->SetupSkillBookDash();

			TestEqual(TEXT("AC-3: Dash SkillBook ItemId"), DashBookData->ItemId, FName(TEXT("Item_SkillBook_Dash")));
			TestEqual(TEXT("AC-3: Dash SkillBook Category must be SkillBook"), DashBookData->Category, EPAItemCategory::SkillBook);
			TestEqual(TEXT("AC-3: Dash SkillBook MaxStack must be 1"), DashBookData->MaxStackSize, 1);

			// RequiredClassTag: Class.Vanguard
			const FGameplayTag VanguardTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Class.Vanguard")), false);
			TestTrue(TEXT("AC-3: RequiredClassTag must match Class.Vanguard"), DashBookData->RequiredClassTag.MatchesTag(VanguardTag));

			// GrantedAbilityClass: UPAGameplayAbility_Dash / UGA_Dash
			TestEqual(TEXT("AC-3: GrantedAbilityClass must be UPAGameplayAbility_Dash (UGA_Dash)"), DashBookData->GrantedAbilityClass, TSubclassOf<UGameplayAbility>(UGA_Dash::StaticClass()));
		}
	}

	// =========================================================================
	// Test 4: UAsset Package Verification on Disk (/Game/Items/DataAssets/)
	// =========================================================================
	{
		const TCHAR* SwordAssetPath = TEXT("/Game/Items/DataAssets/DA_Weapon_IronSword.DA_Weapon_IronSword");
		UItemStaticDataAsset* LoadedSword = Cast<UItemStaticDataAsset>(StaticLoadObject(UItemStaticDataAsset::StaticClass(), nullptr, SwordAssetPath));
		TestNotNull(TEXT("AC-1: DA_Weapon_IronSword.uasset loaded from Content/Items/DataAssets"), LoadedSword);
		if (LoadedSword)
		{
			TestEqual(TEXT("AC-1 (.uasset): Iron Sword slot is Mainhand"), LoadedSword->AllowedEquipmentSlot, EPAEquipmentSlot::Mainhand);
			TestNearlyEqual(TEXT("AC-1 (.uasset): Iron Sword damage bonus is +15.0f"), LoadedSword->BasePhysicalDamageBonus, 15.0f, 0.001f);
			TestEqual(TEXT("AC-1 (.uasset): Iron Sword rarity is Uncommon"), LoadedSword->RarityTier, EPAItemRarity::Uncommon);
			TestEqual(TEXT("AC-1 (.uasset): Iron Sword max stack is 1"), LoadedSword->MaxStackSize, 1);
		}

		const TCHAR* PlateAssetPath = TEXT("/Game/Items/DataAssets/DA_Armor_IronPlate.DA_Armor_IronPlate");
		UItemStaticDataAsset* LoadedPlate = Cast<UItemStaticDataAsset>(StaticLoadObject(UItemStaticDataAsset::StaticClass(), nullptr, PlateAssetPath));
		TestNotNull(TEXT("AC-1: DA_Armor_IronPlate.uasset loaded from Content/Items/DataAssets"), LoadedPlate);
		if (LoadedPlate)
		{
			TestEqual(TEXT("AC-1 (.uasset): Iron Plate slot is BodyArmor"), LoadedPlate->AllowedEquipmentSlot, EPAEquipmentSlot::BodyArmor);
			TestNearlyEqual(TEXT("AC-1 (.uasset): Iron Plate armor bonus is +40.0f"), LoadedPlate->BaseArmorBonus, 40.0f, 0.001f);
			TestEqual(TEXT("AC-1 (.uasset): Iron Plate rarity is Rare"), LoadedPlate->RarityTier, EPAItemRarity::Rare);
			TestEqual(TEXT("AC-1 (.uasset): Iron Plate max stack is 1"), LoadedPlate->MaxStackSize, 1);
		}

		const TCHAR* PotionAssetPath = TEXT("/Game/Items/DataAssets/DA_Potion_Health.DA_Potion_Health");
		UItemStaticDataAsset* LoadedPotion = Cast<UItemStaticDataAsset>(StaticLoadObject(UItemStaticDataAsset::StaticClass(), nullptr, PotionAssetPath));
		TestNotNull(TEXT("AC-2: DA_Potion_Health.uasset loaded from Content/Items/DataAssets"), LoadedPotion);
		if (LoadedPotion)
		{
			TestEqual(TEXT("AC-2 (.uasset): Potion slot is Quickbar_1"), LoadedPotion->PreferredQuickbarSlot, EPAQuickbarSlot::Quickbar_1);
			TestNearlyEqual(TEXT("AC-2 (.uasset): Potion duration is 0.8s"), LoadedPotion->ConsumableDuration, 0.8f, 0.001f);
			TestNearlyEqual(TEXT("AC-2 (.uasset): Potion speed debuff is 30%"), LoadedPotion->ConsumableSpeedDebuffRatio, 0.30f, 0.001f);
			TestNearlyEqual(TEXT("AC-2 (.uasset): Potion restore is 100 HP"), LoadedPotion->ConsumableHealthRestore, 100.0f, 0.001f);
			TestEqual(TEXT("AC-2 (.uasset): Potion max stack is 20"), LoadedPotion->MaxStackSize, 20);
		}

		const TCHAR* SkillBookAssetPath = TEXT("/Game/Items/DataAssets/DA_SkillBook_Dash.DA_SkillBook_Dash");
		UItemStaticDataAsset* LoadedSkillBook = Cast<UItemStaticDataAsset>(StaticLoadObject(UItemStaticDataAsset::StaticClass(), nullptr, SkillBookAssetPath));
		TestNotNull(TEXT("AC-3: DA_SkillBook_Dash.uasset loaded from Content/Items/DataAssets"), LoadedSkillBook);
		if (LoadedSkillBook)
		{
			TestEqual(TEXT("AC-3 (.uasset): SkillBook category is SkillBook"), LoadedSkillBook->Category, EPAItemCategory::SkillBook);
			const FGameplayTag VanguardTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Class.Vanguard")), false);
			TestTrue(TEXT("AC-3 (.uasset): SkillBook RequiredClassTag is Class.Vanguard"), LoadedSkillBook->RequiredClassTag.MatchesTag(VanguardTag));
			TestEqual(TEXT("AC-3 (.uasset): SkillBook GrantedAbilityClass is UPAGameplayAbility_Dash"), LoadedSkillBook->GrantedAbilityClass, TSubclassOf<UGameplayAbility>(UGA_Dash::StaticClass()));
			TestEqual(TEXT("AC-3 (.uasset): SkillBook max stack is 1"), LoadedSkillBook->MaxStackSize, 1);
		}
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
