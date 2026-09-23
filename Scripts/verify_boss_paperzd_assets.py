import unreal

def verify_boss_paperzd_assets():
    unreal.log("=== Project Ascendant: Verifying Boss PaperZD Assets ===")
    
    anim_package_path = "/Game/art/characters/boss/anim"
    as_path = f"{anim_package_path}/AS_StoneGolem"
    abp_path = f"{anim_package_path}/ABP_StoneGolem"
    
    as_exists = unreal.EditorAssetLibrary.does_asset_exist(as_path)
    abp_exists = unreal.EditorAssetLibrary.does_asset_exist(abp_path)
    
    unreal.log(f"AS_StoneGolem Exists: {as_exists}")
    unreal.log(f"ABP_StoneGolem Exists: {abp_exists}")
    
    # Also verify all 5 boss flipbooks exist
    flipbook_dir = "/Game/art/characters/boss/flipbooks"
    required_flipbooks = [
        "FB_Golem_Idle",
        "FB_Golem_Walk",
        "FB_Golem_Slam",
        "FB_Golem_Stagger",
        "FB_Golem_Death"
    ]
    
    all_fb_exist = True
    for fb in required_flipbooks:
        fb_path = f"{flipbook_dir}/{fb}"
        exists = unreal.EditorAssetLibrary.does_asset_exist(fb_path)
        unreal.log(f"Flipbook {fb} Exists: {exists}")
        if not exists:
            all_fb_exist = False
            
    if as_exists and abp_exists and all_fb_exist:
        unreal.log("=== ALL BOSS PAPERZD ASSETS VERIFICATION PASSED ===")
        return True
    else:
        unreal.log_error("=== BOSS PAPERZD ASSETS VERIFICATION FAILED ===")
        return False

if __name__ == "__main__":
    verify_boss_paperzd_assets()
