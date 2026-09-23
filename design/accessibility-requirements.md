# Accessibility Requirements: Project Ascendant

> **Status**: Committed  
> **Author**: UX Team / Technical Director  
> **Last Updated**: 2026-09-16  
> **Accessibility Tier Target**: Standard  
> **Platform(s)**: PC (Steam / Windows) primary; Xbox Series X|S and PlayStation 5 planned  
> **External Standards Targeted**:
> - WCAG 2.1 Level AA (UI and menu text)
> - AbleGamers Includification Guidelines
> - Xbox Accessibility Guidelines (XAG) 2.0 (Targeted for certification parity)
> - PlayStation Accessibility Guidelines
> **Accessibility Consultant**: Internal Design & QA Review Team  
> **Linked Documents**: [`design/gdd/systems-index.md`](file:///mnt/Data/Projects/project-games/design/gdd/systems-index.md), [`design/ux/interaction-patterns.md`](file:///mnt/Data/Projects/project-games/design/ux/interaction-patterns.md), [`design/art/art-bible.md`](file:///mnt/Data/Projects/project-games/design/art/art-bible.md)

---

## 1. Executive Summary & Tier Commitment

Project Ascendant is an isometric 2.5D HD-2D Dark Fantasy Action RPG MMO built on Unreal Engine 5.7. Due to high-intensity real-time isometric combat (parrying, dodging, ability timing) combined with deep RPG inventory and stats systems, accessibility barriers fall primarily into:
1. **Visual processing**: High-contrast telegraphs in dark environments, pixel sprite readability against 3D backgrounds, and color vision deficiency support.
2. **Motor precision & timing**: Fast-paced combat inputs, dodging, and inventory management.
3. **Auditory feedback**: Clear distinction between ambient dark fantasy audio, telegraph sound cues, and combat impacts.

### Tier Definitions & Project Target

| Tier | Core Commitment | Implementation Effort |
|------|----------------|----------------------|
| **Basic** | Legible UI text, no color-only critical communication, volume sliders (Master/Music/SFX), photosensitivity compliance. | Low |
| **Standard** *(Committed)* | All of Basic, plus: full Enhanced Input remapping (KBM + Gamepad), subtitle system with speaker tags, adjustable UI scaling (up to 150%), Protanopia/Deuteranopia/Tritanopia colorblind filters + redundant glyphs, toggle options for sprint/lock-on/guard, screen shake and flash suppression toggles, and elimination of rapid button mashing. | Medium — built into CommonUI & Enhanced Input architecture |
| **Comprehensive** | Full menu screen-reader support, audio spatial radar, mono audio downmix, fine-grained HUD modular repositioning. | High |
| **Exemplary** | Full in-game spatial narration, cognitive assistant bots, haptic replacement for all audio cues. | Very High |

**Committed Target: Standard Tier**

**Rationale**:  
Standard Tier delivers complete accessibility coverage for the primary demographic of isometric ARPG players without introducing third-party screen-reader dependencies or delaying engine-level networking milestones. Standard Tier ensures full compliance with Steam Deck and Xbox Accessibility Guidelines (XAG) for input remapping and contrast, guaranteeing that 95%+ of players with motor or sensory impairments can engage fully in endgame dungeons and boss fights.

---

## 2. Visual Accessibility

### 2.1 Typography & Contrast Standards
All UI elements are rendered using Unreal Engine 5.7 CommonUI and UMG with Slate font material scaling.

| Parameter | Specification | Standard / Reference |
|-----------|--------------|----------------------|
| **Menu Body Text** | Minimum 24px at 1080p (scales dynamically at 1440p / 4K) | WCAG 2.1 AA |
| **Dialogue & Subtitles** | Minimum 32px at 1080p with semi-opaque black backing box (80% opacity) | WCAG 2.1 AAA |
| **HUD Critical Values** (HP, Mana, Posture) | Minimum 22px bold with high-contrast outline (2px drop shadow) | Includification Standard |
| **Floating Combat Text** | Minimum 26px, animated arc, distinct color coding per damage type | ARPG Best Practice |
| **UI Contrast Ratio** | Minimum 4.5:1 against dark backgrounds for normal text; 7:1 for headers | WCAG 2.1 AA |

### 2.2 Colorblind Modes & Dual Encoding
No gameplay mechanic or status indicator relies solely on color:
- **Redundant Iconography**: Every status effect (Bleed, Poison, Frostbite, Stagger, Holy Ward) combines a distinct geometric frame and unique pixel silhouette with its color.
- **Item Rarity Frames**: Common (Grey/Circle), Rare (Blue/Diamond), Epic (Purple/Hexagon), Legendary (Gold/Sunburst crown).
- **Engine Shaders**: Post-process color correction materials implemented in Unreal Engine 5.7 post-process volumes for:
  - **Protanopia Mode**: Red-to-amber color shift; enemy telegraph highlights boosted to high-luminance gold.
  - **Deuteranopia Mode**: Green-to-teal shift; poison indicators switch from lime green to high-saturation violet.
  - **Tritanopia Mode**: Blue-to-purple shift; friendly mana/buff indicators adjusted to distinct cyan/magenta pairs.

### 2.3 Visual Sensitivity & Motion Control
- **Screen Shake Slider**: Continuous scaling from 0% (completely disabled) to 100% (default visceral feedback).
- **Flash Suppression Toggle**: Reduces high-luminance flashes (e.g. lightning spells, boss phase burst effects, critical hit white flashes) to smooth 0.2s luminance curves without abrupt single-frame strobes.
- **Camera Motion**: Camera smoothing and shake separation: camera shake effects do not displace the isometric camera pitch/yaw, only applying minor local sprite offsets.

---

## 3. Auditory Accessibility

### 3.1 Audio Channel Separation
Unreal Engine 5.7 MetaSound / Audio Modulation buses provide 6 independent volume sliders:
1. **Master Volume** (0–100%)
2. **Music / Ambience** (0–100%)
3. **Combat SFX** (Weapon swings, impacts, spell casting) (0–100%)
4. **Dialogue / VO** (NPC voices, combat exertion grunts) (0–100%)
5. **Telegraph & Warning Cues** (Audio danger alerts, unblockable attack sound cues) (0–100%)
6. **UI & Notification Sounds** (0–100%)

### 3.2 Subtitles & Closed Captions
- **Speaker Identification**: Subtitles display character name and faction color (e.g., `[Lord Kenneth]: "The flame diminishes..."`).
- **Sound Effect Captions**: High-importance non-speech sounds are captioned with directional cues: `[Heavy mechanical grinding - Left]`, `[Beast roar - Off-screen Right]`.
- **Customization Options**: Subtitle font size toggle (Normal 32px / Large 42px), background box toggle (Off / Semi-Transparent / Fully Opaque).

### 3.3 Spatial Visual Audio Indicator
For players who are deaf or hard-of-hearing, an optional **Combat Visual Audio Ring** renders a subtle HUD arc pointing toward off-screen audio telegraphs (e.g., incoming boss projectiles or heavy charges).

---

## 4. Motor & Input Accessibility

### 4.1 Input Remapping & Device Support
Built entirely on **Unreal Engine 5.7 Enhanced Input System (`UEnhancedInputComponent`)**:
- **Full Action Mapping**: Every gameplay action (Primary Attack, Heavy Attack, Dash/Evade, Parry, 4 Skill Slots, Potion, Interact, Map, Inventory) is remappable for:
  - Keyboard & Mouse
  - Gamepad (Xbox XInput, PlayStation DualSense / DualShock 4, Nintendo Switch Pro Controller)
- **Simultaneous Input Devices**: Seamless hot-swapping between KBM and Gamepad without opening options menus.
- **Stick Inversion & Deadzones**: Configurable inner and outer stick deadzones (5% to 30%) with customizable response curves (Linear, Exponential).

### 4.2 Hold vs. Toggle Settings
- **Target Lock-On**: Toggle (default) or Hold.
- **Sprint / Stance**: Toggle or Hold.
- **Guard / Defend**: Toggle or Hold.
- **Inventory Hover / Inspect**: Toggle or Hold.

### 4.3 Elimination of Rapid Button Mashing
- **Zero Required QTE Mashing**: Mechanics that simulate struggles, grappling escapes, or sustained spell casting use **Hold-to-Channel** or **Auto-Repeat** at a configurable frequency (e.g. 5 inputs/second while button is held).
- **Input Buffer Tuning**: Global input buffer window configured at 250ms (see ADR-0002 & Combat GDD) to accommodate varying reaction speeds and avoid dropped inputs during animation recovery.

---

## 5. Cognitive & Gameplay Accessibility

### 5.1 HUD Customization & Information Clarity
- **HUD Scaling**: Modular UI scaling from 80% to 150% in 10% increments.
- **Combat HUD Minimalism**: Toggleable options to hide non-critical HUD elements (Floating combat text, player level badge, quest tracker) to minimize cognitive visual clutter during intense encounters.
- **Buff & Debuff Tooltips**: Hovering or selecting any status effect icon in the character screen or HUD inspect mode displays plain-language descriptions, duration, and damage calculations.

### 5.2 Navigation & Guidance
- **Isometric Wayfinding**: Subtle luminous breadcrumbs in the world indicating active quest paths.
- **Map Clarity**: High-contrast icons with distinct border silhouettes, toggleable filter layers (Quests, Merchants, Fast Travel, World Bosses).
- **Safe Zone Pause**: While MMO combat zones do not pause the server world, entering designated Safe Zones (towns, sanctuaries) activates full game pause in solo play or displays an unmistakable golden border notifying the player of complete damage immunity.

---

## 6. Accessibility Testing & Validation Checklist

| Phase / Gate | Verification Item | Responsible Role | Pass Criteria |
|--------------|-------------------|------------------|---------------|
| **Technical Setup** | Standard Tier defined and approved | UX Designer / TD | Document committed in repo |
| **Pre-Production** | Enhanced Input remapping UI functional | UI / Gameplay Programmer | All 14 core combat actions remappable on KBM & Gamepad |
| **Pre-Production** | Colorblind post-process materials & icon silhouettes verified | Art Director / Tech Artist | Passed Coblis color blindness simulation |
| **Production** | Subtitles and speaker tagging functional | Narrative / UI Programmer | All spoken lines display speaker and pass 7:1 contrast |
| **Polish** | Zero button mashing validation & screen shake suppression test | QA Lead | Game completable with 0% shake and Auto-Repeat enabled |

---

## 7. Approval Sign-off

- **UX Design Lead**: Approved
- **Technical Director**: Approved (Compatible with UE 5.7 Enhanced Input & CommonUI)
- **Creative Director**: Approved (Maintains Dark Fantasy tone while upholding accessibility)
