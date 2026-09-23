# Story 002: Skill Tree & Talent Node Binding

> **Epic**: Expansion Progression & Class Specialization  
> **Status**: Done  
> **Layer**: Expansion  
> **Type**: Logic & GAS  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-18  
> **Last Updated**: 2026-09-18  

## Context

**GDD**: [`design/gdd/foundational-classes.md`](../../design/gdd/foundational-classes.md) & [`design/gdd/skill-progression-system.md`](../../design/gdd/skill-progression-system.md)  
**Requirement**: `TR-prog-002`  

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Server-Authoritative talent unlocking and validation)
- [`ADR-0002: GAS Integration Strategy`](../../docs/architecture/adr-0002-gas-integration-strategy-paperzd-pixel-sprites.md) (Direct AttributeSet binding for passive talent modifiers)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟢 LOW  
**Engine Notes**: Implements `UPATalentTreeComponent` binding to `UPAProgressionComponent` and `UAscendantAttributeSet`.

**Control Manifest Rules (Expansion Layer)**:
- Required: Server-authoritative validation for all node unlocks and respec actions.
- Required: Skill point deduction via `UPAProgressionComponent::SpendSkillPoint()` on node unlock.
- Forbidden: Client-side speculative talent unlock without server acknowledgement.
- Guardrail: Talent nodes require level prerequisite and linear branch tier prerequisites.

---

## Acceptance Criteria

- [x] **AC-1 (3-Branch Specialization Trees)**: Cấu trúc cây kỹ năng 3 nhánh chuyên môn hóa cho 3 chức nghiệp:
  - Vanguard: `Ironclad` (Phòng ngự), `Duelist` (Phản đòn), `Juggernaut` (Công kích)
  - Ranger: `Marksman` (Sát thương tầm xa), `Windrunner` (Tật phong), `Trapper` (Bẫy)
  - Arcanist: `Pyromancer` (Hỏa thuật), `Chronomancer` (Thời gian), `Leyline` (Địa mạch)
- [x] **AC-2 (Passive Talent Node Unlocking & GAS Binding)**: Mở khóa các node nội tại (Passive Nodes) tiêu hao Skill Points từ `UPAProgressionComponent`, kiểm tra điều kiện cấp độ (Level) và node tiên quyết (Prerequisite). Tăng trực tiếp các chỉ số trên `UAscendantAttributeSet`:
  - `BonusAttackPower` (Tăng sát thương)
  - `BonusMaxPosture` (Tăng thanh thế đứng)
  - `DashCooldownReduction` (Giảm hồi chiêu Dash)
  - `BonusArmor` & `BonusMaxHealth`
- [x] **AC-3 (Respec / Talent Reset Loop)**: Hỗ trợ tẩy điểm kỹ năng (`ResetTalents`), hoàn trả toàn bộ Skill Points đã tiêu vào `UPAProgressionComponent`, gỡ bỏ các chỉ số cộng thêm khỏi `UAscendantAttributeSet` và làm sạch danh sách node đã mở khóa.

---

## Implementation Notes

1. **Talent Tree Types (`PATalentTreeTypes.h`)**:
   - `EPATalentTreeError`: `None`, `InvalidNodeId`, `NodeAlreadyUnlocked`, `PrerequisiteNotMet`, `InsufficientLevel`, `InsufficientSkillPoints`, `NoNodesToReset`, `NotAuthoritative`, `ServerRejected`.
   - Struct `FPATalentNode`: `NodeId`, `NodeName`, `BranchIndex`, `Tier`, `RequiredLevel`, `PrerequisiteNodeId`, `SkillPointCost`, stat modifiers (`BonusAttackPower`, `BonusMaxPosture`, `DashCooldownReduction`, `BonusArmor`, `BonusMaxHealth`).
   - Static registry `FPATalentTreeDatabase`: Definitions for Vanguard, Ranger, Arcanist 3-branch trees.
2. **Talent Tree Component (`PATalentTreeComponent.h` / `PATalentTreeComponent.cpp`)**:
   - `UnlockNode(FName NodeId, EPATalentTreeError& OutError)`
   - `ResetTalents(EPATalentTreeError& OutError)`
   - `IsNodeUnlocked(FName NodeId) const`
   - `GetUnlockedNodes() const`
   - `GetTotalDashCooldownReduction() const`
   - Server RPCs: `Server_UnlockNode`, `Server_ResetTalents`.
