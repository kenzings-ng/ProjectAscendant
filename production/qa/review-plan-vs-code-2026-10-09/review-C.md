# Workstream C — Test authenticity (Claude subagent) — BLOCKER 0 / MAJOR 11 / MINOR 6 / INFO 1

1. MAJOR — Combat/PACombatRegressionHardeningTests.cpp:96-138,203-213 test an in-file mock (FPACombatBufferMock :30-86) and a lambda; prod has no input buffer; cancel window PADashTypes.h:55 = 0.35s vs mock 0.25-0.45s; Stun/HitStun reset claimed (:17) not tested.
2. MAJOR — tautologies: Character/PAVanguardRuntimeWiringTests.cpp:40-53; Character/boss_paperzd_aggro_test.cpp:61-66.
3. MAJOR — Network/PANetworkReplicationTests.cpp: tests distance math only (:34-64), registers tags then asserts them (:107-110,:122-124); filter not wired into Iris (inferred).
4. MAJOR — Account: ValidateJoinToken (Private/Account/PAAccountSubsystem.cpp:263-277) accepts any `PA-TOKEN-<id>-` prefix (forgeable); tests :97-106 don't try forged token; AC-1 :28-40 struct ctor; AC-2 :47-59 always true.
5. MINOR — silent-skip `if (ASC)` guards: PANetworkReplicationTests.cpp:112-116; code_review_fixes_regression_test.cpp:103,113.
6. MINOR — PACharacterVisualIdentityTests.cpp:16,72-85 covers 12 classes, DECISIONS has 16 (missing Swordmaster, Phantom Stalker, Inquisitor, Seraph).
7. MINOR — PARaidCombatCalculationTests.cpp:48 "50 bots" = GUID count; many tests only test F*Model structs, not components/widgets.
8. INFO — 45 tests registered = PROGRESS claim; 38 files (2 lowercase `_test.cpp`).
9. MAJOR — test_backend_postgres.py: real PG, FOR UPDATE, 2 conns, DEFERRABLE ok; but DDL inline in test (:57-103), no migration/backend code exists; promotion txn skips item_def_id/eligibility check required by DECISIONS.md:160 (:213-225).
10. MINOR — bare except (:145), no thread barrier, DROP CASCADE on whatever PGDATABASE (:60-62).
11. MAJOR — validate_gdd_consistency.py passes despite itemization.md:200 Dragon Knight Weapon.2H.Heavy vs DECISIONS.md:122 Polearm-only (parser :118-122 collects pending-note tag); narrow blacklist scope; "deprecated" skips line (:177-178); no assert that 16 classes parsed (:258).
12. MAJOR — run_headless_tests.sh:64-72 turns any non-zero Postgres result (incl. assertion failure) into WARN locally, exit 0, misleading message.
13. MINOR — ExitCode=1 acceptance gated by log parse (:114-128) ok; stale log not deleted (:82-96).
14. MAJOR — CI ue-tests: dispatch-only (tests.yml:65-66), never run, wrong uproject path (:78), `; Quit` truncation bug (:80), no result parsing.
15. MAJOR — main not protected (API "Branch not protected", ruleset enforcement disabled) — CLAUDE.md first-task requirement.
16. MINOR — PROGRESS.md:27 vs :361 ExitCode; :44 22/22 vs :131/:145 92.
17. MAJOR — sprint-status.yaml: sprint 7 dates 11-01..11-14 (:14-15) but completed 09-25; invalid free-text statuses (:54,64,74,84); visual-006 "12 Class" (:71).
18. MAJOR — qa-plan-sprint-1 requires 13 tests under tests/ — none exist (DoD :273-275).
