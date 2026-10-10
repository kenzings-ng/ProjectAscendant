#!/usr/bin/env python3
"""
Project Ascendant - GDD & Story Consistency Validator
Validates design documents, stories, sprint plans, architecture docs and the
entity registry against production/DECISIONS.md.

Performs deep structural, relationship, terminology, schema, and frame calculation validation.
All approved classes, lines, ranks, weapon families, and direction counts are dynamically loaded
from DECISIONS.md (no hardcoded class names or weapons).

SCAN SCOPE (X13, 2026-10-10) -- see SCAN_TARGETS:
  - design/gdd/*.md
  - production/epics/**/*.md
  - production/sprints/*.md
  - production/sprint-status.yaml
  - docs/architecture/*.md, docs/architecture/*.yaml
  - design/registry/*.yaml

SKIPPED HISTORICAL / DATED RECORDS -- see SKIP_PATH_PREFIXES / SKIP_NAME_PATTERNS:
  These files are snapshots of the past (review reports, gate checks, plans,
  progress logs, retrospectives). They quote old terms on purpose and must not be
  rewritten, so they are never scanned:
  - production/qa/**, production/plans/**, production/retrospectives/**
  - production/PROGRESS.md
  - any file named gdd-cross-review-*, *-review-YYYY-MM-DD*, gate-check-*, *retrospective*

DEPRECATION CONTEXT (narrow skip):
  A banned term is only accepted when the SAME LINE explicitly marks that term as
  deprecated / removed / banned near the match (see is_marked_deprecated). A line
  that merely contains the word "deprecated" somewhere does not hide other matches.
  YAML registry entries with "status: deprecated" are also treated as marked.

ALLOWLIST:
  Tools/QA/gdd_validator_allowlist.yaml lists individual known findings that need an
  owner decision (file + check id + line regex + reason). Allowlisted findings are
  printed as [ALLOW]; an allowlist entry that no longer matches anything is an error,
  so entries cannot silently outlive the problem they describe.
"""

import json
import re
import sys
from pathlib import Path
from typing import Dict, List, Optional, Tuple

# ---------------------------------------------------------------------------
# Scan scope
# ---------------------------------------------------------------------------

# (directory relative to project root, glob pattern)
SCAN_TARGETS = [
    ("design/gdd", "*.md"),
    ("production/epics", "**/*.md"),
    ("production/sprints", "*.md"),
    ("production", "sprint-status.yaml"),
    ("docs/architecture", "*.md"),
    ("docs/architecture", "*.yaml"),
    ("design/registry", "*.yaml"),
]

# Historical / dated records that are never scanned (path prefixes, posix, relative to root).
SKIP_PATH_PREFIXES = [
    "production/qa/",
    "production/plans/",
    "production/retrospectives/",
    "production/PROGRESS.md",
]

# Historical / dated records that are never scanned (file-name regexes).
SKIP_NAME_PATTERNS = [
    re.compile(r"^gdd-cross-review-"),
    re.compile(r"-review-\d{4}-\d{2}-\d{2}"),
    re.compile(r"^gate-check-"),
    re.compile(r"retrospective", re.IGNORECASE),
]

DEFAULT_ALLOWLIST = Path("Tools/QA/gdd_validator_allowlist.yaml")

EXPECTED_CLASS_COUNT = 16
EXPECTED_LINE_COUNT = 5

# Metadata tags allowed
ALLOWED_META_TAGS = {
    "Class.Primary",
    "Class.Secondary",
    "Class.Identity",
    "Class.Slot",
}

# ---------------------------------------------------------------------------
# Deprecation context (narrow skip)
# ---------------------------------------------------------------------------

