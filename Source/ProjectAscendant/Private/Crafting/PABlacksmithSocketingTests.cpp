// Copyright Project Ascendant. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Itemization/PAAffixTypes.h"
#include "Itemization/PASavedItemInstance.h"
#include "Itemization/PAServerItemGeneratorSubsystem.h"
#include "Crafting/PABlacksmithTypes.h"
#include "Crafting/PABlacksmithSubsystem.h"
#include "Combat/AscendantAttributeSet.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPABlacksmithSocketingTest,
	"ProjectAscendant.Itemization.BlacksmithSocketing",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPABlacksmithSocketingTest::RunTest(const FString& Parameters)
{
	// 1. Khởi tạo GameInstance và Subsystems
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
	// AC-1: Outpost Forge (Tier 1) - Khóa 100% việc đục lỗ
	// CanAddSocket luôn trả về false, mọi socket giữ nguyên bIsUnlocked = false.
	// =========================================================================
	{
		FPASavedItemInstance RareItem = ItemGenerator->GenerateItemInstance(
			FName("IronSword"), 15, EPAItemRarity::Rare, EPAForgeTier::Tier1_Outpost);
		FPASavedItemInstance EpicItem = ItemGenerator->GenerateItemInstance(
			FName("SteelSword"), 20, EPAItemRarity::Epic, EPAForgeTier::Tier1_Outpost);
		FPASavedItemInstance LegendaryItem = ItemGenerator->GenerateItemInstance(
			FName("DragonSword"), 20, EPAItemRarity::Legendary, EPAForgeTier::Tier1_Outpost);

		// Kiểm tra Rare tại Outpost Forge
		EPACraftingError Error = EPACraftingError::None;
		int32 TargetIndex = INDEX_NONE;
		bool bCanRare = Blacksmith->CanAddSocket(RareItem, EPAForgeTier::Tier1_Outpost, TargetIndex, Error);
		TestFalse(TEXT("Outpost Forge: CanAddSocket on Rare item must return false"), bCanRare);
		TestEqual(TEXT("Outpost Forge error must be MaxTierLevelReached"), Error, EPACraftingError::MaxTierLevelReached);

		bool bUnlockedRare = Blacksmith->UnlockNextSocket(RareItem, EPAForgeTier::Tier1_Outpost, Error);
		TestFalse(TEXT("Outpost Forge: UnlockNextSocket on Rare must fail"), bUnlockedRare);
		TestEqual(TEXT("Rare item socket 0 must remain bIsUnlocked = false"), RareItem.SocketSlots[0].bIsUnlocked, false);

		// Kiểm tra Epic tại Outpost Forge
		bool bCanEpic = Blacksmith->CanAddSocket(EpicItem, EPAForgeTier::Tier1_Outpost, Error);
		TestFalse(TEXT("Outpost Forge: CanAddSocket on Epic item must return false"), bCanEpic);
		TestFalse(TEXT("Epic item socket 0 must remain bIsUnlocked = false"), EpicItem.SocketSlots[0].bIsUnlocked);
		TestFalse(TEXT("Epic item socket 1 must remain bIsUnlocked = false"), EpicItem.SocketSlots[1].bIsUnlocked);

		// Kiểm tra Legendary tại Outpost Forge
		bool bCanLegendary = Blacksmith->CanAddSocket(LegendaryItem, EPAForgeTier::Tier1_Outpost, Error);
		TestFalse(TEXT("Outpost Forge: CanAddSocket on Legendary item must return false"), bCanLegendary);
		for (int32 i = 0; i < LegendaryItem.SocketSlots.Num(); ++i)
		{
			TestFalse(
				FString::Printf(TEXT("Legendary socket %d must remain bIsUnlocked = false"), i),
				LegendaryItem.SocketSlots[i].bIsUnlocked);
		}
	}

	// =========================================================================
	// AC-2: Field Forge (Tier 2) - Mở tối đa 2 socket Regular trên đồ Rare/Epic
	// Từ chối nếu vượt quá 2 hoặc item không đủ Regular socket slot.
	// =========================================================================
	{
		// 1. Đồ Rare: Có đúng 1 Regular slot
		FPASavedItemInstance RareItem = ItemGenerator->GenerateItemInstance(
			FName("RareBlade"), 25, EPAItemRarity::Rare, EPAForgeTier::Tier2_Field);
		TestEqual(TEXT("Rare item has 1 socket slot"), RareItem.SocketSlots.Num(), 1);
		TestFalse(TEXT("Rare socket starts locked"), RareItem.SocketSlots[0].bIsUnlocked);

		EPACraftingError Error = EPACraftingError::None;
		int32 TargetIndex = INDEX_NONE;

		// Lần 1: Cho phép mở
		bool bCanRare1 = Blacksmith->CanAddSocket(RareItem, EPAForgeTier::Tier2_Field, TargetIndex, Error);
		TestTrue(TEXT("Field Forge: Rare socket 0 can be added"), bCanRare1);
		TestEqual(TEXT("Target socket index must be 0"), TargetIndex, 0);

		bool bUnlockedRare1 = Blacksmith->UnlockNextSocket(RareItem, EPAForgeTier::Tier2_Field, Error);
		TestTrue(TEXT("Field Forge: Rare socket 0 unlocked successfully"), bUnlockedRare1);
		TestTrue(TEXT("Rare socket 0 is now bIsUnlocked = true"), RareItem.SocketSlots[0].bIsUnlocked);
		TestEqual(TEXT("Unlocked regular count on Rare is 1"), UPABlacksmithSubsystem::GetUnlockedRegularSocketCount(RareItem), 1);

		// Lần 2: Từ chối vì đồ Rare chỉ có 1 slot (không đủ slot để mở tiếp)
		bool bCanRare2 = Blacksmith->CanAddSocket(RareItem, EPAForgeTier::Tier2_Field, TargetIndex, Error);
		TestFalse(TEXT("Field Forge: Rare item cannot add second socket (slot limit reached)"), bCanRare2);
		TestEqual(TEXT("Error must be MaxSocketsReached"), Error, EPACraftingError::MaxSocketsReached);

		// 2. Đồ Epic: Có đúng 2 Regular slots
		FPASavedItemInstance EpicItem = ItemGenerator->GenerateItemInstance(
			FName("EpicStaff"), 30, EPAItemRarity::Epic, EPAForgeTier::Tier2_Field);
		TestEqual(TEXT("Epic item has 2 socket slots"), EpicItem.SocketSlots.Num(), 2);

		// Lần 1: Mở slot 0
		bool bCanEpic1 = Blacksmith->CanAddSocket(EpicItem, EPAForgeTier::Tier2_Field, TargetIndex, Error);
		TestTrue(TEXT("Field Forge: Epic socket 0 can be added"), bCanEpic1);
		TestEqual(TEXT("Target socket index must be 0"), TargetIndex, 0);
		TestTrue(TEXT("Unlock socket 0"), Blacksmith->UnlockNextSocket(EpicItem, EPAForgeTier::Tier2_Field, Error));
		TestTrue(TEXT("Epic slot 0 is unlocked"), EpicItem.SocketSlots[0].bIsUnlocked);
		TestEqual(TEXT("Unlocked regular count on Epic is 1"), UPABlacksmithSubsystem::GetUnlockedRegularSocketCount(EpicItem), 1);

		// Lần 2: Mở slot 1
		bool bCanEpic2 = Blacksmith->CanAddSocket(EpicItem, EPAForgeTier::Tier2_Field, TargetIndex, Error);
		TestTrue(TEXT("Field Forge: Epic socket 1 can be added"), bCanEpic2);
		TestEqual(TEXT("Target socket index must be 1"), TargetIndex, 1);
		TestTrue(TEXT("Unlock socket 1"), Blacksmith->UnlockNextSocket(EpicItem, EPAForgeTier::Tier2_Field, Error));
		TestTrue(TEXT("Epic slot 1 is unlocked"), EpicItem.SocketSlots[1].bIsUnlocked);
		TestEqual(TEXT("Unlocked regular count on Epic is 2"), UPABlacksmithSubsystem::GetUnlockedRegularSocketCount(EpicItem), 2);

		// Lần 3: Từ chối vì đã đạt trần tối đa 2 socket Regular
		bool bCanEpic3 = Blacksmith->CanAddSocket(EpicItem, EPAForgeTier::Tier2_Field, TargetIndex, Error);
		TestFalse(TEXT("Field Forge: Epic item cannot exceed 2 Regular sockets"), bCanEpic3);
		TestEqual(TEXT("Error must be MaxSocketsReached"), Error, EPACraftingError::MaxSocketsReached);

		// 3. Đồ Common / Uncommon: 0 slots -> Không thể đục lỗ
		FPASavedItemInstance CommonItem = ItemGenerator->GenerateItemInstance(
			FName("CommonDagger"), 25, EPAItemRarity::Common, EPAForgeTier::Tier2_Field);
		bool bCanCommon = Blacksmith->CanAddSocket(CommonItem, EPAForgeTier::Tier2_Field, Error);
		TestFalse(TEXT("Field Forge: Common item cannot be socketed"), bCanCommon);
		TestEqual(TEXT("Error must be ItemCannotBeSocketed"), Error, EPACraftingError::ItemCannotBeSocketed);

		// 4. Đồ Legendary tại Field Forge: Cho phép mở tối đa 2 Regular, nhưng KHÔNG được mở Prismatic
		FPASavedItemInstance LegendaryItem = ItemGenerator->GenerateItemInstance(
			FName("LegAxe"), 35, EPAItemRarity::Legendary, EPAForgeTier::Tier2_Field);
		TestEqual(TEXT("Legendary has 3 slots"), LegendaryItem.SocketSlots.Num(), 3);

		// Mở 2 slot Regular thành công
		TestTrue(TEXT("Field Forge: Legendary socket 0 (Regular) unlocked"), Blacksmith->UnlockNextSocket(LegendaryItem, EPAForgeTier::Tier2_Field, Error));
		TestTrue(TEXT("Field Forge: Legendary socket 1 (Regular) unlocked"), Blacksmith->UnlockNextSocket(LegendaryItem, EPAForgeTier::Tier2_Field, Error));
		TestEqual(TEXT("Regular sockets unlocked = 2"), UPABlacksmithSubsystem::GetUnlockedRegularSocketCount(LegendaryItem), 2);

		// Slot thứ 3 là Prismatic -> Field Forge PHẢI TỪ CHỐI
		TestEqual(TEXT("Slot 2 is Prismatic"), LegendaryItem.SocketSlots[2].SocketType, EPASocketType::Prismatic);
		bool bCanLegPrismaticAtField = Blacksmith->CanAddSocket(LegendaryItem, EPAForgeTier::Tier2_Field, TargetIndex, Error);
		TestFalse(TEXT("Field Forge CANNOT unlock Prismatic socket"), bCanLegPrismaticAtField);
		TestEqual(TEXT("Error must be MaxTierLevelReached"), Error, EPACraftingError::MaxTierLevelReached);
		TestFalse(TEXT("Legendary Prismatic slot remains locked"), LegendaryItem.SocketSlots[2].bIsUnlocked);
	}

	// =========================================================================
	// AC-3: Forbidden Forge (Tier 3) - Mở khóa Prismatic (Độc quyền Legendary)
	// =========================================================================
	{
		// Tiếp tục với LegendaryItem có 2 Regular đã mở và slot 2 Prismatic đang khóa
		FPASavedItemInstance LegendaryItem = ItemGenerator->GenerateItemInstance(
			FName("ForbiddenBlade"), 50, EPAItemRarity::Legendary, EPAForgeTier::Tier3_Forbidden);

		// Mở 2 socket Regular đầu tiên tại Forbidden Forge
		EPACraftingError Error = EPACraftingError::None;
		TestTrue(TEXT("Forbidden Forge: Regular 0 unlocked"), Blacksmith->UnlockNextSocket(LegendaryItem, EPAForgeTier::Tier3_Forbidden, Error));
		TestTrue(TEXT("Forbidden Forge: Regular 1 unlocked"), Blacksmith->UnlockNextSocket(LegendaryItem, EPAForgeTier::Tier3_Forbidden, Error));

		// Giờ đây slot tiếp theo là Prismatic:
		int32 TargetIndex = INDEX_NONE;
		bool bCanPrismatic = Blacksmith->CanAddSocket(LegendaryItem, EPAForgeTier::Tier3_Forbidden, TargetIndex, Error);
		TestTrue(TEXT("Forbidden Forge: CAN unlock Prismatic socket on Legendary"), bCanPrismatic);
		TestEqual(TEXT("TargetIndex must be 2"), TargetIndex, 2);

		bool bUnlockedPrismatic = Blacksmith->UnlockNextSocket(LegendaryItem, EPAForgeTier::Tier3_Forbidden, Error);
		TestTrue(TEXT("Forbidden Forge: Prismatic socket unlocked successfully"), bUnlockedPrismatic);
		TestTrue(TEXT("Legendary slot 2 is now unlocked"), LegendaryItem.SocketSlots[2].bIsUnlocked);
		TestEqual(TEXT("Unlocked Prismatic count is 1"), UPABlacksmithSubsystem::GetUnlockedPrismaticSocketCount(LegendaryItem), 1);
		TestEqual(TEXT("Total unlocked sockets on Legendary is 3"), UPABlacksmithSubsystem::GetTotalUnlockedSocketCount(LegendaryItem), 3);

		// Lần 4: Cả 3 slot đã mở -> Từ chối
		bool bCanMore = Blacksmith->CanAddSocket(LegendaryItem, EPAForgeTier::Tier3_Forbidden, Error);
		TestFalse(TEXT("Legendary cannot add any more sockets (all 3 unlocked)"), bCanMore);
		TestEqual(TEXT("Error must be MaxSocketsReached"), Error, EPACraftingError::MaxSocketsReached);

		// Kiểm tra tính độc quyền Legendary của Prismatic:
		// Một item giả định Epic có ô Prismatic sẽ bị từ chối
		FPASavedItemInstance FakeEpicItem;
		FakeEpicItem.ItemDefId = FName("FakeEpic");
		FakeEpicItem.RarityTag = FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Epic"), false);
		FakeEpicItem.SocketSlots.Add(FPASocketSlot(EPASocketType::Prismatic, false));

		bool bCanFakeEpicPrismatic = Blacksmith->CanAddSocket(FakeEpicItem, EPAForgeTier::Tier3_Forbidden, Error);
		TestFalse(TEXT("Prismatic socket is EXCLUSIVE to Legendary (Epic must be rejected)"), bCanFakeEpicPrismatic);
		TestEqual(TEXT("Error must be ItemCannotBeSocketed"), Error, EPACraftingError::ItemCannotBeSocketed);
	}

	// =========================================================================
	// AC-4: SocketGem & AttributeSet Integration (Khảm ngọc & cộng chỉ số GAS)
	// =========================================================================
	{
		UAscendantAttributeSet* AttributeSet = NewObject<UAscendantAttributeSet>();
		TestNotNull(TEXT("AttributeSet must be valid"), AttributeSet);
		if (!AttributeSet)
		{
			return false;
		}

		AttributeSet->InitMaxPosture(100.0f);
		AttributeSet->InitPosture(0.0f);
		AttributeSet->InitMaxMana(100.0f);
		AttributeSet->InitMana(100.0f);
		AttributeSet->InitMaxStamina(100.0f);
		AttributeSet->InitStamina(100.0f);
		AttributeSet->InitAttackPower(10.0f);

		EPACraftingError Error = EPACraftingError::None;

		// 1. Thử khảm ngọc vào socket CHƯA mở khóa -> Phải thất bại
		FPASavedItemInstance RareItem = ItemGenerator->GenerateItemInstance(
			FName("LockedRareSword"), 25, EPAItemRarity::Rare, EPAForgeTier::Tier2_Field);
		TestFalse(TEXT("Rare slot 0 is locked initially"), RareItem.SocketSlots[0].bIsUnlocked);

		bool bSocketLocked = Blacksmith->SocketGem(RareItem, 0, FName("Ruby"), AttributeSet, Error);
		TestFalse(TEXT("SocketGem on locked slot must fail"), bSocketLocked);
		TestEqual(TEXT("Error on locked slot must be ItemCannotBeSocketed"), Error, EPACraftingError::ItemCannotBeSocketed);
		TestEqual(TEXT("MaxPosture remains 100.0f after failed socket"), AttributeSet->GetMaxPosture(), 100.0f);

		// 2. Mở khóa slot 0 và khảm Ruby (+20 MaxPosture)
		TestTrue(TEXT("Unlock Rare socket 0 at Field Forge"), Blacksmith->UnlockNextSocket(RareItem, EPAForgeTier::Tier2_Field, Error));
		bool bSocketRuby = Blacksmith->SocketGem(RareItem, 0, FName("Ruby"), AttributeSet, Error);
		TestTrue(TEXT("SocketGem Ruby on unlocked slot succeeds"), bSocketRuby);
		TestEqual(TEXT("SocketedGemId is Ruby"), RareItem.SocketSlots[0].SocketedGemId, FName("Ruby"));
		TestEqual(TEXT("MaxPosture increases by +20 (from 100 to 120)"), AttributeSet->GetMaxPosture(), 120.0f);

		// 3. Thử khảm đè lên slot đã có ngọc -> Phải thất bại
		bool bSocketDuplicate = Blacksmith->SocketGem(RareItem, 0, FName("Ruby"), AttributeSet, Error);
		TestFalse(TEXT("SocketGem on already occupied slot must fail"), bSocketDuplicate);
		TestEqual(TEXT("Error on occupied slot must be SocketNotEmpty"), Error, EPACraftingError::SocketNotEmpty);
		TestEqual(TEXT("MaxPosture still 120.0f"), AttributeSet->GetMaxPosture(), 120.0f);

		// 4. Khảm Sapphire (+25 MaxMana) và Topaz (+15 MaxStamina) trên EpicItem
		FPASavedItemInstance EpicItem = ItemGenerator->GenerateItemInstance(
			FName("EpicArmor"), 30, EPAItemRarity::Epic, EPAForgeTier::Tier2_Field);
		TestTrue(TEXT("Unlock Epic slot 0"), Blacksmith->UnlockNextSocket(EpicItem, EPAForgeTier::Tier2_Field, Error));
		TestTrue(TEXT("Unlock Epic slot 1"), Blacksmith->UnlockNextSocket(EpicItem, EPAForgeTier::Tier2_Field, Error));

		bool bSocketSapphire = Blacksmith->SocketGem(EpicItem, 0, FName("Sapphire"), AttributeSet, Error);
		TestTrue(TEXT("SocketGem Sapphire succeeds"), bSocketSapphire);
		TestEqual(TEXT("MaxMana increases by +25 (from 100 to 125)"), AttributeSet->GetMaxMana(), 125.0f);
		TestEqual(TEXT("Mana increases by +25 (from 100 to 125)"), AttributeSet->GetMana(), 125.0f);

		bool bSocketTopaz = Blacksmith->SocketGem(EpicItem, 1, FName("Topaz"), AttributeSet, Error);
		TestTrue(TEXT("SocketGem Topaz succeeds"), bSocketTopaz);
		TestEqual(TEXT("MaxStamina increases by +15 (from 100 to 115)"), AttributeSet->GetMaxStamina(), 115.0f);
		TestEqual(TEXT("Stamina increases by +15 (from 100 to 115)"), AttributeSet->GetStamina(), 115.0f);

		// 5. Thử khảm Prismatic Gem vào Regular slot -> Phải thất bại
		bool bPrismaticOnRegular = Blacksmith->SocketGem(EpicItem, 1, FName("Prismatic_Gem"), AttributeSet, Error);
		TestFalse(TEXT("Prismatic gem cannot be socketed in Regular slot"), bPrismaticOnRegular);

		// 6. Khảm Prismatic Gem (+10 AttackPower) vào Prismatic slot trên LegendaryItem
		FPASavedItemInstance LegItem = ItemGenerator->GenerateItemInstance(
			FName("LegendaryHammer"), 50, EPAItemRarity::Legendary, EPAForgeTier::Tier3_Forbidden);
		TestTrue(TEXT("Unlock Leg slot 0"), Blacksmith->UnlockNextSocket(LegItem, EPAForgeTier::Tier3_Forbidden, Error));
		TestTrue(TEXT("Unlock Leg slot 1"), Blacksmith->UnlockNextSocket(LegItem, EPAForgeTier::Tier3_Forbidden, Error));
		TestTrue(TEXT("Unlock Leg slot 2 (Prismatic)"), Blacksmith->UnlockNextSocket(LegItem, EPAForgeTier::Tier3_Forbidden, Error));

		bool bSocketPrismatic = Blacksmith->SocketGem(LegItem, 2, FName("Prismatic_Gem"), AttributeSet, Error);
		TestTrue(TEXT("SocketGem Prismatic on Prismatic slot succeeds"), bSocketPrismatic);
		TestEqual(TEXT("AttackPower increases by +10 (from 10 to 20)"), AttributeSet->GetAttackPower(), 20.0f);

		// 7. Tháo ngọc (UnsocketGem) và thu hồi chỉ số khỏi AttributeSet
		bool bUnsocketRuby = Blacksmith->UnsocketGem(RareItem, 0, AttributeSet, Error);
		TestTrue(TEXT("UnsocketGem Ruby succeeds"), bUnsocketRuby);
		TestFalse(TEXT("Rare slot 0 no longer has gem"), RareItem.SocketSlots[0].HasGem());
		TestEqual(TEXT("MaxPosture reverts to 100.0f after unsocket"), AttributeSet->GetMaxPosture(), 100.0f);

		// 8. Kiểm tra SocketGemByUID qua hành trang
		TArray<FPASavedItemInstance> Inventory;
		FPASavedItemInstance InvItem = ItemGenerator->GenerateItemInstance(
			FName("InvSword"), 20, EPAItemRarity::Rare, EPAForgeTier::Tier2_Field);
		TestTrue(TEXT("Unlock InvItem socket 0"), Blacksmith->UnlockNextSocket(InvItem, EPAForgeTier::Tier2_Field, Error));
		Inventory.Add(InvItem);

		bool bSocketUID = Blacksmith->SocketGemByUID(
			Inventory, InvItem.ItemInstanceUID, 0, FName("Ruby"), AttributeSet, Error);
		TestTrue(TEXT("SocketGemByUID succeeds"), bSocketUID);
		TestEqual(TEXT("MaxPosture increased back to 120.0f via UID socketing"), AttributeSet->GetMaxPosture(), 120.0f);
		TestEqual(TEXT("Inventory item slot 0 has Ruby"), Inventory[0].SocketSlots[0].SocketedGemId, FName("Ruby"));

		// Gọi SocketGemByUID với UID ngẫu nhiên không tồn tại -> Phải trả về ItemNotFound
		FGuid FakeUID = FGuid::NewGuid();
		bool bSocketFakeUID = Blacksmith->SocketGemByUID(
			Inventory, FakeUID, 0, FName("Ruby"), AttributeSet, Error);
		TestFalse(TEXT("SocketGemByUID with fake UID fails"), bSocketFakeUID);
		TestEqual(TEXT("Error must be ItemNotFound"), Error, EPACraftingError::ItemNotFound);
	}

	return true;
}
