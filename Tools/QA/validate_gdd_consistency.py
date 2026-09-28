#!/usr/bin/env python3
"""
GDD Consistency Validator for Project Ascendant
Validates all design documents in design/gdd against production/DECISIONS.md
"""

import os
import re
import sys
from pathlib import Path

# Approved canonical classes and lines
APPROVED_CLASSES = {
    "Guard": {
        "T1": ["Vanguard"],
        "T2": ["Templar", "Berserker"],
        "T3": ["DragonKnight"],
    },
    "Scout": {
        "T1": ["Ranger"],
        "T2": ["Shadowblade", "Swordmaster"],
        "T3": ["PhantomStalker"],
    },
    "Caster": {
        "T1": ["Arcanist"],
        "T2": ["Elementalist", "Chronomancer"],
        "T3": ["VoidWeaver"],
    },
    "Faith": {
        "T1": ["Acolyte"],
        "T2": ["Inquisitor", "Oracle"],
        "T3": ["Seraph"],
    },
    "Apex": {
        "T4": ["GodSlayer"],
    },
}

ALL_APPROVED_CLASSES = set()
for line, tiers in APPROVED_CLASSES.items():
    for tier, classes in tiers.items():
        for c in classes:
            ALL_APPROVED_CLASSES.add(c.lower())

APPROVED_LINES = set(APPROVED_CLASSES.keys())

PROHIBITED_PATTERNS = [
    (re.compile(r'\bash[\s_-]?shards?\b', re.IGNORECASE), "CẤM: 'Ash Shards'. Quyển trục phân rã thành 'Tàn Trang' (Skill Shards / item_skill_shard)."),
    (re.compile(r'roi\s+x[ií]ch|chain[\s_-]?whip', re.IGNORECASE), "CẤM: Roi xích cho Inquisitor. Inquisitor chỉ dùng Weapon.1H.Mace (Chùy 1 tay)."),
    (re.compile(r'Class\.Tier[1-4]\.', re.IGNORECASE), "CẤM định dạng tag cũ 'Class.TierX.'. Phải dùng 'Class.Line.<Nhánh>.<Class>'."),
    (re.compile(r'Class\.Rank[1-4]\.', re.IGNORECASE), "CẤM định dạng tag cũ 'Class.RankX.'. Phải dùng 'Class.Line.<Nhánh>.<Class>'."),
]

def check_file(file_path: Path):
    errors = []
    warnings = []
    
    with open(file_path, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()

    # Skip historical / archived gate check notes if marked as archive
    is_archive = "gdd-cross-review" in file_path.name

    for line_idx, line in enumerate(lines, 1):
        if is_archive:
            continue

        # Check prohibited terms
        for pattern, msg in PROHIBITED_PATTERNS:
            if pattern.search(line):
                # Check context - allow if explicitly saying "không dùng Ash Shards" or "bỏ roi xích"
                lower = line.lower()
                if "không" in lower or "bỏ" in lower or "cấm" in lower or "thay vì" in lower or "trước đây" in lower:
                    continue
                errors.append(f"{file_path}:{line_idx}: {msg}")

        # Check GameplayTags pattern
        tag_matches = re.findall(r'Class\.[A-Za-z0-9_.]+', line)
        for tag in tag_matches:
            if tag.startswith("Class.Line."):
                parts = tag.split(".")
                if len(parts) >= 4:
                    line_name = parts[2]
                    class_name = parts[3]
                    if line_name not in APPROVED_LINES:
                        errors.append(f"{file_path}:{line_idx}: Tag '{tag}' chứa nhánh '{line_name}' không hợp lệ. Phải là một trong: {list(APPROVED_LINES)}.")
            elif tag in ["Class.Primary", "Class.Secondary", "Class.Identity", "Class.Slot"]:
                continue
            elif "Class.Tier" in tag or "Class.Rank" in tag:
                errors.append(f"{file_path}:{line_idx}: Tag '{tag}' sai cấu trúc. Cấu trúc chuẩn: 'Class.Line.<Nhánh>.<Class>'.")

        # Check Database schema specifics if SQL file or sql codeblock
        if "CONSTRAINT uk_owner_slot" in line:
            if "DEFERRABLE INITIALLY DEFERRED" not in line:
                errors.append(f"{file_path}:{line_idx}: Ràng buộc uk_owner_slot thiếu 'DEFERRABLE INITIALLY DEFERRED' để cho phép hoán đổi ô đồ nguyên tử.")

        if "item_instance_instance_id" in line:
            errors.append(f"{file_path}:{line_idx}: Lỗi tên cột 'item_instance_instance_id', phải là 'item_instance_id'.")

    return errors, warnings

def validate_all_gdd(project_root: Path):
    gdd_dir = project_root / "design" / "gdd"
    decisions_file = project_root / "production" / "DECISIONS.md"

    print("=" * 60)
    print("Project Ascendant: GDD Consistency Validator")
    print(f"Target Directory: {gdd_dir}")
    print("=" * 60)

    if not decisions_file.exists():
        print(f"[ERROR] Master decisions file not found at: {decisions_file}")
        return 1

    if not gdd_dir.exists():
        print(f"[ERROR] GDD directory not found at: {gdd_dir}")
        return 1

    total_files = 0
    total_errors = 0
    total_warnings = 0

    for file_path in sorted(gdd_dir.glob("*.md")):
        total_files += 1
        errors, warnings = check_file(file_path)
        
        if errors:
            print(f"\n[FAIL] {file_path.name}:")
            for err in errors:
                print(f"  - {err}")
            total_errors += len(errors)
        elif warnings:
            print(f"\n[WARN] {file_path.name}:")
            for warn in warnings:
                print(f"  - {warn}")
            total_warnings += len(warnings)
        else:
            print(f"[PASS] {file_path.name}")

    print("\n" + "=" * 60)
    print(f"Validation Summary: {total_files} files checked.")
    print(f"Errors: {total_errors} | Warnings: {total_warnings}")
    print("=" * 60)

    if total_errors > 0:
        print("[RESULT] GDD Consistency Check FAILED.")
        return 1
    else:
        print("[RESULT] GDD Consistency Check PASSED.")
        return 0

if __name__ == "__main__":
    current_dir = Path(__file__).resolve().parent
    # Project root is 2 levels up from Tools/QA
    root = current_dir.parent.parent
    sys.exit(validate_all_gdd(root))
