# Package M5 — New Character Production & Animation Foundation Report

## 1. Executive Summary

Package M5 establishes the new production-quality character art foundation for **Project Ascendant**, moving definitively away from the obsolete legacy Paperdoll visual style. The new Vanguard character foundation is authored strictly on the project's native 128×128 master canvas grid, anchored to the standardized foot pivot `(64, 114)` and waist seam `Y = 80`, utilizing the project's official 32-color 4-tone ramp palette with consistent 10 o'clock lighting.

All legacy Paperdoll visual designs were completely ignored: zero tracing, zero repainting, zero upscaling, and zero cosmetic retouches were applied to the legacy assets. The new Vanguard prototype is built from scratch as a true modular character architecture featuring clean separation between anatomical base body, lower armor (greaves/sabatons), upper armor (cuirass/faulds/pauldrons), helm (bascinet/visor), and gauntleted weapon sockets (`hand_r` and `hand_l`).

A complete 22-frame Action-First animation foundation was produced and validated across four core states: **Idle** (4f), **Walk** (6f), **Run** (6f), and **Attack** (6f). Weapon and shield modular attachment was verified using production weapon assets (`WPN_Vanguard_01_Broadsword_Tier0_RustedIron.png` and `WPN_Vanguard_09_Iron_Round_Shield.png`), proving zero baking into character geometry.

The assets have undergone programmatic Art Gate validation via `Tools/Aseprite/character_art_gate.py` and passed all automated checks with zero failures.

---

## 2. Character Specification

```text
Character:      Vanguard
Class:          Heavy Tank / Knight
Archetype:      Melee Frontliner
Design source:  Original from-scratch pixel art conforming to SPEC-ART-2026-09-23-V2
Canvas:         128×128 pixels (native master rig)
Pivot:          (64, 114) — ground contact line
Waist Seam:     Y = 80 (covered continuously across Y=79..83)
Proportions:    Heroic Chibi 3.2–3.5 heads
Color Palette:  Project Ascendant 32-Color 4-Tone Ramp Palette
Lighting:       10 o'clock directional key light, specular glints on steel, deep shadow contouring
```

### Layer Architecture (Bottom to Top)
1. **Layer_BaseBody**: Anatomical humanoid base (head, neck, undertunic, arms, legs)
2. **Layer_ArmorLower**: Greaves, sabatons, and gold-trimmed poleyns
3. **Layer_ArmorUpper**: Steel cuirass, breastplate, faulds spanning waist seam, and pauldrons
4. **Layer_Helm**: Bascinet with visor T-slit and gold crest
5. **Layer_Hand_R**: Mainhand gauntlet tracking right hand socket
6. **Layer_Hand_L**: Offhand gauntlet tracking left hand socket

---

## 3. Animation Foundation (Action-First)

The animation set is designed around the **Action-First** philosophy: snappier startup frames, distinct key poses, and clear silhouette reads.

```text
Idle:            4 frames @ 150ms/f (Total: 600ms loop)
Walk:            6 frames @ 100ms/f (Total: 600ms loop)
Run:             6 frames @ 80ms/f  (Total: 480ms loop)
Attack:          6 frames @ 110ms/f (Total: 660ms action)
Total Frames:    22 frames
Direction model: 1-direction master rig with horizontal actor yaw flip in PaperZD (consistent with runtime)
```

### Breakdown of Motion Beats
* **Idle (Frames 0..3)**: Organic breathing cycle. Subtly raises chest on frame 1, reaches apex on frame 2, settles naturally on frame 3. No mechanical stepping.
* **Walk (Frames 4..9)**: Complete 6-frame locomotion cycle. Left foot contact -> push-off -> passing pose -> right foot contact -> push-off -> return passing. Every frame carries unique kinematic offsets.
* **Run (Frames 10..15)**: High-speed athletic gait with forward torso lean (`X + 2`). Left stride apex (high knee, body bob `Y - 2`) -> drive -> plant/compression -> right stride apex -> drive -> plant/compression.
* **Attack (Frames 16..21)**: Action-First Broadsword Slash:
  - *Frame 16 (Anticipation / Coil)*: Torso leans back (`X - 2`), blade coiled behind shoulder (`hand_r = [86, 64]`).
  - *Frame 17 (Windup / Step)*: Forward weight transfer, sword raised to apex ready to strike.
  - *Frame 18 (Active Strike / Impact Beat)*: Explosive forward slash (`X + 4`), blade fully extended (`hand_r = [96, 76]`).
  - *Frame 19 (Follow-Through)*: Heavy downward momentum (`hand_r = [88, 86]`), body recovers forward.
  - *Frame 20 (Recovery)*: Weight re-centering, pulling weapon back toward guard.
  - *Frame 21 (Return to Guard)*: Neutral battle-ready stance.

