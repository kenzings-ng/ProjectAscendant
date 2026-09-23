# Control Manifest: Project Ascendant

> **Engine**: Unreal Engine 5.7  
> **Last Updated**: 2026-09-16  
> **Manifest Version**: 2026-09-16  
> **ADRs Covered**: [ADR-0001](adr-0001-open-world-mmo-combat-networking.md), [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md), [ADR-0003](adr-0003-server-authoritative-grid-inventory-fast-array.md)  
> **Status**: Active — regenerate with `/create-control-manifest update` when ADRs change  

`Manifest Version` is the date this manifest was generated. Story files embed this date when created. `/story-readiness` compares a story's embedded version to this field to detect stories written against stale rules. Always matches `Last Updated` — they are the same date, serving different consumers.

This manifest is a programmer's quick-reference extracted from all Accepted ADRs, technical preferences, and engine reference docs. For the reasoning behind each rule, see the referenced ADR.

---

## 1. Foundation Layer Rules

*Applies to: scene management, event architecture, save/load, engine initialisation, networking baseline, data replication*

### Required Patterns
- **Dedicated Server Authority**: Dedicated Server owns 100% of game state (HP, Mana, Stamina, Posture, inventory slot contents, transaction commits). Clients are strictly dumb terminals with local prediction — source: [ADR-0001](adr-0001-open-world-mmo-combat-networking.md)
- **Lag Compensation Rewind Buffer**: All hit validation must query a 200ms circular history buffer of character capsule transforms on the Dedicated Server — source: [ADR-0001](adr-0001-open-world-mmo-combat-networking.md)
- **Iris 3-Tier Spatial Filtering**: Network replication must be prioritized dynamically across 3 spatial tiers: Tier 1 ($\le 15\text{m}$, 60Hz), Tier 2 ($15\text{m} - 45\text{m}$, 20Hz), Tier 3 ($45\text{m} - 100\text{m}$, 5Hz) — source: [ADR-0001](adr-0001-open-world-mmo-combat-networking.md)
- **FastArray Inventory Serialization**: All replicated inventory item collections must inherit from `FFastArraySerializer` and implement `NetDeltaSerialize` — source: [ADR-0003](adr-0003-server-authoritative-grid-inventory-fast-array.md)
- **Transactional Server RPCs**: Client operations modifying items (Move, Equip, Split, Drop, Salvage) must send atomic transaction Server RPCs; clients update UI optimistically and rollback if the Server rejects the transaction — source: [ADR-0003](adr-0003-server-authoritative-grid-inventory-fast-array.md)
- **GAS Attribute Replication**: All gameplay attributes (Health, Mana, Stamina, Posture) must reside in a `UAttributeSet` and replicate via `GAMEPLAYATTRIBUTE_REPNOTIFY` — source: [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md)

### Forbidden Approaches
- **Never trust client hit claims**: Never accept a client RPC saying "I hit target X for Y damage". The client only sends input intentions (e.g. `Server_PerformLightAttack`); the Server performs the sweep and calculates the result — source: [ADR-0001](adr-0001-open-world-mmo-combat-networking.md)
- **Never replicate raw `TArray` for item collections**: Never use `UPROPERTY(Replicated) TArray<FInventorySlot>` because any slot change causes full-array serialization and network saturation — source: [ADR-0003](adr-0003-server-authoritative-grid-inventory-fast-array.md)
- **Never allow client-side mutation of persistent game state**: Inventory, currency, and stat allocations must never be mutated locally on the client without Server validation — source: [ADR-0003](adr-0003-server-authoritative-grid-inventory-fast-array.md)
- **Never broadcast raw transforms at 60Hz to all clients**: Never disable spatial replication filters for moving entities; distant actors must be culled or updated at low frequency — source: [ADR-0001](adr-0001-open-world-mmo-combat-networking.md)
- **Never use Peer-to-Peer or listen-server architectures**: Project Ascendant is built strictly on Dedicated Servers; do not write game logic assuming a local host player — source: [ADR-0001](adr-0001-open-world-mmo-combat-networking.md)

