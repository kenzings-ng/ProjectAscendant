// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/PAInventoryTypes.h"
#include "PAPaperdollTypes.generated.h"

/**
 * EPAPaperdollLayer
 *
 * Các lớp hiển thị đồ họa Modular Paperdoll xếp chồng lên nhân vật:
 * - BaseBody: Thân nhân vật mặc định (Quần áo vải thô tân thủ - Starter Linen Cloth).
 * - ChestArmor: Áo giáp thân (Giáp thép tấm, Áo da thợ săn, Pháp bào ma thuật...).
 * - Helmet: Mũ bảo vệ đầu / Mũ trùm.
 * - MainhandWeapon: Vũ khí tay phải (Đại kiếm, Cung tên, Trượng phép, Chùy thánh).
 * - OffhandShield: Vũ khí phụ / Khiên tay trái (Khiên tròn, Dao găm, Sách phép).
 */
UENUM(BlueprintType)
enum class EPAPaperdollLayer : uint8
{
	BaseBody         = 0 UMETA(DisplayName = "Thân Cơ Bản (Đồ Vải Tân Thủ)"),
	ChestArmor       = 1 UMETA(DisplayName = "Áo Giáp Thân (Chest Armor)"),
	Helmet           = 2 UMETA(DisplayName = "Mũ Nón (Helmet)"),
	MainhandWeapon   = 3 UMETA(DisplayName = "Vũ Khí Chính (Mainhand)"),
	OffhandShield    = 4 UMETA(DisplayName = "Vũ Khí Phụ / Khiên (Offhand)")
};

