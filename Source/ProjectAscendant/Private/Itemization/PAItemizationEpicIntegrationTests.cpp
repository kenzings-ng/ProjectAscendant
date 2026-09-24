// Copyright Project Ascendant. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Itemization/PAAffixTypes.h"
#include "Itemization/PASavedItemInstance.h"
#include "Itemization/PAServerItemGeneratorSubsystem.h"
#include "Character/PAPaperdollTypes.h"
#include "Character/PAPaperdollComponent.h"
#include "Crafting/PABlacksmithTypes.h"
#include "Crafting/PABlacksmithSubsystem.h"
#include "Economy/PACurrencyComponent.h"
#include "Combat/AscendantAttributeSet.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAItemizationEpicIntegrationTest
 *
 * Kiểm thử tích hợp thông suốt toàn bộ chuỗi vòng đời vật phẩm (EPIC-ITEMIZATION-001, Sprint 6):
 * 1. Generate item qua lệnh Cheat.GenerateItem (item-002)
 * 2. Trang bị vào Paperdoll 9-Slot & kiểm tra Directional Sort Key (item-004)
 * 3. Mở khóa ô Socket tại Field Forge bằng giao dịch song tiền tệ (item-003, item-007)
 * 4. Khảm Gem (Ruby) và kiểm tra cộng chỉ số vào UAscendantAttributeSet (item-003 AC-4)
 * 5. Tẩy lại dòng Affix (Reforge) tiêu hao 2,000 Gold + 5 Skill Shards (item-007)
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAItemizationEpicIntegrationTest,
	"ProjectAscendant.Itemization.EpicIntegrationPipeline",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAItemizationEpicIntegrationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("Epic Pipeline: GameInstance must be valid"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UPAServerItemGeneratorSubsystem* ItemGenerator = NewObject<UPAServerItemGeneratorSubsystem>(GameInstance);
	TestNotNull(TEXT("Epic Pipeline: ItemGenerator must be valid"), ItemGenerator);
	if (!ItemGenerator)
	{
		return false;
	}
	ItemGenerator->InitializeAffixDefinitions(nullptr);

	UPABlacksmithSubsystem* Blacksmith = NewObject<UPABlacksmithSubsystem>(GameInstance);
	TestNotNull(TEXT("Epic Pipeline: Blacksmith Subsystem must be valid"), Blacksmith);
	if (!Blacksmith)
	{
		return false;
	}

	// =========================================================================
	// BƯỚC 1: Sinh vật phẩm qua Cheat.GenerateItem (item-002)
	// =========================================================================
	FPASavedItemInstance GeneratedItem;
	const TArray<FString> CheatArgs = { TEXT("25"), TEXT("Rare"), TEXT("Tier2_Field") };
	bool bGenerated = UPAServerItemGeneratorSubsystem::ExecuteCheatGenerateItem(CheatArgs, ItemGenerator, &GeneratedItem);
	TestTrue(TEXT("Step 1: Cheat.GenerateItem executed successfully"), bGenerated);
	TestTrue(TEXT("Step 1: Item UID is valid"), GeneratedItem.ItemInstanceUID.IsValid());
	TestEqual(TEXT("Step 1: Item Level is 25"), GeneratedItem.ItemLevel, 25);
	TestEqual(TEXT("Step 1: Rare item has 3 affixes"), GeneratedItem.ActiveAffixes.Num(), 3);
	TestEqual(TEXT("Step 1: Rare item has 1 socket slot"), GeneratedItem.SocketSlots.Num(), 1);
	TestFalse(TEXT("Step 1: Socket slot is initially locked"), GeneratedItem.SocketSlots[0].bIsUnlocked);

	// =========================================================================
	// BƯỚC 2: Trang bị vào Paperdoll 9-Slot, Directional Sort Key & Material Instance (item-004, item-005)
	// =========================================================================
	{
		FPAPaperdollModel Model;

		// Kiểm tra Directional Sort Key trước khi trang bị:
		// Hướng Đông (Default): MainHand priority 50, OffHand 5
		const int32 EastMain = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::MainHand, EPAAimDirection8Way::East);
		const int32 EastOff = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::OffHand, EPAAimDirection8Way::East);
		TestEqual(TEXT("Step 2: East MainHand sort key is 50"), EastMain, 50);
		TestEqual(TEXT("Step 2: East OffHand sort key is 5"), EastOff, 5);

		// Trang bị vũ khí vừa sinh vào slot MainHand
		const FName WeaponVisualId = FName("Visual_GeneratedWeapon");
		bool bEquipped = Model.EquipSlot(EPAPaperdollSlot::MainHand, GeneratedItem.ItemDefId, WeaponVisualId);
		TestTrue(TEXT("Step 2: EquipSlot to MainHand succeeds"), bEquipped);
		TestTrue(TEXT("Step 2: MainHand slot is occupied"), Model.IsSlotEquipped(EPAPaperdollSlot::MainHand));
		TestEqual(TEXT("Step 2: Active visual matches"), Model.GetActiveVisualAssetForSlot(EPAPaperdollSlot::MainHand), WeaponVisualId);

		// Đổi sang hướng Tây (Mirrored): Directional Sort Key đảo ngược (MainHand = 5, OffHand = 50)
		const int32 WestMain = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::MainHand, EPAAimDirection8Way::West);
		const int32 WestOff = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::OffHand, EPAAimDirection8Way::West);
		TestEqual(TEXT("Step 2: West (Mirrored) MainHand sort key is 5"), WestMain, 5);
		TestEqual(TEXT("Step 2: West (Mirrored) OffHand sort key is 50"), WestOff, 50);
		TestTrue(TEXT("Step 2: Z-order preserved under horizontal mirror"), WestOff > WestMain);

		// Áp dụng Static Material Instance theo RarityTag của item (item-005)
		UPAPaperdollComponent* PaperdollComp = NewObject<UPAPaperdollComponent>();
		TestNotNull(TEXT("Step 2: PaperdollComponent created"), PaperdollComp);
		if (PaperdollComp)
		{
			bool bMatApplied = PaperdollComp->SetWeaponMaterialForRarity(GeneratedItem.RarityTag);
			TestTrue(TEXT("Step 2: Static Material Instance applied for item rarity"), bMatApplied);
		}
	}

	// =========================================================================
	// BƯỚC 3: Mở khóa Socket tại Field Forge qua Dual-Currency (item-003, item-007)
	// =========================================================================
	UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();
	EPACurrencyTransactionError CurrErr;
	Wallet->AddCurrency(EPACurrencyType::Gold, 10000, CurrErr);
	Wallet->AddCurrency(EPACurrencyType::AshShards, 20, CurrErr);

	EPACraftingError SocketErr = EPACraftingError::None;
	bool bSocketUnlocked = Blacksmith->ServerAddSocket(
		GeneratedItem, 1000, 3, EPAForgeTier::Tier2_Field, Wallet, SocketErr);
	TestTrue(TEXT("Step 3: ServerAddSocket at Field Forge succeeds"), bSocketUnlocked);
	TestTrue(TEXT("Step 3: Socket 0 is unlocked"), GeneratedItem.SocketSlots[0].bIsUnlocked);
	TestEqual(TEXT("Step 3: Gold deducted 1,000 -> 9,000"), Wallet->GetGold(), 9000LL);
	TestEqual(TEXT("Step 3: Shards deducted 3 -> 17"), Wallet->GetAshShards(), 17LL);

	// =========================================================================
	// BƯỚC 4: Khảm Gem (Ruby) và cộng chỉ số vào AttributeSet (item-003 AC-4)
	// =========================================================================
	UAscendantAttributeSet* AttributeSet = NewObject<UAscendantAttributeSet>();
	AttributeSet->InitMaxPosture(100.0f);
	AttributeSet->InitPosture(100.0f);

	EPACraftingError GemErr = EPACraftingError::None;
	bool bGemSocketed = Blacksmith->SocketGem(GeneratedItem, 0, FName("ruby"), AttributeSet, GemErr);
	TestTrue(TEXT("Step 4: SocketGem Ruby succeeds"), bGemSocketed);
	TestEqual(TEXT("Step 4: Socket 0 contains Ruby"), GeneratedItem.SocketSlots[0].SocketedGemId, FName("ruby"));
	TestEqual(TEXT("Step 4: AttributeSet MaxPosture gained +20 from Ruby (120.0)"),
		AttributeSet->GetMaxPosture(), 120.0f);

	// =========================================================================
	// BƯỚC 5: Tẩy lại dòng Affix (Reforge) tiêu hao 2,000 Gold + 5 Skill Shards (item-007)
	// =========================================================================
	EPACraftingError ReforgeErr = EPACraftingError::None;
	bool bReforged = Blacksmith->ServerReforgeAffix(
		GeneratedItem, 0, 2000, 5, EPAForgeTier::Tier2_Field, Wallet, ItemGenerator, ReforgeErr);
	TestTrue(TEXT("Step 5: ServerReforgeAffix succeeds"), bReforged);
	TestEqual(TEXT("Step 5: Gold deducted 2,000 -> 7,000"), Wallet->GetGold(), 7000LL);
	TestEqual(TEXT("Step 5: Shards deducted 5 -> 12"), Wallet->GetAshShards(), 12LL);
	TestTrue(TEXT("Step 5: Reforged Affix 0 has valid ID"), !GeneratedItem.ActiveAffixes[0].AffixId.IsNone());
	TestTrue(TEXT("Step 5: Reforged Affix 0 has positive value"), GeneratedItem.ActiveAffixes[0].RolledValue > 0.0f);

	// Đảm bảo không trùng affix trên món đồ
	TSet<FName> AffixSet;
	for (const FPAAffixInstance& Affix : GeneratedItem.ActiveAffixes)
	{
		TestFalse(FString::Printf(TEXT("Step 5: Unique affix [%s]"), *Affix.AffixId.ToString()),
			AffixSet.Contains(Affix.AffixId));
		AffixSet.Add(Affix.AffixId);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