### Performance Guardrails
- **Client Network Bandwidth**: Max 50 KB/s ingress, 20 KB/s egress in dense 20-player combat — source: [ADR-0001](adr-0001-open-world-mmo-combat-networking.md)
- **Dedicated Server Tick Rate**: Fixed 60Hz tick rate; automatic AI shedding to 30Hz if server CPU load exceeds 80% — source: [ADR-0001](adr-0001-open-world-mmo-combat-networking.md)
- **Lag Compensation Query**: Max 0.5ms per hit validation query on server game thread — source: [ADR-0001](adr-0001-open-world-mmo-combat-networking.md)
- **FastArray Replication Delta**: Max 0.8ms per server tick for all connected player inventory delta updates — source: [ADR-0003](adr-0003-server-authoritative-grid-inventory-fast-array.md)

---

## 2. Core Layer Rules

*Applies to: core gameplay loops, player locomotion, combat execution, collision, evasion, stagger*

### Required Patterns
- **PaperZD AnimNotify to GAS Bridge**: Combat attack montages must place custom PaperZD AnimNotify tracks (`AnimNotify_ActiveHitboxStart` / `End`) to trigger server-side 3D Chaos capsule sweeps — source: [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md)
- **True 3D Chaos Sweep Volumes**: Hitboxes must use 3D Chaos collision sweeps (e.g. `SweepMultiByChannel` with capsule or box geometry) aligned with the isometric ground plane, not 2D sprite box collisions — source: [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md)
- **Server-Authoritative GameplayEffects**: Damage, healing, posture damage, and status effects must be applied exclusively via `UGameplayEffect` on the Dedicated Server — source: [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md)
- **I-Frame Tag Application**: Dash/evasion mechanics must grant `GameplayTag.State.Invulnerable` during the 0.25s active window via a replicated GameplayEffect — source: [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md)
- **Posture Break (Stagger) Execution**: When Posture reaches 100%, the entity must enter `GameplayTag.State.Staggered`, triggering a 3.0s vulnerability window and critical damage multipliers — source: [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md)
- **Input Buffer Queue**: The combo state machine must support a 250ms input buffer queue to capture queued attacks during animation recovery frames — source: [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md)

### Forbidden Approaches
- **Never use invisible 3D skeletal mesh proxies under 2D sprites**: Characters are rendered directly via PaperZD flipbook components. Do not attach hidden 3D skeletal meshes to drive hitboxes — source: [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md)
- **Never evaluate hitbox collision on client machines**: Clients only render cosmetic attack effects and play local feedback sounds; they must never report overlaps as confirmed hits — source: [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md)
- **Never use raw Actor `Tick()` to poll ability cooldowns or status durations**: Cooldowns and durations must be managed strictly via GAS timer delegates or `FTimerManager` — source: [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md)
- **Never bypass CharacterMovementComponent for dash locomotion**: Dash evasion must execute through `UCharacterMovementComponent` root motion or velocity launch to ensure network prediction and rewind synchronization — source: [ADR-0001](adr-0001-open-world-mmo-combat-networking.md)

### Performance Guardrails
- **Hitbox 3D Sweep Budget**: Max 2.5ms total server tick time dedicated to combat physics sweeps across all active combatants — source: [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md)
- **Input Responsiveness**: Input-to-animation reaction latency on client must not exceed 1 frame (16.67ms) — source: [ADR-0002](adr-0002-gas-integration-paperzd-pixel-sprites.md)

---

## 3. Feature Layer Rules

*Applies to: secondary mechanics, AI systems, boss encounters, economy, crafting, progression*

### Required Patterns
- **StateTree Hierarchical AI**: Boss and elite mob AI must be implemented using Unreal Engine 5.7 **StateTree**, partitioning logic into idle, patrol, combat, and phase transitions — source: [`docs/architecture/architecture.md`](architecture.md)
- **Immutable Item Metadata in DataAssets**: Item static attributes (base mesh, display name, icon, base stats, lore) must reside in `UPrimaryDataAsset` (`UItemStaticDataAsset`) — source: [ADR-0003](adr-0003-server-authoritative-grid-inventory-fast-array.md)
- **Mutable Instance State in Structs**: Dynamic item attributes (Durability, Affixes, Enhancement Level, Gem Sockets) must be stored inside `FItemInstanceData` within the FastArray item entry — source: [ADR-0003](adr-0003-server-authoritative-grid-inventory-fast-array.md)
- **Zone Checkpoint & Forge Proximity Validation**: Crafting, upgrading, and merchant interactions must validate player physical proximity ($\le 250\text{cm}$) to the relevant world actor before executing transactions — source: [ADR-0003](adr-0003-server-authoritative-grid-inventory-fast-array.md)

