#!/usr/bin/env python3
"""
Project Ascendant - GDD & Story Consistency Validator
Validates all design documents (design/gdd) and story files (production/epics, production/sprints)
against production/DECISIONS.md.

Performs deep structural, relationship, terminology, schema, and frame calculation validation.
All approved classes, lines, ranks, weapon families, and direction counts are dynamically loaded
from DECISIONS.md (no hardcoded class names or weapons).
"""

import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Set, Tuple, Optional

# Metadata tags allowed
ALLOWED_META_TAGS = {
    "Class.Primary",
    "Class.Secondary",
    "Class.Identity",
    "Class.Slot",
}

# Negative/deprecation phrases that make a match legitimate (not a violation)
LEGITIMATE_CONTEXT_PATTERNS = [
    re.compile(r'(?:thay\s+v[iì]|thay\s+th[eế]|thay\s+cho|kh[oô]ng\s+d[uù]ng|kh[oô]ng\s+t[aạ]o|b[oỏ]|c[aấ]m|lo[aạ]i\s+b[oỏ]|thay\s+b[oở]i|tr[uư][oớ]c\s+[đd][aâ]y|thay\s+v[iì]\s+d[uù]ng|tuy[eệ]t\s+[đd][oố]i\s+kh[oô]ng|kh[oô]ng\s+c[oò]n)\s+[^.\n]*?\bash[\s_-]?shards?\b', re.IGNORECASE),
    re.compile(r'\bash[\s_-]?shards?\b\s*\(?(?:c[uũ]|tr[uư][oớ]c\s+[đd][aâ]y|b[oỏ]|kh[oô]ng\s+c[oò]n\s+d[uù]ng)\)?', re.IGNORECASE),
    re.compile(r'(?:b[oỏ]|kh[oô]ng\s+d[uù]ng|c[aấ]m|lo[aạ]i\s+b[oỏ])\s+[^.\n]*?(?:roi\s+x[ií]ch|chain[\s_-]?whip)', re.IGNORECASE),
    re.compile(r'(?:tuy[eệ]t\s+[đd][oố]i\s+kh[oô]ng\s+d[uù]ng|kh[oô]ng\s+d[uù]ng\s+chung)\s+[^.\n]*?tier', re.IGNORECASE),
    re.compile(r'(?:x[oó]a\s+b[oỏ]|lo[aạ]i\s+b[oỏ]|c[aấ]m|kh[oô]ng\s+[đd][uư][oợ]c\s+duy[eệ]t|kh[oô]ng\s+d[uù]ng)\s+[^.\n]*?\b(?:oracle|void[\s_-]?weaver)\b', re.IGNORECASE),
    re.compile(r'\b(?:oracle|void[\s_-]?weaver)\b[^.\n]*?(?:kh[oô]ng\s+[đd][uư][oợ]c\s+duy[eệ]t|[đd][aã]\s+b[iị]\s+lo[aạ]i|b[iị]\s+x[oó]a)', re.IGNORECASE),
]

