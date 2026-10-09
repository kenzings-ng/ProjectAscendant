// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Crafting/PABlacksmithComponent.h"
#include "Crafting/PABlacksmithTypes.h"
#include "Economy/PACurrencyComponent.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAInventoryTypes.h"
#include "Inventory/PAItemStaticDataAsset.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * X6 — Thang độ hiếm kỹ năng 4 bậc độc lập với thang trang bị 5 bậc (DECISIONS.md §5).
 *
 *  - EPASkillRarity có đúng 4 giá trị theo thứ tự Normal -> Rare -> Epic -> Mythic, không có "Tier" trong tên hiển thị.
 *  - Phân rã Sách Kỹ Năng theo SkillRarity: 1 / 3 / 8 / 25 Tàn Trang (skill-progression-system.md).
 *  - Phân rã trang bị vẫn theo EPAItemRarity, không bị SkillRarity ảnh hưởng.
 *  - Preset Sách Dash dùng thang kỹ năng (Normal), không mang độ hiếm trang bị.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPASkillRarityScaleTest,
	"ProjectAscendant.Foundation.Inventory.SkillRarityScale",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPASkillRarityScaleTest::RunTest(const FString& Parameters)
{
	// -------------------------------------------------------------------------
	// 1. Enum: exactly 4 values, in order, no "Tier" in display names
	// -------------------------------------------------------------------------
	{
		const UEnum* SkillEnum = StaticEnum<EPASkillRarity>();
		TestNotNull(TEXT("EPASkillRarity is reflected"), SkillEnum);
		if (SkillEnum)
		{
			// NumEnums() includes the generated _MAX entry.
			TestEqual(TEXT("EPASkillRarity has exactly 4 values"), SkillEnum->NumEnums() - 1, 4);

			const TCHAR* ExpectedNames[] = { TEXT("Normal"), TEXT("Rare"), TEXT("Epic"), TEXT("Mythic") };
			for (int32 Index = 0; Index < 4; ++Index)
			{
				TestEqual(FString::Printf(TEXT("EPASkillRarity[%d] name"), Index), SkillEnum->GetNameStringByIndex(Index), FString(ExpectedNames[Index]));
				TestEqual(FString::Printf(TEXT("EPASkillRarity[%d] value"), Index), SkillEnum->GetValueByIndex(Index), static_cast<int64>(Index));
				const FString DisplayName = SkillEnum->GetDisplayNameTextByIndex(Index).ToString();
				TestFalse(FString::Printf(TEXT("EPASkillRarity[%d] display name has no 'Tier'"), Index), DisplayName.Contains(TEXT("Tier")));
				TestFalse(FString::Printf(TEXT("EPASkillRarity[%d] display name has no 'Bậc'"), Index), DisplayName.Contains(TEXT("Bậc")));
			}

			TestEqual(TEXT("No Legendary on skill scale"), SkillEnum->GetIndexByNameString(TEXT("Legendary")), static_cast<int32>(INDEX_NONE));
			TestEqual(TEXT("No Divine on skill scale"), SkillEnum->GetIndexByNameString(TEXT("Divine")), static_cast<int32>(INDEX_NONE));
		}

		TestTrue(TEXT("Normal < Rare < Epic < Mythic"),
			EPASkillRarity::Normal < EPASkillRarity::Rare && EPASkillRarity::Rare < EPASkillRarity::Epic && EPASkillRarity::Epic < EPASkillRarity::Mythic);

		// Equipment scale untouched: None + 5 rarities.
		const UEnum* ItemEnum = StaticEnum<EPAItemRarity>();
		TestNotNull(TEXT("EPAItemRarity is reflected"), ItemEnum);
		if (ItemEnum)
		{
			TestEqual(TEXT("EPAItemRarity still has None + 5 equipment rarities"), ItemEnum->NumEnums() - 1, 6);
		}

		UItemStaticDataAsset* DefaultAsset = NewObject<UItemStaticDataAsset>();
		TestEqual(TEXT("Default SkillRarity is Normal"), DefaultAsset->SkillRarity, EPASkillRarity::Normal);
		TestEqual(TEXT("Default equipment RarityTier unchanged (Common)"), DefaultAsset->RarityTier, EPAItemRarity::Common);
	}

	// -------------------------------------------------------------------------
	// 2. Skill book salvage formula: 1 / 3 / 8 / 25, independent of EPAItemRarity
	// -------------------------------------------------------------------------
	{
		TestEqual(TEXT("Normal book -> 1 shard"), FPABlacksmithFormulas::GetSkillBookSalvageShards(EPASkillRarity::Normal), 1);
		TestEqual(TEXT("Rare book -> 3 shards"), FPABlacksmithFormulas::GetSkillBookSalvageShards(EPASkillRarity::Rare), 3);
		TestEqual(TEXT("Epic book -> 8 shards"), FPABlacksmithFormulas::GetSkillBookSalvageShards(EPASkillRarity::Epic), 8);
		TestEqual(TEXT("Mythic book -> 25 shards"), FPABlacksmithFormulas::GetSkillBookSalvageShards(EPASkillRarity::Mythic), 25);

		// Equipment rarity on a skill book is ignored.
		TestEqual(TEXT("Skill book: Legendary equipment rarity ignored (Normal -> 1)"),
			FPABlacksmithFormulas::GetSalvageSkillShards(EPAItemRarity::Legendary, EPAItemCategory::SkillBook, EPASkillRarity::Normal), 1);
		TestEqual(TEXT("Skill book: Common equipment rarity ignored (Mythic -> 25)"),
			FPABlacksmithFormulas::GetSalvageSkillShards(EPAItemRarity::Common, EPAItemCategory::SkillBook, EPASkillRarity::Mythic), 25);
	}

	// -------------------------------------------------------------------------
	// 3. Equipment salvage unchanged (1/3/10/25/75 by EPAItemRarity), SkillRarity ignored
	// -------------------------------------------------------------------------
	{
		const EPAItemRarity EquipRarities[] = { EPAItemRarity::Common, EPAItemRarity::Uncommon, EPAItemRarity::Rare, EPAItemRarity::Epic, EPAItemRarity::Legendary };
		const int32 ExpectedEquip[] = { 1, 3, 10, 25, 75 };
		for (int32 Index = 0; Index < 5; ++Index)
		{
			TestEqual(FString::Printf(TEXT("Equipment rarity %d salvage unchanged (SkillRarity Normal)"), Index + 1),
				FPABlacksmithFormulas::GetSalvageSkillShards(EquipRarities[Index], EPAItemCategory::Equipment, EPASkillRarity::Normal), ExpectedEquip[Index]);
			TestEqual(FString::Printf(TEXT("Equipment rarity %d salvage unchanged (SkillRarity Mythic)"), Index + 1),
				FPABlacksmithFormulas::GetSalvageSkillShards(EquipRarities[Index], EPAItemCategory::Equipment, EPASkillRarity::Mythic), ExpectedEquip[Index]);
		}
	}

	// -------------------------------------------------------------------------
	// 4. End-to-end: UPABlacksmithComponent::SalvageItem on skill books of each rarity
	// -------------------------------------------------------------------------
	{
		UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>();
		UPAInventoryComponent* Inventory = NewObject<UPAInventoryComponent>();
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();

		const EPASkillRarity BookRarities[] = { EPASkillRarity::Normal, EPASkillRarity::Rare, EPASkillRarity::Epic, EPASkillRarity::Mythic };
		const int32 ExpectedShards[] = { 1, 3, 8, 25 };
		int64 RunningTotal = 0;

		for (int32 Index = 0; Index < 4; ++Index)
		{
			UItemStaticDataAsset* Book = NewObject<UItemStaticDataAsset>();
			Book->ItemId = FName(*FString::Printf(TEXT("item_test_skill_book_%d"), Index));
			Book->Category = EPAItemCategory::SkillBook;
			Book->MaxStackSize = 1;
			Book->RarityTier = EPAItemRarity::Legendary; // must be ignored for skill books
			Book->SkillRarity = BookRarities[Index];

			Inventory->AddItemToSlot(Index, Book, 1);

			int32 ShardsGained = 0;
			EPACraftingError CraftErr = EPACraftingError::None;
			const bool bSalvaged = Blacksmith->SalvageItem(Inventory, Wallet, Index, ShardsGained, CraftErr);
			RunningTotal += ExpectedShards[Index];

			TestTrue(FString::Printf(TEXT("SalvageItem succeeds for skill book %d"), Index), bSalvaged);
			TestEqual(FString::Printf(TEXT("Skill book %d yields GDD shards"), Index), ShardsGained, ExpectedShards[Index]);
			TestEqual(FString::Printf(TEXT("Wallet shards after book %d"), Index), Wallet->GetSkillShards(), RunningTotal);
			TestNull(FString::Printf(TEXT("Skill book %d consumed"), Index), Inventory->GetItemAtSlot(Index));
		}
	}

	// -------------------------------------------------------------------------
	// 5. Dash skill book preset uses the skill scale (Vanguard T1 -> Normal)
	// -------------------------------------------------------------------------
	{
		UItemStaticDataAsset* DashBook = NewObject<UItemStaticDataAsset>();
		FPAItemDataAssetPresets::ConfigureSkillBookDash(DashBook);
		TestEqual(TEXT("Dash book category is SkillBook"), DashBook->Category, EPAItemCategory::SkillBook);
		TestEqual(TEXT("Dash book SkillRarity is Normal"), DashBook->SkillRarity, EPASkillRarity::Normal);
		TestEqual(TEXT("Dash book carries no equipment rarity"), DashBook->RarityTier, EPAItemRarity::None);
		TestEqual(TEXT("Dash book salvages into 1 shard"),
			FPABlacksmithFormulas::GetSalvageSkillShards(DashBook->RarityTier, DashBook->Category, DashBook->SkillRarity), 1);

		// Equipment presets keep their equipment rarity.
		UItemStaticDataAsset* Plate = NewObject<UItemStaticDataAsset>();
		FPAItemDataAssetPresets::ConfigureArmorIronPlate(Plate);
		TestEqual(TEXT("Iron Plate keeps equipment rarity Rare"), Plate->RarityTier, EPAItemRarity::Rare);
		TestEqual(TEXT("Iron Plate salvage unchanged (10)"),
			FPABlacksmithFormulas::GetSalvageSkillShards(Plate->RarityTier, Plate->Category, Plate->SkillRarity), 10);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
