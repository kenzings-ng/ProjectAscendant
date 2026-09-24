// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UI/PAShopForgeUITypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAShopForgeUITests
 *
 * Kiểm thử tự động cho Story hud-004 (Merchant Shop & Blacksmith Forge Interactive Windows):
 *  - AC-1: 2-Column Merchant Shop — catalog, inventory, tab navigation, Karma surcharge 20%.
 *  - AC-2: Blacksmith Forge Anvil — material sufficiency, stat delta preview, ward toggle.
 *  - AC-3: 0.8s Hold-to-Craft — progress bar, cancel on release, trigger on completion.
 *  - AC-4: Proximity & In-Combat Auto-Close — 300cm open, 500cm close, combat guard.
 *
 * Tất cả test chạy trên FPAShopUIModel / FPAForgeUIModel thuần túy (không UMG).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAShopForgeUITests,
	"ProjectAscendant.UI.ShopForgeUI",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
	constexpr float kShopForgeTolerance = 0.01f;

	/** Tạo shop model với catalog mẫu */
	FPAShopUIModel CreateSampleShopModel()
	{
		FPAShopUIModel Model;

		// Catalog 3 items
		FPAShopItemEntry Item1;
		Item1.ItemId = FName("item_health_flask");
		Item1.DisplayName = TEXT("Greater Health Flask");
		Item1.Quantity = -1; // Unlimited
		Item1.PriceGold = 100;
		Item1.FinalPrice = 100;
		Model.CatalogItems.Add(Item1);

		FPAShopItemEntry Item2;
		Item2.ItemId = FName("item_mana_flask");
		Item2.DisplayName = TEXT("Greater Mana Flask");
		Item2.Quantity = -1;
		Item2.PriceGold = 80;
		Item2.FinalPrice = 80;
		Model.CatalogItems.Add(Item2);

		FPAShopItemEntry Item3;
		Item3.ItemId = FName("item_blacksmith_ward");
		Item3.DisplayName = TEXT("Blacksmith Ward");
		Item3.Quantity = 5;
		Item3.PriceGold = 500;
		Item3.FinalPrice = 500;
		Model.CatalogItems.Add(Item3);

		// Inventory 2 items
		FPAShopItemEntry Inv1;
		Inv1.ItemId = FName("item_wolf_pelt");
		Inv1.DisplayName = TEXT("Wolf Pelt");
		Inv1.Quantity = 3;
		Inv1.PriceGold = 25;
		Inv1.FinalPrice = 25;
		Model.InventoryItems.Add(Inv1);

		FPAShopItemEntry Inv2;
		Inv2.ItemId = FName("item_iron_ore");
		Inv2.DisplayName = TEXT("Iron Ore");
		Inv2.Quantity = 10;
		Inv2.PriceGold = 10;
		Inv2.FinalPrice = 10;
		Model.InventoryItems.Add(Inv2);

		return Model;
	}
}

// ---------------------------------------------------------------------------
// RunTest
// ---------------------------------------------------------------------------