# Prohibited class terminology regexes (DECISIONS.md Mục 1: Bậc T1-T4, không dùng Tier hoặc Rarity cho Class)
CLASS_TIER_RARITY_PATTERNS = [
    (re.compile(r'\b(?:Normal|Rare|Epic|Mythic(?:\s+Hidden)?)\s*(?:[-–/]\s*Tier\s*[1-4]|\(\s*Tier\s*[1-4]\s*\))', re.IGNORECASE),
     "CẤM: Dùng độ hiếm kết hợp 'Tier [1-4]' để chỉ class. Bậc chức nghiệp bắt buộc dùng chuẩn 'Bậc T1/T2/T3/T4' theo DECISIONS.md mục 1."),
    (re.compile(r'\b(?:độ\s+hiếm\s+chức\s+nghiệp|độ\s+hiếm\s+class)\b', re.IGNORECASE),
     "CẤM: 'Độ hiếm chức nghiệp/class'. Chức nghiệp dùng 'Bậc' (Rank: T1, T2, T3, T4), vật phẩm dùng 'Độ hiếm' (Rarity)."),
    (re.compile(r'\bClass\s+Bậc\s+(?:Normal|Rare|Epic|Mythic)\b', re.IGNORECASE),
     "CẤM: Dùng tên độ hiếm (Normal/Rare/Epic/Mythic) sau chữ 'Class Bậc'. Phải dùng Bậc T1 (Sơ cấp) / T2 (Trung cấp) / T3 (Cao cấp) / T4 (Ẩn)."),
    (re.compile(r'\b(?:Normal|Rare|Epic|Mythic)\s+Class(?:es)?\b', re.IGNORECASE),
     "CẤM: Dùng tên độ hiếm trước 'Class' (như 'Mythic Class', 'Rare Class'). Dùng 'Class Bậc T1/T2/T3/T4'."),
    (re.compile(r'\b(?:Class\s+Tier\s*[1-4]|Tier\s*[1-4]\s+Class(?:es)?)\b', re.IGNORECASE),
     "CẤM: Dùng từ 'Tier' cho Class. Phải dùng 'Bậc T1/T2/T3/T4' theo DECISIONS.md mục 1."),
    (re.compile(r'\bTầng\s+Class\b', re.IGNORECASE),
     "CẤM: Dùng 'Tầng Class'. Phải dùng 'Bậc Chức Nghiệp'."),
]

# Weapon family regex patterns for normalization
WEAPON_PATTERNS = {
    "1h_blade": [re.compile(r"Weapon\.1H\.Blade|\b1H\.Blade\b|weapon_family_(?:1h_)?blades|\b1H Blades\b|\bOne-Handed Blades\b", re.IGNORECASE)],
    "2h_heavy": [re.compile(r"Weapon\.2H\.Heavy|\b2H\.Heavy\b|weapon_family_(?:2h_)?heavy|\b2H Heavy\b|\bTwo-Handed Heavy\b", re.IGNORECASE)],
    "2h_polearm": [re.compile(r"Weapon\.2H\.Polearm|\b2H\.Polearm\b|weapon_family_(?:2h_)?polearms|\bPolearms\b|\b2H Polearm\b|\bPolearms & Halberds\b", re.IGNORECASE)],
    "2h_bow": [re.compile(r"Weapon\.2H\.Bow|\b2H\.Bow\b|weapon_family_(?:2h_)?bows|\b2H Bow\b|\bRanged Bows\b|\bBows\b", re.IGNORECASE)],
    "dual_daggers": [re.compile(r"Weapon\.Dual\.Daggers|\bDual\.Daggers\b|weapon_family_(?:dual|twin)_daggers|\bDual Daggers\b|\bTwin Light Blades\b|\bTwin Daggers\b", re.IGNORECASE)],
    "2h_staff": [re.compile(r"Weapon\.2H\.Staff|\b2H\.Staff\b|weapon_family_(?:2h_)?staves|\b2H Staff\b|\bMagic Staves\b|\bStaves\b", re.IGNORECASE)],
    "1h_mace": [re.compile(r"Weapon\.1H\.Mace|\b1H\.Mace\b|weapon_family_(?:1h_)?maces_relics|\b1H Mace\b|\bBlunt Maces & Relics\b|\bMaces & Relics\b", re.IGNORECASE)],
}

def is_legitimate_context(line: str) -> bool:
    """Checks if the occurrence of a prohibited term is part of an explicit deprecation or negative rule statement."""
    for leg_pat in LEGITIMATE_CONTEXT_PATTERNS:
        if leg_pat.search(line):
            return True
    return False

