# Story 002: GAS 3-Hit Combo & Posture Finisher Execution

> **Epic**: Core Combat & Abilities  
> **Status**: In Progress  
> **X14 (2026-10-10) — đối chiếu trạng thái** (trước đây ghi `Complete`): Bằng chứng: `Core.Combat.GASComboFinisherIntegration` trong `Tests/evidence/x11b-e34f428-ue-automation.log` (77/77 PASS). AC-2 chưa đạt ở runtime: lag-comp dùng đồng hồ client (M8), sát thương bỏ qua công thức GDD (M11); thông số combo/finisher chờ duyệt cảm giác chơi (DECISIONS.md §12). Bảng tổng: `production/qa/x14-status-reconciliation.md`.  
> **Layer**: Core  
> **Type**: Logic  
> **Estimate**: 8 hours (L)  
> **Manifest Version**: 2026-09-17  
> **Last Updated**: 2026-09-17  

## Context

**GDD**: [`design/gdd/combat-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/combat-system.md), [`design/gdd/attributes-system.md`](file:///mnt/Data/Projects/project-games/design/gdd/attributes-system.md)  
**Requirement**: `TR-cmbt-001`, `TR-cmbt-002`  
**ADR Governing Implementation**: [`ADR-0001: Open World MMO Combat Networking`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0001-open-world-mmo-combat-networking.md), [`ADR-0002: GAS Integration Strategy for PaperZD & 2.5D Pixel Sprites`](file:///mnt/Data/Projects/project-games/docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md)  

**Engine**: Unreal Engine 5.8 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Implements `UPAGameplayAbility_MeleeAttack` and `UPAGameplayAbility_Finisher`. Integrates with `PAPostureSyncComponent` and `PALagCompensationComponent`.

---

## Acceptance Criteria

- [x] **AC-1 (3-Hit Combo Sequence)**: Successive inputs within 1.2s chain Attack 1 (1.0x damage, 10 Posture damage) $\to$ Attack 2 (1.2x damage, 15 Posture damage) $\to$ Attack 3 (1.6x damage, 25 Posture damage). Delay $> 1.2\text{s}$ resets combo to Attack 1.
- [x] **AC-2 (AnimNotify Hitbox Raycast)**: PaperZD anim notify triggers sector trace (radius 180cm, angle $90^\circ$ forward cone) on server, validated via 200ms lag compensation history. *(X14 2026-10-10: chưa đạt / chưa có bằng chứng — xem dòng X14 ở đầu file.)*
- [x] **AC-3 (Posture Finisher Execution)**: When targeted enemy has `State.Broken` tag and distance $\le 250\text{cm}$, activating Finisher triggers execution sequence: target locked in 1.5s stun, attacker gains `State.Invulnerable` for 1.5s, dealing 25% max HP pure damage and resetting target posture.
