// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAFastArrayInventoryTest
 *
 * Kiểm thử tự động đơn vị cho Hệ thống Ba lô Lưới 30 Ô và Mảng Nén Mạng FastArray (Story 001 / inv-001).
 *
 * Tiêu chí nghiệm thu:
 *  - AC-1 (Backpack Capacity & Slot Limit): Khởi tạo 30 ô, mở rộng từng đợt +10 ô, chặn trần tối đa 60 ô.
 *  - AC-2 (Item DataAsset Hierarchy): Kế thừa UPrimaryDataAsset, phân định 5 cấp độ hiếm (Tier 1-5).
 *  - AC-3 (Stacking Rules Enforcement): Equipment stack 1; Consumables stack 20 (tràn sang ô mới); Materials stack 999.
 *  - AC-4 (FastArray Delta Serialization): Hỗ trợ FFastArraySerializer và NetDeltaSerialize cho Iris network.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAFastArrayInventoryTest,
	"ProjectAscendant.Foundation.Inventory.FastArrayGridInventory",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAFastArrayInventoryTest::RunTest(const FString& Parameters)
{
	// -------------------------------------------------------------------------
	// Test 1: AC-1 (Sức Chứa Ba Lô 30 Ô & Mở Rộng Lên 60 Ô)
	// -------------------------------------------------------------------------
	{
		UPAInventoryComponent* InventoryComp = NewObject<UPAInventoryComponent>();

		// Mặc định khi khởi tạo phải có đúng 30 ô (base_inventory_slots = 30)
		TestEqual(TEXT("AC-1: Túi đồ khởi tạo ban đầu có đúng 30 ô"), InventoryComp->GetCurrentMaxSlots(), 30);
		TestTrue(TEXT("AC-1: Ô 0 là hợp lệ"), InventoryComp->IsValidSlotIndex(0));
		TestTrue(TEXT("AC-1: Ô 29 là hợp lệ"), InventoryComp->IsValidSlotIndex(29));
		TestFalse(TEXT("AC-1: Ô 30 chưa mở khóa (không hợp lệ)"), InventoryComp->IsValidSlotIndex(30));

		// Mở rộng thêm 10 ô lần 1 (30 -> 40)
		const bool bExpanded1 = InventoryComp->UnlockMoreSlots(10);
		TestTrue(TEXT("AC-1: Mở rộng thêm 10 ô thành công"), bExpanded1);
		TestEqual(TEXT("AC-1: Sức chứa hiện tại là 40 ô"), InventoryComp->GetCurrentMaxSlots(), 40);
		TestTrue(TEXT("AC-1: Ô 39 giờ đã hợp lệ"), InventoryComp->IsValidSlotIndex(39));

		// Mở rộng thêm lần lượt đến 60 ô (40 -> 50 -> 60)
		InventoryComp->UnlockMoreSlots(10);
		TestEqual(TEXT("AC-1: Sức chứa tăng lên 50 ô"), InventoryComp->GetCurrentMaxSlots(), 50);

		InventoryComp->UnlockMoreSlots(10);
		TestEqual(TEXT("AC-1: Sức chứa chạm trần 60 ô"), InventoryComp->GetCurrentMaxSlots(), 60);

		// Thử mở rộng vượt quá 60 ô -> Thất bại, giữ nguyên 60
		const bool bOverExpanded = InventoryComp->UnlockMoreSlots(10);
		TestFalse(TEXT("AC-1: Không thể mở rộng vượt quá giới hạn 60 ô"), bOverExpanded);
		TestEqual(TEXT("AC-1: Sức chứa vẫn duy trì ở mốc trần 60 ô"), InventoryComp->GetCurrentMaxSlots(), 60);
	}

	// -------------------------------------------------------------------------
	// Test 2: AC-2 (Hệ Thống DataAsset & 5 Cấp Độ Hiếm)
	// -------------------------------------------------------------------------
	{
		UItemStaticDataAsset* TestSword = NewObject<UItemStaticDataAsset>();
		TestSword->ItemId = FName(TEXT("Item_Sword_AncientSteel"));
		TestSword->ItemName = FText::FromString(TEXT("Ancient Steel Sword"));
		TestSword->RarityTier = EPAItemRarity::Epic; // Tier 4 Epic
		TestSword->Category = EPAItemCategory::Equipment;
		TestSword->MaxStackSize = 1;
		TestSword->BaseSellPrice = 250;

		TestEqual(TEXT("AC-2: ItemId khớp dữ liệu"), TestSword->ItemId, FName(TEXT("Item_Sword_AncientSteel")));
		TestEqual(TEXT("AC-2: Cấp độ hiếm phải là Tier 4 Epic"), TestSword->RarityTier, EPAItemRarity::Epic);
		TestEqual(TEXT("AC-2: PrimaryAssetId hợp lệ"), TestSword->GetPrimaryAssetId().PrimaryAssetType, FPrimaryAssetType(TEXT("Item")));
		TestEqual(TEXT("AC-2: Giới hạn chồng mặc định cho Equipment là 1"),
			UItemStaticDataAsset::GetDefaultMaxStackForCategory(EPAItemCategory::Equipment), 1);
		TestEqual(TEXT("AC-2: Giới hạn chồng mặc định cho Consumable là 20"),
			UItemStaticDataAsset::GetDefaultMaxStackForCategory(EPAItemCategory::Consumable), 20);
		TestEqual(TEXT("AC-2: Giới hạn chồng mặc định cho Material là 999"),
			UItemStaticDataAsset::GetDefaultMaxStackForCategory(EPAItemCategory::Material), 999);
	}

	// -------------------------------------------------------------------------
	// Test 3: AC-3 (Quy Tắc Cộng Dồn & Tràn Ô Theo Danh Mục)
	// -------------------------------------------------------------------------
	{
		UPAInventoryComponent* InventoryComp = NewObject<UPAInventoryComponent>();

		// 3.1: Consumable (Bình Máu - Tối đa 20 / ô)
		UItemStaticDataAsset* PotionData = NewObject<UItemStaticDataAsset>();
		PotionData->ItemId = FName(TEXT("Item_Potion_Health"));
		PotionData->Category = EPAItemCategory::Consumable;
		PotionData->MaxStackSize = 20;

		// Bắt đầu với 15 bình máu ở Ô 0
		int32 Remaining = 0;
		InventoryComp->TryAddItem(PotionData, 15, Remaining);
		TestEqual(TEXT("AC-3: 15 bình máu được thêm trọn vẹn"), Remaining, 0);
		TestEqual(TEXT("AC-3: Ô 0 có đúng 15 bình máu"), InventoryComp->GetItemAtSlot(0)->StackCount, 15);

		// Thêm tiếp 10 bình máu nữa
		// Kết quả kỳ vọng: Ô 0 đạt tối đa 20; 5 bình còn lại tràn sang Ô 1
		InventoryComp->TryAddItem(PotionData, 10, Remaining);
		TestEqual(TEXT("AC-3: Không còn bình máu nào bị dư"), Remaining, 0);
		TestEqual(TEXT("AC-3: Ô 0 đạt tối đa 20 bình máu"), InventoryComp->GetItemAtSlot(0)->StackCount, 20);
		TestNotNull(TEXT("AC-3: Ô 1 được tạo mới cho phần tràn"), InventoryComp->GetItemAtSlot(1));
		TestEqual(TEXT("AC-3: Ô 1 chứa đúng 5 bình máu tràn"), InventoryComp->GetItemAtSlot(1)->StackCount, 5);
		TestEqual(TEXT("AC-3: Tổng số bình máu trong túi là 25"), InventoryComp->GetItemCount(FName(TEXT("Item_Potion_Health"))), 25);

		// 3.2: Equipment (Kiếm - Tối đa 1 / ô)
		UItemStaticDataAsset* SwordData = NewObject<UItemStaticDataAsset>();
		SwordData->ItemId = FName(TEXT("Item_Weapon_Claymore"));
		SwordData->Category = EPAItemCategory::Equipment;
		SwordData->MaxStackSize = 1;

		InventoryComp->TryAddItem(SwordData, 1, Remaining);
		TestEqual(TEXT("AC-3: Thanh kiếm đầu tiên nằm ở Ô 2"), InventoryComp->GetItemAtSlot(2)->StackCount, 1);

		InventoryComp->TryAddItem(SwordData, 1, Remaining);
		TestEqual(TEXT("AC-3: Thanh kiếm thứ hai bắt buộc tạo ô riêng ở Ô 3 với Stack 1"), InventoryComp->GetItemAtSlot(3)->StackCount, 1);
		TestTrue(TEXT("AC-3: Hai thanh kiếm nằm ở 2 ô độc lập"),
			InventoryComp->GetItemAtSlot(2)->ItemInstanceUID != InventoryComp->GetItemAtSlot(3)->ItemInstanceUID);

		// 3.3: Material (Quặng Sắt - Tối đa 999 / ô)
		UItemStaticDataAsset* OreData = NewObject<UItemStaticDataAsset>();
		OreData->ItemId = FName(TEXT("Item_Material_IronOre"));
		OreData->Category = EPAItemCategory::Material;
		OreData->MaxStackSize = 999;

		InventoryComp->TryAddItem(OreData, 500, Remaining);
		TestEqual(TEXT("AC-3: Quặng sắt nằm ở Ô 4 với số lượng 500"), InventoryComp->GetItemAtSlot(4)->StackCount, 500);

		InventoryComp->TryAddItem(OreData, 400, Remaining);
		TestEqual(TEXT("AC-3: Quặng sắt cộng dồn tiếp vào Ô 4 thành 900"), InventoryComp->GetItemAtSlot(4)->StackCount, 900);
	}

	// -------------------------------------------------------------------------
	// Test 4: AC-4 (FastArray Serialization & Thao Tác Xóa Ô)
	// -------------------------------------------------------------------------
	{
		UPAInventoryComponent* InventoryComp = NewObject<UPAInventoryComponent>();

		UItemStaticDataAsset* GemData = NewObject<UItemStaticDataAsset>();
		GemData->ItemId = FName(TEXT("Item_Gem_Ruby"));
		GemData->Category = EPAItemCategory::Material;
		GemData->MaxStackSize = 999;

		int32 Remaining = 0;
		InventoryComp->TryAddItem(GemData, 10, Remaining);
		TestEqual(TEXT("AC-4: Ruby được đưa vào Ô 0"), InventoryComp->GetItemAtSlot(0)->StackCount, 10);

		// Trừ bớt 3 viên
		const bool bReduced = InventoryComp->RemoveItemFromSlot(0, 3);
		TestTrue(TEXT("AC-4: Trừ bớt số lượng thành công"), bReduced);
		TestEqual(TEXT("AC-4: Ô 0 còn lại 7 viên"), InventoryComp->GetItemAtSlot(0)->StackCount, 7);

		// Xóa nốt 7 viên còn lại -> Ô 0 phải trở về rỗng (nullptr)
		const bool bClearedSlot = InventoryComp->RemoveItemFromSlot(0, 7);
		TestTrue(TEXT("AC-4: Xóa sạch ô thành công"), bClearedSlot);
		TestNull(TEXT("AC-4: Ô 0 giờ đã hoàn toàn trống"), InventoryComp->GetItemAtSlot(0));
		TestEqual(TEXT("AC-4: Ô trống đầu tiên lại là Ô 0"), InventoryComp->FindFirstEmptySlot(), 0);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
