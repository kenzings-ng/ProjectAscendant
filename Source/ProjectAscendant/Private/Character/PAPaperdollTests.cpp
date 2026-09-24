// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Character/PAPaperdollTypes.h"
#include "Inventory/PAItemStaticDataAsset.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAPaperdollTests
 *
 * Kiểm thử tự động cho hệ thống trực quan hóa trang bị đa tầng Modular Paperdoll:
 * - AC-1: Default Appearance - Khởi tạo nhân vật luôn mặc định ở trạng thái đồ vải thô tân thủ (Starter Linen Cloth).
 * - AC-2: Equip Body Armor - Khi trang bị áo giáp, lớp giáp trở nên tích cực và che phủ đồ vải (IsWearingStarterCloth == false).
 * - AC-3: Unequip Body Armor - Khi tháo áo giáp, nhân vật lập tức trở lại trang phục đồ vải ban đầu.
 * - AC-4: Equip Weapons & Shield - Mặc vũ khí chính và khiên phụ hiển thị độc lập, không làm mất lớp áo.
 * - AC-5: Inventory Slot Mapping - Chuyển đổi chính xác từ EPAEquipmentSlot sang EPAPaperdollLayer.
 *
 * 100% headless testable trên FPAPaperdollModel.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAPaperdollTests,
	"ProjectAscendant.Character.PaperdollModularSystem",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAPaperdollTests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: Default Appearance (Starter Linen Cloth)
	// ===========================================================
	{
		FPAPaperdollModel Model;

		TestTrue(TEXT("AC1: Default character is wearing Starter Linen Cloth"), Model.IsWearingStarterCloth());
		TestEqual(TEXT("AC1: Base body visual asset is Visual_StarterCloth"), Model.GetActiveVisualAssetId(EPAPaperdollLayer::BaseBody), FName(TEXT("Visual_StarterCloth")));
		TestFalse(TEXT("AC1: ChestArmor slot is not visible initially"), Model.CurrentAppearance.ChestArmor.bIsVisible);
		TestFalse(TEXT("AC1: Helmet slot is not visible initially"), Model.CurrentAppearance.Helmet.bIsVisible);
		TestFalse(TEXT("AC1: Mainhand slot is not visible initially"), Model.CurrentAppearance.Mainhand.bIsVisible);
		TestFalse(TEXT("AC1: Offhand slot is not visible initially"), Model.CurrentAppearance.Offhand.bIsVisible);
	}

	// ===========================================================
	// AC-2: Equip Body Armor (Chest Armor Layer)
	// ===========================================================
	{
		FPAPaperdollModel Model;

		const bool bEquipped = Model.EquipVisual(EPAPaperdollLayer::ChestArmor, FName(TEXT("item_iron_armor")), FName(TEXT("Visual_IronArmor")));
		TestTrue(TEXT("AC2: EquipVisual for ChestArmor succeeded"), bEquipped);
		TestFalse(TEXT("AC2: Character is no longer wearing only starter cloth"), Model.IsWearingStarterCloth());
		TestTrue(TEXT("AC2: ChestArmor layer is visible"), Model.CurrentAppearance.ChestArmor.bIsVisible);
		TestEqual(TEXT("AC2: ChestArmor has Visual_IronArmor asset"), Model.GetActiveVisualAssetId(EPAPaperdollLayer::ChestArmor), FName(TEXT("Visual_IronArmor")));
		TestEqual(TEXT("AC2: ChestArmor ItemId matches item_iron_armor"), Model.CurrentAppearance.ChestArmor.ItemId, FName(TEXT("item_iron_armor")));
	}

	// ===========================================================
	// AC-3: Unequip Body Armor (Revert to Starter Cloth)
	// ===========================================================
	{
		FPAPaperdollModel Model;

		Model.EquipVisual(EPAPaperdollLayer::ChestArmor, FName(TEXT("item_arcanist_robe")), FName(TEXT("Visual_ArcanistRobe")));
		TestFalse(TEXT("AC3: Wearing Arcanist Robe"), Model.IsWearingStarterCloth());

		const bool bUnequipped = Model.UnequipVisual(EPAPaperdollLayer::ChestArmor);
		TestTrue(TEXT("AC3: UnequipVisual succeeded"), bUnequipped);
		TestTrue(TEXT("AC3: Character reverted to starter cloth"), Model.IsWearingStarterCloth());
		TestFalse(TEXT("AC3: ChestArmor is hidden"), Model.CurrentAppearance.ChestArmor.bIsVisible);
		TestEqual(TEXT("AC3: Active visual defaults back to Visual_StarterCloth"), Model.GetActiveVisualAssetId(EPAPaperdollLayer::ChestArmor), FName(TEXT("Visual_StarterCloth")));
	}

	// ===========================================================
	// AC-4: Equip Weapons & Offhand (Layer Stacking)
	// ===========================================================
	{
		FPAPaperdollModel Model;

		// Mặc trang phục Thợ săn Da (Ranger Leather)
		Model.EquipVisual(EPAPaperdollLayer::ChestArmor, FName(TEXT("item_leather_ranger")), FName(TEXT("Visual_LeatherRanger")));

		// Trang bị Cung tên và Dao găm phụ
		Model.EquipVisual(EPAPaperdollLayer::MainhandWeapon, FName(TEXT("item_recurve_bow")), FName(TEXT("Visual_RecurveBow")));
		Model.EquipVisual(EPAPaperdollLayer::OffhandShield, FName(TEXT("item_hunter_dagger")), FName(TEXT("Visual_HunterDagger")));

		TestEqual(TEXT("AC4: Mainhand visual is Visual_RecurveBow"), Model.GetActiveVisualAssetId(EPAPaperdollLayer::MainhandWeapon), FName(TEXT("Visual_RecurveBow")));
		TestEqual(TEXT("AC4: Offhand visual is Visual_HunterDagger"), Model.GetActiveVisualAssetId(EPAPaperdollLayer::OffhandShield), FName(TEXT("Visual_HunterDagger")));
		TestEqual(TEXT("AC4: ChestArmor visual is Visual_LeatherRanger"), Model.GetActiveVisualAssetId(EPAPaperdollLayer::ChestArmor), FName(TEXT("Visual_LeatherRanger")));

		// Tháo vũ khí chính
		Model.UnequipVisual(EPAPaperdollLayer::MainhandWeapon);
		TestEqual(TEXT("AC4: Mainhand active visual is None after unequip"), Model.GetActiveVisualAssetId(EPAPaperdollLayer::MainhandWeapon), FName(NAME_None));
		// Giáp và vũ khí phụ vẫn được giữ nguyên
		TestEqual(TEXT("AC4: Offhand visual remains"), Model.GetActiveVisualAssetId(EPAPaperdollLayer::OffhandShield), FName(TEXT("Visual_HunterDagger")));
		TestEqual(TEXT("AC4: Chest visual remains"), Model.GetActiveVisualAssetId(EPAPaperdollLayer::ChestArmor), FName(TEXT("Visual_LeatherRanger")));
	}

	// ===========================================================
	// AC-5: Inventory Slot to Paperdoll Layer Mapping
	// ===========================================================
	{
		EPAPaperdollLayer OutLayer;

		TestTrue(TEXT("AC5: BodyArmor maps to ChestArmor"), FPAPaperdollModel::EquipmentSlotToPaperdollLayer(EPAEquipmentSlot::BodyArmor, OutLayer));
		TestEqual(TEXT("AC5: OutLayer is ChestArmor"), OutLayer, EPAPaperdollLayer::ChestArmor);

		TestTrue(TEXT("AC5: Mainhand maps to MainhandWeapon"), FPAPaperdollModel::EquipmentSlotToPaperdollLayer(EPAEquipmentSlot::Mainhand, OutLayer));
		TestEqual(TEXT("AC5: OutLayer is MainhandWeapon"), OutLayer, EPAPaperdollLayer::MainhandWeapon);

		TestTrue(TEXT("AC5: Offhand maps to OffhandShield"), FPAPaperdollModel::EquipmentSlotToPaperdollLayer(EPAEquipmentSlot::Offhand, OutLayer));
		TestEqual(TEXT("AC5: OutLayer is OffhandShield"), OutLayer, EPAPaperdollLayer::OffhandShield);

		TestFalse(TEXT("AC5: Ring1 does not map to large paperdoll layer"), FPAPaperdollModel::EquipmentSlotToPaperdollLayer(EPAEquipmentSlot::Ring1, OutLayer));
		TestFalse(TEXT("AC5: Amulet does not map to large paperdoll layer"), FPAPaperdollModel::EquipmentSlotToPaperdollLayer(EPAEquipmentSlot::Amulet, OutLayer));
	}

	// ===========================================================
	// AC-6: DataAsset Presets & Paperdoll Visual IDs
	// ===========================================================
	{
		UItemStaticDataAsset* ClothAsset = NewObject<UItemStaticDataAsset>();
		ClothAsset->SetupArmorStarterCloth();
		TestEqual(TEXT("AC6: Starter cloth visual ID is Visual_StarterCloth"), ClothAsset->PaperdollVisualId, FName(TEXT("Visual_StarterCloth")));
		TestEqual(TEXT("AC6: Starter cloth slot is BodyArmor"), ClothAsset->AllowedEquipmentSlot, EPAEquipmentSlot::BodyArmor);

		UItemStaticDataAsset* IronAsset = NewObject<UItemStaticDataAsset>();
		IronAsset->SetupArmorIronPlate();
		TestEqual(TEXT("AC6: Iron armor visual ID is Visual_IronArmor"), IronAsset->PaperdollVisualId, FName(TEXT("Visual_IronArmor")));

		UItemStaticDataAsset* RangerAsset = NewObject<UItemStaticDataAsset>();
		RangerAsset->SetupArmorLeatherRanger();
		TestEqual(TEXT("AC6: Ranger leather visual ID is Visual_LeatherRanger"), RangerAsset->PaperdollVisualId, FName(TEXT("Visual_LeatherRanger")));

		UItemStaticDataAsset* RobeAsset = NewObject<UItemStaticDataAsset>();
		RobeAsset->SetupArmorArcanistRobe();
		TestEqual(TEXT("AC6: Arcanist robe visual ID is Visual_ArcanistRobe"), RobeAsset->PaperdollVisualId, FName(TEXT("Visual_ArcanistRobe")));
	}

	return true;
}

