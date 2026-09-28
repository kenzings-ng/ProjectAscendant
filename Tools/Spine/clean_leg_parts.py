#!/usr/bin/env python3
import numpy as np
from PIL import Image

def clean_leg(part_path, is_thigh=True):
    img = Image.open(part_path).convert("RGBA")
    arr = np.array(img)
    h, w, _ = arr.shape
    
    # In each row, find the true leg stone block from the right side.
    # Fist fingers on the left have an outline separation.
    # In the scaled image (0.5 scale), let's inspect where fist vs leg is.
    print(f"Loaded {part_path}, size {w}x{h}")

clean_leg("Content/art/characters/boss/spine/clean_parts/thigh_r.png", is_thigh=True)
clean_leg("Content/art/characters/boss/spine/clean_parts/calf_r.png", is_thigh=False)
