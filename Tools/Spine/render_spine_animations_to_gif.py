#!/usr/bin/env python3
# ==============================================================================
# render_spine_animations_to_gif.py
# Renders Spine 4.3 animations (idle, slam, walk) using the REAL Master Artwork parts!
# ==============================================================================

import os
import sys
import json
import math
from PIL import Image

SPINE_DIR = "Content/art/characters/boss/spine"
PARTS_DIR = os.path.join(SPINE_DIR, "master_parts")
JSON_PATH = os.path.join(SPINE_DIR, "stone_golem.json")

def lerp(a, b, t):
    return a + (b - a) * t

def sample_timeline(keys, t, val_key="value", default=0.0):
    if not keys:
        return default
    if len(keys) == 1 or t <= keys[0]["time"]:
        return keys[0].get(val_key, default)
    if t >= keys[-1]["time"]:
        return keys[-1].get(val_key, default)
    
    for i in range(len(keys) - 1):
        k0 = keys[i]
        k1 = keys[i + 1]
        if k0["time"] <= t <= k1["time"]:
            dur = k1["time"] - k0["time"]
            fraction = (t - k0["time"]) / dur if dur > 0 else 0
            v0 = k0.get(val_key, default)
            v1 = k1.get(val_key, default)
            return lerp(v0, v1, fraction)
    return default

def sample_vec2_timeline(keys, t, default_x=0.0, default_y=0.0):
    if not keys:
        return default_x, default_y
    if len(keys) == 1 or t <= keys[0]["time"]:
        return keys[0].get("x", default_x), keys[0].get("y", default_y)
    if t >= keys[-1]["time"]:
        return keys[-1].get("x", default_x), keys[-1].get("y", default_y)
        
    for i in range(len(keys) - 1):
        k0 = keys[i]
        k1 = keys[i + 1]
        if k0["time"] <= t <= k1["time"]:
            dur = k1["time"] - k0["time"]
            fraction = (t - k0["time"]) / dur if dur > 0 else 0
            x0, y0 = k0.get("x", default_x), k0.get("y", default_y)
            x1, y1 = k1.get("x", default_x), k1.get("y", default_y)
            return lerp(x0, x1, fraction), lerp(y0, y1, fraction)
    return default_x, default_y

def render_animation(anim_name, duration_sec, fps=15):
    with open(JSON_PATH, "r", encoding="utf-8") as f:
        skel = json.load(f)

    bones_def = {b["name"]: b for b in skel["bones"]}
    bone_order = [b["name"] for b in skel["bones"]]
    slots_def = skel["slots"]
    anim_data = skel["animations"].get(anim_name, {})
    anim_bones = anim_data.get("bones", {})
    attachments_def = skel["skins"][0]["attachments"]

    total_frames = int(duration_sec * fps)
    frames = []

    # Preload master parts
    parts_imgs = {}
    for slot in slots_def:
        name = slot["name"]
        p_path = os.path.join(PARTS_DIR, f"{name}.png")
        if os.path.exists(p_path):
            parts_imgs[name] = Image.open(p_path).convert("RGBA")

    canvas_w, canvas_h = 360, 360
    origin_x, origin_y = 180, 310

    for f_idx in range(total_frames):
        t = (f_idx / total_frames) * duration_sec
        canvas = Image.new("RGBA", (canvas_w, canvas_h), (0, 0, 0, 0))

        # Forward Kinematics
        world_transforms = {}
        for b_name in bone_order:
            b_def = bones_def[b_name]
            parent_name = b_def.get("parent")

            setup_x = b_def.get("x", 0.0)
            setup_y = b_def.get("y", 0.0)
            setup_rot = b_def.get("rotation", 0.0)

            delta_x, delta_y = 0.0, 0.0
            delta_rot = 0.0

            if b_name in anim_bones:
                b_anim = anim_bones[b_name]
                if "translate" in b_anim:
                    delta_x, delta_y = sample_vec2_timeline(b_anim["translate"], t)
                if "rotate" in b_anim:
                    delta_rot = sample_timeline(b_anim["rotate"], t, val_key="value")

            local_x = setup_x + delta_x
            local_y = setup_y + delta_y
            local_rot = setup_rot + delta_rot

            if parent_name and parent_name in world_transforms:
                p_x, p_y, p_rot = world_transforms[parent_name]
                rad = math.radians(p_rot)
                cos_r = math.cos(rad)
                sin_r = math.sin(rad)
                # y is up in Spine
                w_x = p_x + (local_x * cos_r - local_y * sin_r)
                w_y = p_y + (local_x * sin_r + local_y * cos_r)
                w_rot = p_rot + local_rot
            else:
                w_x = local_x
                w_y = local_y
                w_rot = local_rot

            world_transforms[b_name] = (w_x, w_y, w_rot)

        # Draw slots back-to-front
        for slot in slots_def:
            s_name = slot["name"]
            bone_name = slot["bone"]
            if s_name not in parts_imgs or bone_name not in world_transforms:
                continue

            part_img = parts_imgs[s_name]
            w_x, w_y, w_rot = world_transforms[bone_name]

            # Read attachment offset
            att_info = attachments_def.get(s_name, {}).get(s_name, {})
            att_x = att_info.get("x", 0.0)
            att_y = att_info.get("y", 0.0)

            rad = math.radians(w_rot)
            cos_r = math.cos(rad)
            sin_r = math.sin(rad)

            part_center_x = w_x + (att_x * cos_r - att_y * sin_r)
            part_center_y = w_y + (att_x * sin_r + att_y * cos_r)

            # Invert y for screen space
            screen_x = int(origin_x + part_center_x)
            screen_y = int(origin_y - part_center_y)

            if abs(w_rot) > 0.1:
                rot_img = part_img.rotate(w_rot, resample=Image.Resampling.NEAREST, expand=True)
            else:
                rot_img = part_img

            paste_x = screen_x - rot_img.width // 2
            paste_y = screen_y - rot_img.height // 2

            canvas.paste(rot_img, (paste_x, paste_y), rot_img)

        frames.append(canvas)

    out_gif = os.path.join(SPINE_DIR, f"stone_golem_{anim_name}.gif")
    if frames:
        frame_duration_ms = int(1000 / fps)
        frames[0].save(
            out_gif,
            save_all=True,
            append_images=frames[1:],
            duration=frame_duration_ms,
            loop=0,
            disposal=2
        )
        print(f"🎬 Exported Master Animated GIF: {out_gif} ({len(frames)} frames @ {fps} FPS)")
    return out_gif

def main():
    print("=== RENDERING MASTER STONE GOLEM ANIMATED GIFS ===")
    render_animation("idle", 1.6, fps=15)
    render_animation("slam", 1.0, fps=15)
    render_animation("walk", 1.2, fps=15)
    print("\n🎉 ALL MASTER ANIMATIONS RENDERED!")

if __name__ == "__main__":
    main()
