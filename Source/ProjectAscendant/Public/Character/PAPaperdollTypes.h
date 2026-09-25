// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/PAInventoryTypes.h"
#include "Controller/PAIsometricMovementMath.h"
#include "GameplayTagContainer.h"
#include "PAPaperdollTypes.generated.h"

/**
 * EPAPaperdollSlot
 *
 * 9 Vị trí trang bị nhân vật (Story item-004, EPIC-ITEMIZATION-001, Sprint 6):
 * 1. Helm: Mũ giáp / Mũ trùm
 * 2. Chest: Giáp thân / Áo choàng
 * 3. Gloves: Găng tay
 * 4. Pants: Quần chiến / Xà cạp
 * 5. Boots: Ủng / Giày
 * 6. MainHand: Vũ khí tay phải (gắn HandSocket_R)
 * 7. OffHand: Vũ khí phụ / Khiên tay trái (gắn HandSocket_L)
 * 8. Amulet: Dây chuyền
 * 9. Ring: Nhẫn
 */
UENUM(BlueprintType)
enum class EPAPaperdollSlot : uint8
{
	Helm        = 0 UMETA(DisplayName = "Mũ Nón (Helm)"),
	Chest       = 1 UMETA(DisplayName = "Áo Giáp Thân (Chest)"),
	Gloves      = 2 UMETA(DisplayName = "Găng Tay (Gloves)"),
	Pants       = 3 UMETA(DisplayName = "Quần Chiến (Pants)"),
	Boots       = 4 UMETA(DisplayName = "Ủng (Boots)"),
	MainHand    = 5 UMETA(DisplayName = "Vũ Khí Chính (MainHand)"),
	OffHand     = 6 UMETA(DisplayName = "Vũ Khí Phụ / Khiên (OffHand)"),
	Amulet      = 7 UMETA(DisplayName = "Dây Chuyền (Amulet)"),
	Ring        = 8 UMETA(DisplayName = "Nhẫn (Ring)"),
	Count       = 9 UMETA(Hidden)
};

/**
 * EPAMasterRig
 *
 * 4 Master Animation Rigs cốt lõi (Story visual-001, EPIC-CHARACTER-VISUAL-001, Sprint 7):
 * - HeavyTank: Vanguard, Berserker, Templar, Dragon Knight (Chân bước đầm chắc, tấn thấp, chìm trọng tâm).
 * - Agility: Ranger, Shadowblade, Void Blade, God Slayer (Kiễng mũi chân, bước sải nhanh, cơ động).
 * - Caster: Arcanist, Elementalist, Chronomancer (Dáng đứng thẳng đĩnh đạc, tà váy buông, lướt nhẹ).
 * - Monk: Acolyte (Thế tấn mã bộ tĩnh, chuyển động ổn định cân bằng).
 */
UENUM(BlueprintType)
enum class EPAMasterRig : uint8
{
	HeavyTank = 0 UMETA(DisplayName = "Hạng Nặng (Heavy Tank)"),
	Agility   = 1 UMETA(DisplayName = "Linh Hoạt (Agility)"),
	Caster    = 2 UMETA(DisplayName = "Pháp Giới (Caster)"),
	Monk      = 3 UMETA(DisplayName = "Khí Công (Monk)"),
	Count     = 4 UMETA(Hidden)
};

/**
 * EPAWeaponFamily
 *
 * 7 Dòng Vũ Khí theo chuẩn GDD itemization.md & character-visual-system.md:
 * - Blade_1H: Kiếm 1 tay (Vanguard, Templar, Void Blade, God Slayer).
 * - Heavy_2H: Đại khí hạng nặng (Berserker, Vanguard 2H, Dragon Knight).
 * - Polearm_2H: Thương dài / Kích (Dragon Knight, God Slayer, Town Guard).
 * - Bow_2H: Cung tên xạ kích (Ranger).
 * - DualDaggers: Song đoản đao (Shadowblade, Ranger sub-set).
 * - Staff_2H: Trượng phép (Arcanist, Elementalist).
 * - MaceRelic_1H: Chùy & Pháp bảo (Acolyte, Templar, Chronomancer).
 * - None: Không vũ khí / Chưa gán.
 */
