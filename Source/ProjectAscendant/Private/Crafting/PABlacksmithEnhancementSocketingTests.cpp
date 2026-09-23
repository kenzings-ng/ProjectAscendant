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
 * FPABlacksmithEnhancementSocketingTests
 *
 * Automated unit tests for Story crft-002 (Enhancement Pipeline +4 to +10 & Socketing):
 *  - AC-1: Tier 2 Wilderness Forge enhancement (+4 to +6), failure preserves level.
 *  - AC-2: Tier 3 Ancient Sanctuary Forge enhancement (+7 to +10), failure drops 1 level unless protected by ward.
 *  - AC-3: Gem socketing (up to 2 sockets for Rare+ gear) and safe unsocketing with 100 Gold fee.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPABlacksmithEnhancementSocketingTests,
	"ProjectAscendant.Crafting.EnhancementSocketing",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPABlacksmithEnhancementSocketingTests::RunTest(const FString& Parameters)
{
	// =========================================================================
	// AC-1: Tier 2 Wilderness Forge Enhancement (+4 to +6) & Failure Floor
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
			// Configure Blacksmith as Tier 2 Wilderness Forge
			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier2_Wilderness);

			// Seed Wallet with 10,000 Gold
			EPACurrencyTransactionError CurrErr;
			Wallet->AddCurrency(EPACurrencyType::Gold, 10000, CurrErr);

			// Seed Inventory with 100 Iron Ore in Slot 1
			UItemStaticDataAsset* IronOreAsset = NewObject<UItemStaticDataAsset>();
			IronOreAsset->ItemId = FName("iron_ore");
			IronOreAsset->Category = EPAItemCategory::Material;
			IronOreAsset->MaxStackSize = 999;
			Inventory->AddItemToSlot(1, IronOreAsset, 100);

			// Place a +3 Rare Weapon in Slot 0
			UItemStaticDataAsset* WeaponAsset = NewObject<UItemStaticDataAsset>();
			WeaponAsset->ItemId = FName("item_rare_sword");
			WeaponAsset->Category = EPAItemCategory::Equipment;
			WeaponAsset->RarityTier = EPAItemRarity::Rare;

			FPAItemInstanceData WeaponData;
			WeaponData.EnhancementLevel = 3;
			Inventory->AddItemToSlot(0, WeaponAsset, 1, WeaponData);

			EPACraftingError CraftErr = EPACraftingError::None;

			// --- Test +3 -> +4 Success (70% rate, forced roll 0.50) ---
			Blacksmith->SetTestRollOverride(0.50f);
			bool bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, CraftErr);
			TestTrue(TEXT("AC-1: Enhance +3 -> +4 succeeds with roll 0.50 < 0.70"), bEnhanced);
			TestEqual(TEXT("AC-1: Weapon is now +4"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 4);

			// --- Test +4 -> +5 Failure preserves level (60% rate, forced roll 0.80) ---
			Blacksmith->SetTestRollOverride(0.80f);
			const int64 GoldBeforeFail = Wallet->GetGold();
			const int32 OreBeforeFail = Inventory->GetItemCount(FName("iron_ore"));

			bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, CraftErr);
			TestFalse(TEXT("AC-1: Enhance +4 -> +5 fails with roll 0.80 >= 0.60"), bEnhanced);
			TestEqual(TEXT("AC-1: Failure keeps level at +4 (Safe Floor)"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 4);
			TestTrue(TEXT("AC-1: Gold was consumed on failure"), Wallet->GetGold() < GoldBeforeFail);
			TestTrue(TEXT("AC-1: Iron ore was consumed on failure"), Inventory->GetItemCount(FName("iron_ore")) < OreBeforeFail);

			// --- Test +4 -> +5 Success (forced roll 0.10) ---
			Blacksmith->SetTestRollOverride(0.10f);
			bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, CraftErr);
			TestTrue(TEXT("AC-1: Enhance +4 -> +5 succeeds"), bEnhanced);
			TestEqual(TEXT("AC-1: Weapon is now +5"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 5);

			// --- Test +5 -> +6 Success (forced roll 0.10) ---
			bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, CraftErr);
			TestTrue(TEXT("AC-1: Enhance +5 -> +6 succeeds"), bEnhanced);
			TestEqual(TEXT("AC-1: Weapon is now +6"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 6);

			// --- Test Tier 2 Forge Ceiling (+6 max) ---
			bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, CraftErr);
			TestFalse(TEXT("AC-1: Attempting +6 -> +7 at Tier 2 Forge fails"), bEnhanced);
			TestEqual(TEXT("AC-1: Error is MaxTierLevelReached"), CraftErr, EPACraftingError::MaxTierLevelReached);
			TestEqual(TEXT("AC-1: Weapon remains +6"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 6);
		}
	}

	// =========================================================================
	// AC-2: Tier 3 Ancient Sanctuary Forge Enhancement (+7 to +10) & Ward Stone
	// =========================================================================
	{
		UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>();
		UPAInventoryComponent* Inventory = NewObject<UPAInventoryComponent>();
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();

		if (Blacksmith && Inventory && Wallet)
		{
			// Configure Blacksmith as Tier 3 Ancient Sanctuary Forge
			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier3_Sanctuary);

			// Seed Wallet with 50,000 Gold
			EPACurrencyTransactionError CurrErr;
			Wallet->AddCurrency(EPACurrencyType::Gold, 50000, CurrErr);

			// Seed Inventory with 500 Iron Ore
			UItemStaticDataAsset* IronOreAsset = NewObject<UItemStaticDataAsset>();
			IronOreAsset->ItemId = FName("iron_ore");
			IronOreAsset->Category = EPAItemCategory::Material;
			IronOreAsset->MaxStackSize = 999;
			Inventory->AddItemToSlot(1, IronOreAsset, 500);

			// Place a +7 Legendary Weapon in Slot 0
			UItemStaticDataAsset* WeaponAsset = NewObject<UItemStaticDataAsset>();
			WeaponAsset->ItemId = FName("item_legendary_sword");
			WeaponAsset->Category = EPAItemCategory::Equipment;
			WeaponAsset->RarityTier = EPAItemRarity::Legendary;

			FPAItemInstanceData WeaponData;
			WeaponData.EnhancementLevel = 7;
			WeaponData.CurrentDurability = 100.0f;
			Inventory->AddItemToSlot(0, WeaponAsset, 1, WeaponData);

			EPACraftingError CraftErr = EPACraftingError::None;

			// --- Test +7 -> +8 Failure WITHOUT Ward (drops 1 level to +6) ---
			Blacksmith->SetTestRollOverride(0.50f); // 30% success rate, 0.50 fails
			bool bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, false, CraftErr);
			TestFalse(TEXT("AC-2: Enhance +7 -> +8 fails without ward"), bEnhanced);
			TestEqual(TEXT("AC-2: Weapon drops from +7 to +6 on failure"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 6);
			TestEqual(TEXT("AC-2: Durability is preserved (never broken)"), Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 100.0f);

			// Re-enhance to +7
			Blacksmith->SetTestRollOverride(0.10f);
			bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, false, CraftErr);
			TestTrue(TEXT("AC-2: Re-enhanced to +7"), bEnhanced);
			TestEqual(TEXT("AC-2: Weapon back at +7"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 7);

			// --- Test requesting ward without having ward item ---
			bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, true, CraftErr);
			TestFalse(TEXT("AC-2: Enhance with ward fails when player has no ward stone"), bEnhanced);
			TestEqual(TEXT("AC-2: Error is NoWardItem"), CraftErr, EPACraftingError::NoWardItem);
			TestEqual(TEXT("AC-2: Weapon remains +7"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 7);

			// --- Add 1 Blacksmith Ward Stone in Slot 2 ---
			UItemStaticDataAsset* WardAsset = NewObject<UItemStaticDataAsset>();
			WardAsset->ItemId = FName("item_blacksmith_ward");
			WardAsset->Category = EPAItemCategory::Material;
			WardAsset->MaxStackSize = 99;
			Inventory->AddItemToSlot(2, WardAsset, 1);

			// --- Test +7 -> +8 Failure WITH Ward (level PRESERVED, ward consumed) ---
			Blacksmith->SetTestRollOverride(0.50f); // Forced fail
			bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, true, CraftErr);
			TestFalse(TEXT("AC-2: Enhance fails with roll 0.50"), bEnhanced);
			TestEqual(TEXT("AC-2: Ward stone protects weapon: level preserved at +7"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 7);
			TestEqual(TEXT("AC-2: Ward stone was consumed from inventory (now 0)"), Inventory->GetItemCount(FName("item_blacksmith_ward")), 0);

			// --- Reach +10 (Mastery Capstone) ---
			Blacksmith->SetTestRollOverride(0.05f); // Forced success for all steps
			Blacksmith->EnhanceItem(Inventory, Wallet, 0, false, CraftErr); // +7 -> +8
			Blacksmith->EnhanceItem(Inventory, Wallet, 0, false, CraftErr); // +8 -> +9
			Blacksmith->EnhanceItem(Inventory, Wallet, 0, false, CraftErr); // +9 -> +10
			TestEqual(TEXT("AC-2: Weapon reached +10"), Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel, 10);

			// Stat scaling check at +10
			const float ScaledAtk = FPABlacksmithFormulas::CalculateWeaponAttack(50.0f, 10);
			TestEqual(TEXT("AC-2: Attack scaling at +10 yields 80 (+60%) for base 50"), ScaledAtk, 80.0f);

			// Attempting +10 -> +11 fails (max level is 10)
			bEnhanced = Blacksmith->EnhanceItem(Inventory, Wallet, 0, false, CraftErr);
			TestFalse(TEXT("AC-2: Cannot enhance beyond +10"), bEnhanced);
			TestEqual(TEXT("AC-2: Error is MaxTierLevelReached"), CraftErr, EPACraftingError::MaxTierLevelReached);
		}
	}

	// =========================================================================
	// AC-3: Gem Socketing & Safe Unsocketing
	// =========================================================================
	{
		UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>();
		UPAInventoryComponent* Inventory = NewObject<UPAInventoryComponent>();
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();

		if (Blacksmith && Inventory && Wallet)
		{
			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier2_Wilderness);

			// Seed Wallet with 1,000 Gold
			EPACurrencyTransactionError CurrErr;
			Wallet->AddCurrency(EPACurrencyType::Gold, 1000, CurrErr);

			// Equipment 1: Common Armor (cannot be socketed)
			UItemStaticDataAsset* CommonArmorAsset = NewObject<UItemStaticDataAsset>();
			CommonArmorAsset->ItemId = FName("item_common_armor");
			CommonArmorAsset->Category = EPAItemCategory::Equipment;
			CommonArmorAsset->RarityTier = EPAItemRarity::Common;
			Inventory->AddItemToSlot(0, CommonArmorAsset, 1);

			EPACraftingError CraftErr = EPACraftingError::None;
			bool bUnlocked = Blacksmith->UnlockSocket(Inventory, Wallet, 0, CraftErr);
			TestFalse(TEXT("AC-3: Common armor cannot be socketed"), bUnlocked);
			TestEqual(TEXT("AC-3: Error is ItemCannotBeSocketed"), CraftErr, EPACraftingError::ItemCannotBeSocketed);

			// Equipment 2: Rare Weapon in Slot 1
			UItemStaticDataAsset* RareWeaponAsset = NewObject<UItemStaticDataAsset>();
			RareWeaponAsset->ItemId = FName("item_rare_sword");
			RareWeaponAsset->Category = EPAItemCategory::Equipment;
			RareWeaponAsset->RarityTier = EPAItemRarity::Rare;
			Inventory->AddItemToSlot(1, RareWeaponAsset, 1);

			// Tier 1 check: Tier 1 Forge cannot unlock sockets
			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier1_Outpost);
			bUnlocked = Blacksmith->UnlockSocket(Inventory, Wallet, 1, CraftErr);
			TestFalse(TEXT("AC-3: Tier 1 Forge cannot unlock sockets"), bUnlocked);
			TestEqual(TEXT("AC-3: Error is MaxTierLevelReached"), CraftErr, EPACraftingError::MaxTierLevelReached);

			// Switch back to Tier 2 Forge
			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier2_Wilderness);

			// Unlock Socket 1
			bUnlocked = Blacksmith->UnlockSocket(Inventory, Wallet, 1, CraftErr);
			TestTrue(TEXT("AC-3: Unlock 1st socket succeeds"), bUnlocked);
			TestEqual(TEXT("AC-3: Weapon has 1 socket"), Inventory->GetItemAtSlot(1)->DynamicData.SocketedGemIds.Num(), 1);
			TestEqual(TEXT("AC-3: Socket 0 is empty (NAME_None)"), Inventory->GetItemAtSlot(1)->DynamicData.SocketedGemIds[0], NAME_None);

			// Unlock Socket 2
			bUnlocked = Blacksmith->UnlockSocket(Inventory, Wallet, 1, CraftErr);
			TestTrue(TEXT("AC-3: Unlock 2nd socket succeeds"), bUnlocked);
			TestEqual(TEXT("AC-3: Weapon has 2 sockets"), Inventory->GetItemAtSlot(1)->DynamicData.SocketedGemIds.Num(), 2);

			// Attempt Unlock Socket 3 at Tier 2 Forge (cap is 2)
			bUnlocked = Blacksmith->UnlockSocket(Inventory, Wallet, 1, CraftErr);
			TestFalse(TEXT("AC-3: 3rd socket fails at Tier 2 Forge"), bUnlocked);
			TestEqual(TEXT("AC-3: Error is MaxSocketsReached"), CraftErr, EPACraftingError::MaxSocketsReached);

			// --- Socket Gem Tests ---
			// Add 1 Ruby in Slot 2
			UItemStaticDataAsset* RubyAsset = NewObject<UItemStaticDataAsset>();
			RubyAsset->ItemId = FName("ruby");
			RubyAsset->Category = EPAItemCategory::Material;
			RubyAsset->MaxStackSize = 99;
			Inventory->AddItemToSlot(2, RubyAsset, 1);

			// Attempt socket with invalid gem
			bool bSocketed = Blacksmith->SocketGem(Inventory, 1, 0, FName("invalid_stone"), CraftErr);
			TestFalse(TEXT("AC-3: Socketing invalid stone fails"), bSocketed);
			TestEqual(TEXT("AC-3: Error is InvalidGemItem"), CraftErr, EPACraftingError::InvalidGemItem);

			// Socket Ruby into Socket 0
			bSocketed = Blacksmith->SocketGem(Inventory, 1, 0, FName("ruby"), CraftErr);
			TestTrue(TEXT("AC-3: Socketing Ruby into Socket 0 succeeds"), bSocketed);
			TestEqual(TEXT("AC-3: Socket 0 now contains Ruby"), Inventory->GetItemAtSlot(1)->DynamicData.SocketedGemIds[0], FName("ruby"));
			TestEqual(TEXT("AC-3: Ruby consumed from inventory"), Inventory->GetItemCount(FName("ruby")), 0);

			// Attempt socket into already occupied socket 0
			// Add another Ruby
			Inventory->AddItemToSlot(2, RubyAsset, 1);
			bSocketed = Blacksmith->SocketGem(Inventory, 1, 0, FName("ruby"), CraftErr);
			TestFalse(TEXT("AC-3: Socketing into occupied socket fails"), bSocketed);
			TestEqual(TEXT("AC-3: Error is SocketNotEmpty"), CraftErr, EPACraftingError::SocketNotEmpty);

			// --- Unsocket Gem Tests ---
			// Attempt unsocket from empty Socket 1
			bool bUnsocketed = Blacksmith->UnsocketGem(Inventory, Wallet, 1, 1, CraftErr);
			TestFalse(TEXT("AC-3: Unsocketing empty socket fails"), bUnsocketed);
			TestEqual(TEXT("AC-3: Error is SocketEmpty"), CraftErr, EPACraftingError::SocketEmpty);

			// Unsocket Ruby from Socket 0 (costs 100 Gold)
			const int64 GoldBeforeUnsocket = Wallet->GetGold();
			bUnsocketed = Blacksmith->UnsocketGem(Inventory, Wallet, 1, 0, CraftErr);
			TestTrue(TEXT("AC-3: Unsocketing Ruby from Socket 0 succeeds"), bUnsocketed);
			TestEqual(TEXT("AC-3: Wallet deducted 100 Gold"), Wallet->GetGold(), GoldBeforeUnsocket - 100);
			TestEqual(TEXT("AC-3: Socket 0 is back to empty (NAME_None)"), Inventory->GetItemAtSlot(1)->DynamicData.SocketedGemIds[0], NAME_None);
			TestEqual(TEXT("AC-3: Ruby returned to inventory (now 2 including extra)"), Inventory->GetItemCount(FName("ruby")), 2);
		}
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
