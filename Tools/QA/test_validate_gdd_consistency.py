#!/usr/bin/env python3
"""
Self-test for Tools/QA/validate_gdd_consistency.py (X13, 2026-10-10).

Stdlib unittest only. Proves that each check catches a violation, that legitimate
forge / zone / backpack / class-rank tiers and Class.Line tags pass, that the
deprecation skip is narrow, that DECISIONS.md parsing is asserted, and that the
scan scope / skip list / allowlist behave as documented.

Run: python Tools/QA/test_validate_gdd_consistency.py
"""

import contextlib
import io
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO_ROOT = HERE.parent.parent
sys.path.insert(0, str(HERE))

import validate_gdd_consistency as v  # noqa: E402

DECISIONS = REPO_ROOT / "production" / "DECISIONS.md"


class TempProject:
    """A throw-away project root containing a copy of the real DECISIONS.md."""

    def __init__(self):
        self.dir = tempfile.TemporaryDirectory()
        self.root = Path(self.dir.name)
        (self.root / "production").mkdir()
        shutil.copy(DECISIONS, self.root / "production" / "DECISIONS.md")

    def write(self, rel: str, text: str) -> Path:
        p = self.root / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(text, encoding="utf-8")
        return p

    def run(self, allowlist: str = None):
        al = None
        if allowlist is not None:
            al = self.write("Tools/QA/allowlist.yaml", allowlist)
        else:
            al = self.root / "no-allowlist.yaml"
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            code = v.validate_all(self.root, al)
        return code, buf.getvalue()

    def close(self):
        self.dir.cleanup()


DECISIONS_CACHE = None


def decisions():
    global DECISIONS_CACHE
    if DECISIONS_CACHE is None:
        DECISIONS_CACHE = v.load_decisions(DECISIONS)
    return DECISIONS_CACHE


def findings_for(text: str, name: str = "story-999-fixture.md"):
    with tempfile.TemporaryDirectory() as d:
        p = Path(d) / name
        p.write_text(text + "\n", encoding="utf-8")
        found, _ = v.check_file(p, decisions())
    return found


def check_ids(text: str, name: str = "story-999-fixture.md"):
    return {cid for _, cid, _ in findings_for(text, name)}


class DecisionsParsingTests(unittest.TestCase):
    def test_real_decisions_parse_16_classes_5_lines(self):
        d = decisions()
        self.assertEqual(len(d["canonical_classes"]), 16)
        self.assertEqual(d["approved_lines"], {"Guard", "Scout", "Caster", "Faith", "Apex"})

    def test_pending_weapon_note_is_ignored(self):
        # DECISIONS.md:122 -- Dragon Knight: Polearm approved, 2H.Heavy only "đang trình duyệt".
        self.assertEqual(decisions()["class_weapons"]["dragon knight"], {"2h_polearm"})
        # Non-pending sub-set note is still honoured (Ranger dual daggers).
        self.assertIn("dual_daggers", decisions()["class_weapons"]["ranger"])

    def test_missing_class_fails_loudly(self):
        text = DECISIONS.read_text(encoding="utf-8").replace(", `Swordmaster` (Kiếm Sư)", "", 1)
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "DECISIONS.md"
            p.write_text(text, encoding="utf-8")
            with self.assertRaises(v.DecisionsParseError):
                v.load_decisions(p)

    def test_missing_line_fails_loudly(self):
        text = DECISIONS.read_text(encoding="utf-8").replace("(Faith Line", "(Faith Group", 1)
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "DECISIONS.md"
            p.write_text(text, encoding="utf-8")
            with self.assertRaises(v.DecisionsParseError):
                v.load_decisions(p)

    def test_validate_all_exits_1_on_bad_decisions(self):
        tp = TempProject()
        try:
            dp = tp.root / "production" / "DECISIONS.md"
            dp.write_text(dp.read_text(encoding="utf-8").replace("`Seraph`", "Seraph", 1), encoding="utf-8")
            code, out = tp.run()
            self.assertEqual(code, 1)
            self.assertIn("expected exactly 16", out)
        finally:
            tp.close()


