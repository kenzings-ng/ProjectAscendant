import unreal
import os

def create_boss_paperzd_assets():
    unreal.log("=== Project Ascendant: Creating Boss PaperZD Assets ===")
    
    source_as = "/Game/art/characters/vanguard/anim/AS_Vanguard"
    source_abp = "/Game/art/characters/vanguard/anim/ABP_Vanguard"
    
    target_as = "/Game/art/characters/boss/anim/AS_StoneGolem"
    target_abp = "/Game/art/characters/boss/anim/ABP_StoneGolem"
    
    asset_lib = unreal.EditorAssetLibrary
    
    # 1. Duplicate Animation Source
    if not asset_lib.does_asset_exist(target_as):
        unreal.log(f"Duplicating {source_as} -> {target_as}")
        success_as = asset_lib.duplicate_asset(source_as, target_as)
        unreal.log(f"Duplicate AS result: {success_as}")
    else:
        unreal.log(f"Target AS already exists: {target_as}")
        
    # 2. Duplicate Animation Blueprint
    if not asset_lib.does_asset_exist(target_abp):
        unreal.log(f"Duplicating {source_abp} -> {target_abp}")
        success_abp = asset_lib.duplicate_asset(source_abp, target_abp)
        unreal.log(f"Duplicate ABP result: {success_abp}")
    else:
        unreal.log(f"Target ABP already exists: {target_abp}")
        
    # Save assets
    asset_lib.save_asset(target_as)
    asset_lib.save_asset(target_abp)
    
    unreal.log("=== Project Ascendant: Boss PaperZD Assets Created Successfully ===")
    return True

if __name__ == "__main__":
    create_boss_paperzd_assets()