/**
 * FPAPaperdoll9SlotTest
 *
 * Kiểm thử cho Story item-004 (Paperdoll 9-Slot, EPIC-ITEMIZATION-001, Sprint 6):
 * - AC-1: Khởi tạo 9 sub-component (Helm/Chest/Gloves/Pants/Boots/MainHand/OffHand/Amulet/Ring),
 *         placeholder block 128x128, pivot chân (64, 114), socket HandSocket_R và HandSocket_L.
 * - AC-2: Binding và trang bị trên toàn bộ 9 slot.
 * - AC-3: Directional Sort Key đảo priority giữa MainHand và OffHand khi mirror hướng Tây,
 *         không bị đảo lệch thứ tự lớp giữa trái/phải.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAPaperdoll9SlotTest,
	"ProjectAscendant.Itemization.Paperdoll9Slot",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAPaperdoll9SlotTest::RunTest(const FString& Parameters)
{
	// =========================================================================
	// AC-1: 9-Slot Hierarchy, Foot Pivot & Sockets
	// =========================================================================
	{
		TestEqual(TEXT("AC-1: Paperdoll defines 9 distinct slots"), (uint8)EPAPaperdollSlot::Count, (uint8)9);

		TestEqual(TEXT("AC-1: Placeholder block dimensions are 128x128"),
			FPAPaperdollConstants::PlaceholderDimensions, FIntPoint(128, 128));

		TestEqual(TEXT("AC-1: Foot pivot is (64, 114)"),
			FPAPaperdollConstants::FootPivot, FVector2D(64.0f, 114.0f));

		TestEqual(TEXT("AC-1: MainHand socket is HandSocket_R"),
			FPAPaperdollSortKey::GetSocketNameForSlot(EPAPaperdollSlot::MainHand), FName(TEXT("HandSocket_R")));

		TestEqual(TEXT("AC-1: OffHand socket is HandSocket_L"),
			FPAPaperdollSortKey::GetSocketNameForSlot(EPAPaperdollSlot::OffHand), FName(TEXT("HandSocket_L")));

		TestEqual(TEXT("AC-1: Helm has no specialized hand socket"),
			FPAPaperdollSortKey::GetSocketNameForSlot(EPAPaperdollSlot::Helm), FName(NAME_None));
	}

	// =========================================================================
	// AC-2: 9-Slot Equip & Unequip Logic
	// =========================================================================
	{
		FPAPaperdollModel Model;

		const struct FSlotTestData
		{
			EPAPaperdollSlot Slot;
			FName ItemId;
			FName VisualId;
		} TestSlots[] =
		{
			{ EPAPaperdollSlot::Helm,     FName("item_iron_helm"),    FName("Visual_IronHelm") },
			{ EPAPaperdollSlot::Chest,    FName("item_iron_chest"),   FName("Visual_IronChest") },
			{ EPAPaperdollSlot::Gloves,   FName("item_steel_gloves"), FName("Visual_SteelGloves") },
			{ EPAPaperdollSlot::Pants,    FName("item_iron_pants"),   FName("Visual_IronPants") },
			{ EPAPaperdollSlot::Boots,    FName("item_iron_boots"),   FName("Visual_IronBoots") },
			{ EPAPaperdollSlot::MainHand, FName("item_broadsword"),   FName("Visual_Broadsword") },
			{ EPAPaperdollSlot::OffHand,  FName("item_iron_shield"),  FName("Visual_IronShield") },
			{ EPAPaperdollSlot::Amulet,   FName("item_ruby_amulet"),  FName("Visual_RubyAmulet") },
			{ EPAPaperdollSlot::Ring,     FName("item_gold_ring"),    FName("Visual_GoldRing") }
		};

		// 1. Kiểm tra ban đầu mọi slot đều trống
		for (const auto& TestData : TestSlots)
		{
			TestFalse(FString::Printf(TEXT("AC-2: Slot %d is initially empty"), (int32)TestData.Slot),
				Model.IsSlotEquipped(TestData.Slot));
		}

		// 2. Mặc trang bị lên cả 9 slot
		for (const auto& TestData : TestSlots)
		{
			bool bEquipped = Model.EquipSlot(TestData.Slot, TestData.ItemId, TestData.VisualId);
			TestTrue(FString::Printf(TEXT("AC-2: EquipSlot succeeded for slot %d"), (int32)TestData.Slot), bEquipped);
			TestTrue(FString::Printf(TEXT("AC-2: Slot %d is equipped"), (int32)TestData.Slot), Model.IsSlotEquipped(TestData.Slot));
			TestEqual(FString::Printf(TEXT("AC-2: Active visual matches for slot %d"), (int32)TestData.Slot),
				Model.GetActiveVisualAssetForSlot(TestData.Slot), TestData.VisualId);
		}

		// 3. Tháo trang bị từng slot và kiểm tra hoàn trả
		for (const auto& TestData : TestSlots)
		{
			bool bUnequipped = Model.UnequipSlot(TestData.Slot);
			TestTrue(FString::Printf(TEXT("AC-2: UnequipSlot succeeded for slot %d"), (int32)TestData.Slot), bUnequipped);
			TestFalse(FString::Printf(TEXT("AC-2: Slot %d is no longer equipped"), (int32)TestData.Slot), Model.IsSlotEquipped(TestData.Slot));
			TestEqual(FString::Printf(TEXT("AC-2: Visual cleared to None for slot %d"), (int32)TestData.Slot),
				Model.GetActiveVisualAssetForSlot(TestData.Slot), FName(NAME_None));
		}
	}

	// =========================================================================
	// AC-3: Directional Sort Key & Horizontal Mirroring Priority Inversion
	// =========================================================================
	{
		// 1. Kiểm tra phát hiện hướng Mirror
		TestFalse(TEXT("AC-3: East is NOT mirrored"), FPAPaperdollSortKey::IsMirroredDirection(EPAAimDirection8Way::East));
		TestFalse(TEXT("AC-3: NorthEast is NOT mirrored"), FPAPaperdollSortKey::IsMirroredDirection(EPAAimDirection8Way::NorthEast));
		TestFalse(TEXT("AC-3: SouthEast is NOT mirrored"), FPAPaperdollSortKey::IsMirroredDirection(EPAAimDirection8Way::SouthEast));

		TestTrue(TEXT("AC-3: West IS mirrored"), FPAPaperdollSortKey::IsMirroredDirection(EPAAimDirection8Way::West));
		TestTrue(TEXT("AC-3: NorthWest IS mirrored"), FPAPaperdollSortKey::IsMirroredDirection(EPAAimDirection8Way::NorthWest));
		TestTrue(TEXT("AC-3: SouthWest IS mirrored"), FPAPaperdollSortKey::IsMirroredDirection(EPAAimDirection8Way::SouthWest));

		// 2. Hướng Đông (Chuẩn không lật): MainHand (50) nằm TRƯỚC Chest (25) và OffHand (5) nằm SAU
		const int32 EastMain = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::MainHand, EPAAimDirection8Way::East);
		const int32 EastOff = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::OffHand, EPAAimDirection8Way::East);
		const int32 EastChest = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::Chest, EPAAimDirection8Way::East);

		TestEqual(TEXT("AC-3: East MainHand priority is 50"), EastMain, 50);
		TestEqual(TEXT("AC-3: East OffHand priority is 5"), EastOff, 5);
		TestTrue(TEXT("AC-3: East MainHand > Chest > OffHand"), EastMain > EastChest && EastChest > EastOff);

		// 3. Hướng Tây (Mirror lật ngang): ĐẢO PRIORITY giữa MainHand và OffHand
		// OffHand (50) giờ đây nằm TRƯỚC Chest (25), MainHand (5) chuyển ra SAU lưng
		const int32 WestMain = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::MainHand, EPAAimDirection8Way::West);
		const int32 WestOff = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::OffHand, EPAAimDirection8Way::West);
		const int32 WestChest = FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::Chest, EPAAimDirection8Way::West);

		TestEqual(TEXT("AC-3: West MainHand priority flipped to 5"), WestMain, 5);
		TestEqual(TEXT("AC-3: West OffHand priority flipped to 50"), WestOff, 50);
		TestTrue(TEXT("AC-3: West OffHand > Chest > MainHand (Z-order preserved under mirror)"), WestOff > WestChest && WestChest > WestMain);

		// 4. Các lớp thân thể không bị ảnh hưởng bởi Mirror
		TestEqual(TEXT("AC-3: Pants priority consistent East vs West"),
			FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::Pants, EPAAimDirection8Way::East),
			FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::Pants, EPAAimDirection8Way::West));

		TestEqual(TEXT("AC-3: Helm priority consistent East vs West"),
			FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::Helm, EPAAimDirection8Way::East),
			FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot::Helm, EPAAimDirection8Way::West));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