class BannedTermTests(unittest.TestCase):
    def assertCaught(self, text, check_id):
        self.assertIn(check_id, check_ids(text), f"expected [{check_id}] for: {text!r}")

    def assertClean(self, text):
        self.assertEqual(findings_for(text), [], f"expected no finding for: {text!r}")

    def test_ash_shards(self):
        self.assertCaught("Phân rã quyển trục thành 10 Ash Shards.", "ash_shards")
        self.assertCaught("Server cộng `AshShardsBalance` sau khi phân rã.", "ash_shards")
        self.assertCaught("max_ash_shards: 99999", "ash_shards")
        # Review fix 1: CamelCase-embedded identifiers.
        self.assertCaught("int32 GetSalvageAshShards() const;", "ash_shards")
        self.assertCaught("struct FAshShardsWallet;", "ash_shards")
        self.assertCaught("UPROPERTY() int32 Ash_Shards;", "ash_shards")

    def test_divine_immortal_rarity(self):
        self.assertCaught("Rèn trang bị Divine (Hoàng kim) tại lò.", "divine_immortal_rarity")
        self.assertCaught("# Story 003: Boss Soul Forging & Divine Equipment", "divine_immortal_rarity")
        self.assertCaught("Lỗ khảm thứ 3 cho đồ Immortal & Divine.", "divine_immortal_rarity")
        self.assertCaught("Thang màu Common → Legendary → Divine.", "divine_immortal_rarity")
        self.assertCaught("Only the forge can forge Divine Tier 5 equipment.", "divine_immortal_rarity")
        self.assertCaught("Boss Soul divine forging.", "divine_immortal_rarity")
        # Review fix 6: widened forms.
        self.assertCaught("Unlocks Divine-tier gear.", "divine_immortal_rarity")
        self.assertCaught("An Immortal-grade drop.", "divine_immortal_rarity")
        self.assertCaught("Boss rơi đồ Divine.", "divine_immortal_rarity")
        self.assertCaught("Vật phẩm hạng Immortal.", "divine_immortal_rarity")
        self.assertCaught("Normal (Xám), Immortal (Đỏ `#DC2626`), Divine (Hoàng Kim `#F59E0B`).", "divine_immortal_rarity")

    def test_rarity_numbered_with_tier_or_bac(self):
        self.assertCaught("Thanh Thần Binh Bậc 5 (Legendary) phát sáng.", "rarity_tier_numbering")
        self.assertCaught("Salvaging a Rare (Tier 3) chestplate yields shards.", "rarity_tier_numbering")
        self.assertCaught("Đục lỗ trên trang bị Bậc Rare và Epic.", "rarity_tier_numbering")
        self.assertCaught("Hệ thống 5 Bậc Hiếm chuẩn RPG.", "rarity_tier_numbering")
        self.assertCaught("Shards based on rarity tier (Common: 1).", "rarity_tier_numbering")
        self.assertCaught("Phân loại độ hiếm 5 Tier.", "rarity_tier_numbering")
        self.assertCaught("Epic: Inventory & 5-Tier Item Database", "equipment_tier")
        self.assertCaught("Supports 5-Tier Items in the grid.", "equipment_tier")
        # Review fix 6: separators, roman numerals, brackets, "T5", "<Rarity> Tier", "Rarity: Tier N".
        for text in [
            "Drops a Tier-5 Legendary sword.",
            "Drops a Tier V Legendary sword.",
            "Tier 5 — Legendary weapons glow.",
            "Tier 4: Epic",
            "Rarity: Tier 5",
            "A Legendary (T5) blade.",
            "An Epic [Tier 4] ring.",
            "Skill Book (Mythic Tier).",
            "Boss drops a Rare Tier Ring.",
        ]:
            self.assertCaught(text, "rarity_tier_numbering")

    def test_legacy_class_tags(self):
        self.assertCaught("`RequiredClassTag = Class.Vanguard`", "legacy_class_tag")
        self.assertCaught("Tag `Class.Tier2.Templar` cho T2.", "legacy_class_tag")
        self.assertCaught("Tag `Class.Rank3.Seraph` cho T3.", "legacy_class_tag")
        self.assertCaught("Tag `Class.Line.Guard.Ranger` sai nhánh.", "legacy_class_tag")
        self.assertCaught("Tag `Class.Line.Guard.Paladin` không có trong DECISIONS.", "legacy_class_tag")

    def test_banned_classes(self):
        self.assertCaught("Class mới: Void Weaver.", "banned_class")
        self.assertCaught("Nhánh Faith thêm Oracle.", "banned_class")
        self.assertCaught("Two Oracles join the party.", "banned_class")

    def test_other_line_checks(self):
        self.assertCaught("Ma Trận 12 Class & Action Deck", "class_count_12")
        self.assertCaught("Supports 12 Classes.", "class_count_12")
        self.assertCaught("The 12-Class matrix.", "class_count_12")
        self.assertCaught("Inquisitor dùng roi xích.", "chain_whip")
        self.assertCaught("Frame budget = 8 frame $\\times$ 8 hướng.", "frame_directions")

    def test_legitimate_tiers_and_tags_pass(self):
        for text in [
            "Tier 2 Forge đục tối đa 2 lỗ ngọc; Tier 3 Ancient Sanctuary Forge đúc Thần Binh.",
            "| **Tier 1** | **Thợ Rèn Tiền Trạm** | Common, Uncommon & Rare | +3 |",
            "Mở khóa **Epic** (Tier 2 Forge).",
            "Nâng cấp túi đồ Bậc 1 (30 → 40 ô), Bậc 2 (40 → 50 ô).",
            "Backpack capacity sequence (Tier 1 -> 40, Tier 2 -> 50, Tier 3 -> 60 max).",
            "Zone Tier 2 có quái Cấp 20+.",
            "Chuyển chức lên Bậc 2 rồi Bậc 3; Bậc Class Phụ ≤ Bậc Class Chính (Bậc T1/T2/T3/T4).",
            "- Quyển Trục Bậc 2 (Rare Scroll): Nhận **3 Tàn Trang**.",
            "Tag `Class.Line.Guard.Vanguard`, `Class.Line.Apex.GodSlayer`, `Class.Line.Scout.PhantomStalker`.",
            "Cấu trúc `Class.Line.<Nhánh>.<Class>` và wildcard `Class.Line.*`; meta `Class.Primary`.",
            "Active 1: Thần Lực Tước Đoạt (Divine Siphon).",
            "`ItemDefId` trỏ sang Asset có độ hiếm (`RarityTier`).",
            "5-Tier Karma State Machine & Death Penalties Matrix",
            "Phân rã thành Tàn Trang (`item_skill_shard`).",
            "Lò Rèn Cấm Địa ở Cấp 35+.",
            "Vùng Bậc 3 nguy hiểm.",
            "Iris dùng 3-tier relevancy (60Hz / 30Hz / Culling).",
            "Flash Shards rơi từ Golem; Splash damage.",
            "Tier 1 Common Ore and Tier 2 Rare Materials.",
            "Mở khóa Epic Tier 2 Forge.",
            "Normal (Trắng - Sách T1), Rare (Xanh dương - Quyển Trục T2).",
        ]:
            self.assertClean(text)


