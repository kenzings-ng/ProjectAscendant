# Story prog-001: Character XP, Leveling & Class Mastery

> **Status**: Done
> **Sprint**: 3
> **Priority**: Must-Have
> **Owner**: ue-gas-specialist
> **Estimate**: 1.0 days (8h)
> **Dependencies**: attr-001

---

## Summary

Implement `UPAProgressionComponent` — server-authoritative component quản lý XP, Leveling (1→50) và Skill Points.

Đường cong XP phi tuyến: `XPToNext(Lv) = ceil(100 * Lv^1.8)` tạo progression curve tự nhiên — level đầu nhanh, level cuối gian nan.

Mỗi lần lên cấp:
- MaxHealth += 25, MaxMana += 5, MaxStamina += 3
- AttackPower += 3, Armor += 2
- +1 Skill Point cho Skill Tree

---

## Acceptance Criteria

- [x] **AC-1**: `UPAProgressionComponent` quản lý XP đường cong phi tuyến Cấp 1→50; công thức `XPToNext(Lv) = ceil(100 * Lv^1.8)`; hỗ trợ multi-level-up; cap tại Level 50
- [x] **AC-2**: Mỗi cấp độ tăng trưởng chỉ số cơ sở (Health/Mana/Stamina/Attack/Armor) và ban thưởng 1 Điểm Kỹ Năng (Skill Point); SpendSkillPoint có validation
- [x] **AC-3**: Replicate CurrentLevel, CurrentXP, AvailableSkillPoints qua network và cập nhật AttributeSet

---

## Implementation Notes

### Files Created
- `Public/Progression/PAProgressionTypes.h` — Enums, formulas, reward struct
- `Public/Progression/PAProgressionComponent.h` — Component header
- `Private/Progression/PAProgressionComponent.cpp` — Server-authoritative implementation
- `Private/Progression/PAProgressionTests.cpp` — 5 TDD tests

### Key Formulas
| Formula | Expression | Example |
|---|---|---|
| XP to next level | `ceil(100 * Level^1.8)` | Lv1→2: 100 XP, Lv10→11: 6310 XP, Lv49→50: ~143K XP |
| Stat growth per level | Fixed increments | +25 HP, +5 MP, +3 STA, +3 ATK, +2 ARM |
| Skill Points | 1 per level-up | 49 total at max level |

### Architecture
- Server-authoritative: `GrantXP()` and `SpendSkillPoint()` require `HasAuthority()`
- Multi-level-up loop handles large XP grants efficiently
- Directly modifies `UAscendantAttributeSet` via `AbilitySystemComponent`
- Replicated via `DOREPLIFETIME` (CurrentLevel, CurrentXP, AvailableSkillPoints)
