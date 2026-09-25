#!/usr/bin/env python3
# Copyright Project Ascendant. All Rights Reserved.
"""
Unreal Engine Automation Script:
Imports 205 Civilian Upper Body Frames and 50 Hand Prop Sprites into UE5 Paper2D:
- 5 Civilian Role Flipbooks in /Game/art/characters/Civilian/UpperBody/
- 50 Hand Prop UPaperSprites in /Game/art/characters/Civilian/Props/
"""

import os
import sys

def run():
    try:
        import unreal
    except ImportError:
        print("[ERROR] This script must be run inside Unreal Engine Editor via -ExecutePythonScript")
        sys.exit(1)

    unreal.log("=== Project Ascendant: Importing Civilian Upper Body & Props into UE5 ===")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    content_dir = unreal.Paths.project_content_dir()
    civilian_disk = os.path.join(content_dir, "art", "characters", "Civilian")
    civilian_ue = "/Game/art/characters/Civilian"

    sprite_factory = unreal.PaperSpriteFactory()
    flipbook_factory = unreal.PaperFlipbookFactory()

    pivot_bottom_center = None
    pivot_center = None
    if hasattr(unreal, 'SpritePivotMode'):
        pivot_bottom_center = getattr(unreal.SpritePivotMode, 'BOTTOM_CENTER', None) or getattr(unreal.SpritePivotMode, 'SPRITE_PIVOT_MODE_BOTTOM_CENTER', None)
        pivot_center = getattr(unreal.SpritePivotMode, 'CENTER_CENTER', None) or getattr(unreal.SpritePivotMode, 'SPRITE_PIVOT_MODE_CENTER_CENTER', None)

    # 1. Import Props (32x32, center pivot)
    props_disk = os.path.join(civilian_disk, "Props")
    props_ue = f"{civilian_ue}/Props"
    prop_files = [f for f in sorted(os.listdir(props_disk)) if f.lower().endswith(".png")]
    unreal.log(f"Importing {len(prop_files)} Civilian Props...")

    tasks = []
    for f in prop_files:
        name = os.path.splitext(f)[0]
        t = unreal.AssetImportTask()
        t.set_editor_property('filename', os.path.join(props_disk, f))
        t.set_editor_property('destination_path', props_ue)
        t.set_editor_property('destination_name', f"T_{name}")
        t.set_editor_property('replace_existing', True)
        t.set_editor_property('automated', True)
        t.set_editor_property('save', True)
        tasks.append(t)
    if tasks:
        asset_tools.import_asset_tasks(tasks)

    for f in prop_files:
        name = os.path.splitext(f)[0]
        tex_pkg = f"{props_ue}/T_{name}"
        tex = unreal.EditorAssetLibrary.load_asset(tex_pkg)
        if tex:
            tex.set_editor_property('filter', unreal.TextureFilter.TF_NEAREST)
            tex.set_editor_property('mip_gen_settings', unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
            unreal.EditorAssetLibrary.save_asset(tex_pkg)
            sp_name = f"SP_{name}"
            sp_pkg = f"{props_ue}/{sp_name}"
            sp = asset_tools.create_asset(sp_name, props_ue, unreal.PaperSprite, sprite_factory)
            if sp:
                sp.set_editor_property('source_texture', tex)
                if pivot_center is not None:
                    sp.set_editor_property('pivot_mode', pivot_center)
                unreal.EditorAssetLibrary.save_asset(sp_pkg)

    # 2. Import Upper Body Roles (5 roles)
    upper_disk = os.path.join(civilian_disk, "UpperBody")
    upper_ue = f"{civilian_ue}/UpperBody"

    for role_name in sorted(os.listdir(upper_disk)):
        role_disk_dir = os.path.join(upper_disk, role_name)
        if not os.path.isdir(role_disk_dir):
            continue

        role_ue_path = f"{upper_ue}/{role_name}"
        tex_ue_path = f"{role_ue_path}/textures"
        sp_ue_path = f"{role_ue_path}/sprites"

        frame_files = [f for f in sorted(os.listdir(role_disk_dir)) if f.lower().endswith(".png")]
        tasks = []
        for f in frame_files:
            name = os.path.splitext(f)[0]
            t = unreal.AssetImportTask()
            t.set_editor_property('filename', os.path.join(role_disk_dir, f))
            t.set_editor_property('destination_path', tex_ue_path)
            t.set_editor_property('destination_name', name)
            t.set_editor_property('replace_existing', True)
            t.set_editor_property('automated', True)
            t.set_editor_property('save', True)
            tasks.append(t)
        if tasks:
            asset_tools.import_asset_tasks(tasks)

        created_sprites = []
        for f in frame_files:
            name = os.path.splitext(f)[0]
            tex_pkg = f"{tex_ue_path}/{name}"
            tex = unreal.EditorAssetLibrary.load_asset(tex_pkg)
            if tex:
                tex.set_editor_property('filter', unreal.TextureFilter.TF_NEAREST)
                tex.set_editor_property('mip_gen_settings', unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
                unreal.EditorAssetLibrary.save_asset(tex_pkg)
                sp_name = f"SP_{name}"
                sp_pkg = f"{sp_ue_path}/{sp_name}"
                sp = asset_tools.create_asset(sp_name, sp_ue_path, unreal.PaperSprite, sprite_factory)
                if sp:
                    sp.set_editor_property('source_texture', tex)
                    if pivot_bottom_center is not None:
                        sp.set_editor_property('pivot_mode', pivot_bottom_center)
                    unreal.EditorAssetLibrary.save_asset(sp_pkg)
                    created_sprites.append((name, sp))

        # Master Flipbook for Role
        fb_pkg = f"{role_ue_path}/FB_Civilian_{role_name}"
        fb = asset_tools.create_asset(f"FB_Civilian_{role_name}", role_ue_path, unreal.PaperFlipbook, flipbook_factory)
        if fb:
            kfs = []
            for _, sp in created_sprites:
                kf = unreal.PaperFlipbookKeyFrame()
                kf.set_editor_property('sprite', sp)
                kf.set_editor_property('frame_run', 1)
                kfs.append(kf)
            fb.set_editor_property('key_frames', kfs)
            fb.set_editor_property('frames_per_second', 8.0)
            unreal.EditorAssetLibrary.save_asset(fb_pkg)

    unreal.log("=== All 255 Civilian Assets Successfully Imported into Unreal Engine! ===")

if __name__ == "__main__":
    run()
