import os
from PIL import Image
import numpy as np

def slice_spritesheet(img_path, output_dir, row_specs, target_fps=12):
    os.makedirs(output_dir, exist_ok=True)
    im = Image.open(img_path)
    if im.mode != 'RGBA':
        im = im.convert('RGBA')
        
    arr = np.array(im)
    alpha = arr[:, :, 3]
    
    summary = {}
    
    for anim_name, (y_start, y_end), min_width in row_specs:
        anim_dir = os.path.join(output_dir, anim_name.lower())
        os.makedirs(anim_dir, exist_ok=True)
        
        # Detect frame columns
        row_alpha = alpha[y_start:y_end, :]
        col_proj = (row_alpha > 10).sum(axis=0)
        in_col = False
        cols = []
        c_start = 0
        for x in range(im.width):
            if col_proj[x] > 0 and not in_col:
                in_col = True
                c_start = x
            elif col_proj[x] == 0 and in_col:
                in_col = False
                if (x - c_start) >= min_width:
                    cols.append((c_start, x))
        if in_col and (im.width - c_start) >= min_width:
            cols.append((c_start, im.width))
            
        frames = []
        # Max height & width across frames in this animation to normalize canvas
        max_h = y_end - y_start
        max_w = max(c_end - c_start for c_start, c_end in cols) if cols else 64
        
        for f_idx, (c_start, c_end) in enumerate(cols):
            # Crop exact frame
            frame_crop = im.crop((c_start, y_start, c_end, y_end))
            
            # Place in normalized transparent canvas centered at bottom
            norm_frame = Image.new('RGBA', (max_w + 16, max_h + 8), (0, 0, 0, 0))
            paste_x = (norm_frame.width - frame_crop.width) // 2
            paste_y = norm_frame.height - frame_crop.height - 4
            norm_frame.paste(frame_crop, (paste_x, paste_y), frame_crop)
            
            frame_filename = f"{anim_name.lower()}_f{f_idx:02d}.png"
            frame_path = os.path.join(anim_dir, frame_filename)
            norm_frame.save(frame_path)
            frames.append(norm_frame)
            
        # Export animated GIF for this flipbook
        if frames:
            gif_filename = f"FB_{anim_name}.gif"
            gif_path = os.path.join(output_dir, gif_filename)
            duration_ms = int(1000 / target_fps)
            frames[0].save(
                gif_path,
                save_all=True,
                append_images=frames[1:],
                duration=duration_ms,
                loop=0,
                disposal=2
            )
            print(f"Exported {anim_name}: {len(frames)} frames -> {gif_path}")
            summary[anim_name] = len(frames)
            
    return summary

def main():
    print("=== Slicing Vanguard Spritesheet ===")
    vanguard_rows = [
        ('Vanguard_Idle', (18, 128), 30),
        ('Vanguard_Run', (149, 256), 30),
        ('Vanguard_Attack1', (266, 382), 30),
        ('Vanguard_Dash', (549, 676), 30),
        ('Vanguard_Hurt', (686, 845), 30),
        ('Vanguard_Death', (868, 1015), 30)
    ]
    vanguard_dir = "ProjectAscendant/Content/art/characters/vanguard"
    vanguard_summary = slice_spritesheet(
        "ProjectAscendant/Content/art/characters/vanguard_pixel_spritesheet.png",
        vanguard_dir,
        vanguard_rows,
        target_fps=12
    )
    
    print("\n=== Slicing Stone Golem Boss Spritesheet ===")
    boss_rows = [
        ('Golem_Idle', (52, 215), 40),
        ('Golem_Walk', (229, 457), 40),
        ('Golem_Slam', (506, 656), 40),
        ('Golem_Stagger', (672, 813), 40),
        ('Golem_Death', (875, 1021), 40)
    ]
    boss_dir = "ProjectAscendant/Content/art/characters/boss"
    boss_summary = slice_spritesheet(
        "ProjectAscendant/Content/art/characters/stone_golem_boss_pixel_spritesheet.png",
        boss_dir,
        boss_rows,
        target_fps=10
    )
    
    # Also copy all generated GIFs to the brain artifact directory for user review
    artifact_dir = "/home/kenzings/.gemini/antigravity-cli/brain/83a2074d-86e3-4888-ac19-47b28c1c1c8a"
    import shutil
    for d in [vanguard_dir, boss_dir]:
        for f in os.listdir(d):
            if f.endswith(".gif"):
                src = os.path.join(d, f)
                dst = os.path.join(artifact_dir, f)
                shutil.copyfile(src, dst)
                print(f"Copied {f} to artifact directory")

if __name__ == "__main__":
    main()
