# Interaction Pattern Library: Project Ascendant

> **Status**: Stable  
> **Author**: UX Designer / Technical Director  
> **Last Updated**: 2026-09-16  
> **Version**: 1.0  
> **Engine**: Unreal Engine 5.7  
> **UI Framework**: Unreal Motion Graphics (UMG) + CommonUI Plugin  
> **Related Documents**:
> - [`design/art/art-bible.md`](file:///mnt/Data/Projects/project-games/design/art/art-bible.md) — Visual standards, dark fantasy palette, typography, ornamentation
> - [`design/accessibility-requirements.md`](file:///mnt/Data/Projects/project-games/design/accessibility-requirements.md) — Standard Tier commitments
> - [`design/gdd/combat-hud.md`](file:///mnt/Data/Projects/project-games/design/gdd/combat-hud.md) — Combat HUD GDD

---

## 1. Introduction & CommonUI Architecture

This interaction pattern library serves as the single source of truth for all user interaction behaviors in *Project Ascendant*. All UI widgets are built on Unreal Engine 5.7 **CommonUI** to guarantee seamless input switching between Keyboard/Mouse and Gamepads, deterministic focus management, and automatic back-stack navigation.

### Core CommonUI Base Classes Used:
- `UCommonUserWidget`: Base for all non-interactive visual containers.
- `UCommonButtonBase`: Base for all clickable, focusable, and hoverable controls.
- `UCommonActivatableWidget`: Base for screens, modals, and fullscreen menus that manage input capture and back-stack layers.
- `UCommonActivatableWidgetContainer`: Manages stacked menus (e.g. Main Menu -> Settings -> Keybinds).

---

## 2. Pattern Catalog Index

| Pattern Name | Category | Description | Primary Engine Class | Used In (Screens) | Status |
|--------------|----------|-------------|----------------------|-------------------|--------|
| **Button (Primary)** | Input | Main call-to-action button with dark fantasy bronze/gold trim. High visual emphasis. | `UCommonButtonBase` | Main Menu, Dialogue Confirm, Crafting | Stable |
| **Button (Secondary)** | Input | Auxiliary actions, cancel, or navigation tab. Subtle iron border. | `UCommonButtonBase` | Modal Dialogs, Settings, Inventory Tabs | Stable |
| **Button (Destructive)** | Input | Critical irreversible actions (Dismantle, Discard, Delete Character). Crimson glowing trim. | `UCommonButtonBase` | Item Salvage, Character Deletion | Stable |
| **Inventory Slot** | Game-Specific | Grid cell representing item slots, supporting drag-drop, hover comparison, and quick-equip. | `UCommonButtonBase` | Inventory Screen, Equipment Sheet, Stash | Stable |
| **Ability / Skill Slot** | Game-Specific | Combat action button displaying cooldown sweep, mana cost, and active activation state. | `UCommonButtonBase` | Combat HUD, Skill Tree Allocation | Stable |
| **Health & Posture Bar** | Game-Specific | Dynamic combat meter displaying HP, Mana, Stamina, and Stagger/Posture break with ghosting damage. | `UCommonUserWidget` | Combat HUD, Boss Health Bar | Stable |
| **Context Action Prompt** | Game-Specific | 2.5D World-space prompt hovering above interactables ("Press [E] to Speak / Loot"). | `UCommonUserWidget` | World Interaction, NPC dialogue, Chests | Stable |
| **Floating Combat Text** | Game-Specific | Screen-space / world-anchored damage numbers, critical strike popups, and posture break text. | `AActor` / Pooled Widget | Combat HUD, Floating Feedback | Stable |
| **Status Effect Icon** | Game-Specific | Buff / debuff indicator displaying radial duration cooldown and stack count badges. | `UCommonUserWidget` | Combat HUD, Player Frame, Boss Frame | Stable |
| **Dialogue Box** | Game-Specific | Narrative interaction interface featuring character portrait, typewriter dialogue, and choices. | `UCommonActivatableWidget` | NPC Dialogue, Story Quests | Stable |
| **Modal Dialog** | Feedback / Layout | Focus-trapping popup window darkening background screen for confirmations or alerts. | `UCommonActivatableWidget` | Confirmation Prompts, Alerts | Stable |
| **Tooltip & Item Compare** | Feedback | Contextual comparison popup displaying item stats, gear score diffs (green/red), and lore. | `UCommonUserWidget` | Inventory, Merchant, Equipment | Stable |
| **Focus Navigation** | Navigation | Deterministic D-pad and arrow navigation with sound feedback and focus memory. | `UCommonUIActionRouter` | All Menus and Panels | Stable |

---

## 3. Detailed Pattern Specifications

### 3.1 Button (Primary, Secondary, Destructive)
- **Visual Appearance**:
  - *Primary*: Dark wrought iron background (`#1A1715`), ornate aged bronze border (`#C89B3C`), ivory text (`#F0EDE6`).
  - *Secondary*: Slate stone background (`#141312`), weathered silver border (`#787672`), light grey text (`#D0CFCB`).
  - *Destructive*: Deep char background (`#1F0E0E`), bloodstone crimson border (`#A82424`), flame red text (`#FF6B6B`).
- **Interaction States**:
  - *Normal*: 100% scale, base border opacity 70%.
  - *Hovered (Mouse)*: Scale 103%, border opacity 100% with subtle inner glow, SFX: `SFX_UI_Hover`.
  - *Focused (Gamepad)*: Animated golden pulse corner brackets, SFX: `SFX_UI_Focus`.
  - *Pressed*: Scale 98%, brightness drop by 15%, SFX: `SFX_UI_Click`.
  - *Disabled*: 40% opacity, greyscale desaturation, no hover/click response.
- **Accessibility Notes**: Minimum hit box $48 \times 48\text{ px}$. Contrast ratio $\ge 4.5:1$. Full keyboard Enter/Space and Gamepad Face Button Bottom (`A` / `Cross`) activation.

### 3.2 Inventory Grid Slot & Item Cell
- **Visual Appearance**:
  - Square slot ($64 \times 64\text{ px}$ at 1080p, scales to $96\text{ px}$ at 4K).
  - Background: Sunken stone socket (`#100F0E`) with faint inner shadow.
  - Border: Color-coded by rarity (Common: Grey, Rare: Sapphire Blue, Epic: Amethyst, Legendary: Sun Gold).
  - Stack Count: Bottom-right corner in 16px bold white text with 1px black outline.
- **Interaction States**:
  - *Hover / Focus*: Rarity border brightness +30%, displays linked Item Comparison Tooltip instantly (0.1s debounce).
  - *Left Click / Face Button Down*: Select / pick up item for Drag-and-Drop.
  - *Right Click / Face Button Top (`Y` / `Triangle`)*: Context action (Use potion, Quick-equip weapon, Split stack).
  - *Item Moving*: Semi-transparent ghost sprite follows cursor / gamepad reticle. Original slot shows dashed highlight.
- **Data Linkage**: Directly backed by `FFastArraySerializer` on the client (ADR-0003). Mutations dispatch Server RPCs; client UI displays optimistic ghost states during validation.

### 3.3 Ability / Skill Slot (Combat HUD)
- **Visual Appearance**:
  - Circular or shield-shaped icon ($56 \times 56\text{ px}$).
  - Cooldown Overlay: Radial clockwise sweep using dynamic material instance (`MIC_CooldownSweep`), dark semi-transparent mask (`#000000AA`).
  - Remaining seconds displayed in center in bold white text when cooldown $> 1.0\text{s}$; tenths of a second when $< 1.0\text{s}$.
  - Cost Overlay: Dimmed with blue diagonal hash when insufficient Mana/Stamina.
  - Keybind Prompt: Small pill badge at bottom center (e.g. `[Q]`, `[E]`, `[R]`, `[X]`, `[RT + X]`).
- **Interaction States**:
  - *Triggered*: Instant golden flash flare (`0.15s`), triggers PaperZD animation montage via GAS bridge (ADR-0002).
  - *On Cooldown*: Rejected click produces soft audio error `SFX_UI_Ability_Cooldown` and subtle red border twitch.

### 3.4 Health, Stamina & Posture Bar
- **Visual Appearance**:
  - Layered `UProgressBar`:
    - Layer 1 (Background): Dark charcoal groove (`#0B0B0C`).
    - Layer 2 (Ghost Bar): Light orange/amber ghost fill that hangs for $0.4\text{s}$ before smoothly sliding down to current value.
    - Layer 3 (Current Value): Crimson Red (`#C42021`) for HP; Cobalt Blue (`#205FC4`) for Mana; Amber (`#D49B2A`) for Stamina; Stone Silver (`#B0B4BC`) for Posture.
- **Interaction & Feedback**:
  - *Damage Taken*: Current bar drops instantly; Ghost bar catches up after $0.4\text{s}$ at $150\%/\text{s}$ rate. Screen shakes proportionally to damage if enabled.
  - *Posture Break (Stagger)*: When posture reaches $100\%$, bar shatters with a white-hot flash and audio chime `SFX_Combat_PostureBreak`. Bar changes to glowing cracked crimson during the vulnerable stagger window.

### 3.5 Context Action World Prompt (2.5D World-Space)
- **Visual Appearance**:
  - 2.5D Billboard Widget anchored above world interactables (NPCs, Chests, Shrines).
  - Displays icon chip of primary interact key (`[E]` on KBM, `[X]` / `[Square]` on Gamepad) followed by localized action verb ("Talk", "Open Chest", "Inspect").
- **Behavior & Distance Culling**:
  - Appears smoothly via fade-in ($0.2\text{s}$) when player enters interaction radius ($250\text{ cm}$).
  - Automatically sorts depth with sprite characters and scene geometry to prevent occluding the player character.
  - Clamps to screen edge with subtle arrow pointer if target is off-screen but within active interaction tether.

### 3.6 Floating Combat Text (Damage Numbers)
- **Visual Appearance**:
  - High-visibility bitmap/SDF font with dark outline.
  - Color Palette:
    - Normal Physical Damage: Pure White (`#FFFFFF`).
    - Critical Strike: Bright Gold / Amber (`#FFD700`), 130% font scale, explosive upward pop.
    - Elemental Fire / Dark / Frost: Orange (`#FF5500`), Violet (`#8A2BE2`), Cyan (`#00FFFF`).
    - Healing: Emerald Green (`#2ECC71`).
    - Posture Damage: Light Slate (`#C0D0E0`).
- **Animation Pattern**:
  - Numbers pop upward in a slight randomized parabolic arc ($45^\circ$), deceleration apex at $0.3\text{s}$, fade to transparent by $0.8\text{s}$.
  - Re-used via pre-allocated actor/widget pool (max 64 concurrent damage numbers to eliminate memory allocations during heavy AOE spells).

### 3.7 Tooltip & Item Comparison Popup
- **Visual Appearance**:
  - Double-panel layout when comparing equipped vs. inventory item.
  - Left panel: Currently equipped item. Right panel: Inspected item.
  - Header: Item Name in rarity color + Item Type / Subtype.
  - Stats Differential: Green (`+#%`) for superior stats; Red (`-#%`) for inferior stats; Grey for identical stats.
- **Positioning**:
  - Smart viewport clamping: automatically flips from right of cursor to left if approaching screen edge.
  - Gamepad mode: Statically docks to dedicated inspection frame next to inventory grid.

### 3.8 Focus Management & Navigation (CommonUI)
- **Gamepad D-Pad & Thumbstick Routing**:
  - Clear orthogonal grid routing (Up/Down/Left/Right).
  - Explicit `NextWidget` and `PreviousWidget` links on edge cells to prevent cursor trapping.
  - Focus Restore: When closing a sub-menu (e.g. Settings dialog), CommonUI automatically restores focus to the button that spawned it.
- **Cancel / Back Stack**:
  - Gamepad `B` / `Circle` or Keyboard `Escape` always pops the top-most `UCommonActivatableWidget` in the active root container.

---

## 4. Audio-Visual Feedback Matrix

| Action | Visual Feedback | Audio Feedback | Haptic Feedback (Gamepad) |
|--------|----------------|----------------|---------------------------|
| Menu Navigate | Focus frame pulse | `SFX_UI_Focus` (subtle stone tap) | None |
| Menu Select | Button click depression + flash | `SFX_UI_Confirm` (deep gong/bell) | Low-intensity rumble ($0.05\text{s}$) |
| Menu Cancel / Back | Slide out left / fade out | `SFX_UI_Cancel` (wood click) | None |
| Item Move / Drop | Ghost sprite drop shadow | `SFX_UI_Item_Place` (cloth/metal rustle) | Very light click |
| Posture Break | Screen flash + cracked gauge | `SFX_Combat_PostureBreak` (glass shatter) | High-intensity rumble ($0.3\text{s}$) |
| Ability Ready | Golden glint on icon border | `SFX_UI_Ability_Ready` (subtle chime) | None |

---

## 5. Approval Sign-off

- **UX Lead**: Approved  
- **Technical Director**: Approved (100% compliant with UE 5.7 CommonUI & UMG standards)  
- **Art Director**: Approved (Consistent with HD-2D Dark Fantasy palette)  