UENUM(BlueprintType)
enum class EPAWeaponFamily : uint8
{
	Blade_1H      = 0 UMETA(DisplayName = "Kiếm 1 Tay (1H Blade)"),
	Heavy_2H      = 1 UMETA(DisplayName = "Đại Khí 2 Tay (2H Heavy)"),
	Polearm_2H    = 2 UMETA(DisplayName = "Thương/Kích (2H Polearm)"),
	Bow_2H        = 3 UMETA(DisplayName = "Cung Tên (2H Bow)"),
	DualDaggers   = 4 UMETA(DisplayName = "Song Đoản Đao (Dual Daggers)"),
	Staff_2H      = 5 UMETA(DisplayName = "Trượng Phép (2H Staff)"),
	MaceRelic_1H  = 6 UMETA(DisplayName = "Chùy & Pháp Bảo (1H Mace/Relic)"),
	None          = 7 UMETA(DisplayName = "Không Vũ Khí (None)"),
	Count         = 8 UMETA(Hidden)
};

/** Hằng số chuẩn hóa cho Paperdoll (kích thước, socket, pivot) */
struct PROJECTASCENDANT_API FPAPaperdollConstants
{
	static const FName HandSocket_R;
	static const FName HandSocket_L;
	static const FVector2D FootPivot;             // (64, 114) theo SPEC-ART-2026-09-23-V2
	static const FIntPoint PlaceholderDimensions; // (128, 128)

	static const FName Socket_LowerBody;
	static const FName Socket_UpperBody;
	static const FVector2D WaistPivot;            // (64, 80)

	// Class Identity Sockets (Story visual-002, EPIC-CHARACTER-VISUAL-001)
	static const FName Socket_HelmCrest;          // TEXT("Socket_HelmCrest")
	static const FName Socket_Tabard;             // TEXT("Socket_Tabard")
	static const FVector2D HelmCrestLocation;     // (64, 40)
	static const FVector2D TabardLocation;        // (64, 60)
	static const FIntPoint CrestPlaceholderDimensions;  // (32, 32)
	static const FIntPoint TabardPlaceholderDimensions; // (48, 64)

	static FName GetDefaultLowerBodyAssetForRig(EPAMasterRig Rig);
	static FName GetDefaultUpperBodyAssetForFamily(EPAWeaponFamily Family);
	static EPAWeaponFamily GetWeaponFamilyFromTag(FGameplayTag Tag);
	static FGameplayTag GetTagForWeaponFamily(EPAWeaponFamily Family);

	static FName GetDefaultCrestAssetForClass(FName ClassName);
	static FName GetDefaultTabardAssetForClass(FName ClassName);
	static FGameplayTag GetTagForClass(FName ClassName);
	static FName GetClassNameFromTag(FGameplayTag ClassTag);
};

/**
 * FPAPaperdollSortKey
 *
 * Tính toán Directional Sort Key / Z-order cho từng layer Paperdoll:
 * Khi mirror hướng Tây/Tây Nam/Tây Bắc, đảo priority giữa MainHand và OffHand để bảo toàn trật tự thị sai chiều sâu.
 */
class PROJECTASCENDANT_API FPAPaperdollSortKey
{
public:
	static bool IsMirroredDirection(EPAAimDirection8Way Direction);
	static int32 GetSortPriorityForSlot(EPAPaperdollSlot Slot, EPAAimDirection8Way Direction);
	static int32 GetSortPriorityForIdentitySocket(FName SocketName, EPAAimDirection8Way Direction);
	static FName GetSocketNameForSlot(EPAPaperdollSlot Slot);
};

