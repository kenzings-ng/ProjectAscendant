# Workstream B3 — Items/Inventory/Crafting/Economy/Progression — BLOCKER 2 / MAJOR 13 / MINOR 7
S/ = Source/ProjectAscendant/

Stories: inv-001 PARTIAL, inv-002 PARTIAL, inv-003 PARTIAL, items-001 CONTRADICTS, crft-001 CONTRADICTS, crft-002 PARTIAL, crft-003 CONTRADICTS, econ-001 CONTRADICTS, econ-002 PARTIAL, econ-003 PARTIAL (In Progress, consistent), prog-001 PARTIAL, prog-002 CONTRADICTS.

1. BLOCKER — banned "Ash Shards" currency in code: PACurrencyTypes.h:16,22,31; PACurrencyComponent.cpp:61,95; PABlacksmithComponent.cpp:141-145; PABlacksmithTests.cpp:19,115; also stories econ-001 AC-1, crft-001 AC-2. GDD makes item_skill_shard an inventory material (inventory-system.md:49,62).
2. BLOCKER — no 4-tier skill rarity (Normal→Mythic); only EPAItemRarity (PAInventoryTypes.h:24); Dash skill book = equipment Rare (PAItemStaticDataAsset.cpp:154); skill book salvage flat 5 (PABlacksmithTypes.h:126-129) vs GDD 1/3/8/25 (skill-progression-system.md:107-111,233-236).
3. MAJOR — equipment salvage pays shards 1/3/10/25/75 (PABlacksmithTypes.h:131-142) vs GDD ore formula (blacksmithing-system.md:249-259).
4. MAJOR — "Tier N: <Rarity>" display names (PAInventoryTypes.h:27-31) banned by DECISIONS §1.
5. MAJOR — banned tag Class.Vanguard: PAItemStaticDataAsset.cpp:156; Scripts/verify_item_data_assets.py:74; items-001 AC-3.
6. MAJOR — Tests/{unit,integration}/inventory/*.cpp outside UBT module, never compiled; inv-001..003, items-001 marked Complete.
7. MAJOR — server trusts client cost/tier: PABlacksmithComponent.cpp:847-852→PABlacksmithSubsystem.cpp:496-500; :546-547; :617-618; InForgeTier Component.cpp:964-969 (violates control-manifest:30 / ADR-0003).
8. MAJOR — RPCs accept client-supplied Inventory/Wallet pointers without ownership check (Blacksmith Component.cpp:534-566; Merchant .h:98-108, .cpp:335-370); ForgeBossSoul output asset from client (Component.cpp:612-660,807-827). Inferred: components on NPC actors → client RPCs dropped, feature has no working client path.
9. MAJOR — ValidateInteraction (distance/combat guard) only called from tests; radius 300cm code/GDD vs control-manifest:74 250cm.
10. MAJOR — buyback list shared per merchant across all players (PAMerchantComponent.h:137, .cpp:186-196).
11. MAJOR — OverflowStash replicated as raw TArray (PAInventoryComponent.h:232-233) vs control-manifest:29.
12. MAJOR — enhancement cost fixed table (PABlacksmithTypes.h:154-200) vs GDD formula (blacksmithing-system.md:197-211).
13. MAJOR — socket limits (PABlacksmithTypes.h:298) vs GDD (itemization.md:46-48; blacksmithing-system.md:85); generator follows GDD (PAServerItemGeneratorSubsystem.cpp:253-266) → two rules.
14. MAJOR — dual-track Class Level 1-20 + 70/30 EXP (DECISIONS §4) missing.
15. MAJOR — talent tree (PATalentTreeTypes.h:120-364) no GDD/DECISIONS trace; TR-prog-002 absent; Arcanist branch named "Chronomancer" (:364) collides with T3 class.
16. MINOR — "Divine/Immortal" in crft-003 ACs and PABlacksmithBossSoulTests.cpp:17-18,46.
17. MINOR — 5000 Gold boss soul cost (PABlacksmithTypes.h:358) not in GDD.
18. MINOR — code cites nonexistent stories item-001..007; two item models; two forge-tier enums.
19. MINOR — Lock/Junk no Server RPC (PAInventoryComponent.cpp:577,594); TransactionID=0 bypasses replay (:382,501); InventoryList replicates to all (:709).
20. MINOR — 13 stories absent from sprint-status.yaml; EPIC "Ready" vs stories Complete; expansion-progression/EPIC.md missing; TR-crft-* vs TR-blacksmith-*.
21. MINOR — Server_AddCurrency/DeductCurrency absent; Server_TransferCurrency (PACurrencyComponent.h:82) P2P transfer, DECISIONS §6 defers trade.
22. MINOR — econ-003: no 500cm auto-close; restock uses World time (PARestockComponent.cpp:52,112).
