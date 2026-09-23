import unreal
import os

def verify_paperzd_assets():
    """
    Project Ascendant - PaperZD Asset Verification Script
    Verifies that AS_Vanguard and ABP_Vanguard exist and are properly registered in the Asset Registry.
    """
    unreal.log("=== Project Ascendant: Verifying PaperZD Assets ===")
    
    anim_package_path = "/Game/art/characters/vanguard/anim"
    as_path = f"{anim_package_path}/AS_Vanguard"
    abp_path = f"{anim_package_path}/ABP_Vanguard"
    
    as_exists = unreal.EditorAssetLibrary.does_asset_exist(as_path)
    abp_exists = unreal.EditorAssetLibrary.does_asset_exist(abp_path)
    
    unreal.log(f"AS_Vanguard Exists: {as_exists}")
    unreal.log(f"ABP_Vanguard Exists: {abp_exists}")
    
    if as_exists and abp_exists:
        unreal.log("=== PaperZD Assets Verification PASSED ===")
        return True
    else:
        unreal.log_error("=== PaperZD Assets Verification FAILED ===")
        return False

if __name__ == "__main__":
    verify_paperzd_assets()
