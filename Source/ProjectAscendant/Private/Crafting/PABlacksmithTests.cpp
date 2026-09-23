// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Crafting/PABlacksmithComponent.h"
#include "Crafting/PABlacksmithTypes.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAInventoryTypes.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "Economy/PACurrencyComponent.h"
#include "Economy/PACurrencyTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPABlacksmithTests
 *
 * Automated unit tests for Story crft-001 (Blacksmith Tier 1 Outpost Forge & Item Repair):
 *  - AC-1: Durability repair calculation and restoration to 100.0% with Gold deduction.
 *  - AC-2: Item & Skill Book salvaging into Ash Shards and locked-item protection.
 *  - AC-3: Safe 100% enhancement from +0 up to +3, respecting Tier 1 Outpost Forge cap.
 *  - AC-4: Interaction distance and validation guardrails.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPABlacksmithTests,
	"ProjectAscendant.Crafting.Blacksmith",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPABlacksmithTests::RunTest(const FString& Parameters)
{
	// =========================================================================
	// AC-1: Durability Repair Engine & Formula
	// =========================================================================
	{
		UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>();
		UPAInventoryComponent* Inventory = NewObject<UPAInventoryComponent>();
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();

		TestNotNull(TEXT("AC-1: BlacksmithComponent created"), Blacksmith);
		TestNotNull(TEXT("AC-1: InventoryComponent created"), Inventory);
		TestNotNull(TEXT("AC-1: CurrencyComponent created"), Wallet);

		if (Blacksmith && Inventory && Wallet)
		{
			// Formula validation: Item base price 200, durability 40% (40.0 / 100.0)
			// repair_cost = ceil(200 * 0.25 * (1.0 - 0.40)) = ceil(30.0) = 30 Gold
			const int32 CalculatedCost = FPABlacksmithFormulas::CalculateRepairCost(200, 40.0f, 100.0f);
			TestEqual(TEXT("AC-1: Formula calculates 30 Gold for 200 base price at 40% durability"), CalculatedCost, 30);

			// Seed Wallet with 100 Gold
			EPACurrencyTransactionError CurrErr;
			Wallet->AddCurrency(EPACurrencyType::Gold, 100, CurrErr);

			// Create a damaged equipment item in Slot 0
			UItemStaticDataAsset* SwordAsset = NewObject<UItemStaticDataAsset>();
			SwordAsset->ItemId = FName("item_test_sword");
			SwordAsset->Category = EPAItemCategory::Equipment;
			SwordAsset->RarityTier = EPAItemRarity::Common;
			SwordAsset->BaseSellPrice = 200;

			FPAItemInstanceData DamagedData;
			DamagedData.CurrentDurability = 40.0f;
			Inventory->AddItemToSlot(0, SwordAsset, 1, DamagedData);

			// Perform Repair
			EPACraftingError CraftErr = EPACraftingError::None;
			bool bRepaired = Blacksmith->RepairItem(Inventory, Wallet, 0, CraftErr);
			TestTrue(TEXT("AC-1: RepairItem succeeds"), bRepaired);
			TestEqual(TEXT("AC-1: Wallet deducted 30 Gold -> 70 remaining"), Wallet->GetGold(), 70LL);

			const FPAInventoryItemEntry* RepairedItem = Inventory->GetItemAtSlot(0);
			TestNotNull(TEXT("AC-1: Item still exists in slot 0"), RepairedItem);
			if (RepairedItem)
			{
				TestEqual(TEXT("AC-1: Item durability restored to 100%"), RepairedItem->DynamicData.CurrentDurability, 100.0f);
			}

			// Attempt repair on fully repaired item -> MaxDurabilityAlready error
			bRepaired = Blacksmith->RepairItem(Inventory, Wallet, 0, CraftErr);
			TestFalse(TEXT("AC-1: Repairing item at 100% durability fails"), bRepaired);
			TestEqual(TEXT("AC-1: Error is MaxDurabilityAlready"), CraftErr, EPACraftingError::MaxDurabilityAlready);
			TestEqual(TEXT("AC-1: Wallet unchanged at 70 Gold"), Wallet->GetGold(), 70LL);
		}
	}

	// =========================================================================
	// AC-2: Item & Skill Book Salvaging into Ash Shards
	// =========================================================================
	{
		UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>();
		UPAInventoryComponent* Inventory = NewObject<UPAInventoryComponent>();
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();

		if (Blacksmith && Inventory && Wallet)
		{
			// Rare Equipment asset (yields 10 Ash Shards)
			UItemStaticDataAsset* RareArmor = NewObject<UItemStaticDataAsset>();
			RareArmor->ItemId = FName("item_rare_plate");
			RareArmor->Category = EPAItemCategory::Equipment;
			RareArmor->RarityTier = EPAItemRarity::Rare;

			// Skill Book asset (yields 5 Ash Shards)
			UItemStaticDataAsset* SkillBook = NewObject<UItemStaticDataAsset>();
			SkillBook->ItemId = FName("item_skill_book");
			SkillBook->Category = EPAItemCategory::SkillBook;
			SkillBook->RarityTier = EPAItemRarity::Uncommon;

			Inventory->AddItemToSlot(0, RareArmor, 1);
			Inventory->AddItemToSlot(1, SkillBook, 1);

			// Salvage Rare Armor in Slot 0
			int32 ShardsGained = 0;
			EPACraftingError CraftErr = EPACraftingError::None;
			bool bSalvaged = Blacksmith->SalvageItem(Inventory, Wallet, 0, ShardsGained, CraftErr);
			TestTrue(TEXT("AC-2: Salvaging Rare armor succeeds"), bSalvaged);
			TestEqual(TEXT("AC-2: Rare armor yields 10 Ash Shards"), ShardsGained, 10);
			TestEqual(TEXT("AC-2: Wallet Ash Shards is 10"), Wallet->GetAshShards(), 10LL);
			TestNull(TEXT("AC-2: Rare armor consumed and slot 0 is now empty"), Inventory->GetItemAtSlot(0));

			// Salvage Skill Book in Slot 1
			bSalvaged = Blacksmith->SalvageItem(Inventory, Wallet, 1, ShardsGained, CraftErr);
			TestTrue(TEXT("AC-2: Salvaging Skill Book succeeds"), bSalvaged);
			TestEqual(TEXT("AC-2: Skill Book yields 5 Ash Shards"), ShardsGained, 5);
			TestEqual(TEXT("AC-2: Total Ash Shards is 15"), Wallet->GetAshShards(), 15LL);
			TestNull(TEXT("AC-2: Skill Book consumed and slot 1 is now empty"), Inventory->GetItemAtSlot(1));

			// Locked Item Protection: create locked item in Slot 2
			FPAItemInstanceData LockedData;
			LockedData.bIsLocked = true;
			Inventory->AddItemToSlot(2, RareArmor, 1, LockedData);

			bSalvaged = Blacksmith->SalvageItem(Inventory, Wallet, 2, ShardsGained, CraftErr);
			TestFalse(TEXT("AC-2: Salvaging locked item fails"), bSalvaged);
			TestEqual(TEXT("AC-2: Error is ItemLocked"), CraftErr, EPACraftingError::ItemLocked);
			TestNotNull(TEXT("AC-2: Locked item remains intact in slot 2"), Inventory->GetItemAtSlot(2));
		}
	}

	// =========================================================================
	// AC-3: Safe Enhancement +1 to +3 at Tier 1 Forge
	// =========================================================================
	{
		UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>();
		UPAInventoryComponent* Inventory = NewObject<UPAInventoryComponent>();
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();

		if (Blacksmith && Inventory && Wallet)
		{
			// Tier 1 Outpost Forge default
			TestEqual(TEXT("AC-3: Default Forge is Tier 1 Outpost Forge"), Blacksmith->GetForgeTier(), EPABlacksmithTier::Tier1_Outpost);

			// Seed Wallet with 1,000 Gold
			EPACurrencyTransactionError CurrErr;
			Wallet->AddCurrency(EPACurrencyType::Gold, 1000, CurrErr);

			// Seed Inventory with 20 Iron Ore in Slot 1
			UItemStaticDataAsset* IronOreAsset = NewObject<UItemStaticDataAsset>();
			IronOreAsset->ItemId = FName("iron_ore");
			IronOreAsset->Category = EPAItemCategory::Material;
			IronOreAsset->MaxStackSize = 999;
			Inventory->AddItemToSlot(1, IronOreAsset, 20);

			// Equipment in Slot 0 (Enhancement level +0)
			UItemStaticDataAsset* WeaponAsset = NewObject<UItemStaticDataAsset>();
			WeaponAsset->ItemId = FName("item_iron_blade");
			WeaponAsset->Category = EPAItemCategory::Equipment;
			WeaponAsset->RarityTier = EPAItemRarity::Common;

			FPAItemInstanceData WeaponData;
			WeaponData.EnhancementLevel = 0;
			Inventory->AddItemToSlot(0, WeaponAsset, 1, WeaponData);

			EPACraftingError CraftErr = EPACraftingError::None;

			// Enhance +0 -> +1 (Cost: 100 Gold + 2 Iron Ore)
			bool bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, CraftErr);
			TestTrue(TEXT("AC-3: Enhance +0 -> +1 succeeds"), bEnhanced);
			TestEqual(TEXT("AC-3: Wallet deducted 100 Gold -> 900 remaining"), Wallet->GetGold(), 900LL);
			TestEqual(TEXT("AC-3: Iron Ore consumed 2 -> 18 remaining"), Inventory->GetItemCount(FName("iron_ore")), 18);
			TestEqual(TEXT("AC-3: Weapon is now +1"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 1);

			// Enhance +1 -> +2 (Cost: 250 Gold + 4 Iron Ore)
			bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, CraftErr);
			TestTrue(TEXT("AC-3: Enhance +1 -> +2 succeeds"), bEnhanced);
			TestEqual(TEXT("AC-3: Wallet deducted 250 Gold -> 650 remaining"), Wallet->GetGold(), 650LL);
			TestEqual(TEXT("AC-3: Iron Ore consumed 4 -> 14 remaining"), Inventory->GetItemCount(FName("iron_ore")), 14);
			TestEqual(TEXT("AC-3: Weapon is now +2"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 2);

			// Enhance +2 -> +3 (Cost: 500 Gold + 8 Iron Ore)
			bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, CraftErr);
			TestTrue(TEXT("AC-3: Enhance +2 -> +3 succeeds"), bEnhanced);
			TestEqual(TEXT("AC-3: Wallet deducted 500 Gold -> 150 remaining"), Wallet->GetGold(), 150LL);
			TestEqual(TEXT("AC-3: Iron Ore consumed 8 -> 6 remaining"), Inventory->GetItemCount(FName("iron_ore")), 6);
			TestEqual(TEXT("AC-3: Weapon is now +3"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 3);

			// Attempt +3 -> +4 at Tier 1 Forge -> MaxTierLevelReached error
			bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, CraftErr);
			TestFalse(TEXT("AC-3: Enhancing +3 at Tier 1 Forge fails (Tier 1 cap is +3)"), bEnhanced);
			TestEqual(TEXT("AC-3: Error is MaxTierLevelReached"), CraftErr, EPACraftingError::MaxTierLevelReached);
			TestEqual(TEXT("AC-3: Weapon remains +3 safely"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 3);
		}
	}

	// =========================================================================
	// AC-4: Interaction Distance & In-Combat Guardrails
	// =========================================================================
	{
		UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>();
		if (Blacksmith)
		{
			EPACraftingError CraftErr = EPACraftingError::None;

			// In-combat rejection
			bool bValid = Blacksmith->ValidateInteraction(nullptr, true, CraftErr);
			TestFalse(TEXT("AC-4: Interaction rejected when player is in combat"), bValid);
			TestEqual(TEXT("AC-4: Error is InCombat"), CraftErr, EPACraftingError::InCombat);

			// Out of combat interaction
			bValid = Blacksmith->ValidateInteraction(nullptr, false, CraftErr);
			TestTrue(TEXT("AC-4: Interaction allowed when out of combat"), bValid);
			TestEqual(TEXT("AC-4: Error is None"), CraftErr, EPACraftingError::None);

			// Formula stat scaling verification
			const float BaseAtk = 50.0f;
			const float AtkAt3 = FPABlacksmithFormulas::CalculateWeaponAttack(BaseAtk, 3);
			TestEqual(TEXT("AC-4: Attack scaling formula at +3 yields 58 for base 50"), AtkAt3, 58.0f);
		}
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
