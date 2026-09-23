// Copyright Project Ascendant. All Rights Reserved.

#include "Inventory/PAItemStaticDataAsset.h"
#include "Inventory/GE_Item_IronSword.h"
#include "Inventory/GE_Item_IronPlate.h"
#include "Combat/AscendantAttributeSet.h"
#include "Combat/PAGameplayAbility_Dash.h"
#include "GameplayTagsManager.h"

UItemStaticDataAsset::UItemStaticDataAsset()
	: ItemId(NAME_None)
	, ItemName(FText::FromString(TEXT("Unknown Item")))
	, ItemDescription(FText::GetEmpty())
	, RarityTier(EPAItemRarity::Common)
	, Category(EPAItemCategory::Material)
	, MaxStackSize(999)
	, BaseSellPrice(1)
	, AllowedEquipmentSlot(EPAEquipmentSlot::None)
	, BasePhysicalDamageBonus(0.0f)
	, BaseArmorBonus(0.0f)
	, PreferredQuickbarSlot(EPAQuickbarSlot::None)
	, ConsumableDuration(0.8f)
	, ConsumableHealthRestore(150.0f)
	, ConsumableSpeedDebuffRatio(0.30f)
{
}

int32 UItemStaticDataAsset::GetDefaultMaxStackForCategory(EPAItemCategory InCategory)
{
	switch (InCategory)
	{
	case EPAItemCategory::Equipment:
	case EPAItemCategory::SkillBook:
	case EPAItemCategory::Quest:
		return 1; // AC-3: Equipment và Skill Book không cộng dồn (max stack 1)
	case EPAItemCategory::Consumable:
		return 20; // AC-3: Consumables cộng dồn tối đa 20
	case EPAItemCategory::Material:
		return 999; // AC-3: Materials và Shards cộng dồn tối đa 999
	default:
		return 1;
	}
}

FPrimaryAssetId UItemStaticDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("Item"), ItemId.IsNone() ? GetFName() : ItemId);
}

// -----------------------------------------------------------------------------
// Core Setup Presets Implementation (Story item-001)
// -----------------------------------------------------------------------------

void FPAItemDataAssetPresets::ConfigureWeaponIronSword(UItemStaticDataAsset* Asset)
{
	if (!Asset) return;
	Asset->ItemId = FName(TEXT("Item_Weapon_IronSword"));
	Asset->ItemName = FText::FromString(TEXT("Iron Sword"));
	Asset->ItemDescription = FText::FromString(TEXT("A standard vanguard iron sword delivering +15 physical attack power."));
	Asset->RarityTier = EPAItemRarity::Uncommon;
	Asset->Category = EPAItemCategory::Equipment;
	Asset->AllowedEquipmentSlot = EPAEquipmentSlot::Mainhand;
	Asset->BasePhysicalDamageBonus = 15.0f;
	Asset->BaseArmorBonus = 0.0f;
	Asset->MaxStackSize = 1;
	Asset->BaseSellPrice = 50;
	Asset->EquipGameplayEffect = UGE_Item_IronSword::StaticClass();
}

void FPAItemDataAssetPresets::ConfigureArmorIronPlate(UItemStaticDataAsset* Asset)
{
	if (!Asset) return;
	Asset->ItemId = FName(TEXT("Item_Armor_IronPlate"));
	Asset->ItemName = FText::FromString(TEXT("Iron Plate Armor"));
	Asset->ItemDescription = FText::FromString(TEXT("Heavy iron body plate armor providing +40 defense."));
	Asset->RarityTier = EPAItemRarity::Rare;
	Asset->Category = EPAItemCategory::Equipment;
	Asset->AllowedEquipmentSlot = EPAEquipmentSlot::BodyArmor;
	Asset->BasePhysicalDamageBonus = 0.0f;
	Asset->BaseArmorBonus = 40.0f;
	Asset->MaxStackSize = 1;
	Asset->BaseSellPrice = 120;
	Asset->EquipGameplayEffect = UGE_Item_IronPlate::StaticClass();
}

void FPAItemDataAssetPresets::ConfigurePotionHealth(UItemStaticDataAsset* Asset)
{
	if (!Asset) return;
	Asset->ItemId = FName(TEXT("Item_Potion_Health"));
	Asset->ItemName = FText::FromString(TEXT("Health Potion"));
	Asset->ItemDescription = FText::FromString(TEXT("Restores 100 Health over 0.8 seconds. Slows movement by 30% while consuming."));
	Asset->RarityTier = EPAItemRarity::Common;
	Asset->Category = EPAItemCategory::Consumable;
	Asset->PreferredQuickbarSlot = EPAQuickbarSlot::Quickbar_1;
	Asset->ConsumableDuration = 0.8f;
	Asset->ConsumableSpeedDebuffRatio = 0.30f;
	Asset->ConsumableHealthRestore = 100.0f;
	Asset->MaxStackSize = 20;
	Asset->BaseSellPrice = 15;
}

void FPAItemDataAssetPresets::ConfigureSkillBookDash(UItemStaticDataAsset* Asset)
{
	if (!Asset) return;
	Asset->ItemId = FName(TEXT("Item_SkillBook_Dash"));
	Asset->ItemName = FText::FromString(TEXT("Skill Tome: Vanguard Dash"));
	Asset->ItemDescription = FText::FromString(TEXT("Teaches the evasive Dash maneuver to Vanguard warriors."));
	Asset->RarityTier = EPAItemRarity::Rare;
	Asset->Category = EPAItemCategory::SkillBook;
	Asset->RequiredClassTag = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TEXT("Class.Vanguard")), TEXT("Vanguard warrior class"));
	Asset->GrantedAbilityClass = UPAGameplayAbility_Dash::StaticClass();
	Asset->MaxStackSize = 1;
	Asset->BaseSellPrice = 250;
}

void UItemStaticDataAsset::SetupWeaponIronSword()
{
	FPAItemDataAssetPresets::ConfigureWeaponIronSword(this);
}

void UItemStaticDataAsset::SetupArmorIronPlate()
{
	FPAItemDataAssetPresets::ConfigureArmorIronPlate(this);
}

void UItemStaticDataAsset::SetupPotionHealth()
{
	FPAItemDataAssetPresets::ConfigurePotionHealth(this);
}

void UItemStaticDataAsset::SetupSkillBookDash()
{
	FPAItemDataAssetPresets::ConfigureSkillBookDash(this);
}