def load_decisions(decisions_path: Path) -> Dict:
    """
    Dynamically loads approved classes, lines, ranks, weapon families,
    and view direction count from production/DECISIONS.md.
    No hardcoded class lists or weapon lists.
    """
    if not decisions_path.exists():
        raise FileNotFoundError(f"DECISIONS.md not found at {decisions_path}")

    with open(decisions_path, "r", encoding="utf-8") as f:
        text = f.read()

    class_to_line = {}
    class_to_rank = {}
    all_approved_classes = set()
    approved_lines = set()

    # 1. Parse Section 2: Approved Classes, Lines, Ranks
    sec2_match = re.search(r"## 2\.\s+Cây Chuyển Chức.*?(?=## 3\.)", text, re.DOTALL)
    if sec2_match:
        curr_line = None
        for l in sec2_match.group(0).splitlines():
            m_line = re.search(r"\((Guard|Scout|Caster|Faith|Apex)\s+Line", l)
            if not m_line:
                m_line = re.search(r"\((Apex\s+Class)", l)
            if m_line:
                curr_line = "Apex" if "Apex" in m_line.group(1) else m_line.group(1)
                approved_lines.add(curr_line)
            m_rank = re.search(r"-\s+\*\*(T[1-4])\*\*:\s*(.*)", l)
            if m_rank and curr_line:
                rank = m_rank.group(1)
                raw_classes = m_rank.group(2)
                classes = [c for c in re.findall(r"`([^`]+)`", raw_classes) if not c.startswith("Class.")]
                for c in classes:
                    cl = c.lower().strip()
                    class_to_line[cl] = curr_line
                    class_to_rank[cl] = rank
                    all_approved_classes.add(cl)
                    no_space = cl.replace(" ", "")
                    class_to_line[no_space] = curr_line
                    class_to_rank[no_space] = rank
                    all_approved_classes.add(no_space)

    # 2. Parse Section 7: Weapon Family Mappings for each class
    class_weapons = {}  # class_lower -> set of weapon keys e.g. "1h_blade"
    sec7_match = re.search(r"## 7\.\s+Quy Chuẩn Vũ Khí.*?(?=## 8\.)", text, re.DOTALL)
    if sec7_match:
        for l in sec7_match.group(0).splitlines():
            m = re.search(r"-\s+\*\*([A-Za-z\s]+)\s*\((T[1-4])\)\*\*:\s*(.*)", l)
            if m:
                cname = m.group(1).strip()
                rest = m.group(3)
                weaps = re.findall(r"`(Weapon\.[A-Za-z0-9_.]+)`", rest)
                cl = cname.lower()
                norm_weaps = set()
                for w in weaps:
                    wl = w.lower()
                    if "1h.blade" in wl: norm_weaps.add("1h_blade")
                    elif "2h.heavy" in wl: norm_weaps.add("2h_heavy")
                    elif "2h.polearm" in wl: norm_weaps.add("2h_polearm")
                    elif "2h.bow" in wl: norm_weaps.add("2h_bow")
                    elif "dual.daggers" in wl: norm_weaps.add("dual_daggers")
                    elif "2h.staff" in wl: norm_weaps.add("2h_staff")
                    elif "1h.mace" in wl: norm_weaps.add("1h_mace")
                class_weapons[cl] = norm_weaps
                class_weapons[cl.replace(" ", "")] = norm_weaps

    # 3. Parse Section 10: Approved Direction Count
    view_direction_count = 5
    sec10_match = re.search(r"## 10\.\s+Tiêu Chuẩn Art Gate.*?(?=## 11\.)", text, re.DOTALL)
    if sec10_match:
        m_dir = re.search(r"\*\*(\d+)\s*hướng\s*nhìn\*\*", sec10_match.group(0))
        if m_dir:
            view_direction_count = int(m_dir.group(1))

    return {
        "class_to_line": class_to_line,
        "class_to_rank": class_to_rank,
        "all_approved_classes": all_approved_classes,
        "approved_lines": approved_lines,
        "class_weapons": class_weapons,
        "view_direction_count": view_direction_count,
    }