---

## 4. Modular Equipment & Weapon Separation

A core failure of the legacy Paperdoll assets was the baking of weapons and shields directly into character textures. Package M5 establishes true modular equipment attachment.

```text
Mainhand socket:  hand_r [X, Y] tracked dynamically per frame in metadata
Offhand socket:   hand_l [X, Y] tracked dynamically per frame in metadata
Sword test:       Content/Art/Weapons/WPN_Vanguard_01_Broadsword_Tier0_RustedIron.png (Grip: 8, 23)
Shield test:      Content/Art/Weapons/WPN_Vanguard_09_Iron_Round_Shield.png (Grip: 16, 16)
Weapon compatibility: PASS — Seamless socket binding across all 22 frames without clipping or detachment
Shield compatibility: PASS — Modular attachment to offhand gauntlet across all 22 frames
```

### Equipment Separation Proof
* `Content/Art/Characters/Vanguard/Sprites/vanguard_basebody_spritesheet.png`: Naked anatomical body layer.
* `Content/Art/Characters/Vanguard/Sprites/vanguard_character_spritesheet.png`: Armor-only composite. Weapons and shields are 100% absent from all layers.
* `Content/Art/Characters/Vanguard/Sprites/vanguard_equipped_combat_spritesheet.png`: Dynamically composed equipped sheet demonstrating runtime attachment via socket transforms.
* Gauntlets are rendered on dedicated top layers (`Layer_Hand_R`, `Layer_Hand_L`) to wrap around weapon and shield grips correctly without re-rendering weapons into armor.

---

## 5. Art Gate Validation Results

The character was validated against the project's deterministic Art Gate rules using `Tools/Aseprite/character_art_gate.py`.

```text
================================================================================
Character Art Gate Validation Results
================================================================================
Target Asset:         Content/Art/Characters/Vanguard/Source/vanguard_character.aseprite
Canvas Dimensions:    128×128 px (Composite spritesheet: 2816×128 px) — PASS
Alpha Channel:        Strictly binary alpha: alpha in {0, 255}, 0 semi-transparent pixels — PASS
Foot Pivot:           Foot ground contact maintained at Y = 114 (+-1px across all 22 frames) — PASS
Waist Seam:           Row Y = 80 continuously opaque across all frames (covered Y=79..83) — PASS
Silhouette:           100% legible at 128x128, 64x64, 32x32, and 16x16 thumbnail downsamplings — PASS
Palette Constraints:  32-Color 4-Tone Ramp Palette; Composite character uses 12 colors — PASS
Frame Sequences:      Idle (4f), Walk (6f), Run (6f), Attack (6f) — PASS
Motion Uniqueness:    0 duplicate or pixel-identical adjacent frames — PASS
Equipment Separation: Weapons/shields 0% baked into body or armor — PASS
Overall Status:       APPROVED
================================================================================
```

---

## 6. Generated Production Assets & Inventory

All production files have been generated and placed under `Content/Art/Characters/Vanguard/`:

