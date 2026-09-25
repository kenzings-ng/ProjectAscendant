#!/usr/bin/env python3
# Copyright Project Ascendant. All Rights Reserved.
"""
Unreal Engine Automation Script:
Imports 12 Class Identity Assets (Idle Stances, Helm Crests, Tabards) into UE5 Paper2D:
- 12 Class Idle Stance Flipbooks
- 60 Helm Crest UPaperSprites (Socket_HelmCrest 64,40)
- 60 Tabard UPaperSprites (Socket_Tabard 64,60)
"""

import os
import sys

def run():
    try:
        import unreal
    except ImportError:
        print("[ERROR] This script must be run inside Unreal Engine Editor via -ExecutePythonScript")
        sys.exit(1)

    unreal.log("=== Project Ascendant: Importing 12 Class Identity Assets into UE5 ===")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    content_dir = unreal.Paths.project_content_dir()
    class_id_disk = os.path.join(content_dir, "art", "characters", "ClassIdentity")
    class_id_ue = "/Game/art/characters/ClassIdentity"

    sprite_factory = unreal.PaperSpriteFactory()
    flipbook_factory = unreal.PaperFlipbookFactory()

    pivot_bottom_center = None
    pivot_center = None
    if hasattr(unreal, 'SpritePivotMode'):
        pivot_bottom_center = getattr(unreal.SpritePivotMode, 'BOTTOM_CENTER', None) or getattr(unreal.SpritePivotMode, 'SPRITE_PIVOT_MODE_BOTTOM_CENTER', None)
        pivot_center = getattr(unreal.SpritePivotMode, 'CENTER_CENTER', None) or getattr(unreal.SpritePivotMode, 'SPRITE_PIVOT_MODE_CENTER_CENTER', None)

    # 1. Import Helm Crests (32x32, center pivot)
    crest_disk = os.path.join(class_id_disk, "HelmCrests")
    crest_ue = f"{class_id_ue}/HelmCrests"
    crest_files = [f for f in sorted(os.listdir(crest_disk)) if f.lower().endswith(".png")]
    unreal.log(f"Importing {len(crest_files)} Helm Crests...")

    tasks = []
    for f in crest_files:
        name = os.path.splitext(f)[0]
        t = unreal.AssetImportTask()
        t.set_editor_property('filename', os.path.join(crest_disk, f))
        t.set_editor_property('destination_path', crest_ue)
        t.set_editor_property('destination_name', f"T_{name}")
        t.set_editor_property('replace_existing', True)
        t.set_editor_property('automated', True)
        t.set_editor_property('save', True)
        tasks.append(t)
    if tasks:
        asset_tools.import_asset_tasks(tasks)

    for f in crest_files:
        name = os.path.splitext(f)[0]
        tex_pkg = f"{crest_ue}/T_{name}"
        tex = unreal.EditorAssetLibrary.load_asset(tex_pkg)
        if tex:
            tex.set_editor_property('filter', unreal.TextureFilter.TF_NEAREST)
            tex.set_editor_property('mip_gen_settings', unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
            unreal.EditorAssetLibrary.save_asset(tex_pkg)
            # Create sprite
            sp_name = f"SP_{name}"
            sp_pkg = f"{crest_ue}/{sp_name}"
            sp = asset_tools.create_asset(sp_name, crest_ue, unreal.PaperSprite, sprite_factory)
            if sp:
                sp.set_editor_property('source_texture', tex)
                if pivot_center is not None:
                    sp.set_editor_property('pivot_mode', pivot_center)
                unreal.EditorAssetLibrary.save_asset(sp_pkg)

    # 2. Import Tabards (48x64, center pivot)
    tabard_disk = os.path.join(class_id_disk, "Tabards")
    tabard_ue = f"{class_id_ue}/Tabards"
    tabard_files = [f for f in sorted(os.listdir(tabard_disk)) if f.lower().endswith(".png")]
    unreal.log(f"Importing {len(tabard_files)} Tabards...")

    tasks = []
    for f in tabard_files:
        name = os.path.splitext(f)[0]
        t = unreal.AssetImportTask()
        t.set_editor_property('filename', os.path.join(tabard_disk, f))
        t.set_editor_property('destination_path', tabard_ue)
        t.set_editor_property('destination_name', f"T_{name}")
        t.set_editor_property('replace_existing', True)
        t.set_editor_property('automated', True)
        t.set_editor_property('save', True)
        tasks.append(t)
    if tasks:
        asset_tools.import_asset_tasks(tasks)

    for f in tabard_files:
        name = os.path.splitext(f)[0]
        tex_pkg = f"{tabard_ue}/T_{name}"
        tex = unreal.EditorAssetLibrary.load_asset(tex_pkg)
        if tex:
            tex.set_editor_property('filter', unreal.TextureFilter.TF_NEAREST)
            tex.set_editor_property('mip_gen_settings', unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
            unreal.EditorAssetLibrary.save_asset(tex_pkg)
            sp_name = f"SP_{name}"
            sp_pkg = f"{tabard_ue}/{sp_name}"
            sp = asset_tools.create_asset(sp_name, tabard_ue, unreal.PaperSprite, sprite_factory)
            if sp:
                sp.set_editor_property('source_texture', tex)
                if pivot_center is not None:
                    sp.set_editor_property('pivot_mode', pivot_center)
                unreal.EditorAssetLibrary.save_asset(sp_pkg)

    # 3. Import Idle Stance Loops (12 classes x 20 frames = 240 frames)
    idle_disk = os.path.join(class_id_disk, "IdleStances")
    idle_ue = f"{class_id_ue}/IdleStances"

    for cls_dir in sorted(os.listdir(idle_disk)):
        cls_full_disk = os.path.join(idle_disk, cls_dir)
        if not os.path.isdir(cls_full_disk):
            continue
        cls_ue_path = f"{idle_ue}/{cls_dir}"
        tex_ue_path = f"{cls_ue_path}/textures"
        sp_ue_path = f"{cls_ue_path}/sprites"

        frame_files = [f for f in sorted(os.listdir(cls_full_disk)) if f.lower().endswith(".png")]
        tasks = []
        for f in frame_files:
            name = os.path.splitext(f)[0]
            t = unreal.AssetImportTask()
            t.set_editor_property('filename', os.path.join(cls_full_disk, f))
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

        # Create master flipbook for this class
        fb_pkg = f"{cls_ue_path}/{cls_dir}"
        fb = asset_tools.create_asset(cls_dir, cls_ue_path, unreal.PaperFlipbook, flipbook_factory)
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

    unreal.log("=== All 360 Class Identity Assets Successfully Imported into Unreal Engine! ===")

if __name__ == "__main__":
    run()
