import unreal
import os
import sys

def import_and_configure_textures():
    unreal.log("=== Project Ascendant: Importing and Configuring Class Textures ===")
    
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    content_dir = unreal.Paths.project_content_dir()
    
    # Textures to import
    ranger_png = os.path.join(content_dir, "art/characters/ranger_pixel_spritesheet.png")
    arcanist_png = os.path.join(content_dir, "art/characters/arcanist_pixel_spritesheet.png")
    golem_png = os.path.join(content_dir, "art/characters/boss/spine/stone_golem.png")
    boss_spritesheet_png = os.path.join(content_dir, "art/characters/stone_golem_boss_pixel_spritesheet.png")
    
    import_specs = [
        (ranger_png, "/Game/art/characters", "ranger_pixel_spritesheet"),
        (arcanist_png, "/Game/art/characters", "arcanist_pixel_spritesheet"),
        (golem_png, "/Game/art/characters/boss/spine", "stone_golem"),
        (boss_spritesheet_png, "/Game/art/characters", "T_Boss_Spritesheet")
    ]
    
    tasks = []
    for file_path, dest_path, dest_name in import_specs:
        if not os.path.exists(file_path):
            unreal.log_error(f"Source file not found: {file_path}")
            continue
            
        t = unreal.AssetImportTask()
        t.set_editor_property('filename', file_path)
        t.set_editor_property('destination_path', dest_path)
        t.set_editor_property('destination_name', dest_name)
        t.set_editor_property('replace_existing', True)
        t.set_editor_property('automated', True)
        t.set_editor_property('save', True)
        tasks.append(t)
        
    if tasks:
        asset_tools.import_asset_tasks(tasks)
        unreal.log("Texture import tasks completed.")
        
    # Configure and verify all character/boss textures for Pixel Art (Nearest & No Mipmaps)
    assets_to_configure = [
        "/Game/art/characters/T_Vanguard_Spritesheet.T_Vanguard_Spritesheet",
        "/Game/art/characters/T_Boss_Spritesheet.T_Boss_Spritesheet",
        "/Game/art/characters/ranger_pixel_spritesheet.ranger_pixel_spritesheet",
        "/Game/art/characters/arcanist_pixel_spritesheet.arcanist_pixel_spritesheet",
        "/Game/art/characters/boss/spine/stone_golem.stone_golem"
    ]
    
    for asset_path in assets_to_configure:
        tex = unreal.load_asset(asset_path)
        if not tex:
            unreal.log_warning(f"Could not load asset: {asset_path}")
            continue
            
        # Configure Nearest filtering
        try:
            tex.set_editor_property('filter', unreal.TextureFilter.TF_NEAREST)
        except Exception as e:
            unreal.log_warning(f"Failed to set filter on {asset_path}: {e}")
            
        # Configure No Mipmaps
        try:
            tex.set_editor_property('mip_gen_settings', unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        except Exception as e:
            unreal.log_warning(f"Failed to set mip_gen_settings on {asset_path}: {e}")
            
        # Set LOD Group to Pixels
        for group_attr in ['TEXTURE_GROUP_PIXELS', 'TEXTUREGROUP_PIXELS', 'TEXTURE_GROUP_UI']:
            if hasattr(unreal.TextureGroup, group_attr):
                try:
                    tex.set_editor_property('lod_group', getattr(unreal.TextureGroup, group_attr))
                    break
                except Exception:
                    pass
                    
        unreal.EditorAssetLibrary.save_loaded_asset(tex)
        
        # Verify
        curr_filter = tex.get_editor_property('filter')
        curr_mips = tex.get_editor_property('mip_gen_settings')
        unreal.log(f"[VERIFIED] {asset_path} -> Filter: {curr_filter}, MipGenSettings: {curr_mips}")
        
    unreal.log("=== Class Texture Import & Configuration Complete! ===")

if __name__ == "__main__":
    import_and_configure_textures()