def check_file(file_path: Path, decisions: Dict) -> Tuple[List[str], List[str]]:
    errors = []
    warnings = []

    with open(file_path, "r", encoding="utf-8", errors="replace") as f:
        content = f.read()

    # Skip historical / archived gate check notes if marked as archive
    if "gdd-cross-review" in file_path.name:
        return errors, warnings

    lines = content.splitlines()

    class_to_line = decisions["class_to_line"]
    approved_lines = decisions["approved_lines"]
    all_approved_classes = decisions["all_approved_classes"]
    class_weapons = decisions["class_weapons"]
    expected_dirs = decisions["view_direction_count"]

    is_gdd_file = "design/gdd" in str(file_path.as_posix())

    # 1. Common Line-by-line checks for both GDD and Story files
    for line_idx, line in enumerate(lines, 1):
        if is_legitimate_context(line):
            continue

        # Check Unapproved Classes: Void Weaver & Oracle (in any case, lowercase or uppercase)
        if re.search(r'\bvoid[\s_-]?weaver\b', line, re.IGNORECASE):
            errors.append(f"{file_path}:{line_idx}: CẤM: Class 'Void Weaver' không được duyệt. Tổng 15 class + 1 Apex = 16 class.")
        if re.search(r'\boracle\b', line, re.IGNORECASE):
            errors.append(f"{file_path}:{line_idx}: CẤM: Class 'Oracle' không được duyệt. Tổng 15 class + 1 Apex = 16 class.")

        # Check Frame Calculation Direction Count
        # Matches expressions like: \times 8 hướng, $\times 8$ hướng, x 8 hướng, * 8 hướng, 8 hướng \times, etc.
        m_frame_calc = re.search(r'(?:\\times|[\$x×*]|\bnhân\b)\s*(\d+)\s*\$?\s*(?:hướng|directions?)\b', line, re.IGNORECASE)
        if not m_frame_calc:
            m_frame_calc = re.search(r'(\d+)\s*\$?\s*(?:hướng|directions?)\s*(?:\\times|[\$x×*]|\bnhân\b)', line, re.IGNORECASE)
        if m_frame_calc:
            dirs_found = int(m_frame_calc.group(1))
            if dirs_found != expected_dirs:
                errors.append(f"{file_path}:{line_idx}: Phép tính frame sai số hướng nhìn: Dùng '{dirs_found} hướng'. Theo DECISIONS.md Mục 10, nhân vật dùng {expected_dirs} hướng nhìn.")

        # Check obsolete 12 Class count (both GDD and Story files)
        if re.search(r'\b12\s+(?:Class|Chức\s+nghiệp)\b', line, re.IGNORECASE):
            errors.append(f"{file_path}:{line_idx}: Lỗi số lượng: Còn sót '12 Class' / '12 Chức nghiệp'. Tổng số class chính thức là 16 Class theo DECISIONS.md Mục 2.")

        # GDD-only detailed checks
        if is_gdd_file:
            # Check Ash Shards
            if re.search(r'\bash[\s_-]?shards?\b', line, re.IGNORECASE):
                errors.append(f"{file_path}:{line_idx}: CẤM: 'Ash Shards' dùng làm tiền tệ. Quyển trục phân rã thành 'Tàn Trang' (Skill Shards / item_skill_shard).")

            # Check Chain Whip for Inquisitor
            if re.search(r'roi\s+x[ií]ch|chain[\s_-]?whip', line, re.IGNORECASE):
                errors.append(f"{file_path}:{line_idx}: CẤM: Roi xích cho Inquisitor. Inquisitor chỉ dùng Weapon.1H.Mace (Chùy 1 tay).")

            # Check Prohibited Class Tier / Rarity Terminology
            for pat, err_msg in CLASS_TIER_RARITY_PATTERNS:
                if pat.search(line):
                    errors.append(f"{file_path}:{line_idx}: {err_msg}")

            # Check GameplayTags
            tag_matches = re.findall(r'\bClass\.[A-Za-z0-9_.]+', line)
            for raw_tag in tag_matches:
                tag = raw_tag.rstrip(".:,;()[]\"'`")
                if not tag or tag in ALLOWED_META_TAGS:
                    continue

                if tag.startswith("Class.Line."):
                    parts = tag.split(".")
                    if len(parts) != 4:
                        errors.append(f"{file_path}:{line_idx}: Tag '{tag}' sai định dạng. Cấu trúc chuẩn: 'Class.Line.<Nhánh>.<Class>'.")
                        continue

                    line_name = parts[2]
                    class_name = parts[3]
                    class_lower = class_name.lower()

                    if line_name not in approved_lines:
                        errors.append(f"{file_path}:{line_idx}: Tag '{tag}' chứa nhánh '{line_name}' không hợp lệ. Phải là một trong: {list(approved_lines)}.")
                        continue

                    if class_lower not in all_approved_classes:
                        errors.append(f"{file_path}:{line_idx}: Tag '{tag}' chứa class '{class_name}' không tồn tại trong danh mục class đã chốt.")
                        continue

                    expected_line = class_to_line.get(class_lower)
                    if expected_line and line_name != expected_line:
                        errors.append(f"{file_path}:{line_idx}: Tag '{tag}' mâu thuẫn: Class '{class_name}' thuộc nhánh '{expected_line}', không phải '{line_name}'.")
                else:
                    errors.append(f"{file_path}:{line_idx}: Tag '{tag}' sai cấu trúc hoặc lỗi thời. Toàn bộ GameplayTag chức nghiệp bắt buộc dùng chuẩn 'Class.Line.<Nhánh>.<Class>'.")

            # Check column name error
            if "item_instance_instance_id" in line:
                errors.append(f"{file_path}:{line_idx}: Lỗi tên cột 'item_instance_instance_id', phải là 'item_instance_id'.")

            # Check prohibited equipment tiers / rarities
            if re.search(r'\b(?:ShieldTier|5-Tier\s+Item)\b', line, re.IGNORECASE):
                errors.append(f"{file_path}:{line_idx}: CẤM: Dùng từ 'Tier' cho trang bị/khiên. Trang bị dùng thang 'Độ Hiếm' (Rarity: Common -> Legendary) theo DECISIONS.md Mục 1.")

            if re.search(r'\b(?:trang\s+bị\s+(?:Immortal|Divine)|đúc\s+đồ\s+Immortal|Immortal\s*\([Đđ]ỏ\)|Divine\s*\([Hh]oàng\s+kim\))\b', line, re.IGNORECASE):
                errors.append(f"{file_path}:{line_idx}: CẤM: Trang bị 'Immortal' / 'Divine'. Thang độ hiếm trang bị chỉ gồm 5 bậc (Common, Uncommon, Rare, Epic, Legendary) theo DECISIONS.md Mục 5.")

    # 2. General Table & YAML Weapon Family Relationship Validation (MỌI bảng và khối YAML trong GDD và Story)
    all_classes_sorted = sorted(list(class_weapons.keys()), key=len, reverse=True)

    # 2.1 Table row checks
    for idx, line in enumerate(lines, 1):
        if "|" in line:
            cols = [c.strip() for c in line.split("|")]
            matched_weap = None
            for wkey, pats in WEAPON_PATTERNS.items():
                for p in pats:
                    if any(p.search(col) for col in cols[:3]):
                        matched_weap = wkey
                        break
                if matched_weap:
                    break

            if matched_weap:
                # Find classes mentioned in the row
                for c in all_classes_sorted:
                    c_pat = re.compile(r"\b" + re.escape(c) + r"\b", re.IGNORECASE)
                    if c_pat.search(line):
                        allowed = class_weapons.get(c, set())
                        if matched_weap not in allowed:
                            errors.append(f"{file_path}:{idx}: Quan hệ vũ khí sai: Class '{c.title()}' không được phép sử dụng dòng vũ khí '{matched_weap}' theo DECISIONS.md mục 7.")

    # 2.2 YAML attributes block checks
    curr_yaml_weapon = None
    for idx, line in enumerate(lines, 1):
        m_yaml_weap = re.search(r'name:\s*(weapon_family_[a-z0-9_]+)', line)
        if m_yaml_weap:
            yname = m_yaml_weap.group(1)
            curr_yaml_weapon = None
            for wkey, pats in WEAPON_PATTERNS.items():
                if any(p.search(yname) for p in pats):
                    curr_yaml_weapon = wkey
                    break
        elif line.strip().startswith("- name:"):
            curr_yaml_weapon = None
        elif curr_yaml_weapon and "classes:" in line:
            raw_classes = re.findall(r'"([^"]+)"|\'([^\']+)\'', line)
            for c_tuple in raw_classes:
                c_name = c_tuple[0] or c_tuple[1]
                cl = c_name.lower().strip()
                allowed = class_weapons.get(cl, set()) or class_weapons.get(cl.replace(" ", ""), set())
                if curr_yaml_weapon not in allowed:
                    errors.append(f"{file_path}:{idx}: YAML block '{curr_yaml_weapon}' chứa class '{c_name}' không hợp lệ theo DECISIONS.md mục 7.")

    # 3. Structural Validation: game-concept.md Class Section
    if file_path.name == "game-concept.md":
        if "## 4. Hệ Thống" in content:
            has_deprecation_notice = "Đã thay thế bởi advanced-classes.md và DECISIONS.md" in content
            if not has_deprecation_notice:
                if "Song Rìu / Búa Tạ" in content or "CLASS ẨN (MYTHIC)" in content or "CLASS CƠ BẢN (NORMAL)" in content:
                    errors.append(f"{file_path}: Mục Class Architecture chưa được cập nhật theo DECISIONS.md hoặc thiếu ghi chú: 'Đã thay thế bởi advanced-classes.md và DECISIONS.md'.")

    # 4. Multiline SQL Schema Validation
    if "CREATE TABLE items" in content:
        uk_pattern = re.compile(
            r'CONSTRAINT\s+uk_owner_slot\s+UNIQUE\s*\([^)]+\)\s+DEFERRABLE\s+INITIALLY\s+DEFERRED',
            re.IGNORECASE | re.MULTILINE
        )
        if not uk_pattern.search(content):
            errors.append(f"{file_path}: Bảng 'items' thiếu hoặc khai báo sai ràng buộc 'CONSTRAINT uk_owner_slot UNIQUE (...) DEFERRABLE INITIALLY DEFERRED'.")

    return errors, warnings

