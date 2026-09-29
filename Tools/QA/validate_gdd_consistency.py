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
# 15 Branch Classes + 1 Apex Class = 16 Classes Total
APPROVED_CLASSES = {
    "Guard": {
        "T1": ["Vanguard"],
        "T2": ["Templar", "Berserker", "Swordmaster"],
        "T3": ["DragonKnight", "VoidBlade"],
    },
    "Scout": {
        "T1": ["Ranger"],
        "T2": ["Shadowblade"],
        "T3": ["PhantomStalker"],
    },
    "Caster": {
        "T1": ["Arcanist"],
        "T2": ["Elementalist"],
        "T3": ["Chronomancer"],
    },
    "Faith": {
        "T1": ["Acolyte"],
        "T2": ["Inquisitor"],
        "T3": ["Seraph"],
    },
    "Apex": {
        "T4": ["GodSlayer"],
    },
}

# Lookup map: class_name_lower -> line_name
CLASS_TO_LINE = {}
ALL_APPROVED_CLASSES = set()
for line, tiers in APPROVED_CLASSES.items():
    for tier, classes in tiers.items():
        for c in classes:
            c_lower = c.lower()
            CLASS_TO_LINE[c_lower] = line
            ALL_APPROVED_CLASSES.add(c_lower)

APPROVED_LINES = set(APPROVED_CLASSES.keys())

ALLOWED_META_TAGS = {
    "Class.Primary",
    "Class.Secondary",
    "Class.Identity",
    "Class.Slot",
}

# Regex to detect prohibited terms with targeted context inspection
ASH_SHARDS_REGEX = re.compile(r'\bash[\s_-]?shards?\b', re.IGNORECASE)
CHAIN_WHIP_REGEX = re.compile(r'roi\s+x[ií]ch|chain[\s_-]?whip', re.IGNORECASE)
UNAPPROVED_CLASSES_REGEX = re.compile(r'\b(?:Void[\s_-]?Weaver|Class\.Line\.[A-Za-z0-9_.]*Oracle)\b', re.IGNORECASE)

LEGITIMATE_CONTEXT_PATTERNS = [
    re.compile(r'(?:thay\s+v[iì]|thay\s+th[eế]|thay\s+cho|kh[oô]ng\s+d[uù]ng|kh[oô]ng\s+t[aạ]o|b[oỏ]|c[aấ]m|lo[aạ]i\s+b[oỏ]|thay\s+b[oở]i|tr[uư][oớ]c\s+[đd][aâ]y|thay\s+v[iì]\s+d[uù]ng|tuy[eệ]t\s+[đd][oố]i\s+kh[oô]ng)\s+[^.\n]*?\bash[\s_-]?shards?\b', re.IGNORECASE),
    re.compile(r'\bash[\s_-]?shards?\b\s*\(?(?:c[uũ]|tr[uư][oớ]c\s+[đd][aâ]y|b[oỏ]|kh[oô]ng\s+c[oò]n\s+d[uù]ng)\)?', re.IGNORECASE),
    re.compile(r'(?:b[oỏ]|kh[oô]ng\s+d[uù]ng|c[aấ]m)\s+[^.\n]*?(?:roi\s+x[ií]ch|chain[\s_-]?whip)', re.IGNORECASE),
]

def is_legitimate_context(line: str, pattern_match) -> bool:
    """Checks if the occurrence of a prohibited term is part of an explicit deprecation or negative rule statement."""
    for leg_pat in LEGITIMATE_CONTEXT_PATTERNS:
        if leg_pat.search(line):
            return True
    return False