class DeprecationContextTests(unittest.TestCase):
    def test_marked_on_same_line_is_accepted(self):
        for text in [
            'Không tạo tiền tệ mới "Ash Shards".',
            "Tên \"Ash Shards\" bị cấm; dùng `item_skill_shard`.",
            "*(Cập nhật 2026-10-10 (X7): bỏ \"Immortal/Divine\" — không phải độ hiếm.)*",
            "used_for_boss_soul_forging: true  # X7 2026-10-10: renamed from used_for_divine_crafting",
            "Ash Shards (deprecated, trước 2026-10-09).",
        ]:
            self.assertEqual(findings_for(text), [], text)

    def test_generic_marker_elsewhere_on_line_does_not_hide(self):
        # Review fix 2: real line shape from expansion-crafting story-001 before X7.
        ids = check_ids("Salvaging a Rare (Tier 3) chestplate yields 10 Ash Shards; item is removed from slot.")
        self.assertIn("ash_shards", ids)
        self.assertIn("rarity_tier_numbering", ids)
        self.assertIn("ash_shards", check_ids("Old field deprecated; now pays 5 Ash Shards."))
        self.assertIn("ash_shards", check_ids("Ash Shards are granted, the cap was removed later."))

    def test_dated_note_only_covers_its_own_parenthetical(self):
        # Review fix 3.
        self.assertIn("ash_shards", check_ids("Ash Shards (Cập nhật 2026-10-10 (X13): giữ nguyên)"))
        self.assertIn("ash_shards", check_ids("*(Cập nhật 2026-10-10 (X7): sửa số liệu.)* Nhận 10 Ash Shards."))
        self.assertEqual(findings_for("Nhận Tàn Trang *(Cập nhật 2026-10-10 (X13): bỏ \"Ash Shards\", DECISIONS.md §5.)*"), [])

    def test_replacement_verbs_are_not_markers(self):
        # Review fix 4: "thay bằng"/"thay thế"/"đổi" introduce the NEW term.
        self.assertIn("ash_shards", check_ids("Tàn Trang được thay bằng Ash Shards."))
        self.assertIn("ash_shards", check_ids("Thay thế Gold bằng Ash Shards."))
        self.assertIn("ash_shards", check_ids("Đổi sang Ash Shards."))
        self.assertEqual(findings_for("Thay vì Ash Shards, dùng Tàn Trang."), [])

    def test_second_unmarked_match_is_reported(self):
        # Review fix 5: a marked first match must not hide a later unmarked one (12 Class, class tier, equipment tier, frames).
        self.assertIn("class_count_12", check_ids('bỏ "12 Class" cũ; Ma Trận 12 Class & Action Deck'))
        self.assertIn("equipment_tier", check_ids('bỏ "5-Tier Item" ở đây; còn 5-Tier Item Database'))
        self.assertIn("class_tier_rarity", check_ids('bỏ "Mythic Class" cũ; mở khóa Mythic Class mới'))
        self.assertIn("frame_directions", check_ids("Ngân sách 5 hướng x 3 frame, còn 8 frame x 8 hướng."))

    def test_unrelated_deprecated_word_does_not_hide_match(self):
        # The old validator skipped the whole line when it contained a deprecation phrase.
        far = "Trường cũ deprecated." + " Mô tả chi tiết luồng giao dịch." * 8 + " Người chơi nhận 5 Ash Shards."
        self.assertIn("ash_shards", check_ids(far))
        # "không dùng" about something else, far from the term, does not hide it.
        self.assertIn("rarity_tier_numbering",
                      check_ids("TUYỆT ĐỐI KHÔNG DÙNG MÀU ĐỎ cho bất kỳ bậc hiếm nào."))
        # "Cấm Địa" is a place name, not a ban marker.
        self.assertIn("ash_shards", check_ids("Chợ Đen Cấm Địa thu mua Ash Shards."))
        # A marked term on the line does not hide a second, unmarked term.
        self.assertIn("legacy_class_tag", check_ids('Bỏ "Ash Shards". Tag `Class.Vanguard`.'))

    def test_yaml_deprecated_entry(self):
        yaml_text = "\n".join([
            "entities:",
            "  - name: ash_shards_carry_limit",
            "    status: deprecated",
            "    value: 99999",
            "  - name: ash_shards_bonus",
            "    status: active",
        ])
        found = findings_for(yaml_text, "entities.yaml")
        self.assertEqual([(ln, cid) for ln, cid, _ in found], [(5, "ash_shards")])