/**
 * FPAPaperdollVisualSlot
 *
 * Dữ liệu của một lớp trang bị hiển thị.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAPaperdollVisualSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	FName VisualAssetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	bool bIsVisible = false;

	FPAPaperdollVisualSlot() = default;

	FPAPaperdollVisualSlot(FName InItemId, FName InVisualId, bool bInVisible = true)
		: ItemId(InItemId)
		, VisualAssetId(InVisualId)
		, bIsVisible(bInVisible)
	{
	}
};

/**
 * FPAPaperdollAppearance
 *
 * Tổng hợp ngoại hình các lớp trang bị hiện tại của nhân vật.
 * Đồng bộ qua mạng Iris để mọi người chơi khác trong thế giới đều nhìn thấy.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAPaperdollAppearance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	FPAPaperdollVisualSlot BaseBody;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	FPAPaperdollVisualSlot ChestArmor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	FPAPaperdollVisualSlot Helmet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	FPAPaperdollVisualSlot Mainhand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	FPAPaperdollVisualSlot Offhand;

	FPAPaperdollAppearance()
	{
		// Mặc định luôn khởi tạo thân nhân vật mặc đồ vải thô tân thủ
		BaseBody = FPAPaperdollVisualSlot(FName(TEXT("item_starter_cloth")), FName(TEXT("Visual_StarterCloth")), true);
		ChestArmor = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
		Helmet = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
		Mainhand = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
		Offhand = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
	}
};

/**
 * FPAPaperdollModel
 *
 * Pure data model quản lý logic hiển thị Paperdoll:
 * - Khi chưa mặc giáp: Hiển thị bộ quần áo vải tân thủ (`Visual_StarterCloth`).
 * - Khi mặc trang bị (Click item): Cập nhật lớp Sprite tương ứng (Giáp, Vũ khí, Khiên).
 * - Khi tháo trang bị (Unequip): Tự động hồi phục về trạng thái đồ vải ban đầu.
 * 100% headless testable.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAPaperdollModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Paperdoll|Model")
	FPAPaperdollAppearance CurrentAppearance;

	FPAPaperdollModel()
	{
		ResetToStarterCloth();
	}

	/**
	 * Khởi tạo / Trả nhân vật về trạng thái mặc đồ vải thô tân thủ.
	 */
	void ResetToStarterCloth()
	{
		CurrentAppearance = FPAPaperdollAppearance();
	}

	/**
	 * Mặc một món trang bị lên lớp Paperdoll tương ứng.
	 * @return true nếu trang phục hiển thị được cập nhật.
	 */
	bool EquipVisual(EPAPaperdollLayer Layer, FName ItemId, FName VisualAssetId)
	{
		if (ItemId.IsNone() || VisualAssetId.IsNone())
		{
			return false;
		}

		switch (Layer)
		{
		case EPAPaperdollLayer::ChestArmor:
			CurrentAppearance.ChestArmor = FPAPaperdollVisualSlot(ItemId, VisualAssetId, true);
			break;
		case EPAPaperdollLayer::Helmet:
			CurrentAppearance.Helmet = FPAPaperdollVisualSlot(ItemId, VisualAssetId, true);
			break;
		case EPAPaperdollLayer::MainhandWeapon:
			CurrentAppearance.Mainhand = FPAPaperdollVisualSlot(ItemId, VisualAssetId, true);
			break;
		case EPAPaperdollLayer::OffhandShield:
			CurrentAppearance.Offhand = FPAPaperdollVisualSlot(ItemId, VisualAssetId, true);
			break;
		case EPAPaperdollLayer::BaseBody:
			CurrentAppearance.BaseBody = FPAPaperdollVisualSlot(ItemId, VisualAssetId, true);
			break;
		default:
			return false;
		}

		return true;
	}

	/**
	 * Tháo một món trang bị khỏi lớp Paperdoll tương ứng.
	 * @return true nếu tháo thành công.
	 */
	bool UnequipVisual(EPAPaperdollLayer Layer)
	{
		switch (Layer)
		{
		case EPAPaperdollLayer::ChestArmor:
			CurrentAppearance.ChestArmor = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
			break;
		case EPAPaperdollLayer::Helmet:
			CurrentAppearance.Helmet = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
			break;
		case EPAPaperdollLayer::MainhandWeapon:
			CurrentAppearance.Mainhand = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
			break;
		case EPAPaperdollLayer::OffhandShield:
			CurrentAppearance.Offhand = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
			break;
		case EPAPaperdollLayer::BaseBody:
			// BaseBody luôn duy trì mặc định là đồ vải tân thủ
			CurrentAppearance.BaseBody = FPAPaperdollVisualSlot(FName(TEXT("item_starter_cloth")), FName(TEXT("Visual_StarterCloth")), true);
			break;
		default:
			return false;
		}

		return true;
	}

	/**
	 * Kiểm tra xem nhân vật hiện có đang mặc quần áo vải tân thủ hay không
	 * (tức là ô ChestArmor đang trống, không khoác giáp nặng bên ngoài).
	 */
	bool IsWearingStarterCloth() const
	{
		return !CurrentAppearance.ChestArmor.bIsVisible || CurrentAppearance.ChestArmor.VisualAssetId.IsNone();
	}

	/**
	 * Lấy mã VisualAsset đang hiển thị ở lớp chỉ định.
	 */
	FName GetActiveVisualAssetId(EPAPaperdollLayer Layer) const
	{
		switch (Layer)
		{
		case EPAPaperdollLayer::ChestArmor:
			return CurrentAppearance.ChestArmor.bIsVisible ? CurrentAppearance.ChestArmor.VisualAssetId : FName(TEXT("Visual_StarterCloth"));
		case EPAPaperdollLayer::Helmet:
			return CurrentAppearance.Helmet.bIsVisible ? CurrentAppearance.Helmet.VisualAssetId : NAME_None;
		case EPAPaperdollLayer::MainhandWeapon:
			return CurrentAppearance.Mainhand.bIsVisible ? CurrentAppearance.Mainhand.VisualAssetId : NAME_None;
		case EPAPaperdollLayer::OffhandShield:
			return CurrentAppearance.Offhand.bIsVisible ? CurrentAppearance.Offhand.VisualAssetId : NAME_None;
		case EPAPaperdollLayer::BaseBody:
			return CurrentAppearance.BaseBody.VisualAssetId;
		default:
			return NAME_None;
		}
	}

	/**
	 * Chuyển đổi từ EPAEquipmentSlot của Inventory sang EPAPaperdollLayer.
	 */
	static bool EquipmentSlotToPaperdollLayer(EPAEquipmentSlot InSlot, EPAPaperdollLayer& OutLayer)
	{
		switch (InSlot)
		{
		case EPAEquipmentSlot::BodyArmor:
			OutLayer = EPAPaperdollLayer::ChestArmor;
			return true;
		case EPAEquipmentSlot::Mainhand:
			OutLayer = EPAPaperdollLayer::MainhandWeapon;
			return true;
		case EPAEquipmentSlot::Offhand:
			OutLayer = EPAPaperdollLayer::OffhandShield;
			return true;
		default:
			return false; // Amulet, Ring không hiển thị lớp áo giáp lớn bên ngoài
		}
	}
};
