# Workstream B1 — Character foundation — BLOCKER 1 / MAJOR 13 / MINOR 7
1. BLOCKER — Tests/unit, Tests/integration (root) not compiled by any module; stories attr-001..003, ctrl-001..003, pzd-001/002 claim PASS. Proof: camera_lookahead_occlusion_test.cpp:36,50 asserts 1200 vs code 1400 (PAIsometricMovementMath.h:51).
2. MAJOR — ABP_Vanguard / ABP_StoneGolem contain only Sink node; AS_* no sequences/notifies; Hitbox notify not placed. pzd-002 AC1/3, pzd-003 AC1 not built.
3. MAJOR — banned Class.<Name> tags: PABaseCharacter.cpp:135,139; PAPaperdollComponent.cpp:146; PAAccountSubsystem.h:214; PACharacterSelectTypes.cpp:13,36,59; PAItemStaticDataAsset.cpp:156; story visual-002:31.
4. MAJOR — posture break adds State.Broken/Stunned (PABaseCharacter.cpp:407-416) not State.Staggered (control-manifest:51) → execution branch PADamageExecutionCalculation.cpp:138-146 dead.
5. MAJOR — execution bonus applies on every hit vs staggered (PADamageExecutionCalculation.cpp:143-147) vs GDD 3m/1.5s rules.
6. MAJOR — stamina SetStamina without authority (PAStaminaComponent.cpp:131,164,268); loose non-replicated tags (:377-383).
7. MAJOR — Desperation Roll blocked (PAGameplayAbility_Dash.cpp:29-30) vs attributes-system.md:111.
8. MAJOR — move speed from member BaseMoveSpeed (PABaseCharacter.h:248, .cpp:316) not GAS attribute; exhaustion bool not GE.
9. MAJOR — arm length 1400 vs 1200 (isometric-controller.md:34); lookahead off by default (PABasePlayerController.h:115).
10. MAJOR — aim never sent to server (PABaseCharacter.cpp:326; PABasePlayerController.cpp:97-100).
11. MAJOR — legacy input IsInputKeyDown/BindKey (PABasePlayerController.cpp:78-81,117-124) vs Enhanced Input rule.
12. MAJOR — boss Mixed replication (PABaseCharacter.cpp:81) vs DECISIONS.md:185 Minimal; boss ticks.
13. MAJOR — 12/16 classes in crest/tabard map (PAPaperdollComponent.cpp:102-134), tests, Content.
14. MAJOR — visual-003 palette swap not implemented; M_PaperZD_Civilian_Base missing.
15-21. MINOR — guard knockback no authority; Karma not replicated & BlueprintReadWrite; visual-001 status mismatch (story Ready vs yaml done; sprint 7 future dates); boss PaperZD vs DECISIONS.md:166,174 Spine; tracking gaps; posture decay & mana regen missing; GDD/manifest i-frame 0.25 vs 0.28; TakeDamage transient GE no story.

# Workstream B2 — Combat & Boss — BLOCKER 0 / MAJOR 13 / MINOR 6
1. MAJOR — UPABossAIComponent, UPAStaggerComponent, UPAPartBreakingComponent, UPADashEvasionComponent never attached to any actor; real boss APAStoneGolemBoss uses own AI (PAStoneGolemBoss.cpp:104-130,236-300). 4 "Done" encounter-boss stories are data models only.
2. MAJOR — two dash impls: GA_Dash 0.35s/0.20s/450cm (Dash.h:30-36) vs GDD 0.45/0.28/380 (dash-evasion.md:35,52,152) vs manifest 0.25 (control-manifest.md:50; tr-registry:86); duplicate id dash-001 (sprint-2:23, sprint-5:31).
3. MAJOR — combo 1.2s reset, 1.6x/25 (MeleeAttack.cpp:27, .h:36) vs GDD 0.60s, 1.8x/30 (combat-system.md:41-45,110,115,151).
4. MAJOR — no production input buffer (GDD 0.15s combat-system.md:37; manifest 250ms :52); tests use in-file mock.
5. MAJOR — melee raw Health subtraction (MeleeAttack.cpp:343,403), BaseAttackDamage=20 hardcoded; UPADamageExecutionCalculation unused; vs sprint-2.md:24 AC-2.
6. MAJOR — lag comp uses unsynced client clock (MeleeAttack.cpp:195,218 vs ~361).
7. MAJOR — stagger runtime path diverges; State.Staggered/PostureImmune/WallStunned/PerfectDodgeTriggered/FinisherPriority not in DefaultGameplayTags.ini → RequestGameplayTag(false) invalid; posture decay unused.
8. MAJOR — cmbt-002 1.5s stun/invuln (Finisher.h:91, MeleeAttack.h:40) vs stgr-001/GDD 3.0/1.2/2.0 (stagger-system.md:46-47).
9. MAJOR — boss Mixed vs DECISIONS.md:185 Minimal.
10. MAJOR — DECISIONS.md:186 requires headless server+2 clients replication tests per combat feature: none; core-combat-001 zero tests.
11. MAJOR — boss AI: manifest StateTree (:71) vs GDD BT+EQS vs code hand-rolled Tick scoring without authority (PABossAIComponent.cpp:7,20-24); Tick timers vs manifest :57.
12. MAJOR — Phase-2 combos missing (PABossAITypes.h:61); recovery formula off → Tail Sweep 0.50s < 0.60s min.
13. MAJOR (inferred) — 16 transient NewObject<UGameplayEffect> sites; predicted GE in LocalPredicted dash.
14-19. MINOR — part breaking not wired, no drop location; TR-cmbt/stgr ids vs registry combat/stagger, TR-stagger-001 exponential vs GDD linear, TR-stagger-002 200% crit; manifest version citations; flash cue timing; GDD desperation roll vs code; Golem tick/stats/leash outside epics.
