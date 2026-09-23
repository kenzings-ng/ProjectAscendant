// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAInventoryTransactionTest
 *
 * Kiểm thử tự động đơn vị cho Giao dịch Ô Đồ Nguyên Tử và Chống Nhân Bản Vật Phẩm (Story 002 / inv-002).
 *
 * Tiêu chí nghiệm thu:
 *  - AC-1 (Atomic Move & Swap Validation): Di chuyển / Hoán đổi nguyên tử, chống nhân bản (Anti-duping) khi spam lệnh.
 *  - AC-2 (Split Stack Validation): Tách chồng đồ, kiểm tra ô đích rỗng và số lượng tách hợp lệ.
 *  - AC-3 (QoL Item Lock & Junk Flags): Khóa vật phẩm (bIsLocked) ngăn chặn vứt hoặc bán nhầm; Đánh dấu rác (bIsJunk).
 *  - AC-4 (Overflow Stash Routing): Định tuyến vật phẩm quý (Tier Rare trở lên) vào Hòm Đệm 20 ô khi ba lô đầy.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAInventoryTransactionTest,
	"ProjectAscendant.Foundation.Inventory.TransactionalRPCsAndSafeguards",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAInventoryTransactionTest::RunTest(const FString& Parameters)
{
	// -------------------------------------------------------------------------
	// Test 1: AC-1 (Di Chuyển, Hoán Đổi Nguyên Tử & Chống Nhân Bản Vật Phẩm)
	// -------------------------------------------------------------------------
	{
		UPAInventoryComponent* InventoryComp = NewObject<UPAInventoryComponent>();

		UItemStaticDataAsset* SwordAsset = NewObject<UItemStaticDataAsset>();
		SwordAsset->ItemId = FName(TEXT("Item_Sword_Excalibur"));
		SwordAsset->Category = EPAItemCategory::Equipment;
		SwordAsset->MaxStackSize = 1;

		// Đặt thanh kiếm vào Ô 0
		InventoryComp->AddItemToSlot(0, SwordAsset, 1);
		const FGuid SwordUID = InventoryComp->GetItemAtSlot(0)->ItemInstanceUID;
		TestTrue(TEXT("AC-1: Thanh kiếm có UID hợp lệ"), SwordUID.IsValid());

		// Lần gọi 1: Di chuyển từ Ô 0 sang Ô 1
		const bool bMove1 = InventoryComp->MoveItem(0, 1, SwordUID);
		TestTrue(TEXT("AC-1: Di chuyển lần 1 thành công"), bMove1);
		TestNull(TEXT("AC-1: Ô 0 giờ đã trống"), InventoryComp->GetItemAtSlot(0));
		TestNotNull(TEXT("AC-1: Ô 1 chứa thanh kiếm"), InventoryComp->GetItemAtSlot(1));

		// Lần gọi 2 (Mô phỏng hacker spam gói tin packet duplicate cùng UID từ Ô 0)
		const bool bMoveDuplicate = InventoryComp->MoveItem(0, 1, SwordUID);
		TestFalse(TEXT("AC-1: Lần di chuyển thứ 2 bị Server từ chối vì Ô 0 không còn vật phẩm"), bMoveDuplicate);
		TestEqual(TEXT("AC-1: Tổng số thanh kiếm trên toàn túi đồ vẫn duy nhất là 1"),
			InventoryComp->GetItemCount(FName(TEXT("Item_Sword_Excalibur"))), 1);

		// Thử nghiệm Hoán đổi (Atomic Swap) giữa Ô 1 (Kiếm) và Ô 2 (Khiên)
		UItemStaticDataAsset* ShieldAsset = NewObject<UItemStaticDataAsset>();
		ShieldAsset->ItemId = FName(TEXT("Item_Shield_Aegis"));
		ShieldAsset->Category = EPAItemCategory::Equipment;
		ShieldAsset->MaxStackSize = 1;

		InventoryComp->AddItemToSlot(2, ShieldAsset, 1);

		const bool bSwapSuccess = InventoryComp->MoveItem(1, 2);
		TestTrue(TEXT("AC-1: Hoán đổi nguyên tử giữa 2 ô thành công"), bSwapSuccess);
		TestEqual(TEXT("AC-1: Ô 1 giờ chứa Khiên"), InventoryComp->GetItemAtSlot(1)->ItemDefId, FName(TEXT("Item_Shield_Aegis")));
		TestEqual(TEXT("AC-1: Ô 2 giờ chứa Kiếm"), InventoryComp->GetItemAtSlot(2)->ItemDefId, FName(TEXT("Item_Sword_Excalibur")));
	}

	// -------------------------------------------------------------------------
	// Test 2: AC-2 (Tách Chồng Đồ Hợp Lệ & Thất Bại Khi Ô Đích Đã Có Đồ)
	// -------------------------------------------------------------------------
	{
		UPAInventoryComponent* InventoryComp = NewObject<UPAInventoryComponent>();

		UItemStaticDataAsset* PotionAsset = NewObject<UItemStaticDataAsset>();
		PotionAsset->ItemId = FName(TEXT("Item_Potion_Mana"));
		PotionAsset->Category = EPAItemCategory::Consumable;
		PotionAsset->MaxStackSize = 20;

		// Đặt 20 bình mana vào Ô 0
		InventoryComp->AddItemToSlot(0, PotionAsset, 20);

		// Tách 5 bình sang Ô 1 (đang trống)
		const bool bSplitSuccess = InventoryComp->SplitStack(0, 5, 1);
		TestTrue(TEXT("AC-2: Tách 5 bình sang ô trống thành công"), bSplitSuccess);
		TestEqual(TEXT("AC-2: Ô 0 còn lại 15 bình"), InventoryComp->GetItemAtSlot(0)->StackCount, 15);
		TestEqual(TEXT("AC-2: Ô 1 chứa đúng 5 bình được tách"), InventoryComp->GetItemAtSlot(1)->StackCount, 5);

		// Thử tách sang Ô 1 một lần nữa (Ô 1 giờ không còn trống) -> Phải thất bại
		const bool bSplitToOccupied = InventoryComp->SplitStack(0, 5, 1);
		TestFalse(TEXT("AC-2: Không thể tách chồng vào ô đã có vật phẩm"), bSplitToOccupied);

		// Thử tách số lượng >= số lượng hiện có (tách 15 / 15 bình) -> Phải thất bại
		const bool bSplitAll = InventoryComp->SplitStack(0, 15, 2);
		TestFalse(TEXT("AC-2: Tách toàn bộ hoặc vượt số lượng bị từ chối"), bSplitAll);
	}

	// -------------------------------------------------------------------------
	// Test 3: AC-3 (Khóa Vật Phẩm bIsLocked & Cờ Rác bIsJunk)
	// -------------------------------------------------------------------------
	{
		UPAInventoryComponent* InventoryComp = NewObject<UPAInventoryComponent>();

		UItemStaticDataAsset* RareArmor = NewObject<UItemStaticDataAsset>();
		RareArmor->ItemId = FName(TEXT("Item_Armor_ShadowMantle"));
		RareArmor->Category = EPAItemCategory::Equipment;

		InventoryComp->AddItemToSlot(3, RareArmor, 1);
		TestTrue(TEXT("AC-3: Mặc định đồ không bị khóa, có thể bán"), InventoryComp->CanDropOrSellItem(3));

		// Bật khóa vật phẩm
		InventoryComp->SetItemLocked(3, true);
		TestFalse(TEXT("AC-3: Vật phẩm bị khóa tuyệt đối KHÔNG được phép bán hoặc vứt"), InventoryComp->CanDropOrSellItem(3));
		TestFalse(TEXT("AC-3: Vật phẩm đang bị khóa tuyệt đối KHÔNG được di chuyển hoặc hoán đổi"), InventoryComp->MoveItem(3, 4));

		// Đánh dấu rác (bIsJunk)
		const bool bJunkSet = InventoryComp->ToggleItemJunk(3);
		TestTrue(TEXT("AC-3: Đảo cờ rác thành công"), bJunkSet);
		TestTrue(TEXT("AC-3: Vật phẩm có cờ bIsJunk = true"), InventoryComp->GetItemAtSlot(3)->DynamicData.bIsJunk);

		// Mở khóa lại
		InventoryComp->SetItemLocked(3, false);
		TestTrue(TEXT("AC-3: Sau khi mở khóa, vật phẩm được phép giao dịch bình thường"), InventoryComp->CanDropOrSellItem(3));
		TestTrue(TEXT("AC-3: Sau khi mở khóa, vật phẩm có thể di chuyển bình thường"), InventoryComp->MoveItem(3, 4));
	}

	// -------------------------------------------------------------------------
	// Test 4: AC-4 (Định Tuyến Hòm Đệm Lửa Trại Khi Ba Lô Đầy)
	// -------------------------------------------------------------------------
	{
		UPAInventoryComponent* InventoryComp = NewObject<UPAInventoryComponent>();

		UItemStaticDataAsset* BossRing = NewObject<UItemStaticDataAsset>();
		BossRing->ItemId = FName(TEXT("Item_Ring_DemonLord"));
		BossRing->RarityTier = EPAItemRarity::Rare; // Tier Rare

		// 4.0: Túi đồ còn chỗ trống -> Không được chuyển vào Hòm Đệm
		TestFalse(TEXT("AC-4: Túi đồ còn chỗ trống thì không chuyển vào Hòm Đệm"), InventoryComp->RouteToOverflowStash(BossRing, 1));

		// Lấp đầy toàn bộ 30 ô túi đồ (Base slots = 30)
		UItemStaticDataAsset* FillerItem = NewObject<UItemStaticDataAsset>();
		FillerItem->ItemId = FName(TEXT("Item_Material_Stone"));
		FillerItem->Category = EPAItemCategory::Equipment;
		FillerItem->MaxStackSize = 1;
		for (int32 i = 0; i < 30; ++i)
		{
			InventoryComp->AddItemToSlot(i, FillerItem, 1);
		}
		TestEqual(TEXT("AC-4: Toàn bộ 30 ô đã bị chiếm dụng"), InventoryComp->FindFirstEmptySlot(), INDEX_NONE);

		// 4.1: Vật phẩm Tier Common rơi khi túi đầy -> Bị từ chối
		UItemStaticDataAsset* JunkItem = NewObject<UItemStaticDataAsset>();
		JunkItem->ItemId = FName(TEXT("Item_Junk_BrokenBone"));
		JunkItem->RarityTier = EPAItemRarity::Common;

		const bool bJunkRouted = InventoryComp->RouteToOverflowStash(JunkItem, 1);
		TestFalse(TEXT("AC-4: Đồ Common không được đưa vào Hòm Đệm khi túi đầy"), bJunkRouted);
		TestEqual(TEXT("AC-4: Hòm đệm vẫn rỗng"), InventoryComp->GetOverflowStashCount(), 0);

		// 4.2: Boss rơi nhẫn quý Tier Rare (3) khi túi đầy -> Chuyển vào Hòm Đệm
		const bool bRingRouted = InventoryComp->RouteToOverflowStash(BossRing, 1);
		TestTrue(TEXT("AC-4: Nhẫn quý của Boss được chuyển an toàn vào Hòm Đệm"), bRingRouted);
		TestEqual(TEXT("AC-4: Hòm đệm có 1 vật phẩm"), InventoryComp->GetOverflowStashCount(), 1);

		// 4.3: Rút đồ từ Hòm Đệm khi túi vẫn đầy -> Thất bại
		TestFalse(TEXT("AC-4: Rút đồ khi ba lô đầy phải thất bại"), InventoryComp->ClaimOverflowStashItem(0));

		// Dọn trống Ô 0 trong ba lô
		InventoryComp->RemoveItemFromSlot(0, 1);
		TestEqual(TEXT("AC-4: Ô 0 đã được dọn trống"), InventoryComp->FindFirstEmptySlot(), 0);

		const bool bClaimed = InventoryComp->ClaimOverflowStashItem(0);
		TestTrue(TEXT("AC-4: Rút đồ từ Hòm Đệm vào ba lô thành công khi đã có ô trống"), bClaimed);
		TestEqual(TEXT("AC-4: Hòm đệm dọn sạch sau khi nhận"), InventoryComp->GetOverflowStashCount(), 0);
		TestNotNull(TEXT("AC-4: Nhẫn quý đã nằm an toàn trong ô 0 ba lô người chơi"), InventoryComp->GetItemAtSlot(0));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
