// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Character/PAPaperdollTypes.h"

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

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
