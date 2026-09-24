// Copyright Project Ascendant. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Itemization/PAAffixTypes.h"
#include "Itemization/PASavedItemInstance.h"
#include "Itemization/PAServerItemGeneratorSubsystem.h"
#include "Crafting/PABlacksmithTypes.h"
#include "Crafting/PABlacksmithSubsystem.h"
#include "Crafting/PABlacksmithComponent.h"
#include "Economy/PACurrencyComponent.h"
#include "Economy/PACurrencyTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAItemizationTransactionTest
 *
 * Kiểm thử tự động cho Story item-007 (Dual-Currency Transactions, EPIC-ITEMIZATION-001, Sprint 6):
 * - AC-1: Server-Authoritative Repair Transaction & Gold sink, rollback khi thiếu vàng.
 * - AC-2: Reforge Transaction (2,000 Gold + 5 Skill Shards), atomic rollback khi thiếu 1 trong 2 loại tiền.
 * - AC-3: AddSocket Transaction kết hợp điều kiện Forge Tier và chi phí song tiền tệ, khóa 100% tại Outpost.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAItemizationTransactionTest,
	"ProjectAscendant.Itemization.DualCurrencyTransactions",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAItemizationTransactionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance must be valid"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UPAServerItemGeneratorSubsystem* ItemGenerator = NewObject<UPAServerItemGeneratorSubsystem>(GameInstance);
	TestNotNull(TEXT("ItemGenerator must be valid"), ItemGenerator);
	if (!ItemGenerator)
	{
		return false;
	}
	ItemGenerator->InitializeAffixDefinitions(nullptr);

	UPABlacksmithSubsystem* Blacksmith = NewObject<UPABlacksmithSubsystem>(GameInstance);
	TestNotNull(TEXT("Blacksmith Subsystem must be valid"), Blacksmith);
	if (!Blacksmith)
	{
		return false;
	}

	// =========================================================================
	// AC-1: Repair Transaction & Gold Sink (Atomic Rollback)
	// =========================================================================
	{
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();
		TestNotNull(TEXT("AC-1: Wallet must be valid"), Wallet);

		FPASavedItemInstance DamagedItem = ItemGenerator->GenerateItemInstance(
			FName("IronSword"), 10, EPAItemRarity::Rare, EPAForgeTier::Tier1_Outpost);
		DamagedItem.CurrentDurability = 40.0f;
		DamagedItem.MaxDurability = 100.0f;

		// 1. Thử sửa chữa khi ví có 0 Gold -> Thất bại, mã lỗi InsufficientGold, giữ nguyên độ bền 40
		EPACraftingError RepairErr = EPACraftingError::None;
		bool bRepairedNoGold = Blacksmith->ServerRepairItem(DamagedItem, Wallet, 50, RepairErr);
		TestFalse(TEXT("AC-1: Repair without gold must fail"), bRepairedNoGold);
		TestEqual(TEXT("AC-1: Error code must be InsufficientGold"), RepairErr, EPACraftingError::InsufficientGold);
		TestEqual(TEXT("AC-1: Durability remains untouched at 40.0"), DamagedItem.CurrentDurability, 40.0f);
		TestEqual(TEXT("AC-1: Wallet gold remains 0"), Wallet->GetGold(), 0LL);

		// 2. Nạp 200 Gold vào ví, sửa chữa với chi phí 50 Gold -> Thành công
		EPACurrencyTransactionError CurrErr;
		Wallet->AddCurrency(EPACurrencyType::Gold, 200, CurrErr);

		bool bRepairedSuccess = Blacksmith->ServerRepairItem(DamagedItem, Wallet, 50, RepairErr);
		TestTrue(TEXT("AC-1: Repair with sufficient gold must succeed"), bRepairedSuccess);
		TestEqual(TEXT("AC-1: Durability restored to 100.0"), DamagedItem.CurrentDurability, 100.0f);
		TestEqual(TEXT("AC-1: Wallet deducted 50 Gold -> 150 remaining"), Wallet->GetGold(), 150LL);

		// 3. Thử sửa chữa khi đồ đã 100% độ bền -> Thất bại MaxDurabilityAlready, 0 Gold bị trừ
		bool bRepairedFull = Blacksmith->ServerRepairItem(DamagedItem, Wallet, 50, RepairErr);
		TestFalse(TEXT("AC-1: Repairing fully durable item must fail"), bRepairedFull);
		TestEqual(TEXT("AC-1: Error is MaxDurabilityAlready"), RepairErr, EPACraftingError::MaxDurabilityAlready);
		TestEqual(TEXT("AC-1: Wallet gold remains 150"), Wallet->GetGold(), 150LL);

		// 4. Kiểm thử qua Component Server RPC
		UPABlacksmithComponent* BlacksmithComp = NewObject<UPABlacksmithComponent>();
		TArray<FPASavedItemInstance> InventoryItems;
		FPASavedItemInstance RpcItem = ItemGenerator->GenerateItemInstance(
			FName("SteelMace"), 15, EPAItemRarity::Rare, EPAForgeTier::Tier1_Outpost);
		RpcItem.CurrentDurability = 60.0f;
		InventoryItems.Add(RpcItem);

		BlacksmithComp->BindSavedItemInventory(&InventoryItems, Wallet);
		BlacksmithComp->Server_RepairItem(RpcItem.ItemInstanceUID, 50);

		TestEqual(TEXT("AC-1: Component RPC restored durability to 100"), InventoryItems[0].CurrentDurability, 100.0f);
		TestEqual(TEXT("AC-1: Wallet deducted 50 Gold -> 100 remaining"), Wallet->GetGold(), 100LL);
	}

	// =========================================================================
	// AC-2: Reforge Transaction & Dual-Currency Atomic Rollback (2000 Gold + 5 Shards)
	// =========================================================================
	{
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();
		FPASavedItemInstance RareItem = ItemGenerator->GenerateItemInstance(
			FName("CrystalBlade"), 25, EPAItemRarity::Rare, EPAForgeTier::Tier2_Field);
		TestEqual(TEXT("AC-2: Rare item generated with 3 affixes"), RareItem.ActiveAffixes.Num(), 3);

		const FName OriginalAffixId = RareItem.ActiveAffixes[0].AffixId;
		const float OriginalValue = RareItem.ActiveAffixes[0].RolledValue;

		// 1. Trường hợp thiếu Vàng (Chỉ có 1,000 Gold, nhưng đủ 10 Shards)
		EPACurrencyTransactionError CurrErr;
		Wallet->AddCurrency(EPACurrencyType::Gold, 1000, CurrErr);
		Wallet->AddCurrency(EPACurrencyType::AshShards, 10, CurrErr);

		EPACraftingError ReforgeErr = EPACraftingError::None;
		bool bReforgeFailGold = Blacksmith->ServerReforgeAffix(
			RareItem, 0, 2000, 5, EPAForgeTier::Tier2_Field, Wallet, ItemGenerator, ReforgeErr);
		TestFalse(TEXT("AC-2: Reforge without sufficient gold must fail"), bReforgeFailGold);
		TestEqual(TEXT("AC-2: Error code is InsufficientGold"), ReforgeErr, EPACraftingError::InsufficientGold);
		TestEqual(TEXT("AC-2: Atomic rollback - Gold unchanged"), Wallet->GetGold(), 1000LL);
		TestEqual(TEXT("AC-2: Atomic rollback - Shards unchanged"), Wallet->GetAshShards(), 10LL);
		TestEqual(TEXT("AC-2: Affix 0 unchanged"), RareItem.ActiveAffixes[0].AffixId, OriginalAffixId);
		TestEqual(TEXT("AC-2: Affix value unchanged"), RareItem.ActiveAffixes[0].RolledValue, OriginalValue);

		// 2. Trường hợp thiếu Tàn Trang (Có 5,000 Gold, nhưng chỉ có 2 Shards < 5)
		Wallet->AddCurrency(EPACurrencyType::Gold, 4000, CurrErr); // Tổng 5000 Gold
		Wallet->DeductCurrency(EPACurrencyType::AshShards, 8, CurrErr); // Còn lại 2 Shards

		bool bReforgeFailShards = Blacksmith->ServerReforgeAffix(
			RareItem, 0, 2000, 5, EPAForgeTier::Tier2_Field, Wallet, ItemGenerator, ReforgeErr);
		TestFalse(TEXT("AC-2: Reforge without sufficient shards must fail"), bReforgeFailShards);
		TestEqual(TEXT("AC-2: Error code is InsufficientSkillShards"), ReforgeErr, EPACraftingError::InsufficientSkillShards);
		TestEqual(TEXT("AC-2: Atomic rollback - Gold unchanged at 5000"), Wallet->GetGold(), 5000LL);
		TestEqual(TEXT("AC-2: Atomic rollback - Shards unchanged at 2"), Wallet->GetAshShards(), 2LL);
		TestEqual(TEXT("AC-2: Affix 0 still unchanged"), RareItem.ActiveAffixes[0].AffixId, OriginalAffixId);

		// 3. Trường hợp đủ cả hai (5,000 Gold và 12 Shards) -> Thành công
		Wallet->AddCurrency(EPACurrencyType::AshShards, 10, CurrErr); // Giờ có 12 Shards

		bool bReforgeSuccess = Blacksmith->ServerReforgeAffix(
			RareItem, 0, 2000, 5, EPAForgeTier::Tier2_Field, Wallet, ItemGenerator, ReforgeErr);
		TestTrue(TEXT("AC-2: Reforge with sufficient dual currency must succeed"), bReforgeSuccess);
		TestEqual(TEXT("AC-2: Wallet deducted 2000 Gold -> 3000 remaining"), Wallet->GetGold(), 3000LL);
		TestEqual(TEXT("AC-2: Wallet deducted 5 Shards -> 7 remaining"), Wallet->GetAshShards(), 7LL);
		TestTrue(TEXT("AC-2: Reforged affix has valid name"), !RareItem.ActiveAffixes[0].AffixId.IsNone());
		TestTrue(TEXT("AC-2: Reforged affix has positive rolled value"), RareItem.ActiveAffixes[0].RolledValue > 0.0f);

		// Kiểm tra không có affix trùng tên trên món đồ sau khi tẩy
		TSet<FName> SeenAffixes;
		for (const FPAAffixInstance& Affix : RareItem.ActiveAffixes)
		{
			TestFalse(FString::Printf(TEXT("AC-2: No duplicate affix [%s]"), *Affix.AffixId.ToString()),
				SeenAffixes.Contains(Affix.AffixId));
			SeenAffixes.Add(Affix.AffixId);
		}
	}

	// =========================================================================
	// AC-3: AddSocket Transaction & Forge Tier Gating with Currency
	// =========================================================================
	{
		UPACurrencyComponent* Wallet = NewObject<UPACurrencyComponent>();
		EPACurrencyTransactionError CurrErr;
		Wallet->AddCurrency(EPACurrencyType::Gold, 25000, CurrErr);
		Wallet->AddCurrency(EPACurrencyType::AshShards, 50, CurrErr);

		FPASavedItemInstance RareItem = ItemGenerator->GenerateItemInstance(
			FName("ShadowDagger"), 25, EPAItemRarity::Rare, EPAForgeTier::Tier1_Outpost);

		// 1. Outpost Forge (Tier 1): Khóa 100% việc đục lỗ, tiền không bị trừ
		EPACraftingError SocketErr = EPACraftingError::None;
		bool bOutpostSocket = Blacksmith->ServerAddSocket(
			RareItem, 1000, 3, EPAForgeTier::Tier1_Outpost, Wallet, SocketErr);
		TestFalse(TEXT("AC-3: Outpost Forge must reject AddSocket"), bOutpostSocket);
		TestEqual(TEXT("AC-3: Error is MaxTierLevelReached"), SocketErr, EPACraftingError::MaxTierLevelReached);
		TestEqual(TEXT("AC-3: Gold untouched at 25000"), Wallet->GetGold(), 25000LL);
		TestEqual(TEXT("AC-3: Shards untouched at 50"), Wallet->GetAshShards(), 50LL);
		TestFalse(TEXT("AC-3: Socket 0 remains locked"), RareItem.SocketSlots[0].bIsUnlocked);

		// 2. Field Forge (Tier 2): Thiếu tiền -> Thất bại và không mở socket
		UPACurrencyComponent* PoorWallet = NewObject<UPACurrencyComponent>();
		PoorWallet->AddCurrency(EPACurrencyType::Gold, 500, CurrErr); // Thiếu (< 1000)
		PoorWallet->AddCurrency(EPACurrencyType::AshShards, 5, CurrErr);

		bool bPoorSocket = Blacksmith->ServerAddSocket(
			RareItem, 1000, 3, EPAForgeTier::Tier2_Field, PoorWallet, SocketErr);
		TestFalse(TEXT("AC-3: AddSocket with insufficient gold must fail"), bPoorSocket);
		TestEqual(TEXT("AC-3: Error code is InsufficientGold"), SocketErr, EPACraftingError::InsufficientGold);
		TestEqual(TEXT("AC-3: Poor wallet gold untouched"), PoorWallet->GetGold(), 500LL);
		TestFalse(TEXT("AC-3: Socket 0 still locked"), RareItem.SocketSlots[0].bIsUnlocked);

		// 3. Field Forge (Tier 2): Đủ tiền -> Mở Socket 1 (1,000 Gold + 3 Shards) trên RareItem
		bool bSocket1Success = Blacksmith->ServerAddSocket(
			RareItem, 1000, 3, EPAForgeTier::Tier2_Field, Wallet, SocketErr);
		TestTrue(TEXT("AC-3: Field Forge AddSocket 1 succeeds on Rare item"), bSocket1Success);
		TestTrue(TEXT("AC-3: RareItem Socket 0 is now unlocked"), RareItem.SocketSlots[0].bIsUnlocked);
		TestEqual(TEXT("AC-3: Wallet deducted 1000 Gold -> 24000 remaining"), Wallet->GetGold(), 24000LL);
		TestEqual(TEXT("AC-3: Wallet deducted 3 Shards -> 47 remaining"), Wallet->GetAshShards(), 47LL);

		// 4. RareItem chỉ có 1 socket capacity -> Thử mở tiếp socket 2 phải bị từ chối MaxSocketsReached
		bool bSocket2Rare = Blacksmith->ServerAddSocket(
			RareItem, 3000, 8, EPAForgeTier::Tier2_Field, Wallet, SocketErr);
		TestFalse(TEXT("AC-3: Rare item cannot exceed 1 socket capacity"), bSocket2Rare);
		TestEqual(TEXT("AC-3: Error is MaxSocketsReached"), SocketErr, EPACraftingError::MaxSocketsReached);
		TestEqual(TEXT("AC-3: Currency remains unchanged"), Wallet->GetGold(), 24000LL);

		// 5. Field Forge (Tier 2): Đồ Epic có 2 socket capacity -> Mở Socket 1 và Socket 2 (3,000 Gold + 8 Shards)
		FPASavedItemInstance EpicItem = ItemGenerator->GenerateItemInstance(
			FName("SteelCuirass"), 30, EPAItemRarity::Epic, EPAForgeTier::Tier2_Field);
		TestEqual(TEXT("AC-3: Epic item has 2 socket capacity"), EpicItem.SocketSlots.Num(), 2);

		bool bEpicSocket1 = Blacksmith->ServerAddSocket(EpicItem, 1000, 3, EPAForgeTier::Tier2_Field, Wallet, SocketErr);
		TestTrue(TEXT("AC-3: Epic Socket 0 unlocked"), bEpicSocket1);
		TestTrue(TEXT("AC-3: Epic Socket 0 is unlocked"), EpicItem.SocketSlots[0].bIsUnlocked);

		bool bEpicSocket2 = Blacksmith->ServerAddSocket(EpicItem, 3000, 8, EPAForgeTier::Tier2_Field, Wallet, SocketErr);
		TestTrue(TEXT("AC-3: Epic Socket 1 unlocked"), bEpicSocket2);
		TestTrue(TEXT("AC-3: Epic Socket 1 is unlocked"), EpicItem.SocketSlots[1].bIsUnlocked);
		TestEqual(TEXT("AC-3: Wallet deducted 4000 Gold total -> 20000 remaining"), Wallet->GetGold(), 20000LL);
		TestEqual(TEXT("AC-3: Wallet deducted 11 Shards total -> 36 remaining"), Wallet->GetAshShards(), 36LL);

		// 6. Forbidden Forge (Tier 3): Đồ Legendary mở Socket 3 Prismatic (15,000 Gold + 20 Shards)
		FPASavedItemInstance LegendaryItem = ItemGenerator->GenerateItemInstance(
			FName("DragonSlayer"), 45, EPAItemRarity::Legendary, EPAForgeTier::Tier3_Forbidden);
		TestEqual(TEXT("AC-3: Legendary has 3 socket capacity"), LegendaryItem.SocketSlots.Num(), 3);

		// Mở 2 socket đầu
		Blacksmith->ServerAddSocket(LegendaryItem, 1000, 3, EPAForgeTier::Tier2_Field, Wallet, SocketErr);
		Blacksmith->ServerAddSocket(LegendaryItem, 3000, 8, EPAForgeTier::Tier2_Field, Wallet, SocketErr);

		// Mở Socket Prismatic thứ 3 tại Forbidden Forge
		const int64 GoldBeforePrismatic = Wallet->GetGold();
		const int64 ShardsBeforePrismatic = Wallet->GetAshShards();

		bool bPrismaticSuccess = Blacksmith->ServerAddSocket(
			LegendaryItem, 15000, 20, EPAForgeTier::Tier3_Forbidden, Wallet, SocketErr);
		TestTrue(TEXT("AC-3: Forbidden Forge unlocks Prismatic socket on Legendary"), bPrismaticSuccess);
		TestTrue(TEXT("AC-3: Socket 2 (Prismatic) is unlocked"), LegendaryItem.SocketSlots[2].bIsUnlocked);
		TestEqual(TEXT("AC-3: Socket 2 type is Prismatic"), LegendaryItem.SocketSlots[2].SocketType, EPASocketType::Prismatic);
		TestEqual(TEXT("AC-3: Deducted exactly 15000 Gold"), Wallet->GetGold(), GoldBeforePrismatic - 15000LL);
		TestEqual(TEXT("AC-3: Deducted exactly 20 Shards"), Wallet->GetAshShards(), ShardsBeforePrismatic - 20LL);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