bool FPAShopForgeUITests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: 2-Column Merchant Shop Interface
	// ===========================================================
	{
		FPAShopUIModel Model = CreateSampleShopModel();

		// QA Test 1: Catalog và Inventory populate chính xác
		TestEqual(TEXT("AC1: 3 catalog items"), Model.CatalogItems.Num(), 3);
		TestEqual(TEXT("AC1: 2 inventory items"), Model.InventoryItems.Num(), 2);
		TestEqual(TEXT("AC1: Default tab = Buy"), Model.CurrentTab, EPAShopTab::Buy);

		// Tab navigation
		Model.SwitchTab(EPAShopTab::Sell);
		TestEqual(TEXT("AC1: Switch to Sell tab"), Model.CurrentTab, EPAShopTab::Sell);

		Model.SwitchTab(EPAShopTab::Buyback);
		TestEqual(TEXT("AC1: Switch to Buyback tab"), Model.CurrentTab, EPAShopTab::Buyback);

		// Selection
		Model.SwitchTab(EPAShopTab::Buy);
		Model.SelectCatalogItem(1);
		TestEqual(TEXT("AC1: Selected catalog index = 1"), Model.SelectedCatalogIndex, 1);

		Model.SelectInventoryItem(0);
		TestEqual(TEXT("AC1: Selected inventory index = 0"), Model.SelectedInventoryIndex, 0);

		// Tab switch resets selection
		Model.SwitchTab(EPAShopTab::Sell);
		TestEqual(TEXT("AC1: Tab switch resets catalog selection"), Model.SelectedCatalogIndex, -1);
		TestEqual(TEXT("AC1: Tab switch resets inventory selection"), Model.SelectedInventoryIndex, -1);

		// Invalid selection
		Model.SelectCatalogItem(99);
		TestEqual(TEXT("AC1: Invalid catalog index → -1"), Model.SelectedCatalogIndex, -1);

		// Karma surcharge +20%
		Model.ApplyKarmaSurcharge(true);
		TestTrue(TEXT("AC1: Surcharge active"), Model.bKarmaSurchargeActive);
		// Item1: 100 * 1.20 = 120
		TestEqual(TEXT("AC1: Health Flask price with surcharge = 120"), Model.CatalogItems[0].FinalPrice, 120);
		// Item2: 80 * 1.20 = 96
		TestEqual(TEXT("AC1: Mana Flask price with surcharge = 96"), Model.CatalogItems[1].FinalPrice, 96);
		// Item3: 500 * 1.20 = 600
		TestEqual(TEXT("AC1: Ward price with surcharge = 600"), Model.CatalogItems[2].FinalPrice, 600);
		TestTrue(TEXT("AC1: Items flagged with surcharge"), Model.CatalogItems[0].bHasSurcharge);

		// Surcharge off
		Model.ApplyKarmaSurcharge(false);
		TestEqual(TEXT("AC1: Health Flask price without surcharge = 100"), Model.CatalogItems[0].FinalPrice, 100);
		TestFalse(TEXT("AC1: Surcharge flag cleared"), Model.CatalogItems[0].bHasSurcharge);

		// Affordability check
		Model.UpdateAffordability(150);
		TestTrue(TEXT("AC1: Can afford 100g item with 150g"), Model.CatalogItems[0].bCanAfford);
		TestTrue(TEXT("AC1: Can afford 80g item with 150g"), Model.CatalogItems[1].bCanAfford);
		TestFalse(TEXT("AC1: Cannot afford 500g item with 150g"), Model.CatalogItems[2].bCanAfford);
	}

	// ===========================================================
	// AC-1 (Bổ sung): Buyback FIFO (10 entries max)
	// ===========================================================
	{
		FPAShopUIModel Model;

		// QA Test 2: Buyback FIFO — add 12 entries, oldest gets evicted
		for (int32 i = 0; i < 12; ++i)
		{
			FPAShopItemEntry Entry;
			Entry.ItemId = *FString::Printf(TEXT("item_%d"), i);
			Entry.PriceGold = (i + 1) * 10;
			Entry.FinalPrice = (i + 1) * 10;
			Model.AddBuybackEntry(Entry);
		}

		TestEqual(TEXT("AC1: Buyback capped at 10"), Model.BuybackItems.Num(), 10);
		// Oldest 2 (item_0, item_1) evicted → first entry = item_2
		TestEqual(TEXT("AC1: Oldest evicted (first = item_2)"), Model.BuybackItems[0].ItemId, FName("item_2"));
		// Last entry = item_11
		TestEqual(TEXT("AC1: Last entry = item_11"), Model.BuybackItems[9].ItemId, FName("item_11"));

		// Remove buyback entry
		Model.RemoveBuybackEntry(0);
		TestEqual(TEXT("AC1: After remove → 9 entries"), Model.BuybackItems.Num(), 9);
		TestEqual(TEXT("AC1: New first = item_3"), Model.BuybackItems[0].ItemId, FName("item_3"));
	}

	// ===========================================================
	// AC-2: Blacksmith Forge Anvil Interface
	// ===========================================================
	{
		FPAForgeUIModel Model;

		// Material sufficiency
		FPAForgeSlotEntry Slot1;
		Slot1.MaterialId = FName("mat_iron");
		Slot1.RequiredAmount = 5;
		Slot1.OwnedAmount = 10;
		Slot1.UpdateSufficiency();
		TestTrue(TEXT("AC2: Iron sufficient (10 >= 5)"), Slot1.bIsSufficient);

		FPAForgeSlotEntry Slot2;
		Slot2.MaterialId = FName("mat_dragon_scale");
		Slot2.RequiredAmount = 3;
		Slot2.OwnedAmount = 1;
		Slot2.UpdateSufficiency();
		TestFalse(TEXT("AC2: Dragon Scale deficient (1 < 3)"), Slot2.bIsSufficient);

		Model.MaterialSlots.Add(Slot1);
		Model.MaterialSlots.Add(Slot2);
		Model.GoldCost = 200;
		Model.PlayerGold = 500;
		Model.SetTargetEquipment(FName("eq_sword"), TEXT("Dragon Slayer"));

		TestFalse(TEXT("AC2: Cannot forge (insufficient Dragon Scale)"), Model.CanForge());
		TestTrue(TEXT("AC2: Can afford gold"), Model.CanAffordGold());
		TestFalse(TEXT("AC2: Not all materials sufficient"), Model.AllMaterialsSufficient());

		// Fix Dragon Scale
		Model.MaterialSlots[1].OwnedAmount = 5;
		Model.MaterialSlots[1].UpdateSufficiency();
		TestTrue(TEXT("AC2: All materials sufficient now"), Model.AllMaterialsSufficient());
		TestTrue(TEXT("AC2: Can forge now"), Model.CanForge());

		// Stat delta preview
		FPAStatDeltaPreview Preview;
		Preview.StatName = TEXT("Attack");
		Preview.CurrentValue = 50.0f;
		Preview.NewValue = 58.0f;
		TestNearlyEqual(TEXT("AC2: Stat delta = +8"), Preview.GetDelta(), 8.0f, kShopForgeTolerance);

		FString PreviewText = Preview.GetPreviewText();
		TestTrue(TEXT("AC2: Preview contains 'Attack'"), PreviewText.Contains(TEXT("Attack")));
		TestTrue(TEXT("AC2: Preview contains '50 -> 58'"), PreviewText.Contains(TEXT("50 -> 58")));
		TestTrue(TEXT("AC2: Preview contains '(+8)'"), PreviewText.Contains(TEXT("(+8)")));

		// Ward toggle
		TestFalse(TEXT("AC2: Ward off by default"), Model.bUsingWard);
		Model.ToggleWard(true);
		TestTrue(TEXT("AC2: Ward toggled on"), Model.bUsingWard);

		// No equipment → cannot forge
		FPAForgeUIModel EmptyModel;
		TestFalse(TEXT("AC2: No equipment → cannot forge"), EmptyModel.CanForge());
	}

	// ===========================================================
	// AC-3: 0.8s Hold-to-Craft Safe Interaction
	// ===========================================================
	{
		FPAForgeUIModel Model;
		Model.SetTargetEquipment(FName("eq_test"), TEXT("Test Sword"));

		// QA Test 3: Hold 0.4s → progress = 0.5
		bool bTriggered = false;
		const float Step = 0.016f;
		float Elapsed = 0.0f;

		while (Elapsed < 0.4f)
		{
			bTriggered = Model.UpdateHoldProgress(Step, true);
			Elapsed += Step;
		}

		TestNearlyEqual(TEXT("AC3: Hold 0.4s → progress ~0.5"), Model.HoldProgress, 0.5f, 0.05f);
		TestFalse(TEXT("AC3: Not triggered at 0.4s"), bTriggered);
		TestFalse(TEXT("AC3: Not completed at 0.4s"), Model.bHoldCompleted);

		// Release before 0.8s → cancels
		Model.UpdateHoldProgress(Step, false);
		TestNearlyEqual(TEXT("AC3: Release → progress reset to 0"), Model.HoldProgress, 0.0f, kShopForgeTolerance);
		TestFalse(TEXT("AC3: Release → not completed"), Model.bHoldCompleted);

		// Hold full 0.8s+
		Elapsed = 0.0f;
		bTriggered = false;
		while (Elapsed < 0.85f)
		{
			bool bResult = Model.UpdateHoldProgress(Step, true);
			if (bResult) bTriggered = true;
			Elapsed += Step;
		}

		TestTrue(TEXT("AC3: Hold >= 0.8s → triggered"), bTriggered);
		TestTrue(TEXT("AC3: Hold completed"), Model.bHoldCompleted);
		TestNearlyEqual(TEXT("AC3: Progress = 1.0"), Model.HoldProgress, 1.0f, kShopForgeTolerance);

		// Giữ tiếp không trigger lại
		bTriggered = Model.UpdateHoldProgress(Step, true);
		TestFalse(TEXT("AC3: Second hold → no re-trigger"), bTriggered);
	}

	// ===========================================================
	// AC-4: Proximity & In-Combat Auto-Close
	// ===========================================================
	{
		FPAForgeUIModel Model;

		// Mở tại khoảng cách 200cm (< 300cm), không combat
		Model.CurrentDistance = 200.0f;
		Model.bInCombat = false;
		TestTrue(TEXT("AC4: CanOpen at 200cm"), Model.CanOpen());
		Model.Open();
		TestTrue(TEXT("AC4: Widget opened"), Model.bIsOpen);

		// Khoảng cách 400cm (< 500cm) → vẫn mở
		bool bShouldClose = Model.UpdateProximity(400.0f, false);
		TestFalse(TEXT("AC4: 400cm → no auto-close"), bShouldClose);
		TestTrue(TEXT("AC4: Still open at 400cm"), Model.bIsOpen);

		// QA Test 4: Khoảng cách > 500cm → auto-close
		bShouldClose = Model.UpdateProximity(550.0f, false);
		TestTrue(TEXT("AC4: 550cm → auto-close triggered"), bShouldClose);
		TestFalse(TEXT("AC4: Widget closed"), Model.bIsOpen);

		// Mở lại, test combat close
		Model.CurrentDistance = 100.0f;
		Model.bInCombat = false;
		Model.Open();
		TestTrue(TEXT("AC4: Re-opened"), Model.bIsOpen);

		bShouldClose = Model.UpdateProximity(100.0f, true); // InCombat = true
		TestTrue(TEXT("AC4: Combat → auto-close triggered"), bShouldClose);
		TestFalse(TEXT("AC4: Widget closed by combat"), Model.bIsOpen);

		// Cannot open during combat
		TestFalse(TEXT("AC4: Cannot open during combat"), Model.CanOpen());

		// Cannot open at > 300cm
		Model.bInCombat = false;
		Model.CurrentDistance = 350.0f;
		TestFalse(TEXT("AC4: Cannot open at 350cm"), Model.CanOpen());

		// Can open at exactly 300cm
		Model.CurrentDistance = 300.0f;
		TestTrue(TEXT("AC4: Can open at exactly 300cm"), Model.CanOpen());
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
