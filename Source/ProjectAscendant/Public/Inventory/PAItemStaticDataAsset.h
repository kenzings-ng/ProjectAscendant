// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Inventory/PAInventoryTypes.h"
#include "PAItemStaticDataAsset.generated.h"

class UTexture2D;

/**
 * UItemStaticDataAsset
 *
 * Định nghĩa dữ liệu tĩnh bất biến của vật phẩm (ADR-0003, AC-2).
 * Kế thừa UPrimaryDataAsset để hỗ trợ Unreal Engine Asset Manager nạp bất đồng bộ.
 */
UCLASS(BlueprintType)
class PROJECTASCENDANT_API UItemStaticDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UItemStaticDataAsset();

	/** Khóa ngoại định danh duy nhất của loại vật phẩm (ví dụ: Item_IronSword_01) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Identification")
	FName ItemId;

	/** Tên hiển thị của vật phẩm */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display")
	FText ItemName;

	/** Mô tả công dụng và lore của vật phẩm */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display")
	FText ItemDescription;

	/** 5 Tier cấp độ hiếm (Common -> Legendary) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Properties")
	EPAItemRarity RarityTier;

	/** Phân loại vật phẩm (Equipment, Consumable, Material...) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Properties")
	EPAItemCategory Category;

	/** Số lượng tối đa trong 1 ô chồng (AC-3: Equipment/Skill=1, Consumable=20, Material=999) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Properties", meta = (ClampMin = "1", ClampMax = "999"))
	int32 MaxStackSize;

	/** Giá trị tiền tệ cơ bản khi bán cho thương nhân */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Economy", meta = (ClampMin = "0"))
	int32 BaseSellPrice;

	/** Biểu tượng đồ họa hiển thị trên lưới giao diện ba lô 30 ô */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display")
	TSoftObjectPtr<UTexture2D> IconTexture;

	// -------------------------------------------------------------------------
	// Equipment & GAS Integration (Story 003 / AC-1 & AC-2)
	// -------------------------------------------------------------------------

	/** Vị trí ô Paperdoll được phép trang bị (AC-1) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment")
	EPAEquipmentSlot AllowedEquipmentSlot = EPAEquipmentSlot::None;

	/** Mã nhận diện tài nguyên đồ họa Modular Paperdoll hiển thị trên nhân vật */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment|Paperdoll")
	FName PaperdollVisualId = NAME_None;

	/** Sát thương vật lý cộng thêm của trang bị (AC-1) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment|Stats", meta = (ClampMin = "0.0"))
	float BasePhysicalDamageBonus = 0.0f;

	/** Điểm giáp phòng ngự cộng thêm của trang bị (AC-1) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment|Stats", meta = (ClampMin = "0.0"))
	float BaseArmorBonus = 0.0f;

	/** GameplayEffect được áp dụng lên ASC khi trang bị và thu hồi khi tháo trang bị (AC-2) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment|GAS")
	TSubclassOf<class UGameplayEffect> EquipGameplayEffect;

	// -------------------------------------------------------------------------
	// Skill Book Properties (Story 003 / AC-4)
	// -------------------------------------------------------------------------

	/** Kỹ năng GAS được mở khóa khi đọc Sách Kỹ Năng (AC-4) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|SkillBook")
	TSubclassOf<class UGameplayAbility> GrantedAbilityClass;

	/** Thẻ chức nghiệp yêu cầu để học kỹ năng (ví dụ: Class.Ranger, Class.Vanguard - AC-4) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|SkillBook")
	FGameplayTag RequiredClassTag;

	// -------------------------------------------------------------------------
	// Consumable Properties (Story 003 / AC-3)
	// -------------------------------------------------------------------------

	/** Vị trí phím tắt Quickbar ưu tiên / mặc định nếu là vật phẩm tiêu hao (AC-2) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Consumable")
	EPAQuickbarSlot PreferredQuickbarSlot = EPAQuickbarSlot::None;

	/** Thời gian thực hiện hành động dùng tiêu hao / uống dược phẩm (mặc định 0.8s - AC-3) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Consumable", meta = (ClampMin = "0.0"))
	float ConsumableDuration = 0.8f;

	/** Lượng máu hồi phục cơ bản khi uống bình dược phẩm (mặc định 150 điểm - AC-3) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Consumable", meta = (ClampMin = "0.0"))
	float ConsumableHealthRestore = 150.0f;

	/** Tỷ lệ giảm tốc độ di chuyển trong lúc uống bình tiêu hao (mặc định 30% - AC-3) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Consumable", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ConsumableSpeedDebuffRatio = 0.30f;

	/** Trả về giới hạn chồng đồ mặc định theo từng danh mục vật phẩm (AC-3) */
	UFUNCTION(BlueprintPure, Category = "Item|Rules")
	static int32 GetDefaultMaxStackForCategory(EPAItemCategory InCategory);

	// -------------------------------------------------------------------------
	// Core Setup Presets (Story item-001)
	// -------------------------------------------------------------------------

	/** Thiết lập thông số tiêu chuẩn cho Kiếm Sắt (AC-1) */
	UFUNCTION(BlueprintCallable, Category = "Item|Setup")
	void SetupWeaponIronSword();

	/** Thiết lập thông số tiêu chuẩn cho Giáp Sắt Thân (AC-1) */
	UFUNCTION(BlueprintCallable, Category = "Item|Setup")
	void SetupArmorIronPlate();

	/** Thiết lập thông số tiêu chuẩn cho Quần Áo Vải Thô Tân Thủ */
	UFUNCTION(BlueprintCallable, Category = "Item|Setup")
	void SetupArmorStarterCloth();

	/** Thiết lập thông số tiêu chuẩn cho Áo Da Thợ Săn */
	UFUNCTION(BlueprintCallable, Category = "Item|Setup")
	void SetupArmorLeatherRanger();

	/** Thiết lập thông số tiêu chuẩn cho Pháp Bào Thuật Sĩ */
	UFUNCTION(BlueprintCallable, Category = "Item|Setup")
	void SetupArmorArcanistRobe();

	/** Thiết lập thông số tiêu chuẩn cho Bình Máu Tiêu Hao (AC-2) */
	UFUNCTION(BlueprintCallable, Category = "Item|Setup")
	void SetupPotionHealth();

	/** Thiết lập thông số tiêu chuẩn cho Sách Kỹ Năng Lướt (AC-3) */
	UFUNCTION(BlueprintCallable, Category = "Item|Setup")
	void SetupSkillBookDash();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

/**
 * FPAItemDataAssetPresets
 *
 * Tiện ích cấu hình tĩnh cho các DataAsset tiêu chuẩn (Story item-001).
 */
struct PROJECTASCENDANT_API FPAItemDataAssetPresets
{
	static void ConfigureWeaponIronSword(UItemStaticDataAsset* Asset);
	static void ConfigureArmorIronPlate(UItemStaticDataAsset* Asset);
	static void ConfigureArmorStarterCloth(UItemStaticDataAsset* Asset);
	static void ConfigureArmorLeatherRanger(UItemStaticDataAsset* Asset);
	static void ConfigureArmorArcanistRobe(UItemStaticDataAsset* Asset);
	static void ConfigurePotionHealth(UItemStaticDataAsset* Asset);
	static void ConfigureSkillBookDash(UItemStaticDataAsset* Asset);
};

