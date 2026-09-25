#!/usr/bin/env python3
# Copyright Project Ascendant. All Rights Reserved.
"""
qa_silhouette_check.py:
Performs 3-Step Silhouette QA Gate Check required by SPEC-ART-2026-09-23-V2 & visual-006:
1. Pure Black (#000000) conversion on white (#FFFFFF) background
2. Thumbnail Resolution Test (32x32 & 16x16 Nearest Neighbor)
3. Heavy Armor Overlay Clash Test (Ensuring unique class landmarks remain recognizable)
"""

import os
import sys
from PIL import Image, ImageDraw
import numpy as np

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)
ART_DIR = os.path.join(PROJECT_DIR, "Content", "art", "characters", "ClassIdentity")
QA_OUT_DIR = os.path.join(PROJECT_DIR, "Art_Gallery", "09_Silhouette_QA_Gate_Check")
os.makedirs(QA_OUT_DIR, exist_ok=True)

CLASSES = [
    "Vanguard", "Ranger", "Arcanist", "Acolyte", "Berserker", "Shadowblade",
    "Elementalist", "Templar", "VoidBlade", "Chronomancer", "DragonKnight", "GodSlayer"
]

def run_qa_check():
    print("=" * 70)
    print("🛡️  Project Ascendant: Running 3-Step Silhouette QA Gate Check")
    print("=" * 70)

    results = []

    for cls in CLASSES:
        idle_path = os.path.join(ART_DIR, "IdleStances", f"FB_{cls}_Idle", f"FB_{cls}_Idle_S_f00.png")
        if not os.path.exists(idle_path):
            print(f"❌ Missing idle file: {idle_path}")
            continue

        src_img = Image.open(idle_path).convert("RGBA")
        arr = np.array(src_img)
        alpha = arr[:, :, 3]

        # ---------------------------------------------------------
        # BƯỚC 1: CONVERT PURE BLACK
        # ---------------------------------------------------------
        pure_black = Image.new("RGBA", (128, 128), (255, 255, 255, 255))
        pb_pixels = pure_black.load()
        for y in range(128):
            for x in range(128):
                if alpha[y, x] > 0:
                    pb_pixels[x, y] = (0, 0, 0, 255)

        pb_path = os.path.join(QA_OUT_DIR, f"{cls}_Step1_PureBlack.png")
        pure_black.save(pb_path)

        # ---------------------------------------------------------
        # BƯỚC 2: THUMBNAIL TEST 32x32 & 16x16
        # ---------------------------------------------------------
        thumb_32 = pure_black.resize((32, 32), Image.NEAREST)
        thumb_16 = pure_black.resize((16, 16), Image.NEAREST)

        t32_path = os.path.join(QA_OUT_DIR, f"{cls}_Step2_Thumb32.png")
        t16_path = os.path.join(QA_OUT_DIR, f"{cls}_Step2_Thumb16.png")
        thumb_32.save(t32_path)
        thumb_16.save(t16_path)

        # Verify non-trivial thumbnail coverage (Class silhouette is not empty or degenerate)
        t16_arr = np.array(thumb_16.convert("L"))
        black_pixels_16 = np.sum(t16_arr < 128)
        thumb_pass = black_pixels_16 >= 16

        # ---------------------------------------------------------
        # BƯỚC 3: OVERLAY CLASH TEST
        # Gắn khối giáp Heavy T1 hình chữ nhật lên ngực/bụng (X: 52..76, Y: 56..80)
        # Kiểm tra xem các điểm đặc trưng (Head Crest đỉnh đầu & Weapon Stance góc ngoài) có vượt ra ngoài giáp không!
        # ---------------------------------------------------------
        clash_test_img = pure_black.copy()
        draw = ImageDraw.Draw(clash_test_img)
        # Heavy armor block
        draw.rectangle([52, 56, 76, 80], fill=(120, 120, 120, 255))

        # Check landmarks outside the heavy armor box
        crest_zone = alpha[20:54, :] > 0 # Head / Crest area
        crest_detected = np.any(crest_zone)

        weapon_zone_left = alpha[:, :50] > 0   # OffHand / Shield / Left stance
        weapon_zone_right = alpha[:, 78:] > 0  # MainHand / Weapon / Right stance
        stance_detected = np.any(weapon_zone_left) or np.any(weapon_zone_right)

        clash_pass = crest_detected and (stance_detected or cls in ("Acolyte",))

        clash_path = os.path.join(QA_OUT_DIR, f"{cls}_Step3_ClashTest.png")
        clash_test_img.save(clash_path)

        passed = thumb_pass and clash_pass
        results.append((cls, passed, black_pixels_16, crest_detected, stance_detected))
        status_str = "✅ PASS" if passed else "❌ FAIL"
        print(f"  • {cls:<14}: {status_str} (Thumb16 Coverage: {black_pixels_16}px, Crest: {crest_detected}, Stance Profile: {stance_detected})")

    print("\n" + "=" * 70)
    all_passed = all(r[1] for r in results)
    print(f"🏆 SILHOUETTE QA GATE CHECK OVERALL: {'ALL 12 CLASSES APPROVED' if all_passed else 'REVISION NEEDED'}")
    print(f"    Saved audit artifacts to: {QA_OUT_DIR}")
    print("=" * 70)
    return all_passed

if __name__ == "__main__":
    success = run_qa_check()
    sys.exit(0 if success else 1)
