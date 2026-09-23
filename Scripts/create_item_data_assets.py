import unreal
import os

def create_or_update_data_asset(package_path, asset_name, setup_fn):
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    full_path = f"{package_path}/{asset_name}"
    
    asset = unreal.EditorAssetLibrary.load_asset(full_path)
    if not asset:
        unreal.log(f"Creating new ItemStaticDataAsset at {full_path}")
        factory = unreal.DataAssetFactory()
        factory.set_editor_property('data_asset_class', unreal.ItemStaticDataAsset)
        asset = asset_tools.create_asset(asset_name, package_path, unreal.ItemStaticDataAsset, factory)
    else:
        unreal.log(f"Updating existing ItemStaticDataAsset at {full_path}")
        
    if not asset:
        unreal.log_error(f"Failed to create/load asset at {full_path}")
        return None
        
    setup_fn(asset)
    unreal.EditorAssetLibrary.save_asset(full_path)
    unreal.log(f"Successfully saved {full_path}")
    return asset

def run():
    unreal.log("=== Project Ascendant: Creating Item DataAssets (Story item-001) ===")
    
    package_path = "/Game/Items/DataAssets"
    
    # 1. AC-1: DA_Weapon_IronSword
    create_or_update_data_asset(package_path, "DA_Weapon_IronSword", lambda a: a.setup_weapon_iron_sword())

    # 2. AC-1: DA_Armor_IronPlate
    create_or_update_data_asset(package_path, "DA_Armor_IronPlate", lambda a: a.setup_armor_iron_plate())

    # 3. AC-2: DA_Potion_Health
    create_or_update_data_asset(package_path, "DA_Potion_Health", lambda a: a.setup_potion_health())

    # 4. AC-3: DA_SkillBook_Dash
    create_or_update_data_asset(package_path, "DA_SkillBook_Dash", lambda a: a.setup_skill_book_dash())

    unreal.log("=== Project Ascendant: All 4 Item DataAssets Created Successfully ===")

if __name__ == "__main__":
    run()
