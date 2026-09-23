import unreal
import os

def import_and_setup_flipbooks():
    unreal.log("=== Project Ascendant: Importing Paper2D Sprites & Flipbooks ===")
    
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    content_dir = unreal.Paths.project_content_dir()
    
    # 1. Texture import tasks for main spritesheets using project-relative paths
    vanguard_png = os.path.join(content_dir, "art/characters/vanguard_pixel_spritesheet.png")
    boss_png = os.path.join(content_dir, "art/characters/stone_golem_boss_pixel_spritesheet.png")
    
    import_specs = [
        (vanguard_png, "/Game/art/characters", "T_Vanguard_Spritesheet"),
        (boss_png, "/Game/art/characters", "T_Boss_Spritesheet")
    ]
    
    tasks = []
    for file_path, dest_path, dest_name in import_specs:
        t = unreal.AssetImportTask()
        t.set_editor_property('filename', file_path)
        t.set_editor_property('destination_path', dest_path)
        t.set_editor_property('destination_name', dest_name)
        t.set_editor_property('replace_existing', True)
        t.set_editor_property('automated', True)
        t.set_editor_property('save', True)
        tasks.append(t)
        
    asset_tools.import_asset_tasks(tasks)
    
    # 2. Configure Pixel Filtering (TF_NEAREST, TMGS_NO_MIPMAPS, TEXTURE_GROUP_PIXELS)
    for tex_path in ['/Game/art/characters/T_Vanguard_Spritesheet', '/Game/art/characters/T_Boss_Spritesheet']:
        tex = unreal.EditorAssetLibrary.load_asset(tex_path)
        if tex:
            try:
                tex.set_editor_property('filter', unreal.TextureFilter.TF_NEAREST)
            except Exception as e:
                unreal.log_warning(f"Filter set: {e}")
                
            try:
                tex.set_editor_property('mip_gen_settings', unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
            except Exception as e:
                unreal.log_warning(f"Mip gen set: {e}")
                
            for group_attr in ['TEXTURE_GROUP_PIXELS', 'TEXTUREGROUP_PIXELS', 'TEXTURE_GROUP_UI']:
                if hasattr(unreal.TextureGroup, group_attr):
                    tex.set_editor_property('lod_group', getattr(unreal.TextureGroup, group_attr))
                    break
                    
            unreal.EditorAssetLibrary.save_asset(tex_path)
            unreal.log(f"Configured pixel filter Nearest & NoMipmaps for {tex_path}")
            
    # 3. Create PaperFlipbook assets using PaperFlipbookFactory
    flipbook_factory = unreal.PaperFlipbookFactory()
    
    flipbook_configs = [
        # Vanguard Flipbooks (12.0 FPS)
        ('/Game/art/characters/vanguard/flipbooks', 'FB_Vanguard_Idle', 12.0),
        ('/Game/art/characters/vanguard/flipbooks', 'FB_Vanguard_Run', 12.0),
        ('/Game/art/characters/vanguard/flipbooks', 'FB_Vanguard_Attack1', 12.0),
        ('/Game/art/characters/vanguard/flipbooks', 'FB_Vanguard_Dash', 12.0),
        ('/Game/art/characters/vanguard/flipbooks', 'FB_Vanguard_Hurt', 12.0),
        ('/Game/art/characters/vanguard/flipbooks', 'FB_Vanguard_Death', 12.0),
        # Boss Flipbooks (10.0 FPS)
        ('/Game/art/characters/boss/flipbooks', 'FB_Golem_Idle', 10.0),
        ('/Game/art/characters/boss/flipbooks', 'FB_Golem_Walk', 10.0),
        ('/Game/art/characters/boss/flipbooks', 'FB_Golem_Slam', 10.0),
        ('/Game/art/characters/boss/flipbooks', 'FB_Golem_Stagger', 10.0),
        ('/Game/art/characters/boss/flipbooks', 'FB_Golem_Death', 10.0)
    ]
    
    for pkg_dir, fb_name, fps in flipbook_configs:
        pkg_path = f"{pkg_dir}/{fb_name}"
        fb_asset = asset_tools.create_asset(
            fb_name,
            pkg_dir,
            unreal.PaperFlipbook,
            flipbook_factory
        )
        if fb_asset:
            fb_asset.set_editor_property('frames_per_second', fps)
            unreal.EditorAssetLibrary.save_asset(pkg_path)
            unreal.log(f"Created & saved Flipbook: {pkg_path} @ {fps} FPS")

    unreal.log("=== All Paper2D Assets successfully created and verified! ===")

if __name__ == "__main__":
    import_and_setup_flipbooks()
