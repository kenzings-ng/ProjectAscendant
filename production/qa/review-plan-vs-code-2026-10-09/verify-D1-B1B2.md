# D1 verification of B1/B2 (27 BLOCKER/MAJOR): 18 CONFIRMED, 9 PARTLY, 0 REJECTED
B1-1 CONFIRMED BLOCKER (22/24 root Tests not in Source; 2 duplicated into Private/)
B1-2,3(+DefaultGameplayTags.ini:2-5 registers Class.Vanguard etc.),4,7,8,10,11,12,13,14 CONFIRMED MAJOR
B1-5 PARTLY → MINOR (DamageExecutionCalculation dead code)
B1-6 PARTLY MAJOR — stamina pipeline never runs (no callers), exhaustion never fires
B1-9 PARTLY → MINOR (1400 within GDD range 1000-1400, story changed it; lookahead off contradicts story AC-2)
B2-1,2,3,4,6,8,9,10 CONFIRMED MAJOR
B2-5 PARTLY MAJOR (transient GE, not raw subtraction; formula bypassed)
B2-7 PARTLY MAJOR (Staggered/FinisherPriority unregistered; PostureImmune/PerfectDodge unimplemented)
B2-11 PARTLY MAJOR (architecture only; live Golem gates HasAuthority)
B2-12 → MINOR (dead until B2-1 fixed)
B2-13 PARTLY MAJOR (inferred)
Duplicates: B1-12=B2-9; B1-4/B2-7/B1-20; B1-5/B2-5; B1-7=B2 desperation; B1-21⊂B2-2; B1-1~B2-10; B1-6/B1-8; B2-11,12 ⊂ B2-1.
