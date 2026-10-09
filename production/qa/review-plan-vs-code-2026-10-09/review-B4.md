# Workstream B4 — Netcode / World / Auth / UI — BLOCKER 1 / MAJOR 16 / MINOR 5
Stories: netcode-001 PARTIAL, netcode-002 MISSING, netcode-003 PARTIAL, netcode-004 PARTIAL, wza-001 PARTIAL, wza-002 PARTIAL/CONTRADICTS, wza-003 CONTRADICTS, wza-004 PARTIAL, core-world-001 PARTIAL, core-world-002 PARTIAL, ui-001 MET, ui-002 MET, ui-003 PARTIAL, ui-004 PARTIAL.

1. BLOCKER — 7 test files in root Tests/ never compiled (uproject:7-8, Build.cs); netcode stories claim "assertions PASS"; Source/Tests/README.md points to nonexistent dir.
2. MAJOR — join token not enforced: no PreLogin/Login override (PAGameModeBase.h:18-24); ValidateJoinToken prefix-only (PAAccountSubsystem.cpp:263-277), no callers; GDD defers (authentication-account-system.md:172) but AC-4 ticked.
3. MAJOR — Iris filter is BlueprintFunctionLibrary math (PAIrisSpatialFilter.h:69-108), unused; ini sections DefaultEngine.ini:26-36 point to nonexistent classes. TR-net-002 broken.
4. MAJOR (inferred) — Iris maybe not enabled: only cvar (DefaultEngine.ini:24); no bUseIris/SetupIrisSupport in Target.cs.
5. MAJOR — boss Mixed vs DECISIONS.md:185.
6. MAJOR — ghost body subsystem bookkeeping only, no callers.
7. MAJOR — threat component never instantiated; leash duplicated.
8. MAJOR — loot distribution / difficulty scaling / party EXP no runtime callers; contribution logic duplicated.
9. MAJOR — death path (PABaseCharacter.cpp:376-396) always PvE penalty + remnant incl. PvP; no shard drop; karma model/component unused, unreplicated.
10. MINOR — gold rounding up (PACurrencyComponent.cpp:256) vs down (zone-system.md:225, PAKarmaTypes.h:249).
11. MAJOR — citadel autosave in memory only, no persistence (TR-zone-002); OnPlayerEnterCitadel no callers/authority; restoration not applied.
12. MAJOR — sanctuary doesn't block outgoing attacks (PASanctuaryVolume.cpp:63 unused).
13. MAJOR (inferred) — shop/forge widgets call RPCs on NPC components (dropped); UI fires completion before server confirm; karma passed as int; distance check UI-only.
14. MAJOR (inferred) — instanced loot relies on IsNetRelevantFor (PALootDropletActor.cpp:31), ignored by Iris.
15. MAJOR — no server+2 clients replication test (DECISIONS.md:186).
16. MAJOR — value conflicts: HP scaling 0.45 (multiplayer-coop.md:107,194) vs 0.50 (zone-system.md:79,164, code); Iris tiers manifest 45m/20/5Hz vs GDD/code 35m/30Hz; proximity 250 vs 300; FCT pool 64 vs 50; wanted <= -50 vs < -50.
17. MAJOR — CommonUI required (control-manifest.md:91, TR-hud-001) but widgets are UUserWidget; not in Build.cs.
18. MINOR — history 50Hz vs documented 100Hz.
19. MINOR — status bookkeeping (EPIC Ready vs stories Complete; "✅ Done" unmapped; missing Test Evidence).
20. MAJOR — auth: hardcoded default creds, plaintext passwords in memory (PAAccountSubsystem.cpp:15-22,93); LoginFastPlaytest not dev-only; token format differs from GDD; auth GDD absent from tr-registry.
21. MINOR (inferred) — no C++ creates vitals/boss widgets; SpawnCombatText uncalled.
22. MINOR — ADR-0001 says UE 5.7 (:13).