def check_file(file_path: Path):
    errors = []
    warnings = []
    
    with open(file_path, "r", encoding="utf-8", errors="replace") as f:
        content = f.read()

    # Skip historical / archived gate check notes if marked as archive
    if "gdd-cross-review" in file_path.name:
        return errors, warnings

    lines = content.splitlines()

    # 1. Line-by-line checks
    for line_idx, line in enumerate(lines, 1):
        # Check Ash Shards
        ash_match = ASH_SHARDS_REGEX.search(line)
        if ash_match:
            if not is_legitimate_context(line, ash_match):
                errors.append(f"{file_path}:{line_idx}: CẤM: 'Ash Shards' dùng làm tiền tệ. Quyển trục phân rã thành 'Tàn Trang' (Skill Shards / item_skill_shard).")

        # Check Chain Whip for Inquisitor
        whip_match = CHAIN_WHIP_REGEX.search(line)
        if whip_match:
            if not is_legitimate_context(line, whip_match):
                errors.append(f"{file_path}:{line_idx}: CẤM: Roi xích cho Inquisitor. Inquisitor chỉ dùng Weapon.1H.Mace (Chùy 1 tay).")

        # Check Unapproved Classes (Void Weaver, Oracle)
        unapproved_match = UNAPPROVED_CLASSES_REGEX.search(line)
        if unapproved_match:
            errors.append(f"{file_path}:{line_idx}: CẤM: Class '{unapproved_match.group(0)}' không được duyệt. Tổng 15 class + 1 Apex = 16 class.")

        # Check GameplayTags
        tag_matches = re.findall(r'\bClass\.[A-Za-z0-9_.]+', line)
        for raw_tag in tag_matches:
            tag = raw_tag.rstrip(".:,;()[]\"'`")
            if not tag:
                continue

            if tag in ALLOWED_META_TAGS:
                continue

            if tag.startswith("Class.Line."):
                parts = tag.split(".")
                if len(parts) != 4:
                    errors.append(f"{file_path}:{line_idx}: Tag '{tag}' sai định dạng. Cấu trúc chuẩn: 'Class.Line.<Nhánh>.<Class>'.")
                    continue
                
                line_name = parts[2]
                class_name = parts[3]
                class_lower = class_name.lower()

                # Validate Line Name
                if line_name not in APPROVED_LINES:
                    errors.append(f"{file_path}:{line_idx}: Tag '{tag}' chứa nhánh '{line_name}' không hợp lệ. Phải là một trong: {list(APPROVED_LINES)}.")
                    continue

                # Validate Class Name existence
                if class_lower not in ALL_APPROVED_CLASSES:
                    errors.append(f"{file_path}:{line_idx}: Tag '{tag}' chứa class '{class_name}' không tồn tại trong danh mục 17 class đã chốt.")
                    continue

                # Validate Class belongs to Line
                expected_line = CLASS_TO_LINE[class_lower]
                if line_name != expected_line:
                    errors.append(f"{file_path}:{line_idx}: Tag '{tag}' mâu thuẫn: Class '{class_name}' thuộc nhánh '{expected_line}', không phải '{line_name}'.")
            else:
                # Obsolete tag pattern (Class.TierX.*, Class.RankX.*, Class.Vanguard, Class.VoidBlade, etc.)
                errors.append(f"{file_path}:{line_idx}: Tag '{tag}' sai cấu trúc hoặc lỗi thời. Toàn bộ GameplayTag chức nghiệp bắt buộc dùng chuẩn 'Class.Line.<Nhánh>.<Class>'.")

        # Check naming error
        if "item_instance_instance_id" in line:
            errors.append(f"{file_path}:{line_idx}: Lỗi tên cột 'item_instance_instance_id', phải là 'item_instance_id'.")

    # 2. Multiline SQL Schema Validation
    if "CREATE TABLE items" in content:
        # Check multiline uk_owner_slot constraint
        uk_pattern = re.compile(
            r'CONSTRAINT\s+uk_owner_slot\s+UNIQUE\s*\([^)]+\)\s+DEFERRABLE\s+INITIALLY\s+DEFERRED',
            re.IGNORECASE | re.MULTILINE
        )
        if not uk_pattern.search(content):
            errors.append(f"{file_path}: Bảng 'items' thiếu hoặc khai báo sai ràng buộc 'CONSTRAINT uk_owner_slot UNIQUE (...) DEFERRABLE INITIALLY DEFERRED'.")

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
    root = current_dir.parent.parent
    sys.exit(validate_all_gdd(root))
