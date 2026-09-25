#!/usr/bin/env python3
# Copyright Project Ascendant. All Rights Reserved.
"""
Unreal Engine Automation Script:
Imports Master Lower Body Rig textures, configures pixel filtering (TF_NEAREST, TMGS_NO_MIPMAPS),
generates UPaperSprite assets, and builds UPaperFlipbook assets for:
- FB_Lower_HeavyTank_Set
- FB_Lower_Agility_Set
- FB_Lower_Caster_Set
- FB_Lower_Monk_Set
and their per-state/direction flipbooks in /Game/art/characters/MasterRigs.
"""

import os
import sys

def run():
    try:
        import unreal
    except ImportError:
        print("[ERROR] This script must be run inside Unreal Engine Editor via -ExecutePythonScript")
        sys.exit(1)

    unreal.log("=== Project Ascendant: Importing Master Lower Body Rigs into UE5 Paper2D/PaperZD ===")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    content_dir = unreal.Paths.project_content_dir()
    master_rigs_disk = os.path.join(content_dir, "art", "characters", "MasterRigs")
    master_rigs_ue = "/Game/art/characters/MasterRigs"

    rig_names = [
        "FB_Lower_HeavyTank_Set",
        "FB_Lower_Agility_Set",
        "FB_Lower_Caster_Set",
        "FB_Lower_Monk_Set"
    ]

    states = ["idle", "walk", "run", "dash", "hitstun"]
    directions = ["S", "SE", "E", "NE", "N"]

    sprite_factory = unreal.PaperSpriteFactory()
    flipbook_factory = unreal.PaperFlipbookFactory()

    # Determine pivot mode enum
    pivot_enum_val = None
    if hasattr(unreal, 'SpritePivotMode'):
        pivot_enum_val = getattr(unreal.SpritePivotMode, 'BOTTOM_CENTER', None)
        if pivot_enum_val is None:
            pivot_enum_val = getattr(unreal.SpritePivotMode, 'SPRITE_PIVOT_MODE_BOTTOM_CENTER', None)

    for rig_name in rig_names:
        unreal.log(f"--- Processing Rig: {rig_name} ---")
        rig_disk_dir = os.path.join(master_rigs_disk, rig_name)
        frames_disk_dir = os.path.join(rig_disk_dir, "frames")
        rig_ue_path = f"{master_rigs_ue}/{rig_name}"
        textures_ue_path = f"{rig_ue_path}/textures"
        sprites_ue_path = f"{rig_ue_path}/sprites"

        # 1. Gather all frames for this rig
        frame_files = []
        for root, dirs, files in os.walk(frames_disk_dir):
            for f in sorted(files):
                if f.lower().endswith(".png"):
                    frame_files.append(os.path.join(root, f))

        unreal.log(f"Found {len(frame_files)} frame PNGs for {rig_name}")

        # 2. Batch import textures
        import_tasks = []
        for file_path in frame_files:
            tex_name = os.path.splitext(os.path.basename(file_path))[0]
            t = unreal.AssetImportTask()
            t.set_editor_property('filename', file_path)
            t.set_editor_property('destination_path', textures_ue_path)
            t.set_editor_property('destination_name', tex_name)
            t.set_editor_property('replace_existing', True)
            t.set_editor_property('automated', True)
            t.set_editor_property('save', True)
            import_tasks.append(t)

        if import_tasks:
            asset_tools.import_asset_tasks(import_tasks)

        # 3. Configure pixel filtering (TF_NEAREST, TMGS_NO_MIPMAPS) & build UPaperSprite
        all_created_sprites = []
        for file_path in frame_files:
            tex_name = os.path.splitext(os.path.basename(file_path))[0]
            tex_pkg = f"{textures_ue_path}/{tex_name}"
            tex = unreal.EditorAssetLibrary.load_asset(tex_pkg)
            if tex:
                try:
                    tex.set_editor_property('filter', unreal.TextureFilter.TF_NEAREST)
                except Exception as e:
                    pass
                try:
                    tex.set_editor_property('mip_gen_settings', unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
                except Exception as e:
                    pass

                for group_attr in ['TEXTURE_GROUP_PIXELS2D', 'TEXTUREGROUP_PIXELS2D', 'TEXTURE_GROUP_PIXELS', 'TEXTUREGROUP_PIXELS', 'TEXTURE_GROUP_UI']:
                    if hasattr(unreal.TextureGroup, group_attr):
                        tex.set_editor_property('lod_group', getattr(unreal.TextureGroup, group_attr))
                        break

                unreal.EditorAssetLibrary.save_asset(tex_pkg)

                # Create UPaperSprite
                sprite_name = f"SP_{tex_name}"
                sprite_pkg = f"{sprites_ue_path}/{sprite_name}"
                sp = asset_tools.create_asset(sprite_name, sprites_ue_path, unreal.PaperSprite, sprite_factory)
                if sp:
                    sp.set_editor_property('source_texture', tex)
                    if pivot_enum_val is not None:
                        sp.set_editor_property('pivot_mode', pivot_enum_val)
                    unreal.EditorAssetLibrary.save_asset(sprite_pkg)
                    all_created_sprites.append((tex_name, sp))

        unreal.log(f"Created {len(all_created_sprites)} UPaperSprite assets for {rig_name}")

        # 4. Create Master Set Flipbook (contains all 105 frames for the rig)
        master_fb_pkg = f"{master_rigs_ue}/{rig_name}"
        master_fb = asset_tools.create_asset(rig_name, master_rigs_ue, unreal.PaperFlipbook, flipbook_factory)
        if master_fb:
            key_frames = []
            for _, sprite in all_created_sprites:
                kf = unreal.PaperFlipbookKeyFrame()
                kf.set_editor_property('sprite', sprite)
                kf.set_editor_property('frame_run', 1)
                key_frames.append(kf)
            master_fb.set_editor_property('key_frames', key_frames)
            master_fb.set_editor_property('frames_per_second', 12.0)
            unreal.EditorAssetLibrary.save_asset(master_fb_pkg)
            unreal.log(f"Created Master Flipbook: {master_fb_pkg} with {len(key_frames)} key frames")

        # 5. Create per-state/direction Flipbooks (e.g. FB_Lower_HeavyTank_Idle_S)
        for state in states:
            for direction in directions:
                prefix = f"{rig_name}_{state}_{direction}"
                matching_sprites = [sp for name, sp in all_created_sprites if prefix in name]
                if matching_sprites:
                    fb_sub_name = f"FB_{rig_name.replace('FB_Lower_', '').replace('_Set', '')}_{state.capitalize()}_{direction}"
                    sub_fb_pkg = f"{rig_ue_path}/{fb_sub_name}"
                    sub_fb = asset_tools.create_asset(fb_sub_name, rig_ue_path, unreal.PaperFlipbook, flipbook_factory)
                    if sub_fb:
                        kfs = []
                        for sp in matching_sprites:
                            kf = unreal.PaperFlipbookKeyFrame()
                            kf.set_editor_property('sprite', sp)
                            kf.set_editor_property('frame_run', 1)
                            kfs.append(kf)
                        fps = 8.0 if state in ("idle", "hitstun") else 12.0
                        sub_fb.set_editor_property('key_frames', kfs)
                        sub_fb.set_editor_property('frames_per_second', fps)
                        unreal.EditorAssetLibrary.save_asset(sub_fb_pkg)

    unreal.log("=== All 4 Master Lower Body Rigs Successfully Imported into Unreal Engine! ===")


if __name__ == "__main__":
    run()
