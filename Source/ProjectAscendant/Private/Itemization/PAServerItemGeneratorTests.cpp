// Copyright Project Ascendant. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Itemization/PAAffixTypes.h"
#include "Itemization/PAAffixDataLoader.h"
#include "Itemization/PASavedItemInstance.h"
#include "Itemization/PAServerItemGeneratorSubsystem.h"
#include "Inventory/PAInventoryTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAServerItemGeneratorTest,
	"ProjectAscendant.Itemization.ServerItemGenerator",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAServerItemGeneratorTest::RunTest(const FString& Parameters)
{
	// 1. Khởi tạo UGameInstance & Subsystem với Outer chuẩn (Within = GameInstance)
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance must be valid"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UPAServerItemGeneratorSubsystem* Generator = NewObject<UPAServerItemGeneratorSubsystem>(GameInstance);
	TestNotNull(TEXT("Generator Subsystem must be valid"), Generator);
	if (!Generator)
	{
		return false;
	}

	UDataTable* AffixTable = UPAAffixDataLoader::CreateAffixDefinitionsTable(Generator);
	Generator->InitializeAffixDefinitions(AffixTable);

	const TArray<FPAAffixDefinitionRow>& AllDefs = Generator->GetAffixDefinitions();
	TestEqual(TEXT("Generator must load all 28 canonical affixes"), AllDefs.Num(), 28);

	// Xây dựng map lookup nhanh theo AffixName để kiểm tra MinForgeTier
	TMap<FName, FPAAffixDefinitionRow> DefMap;
	for (const FPAAffixDefinitionRow& Def : AllDefs)
	{
		DefMap.Add(Def.AffixName, Def);
	}

	// =========================================================================
	// AC-1: Kiểm tra quy tắc phân bổ Affix theo Rarity (Common, Uncommon, Rare, Epic, Legendary)
	// =========================================================================
	{
		// Common: 0 affix
		for (int32 i = 0; i < 50; ++i)
		{
			FPASavedItemInstance CommonItem = Generator->GenerateItemInstance(
				FName("Test_Sword"),
				FMath::RandRange(1, 50),
				EPAItemRarity::Common,
				EPAForgeTier::Tier1_Outpost);

			TestEqual(TEXT("Common item must have 0 affixes"), CommonItem.ActiveAffixes.Num(), 0);
			TestEqual(TEXT("Common item has 0 sockets"), CommonItem.SocketSlots.Num(), 0);
		}

		// Uncommon: 1-2 affixes
		for (int32 i = 0; i < 50; ++i)
		{
			FPASavedItemInstance UncommonItem = Generator->GenerateItemInstance(
				FName("Test_Sword"),
				FMath::RandRange(1, 50),
				EPAItemRarity::Uncommon,
				EPAForgeTier::Tier1_Outpost);

			TestTrue(
				TEXT("Uncommon item must have 1 or 2 affixes"),
				UncommonItem.ActiveAffixes.Num() >= 1 && UncommonItem.ActiveAffixes.Num() <= 2);
		}

		// Rare: đúng 3 affixes (2 Prefixes + 1 Suffix)
		for (int32 i = 0; i < 50; ++i)
		{
			FPASavedItemInstance RareItem = Generator->GenerateItemInstance(
				FName("Test_Sword"),
				FMath::RandRange(16, 40),
				EPAItemRarity::Rare,
				EPAForgeTier::Tier2_Field);

			TestEqual(TEXT("Rare item must have exactly 3 affixes"), RareItem.ActiveAffixes.Num(), 3);

			int32 PrefixCount = 0;
			int32 SuffixCount = 0;
			for (const FPAAffixInstance& Affix : RareItem.ActiveAffixes)
			{
				if (Affix.AffixType == EPAAffixType::Prefix)
				{
					PrefixCount++;
				}
				else if (Affix.AffixType == EPAAffixType::Suffix)
				{
					SuffixCount++;
				}
			}
			TestEqual(TEXT("Rare item must have 2 Prefixes"), PrefixCount, 2);
			TestEqual(TEXT("Rare item must have 1 Suffix"), SuffixCount, 1);
			TestEqual(TEXT("Rare item has 1 Regular socket capacity"), RareItem.SocketSlots.Num(), 1);
			if (RareItem.SocketSlots.Num() == 1)
			{
				TestFalse(TEXT("Rare socket must start LOCKED (bIsUnlocked = false)"), RareItem.SocketSlots[0].bIsUnlocked);
			}
		}

		// Epic & Legendary: đúng 4 affixes (2 Prefixes + 2 Suffixes)
		for (int32 i = 0; i < 50; ++i)
		{
			FPASavedItemInstance EpicItem = Generator->GenerateItemInstance(
				FName("Test_Sword"),
				FMath::RandRange(30, 50),
				EPAItemRarity::Epic,
				EPAForgeTier::Tier3_Forbidden);

			TestEqual(TEXT("Epic item must have exactly 4 affixes"), EpicItem.ActiveAffixes.Num(), 4);

			int32 PrefixCount = 0;
			int32 SuffixCount = 0;
			for (const FPAAffixInstance& Affix : EpicItem.ActiveAffixes)
			{
				if (Affix.AffixType == EPAAffixType::Prefix)
				{
					PrefixCount++;
				}
				else if (Affix.AffixType == EPAAffixType::Suffix)
				{
					SuffixCount++;
				}
			}
			TestEqual(TEXT("Epic item must have 2 Prefixes"), PrefixCount, 2);
			TestEqual(TEXT("Epic item must have 2 Suffixes"), SuffixCount, 2);
			TestEqual(TEXT("Epic item has 2 Regular socket capacity"), EpicItem.SocketSlots.Num(), 2);
			if (EpicItem.SocketSlots.Num() == 2)
			{
				TestFalse(TEXT("Epic socket 0 must start LOCKED"), EpicItem.SocketSlots[0].bIsUnlocked);
				TestFalse(TEXT("Epic socket 1 must start LOCKED"), EpicItem.SocketSlots[1].bIsUnlocked);
			}

			FPASavedItemInstance LegendaryItem = Generator->GenerateItemInstance(
				FName("Test_Sword"),
				50,
				EPAItemRarity::Legendary,
				EPAForgeTier::Tier3_Forbidden);

			TestEqual(TEXT("Legendary item must have exactly 4 affixes"), LegendaryItem.ActiveAffixes.Num(), 4);
			TestEqual(TEXT("Legendary item has 3 socket capacity (2 Regular + 1 Prismatic)"), LegendaryItem.SocketSlots.Num(), 3);
			if (LegendaryItem.SocketSlots.Num() == 3)
			{
				TestEqual(TEXT("Socket 0 is Regular"), LegendaryItem.SocketSlots[0].SocketType, EPASocketType::Regular);
				TestEqual(TEXT("Socket 1 is Regular"), LegendaryItem.SocketSlots[1].SocketType, EPASocketType::Regular);
				TestEqual(TEXT("Socket 2 is Prismatic"), LegendaryItem.SocketSlots[2].SocketType, EPASocketType::Prismatic);
				TestFalse(TEXT("Legendary socket 0 must start LOCKED"), LegendaryItem.SocketSlots[0].bIsUnlocked);
				TestFalse(TEXT("Legendary socket 1 must start LOCKED"), LegendaryItem.SocketSlots[1].bIsUnlocked);
				TestFalse(TEXT("Legendary socket 2 must start LOCKED"), LegendaryItem.SocketSlots[2].bIsUnlocked);
			}
		}
	}

	// =========================================================================
	// AC-2: Kiểm tra HardCap tuyệt đối (prefix_posture_dmg <= 35%, prefix_stagger_duration <= 0.5s)
	// =========================================================================
	{
		for (int32 i = 0; i < 200; ++i)
		{
			FPASavedItemInstance HighTierItem = Generator->GenerateItemInstance(
				FName("Test_Greatsword"),
				50, // Cấp tối đa để lấy dải Tier 4 cao nhất
				EPAItemRarity::Legendary,
				EPAForgeTier::Tier3_Forbidden);

			for (const FPAAffixInstance& Affix : HighTierItem.ActiveAffixes)
			{
				if (Affix.AffixId == FName("prefix_posture_dmg"))
				{
					TestTrue(
						FString::Printf(TEXT("prefix_posture_dmg (%.2f) must be <= 35.0%%"), Affix.RolledValue),
						Affix.RolledValue <= 35.0f + KINDA_SMALL_NUMBER);
				}
				else if (Affix.AffixId == FName("prefix_stagger_duration"))
				{
					TestTrue(
						FString::Printf(TEXT("prefix_stagger_duration (%.2f) must be <= 0.5s"), Affix.RolledValue),
						Affix.RolledValue <= 0.5f + KINDA_SMALL_NUMBER);
				}
			}
		}
	}

	// =========================================================================
	// AC-3: Stress Test 1.000 lần roll:
	// 1. Không sinh 2 affix trùng tên trên cùng 1 item.
	// 2. Không roll ra affix bậc cao hơn forge tier hiện tại.
	// =========================================================================
	{
		const EPAForgeTier TestTiers[3] = {
			EPAForgeTier::Tier1_Outpost,
			EPAForgeTier::Tier2_Field,
			EPAForgeTier::Tier3_Forbidden
		};

		const EPAItemRarity TestRarities[4] = {
			EPAItemRarity::Uncommon,
			EPAItemRarity::Rare,
			EPAItemRarity::Epic,
			EPAItemRarity::Legendary
		};

		int32 TotalTestedRolls = 0;
		bool bFoundAnyDuplicate = false;
		bool bFoundHigherTierAffix = false;

		for (int32 Iter = 0; Iter < 1000; ++Iter)
		{
			const EPAForgeTier CurrentForge = TestTiers[Iter % 3];
			const EPAItemRarity CurrentRarity = TestRarities[Iter % 4];
			const int32 RandomILvl = FMath::RandRange(1, 50);

			FPASavedItemInstance RolledItem = Generator->GenerateItemInstance(
				FName("RolledItem"),
				RandomILvl,
				CurrentRarity,
				CurrentForge);

			TotalTestedRolls++;

			// 1. Kiểm tra không trùng tên affix trên cùng 1 item
			TSet<FName> SeenAffixNames;
			for (const FPAAffixInstance& Affix : RolledItem.ActiveAffixes)
			{
				if (SeenAffixNames.Contains(Affix.AffixId))
				{
					bFoundAnyDuplicate = true;
					AddError(FString::Printf(TEXT("Iter %d: Duplicate affix '%s' on item!"), Iter, *Affix.AffixId.ToString()));
				}
				SeenAffixNames.Add(Affix.AffixId);

				// 2. Kiểm tra không roll ra affix bậc cao hơn forge tier hiện tại
				const FPAAffixDefinitionRow* Row = DefMap.Find(Affix.AffixId);
				if (Row)
				{
					if (static_cast<uint8>(Row->MinForgeTier) > static_cast<uint8>(CurrentForge))
					{
						bFoundHigherTierAffix = true;
						AddError(FString::Printf(
							TEXT("Iter %d: Rolled affix '%s' requiring Tier %d at current Forge Tier %d!"),
							Iter,
							*Affix.AffixId.ToString(),
							static_cast<int32>(Row->MinForgeTier),
							static_cast<int32>(CurrentForge)));
					}
				}
				else
				{
					AddError(FString::Printf(TEXT("Iter %d: Affix '%s' not found in definitions!"), Iter, *Affix.AffixId.ToString()));
				}
			}
		}

		TestEqual(TEXT("Total endurance rolls must be 1000"), TotalTestedRolls, 1000);
		TestFalse(TEXT("1000 rolls must NOT generate duplicate affix names on any item"), bFoundAnyDuplicate);
		TestFalse(TEXT("1000 rolls must NOT roll affixes higher than current Forge Tier"), bFoundHigherTierAffix);
	}

	// =========================================================================
	// AC-4: Kiểm tra Console Command Cheat.GenerateItem
	// =========================================================================
	{
		// Cú pháp 1: Cheat.GenerateItem 25 Rare 2
		{
			TArray<FString> Args = { TEXT("25"), TEXT("Rare"), TEXT("2") };
			FPASavedItemInstance CheatItem;
			bool bSuccess = UPAServerItemGeneratorSubsystem::ExecuteCheatGenerateItem(Args, Generator, &CheatItem);
			TestTrue(TEXT("Cheat command must succeed with valid args"), bSuccess);
			TestEqual(TEXT("Cheat item level must be 25"), CheatItem.ItemLevel, 25);
			TestEqual(TEXT("Cheat item must have 3 affixes for Rare"), CheatItem.ActiveAffixes.Num(), 3);
		}

		// Cú pháp 2: Cheat.GenerateItem 48 5 (Legendary numeric)
		{
			TArray<FString> Args = { TEXT("48"), TEXT("5"), TEXT("Forbidden") };
			FPASavedItemInstance CheatItem;
			bool bSuccess = UPAServerItemGeneratorSubsystem::ExecuteCheatGenerateItem(Args, Generator, &CheatItem);
			TestTrue(TEXT("Cheat command with numeric rarity must succeed"), bSuccess);
			TestEqual(TEXT("Cheat item must have 4 affixes for Legendary"), CheatItem.ActiveAffixes.Num(), 4);
			TestEqual(TEXT("Cheat item must have 3 sockets"), CheatItem.SocketSlots.Num(), 3);
		}

		// Cú pháp 3: Thiếu tham số -> Phải trả về false và log cảnh báo
		{
			TArray<FString> InvalidArgs = { TEXT("25") };
			bool bSuccess = UPAServerItemGeneratorSubsystem::ExecuteCheatGenerateItem(InvalidArgs, Generator);
			TestFalse(TEXT("Cheat command must fail when missing arguments"), bSuccess);
		}
	}

	return true;
}
