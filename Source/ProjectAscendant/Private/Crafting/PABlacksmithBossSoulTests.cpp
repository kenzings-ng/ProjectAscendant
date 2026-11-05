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
 * FPABlacksmithBossSoulTests
 *
 * Automated unit tests for Story crft-003 (Boss Soul Forging & Legendary Equipment):
 *  - AC-1: Boss Soul Legendary Forging at Tier 3 Forge (1x Soul, 4x Parts, 5x Void Ore, 5,000 Gold).
 *          X11a: output is resolved from the forge's server-side recipe, never supplied by the caller.
 *  - AC-2: Prismatic Socket (3rd Gem Socket Expansion) and Prismatic Gem socketing at Tier 3 Forge.
 *  - AC-3: Backpack capacity expansion sequence (30 -> 40 -> 50 -> 60 max).
 *  - AC-4: Proximity and in-combat interaction guardrails.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPABlacksmithBossSoulTests,
	"ProjectAscendant.Crafting.BossSoulForging",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPABlacksmithBossSoulTests::RunTest(const FString& Parameters)
{
	// =========================================================================
	// AC-1: Boss Soul Legendary Forging (Tier 3 Forge Exclusive)
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
			// Boss-soul Legendary Weapon Static Data Asset
			UItemStaticDataAsset* BossSoulSword = NewObject<UItemStaticDataAsset>();
			BossSoulSword->ItemId = FName("item_boss_soul_dragon_slayer");
			BossSoulSword->Category = EPAItemCategory::Equipment;
			BossSoulSword->RarityTier = EPAItemRarity::Legendary;
			BossSoulSword->AllowedEquipmentSlot = EPAEquipmentSlot::Mainhand;
			BossSoulSword->BaseSellPrice = 5000;

			// Ingredients Assets
			UItemStaticDataAsset* SoulAsset = NewObject<UItemStaticDataAsset>();
			SoulAsset->ItemId = FName("item_boss_soul_dragon");
			SoulAsset->Category = EPAItemCategory::Material;
			SoulAsset->MaxStackSize = 10;

			UItemStaticDataAsset* PartAsset = NewObject<UItemStaticDataAsset>();
			PartAsset->ItemId = FName("item_boss_horn");
			PartAsset->Category = EPAItemCategory::Material;
			PartAsset->MaxStackSize = 50;

			UItemStaticDataAsset* VoidOreAsset = NewObject<UItemStaticDataAsset>();
			VoidOreAsset->ItemId = FName("void_ore");
			VoidOreAsset->Category = EPAItemCategory::Material;
			VoidOreAsset->MaxStackSize = 999;

			// X11a: server-side recipe on the forge maps the consumed boss soul to the Legendary output
			FPABossSoulRecipe Recipe;
			Recipe.BossSoulItemId = FName("item_boss_soul_dragon");
			Recipe.OutputItemData = BossSoulSword;
			Blacksmith->AddBossSoulRecipe(Recipe);

			// Seed Wallet with 10,000 Gold
			EPACurrencyTransactionError CurrErr;
			Wallet->AddCurrency(EPACurrencyType::Gold, 10000, CurrErr);

			// Seed Inventory with materials
			Inventory->AddItemToSlot(1, SoulAsset, 1);
			Inventory->AddItemToSlot(2, PartAsset, 4);
			Inventory->AddItemToSlot(3, VoidOreAsset, 5);

			EPACraftingError CraftErr = EPACraftingError::None;

			// --- Test 1: Tier Gating - Fails at Tier 1 and Tier 2 Forges ---
			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier1_Outpost);
			bool bForged = Blacksmith->ForgeBossSoulEquipment(
				Inventory, Wallet,
				FName("item_boss_soul_dragon"), FName("item_boss_horn"), FName("void_ore"),
				CraftErr);
			TestFalse(TEXT("AC-1: Boss soul forging rejected at Tier 1 Forge"), bForged);
			TestEqual(TEXT("AC-1: Error is MaxTierLevelReached at Tier 1"), CraftErr, EPACraftingError::MaxTierLevelReached);

			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier2_Wilderness);
			bForged = Blacksmith->ForgeBossSoulEquipment(
				Inventory, Wallet,
				FName("item_boss_soul_dragon"), FName("item_boss_horn"), FName("void_ore"),
				CraftErr);
			TestFalse(TEXT("AC-1: Boss soul forging rejected at Tier 2 Forge"), bForged);
			TestEqual(TEXT("AC-1: Error is MaxTierLevelReached at Tier 2"), CraftErr, EPACraftingError::MaxTierLevelReached);

			// --- Test 1b (X11a): boss soul without a server recipe is rejected, nothing consumed ---
			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier3_Sanctuary);
			UItemStaticDataAsset* UnknownSoulAsset = NewObject<UItemStaticDataAsset>();
			UnknownSoulAsset->ItemId = FName("item_boss_soul_unknown");
			UnknownSoulAsset->Category = EPAItemCategory::Material;
			UnknownSoulAsset->MaxStackSize = 10;
			Inventory->AddItemToSlot(4, UnknownSoulAsset, 1);

			bForged = Blacksmith->ForgeBossSoulEquipment(
				Inventory, Wallet,
				FName("item_boss_soul_unknown"), FName("item_boss_horn"), FName("void_ore"),
				CraftErr);
			TestFalse(TEXT("AC-1: Boss soul without server recipe rejected"), bForged);
			TestEqual(TEXT("AC-1: Error is InvalidItemType for unknown recipe"), CraftErr, EPACraftingError::InvalidItemType);
			TestEqual(TEXT("AC-1: Gold untouched after recipe rejection"), Wallet->GetGold(), 10000LL);
			TestEqual(TEXT("AC-1: Unknown soul not consumed"), Inventory->GetItemCount(FName("item_boss_soul_unknown")), 1);
			TestEqual(TEXT("AC-1: Boss parts not consumed"), Inventory->GetItemCount(FName("item_boss_horn")), 4);
			TestEqual(TEXT("AC-1: Void ore not consumed"), Inventory->GetItemCount(FName("void_ore")), 5);
			Inventory->RemoveItemFromSlot(4, 1);

			// --- Test 1c (X11a): ingredient ids must be boss part / void ore items ---
			bForged = Blacksmith->ForgeBossSoulEquipment(
				Inventory, Wallet,
				FName("item_boss_soul_dragon"), FName("iron_ore"), FName("void_ore"),
				CraftErr);
			TestFalse(TEXT("AC-1: Non boss-part ingredient rejected"), bForged);
			TestEqual(TEXT("AC-1: Error is InvalidItemType for wrong ingredient"), CraftErr, EPACraftingError::InvalidItemType);
			TestEqual(TEXT("AC-1: Gold untouched after ingredient rejection"), Wallet->GetGold(), 10000LL);

			// --- Test 2: Successful Forging at Tier 3 Forge ---
			bForged = Blacksmith->ForgeBossSoulEquipment(
				Inventory, Wallet,
				FName("item_boss_soul_dragon"), FName("item_boss_horn"), FName("void_ore"),
				CraftErr);
			TestTrue(TEXT("AC-1: Boss soul forging succeeds at Tier 3 Forge"), bForged);
			TestEqual(TEXT("AC-1: Error is None on success"), CraftErr, EPACraftingError::None);
			TestEqual(TEXT("AC-1: Wallet deducted 5,000 Gold -> 5,000 remaining"), Wallet->GetGold(), 5000LL);
			TestEqual(TEXT("AC-1: Boss Soul consumed (0 remaining)"), Inventory->GetItemCount(FName("item_boss_soul_dragon")), 0);
			TestEqual(TEXT("AC-1: Boss Horn consumed (0 remaining)"), Inventory->GetItemCount(FName("item_boss_horn")), 0);
			TestEqual(TEXT("AC-1: Void Ore consumed (0 remaining)"), Inventory->GetItemCount(FName("void_ore")), 0);

			// Verify boss-soul Legendary weapon added to inventory slot 0
			const FPAInventoryItemEntry* ResultItem = Inventory->GetItemAtSlot(0);
			TestNotNull(TEXT("AC-1: Boss-soul Legendary weapon present in inventory"), ResultItem);
			if (ResultItem)
			{
				TestEqual(TEXT("AC-1: Item ID matches boss-soul Dragon Slayer"), ResultItem->ItemDefId, FName("item_boss_soul_dragon_slayer"));
				TestEqual(TEXT("AC-1: Durability is 100%"), ResultItem->DynamicData.CurrentDurability, 100.0f);
			}

			// --- Test 3: Insufficient Materials Failure ---
			bForged = Blacksmith->ForgeBossSoulEquipment(
				Inventory, Wallet,
				FName("item_boss_soul_dragon"), FName("item_boss_horn"), FName("void_ore"),
				CraftErr);
			TestFalse(TEXT("AC-1: Forging fails without ingredients"), bForged);
			TestEqual(TEXT("AC-1: Error is InsufficientMaterials"), CraftErr, EPACraftingError::InsufficientMaterials);
		}
	}

	// =========================================================================
	// AC-2: Prismatic Socket (3rd Gem Socket Expansion) at Tier 3 Forge
	// =========================================================================
	{
		UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>();
		UPAInventoryComponent* Inventory = NewObject<UPAInventoryComponent>();
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();

		if (Blacksmith && Inventory && Wallet)
		{
			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier3_Sanctuary);

			// X11a: sockets are charged 1,000/3 + 3,000/8 + 15,000/20 (itemization.md 7.2) -> 19,000 Gold + 31 Shards
			EPACurrencyTransactionError CurrErr;
			Wallet->AddCurrency(EPACurrencyType::Gold, 20000, CurrErr);
			Wallet->AddCurrency(EPACurrencyType::SkillShards, 31, CurrErr);

			// Place a Rare Weapon in Slot 0
			UItemStaticDataAsset* RareWeapon = NewObject<UItemStaticDataAsset>();
			RareWeapon->ItemId = FName("item_sanctuary_blade");
			RareWeapon->Category = EPAItemCategory::Equipment;
			RareWeapon->RarityTier = EPAItemRarity::Rare;
			Inventory->AddItemToSlot(0, RareWeapon, 1);

			EPACraftingError CraftErr = EPACraftingError::None;

			// Unlock Socket 1
			Blacksmith->UnlockSocket(Inventory, Wallet, 0, CraftErr);
			// Unlock Socket 2
			Blacksmith->UnlockSocket(Inventory, Wallet, 0, CraftErr);
			TestEqual(TEXT("AC-2: Weapon has 2 sockets"), Inventory->GetItemAtSlot(0)->DynamicData.SocketedGemIds.Num(), 2);

			// Unlock Socket 3 (Prismatic Socket) at Tier 3 Forge
			bool bUnlocked3 = Blacksmith->UnlockSocket(Inventory, Wallet, 0, CraftErr);
			TestTrue(TEXT("AC-2: Unlocking 3rd socket succeeds at Tier 3 Forge"), bUnlocked3);
			TestEqual(TEXT("AC-2: Weapon now has 3 sockets (Prismatic unlocked)"), Inventory->GetItemAtSlot(0)->DynamicData.SocketedGemIds.Num(), 3);
			TestTrue(TEXT("AC-2: Socket index 2 is recognized as Prismatic Socket"), FPABlacksmithFormulas::IsPrismaticSocket(2));
			TestEqual(TEXT("AC-2: Three sockets charged 19,000 Gold -> 1,000"), Wallet->GetGold(), 1000LL);
			TestEqual(TEXT("AC-2: Three sockets charged 31 Shards -> 0"), Wallet->GetSkillShards(), 0LL);

			// Attempting to unlock 4th socket fails (cap is 3)
			bool bUnlocked4 = Blacksmith->UnlockSocket(Inventory, Wallet, 0, CraftErr);
			TestFalse(TEXT("AC-2: 4th socket expansion rejected"), bUnlocked4);
			TestEqual(TEXT("AC-2: Error is MaxSocketsReached"), CraftErr, EPACraftingError::MaxSocketsReached);

			// --- Socket Prismatic Gem into Socket 2 ---
			UItemStaticDataAsset* PrismGem = NewObject<UItemStaticDataAsset>();
			PrismGem->ItemId = FName("prismatic_gem");
			PrismGem->Category = EPAItemCategory::Material;
			PrismGem->MaxStackSize = 99;
			Inventory->AddItemToSlot(1, PrismGem, 1);

			TestTrue(TEXT("AC-2: prismatic_gem is recognized as valid gem"), FPABlacksmithFormulas::IsGemItem(FName("prismatic_gem")));
			TestTrue(TEXT("AC-2: prismatic_gem is recognized as Prismatic Gem"), FPABlacksmithFormulas::IsPrismaticGem(FName("prismatic_gem")));

			bool bSocketed = Blacksmith->SocketGem(Inventory, 0, 2, FName("prismatic_gem"), CraftErr);
			TestTrue(TEXT("AC-2: Socketing Prismatic Gem into 3rd socket succeeds"), bSocketed);
			TestEqual(TEXT("AC-2: Socket 2 now holds prismatic_gem"), Inventory->GetItemAtSlot(0)->DynamicData.SocketedGemIds[2], FName("prismatic_gem"));
			TestEqual(TEXT("AC-2: Prismatic gem consumed from inventory"), Inventory->GetItemCount(FName("prismatic_gem")), 0);
		}
	}

	// =========================================================================
	// AC-3: Backpack Capacity Expansion Sequence (30 -> 40 -> 50 -> 60)
	// =========================================================================
	{
		UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>();
		UPAInventoryComponent* Inventory = NewObject<UPAInventoryComponent>();
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();

		if (Blacksmith && Inventory && Wallet)
		{
			// Initial state: 30 slots
			TestEqual(TEXT("AC-3: Initial backpack capacity is 30 slots"), Inventory->GetCurrentMaxSlots(), 30);

			EPACraftingError CraftErr = EPACraftingError::None;
			EPACurrencyTransactionError CurrErr;

			// Seed Items for all 3 steps:
			// Step 1 (30 -> 40): 10 leather + 5 copper_ore + 500 Gold
			UItemStaticDataAsset* Leather = NewObject<UItemStaticDataAsset>();
			Leather->ItemId = FName("leather");
			Leather->Category = EPAItemCategory::Material;
			Leather->MaxStackSize = 999;
			Inventory->AddItemToSlot(1, Leather, 10);

			UItemStaticDataAsset* CopperOre = NewObject<UItemStaticDataAsset>();
			CopperOre->ItemId = FName("copper_ore");
			CopperOre->Category = EPAItemCategory::Material;
			CopperOre->MaxStackSize = 999;
			Inventory->AddItemToSlot(2, CopperOre, 5);

			// Seed Wallet with 20,000 Gold
			Wallet->AddCurrency(EPACurrencyType::Gold, 20000, CurrErr);

			// Step 1: Expand 30 -> 40 at Tier 1 Forge
			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier1_Outpost);
			bool bExpanded = Blacksmith->ExpandBackpackCapacity(Inventory, Wallet, CraftErr);
			TestTrue(TEXT("AC-3: Step 1 expansion (30 -> 40) succeeds at Tier 1 Forge"), bExpanded);
			TestEqual(TEXT("AC-3: Capacity is now 40 slots"), Inventory->GetCurrentMaxSlots(), 40);
			TestEqual(TEXT("AC-3: Wallet deducted 500 Gold -> 19,500 remaining"), Wallet->GetGold(), 19500LL);
			TestEqual(TEXT("AC-3: Leather consumed (0 remaining)"), Inventory->GetItemCount(FName("leather")), 0);
			TestEqual(TEXT("AC-3: Copper ore consumed (0 remaining)"), Inventory->GetItemCount(FName("copper_ore")), 0);

			// Attempt Step 2 (40 -> 50) at Tier 1 Forge -> Fails MaxTierLevelReached
			bExpanded = Blacksmith->ExpandBackpackCapacity(Inventory, Wallet, CraftErr);
			TestFalse(TEXT("AC-3: Step 2 expansion rejected at Tier 1 Forge"), bExpanded);
			TestEqual(TEXT("AC-3: Error is MaxTierLevelReached at Tier 1"), CraftErr, EPACraftingError::MaxTierLevelReached);

			// Step 2: Seed materials for 40 -> 50: 15 hardened_leather + 5 black_iron_ore
			UItemStaticDataAsset* HardLeather = NewObject<UItemStaticDataAsset>();
			HardLeather->ItemId = FName("hardened_leather");
			HardLeather->Category = EPAItemCategory::Material;
			HardLeather->MaxStackSize = 999;
			Inventory->AddItemToSlot(1, HardLeather, 15);

			UItemStaticDataAsset* BlackIron = NewObject<UItemStaticDataAsset>();
			BlackIron->ItemId = FName("black_iron_ore");
			BlackIron->Category = EPAItemCategory::Material;
			BlackIron->MaxStackSize = 999;
			Inventory->AddItemToSlot(2, BlackIron, 5);

			// Switch to Tier 2 Wilderness Forge
			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier2_Wilderness);
			bExpanded = Blacksmith->ExpandBackpackCapacity(Inventory, Wallet, CraftErr);
			TestTrue(TEXT("AC-3: Step 2 expansion (40 -> 50) succeeds at Tier 2 Forge"), bExpanded);
			TestEqual(TEXT("AC-3: Capacity is now 50 slots"), Inventory->GetCurrentMaxSlots(), 50);
			TestEqual(TEXT("AC-3: Wallet deducted 2,000 Gold -> 17,500 remaining"), Wallet->GetGold(), 17500LL);

			// Attempt Step 3 (50 -> 60) at Tier 2 Forge -> Fails MaxTierLevelReached
			bExpanded = Blacksmith->ExpandBackpackCapacity(Inventory, Wallet, CraftErr);
			TestFalse(TEXT("AC-3: Step 3 expansion rejected at Tier 2 Forge"), bExpanded);
			TestEqual(TEXT("AC-3: Error is MaxTierLevelReached at Tier 2"), CraftErr, EPACraftingError::MaxTierLevelReached);

			// Step 3: Seed materials for 50 -> 60: 5 boss_scale + 2 void_ore
			UItemStaticDataAsset* BossScale = NewObject<UItemStaticDataAsset>();
			BossScale->ItemId = FName("boss_scale");
			BossScale->Category = EPAItemCategory::Material;
			BossScale->MaxStackSize = 999;
			Inventory->AddItemToSlot(1, BossScale, 5);

			UItemStaticDataAsset* VoidOre = NewObject<UItemStaticDataAsset>();
			VoidOre->ItemId = FName("void_ore");
			VoidOre->Category = EPAItemCategory::Material;
			VoidOre->MaxStackSize = 999;
			Inventory->AddItemToSlot(2, VoidOre, 2);

			// Switch to Tier 3 Ancient Sanctuary Forge
			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier3_Sanctuary);
			bExpanded = Blacksmith->ExpandBackpackCapacity(Inventory, Wallet, CraftErr);
			TestTrue(TEXT("AC-3: Step 3 expansion (50 -> 60) succeeds at Tier 3 Forge"), bExpanded);
			TestEqual(TEXT("AC-3: Capacity reached maximum 60 slots"), Inventory->GetCurrentMaxSlots(), 60);
			TestEqual(TEXT("AC-3: Wallet deducted 8,000 Gold -> 9,500 remaining"), Wallet->GetGold(), 9500LL);

			// Attempt further expansion at 60 slots -> Fails MaxBackpackCapacity
			bExpanded = Blacksmith->ExpandBackpackCapacity(Inventory, Wallet, CraftErr);
			TestFalse(TEXT("AC-3: Expansion beyond 60 slots rejected"), bExpanded);
			TestEqual(TEXT("AC-3: Error is MaxBackpackCapacity"), CraftErr, EPACraftingError::MaxBackpackCapacity);
		}
	}

	// =========================================================================
	// AC-4: Interaction & Combat Guardrails
	// =========================================================================
	{
		UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>();
		if (Blacksmith)
		{
			EPACraftingError CraftErr = EPACraftingError::None;

			// In-combat check
			bool bValid = Blacksmith->ValidateInteraction(nullptr, true, CraftErr);
			TestFalse(TEXT("AC-4: Interaction rejected when player is in combat"), bValid);
			TestEqual(TEXT("AC-4: Error is InCombat"), CraftErr, EPACraftingError::InCombat);

			// Out-of-combat interaction
			bValid = Blacksmith->ValidateInteraction(nullptr, false, CraftErr);
			TestTrue(TEXT("AC-4: Interaction valid when player is out of combat"), bValid);
			TestEqual(TEXT("AC-4: Error is None"), CraftErr, EPACraftingError::None);
		}
	}

	// =========================================================================
	// AC-1 / DECISIONS §13: Boss Soul Forging with 4 Distinct Parts & Class Outputs
	// =========================================================================
	{
		UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>();
		UPAInventoryComponent* Inventory = NewObject<UPAInventoryComponent>();
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();

		TestNotNull(TEXT("DECISIONS §13: BlacksmithComponent created"), Blacksmith);
		TestNotNull(TEXT("DECISIONS §13: InventoryComponent created"), Inventory);
		TestNotNull(TEXT("DECISIONS §13: CurrencyComponent created"), Wallet);

		if (Blacksmith && Inventory && Wallet)
		{
			// Setup 3 class-specific Legendary equipment assets + 1 fallback
			UItemStaticDataAsset* FallbackSword = NewObject<UItemStaticDataAsset>();
			FallbackSword->ItemId = FName("item_boss_soul_dragon_slayer");
			FallbackSword->Category = EPAItemCategory::Equipment;
			FallbackSword->RarityTier = EPAItemRarity::Legendary;

			UItemStaticDataAsset* VanguardBlade = NewObject<UItemStaticDataAsset>();
			VanguardBlade->ItemId = FName("item_boss_soul_vanguard_blade");
			VanguardBlade->Category = EPAItemCategory::Equipment;
			VanguardBlade->RarityTier = EPAItemRarity::Legendary;

			UItemStaticDataAsset* RangerBow = NewObject<UItemStaticDataAsset>();
			RangerBow->ItemId = FName("item_boss_soul_ranger_bow");
			RangerBow->Category = EPAItemCategory::Equipment;
			RangerBow->RarityTier = EPAItemRarity::Legendary;

			UItemStaticDataAsset* ArcanistStaff = NewObject<UItemStaticDataAsset>();
			ArcanistStaff->ItemId = FName("item_boss_soul_arcanist_staff");
			ArcanistStaff->Category = EPAItemCategory::Equipment;
			ArcanistStaff->RarityTier = EPAItemRarity::Legendary;

			// Setup materials: 1 soul, 1 horn, 1 scale, 1 chest, 1 wing, 5 void ore
			UItemStaticDataAsset* SoulAsset = NewObject<UItemStaticDataAsset>();
			SoulAsset->ItemId = FName("item_boss_soul_dragon");
			SoulAsset->Category = EPAItemCategory::Material;

			UItemStaticDataAsset* HornAsset = NewObject<UItemStaticDataAsset>();
			HornAsset->ItemId = FName("item_boss_horn");
			HornAsset->Category = EPAItemCategory::Material;

			UItemStaticDataAsset* TailScaleAsset = NewObject<UItemStaticDataAsset>();
			TailScaleAsset->ItemId = FName("item_boss_scale");
			TailScaleAsset->Category = EPAItemCategory::Material;

			UItemStaticDataAsset* ChestArmorAsset = NewObject<UItemStaticDataAsset>();
			ChestArmorAsset->ItemId = FName("item_boss_chest");
			ChestArmorAsset->Category = EPAItemCategory::Material;

			UItemStaticDataAsset* WingAsset = NewObject<UItemStaticDataAsset>();
			WingAsset->ItemId = FName("item_boss_wing");
			WingAsset->Category = EPAItemCategory::Material;

			UItemStaticDataAsset* VoidOreAsset = NewObject<UItemStaticDataAsset>();
			VoidOreAsset->ItemId = FName("void_ore");
			VoidOreAsset->Category = EPAItemCategory::Material;

			FPABossSoulRecipe Recipe;
			Recipe.BossSoulItemId = FName("item_boss_soul_dragon");
			Recipe.OutputItemData = FallbackSword;
			Recipe.ClassOutputs.Add(EPACharacterClass::Vanguard, VanguardBlade);
			Recipe.ClassOutputs.Add(EPACharacterClass::Ranger, RangerBow);
			Recipe.ClassOutputs.Add(EPACharacterClass::Arcanist, ArcanistStaff);
			Blacksmith->AddBossSoulRecipe(Recipe);

			// Formulas unit test for distinct parts
			TestTrue(TEXT("DECISIONS §13: Formulas distinct parts validation succeeds with valid 4 parts"),
				FPABlacksmithFormulas::ValidateFourDistinctBossParts(
					FName("item_boss_horn"), FName("item_boss_scale"), FName("item_boss_chest"), FName("item_boss_wing")));
			TestFalse(TEXT("DECISIONS §13: Formulas distinct parts validation rejects duplicate parts (horn + horn)"),
				FPABlacksmithFormulas::ValidateFourDistinctBossParts(
					FName("item_boss_horn"), FName("item_boss_horn"), FName("item_boss_chest"), FName("item_boss_wing")));
			TestFalse(TEXT("DECISIONS §13: Formulas distinct parts validation rejects non-part item (iron_ore)"),
				FPABlacksmithFormulas::ValidateFourDistinctBossParts(
					FName("item_boss_horn"), FName("item_boss_scale"), FName("iron_ore"), FName("item_boss_wing")));

			// Seed inventory
			Inventory->AddItemToSlot(1, SoulAsset, 1);
			Inventory->AddItemToSlot(2, HornAsset, 1);
			Inventory->AddItemToSlot(3, TailScaleAsset, 1);
			Inventory->AddItemToSlot(4, ChestArmorAsset, 1);
			Inventory->AddItemToSlot(5, WingAsset, 1);
			Inventory->AddItemToSlot(6, VoidOreAsset, 5);

			EPACraftingError CraftErr = EPACraftingError::None;

			// Test Tier gating
			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier2_Wilderness);
			EPACurrencyTransactionError CurrErr;
			Wallet->AddCurrency(EPACurrencyType::Gold, 10000, CurrErr);

			bool bForged = Blacksmith->ForgeBossSoulEquipmentWithFourParts(
				Inventory, Wallet,
				FName("item_boss_soul_dragon"),
				FName("item_boss_horn"), FName("item_boss_scale"), FName("item_boss_chest"), FName("item_boss_wing"),
				FName("void_ore"), EPACharacterClass::Vanguard, CraftErr);
			TestFalse(TEXT("DECISIONS §13: Forging rejected at Tier 2 Forge"), bForged);
			TestEqual(TEXT("DECISIONS §13: Error is MaxTierLevelReached at Tier 2"), CraftErr, EPACraftingError::MaxTierLevelReached);

			// Set to Tier 3 Forge
			Blacksmith->SetForgeTier(EPABlacksmithTier::Tier3_Sanctuary);

			// Test duplicate part rejection at component level
			bForged = Blacksmith->ForgeBossSoulEquipmentWithFourParts(
				Inventory, Wallet,
				FName("item_boss_soul_dragon"),
				FName("item_boss_horn"), FName("item_boss_horn"), FName("item_boss_chest"), FName("item_boss_wing"),
				FName("void_ore"), EPACharacterClass::Vanguard, CraftErr);
			TestFalse(TEXT("DECISIONS §13: Forging rejected when duplicate parts provided"), bForged);
			TestEqual(TEXT("DECISIONS §13: Error is InvalidItemType for duplicates"), CraftErr, EPACraftingError::InvalidItemType);

			// Test non-part rejection
			bForged = Blacksmith->ForgeBossSoulEquipmentWithFourParts(
				Inventory, Wallet,
				FName("item_boss_soul_dragon"),
				FName("item_boss_horn"), FName("item_boss_scale"), FName("void_ore"), FName("item_boss_wing"),
				FName("void_ore"), EPACharacterClass::Vanguard, CraftErr);
			TestFalse(TEXT("DECISIONS §13: Forging rejected when non-part provided"), bForged);
			TestEqual(TEXT("DECISIONS §13: Error is InvalidItemType for non-part"), CraftErr, EPACraftingError::InvalidItemType);

			// Test insufficient gold (< 5,000)
			Wallet->DeductCurrency(EPACurrencyType::Gold, 6000, CurrErr); // Leaves 4,000 Gold
			bForged = Blacksmith->ForgeBossSoulEquipmentWithFourParts(
				Inventory, Wallet,
				FName("item_boss_soul_dragon"),
				FName("item_boss_horn"), FName("item_boss_scale"), FName("item_boss_chest"), FName("item_boss_wing"),
				FName("void_ore"), EPACharacterClass::Vanguard, CraftErr);
			TestFalse(TEXT("DECISIONS §13: Forging rejected with < 5,000 Gold"), bForged);
			TestEqual(TEXT("DECISIONS §13: Error is InsufficientGold"), CraftErr, EPACraftingError::InsufficientGold);

			// Replenish Gold to 10,000
			Wallet->AddCurrency(EPACurrencyType::Gold, 6000, CurrErr);

			// Successful Vanguard Forging: outputs VanguardBlade
			bForged = Blacksmith->ForgeBossSoulEquipmentWithFourParts(
				Inventory, Wallet,
				FName("item_boss_soul_dragon"),
				FName("item_boss_horn"), FName("item_boss_scale"), FName("item_boss_chest"), FName("item_boss_wing"),
				FName("void_ore"), EPACharacterClass::Vanguard, CraftErr);
			TestTrue(TEXT("DECISIONS §13: Vanguard Boss soul forging succeeds with 4 distinct parts"), bForged);
			TestEqual(TEXT("DECISIONS §13: Error is None on success"), CraftErr, EPACraftingError::None);
			TestEqual(TEXT("DECISIONS §13: Wallet deducted 5,000 Gold -> 5,000 remaining"), Wallet->GetGold(), 5000LL);
			TestEqual(TEXT("DECISIONS §13: Boss Soul consumed (0 remaining)"), Inventory->GetItemCount(FName("item_boss_soul_dragon")), 0);
			TestEqual(TEXT("DECISIONS §13: Horn consumed (0 remaining)"), Inventory->GetItemCount(FName("item_boss_horn")), 0);
			TestEqual(TEXT("DECISIONS §13: Tail Scale consumed (0 remaining)"), Inventory->GetItemCount(FName("item_boss_scale")), 0);
			TestEqual(TEXT("DECISIONS §13: Chest Armor consumed (0 remaining)"), Inventory->GetItemCount(FName("item_boss_chest")), 0);
			TestEqual(TEXT("DECISIONS §13: Wing consumed (0 remaining)"), Inventory->GetItemCount(FName("item_boss_wing")), 0);
			TestEqual(TEXT("DECISIONS §13: Void Ore consumed (0 remaining)"), Inventory->GetItemCount(FName("void_ore")), 0);

			const FPAInventoryItemEntry* ResultItem = Inventory->GetItemAtSlot(0);
			TestNotNull(TEXT("DECISIONS §13: Output item present at slot 0"), ResultItem);
			if (ResultItem)
			{
				TestEqual(TEXT("DECISIONS §13: Output item is Vanguard Blade"), ResultItem->ItemDefId, FName("item_boss_soul_vanguard_blade"));
				TestEqual(TEXT("DECISIONS §13: Durability is 100%"), ResultItem->DynamicData.CurrentDurability, 100.0f);
			}

			// Clear slot 0 and test Ranger class output
			Inventory->RemoveItemFromSlot(0, 1);
			Inventory->AddItemToSlot(1, SoulAsset, 1);
			Inventory->AddItemToSlot(2, HornAsset, 1);
			Inventory->AddItemToSlot(3, TailScaleAsset, 1);
			Inventory->AddItemToSlot(4, ChestArmorAsset, 1);
			Inventory->AddItemToSlot(5, WingAsset, 1);
			Inventory->AddItemToSlot(6, VoidOreAsset, 5);

			bForged = Blacksmith->ForgeBossSoulEquipmentWithFourParts(
				Inventory, Wallet,
				FName("item_boss_soul_dragon"),
				FName("item_boss_horn"), FName("item_boss_scale"), FName("item_boss_chest"), FName("item_boss_wing"),
				FName("void_ore"), EPACharacterClass::Ranger, CraftErr);
			TestTrue(TEXT("DECISIONS §13: Ranger Boss soul forging succeeds"), bForged);
			const FPAInventoryItemEntry* RangerResult = Inventory->GetItemAtSlot(0);
			TestNotNull(TEXT("DECISIONS §13: Ranger output item present"), RangerResult);
			if (RangerResult)
			{
				TestEqual(TEXT("DECISIONS §13: Output item is Ranger Bow"), RangerResult->ItemDefId, FName("item_boss_soul_ranger_bow"));
			}

			// Clear slot 0 and test Arcanist class output
			Inventory->RemoveItemFromSlot(0, 1);
			Wallet->AddCurrency(EPACurrencyType::Gold, 5000, CurrErr);
			Inventory->AddItemToSlot(1, SoulAsset, 1);
			Inventory->AddItemToSlot(2, HornAsset, 1);
			Inventory->AddItemToSlot(3, TailScaleAsset, 1);
			Inventory->AddItemToSlot(4, ChestArmorAsset, 1);
			Inventory->AddItemToSlot(5, WingAsset, 1);
			Inventory->AddItemToSlot(6, VoidOreAsset, 5);

			bForged = Blacksmith->ForgeBossSoulEquipmentWithFourParts(
				Inventory, Wallet,
				FName("item_boss_soul_dragon"),
				FName("item_boss_horn"), FName("item_boss_scale"), FName("item_boss_chest"), FName("item_boss_wing"),
				FName("void_ore"), EPACharacterClass::Arcanist, CraftErr);
			TestTrue(TEXT("DECISIONS §13: Arcanist Boss soul forging succeeds"), bForged);
			const FPAInventoryItemEntry* ArcanistResult = Inventory->GetItemAtSlot(0);
			TestNotNull(TEXT("DECISIONS §13: Arcanist output item present"), ArcanistResult);
			if (ArcanistResult)
			{
				TestEqual(TEXT("DECISIONS §13: Output item is Arcanist Staff"), ArcanistResult->ItemDefId, FName("item_boss_soul_arcanist_staff"));
			}
		}
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
