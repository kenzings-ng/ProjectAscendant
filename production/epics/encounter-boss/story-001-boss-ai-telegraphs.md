# Story 001: Prototype Boss AI & 4-Phase Telegraph System

> **Epic**: Encounter & Boss Mechanics Layer  
> **Status**: ✅ Done  
> **Layer**: Encounter / AI  
> **Type**: Gameplay / AI  
> **Estimate**: 8 hours (1.0 days)  
> **Manifest Version**: 2026-09-23  
> **Last Updated**: 2026-09-23  

## Context

**GDD**: [`design/gdd/boss-ai.md`](../../design/gdd/boss-ai.md), [`design/gdd/combat-system.md`](../../design/gdd/combat-system.md)  
**Requirement**: `TR-boss-001` (Ironclad Warlord Behavior Tree, 4-phase attack sequence, EQS action scoring, wall stun)

**ADR Governing Implementation**: 
- [`ADR-0001: Open World MMO Combat Networking`](../../docs/architecture/adr-0001-open-world-mmo-combat-networking.md) (Server-authoritative combat state)
- [`ADR-0002: GAS Integration & PaperZD Pixel Sprites`](../../docs/architecture/adr-0002-gas-integration-paperzd-pixel-sprites.md)

**Engine**: Unreal Engine 5.8 | **Risk**: 🟡 MEDIUM  
**Engine Notes**: Implements `FPABossAIModel` / `UPABossAIComponent` with 4-phase attack timer lifecycle and EQS action evaluation.

---

## Acceptance Criteria

- [x] **AC-1 (4-Phase Attack Timing Lifecycle)**: Mọi đòn tấn công của Trùm tuân thủ nghiêm ngặt 4 pha:
  - **Telegraph Phase (0.50s–0.80s)**: Tỷ lệ lấp đầy decal đỏ $0\% \to 100\%$.
  - **Flash Cue Window (0.10s cuối)**: Khi $FillRatio \ge 0.85$, kích hoạt Flash Cue (cửa sổ vàng để người chơi phản xạ né).
  - **Active Hitbox Phase (0.15s–0.25s)**: Quét vùng sát thương thực tế theo decal.
  - **Recovery / Punish Window (0.60s–1.00s)**: Trùm rơi vào trạng thái hồi phục, cho phép người chơi phản công tích Posture.
- [x] **AC-2 (Moveset & Action Selection Score)**: 5 kỹ năng chiến đấu với trọng số EQS dựa trên khoảng cách và góc quay người chơi:
  - `Cleave Strike`: Cận chiến phía trước ($<350\text{cm}$, góc $\le 60^\circ$).
  - `Overhead Smash`: Vùng thẳng ($<450\text{cm}$).
  - `Iron Horn Charge`: Tầm xa ($>500\text{cm}$).
  - `Iron Tail Sweep`: Phía sau ($>120^\circ$, cự ly $<300\text{cm}$, 90% ưu tiên).
  - `Earthquake Stomp`: Vùng tròn tâm ($<500\text{cm}$, chỉ kích hoạt ở Pha 3).
- [x] **AC-3 (3-Phase Health Progression)**:
  - Phase 1 (100% → 75% HP): Đòn đơn lẻ, hồi chiêu dài (0.90s).
  - Phase 2 (75% → 25% HP): Kích hoạt combo 2-3 đòn, hồi chiêu giảm xuống 0.70s.
  - Phase 3 (<25% HP - Enrage): Tốc độ di chuyển tăng $420 \to 480\text{cm/s}$, mở khóa Earthquake Stomp.
- [x] **AC-4 (Wall Crash Stun)**: Khi trùm dùng kỹ năng Charge húc vào chướng ngại vật/tường, chuyển sang trạng thái `State.WallStunned` trong đúng 1.80s (người chơi được thưởng thời gian gây sát thương tự do).

---

## Implementation Notes

1. **`PABossAITypes.h`**:
   - `EPABossAttackPhase`: `Idle`, `Telegraph`, `FlashCue`, `ActiveHitbox`, `Recovery`, `WallStunned`, `Staggered`.
   - `EPABossAttackType`: `CleaveStrike`, `OverheadSmash`, `IronHornCharge`, `IronTailSweep`, `EarthquakeStomp`.
   - `FPABossAttackData`: Damage, TelegraphDuration, HitboxDuration, RecoveryDuration, Range, MaxAngle, RequiredPhase.
   - `FPABossAIModel`: Pure data model quản lý state machine, cooldowns, EQS action selector, tick-driven attack lifecycle, và wall stun logic.
2. **`PABossAIComponent.h` / `PABossAIComponent.cpp`**:
   - ActorComponent bọc data model để điều khiển Boss Character trong gameplay.

---

## QA Test Cases

- **Test 1: 4-Phase Progression**: Kiểm tra tiến trình thời gian từ Telegraph -> Flash Cue (cuối 0.10s) -> Hitbox -> Recovery -> Idle.
- **Test 2: EQS Selection Logic**: Người chơi đứng sau lưng trùm -> ưu tiên Tail Sweep; người chơi đứng xa >500cm -> ưu tiên Horn Charge.
- **Test 3: Phase Thresholds**: Boss HP từ 100% -> 60% kích hoạt combo Phase 2; <25% mở khóa Earthquake Stomp và tăng tốc chạy.
- **Test 4: Wall Crash Stun**: Gọi sự kiện va chạm tường khi đang Charge -> trùm rơi vào WallStunned đúng 1.8s.
