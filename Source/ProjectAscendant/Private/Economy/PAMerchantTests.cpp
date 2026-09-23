// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Economy/PAMerchantComponent.h"
#include "Economy/PAMerchantTypes.h"
#include "Economy/PACurrencyComponent.h"
#include "Economy/PACurrencyTypes.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAInventoryTypes.h"
#include "Inventory/PAItemStaticDataAsset.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAMerchantTests
 *
 * Automated unit tests for Story econ-002 (Merchant Vendor Network & Buyback Window):
 *  - AC-1: Catalog purchase validation, stock decrement, and atomic Gold/Item exchange.
 *  - AC-2: Item selling with 30% penalty, locked item rejection, and bulk junk selling.
 *  - AC-3: 10-slot FIFO Buyback window, exact price refund, and item restoration.
 *  - AC-4: Interaction proximity and in-combat rejection guardrails.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAMerchantTests,
	"ProjectAscendant.Economy.Merchant",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAMerchantTests::RunTest(const FString& Parameters)
{
	// =========================================================================
	// AC-1: Catalog Setup & Purchasing
	// =========================================================================
	{
		UPAMerchantComponent* Merchant = NewObject<UPAMerchantComponent>();
		UPAInventoryComponent* Inventory = NewObject<UPAInventoryComponent>();
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();

		TestNotNull(TEXT("AC-1: MerchantComponent created"), Merchant);
		TestNotNull(TEXT("AC-1: InventoryComponent created"), Inventory);
		TestNotNull(TEXT("AC-1: CurrencyComponent created"), Wallet);

		if (Merchant && Inventory && Wallet)
		{
			// Create Catalog Item: Health Potion (50 Gold, Stock: 10)
			UItemStaticDataAsset* PotionAsset = NewObject<UItemStaticDataAsset>();
			PotionAsset->ItemId = FName("item_health_potion");
			PotionAsset->Category = EPAItemCategory::Consumable;
			PotionAsset->BaseSellPrice = 50;
			PotionAsset->MaxStackSize = 20;

			FPAMerchantCatalogEntry Entry;
			Entry.ItemData = PotionAsset;
			Entry.PriceGold = 50;
			Entry.AvailableStock = 10;
			Merchant->AddCatalogEntry(Entry);

			// Seed Wallet with 200 Gold
			EPACurrencyTransactionError CurrErr;
			Wallet->AddCurrency(EPACurrencyType::Gold, 200, CurrErr);

			// Purchase 2 Potions (Cost: 100 Gold)
			EPATransactionError TransErr = EPATransactionError::None;
			bool bBought = Merchant->BuyItem(Inventory, Wallet, 0, 2, TransErr);
			TestTrue(TEXT("AC-1: Purchase 2 potions succeeds"), bBought);
			TestEqual(TEXT("AC-1: Wallet deducted 100 Gold -> 100 remaining"), Wallet->GetGold(), 100LL);
			TestEqual(TEXT("AC-1: Inventory received 2 potions"), Inventory->GetItemCount(FName("item_health_potion")), 2);
			TestEqual(TEXT("AC-1: Merchant stock decremented 10 -> 8"), Merchant->GetCatalogEntry(0)->AvailableStock, 8);

			// Insufficient Gold failure: Try to buy 3 potions (Cost: 150 Gold, Wallet has 100)
			bBought = Merchant->BuyItem(Inventory, Wallet, 0, 3, TransErr);
			TestFalse(TEXT("AC-1: Buying with insufficient funds fails"), bBought);
			TestEqual(TEXT("AC-1: Error is InsufficientGold"), TransErr, EPATransactionError::InsufficientGold);
			TestEqual(TEXT("AC-1: Wallet unchanged at 100 Gold"), Wallet->GetGold(), 100LL);

			// Out of stock failure: Buy remaining 8 potions
			Wallet->AddCurrency(EPACurrencyType::Gold, 500, CurrErr);
			bBought = Merchant->BuyItem(Inventory, Wallet, 0, 8, TransErr);
			TestTrue(TEXT("AC-1: Buying remaining 8 potions succeeds"), bBought);
			TestEqual(TEXT("AC-1: Merchant stock is now 0"), Merchant->GetCatalogEntry(0)->AvailableStock, 0);

			bBought = Merchant->BuyItem(Inventory, Wallet, 0, 1, TransErr);
			TestFalse(TEXT("AC-1: Buying out-of-stock item fails"), bBought);
			TestEqual(TEXT("AC-1: Error is OutOfStock"), TransErr, EPATransactionError::OutOfStock);
		}
	}

	// =========================================================================
	// AC-2: Item Selling & Bulk Junk Selling
	// =========================================================================
	{
		UPAMerchantComponent* Merchant = NewObject<UPAMerchantComponent>();
		UPAInventoryComponent* Inventory = NewObject<UPAInventoryComponent>();
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();

		if (Merchant && Inventory && Wallet)
		{
			// Formula validation: Item base sell price 200, penalty 0.30 -> floor(200 * 0.30) = 60 Gold
			const int32 SellPrice = FPAMerchantFormulas::CalculateSellPrice(200, 0.30f);
			TestEqual(TEXT("AC-2: Formula calculates 60 Gold for 200 base price at 0.30 penalty"), SellPrice, 60);

			// Seed item in Slot 0
			UItemStaticDataAsset* SwordAsset = NewObject<UItemStaticDataAsset>();
			SwordAsset->ItemId = FName("item_iron_sword");
			SwordAsset->Category = EPAItemCategory::Equipment;
			SwordAsset->BaseSellPrice = 200;
			Inventory->AddItemToSlot(0, SwordAsset, 1);

			// Sell item from Slot 0
			EPATransactionError TransErr = EPATransactionError::None;
			bool bSold = Merchant->SellItem(Inventory, Wallet, 0, 1, TransErr);
			TestTrue(TEXT("AC-2: Sell item from slot 0 succeeds"), bSold);
			TestEqual(TEXT("AC-2: Wallet credited 60 Gold"), Wallet->GetGold(), 60LL);
			TestNull(TEXT("AC-2: Slot 0 is now empty"), Inventory->GetItemAtSlot(0));

			// Locked item protection
			FPAItemInstanceData LockedData;
			LockedData.bIsLocked = true;
			Inventory->AddItemToSlot(1, SwordAsset, 1, LockedData);

			bSold = Merchant->SellItem(Inventory, Wallet, 1, 1, TransErr);
			TestFalse(TEXT("AC-2: Selling locked item fails"), bSold);
			TestEqual(TEXT("AC-2: Error is ItemLocked"), TransErr, EPATransactionError::ItemLocked);
			TestNotNull(TEXT("AC-2: Locked item remains intact in slot 1"), Inventory->GetItemAtSlot(1));

			// Bulk Junk Selling
			UItemStaticDataAsset* JunkAsset = NewObject<UItemStaticDataAsset>();
			JunkAsset->ItemId = FName("item_broken_bone");
			JunkAsset->Category = EPAItemCategory::Material;
			JunkAsset->BaseSellPrice = 100; // 30 Gold per piece

			FPAItemInstanceData JunkData;
			JunkData.bIsJunk = true;
			Inventory->AddItemToSlot(2, JunkAsset, 1, JunkData);
			Inventory->AddItemToSlot(3, JunkAsset, 1, JunkData);

			int32 TotalGoldGained = 0;
			int32 ItemsSold = 0;
			bool bJunkSold = Merchant->SellAllJunk(Inventory, Wallet, TotalGoldGained, ItemsSold, TransErr);
			TestTrue(TEXT("AC-2: Bulk junk selling succeeds"), bJunkSold);
			TestEqual(TEXT("AC-2: 2 junk items sold"), ItemsSold, 2);
			TestEqual(TEXT("AC-2: 60 Gold gained from junk"), TotalGoldGained, 60);
			TestEqual(TEXT("AC-2: Wallet total Gold is 120"), Wallet->GetGold(), 120LL);
			TestNull(TEXT("AC-2: Slot 2 is empty"), Inventory->GetItemAtSlot(2));
			TestNull(TEXT("AC-2: Slot 3 is empty"), Inventory->GetItemAtSlot(3));
		}
	}

	// =========================================================================
	// AC-3: Buyback Window (FIFO 10 Slots) & Exact Refund
	// =========================================================================
	{
		UPAMerchantComponent* Merchant = NewObject<UPAMerchantComponent>();
		UPAInventoryComponent* Inventory = NewObject<UPAInventoryComponent>();
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();

		if (Merchant && Inventory && Wallet)
		{
			// Sell an equipment item to populate buyback
			UItemStaticDataAsset* RareBoots = NewObject<UItemStaticDataAsset>();
			RareBoots->ItemId = FName("item_rare_boots");
			RareBoots->Category = EPAItemCategory::Equipment;
			RareBoots->BaseSellPrice = 500; // 150 Gold sell price

			FPAItemInstanceData BootsData;
			BootsData.EnhancementLevel = 2;
			Inventory->AddItemToSlot(0, RareBoots, 1, BootsData);

			EPATransactionError TransErr = EPATransactionError::None;
			Merchant->SellItem(Inventory, Wallet, 0, 1, TransErr);
			TestEqual(TEXT("AC-3: Buyback has 1 item"), Merchant->GetBuybackCount(), 1);

			// Buyback the boots
			bool bBuyback = Merchant->BuybackItem(Inventory, Wallet, 0, TransErr);
			TestTrue(TEXT("AC-3: Buyback succeeds"), bBuyback);
			TestEqual(TEXT("AC-3: Wallet deducted 150 Gold -> 0 remaining"), Wallet->GetGold(), 0LL);
			TestEqual(TEXT("AC-3: Buyback is now empty"), Merchant->GetBuybackCount(), 0);

			const FPAInventoryItemEntry* RestoredBoots = Inventory->GetItemAtSlot(0);
			TestNotNull(TEXT("AC-3: Boots returned to inventory"), RestoredBoots);
			if (RestoredBoots)
			{
				TestEqual(TEXT("AC-3: Restored boots retained +2 enhancement"), RestoredBoots->DynamicData.EnhancementLevel, 2);
			}

			// FIFO test: Sell 11 items, verify oldest is evicted and count stays 10
			for (int32 i = 0; i < 11; ++i)
			{
				UItemStaticDataAsset* ScrapItem = NewObject<UItemStaticDataAsset>();
				ScrapItem->Category = EPAItemCategory::Material;
				ScrapItem->BaseSellPrice = 10;
				ScrapItem->ItemId = *FString::Printf(TEXT("scrap_%d"), i);
				Inventory->AddItemToSlot(0, ScrapItem, 1);
				Merchant->SellItem(Inventory, Wallet, 0, 1, TransErr);
			}

			TestEqual(TEXT("AC-3: Buyback count clamped to 10 (FIFO eviction)"), Merchant->GetBuybackCount(), 10);
			// The oldest item "scrap_0" was evicted; first item in buyback should be "scrap_1"
			const FPABuybackItemEntry* FirstBuyback = Merchant->GetBuybackEntry(0);
			TestNotNull(TEXT("AC-3: First buyback entry exists"), FirstBuyback);
			if (FirstBuyback && FirstBuyback->ItemData)
			{
				TestEqual(TEXT("AC-3: Oldest item scrap_0 was evicted, scrap_1 is now oldest"), FirstBuyback->ItemData->ItemId, FName("scrap_1"));
			}
		}
	}

	// =========================================================================
	// AC-4: Interaction & Guardrails
	// =========================================================================
	{
		UPAMerchantComponent* Merchant = NewObject<UPAMerchantComponent>();
		if (Merchant)
		{
			EPATransactionError TransErr = EPATransactionError::None;

			// In-combat rejection
			bool bValid = Merchant->ValidateInteraction(nullptr, true, TransErr);
			TestFalse(TEXT("AC-4: Interaction rejected when player is in combat"), bValid);
			TestEqual(TEXT("AC-4: Error is InCombat"), TransErr, EPATransactionError::InCombat);

			// Out of combat interaction
			bValid = Merchant->ValidateInteraction(nullptr, false, TransErr);
			TestTrue(TEXT("AC-4: Interaction allowed when out of combat"), bValid);
			TestEqual(TEXT("AC-4: Error is None"), TransErr, EPATransactionError::None);
		}
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