```text
Content/Art/Characters/Vanguard/
├── Source/
│   └── vanguard_character.aseprite          (36,834 bytes, 22 frames, 6 layers, 0xA5E0 binary)
├── Sprites/
│   ├── vanguard_character_spritesheet.png   (2816×128 px, composite character without weapons)
│   ├── vanguard_basebody_spritesheet.png    (2816×128 px, anatomical base body)
│   └── vanguard_equipped_combat_spritesheet.png (2816×128 px, modular equipped combat demonstration)
├── Animations/
│   ├── Anim_Vanguard_Idle.gif               (4 frames @ 150ms, clean character)
│   ├── Anim_Vanguard_Walk.gif               (6 frames @ 100ms, clean character)
│   ├── Anim_Vanguard_Run.gif                (6 frames @ 80ms, clean character)
│   ├── Anim_Vanguard_Attack.gif             (6 frames @ 110ms, clean character)
│   ├── Anim_Vanguard_Equipped_Idle.gif      (4 frames @ 150ms, modular equipped)
│   ├── Anim_Vanguard_Equipped_Walk.gif      (6 frames @ 100ms, modular equipped)
│   ├── Anim_Vanguard_Equipped_Run.gif       (6 frames @ 80ms, modular equipped)
│   └── Anim_Vanguard_Equipped_Attack.gif    (6 frames @ 110ms, modular equipped)
└── metadata/
    └── vanguard_metadata.json               (Per-frame sockets for hand_r, hand_l, helm, waist, pivot)
```

---

## 7. Tooling & Pipeline Updates

```text
Tools/Aseprite/
├── generate_vanguard_character.py   (NEW: Production generator & Aseprite binary packager)
├── character_art_gate.py            (NEW: Deterministic Art Gate validation script for character sets)
├── build_vanguard_character_template.py (Reference template builder)
└── vanguard_character.aseprite      (Tooling mirror of master source)
```

* `Tools/Aseprite/aseprite_export_pipeline.py` was kept intact; existing modes (`--weapons`, `--armor`, `--paperdoll`) were verified to continue passing with identical results.
* Added deterministic validation logic in `character_art_gate.py` that verifies binary alpha, pivot drift, waist seam integrity, and thumbnail downscaling without requiring UI interaction.

---

## 8. Runtime Integration Status & Path

```text
Runtime integration completed:
- Native 128x128 production spritesheets exported to Content/Art/Characters/Vanguard/
- Sockets and animation tags formalized in vanguard_metadata.json
- Equipment socketing verified with production sword and shield

Runtime integration deferred:
- PaperZD AnimSequence / Flipbook asset compilation in Unreal Engine (UPaperFlipbook)
- PaperZD AnimBlueprint state machine graph wiring (UPAPaperZDAnimInstance)
- Locomotion speed threshold property distinction (Walk vs Run) in UPAPaperZDAnimInstance
- Hitbox animation notify state (UPAPaperZDNotifyState_Hitbox) re-binding

Reason:
Package M5 is specifically tasked with Character Art Production Foundation. In Unreal Engine 5.8,
PaperZD AnimGraph and StateMachineGraph assets are not fully scriptable via headless Python commands
and require native Editor creation. Modifying C++ classes (PABaseCharacter, UPAPaperZDAnimInstance)
or rewiring active gameplay blueprints in this package would violate the strict non-destructive
art migration boundary. Full PaperZD runtime wiring is scheduled for Package M6/M6.1.
```

---

## 9. Legacy Asset Disposition & Safety

```text
Legacy Paperdoll used as visual source: NO (0% trace/repaint, authored from scratch)
Legacy Paperdoll modified:              NO (Content/art/characters/paperdoll/ untouched)
Legacy character assets modified:       NO (Content/art/characters/vanguard/ untouched)
Legacy assets deleted:                  NO (0 files deleted)
Broken engine references:               0
```

All existing legacy character assets (`Content/art/characters/vanguard/`, `Content/art/characters/boss/`) remain completely intact and untouched to guarantee zero regression on existing gameplay and automated test suites.

---

## 10. Future Architecture / Follow-up Work

1. **Locomotion Tier in C++**: The existing `UPAPaperZDAnimInstance` checks a single boolean `bIsMoving` (no distinction between Walk and Run). In M6, add `bIsRunning` or an `ELocomotionGait` enum to drive the new Walk (6f) and Run (6f) flipbooks.
2. **State Machine Expansion**: The current Vanguard runtime state machine expects 6 states (Idle, Dash, Attack, Hurt, Stunned, Dead). The new foundation provides Idle, Walk, Run, Attack. M6 should introduce production Hurt, Stunned, and Dead animations matching this new art direction.
3. **PaperZD Sockets**: Create native PaperZD Sprite Sockets (`Hand_R`, `Hand_L`, `Helm`) on the generated `UPaperSprite` assets using the exact pixel coordinates specified in `vanguard_metadata.json`.