class ScopeAndAllowlistTests(unittest.TestCase):
    def setUp(self):
        self.tp = TempProject()

    def tearDown(self):
        self.tp.close()

    def test_scope_covers_stories_yaml_architecture_registry(self):
        self.tp.write("design/gdd/clean.md", "Tier 2 Forge.\n")
        self.tp.write("production/epics/e/story-001.md", "Nhận 10 Ash Shards.\n")
        self.tp.write("production/sprints/sprint-9.md", "Rèn đồ Bậc 5 (Legendary).\n")
        self.tp.write("production/sprint-status.yaml", "  - name: \"12 Class Idle Stances\"\n")
        self.tp.write("docs/architecture/architecture.md", "`RequiredClassTag = Class.Vanguard`\n")
        self.tp.write("docs/architecture/tr-registry.yaml", "  text: Divine Equipment forging\n")
        self.tp.write("design/registry/entities.yaml", "  - name: ash_shards_cap\n    status: active\n")
        self.tp.write("design/art/pixel-asset-specifications.md", "## Ma Trận 5 Bậc Hiếm\n")
        self.tp.write("design/ux/hud.md", "Viền đồ Divine (Hoàng kim).\n")
        self.tp.write("design/overview.md", "Tag `Class.Tier2.Templar`.\n")
        self.tp.write("docs/registry/terms.yaml", "  currency: AshShards\n")
        code, out = self.tp.run()
        self.assertEqual(code, 1)
        for rel, cid in [
            ("production/epics/e/story-001.md:1", "ash_shards"),
            ("production/sprints/sprint-9.md:1", "rarity_tier_numbering"),
            ("production/sprint-status.yaml:1", "class_count_12"),
            ("docs/architecture/architecture.md:1", "legacy_class_tag"),
            ("docs/architecture/tr-registry.yaml:1", "divine_immortal_rarity"),
            ("design/registry/entities.yaml:1", "ash_shards"),
            ("design/art/pixel-asset-specifications.md:1", "rarity_tier_numbering"),
            ("design/ux/hud.md:1", "divine_immortal_rarity"),
            ("design/overview.md:1", "legacy_class_tag"),
            ("docs/registry/terms.yaml:1", "ash_shards"),
        ]:
            self.assertIn(f"{rel}: [{cid}]", out)
        self.assertIn("[PASS] design/gdd/clean.md", out)

    def test_historical_records_are_skipped(self):
        bad = "Nhận 10 Ash Shards. Class.Vanguard. Oracle.\n"
        for rel in [
            "design/gdd/gdd-cross-review-2026-09-16.md",
            "docs/architecture/architecture-review-2026-09-16.md",
            "production/epics/gate-check-old.md",
            "production/epics/e/retrospective-sprint-1.md",
            "production/qa/report.md",
            "production/plans/plan.md",
            "production/PROGRESS.md",
        ]:
            self.tp.write(rel, bad)
        code, out = self.tp.run()
        self.assertEqual(code, 0, out)
        self.assertIn("[SKIP] design/gdd/gdd-cross-review-2026-09-16.md", out)
        self.assertIn("[SKIP] docs/architecture/architecture-review-2026-09-16.md", out)
        self.assertIn("[SKIP] production/epics/gate-check-old.md", out)
        self.assertIn("[SKIP] production/epics/e/retrospective-sprint-1.md", out)

    def test_allowlist_accepts_listed_finding_and_flags_stale_entry(self):
        self.tp.write("production/epics/e/story-001.md", "Nhận 10 Ash Shards.\n")
        allow = "\n".join([
            "entries:",
            "  - file: production/epics/e/story-001.md",
            "    check: ash_shards",
            "    pattern: \"Nhận 10 Ash Shards\"",
            "    match: \"Ash Shards\"",
            "    reason: \"OWNER QUESTION: fixture\"",
        ])
        code, out = self.tp.run(allow)
        self.assertEqual(code, 0, out)
        self.assertIn("[ALLOW] production/epics/e/story-001.md:1: [ash_shards]", out)

        stale = allow + "\n" + "\n".join([
            "  - file: production/epics/e/story-002.md",
            "    check: ash_shards",
            "    pattern: \"gone\"",
            "    match: \"x\"",
            "    reason: \"fixed already\"",
        ])
        code, out = self.tp.run(stale)
        self.assertEqual(code, 1)
        self.assertIn("Stale allowlist entry #2", out)

    def test_allowlist_does_not_cover_other_checks(self):
        self.tp.write("production/epics/e/story-001.md", "Nhận 10 Ash Shards. Tag `Class.Vanguard`.\n")
        allow = "entries:\n  - file: production/epics/e/story-001.md\n    check: ash_shards\n    pattern: \"Ash Shards\"\n    match: \"Ash\"\n    reason: \"x\"\n"
        code, out = self.tp.run(allow)
        self.assertEqual(code, 1)
        self.assertIn("[legacy_class_tag]", out)

    def test_allowlist_entry_without_match_is_rejected(self):
        self.tp.write("production/epics/e/story-001.md", "Nhận 10 Ash Shards.\n")
        code, out = self.tp.run("entries:\n  - file: production/epics/e/story-001.md\n    check: ash_shards\n    pattern: \"Ash\"\n    reason: \"x\"\n")
        self.assertEqual(code, 1)
        self.assertIn("missing 'match'", out)

    def test_repo_allowlist_rejects_dragon_knight_in_2h_heavy(self):
        # Y1 (2026-10-10, chủ dự án duyệt): Dragon Knight was removed from Weapon.2H.Heavy (Polearm only,
        # DECISIONS.md:122) and its allowlist entries were deleted. Replaces the X13 test that bound those
        # entries to Dragon Knight only (review fix 7); other classes must still fail.
        repo_allow = (REPO_ROOT / "Tools" / "QA" / "gdd_validator_allowlist.yaml").read_text(encoding="utf-8")
        row_ok = ("| **2. Two-Handed Heavy** | `Weapon.2H.Heavy` | Bổ dọc 180cm. | **Berserker** | Khóa cứng 2 tay. |")
        row_dk = ("| **2. Two-Handed Heavy** | `Weapon.2H.Heavy` | Bổ dọc 180cm. | **Berserker, Dragon Knight** | Khóa cứng 2 tay. |")
        row_bad = ("| **2. Two-Handed Heavy** | `Weapon.2H.Heavy` | Bổ dọc 180cm. | **Berserker, Templar, Inquisitor** | Khóa cứng 2 tay. |")
        yaml_ok = '  - name: weapon_family_2h_heavy\n    attributes:\n      classes: ["Berserker"]\n'
        self.tp.write("design/registry/entities.yaml", yaml_ok)
        art = ("- **Khung Viền Phân Hạng 5 Bậc Hiếm (5-Tier Rarity)**:\n"
               "  - Normal (Xám `#4B5563`), Rare (Lam `#2563EB`), Legendary (Cam `#D97706`), Immortal (Đỏ `#DC2626`), Divine (Hoàng Kim `#F59E0B` + Tím `#7C3AED`).\n")
        self.tp.write("design/art/art-bible.md", art)

        self.tp.write("design/gdd/itemization.md", row_ok + "\n" + yaml_ok)
        code, out = self.tp.run(repo_allow)
        self.assertEqual(code, 0, out)

        self.tp.write("design/gdd/itemization.md", row_dk + "\n" + yaml_ok)
        code, out = self.tp.run(repo_allow)
        self.assertEqual(code, 1)
        self.assertIn("Class 'Dragon Knight'", out)

        self.tp.write("design/gdd/itemization.md", row_ok + "\n" + yaml_ok)
        self.tp.write("design/registry/entities.yaml", yaml_ok.replace('"Berserker"]', '"Berserker", "Dragon Knight"]'))
        code, out = self.tp.run(repo_allow)
        self.assertEqual(code, 1)
        self.assertIn("class 'Dragon Knight'", out)
        self.tp.write("design/registry/entities.yaml", yaml_ok)

        self.tp.write("design/gdd/itemization.md", row_bad + "\n" + yaml_ok)
        code, out = self.tp.run(repo_allow)
        self.assertEqual(code, 1)
        self.assertIn("Class 'Templar'", out)
        self.assertIn("Class 'Inquisitor'", out)

    def test_repo_allowlist_parses_and_matches_pyyaml(self):
        path = REPO_ROOT / "Tools" / "QA" / "gdd_validator_allowlist.yaml"
        entries = v.load_allowlist(path)
        for e in entries:
            for key in ("file", "check", "pattern", "match", "reason"):
                self.assertTrue(e[key])
            self.assertTrue((REPO_ROOT / e["file"]).exists(), e["file"])
        try:
            import yaml  # optional; only used to cross-check the stdlib reader
        except ImportError:
            self.skipTest("PyYAML not installed; stdlib reader already verified above")
        ref = yaml.safe_load(path.read_text(encoding="utf-8"))["entries"]
        self.assertEqual([{k: e[k] for k in ("file", "check", "pattern", "match", "reason")} for e in entries], ref)


if __name__ == "__main__":
    unittest.main(verbosity=2)