# Strong markers: accepted anywhere within CONTEXT_WINDOW characters of the match on the same line.
STRONG_DEPRECATION_MARKERS = re.compile(
    r"(?:bị\s+cấm"
    r"|(?<!\w)cấm(?!\s+[đĐ][ịi]a)(?!\w)"          # "cấm" but not the place name "Cấm Địa"
    r"|deprecated|removed|renamed\s+from|replaced_by"
    r"|không\s+được\s+duyệt|không\s+phải\s+độ\s+hiếm"
    r"|trước\s+2026-10-09"
    r"|Cập\s+nhật\s+2026-\d{2}-\d{2}\s*\(X\d+[a-z]?\)"   # dated X-task note, e.g. "(Cập nhật 2026-10-10 (X7): ...)"
    r"|\bX\d+[a-z]?\s+2026-\d{2}-\d{2}"                 # "# X7 2026-10-10: renamed ..."
    r"|\bX\d+[a-z]?\s+note\b)",
    re.IGNORECASE,
)
# Weak markers: verbs of removal/replacement, accepted only shortly BEFORE the match.
WEAK_DEPRECATION_MARKERS = re.compile(
    r"(?<!\w)(?:bỏ|loại\s+bỏ|xóa|thay\s+bằng|thay\s+vì|thay\s+cho|thay\s+thế|không\s+dùng|không\s+tạo|không\s+còn|đổi)(?!\w)",
    re.IGNORECASE,
)
CONTEXT_WINDOW = 160
WEAK_PRE_WINDOW = 15


def is_marked_deprecated(line: str, start: int, end: int) -> bool:
    """True when the same line explicitly marks the matched term as deprecated/removed/banned."""
    lo = max(0, start - CONTEXT_WINDOW)
    hi = min(len(line), end + CONTEXT_WINDOW)
    if STRONG_DEPRECATION_MARKERS.search(line[lo:hi]):
        return True
    # A weak marker counts only when it ENDS at most WEAK_PRE_WINDOW characters before the match.
    pre_lo = max(0, start - WEAK_PRE_WINDOW - 40)
    for m in WEAK_DEPRECATION_MARKERS.finditer(line, pre_lo, start):
        if start - m.end() <= WEAK_PRE_WINDOW:
            return True
    return False


def yaml_deprecated_lines(lines: List[str]) -> set:
    """1-based line numbers inside YAML list items that declare 'status: deprecated'."""
    result = set()
    i = 0
    n = len(lines)
    while i < n:
        m = re.match(r"^(\s*)- ", lines[i])
        if not m:
            i += 1
            continue
        indent = len(m.group(1))
        j = i + 1
        while j < n:
            s = lines[j]
            if s.strip() == "" or s.lstrip().startswith("#"):
                j += 1
                continue
            cur_indent = len(s) - len(s.lstrip())
            if cur_indent <= indent:  # next sibling item or a parent key ends this item
                break
            j += 1
        block = lines[i:j]
        if any(re.match(r"^\s*status:\s*deprecated\b", b) for b in block):
            result.update(range(i + 1, j + 1))
        i = j if j > i else i + 1
    return result


# ---------------------------------------------------------------------------
# Banned-term rules (apply to every file in scope)
# ---------------------------------------------------------------------------

_RARITY_NAMES = r"(?:Common|Uncommon|Rare|Epic|Legendary|Normal|Mythic|Divine|Immortal)"
_TIER_WORD = r"(?:[Tt]ier|[Bb]ậc)"
_FORGE_WORDS = r"(?:Forge|Blacksmith|Lò|Thợ|Outpost|Field|Ancient|Forbidden)"

