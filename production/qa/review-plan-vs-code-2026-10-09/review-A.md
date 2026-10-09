# Workstream A — Plan docs internal consistency — BLOCKER 2 / MAJOR 16 / MINOR 11
Validator blind spots: doesn't scan docs/architecture, sprint-status.yaml, design/registry, PROGRESS/ROADMAP; skips gdd-cross-review; Ash/Divine/legacy-tag checks didn't fire on epics/stories.

1. BLOCKER — "Ash Shards" currency in epics/stories/sprint/registry vs DECISIONS.md:96-97: expansion-economy/EPIC.md:24,26,39,41; expansion-economy/story-001:42,68,78-79; expansion-crafting/EPIC.md:24,41; expansion-crafting/story-001:34,47,71-72; sprints/sprint-3.md:13; design/registry/entities.yaml:2052.
2. BLOCKER — ROADMAP.md:3-5 placeholder "tạm dừng" vs PROGRESS.md:28 "ROADMAP đã duyệt… 100% Giai đoạn 0", :140; stage.txt Production; epics/index.md:9-69 100% Complete.
3. MAJOR — "Divine Tier 5" 6th rarity: expansion-crafting/story-003:1,27,33; EPIC.md:26,34; sprint-3.md:40; blacksmithing-system.md:42-44 "Tier 1…5"; inventory-system.md:36-40 "Bậc 1-5" vs DECISIONS.md:17-19, :80-86.
4. MAJOR — dual-class/promotion (DECISIONS.md:49-73,159-160; advanced-classes.md:77-201) has no epic; TR-class-*, TR-skill-* no story; talent tree/skill points/respec (expansion-progression/story-002:37-58, story-001:28,46) not in any GDD → new system (CLAUDE.md ban).
5. MAJOR — dash i-frame 4 values: 0.28 (dash-evasion.md:87,152; attributes-system.md:43; combat-system.md:72; boss-ai.md:154; adr-0001:38) / 0.20 (adr-0002:160; core-combat/EPIC.md:11; core-combat/story-001:25; sprint-2) / 0.25 (control-manifest.md:50; tr-registry.yaml:86; requirements-traceability.md:40) / 200ms (core-game-loop.md:58,119).
6. MAJOR — combo story core-combat/story-002:24-26 vs combat-system.md:41-45, stagger-system.md:39,46, encounter-boss/story-002:30,41; input buffer 250ms (manifest:52, tr:125) vs 0.15s (combat-system.md:151).
7. MAJOR — low-stamina dash: core-combat/story-001:24,26 blocks vs attributes-system.md:111, dash-evasion.md:103,183, foundation-attributes/story-003:34 Desperation Roll.
8. MAJOR — rarity table inventory-system.md:36-40 vs itemization.md:44-48 (colors, affixes, multipliers); DECISIONS.md:86 doesn't pick.
9. MAJOR — 18 TR-IDs in stories absent from registry; 19 registry IDs without story; naming cmbt/stgr/crft vs combat/stagger/blacksmith.
10. MAJOR — requirements-traceability.md counts wrong (:22,:25 vs matrix :34-75), "14 GDDs"/100% coverage false; TR-stagger-002 200% crit no basis.
11. MAJOR — 12-class leftovers: architecture.md:60,113,290; adr-0002:27; sprint-status.yaml:71; systems-index.md:67 "7 Class Nâng Cao".
12. MAJOR — legacy Class.<Name> tags in core-items/story-001:30; foundation-inventory/story-003:92; presentation-character-visual/story-002:31; sprint-2.md:30 vs DECISIONS.md:144-148.
13. MAJOR — status mismatches (visual story-001 Ready vs yaml done; econ story-003 In Progress vs EPIC Done; 8 EPIC "Ready" with Complete stories; invalid yaml statuses).
14. MAJOR — sprint 5-7 dates current/future but stories completed 09-23/25; sprints 1-4 overlap.
15. MAJOR — zone capacity 32 / 100 CCU / 20 / 20-50 across docs.
16. MAJOR — Iris tiers 3 versions (control-manifest.md:22; architecture.md:268; multiplayer-coop.md:67-68,249-250).
17. MAJOR — architecture.md:556-597 FPlayerSaveData single class, inventory array vs DECISIONS.md:51-58,154-157 and auth GDD SQL :85-110.
18. MAJOR — salvage yields story vs GDD/DECISIONS.md:107.
19-29. MINOR — DOREPLIFETIME in story vs manifest:153; engine 5.7 in ADRs/architecture/manifest; weapon assignments open (DECISIONS.md:122,127 pending); Exhausted state values differ; missing referenced GDDs (world-bosses, hidden-classes, pvp-wanted-system, apothecary, reincarnation) + missing expansion-progression/EPIC.md; Vietnamese class names differ; gdd-cross-review stale; "Divine" in attributes-system.md:184; respec location differs; 5-agent plan conflicts; broken links (paths missing ProjectAscendant/).
