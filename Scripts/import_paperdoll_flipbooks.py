# Copyright Project Ascendant. All Rights Reserved.
"""
Unreal Engine Editor Automation Script:
Imports Paperdoll textures, configures pixel filtering (Nearest Neighbor / No Mipmaps),
and creates Paper2D Flipbooks and Item DataAssets.
"""

import os

def run():
    try:
        import unreal
    except ImportError:
        print("[ERROR] This script must be run inside Unreal Engine Editor via -ExecutePythonScript")
        return

    unreal.log("=== Project Ascendant: Importing Paperdoll Textures & Flipbooks ===")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    content_dir = unreal.Paths.project_content_dir()

    paperdoll_dir = os.path.join(content_dir, "art/characters/paperdoll")
    dest_path = "/Game/art/characters/paperdoll"

    textures = [
        ("T_Hero_StarterCloth.png", "T_Hero_StarterCloth"),
        ("T_Hero_IronArmor.png", "T_Hero_IronArmor"),
        ("T_Hero_LeatherRanger.png", "T_Hero_LeatherRanger"),
        ("T_Hero_ArcanistRobe.png", "T_Hero_ArcanistRobe"),
    ]

    # 1. Import Texture Tasks
    tasks = []
    for file_name, asset_name in textures:
        full_file_path = os.path.join(paperdoll_dir, file_name)
        if not os.path.exists(full_file_path):
            unreal.log_warning(f"File not found: {full_file_path}")
            continue

        task = unreal.AssetImportTask()
        task.set_editor_property('filename', full_file_path)
        task.set_editor_property('destination_path', dest_path)
        task.set_editor_property('destination_name', asset_name)
        task.set_editor_property('replace_existing', True)
        task.set_editor_property('automated', True)
        task.set_editor_property('save', True)
        tasks.append(task)

    if tasks:
        asset_tools.import_asset_tasks(tasks)
        unreal.log(f"Imported {len(tasks)} Paperdoll textures.")

    # 2. Configure Pixel Filtering (TF_NEAREST, TMGS_NO_MIPMAPS, TEXTURE_GROUP_PIXELS)
    for _, asset_name in textures:
        tex_path = f"{dest_path}/{asset_name}"
        tex = unreal.EditorAssetLibrary.load_asset(tex_path)
        if tex:
            try:
                tex.set_editor_property('filter', unreal.TextureFilter.TF_NEAREST)
            except Exception as e:
                unreal.log_warning(f"Filter set warning for {asset_name}: {e}")

            try:
                tex.set_editor_property('mip_gen_settings', unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
            except Exception as e:
                unreal.log_warning(f"Mip gen warning for {asset_name}: {e}")

            for group_attr in ['TEXTURE_GROUP_PIXELS', 'TEXTUREGROUP_PIXELS', 'TEXTURE_GROUP_UI']:
                if hasattr(unreal.TextureGroup, group_attr):
                    tex.set_editor_property('lod_group', getattr(unreal.TextureGroup, group_attr))
                    break

            unreal.EditorAssetLibrary.save_asset(tex_path)
            unreal.log(f"Configured pixel filter Nearest & NoMipmaps for: {tex_path}")

    # 3. Create PaperFlipbook Assets
    flipbook_factory = unreal.PaperFlipbookFactory()
    flipbook_configs = [
        (dest_path, "FB_Hero_StarterCloth", 12.0),
        (dest_path, "FB_Hero_IronArmor", 12.0),
        (dest_path, "FB_Hero_LeatherRanger", 12.0),
        (dest_path, "FB_Hero_ArcanistRobe", 12.0),
    ]

    for pkg_dir, fb_name, fps in flipbook_configs:
        pkg_path = f"{pkg_dir}/{fb_name}"
        fb_asset = asset_tools.create_asset(fb_name, pkg_dir, unreal.PaperFlipbook, flipbook_factory)
        if fb_asset:
            fb_asset.set_editor_property('frames_per_second', fps)
            unreal.EditorAssetLibrary.save_asset(pkg_path)
            unreal.log(f"Created Flipbook: {pkg_path} @ {fps} FPS")

    # 4. Create / Update Item DataAssets
    item_pkg = "/Game/Items/DataAssets"
    data_asset_presets = [
        ("DA_Armor_StarterCloth", lambda a: a.setup_armor_starter_cloth()),
        ("DA_Armor_LeatherRanger", lambda a: a.setup_armor_leather_ranger()),
        ("DA_Armor_ArcanistRobe", lambda a: a.setup_armor_arcanist_robe()),
    ]

    for da_name, setup_fn in data_asset_presets:
        full_da_path = f"{item_pkg}/{da_name}"
        da_asset = unreal.EditorAssetLibrary.load_asset(full_da_path)
        if not da_asset:
            da_factory = unreal.DataAssetFactory()
            if hasattr(unreal, 'ItemStaticDataAsset'):
                da_factory.set_editor_property('data_asset_class', unreal.ItemStaticDataAsset)
                da_asset = asset_tools.create_asset(da_name, item_pkg, unreal.ItemStaticDataAsset, da_factory)
        if da_asset:
            setup_fn(da_asset)
            unreal.EditorAssetLibrary.save_asset(full_da_path)
            unreal.log(f"Created/Updated Item DataAsset: {full_da_path}")

    unreal.log("=== Paperdoll Assets & DataAssets Setup Complete! ===")

if __name__ == "__main__":
    run()