/**
 * FPARarityMaterialPreset
 *
 * Cấu hình Static Material Instance cho 5 bậc hiếm (Story item-005, EPIC-ITEMIZATION-001):
 * - Common: Emissive = 0.0f, Tint = #E8ECEB
 * - Uncommon: Emissive = 0.5f, Tint = #10B981 (Electric Emerald)
 * - Rare: Emissive = 1.8f, Tint = #3B82F6 (Cobalt Sapphire)
 * - Epic: Emissive = 3.5f, Tint = #A855F7 (Astral Violet)
 * - Legendary: Emissive = 7.0f, Tint = #F59E0B (Solar Amber Gold)
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPARarityMaterialPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Material")
	FGameplayTag RarityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Material")
	EPAItemRarity Rarity = EPAItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Material")
	float EmissiveIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Material")
	FLinearColor TintColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Material")
	FName MaterialPresetName = NAME_None;

	FPARarityMaterialPreset()
		: EmissiveIntensity(0.0f)
		, TintColor(FLinearColor::White)
		, MaterialPresetName(NAME_None)
	{
	}

	FPARarityMaterialPreset(
		FGameplayTag InTag,
		EPAItemRarity InRarity,
		float InEmissive,
		const FLinearColor& InTint,
		FName InName)
		: RarityTag(InTag)
		, Rarity(InRarity)
		, EmissiveIntensity(InEmissive)
		, TintColor(InTint)
		, MaterialPresetName(InName)
	{
	}
};

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
	FPAPaperdollVisualSlot Gloves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	FPAPaperdollVisualSlot Pants;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	FPAPaperdollVisualSlot Boots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	FPAPaperdollVisualSlot Mainhand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	FPAPaperdollVisualSlot Offhand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	FPAPaperdollVisualSlot Amulet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll")
	FPAPaperdollVisualSlot Ring;

	FPAPaperdollAppearance()
	{
		// Mặc định luôn khởi tạo thân nhân vật mặc đồ vải thô tân thủ
		BaseBody = FPAPaperdollVisualSlot(FName(TEXT("item_starter_cloth")), FName(TEXT("Visual_StarterCloth")), true);
		ChestArmor = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
		Helmet = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
		Gloves = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
		Pants = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
		Boots = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
		Mainhand = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
		Offhand = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
		Amulet = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
		Ring = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
	}

	FPAPaperdollVisualSlot& GetVisualSlot(EPAPaperdollSlot Slot)
	{
		switch (Slot)
		{
		case EPAPaperdollSlot::Helm: return Helmet;
		case EPAPaperdollSlot::Chest: return ChestArmor;
		case EPAPaperdollSlot::Gloves: return Gloves;
		case EPAPaperdollSlot::Pants: return Pants;
		case EPAPaperdollSlot::Boots: return Boots;
		case EPAPaperdollSlot::MainHand: return Mainhand;
		case EPAPaperdollSlot::OffHand: return Offhand;
		case EPAPaperdollSlot::Amulet: return Amulet;
		case EPAPaperdollSlot::Ring: return Ring;
		default: return BaseBody;
		}
	}

	const FPAPaperdollVisualSlot& GetVisualSlot(EPAPaperdollSlot Slot) const
	{
		switch (Slot)
		{
		case EPAPaperdollSlot::Helm: return Helmet;
		case EPAPaperdollSlot::Chest: return ChestArmor;
		case EPAPaperdollSlot::Gloves: return Gloves;
		case EPAPaperdollSlot::Pants: return Pants;
		case EPAPaperdollSlot::Boots: return Boots;
		case EPAPaperdollSlot::MainHand: return Mainhand;
		case EPAPaperdollSlot::OffHand: return Offhand;
		case EPAPaperdollSlot::Amulet: return Amulet;
		case EPAPaperdollSlot::Ring: return Ring;
		default: return BaseBody;
		}
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll|Rig")
	EPAMasterRig CurrentMasterRig = EPAMasterRig::HeavyTank;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll|Rig")
	EPAWeaponFamily CurrentWeaponFamily = EPAWeaponFamily::Blade_1H;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll|Rig")
	FName LowerBodyVisualAssetId = FName(TEXT("FB_Lower_HeavyTank_Set"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll|Rig")
	FName UpperBodyVisualAssetId = FName(TEXT("FB_Upper_1H_Blade_Combo"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll|Identity")
	FGameplayTag CurrentClassTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll|Identity")
	FName HelmCrestVisualAssetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll|Identity")
	FName TabardVisualAssetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll|Identity")
	bool bShowHelmCrest = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paperdoll|Identity")
	bool bShowTabard = true;

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
		CurrentMasterRig = EPAMasterRig::HeavyTank;
		CurrentWeaponFamily = EPAWeaponFamily::Blade_1H;
		LowerBodyVisualAssetId = FPAPaperdollConstants::GetDefaultLowerBodyAssetForRig(CurrentMasterRig);
		UpperBodyVisualAssetId = FPAPaperdollConstants::GetDefaultUpperBodyAssetForFamily(CurrentWeaponFamily);
		if (CurrentClassTag.IsValid())
		{
			const FName ClassName = FPAPaperdollConstants::GetClassNameFromTag(CurrentClassTag);
			HelmCrestVisualAssetId = FPAPaperdollConstants::GetDefaultCrestAssetForClass(ClassName);
			TabardVisualAssetId = FPAPaperdollConstants::GetDefaultTabardAssetForClass(ClassName);
		}
		else
		{
			SetClassIdentity(FName(TEXT("Vanguard")));
		}
	}

	/**
	 * Gán Class Identity theo tên Class hoặc Tag.
	 */
	void SetClassIdentity(FName InClassName)
	{
		CurrentClassTag = FPAPaperdollConstants::GetTagForClass(InClassName);
		HelmCrestVisualAssetId = FPAPaperdollConstants::GetDefaultCrestAssetForClass(InClassName);
		TabardVisualAssetId = FPAPaperdollConstants::GetDefaultTabardAssetForClass(InClassName);
	}

	bool SetClassIdentityByTag(FGameplayTag InClassTag)
	{
		if (!InClassTag.IsValid())
		{
			return false;
		}

		CurrentClassTag = InClassTag;
		const FName ClassName = FPAPaperdollConstants::GetClassNameFromTag(InClassTag);
		HelmCrestVisualAssetId = FPAPaperdollConstants::GetDefaultCrestAssetForClass(ClassName);
		TabardVisualAssetId = FPAPaperdollConstants::GetDefaultTabardAssetForClass(ClassName);
		return true;
	}

	FGameplayTag GetCurrentClassTag() const { return CurrentClassTag; }
	FName GetHelmCrestVisualAssetId() const { return bShowHelmCrest ? HelmCrestVisualAssetId : NAME_None; }
	FName GetTabardVisualAssetId() const { return bShowTabard ? TabardVisualAssetId : NAME_None; }
	void SetHelmCrestVisual(FName InAssetId) { HelmCrestVisualAssetId = InAssetId; }
	void SetTabardVisual(FName InAssetId) { TabardVisualAssetId = InAssetId; }
	void SetHelmCrestVisible(bool bVisible) { bShowHelmCrest = bVisible; }
	void SetTabardVisible(bool bVisible) { bShowTabard = bVisible; }

	/**
	 * Gán Master Rig cho Lower Body.
	 */
	void SetMasterRig(EPAMasterRig InRig)
	{
		CurrentMasterRig = InRig;
		LowerBodyVisualAssetId = FPAPaperdollConstants::GetDefaultLowerBodyAssetForRig(InRig);
	}

	EPAMasterRig GetMasterRig() const { return CurrentMasterRig; }
	FName GetLowerBodyVisualAssetId() const { return LowerBodyVisualAssetId; }

	/**
	 * Gán Weapon Family cho Upper Body.
	 */
	void SetUpperBodyWeaponFamily(EPAWeaponFamily InFamily)
	{
		CurrentWeaponFamily = InFamily;
		UpperBodyVisualAssetId = FPAPaperdollConstants::GetDefaultUpperBodyAssetForFamily(InFamily);
	}

	EPAWeaponFamily GetWeaponFamily() const { return CurrentWeaponFamily; }
	FName GetUpperBodyVisualAssetId() const { return UpperBodyVisualAssetId; }

	/**
	 * Gán Weapon Family bằng Gameplay Tag (vd Weapon.1H.Blade).
	 */
	bool SetUpperBodyWeaponFamilyByTag(FGameplayTag WeaponTag)
	{
		const EPAWeaponFamily Family = FPAPaperdollConstants::GetWeaponFamilyFromTag(WeaponTag);
		if (Family != EPAWeaponFamily::None)
		{
			SetUpperBodyWeaponFamily(Family);
			return true;
		}
		return false;
	}

	/**
	 * Mặc trang bị lên một trong 9 ô slot của Paperdoll.
	 */
	bool EquipSlot(EPAPaperdollSlot Slot, FName ItemId, FName VisualAssetId)
	{
		if (ItemId.IsNone() || VisualAssetId.IsNone())
		{
			return false;
		}

		CurrentAppearance.GetVisualSlot(Slot) = FPAPaperdollVisualSlot(ItemId, VisualAssetId, true);
		return true;
	}

	/**
	 * Tháo trang bị khỏi một ô slot trong 9 ô.
	 */
	bool UnequipSlot(EPAPaperdollSlot Slot)
	{
		CurrentAppearance.GetVisualSlot(Slot) = FPAPaperdollVisualSlot(NAME_None, NAME_None, false);
		return true;
	}

	/**
	 * Kiểm tra ô slot có đang mang trang bị và hiển thị hay không.
	 */
	bool IsSlotEquipped(EPAPaperdollSlot Slot) const
	{
		const FPAPaperdollVisualSlot& Visual = CurrentAppearance.GetVisualSlot(Slot);
		return Visual.bIsVisible && !Visual.ItemId.IsNone();
	}

	/**
	 * Lấy VisualAssetId đang hiển thị trên ô slot chỉ định.
	 */
	FName GetActiveVisualAssetForSlot(EPAPaperdollSlot Slot) const
	{
		const FPAPaperdollVisualSlot& Visual = CurrentAppearance.GetVisualSlot(Slot);
		return Visual.bIsVisible ? Visual.VisualAssetId : NAME_None;
	}

	/**
	 * Lấy Directional Sort Key / Translucent Sort Priority cho slot theo hướng ngắm 8 chiều.
	 */
	int32 GetSlotSortPriority(EPAPaperdollSlot Slot, EPAAimDirection8Way Direction) const
	{
		return FPAPaperdollSortKey::GetSortPriorityForSlot(Slot, Direction);
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
