# D2 verification of B3 / B4 / C — VERDICT: FAIL
Content scan: no .umap references any /Script/ProjectAscendant class; only 4 DataAssets in Content/Items/DataAssets reference project script.

## B3
1 CONFIRMED BLOCKER (fix = rebind to item_skill_shard; merchant-economy.md:13,65 already treats item_skill_shard as currency with 99,999 cap)
2 CONFIRMED BLOCKER
3,4,7,9,11,12,13,14,15 CONFIRMED MAJOR
5 CONFIRMED (also DefaultGameplayTags.ini:2-5, DA_SkillBook_Dash.uasset)
6 CONFIRMED (2 tests duplicated into Source/…/Private; PAPaperdollTests.cpp partly covers inv-003)
8 CONFIRMED (Merchant only on APAWanderingSmuggler NPC; Blacksmith component created nowhere; widgets call RPCs → dropped)
10 PARTLY MAJOR (GDD scopes buyback per NPC; per-player implied by logout wipe :144)

## B4
1 CONFIRMED → MAJOR (compiled Source tests exist; false PASS claims are the defect)
2 PARTLY → MINOR (AC-4 only asks to provide function; GDD defers PreLogin wiring)
3,5,6,7,8,11,13,15,16,17 CONFIRMED MAJOR
4 REJECTED → MINOR (Iris built via SetupIrisSupport in Engine/GAS Build.cs; cvar net.Iris.UseIrisReplication=1 enables it; bogus key DefaultEngine.ini:20-21)
9 CONFIRMED stronger (death path also runs for boss)
12 REJECTED → INFO (State.InSanctuary in ActivationBlockedTags: MeleeAttack.cpp:136-140, Finisher.cpp:36-40)
14 CONFIRMED stronger (Iris ignores IsNetRelevantFor; bOnlyRelevantToOwner not set)
20 PARTLY MAJOR latent (client-local stub)

## C
1,2,3,9,11,12,15 CONFIRMED MAJOR
4 CONFIRMED → MINOR
14 PARTLY MAJOR (dispatch-only, never run, wrong path, no parsing confirmed; `; Quit` truncation REJECTED — AutomationCommandline.cpp:582,726)
17 CONFIRMED → MINOR
18 PARTLY → MINOR (files exist under Tests/, root issue is not compiled)

Duplicates: B3-6=B4-1=C-18; B3-8=B4-13 (+B3-7); B4-2=C-4; B4-3=C-3 (→B4-14); B4-15~C-14; B3-9~B4-16; B3-1→B3-3,C-11; B3-20=B4-19=C-17; B3-2~B3-4.
