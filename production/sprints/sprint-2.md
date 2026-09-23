# Sprint 2 — 2026-09-17 to 2026-10-01

## Sprint Goal
Deliver the Core Layer gameplay loop for *Project Ascendant* — raw map level blockout (`L_Ruins_Blockout.umap`) with isometric camera and sanctuary/leash volumes, Paper2D/PaperZD Vanguard and Boss sprite extraction with AnimBP state machine, GAS Dash ability with 0.20s I-frame, GAS 3-hit combo and posture finisher execution, and core Item DataAssets.

## Review Mode
- Mode: `lean` (Saved in [`production/review-mode.txt`](file:///mnt/Data/Projects/project-games/production/review-mode.txt))
- Director Gates: PR-SPRINT skipped — Lean mode.

## Capacity
- Total days: 10 days (80 hours)
- Buffer (20%): 2 days (16 hours reserved for engine setup, asset importing, and debugging)
- Available: 8 days (64 hours)

## Tasks

### Must Have (Critical Path — 40 hours / 5.0 days)
| ID | Task | Agent/Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `map-001` | [Raw Map Blockout & Isometric Camera Setup](file:///mnt/Data/Projects/project-games/production/epics/core-world/story-001-map-blockout-camera.md) | `level-designer` | 1.0 (8h) | `ctrl-003` | AC-1: `L_Ruins_Blockout.umap` created with 3D floor geometry, stone pillars, NavMeshBounds, and PlayerStart; AC-2: Fixed isometric camera (Pitch -45°, Yaw +45°, SpringArm 1400); AC-3: Line-of-sight occlusion test pillars with dither fading verified in PIE. |
| `pzd-001` | [Paper2D Sprite Extraction & Flipbooks](file:///mnt/Data/Projects/project-games/production/epics/core-character/story-001-paper2d-flipbooks.md) | `animator-2d` | 1.0 (8h) | None | AC-1: Vanguard pixel spritesheet sliced into discrete frames (Idle, Run, LightAttack, Dash, Hurt, Death); AC-2: PaperFlipbooks created with 12 FPS playback and zero pixel blurring; AC-3: Stone Golem Boss spritesheet sliced into Idle, Walk, Slam Attack, Stagger, and Death flipbooks. |
| `pzd-002` | [PaperZD AnimBP & State Machine Setup](file:///mnt/Data/Projects/project-games/production/epics/core-character/story-002-paperzd-animbp.md) | `animator-2d` | 1.0 (8h) | `pzd-001`, `ctrl-001` | AC-1: `ABP_Vanguard` PaperZD Animation Blueprint created with state machine (Idle, Locomotion, Attack, Dash, Hurt, Stun, Death); AC-2: Velocity and direction driving locomotion transitions smoothly; AC-3: PaperZD Anim Notifies configured for attack hitbox window and step events. |
| `dash-001` | [GAS Dash Ability & Invulnerability Window](file:///mnt/Data/Projects/project-games/production/epics/core-combat/story-001-gas-dash-ability.md) | `ue-gas-specialist` | 1.0 (8h) | `attr-003`, `pzd-002` | AC-1: `UGA_Dash` Gameplay Ability consumes 25 Stamina, rejected if Stamina < 25; AC-2: Grants 0.20s I-frame tag `State.Invulnerable` during 0.35s total dash; AC-3: Broadcasts `OnDashExecuted` for ghost trail visual and enforces 0.5s cooldown. |
| `cmbt-001` | [GAS 3-Hit Combo & Posture Finisher Execution](file:///mnt/Data/Projects/project-games/production/epics/core-combat/story-002-gas-combo-finisher.md) | `ue-gas-specialist` | 1.0 (8h) | `attr-002`, `net-004`, `pzd-002` | AC-1: `UGA_LightAttack` 3-hit combo state machine with 1.2s combo chain reset window; AC-2: PaperZD Anim Notify triggers sector/box trace hitbox and applies damage via `attr-002` non-linear formula; AC-3: `UGA_PostureFinisher` executes staggered target in 1.5s window dealing 25% max HP pure damage. |

### Should Have (16 hours / 2.0 days)
| ID | Task | Agent/Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `map-002` | [Sanctuary & Combat Zone Volumes](file:///mnt/Data/Projects/project-games/production/epics/core-world/story-002-sanctuary-leash-volumes.md) | `gameplay-programmer` | 1.0 (8h) | `map-001` | AC-1: `APASanctuaryVolume` actor applies safezone immunity and disables hostile ability activation inside 1000cm radius; AC-2: Leash trigger resets boss/mob aggro and regenerates posture/health if pulled > 2500cm from spawn origin. |
| `item-001` | [Item DataAssets & Quickbar Assets Setup](file:///mnt/Data/Projects/project-games/production/epics/core-items/story-001-item-data-assets.md) | `gameplay-programmer` | 1.0 (8h) | `inv-001`, `inv-003` | AC-1: `DA_Weapon_IronSword` (+15 Physical Damage, 1H Sword, Uncommon); AC-2: `DA_Armor_IronPlate` (+40 Armor, Heavy Chest, Rare); AC-3: `DA_Potion_Health` (Restores 100 HP over 3s, 0.8s use time, 30% speed penalty); AC-4: `DA_SkillBook_Dash` (Requires Class Tag `Class.Vanguard`, grants `UGA_Dash`). |

### Nice to Have (4 hours / 0.5 days)
| ID | Task | Agent/Owner | Est. Days | Dependencies | Acceptance Criteria |
|---|---|---|---|---|---|
| `pzd-003` | [Stone Golem Boss PaperZD AnimBP & Aggro Integration](file:///mnt/Data/Projects/project-games/production/epics/core-character/story-003-boss-paperzd-animbp.md) | `animator-2d` | 0.5 (4h) | `pzd-001`, `net-003` | AC-1: `ABP_StoneGolem` configured with Idle, Walk, Slam Attack, Broken Stagger, and Death states; AC-2: PaperZD Anim Notify triggers ground slam AOE threat broadcast. |

## Total Planned Scope
- **Must Have**: 5 stories (40 hours / 5.0 days)
- **Should Have**: 2 stories (16 hours / 2.0 days)
- **Nice to Have**: 1 story (4 hours / 0.5 days)
- **Total**: 8 stories (60 hours / 7.5 days) vs **Available Capacity**: 8.0 days (64 hours)

## Risks
| Risk | Probability | Impact | Mitigation |
|---|---|---|---|
| PaperZD headless/nullrhi asset creation in Linux environment | Medium | High | Use Unreal Commandlet / Python Automation scripts for deterministic asset generation and verification. |
| Sprite texture filtering blur (Bilinear vs Nearest) | Medium | Medium | Force default texture group `TEXTUREGROUP_Pixels` and `Filter: Nearest` during import. |
| Isometric camera clipping with tall geometry | Low | Medium | Enforce occlusion dither material function on all blockout mesh materials. |

## Definition of Done for this Sprint
- [ ] All Must Have tasks completed
- [ ] All tasks pass acceptance criteria
- [ ] Automated integration tests pass without failure
- [ ] Two-axis code review completed for every story
- [ ] Level `L_Ruins_Blockout.umap` is playable in Editor with character movement and abilities working seamlessly