# (check_id, compiled regex, message). Each match is reported unless marked deprecated on the same line.
BANNED_TERM_RULES = [
    ("ash_shards",
     # Also catches identifiers: AshShards, AshShardsBalance, ash_shards_carry_limit.
     re.compile(r"(?<![A-Za-z])(?i:ash[\s_-]?shards?)(?![a-z])"),
     "CẤM: 'Ash Shards' dùng làm tiền tệ. Quyển trục phân rã thành 'Tàn Trang' (Skill Shards / item_skill_shard) — DECISIONS.md §5, §12."),
    ("chain_whip",
     re.compile(r"roi\s+x[ií]ch|chain[\s_-]?whip", re.IGNORECASE),
     "CẤM: Roi xích cho Inquisitor. Inquisitor chỉ dùng Weapon.1H.Mace (Chùy 1 tay) — DECISIONS.md §7."),
    ("banned_class",
     re.compile(r"\bvoid[\s_-]?weaver\b", re.IGNORECASE),
     "CẤM: Class 'Void Weaver' không được duyệt. Tổng 15 class + 1 Apex = 16 class."),
    ("banned_class",
     re.compile(r"\boracle\b", re.IGNORECASE),
     "CẤM: Class 'Oracle' không được duyệt. Tổng 15 class + 1 Apex = 16 class."),
    # Divine / Immortal used as a rarity (DECISIONS.md §1, §5). Only rarity-shaped contexts are matched,
    # so proper names such as the skill "Divine Siphon" are not flagged.
    ("divine_immortal_rarity",
     re.compile(r"\b(?:trang\s+bị\s+(?:Immortal|Divine)|đúc\s+đồ\s+Immortal|Immortal\s*\([Đđ]ỏ\)|Divine\s*\([Hh]oàng\s+kim\))", re.IGNORECASE),
     "CẤM: Trang bị 'Immortal' / 'Divine'. Thang độ hiếm trang bị chỉ gồm 5 độ hiếm (Common → Legendary) theo DECISIONS.md §5."),
    ("divine_immortal_rarity",
     re.compile(r"\b(?:Immortal|Divine)\s*(?:[/&,]|\bvà\b|\bhoặc\b|\bhay\b)\s*(?:Immortal|Divine)\b", re.IGNORECASE),
     "CẤM: 'Immortal/Divine' như độ hiếm. Độ hiếm trang bị: Common → Legendary; kỹ năng: Normal → Mythic (DECISIONS.md §5)."),
    ("divine_immortal_rarity",
     re.compile(r"\b(?:Divine|Immortal)\s+(?:Equipment|Gear|Items?|Weapons?|Rarity|Tier|Bậc)\b", re.IGNORECASE),
     "CẤM: 'Divine/Immortal' dùng làm độ hiếm. Thang độ hiếm theo DECISIONS.md §5."),
    ("divine_immortal_rarity",
     re.compile(r"\b(?:độ\s+hiếm|rarity|Tier\s*\d*|Bậc\s*\d*)\s+(?:Divine|Immortal)\b", re.IGNORECASE),
     "CẤM: 'Divine/Immortal' dùng làm độ hiếm. Thang độ hiếm theo DECISIONS.md §5."),
    ("divine_immortal_rarity",
     re.compile(r"\b(?:Legendary|Epic|Mythic)\s*(?:→|->|\$\\rightarrow\$|,|/|&)\s*(?:Divine|Immortal)\b", re.IGNORECASE),
     "CẤM: Thêm bậc 'Divine/Immortal' sau Legendary/Mythic. Thang độ hiếm theo DECISIONS.md §5."),
    ("divine_immortal_rarity",
     re.compile(r"\bdivine[\s_-]+(?:crafting|forging)\b", re.IGNORECASE),
     "CẤM: 'divine crafting/forging' — đồ đúc từ Linh Hồn Boss là Legendary, 'Divine' không phải độ hiếm (DECISIONS.md §1, §5)."),
    # Rarity named / numbered with "Tier N" / "Bậc N" (DECISIONS.md §1). Same context rules as the X7 sweep:
    # forge tier ("Tier 2 Forge"), backpack tier ("túi đồ Bậc 1"), zone tier and class rank ("Bậc T2", "Bậc 2/3"
    # without a rarity name next to it) are legitimate and are not matched.
    ("rarity_tier_numbering",
     # "Quyển Trục Bậc 2 (Rare Scroll)" is the scroll's class rank (DECISIONS.md §5 "Quyển Trục T2"), not a rarity number.
     re.compile(r"(?<!Quyển Trục )(?<!Quyển trục )(?<!quyển trục )\b" + _TIER_WORD + r"\s*[0-9]+\s*(?:[:(\-–]\s*)?\**\s*" + _RARITY_NAMES + r"\b"),
     "CẤM: Độ hiếm đánh số bằng 'Tier N'/'Bậc N'. Độ hiếm gọi bằng tên (DECISIONS.md §1)."),
    ("rarity_tier_numbering",
     re.compile(_RARITY_NAMES + r"\**\s*\(\s*" + _TIER_WORD + r"\s*[0-9]+(?!\s*" + _FORGE_WORDS + r")"),
     "CẤM: Độ hiếm kèm '(Tier N)'/'(Bậc N)'. Độ hiếm gọi bằng tên (DECISIONS.md §1)."),
    ("rarity_tier_numbering",
     re.compile(r"\b" + _TIER_WORD + r"\s+(?:Common|Uncommon|Rare|Epic|Legendary|Divine|Immortal)\b"),
     "CẤM: 'Bậc/Tier <Độ hiếm>'. Dùng 'độ hiếm <tên>' (DECISIONS.md §1)."),
    ("rarity_tier_numbering",
     # Prose only: the code identifier `RarityTier` (UPAItemStaticDataAsset::RarityTier, type EPAItemRarity) is not matched.
     re.compile(r"\bbậc\s+hiếm\b|\b[0-9]+[\s-]*Tier\s+Rarity\b|\brarity\s+tier\b"
                r"|\b(?:độ\s+hiếm|rarity)\s+[0-9]+[\s-]*Tier\b|\b(?:vật\s+phẩm|trang\s+bị)\s+[0-9]+[\s-]*Tier\b", re.IGNORECASE),
     "CẤM: 'Bậc Hiếm' / 'Tier Rarity' / 'độ hiếm N Tier'. Dùng 'Độ Hiếm (Rarity)' (DECISIONS.md §1)."),
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

# Italic parenthetical notes in DECISIONS.md §7 that describe a weapon still under review,
# e.g. "*(Lưu ý: ... còn ghi thêm `Weapon.2H.Heavy`, đang trình duyệt xác nhận)*". Tags inside them are
# NOT approved and must not be loaded as allowed weapons.
PENDING_NOTE_PATTERN = re.compile(r"\*?\([^()]*?(?:đang\s+trình\s+duyệt|pending)[^()]*\)\*?", re.IGNORECASE)


class DecisionsParseError(Exception):
    """DECISIONS.md could not be parsed into the expected 16 classes / 5 lines."""


def load_decisions(decisions_path: Path) -> Dict:
    """
    Dynamically loads approved classes, lines, ranks, weapon families,
    and view direction count from production/DECISIONS.md.
    No hardcoded class lists or weapon lists.
    Raises DecisionsParseError unless exactly 16 classes in 5 lines (with weapon mappings) are parsed.
    """
    if not decisions_path.exists():
        raise FileNotFoundError(f"DECISIONS.md not found at {decisions_path}")

    with open(decisions_path, "r", encoding="utf-8") as f:
        text = f.read()

    class_to_line = {}
    class_to_rank = {}
    all_approved_classes = set()
    approved_lines = set()
    canonical_classes = set()

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
                    canonical_classes.add(cl)
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
                rest = PENDING_NOTE_PATTERN.sub("", m.group(3))
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

    # 4. Fail loudly when the parse does not match the locked 16-class / 5-line structure.
    problems = []
    if len(canonical_classes) != EXPECTED_CLASS_COUNT:
        problems.append(f"parsed {len(canonical_classes)} classes from §2, expected exactly {EXPECTED_CLASS_COUNT}: {sorted(canonical_classes)}")
    if len(approved_lines) != EXPECTED_LINE_COUNT:
        problems.append(f"parsed {len(approved_lines)} lines from §2, expected exactly {EXPECTED_LINE_COUNT}: {sorted(approved_lines)}")
    missing_weapons = sorted(c for c in canonical_classes if not class_weapons.get(c))
    if missing_weapons:
        problems.append(f"§7 has no weapon family for: {missing_weapons}")
    if problems:
        raise DecisionsParseError("DECISIONS.md parse failed: " + "; ".join(problems))

    return {
        "class_to_line": class_to_line,
        "class_to_rank": class_to_rank,
        "all_approved_classes": all_approved_classes,
        "canonical_classes": canonical_classes,
        "approved_lines": approved_lines,
        "class_weapons": class_weapons,
        "view_direction_count": view_direction_count,
    }


# ---------------------------------------------------------------------------
# Allowlist (minimal YAML subset reader: stdlib only, no PyYAML dependency)
# ---------------------------------------------------------------------------

def _parse_scalar(raw: str) -> str:
    raw = raw.strip()
    if raw.startswith('"'):
        return json.loads(raw)
    if raw.startswith("'") and raw.endswith("'"):
        return raw[1:-1].replace("''", "'")
    return raw


def load_allowlist(path: Path) -> List[Dict[str, str]]:
    """
    Reads Tools/QA/gdd_validator_allowlist.yaml. Supported subset:
        entries:
          - file: <relative path>
            check: <check id>
            pattern: "<python regex searched in the line text>"
            reason: "<owner question>"
    Comments (#) and blank lines are ignored. 'entries: []' means empty.
    """
    if not path.exists():
        return []
    entries: List[Dict[str, str]] = []
    current: Optional[Dict[str, str]] = None
    for lineno, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        s = raw.strip()
        if not s or s.startswith("#"):
            continue
        if re.match(r"^entries:\s*(\[\s*\])?\s*$", s):
            continue
        m = re.match(r"^(-\s+)?([a-z_]+):\s*(.*)$", s)
        if not m:
            raise ValueError(f"{path}:{lineno}: unsupported allowlist syntax: {raw!r}")
        if m.group(1):
            current = {}
            entries.append(current)
        if current is None:
            raise ValueError(f"{path}:{lineno}: key outside of a list entry: {raw!r}")
        current[m.group(2)] = _parse_scalar(m.group(3))
    for i, e in enumerate(entries, 1):
        for key in ("file", "check", "pattern", "reason"):
            if not e.get(key):
                raise ValueError(f"{path}: allowlist entry #{i} is missing '{key}'")
        e["_regex"] = re.compile(e["pattern"])
    return entries


# ---------------------------------------------------------------------------
# Per-file checks
# ---------------------------------------------------------------------------

Finding = Tuple[int, str, str]  # (line number or 0 for whole-file, check id, message)


def check_file(file_path: Path, decisions: Dict) -> Tuple[List[Finding], List[str]]:
    """Returns (findings, warnings). Findings are (line_no, check_id, message)."""
    findings: List[Finding] = []
    warnings: List[str] = []

    with open(file_path, "r", encoding="utf-8", errors="replace") as f:
        content = f.read()

    lines = content.splitlines()

    class_to_line = decisions["class_to_line"]
    approved_lines = decisions["approved_lines"]
    all_approved_classes = decisions["all_approved_classes"]
    class_weapons = decisions["class_weapons"]
    expected_dirs = decisions["view_direction_count"]

    deprecated_yaml_lines = yaml_deprecated_lines(lines) if file_path.suffix in (".yaml", ".yml") else set()

    def add(line_idx: int, check_id: str, msg: str):
        findings.append((line_idx, check_id, msg))

    # 1. Line-by-line checks (every file in scope)
    for line_idx, line in enumerate(lines, 1):
        in_deprecated_entry = line_idx in deprecated_yaml_lines

        def unmarked(m) -> bool:
            return not in_deprecated_entry and not is_marked_deprecated(line, m.start(), m.end())

        # Banned terms
        for check_id, pat, msg in BANNED_TERM_RULES:
            for m in pat.finditer(line):
                if unmarked(m):
                    add(line_idx, check_id, f"{msg} [match: '{m.group(0)}']")
                    break

        # Frame Calculation Direction Count
        m_frame_calc = re.search(r'(?:\\times|[\$x×*]|\bnhân\b)\s*(\d+)\s*\$?\s*(?:hướng|directions?)\b', line, re.IGNORECASE)
        if not m_frame_calc:
            m_frame_calc = re.search(r'(\d+)\s*\$?\s*(?:hướng|directions?)\s*(?:\\times|[\$x×*]|\bnhân\b)', line, re.IGNORECASE)
        if m_frame_calc and unmarked(m_frame_calc):
            dirs_found = int(m_frame_calc.group(1))
            if dirs_found != expected_dirs:
                add(line_idx, "frame_directions", f"Phép tính frame sai số hướng nhìn: Dùng '{dirs_found} hướng'. Theo DECISIONS.md Mục 10, nhân vật dùng {expected_dirs} hướng nhìn.")

        # Obsolete 12 Class count
        m12 = re.search(r'\b12\s+(?:Class|Chức\s+nghiệp)\b', line, re.IGNORECASE)
        if m12 and unmarked(m12):
            add(line_idx, "class_count_12", "Lỗi số lượng: Còn sót '12 Class' / '12 Chức nghiệp'. Tổng số class chính thức là 16 Class theo DECISIONS.md Mục 2.")

        # Prohibited Class Tier / Rarity Terminology
        for pat, err_msg in CLASS_TIER_RARITY_PATTERNS:
            m = pat.search(line)
            if m and unmarked(m):
                add(line_idx, "class_tier_rarity", err_msg)

        # GameplayTags: only Class.Line.<Line>.<Class> (plus metadata tags) are allowed.
        for m_tag in re.finditer(r'\bClass\.[A-Za-z0-9_.*<]+', line):
            raw_tag = m_tag.group(0)
            tag = raw_tag.rstrip(".:,;()[]\"'`")
            if not tag or tag in ALLOWED_META_TAGS:
                continue
            # Documentation placeholders / wildcards for the approved namespace, e.g. Class.Line.<Nhánh>.<Class>, Class.Line.*
            if tag in ("Class.Line", "Class.Line.*") or tag.startswith("Class.Line.<") or tag.startswith("Class.Line.*"):
                continue
            if not unmarked(m_tag):
                continue

            if tag.startswith("Class.Line."):
                parts = tag.split(".")
                if len(parts) != 4:
                    add(line_idx, "legacy_class_tag", f"Tag '{tag}' sai định dạng. Cấu trúc chuẩn: 'Class.Line.<Nhánh>.<Class>'.")
                    continue

                line_name = parts[2]
                class_name = parts[3]
                class_lower = class_name.lower()

                if line_name not in approved_lines:
                    add(line_idx, "legacy_class_tag", f"Tag '{tag}' chứa nhánh '{line_name}' không hợp lệ. Phải là một trong: {sorted(approved_lines)}.")
                    continue

                if class_lower not in all_approved_classes:
                    add(line_idx, "legacy_class_tag", f"Tag '{tag}' chứa class '{class_name}' không tồn tại trong danh mục class đã chốt.")
                    continue

                expected_line = class_to_line.get(class_lower)
                if expected_line and line_name != expected_line:
                    add(line_idx, "legacy_class_tag", f"Tag '{tag}' mâu thuẫn: Class '{class_name}' thuộc nhánh '{expected_line}', không phải '{line_name}'.")
            else:
                add(line_idx, "legacy_class_tag", f"Tag '{tag}' sai cấu trúc hoặc lỗi thời (Class.<Name> / Class.TierX / Class.RankX). Toàn bộ GameplayTag chức nghiệp bắt buộc dùng chuẩn 'Class.Line.<Nhánh>.<Class>' (DECISIONS.md §8).")

        # Column name error
        if "item_instance_instance_id" in line:
            add(line_idx, "column_name", "Lỗi tên cột 'item_instance_instance_id', phải là 'item_instance_id'.")

        # Prohibited equipment tiers
        m_eq = re.search(r'\b(?:ShieldTier|5-Tier\s+Item)\b', line, re.IGNORECASE)
        if m_eq and unmarked(m_eq):
            add(line_idx, "equipment_tier", "CẤM: Dùng từ 'Tier' cho trang bị/khiên. Trang bị dùng thang 'Độ Hiếm' (Rarity: Common -> Legendary) theo DECISIONS.md Mục 1.")

    # 2. General Table & YAML Weapon Family Relationship Validation
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
                for c in all_classes_sorted:
                    c_pat = re.compile(r"\b" + re.escape(c) + r"\b", re.IGNORECASE)
                    if c_pat.search(line):
                        allowed = class_weapons.get(c, set())
                        if matched_weap not in allowed:
                            add(idx, "weapon_relation", f"Quan hệ vũ khí sai: Class '{c.title()}' không được phép sử dụng dòng vũ khí '{matched_weap}' theo DECISIONS.md mục 7.")

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
                    add(idx, "weapon_relation", f"YAML block '{curr_yaml_weapon}' chứa class '{c_name}' không hợp lệ theo DECISIONS.md mục 7.")

    # 3. Structural Validation: game-concept.md Class Section
    if file_path.name == "game-concept.md":
        if "## 4. Hệ Thống" in content:
            has_deprecation_notice = "Đã thay thế bởi advanced-classes.md và DECISIONS.md" in content
            if not has_deprecation_notice:
                if "Song Rìu / Búa Tạ" in content or "CLASS ẨN (MYTHIC)" in content or "CLASS CƠ BẢN (NORMAL)" in content:
                    add(0, "game_concept", "Mục Class Architecture chưa được cập nhật theo DECISIONS.md hoặc thiếu ghi chú: 'Đã thay thế bởi advanced-classes.md và DECISIONS.md'.")

    # 4. Multiline SQL Schema Validation
    if "CREATE TABLE items" in content:
        uk_pattern = re.compile(
            r'CONSTRAINT\s+uk_owner_slot\s+UNIQUE\s*\([^)]+\)\s+DEFERRABLE\s+INITIALLY\s+DEFERRED',
            re.IGNORECASE | re.MULTILINE
        )
        if not uk_pattern.search(content):
            add(0, "sql_schema", "Bảng 'items' thiếu hoặc khai báo sai ràng buộc 'CONSTRAINT uk_owner_slot UNIQUE (...) DEFERRABLE INITIALLY DEFERRED'.")

    return findings, warnings


# ---------------------------------------------------------------------------
# Driver
# ---------------------------------------------------------------------------

def is_skipped(rel_posix: str) -> bool:
    if any(rel_posix == p or rel_posix.startswith(p) for p in SKIP_PATH_PREFIXES):
        return True
    name = rel_posix.rsplit("/", 1)[-1]
    return any(p.search(name) for p in SKIP_NAME_PATTERNS)


def collect_target_files(project_root: Path) -> Tuple[List[Path], List[Path]]:
    """Returns (files to scan, skipped historical files), both sorted and de-duplicated."""
    targets: List[Path] = []
    skipped: List[Path] = []
    seen = set()
    for rel_dir, pattern in SCAN_TARGETS:
        base = project_root / rel_dir
        if not base.exists():
            continue
        for p in sorted(base.glob(pattern)):
            if not p.is_file() or p in seen:
                continue
            seen.add(p)
            rel = p.relative_to(project_root).as_posix()
            (skipped if is_skipped(rel) else targets).append(p)
    return targets, skipped


def validate_all(project_root: Path, allowlist_path: Optional[Path] = None) -> int:
    decisions_file = project_root / "production" / "DECISIONS.md"
    if allowlist_path is None:
        allowlist_path = project_root / DEFAULT_ALLOWLIST

    print("=" * 60)
    print("Project Ascendant: Dynamic GDD & Story Consistency Validator")
    print("Scope: " + ", ".join(f"{d}/{g}" for d, g in SCAN_TARGETS))
    print("=" * 60)

    if not decisions_file.exists():
        print(f"[ERROR] Master decisions file not found at: {decisions_file}")
        return 1

    try:
        decisions = load_decisions(decisions_file)
    except Exception as e:
        print(f"[ERROR] Failed to load decisions from {decisions_file}: {e}")
        return 1

    try:
        allowlist = load_allowlist(allowlist_path)
    except Exception as e:
        print(f"[ERROR] Failed to load allowlist {allowlist_path}: {e}")
        return 1

    print(f"[INFO] Loaded {len(decisions['canonical_classes'])} classes across {len(decisions['approved_lines'])} lines from DECISIONS.md (asserted {EXPECTED_CLASS_COUNT}/{EXPECTED_LINE_COUNT}).")
    print(f"[INFO] Standard view directions: {decisions['view_direction_count']}.")
    print(f"[INFO] Allowlist entries: {len(allowlist)} ({allowlist_path}).")

    target_files, skipped_files = collect_target_files(project_root)
    for p in skipped_files:
        print(f"[SKIP] {p.relative_to(project_root).as_posix()} (historical/dated record)")
    print("=" * 60)

    total_files = 0
    total_errors = 0
    total_warnings = 0
    total_allowed = 0
    used_allow = set()

    for file_path in target_files:
        total_files += 1
        rel = file_path.relative_to(project_root).as_posix()
        findings, warnings = check_file(file_path, decisions)
        file_lines = file_path.read_text(encoding="utf-8", errors="replace").splitlines()

        errors = []
        allowed = []
        for line_no, check_id, msg in findings:
            text = file_lines[line_no - 1] if line_no > 0 else ""
            hit = None
            for i, e in enumerate(allowlist):
                if e["file"] == rel and e["check"] == check_id and e["_regex"].search(text):
                    hit = i
                    break
            loc = f"{rel}:{line_no}" if line_no else rel
            if hit is not None:
                used_allow.add(hit)
                allowed.append(f"{loc}: [{check_id}] {msg} -- allowlisted: {allowlist[hit]['reason']}")
            else:
                errors.append(f"{loc}: [{check_id}] {msg}")

        if errors:
            print(f"\n[FAIL] {rel}:")
            for err in errors:
                print(f"  - {err}")
            total_errors += len(errors)
        elif warnings:
            print(f"\n[WARN] {rel}:")
            for warn in warnings:
                print(f"  - {warn}")
            total_warnings += len(warnings)
        else:
            print(f"[PASS] {rel}")
        for a in allowed:
            print(f"  [ALLOW] {a}")
        total_allowed += len(allowed)

    for i, e in enumerate(allowlist):
        if i not in used_allow:
            print(f"\n[FAIL] Stale allowlist entry #{i + 1} ({e['file']}, {e['check']}, /{e['pattern']}/) matches no finding; remove it.")
            total_errors += 1

    print("\n" + "=" * 60)
    print(f"Validation Summary: {total_files} files checked, {len(skipped_files)} historical files skipped.")
    print(f"Errors: {total_errors} | Warnings: {total_warnings} | Allowlisted: {total_allowed}")
    print("=" * 60)

    if total_errors > 0:
        print("[RESULT] Consistency Check FAILED.")
        return 1
    print("[RESULT] Consistency Check PASSED.")
    return 0


if __name__ == "__main__":
    current_dir = Path(__file__).resolve().parent
    root = current_dir.parent.parent
    sys.exit(validate_all(root))