### Forbidden Approaches
- **Never store transient runtime state inside `UPrimaryDataAsset`**: DataAssets are shared global assets; never modify their properties at runtime — source: [ADR-0003](adr-0003-server-authoritative-grid-inventory-fast-array.md)
- **Never run full AI behavior updates for entities outside 30m**: AI entities with no players within 30m must pause active perception and drop tick frequency to 2Hz — source: [`docs/architecture/architecture.md`](architecture.md)

### Performance Guardrails
- **AI Game Thread Budget**: Max 1.5ms per frame on client/server for all AI evaluation and EQS queries — source: [`docs/architecture/architecture.md`](architecture.md)
- **Inventory Snapshot Size**: Total serialized inventory data for a player character must not exceed 2 KB on initial join, and $<100\text{ bytes}$ per transaction delta — source: [ADR-0003](adr-0003-server-authoritative-grid-inventory-fast-array.md)

---

## 4. Presentation Layer Rules

*Applies to: rendering, audio, UI, VFX, shaders, animations, HD-2D aesthetics*

### Required Patterns
- **CommonUI for All Screens & Menus**: All UI widgets must inherit from CommonUI base classes (`UCommonButtonBase`, `UCommonActivatableWidget`, `UCommonUserWidget`) to guarantee gamepad D-pad routing and focus stacks — source: [`design/ux/interaction-patterns.md`](file:///mnt/Data/Projects/project-games/design/ux/interaction-patterns.md)
- **Texture Atlas Sprite Batching**: All character sprite sheets must be batched into $2048 \times 2048$ texture atlases to draw each character in a single draw call — source: [`design/art/art-bible.md`](file:///mnt/Data/Projects/project-games/design/art/art-bible.md), [`docs/architecture/architecture.md`](architecture.md)
- **Pixel-Art Texture Filtering**: Texture filtering for all 2D pixel assets must be set to **Nearest Neighbor** without Mipmap generation to maintain crisp pixel boundaries — source: [`design/art/art-bible.md`](file:///mnt/Data/Projects/project-games/design/art/art-bible.md)
- **Object-Pooled Floating Combat Text**: Floating damage numbers and combat feedback popups must be recycled through a pre-allocated pool (max 64 instances) to prevent runtime garbage collection hitching — source: [`design/ux/interaction-patterns.md`](file:///mnt/Data/Projects/project-games/design/ux/interaction-patterns.md)
- **Layered Resource Ghosting Bars**: Health and Stagger meters must display a delayed ghost bar ($0.4\text{s}$ delay) behind current values to communicate combat impact clearly — source: [`design/ux/interaction-patterns.md`](file:///mnt/Data/Projects/project-games/design/ux/interaction-patterns.md)

### Forbidden Approaches
- **Never use Cascade particle systems**: Cascade is completely deprecated in UE 5.7; all visual effects (slashes, magic, sparks) must use Niagara — source: `docs/engine-reference/unreal/deprecated-apis.md`
- **Never use legacy Sound Cues for complex interactive audio**: Interactive combat audio and contextual vocal tracks must use MetaSounds — source: `docs/engine-reference/unreal/deprecated-apis.md`
- **Never use Bilinear/Trilinear filtering or Mipmaps on character pixel sprites**: Mipmaps cause blur and edge artifacts on 2.5D pixel art sprites and waste 33% VRAM — source: [`design/art/art-bible.md`](file:///mnt/Data/Projects/project-games/design/art/art-bible.md)

### Performance Guardrails
- **Total Frame Budget**: 16.67ms (target 60 FPS at 1080p / 1440p) — source: [`docs/architecture/architecture.md`](architecture.md)
  - Game Thread: $\le 5.0\text{ms}$
  - Render Thread: $\le 4.5\text{ms}$
  - GPU Time: $\le 6.0\text{ms}$
- **Total Draw Calls**: Max 800 draw calls per frame — source: [`docs/architecture/architecture.md`](architecture.md)
- **Dynamic Lights**: Max 4 dynamic shadow-casting point lights in any visible camera frustum — source: [`docs/architecture/architecture.md`](architecture.md)
- **VRAM Ceiling**: Max 3.5 GB VRAM usage at 1080p — source: [`docs/architecture/architecture.md`](architecture.md)
- **System RAM Ceiling**: Max 4.5 GB client RAM usage — source: [`docs/architecture/architecture.md`](architecture.md)

---

## 5. Global Rules (All Layers)

### 5.1 C++ Naming Conventions
| Element | Convention | Example |
|---|---|---|
| `UObject` / Component Classes | Prefix `U` | `UPostureComponent`, `UCharacterAttributeSet` |
| `AActor` Classes | Prefix `A` | `APABaseCharacter`, `APABossCharacter` |
| `USTRUCT` / Pure Structs | Prefix `F` | `FCombatHitResult`, `FItemInstanceData` |
| Interfaces | Prefix `I` | `ICombatantInterface`, `IInteractableInterface` |
| Enums | Prefix `E` | `ECharacterClass`, `EItemRarity` |
| Booleans | Prefix `b` | `bIsInvulnerable`, `bWasParried` |
| Member Variables | PascalCase | `CurrentHealth`, `MaxStamina` |
| Signals / Delegates | `On[Action]` | `OnPostureBroken`, `OnItemEquipped` |
| Source Files | PascalCase matching class | `PACombatComponent.h`, `PACombatComponent.cpp` |
| Constants | Prefix `k` or constexpr in namespace | `kMaxInputBufferSize = 0.25f` |

### 5.2 Smart Pointers & Memory Management
- **`TObjectPtr<T>` for Member UObjects**: Never use raw C++ pointers (`MyClass*`) for UObject member variables; always use `TObjectPtr<T>` — source: `docs/engine-reference/unreal/deprecated-apis.md`
- **`TWeakObjectPtr<T>` for Non-Owning References**: Always use `TWeakObjectPtr<T>` for cross-actor references to avoid dangling pointers and prevent garbage collection memory leaks — source: [`docs/architecture/architecture.md`](architecture.md)
- **Default Disable Actor Tick**: All Actors must initialize with `PrimaryActorTick.bCanEverTick = false;` in their constructors unless per-frame continuous math is explicitly required — source: [`docs/architecture/architecture.md`](architecture.md)
- **Const Correctness**: Mark all read-only methods `const`. Pass structs and strings by `const &` (e.g. `const FVector&`, `const FItemInstanceData&`) — source: [`docs/architecture/architecture.md`](architecture.md)

### 5.3 Approved Core Plugins & Modules
- **Enhanced Input**: Input mapping contexts, chorded actions, and rebindable controls
- **Gameplay Ability System (GAS)**: GameplayAttributes, GameplayEffects, GameplayTags
- **Iris Replication System**: Bandwidth-optimized, spatial-filtered network replication
- **PaperZD**: 2D animation blueprints, flipbook state machines, and anim notifies
- **CommonUI**: Input routing, cross-platform focus trees, and modal stacks
- **Niagara VFX**: High-performance GPU and CPU particle simulation
- **MetaSounds**: Node-based procedural and interactive audio synthesis

### 5.4 Forbidden APIs (Unreal Engine 5.7)
| Forbidden / Deprecated API | Approved Replacement | Reason |
|---|---|---|
| `InputComponent->BindAction()` / `BindAxis()` | Enhanced Input `BindAction()` | Legacy input system deprecated; lacks runtime remapping |
| `CascadeParticleSystem` | Niagara (`UNiagaraComponent`) | Cascade fully deprecated in UE5; cannot leverage modern GPU simulation |
| Legacy Sound Cue (for complex logic) | MetaSounds | Sound Cues lack procedural parameter modulation |
| UE4 World Composition / Level Streaming Volumes | World Partition & Data Layers | World Composition deprecated; World Partition handles cell streaming |
| `UGameplayStatics::LoadStreamLevel()` | World Partition Data Layers | Incompatible with World Partition streaming architecture |
| `DOREPLIFETIME()` (unconditional) | `DOREPLIFETIME_CONDITION()` / Iris | Unconditional replication causes bandwidth saturation |
| Raw C++ pointers (`UObject*`) | `TObjectPtr<UObject>` | Fails to integrate with UE5 memory tracking & access logging |
| Primary Actor `Tick()` enabled by default | Event-Driven / `FTimerManager` | Actor ticking drains CPU game thread budget |

---

## 6. Verification & Enforcement

Every pull request and implementation story must verify compliance against this Control Manifest:
1. **Automated Static & Linter Checks**: CI rejects raw pointers, deprecated APIs, or unconditioned replication macros.
2. **Story Acceptance Gate**: `/story-readiness` checks that new stories conform to the rules of this manifest version (`2026-09-16`).
3. **Architecture Review**: Any deviation from this manifest requires an approved amendment to the governing ADR before code merge.