def validate_all(project_root: Path):
    gdd_dir = project_root / "design" / "gdd"
    epics_dir = project_root / "production" / "epics"
    sprints_dir = project_root / "production" / "sprints"
    decisions_file = project_root / "production" / "DECISIONS.md"

    print("=" * 60)
    print("Project Ascendant: Dynamic GDD & Story Consistency Validator")
    print(f"Target GDD: {gdd_dir}")
    print(f"Target Stories: {epics_dir}, {sprints_dir}")
    print("=" * 60)

    if not decisions_file.exists():
        print(f"[ERROR] Master decisions file not found at: {decisions_file}")
        return 1

    try:
        decisions = load_decisions(decisions_file)
    except Exception as e:
        print(f"[ERROR] Failed to load decisions from {decisions_file}: {e}")
        return 1

    unique_class_count = len({c.replace(" ", "") for c in decisions["all_approved_classes"]})
    print(f"[INFO] Loaded {unique_class_count} classes across {len(decisions['approved_lines'])} lines from DECISIONS.md.")
    print(f"[INFO] Standard view directions: {decisions['view_direction_count']}.")
    print("=" * 60)

    target_files = []
    if gdd_dir.exists():
        target_files.extend(sorted(gdd_dir.glob("*.md")))
    if epics_dir.exists():
        target_files.extend(sorted(epics_dir.rglob("*.md")))
    if sprints_dir.exists():
        target_files.extend(sorted(sprints_dir.glob("*.md")))

    total_files = 0
    total_errors = 0
    total_warnings = 0

    for file_path in target_files:
        total_files += 1
        errors, warnings = check_file(file_path, decisions)

        if errors:
            rel_path = file_path.relative_to(project_root)
            print(f"\n[FAIL] {rel_path}:")
            for err in errors:
                print(f"  - {err}")
            total_errors += len(errors)
        elif warnings:
            rel_path = file_path.relative_to(project_root)
            print(f"\n[WARN] {rel_path}:")
            for warn in warnings:
                print(f"  - {warn}")
            total_warnings += len(warnings)
        else:
            rel_path = file_path.relative_to(project_root)
            print(f"[PASS] {rel_path}")

    print("\n" + "=" * 60)
    print(f"Validation Summary: {total_files} files checked.")
    print(f"Errors: {total_errors} | Warnings: {total_warnings}")
    print("=" * 60)

    if total_errors > 0:
        print("[RESULT] Consistency Check FAILED.")
        return 1
    else:
        print("[RESULT] Consistency Check PASSED.")
        return 0

if __name__ == "__main__":
    current_dir = Path(__file__).resolve().parent
    root = current_dir.parent.parent
    sys.exit(validate_all(root))
