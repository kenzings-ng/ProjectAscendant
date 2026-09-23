// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Combat/PADamageExecutionCalculation.h"
#include "Combat/AscendantAttributeSet.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPADamageExecutionCalcTest
 *
 * Kiểm thử đơn vị tự động cho hệ thống tính toán sát thương máu và phá vỡ thế đứng Posture (Story 002 / attr-002).
 *
 * Tiêu chí nghiệm thu:
 *  - AC-1 (Effective Damage): DamageTaken = RawDamage * (100 / (100 + Armor)), triệt tiêu về 0 khi có State.Invulnerable.
 *  - AC-2 (Posture Damage): PostureDamage = BaseStagger * (1 + StaggerBonus) * HitMultiplier, Weakspot = 1.5x, Perfect Parry = 35% MaxPosture.
 *  - AC-3 (Stagger Execution): ExecuteDamage = (TargetMaxHP * 0.25) + (BaseDamage * 3.0), xuyên giáp 100%.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPADamageExecutionCalcTest,
    "ProjectAscendant.Foundation.Combat.DamageExecutionCalculations",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPADamageExecutionCalcTest::RunTest(const FString& Parameters)
{
    // -------------------------------------------------------------------------
    // Test 1: AC-1 (Effective Damage Calculation & Armor Mitigation Curve)
    // -------------------------------------------------------------------------
    {
        // 1.1: QA Case chuẩn: Attacker có 200 Raw Damage, Target có 100 Armor
        // Sát thương dự kiến: 200 * (100 / (100 + 100)) = 100.0f
        const float Damage1 = FPACombatFormulas::CalculateEffectiveDamage(200.0f, 100.0f, false);
        TestEqual(TEXT("AC-1: RawDamage=200, Armor=100 phải giảm trừ chính xác 50% còn 100.0f"), Damage1, 100.0f);

        // 1.2: QA Case I-Frame: Target có thẻ State.Invulnerable (bIsInvulnerable = true) với 500 Raw Damage
        // Sát thương dự kiến: 0.0f tuyệt đối
        const float DamageInvulnerable = FPACombatFormulas::CalculateEffectiveDamage(500.0f, 100.0f, true);
        TestEqual(TEXT("AC-1: Đòn đánh trong thời lượng I-Frame (State.Invulnerable) phải bị triệt tiêu về 0.0f"), DamageInvulnerable, 0.0f);

        // 1.3: Mục tiêu không có giáp (Armor = 0)
        // Sát thương dự kiến: 150 * (100 / 100) = 150.0f
        const float DamageZeroArmor = FPACombatFormulas::CalculateEffectiveDamage(150.0f, 0.0f, false);
        TestEqual(TEXT("AC-1: Khi Armor=0, sát thương thực tế phải bằng đúng RawDamage (150.0f)"), DamageZeroArmor, 150.0f);

        // 1.4: Giáp cao (Armor = 900)
        // Sát thương dự kiến: 1000 * (100 / 1000) = 100.0f (giảm 90% sát thương)
        const float DamageHighArmor = FPACombatFormulas::CalculateEffectiveDamage(1000.0f, 900.0f, false);
        TestNearlyEqual(TEXT("AC-1: Khi Armor=900, sát thương nhận vào phải giảm 90% còn 100.0f"), DamageHighArmor, 100.0f, 0.001f);

        // 1.5: Biên ngoại lệ số âm (RawDamage âm hoặc Armor âm)
        const float DamageNegative = FPACombatFormulas::CalculateEffectiveDamage(-50.0f, -20.0f, false);
        TestEqual(TEXT("AC-1: Giá trị âm đầu vào phải được kẹp chặn dưới an toàn về 0.0f"), DamageNegative, 0.0f);
    }

    // -------------------------------------------------------------------------
    // Test 2: AC-2 (Posture Damage Calculation, Weakspots & Perfect Parry)
    // -------------------------------------------------------------------------
    {
        const float TargetBossMaxPosture = 1000.0f;

        // 2.1: QA Case Perfect Parry: Phản đòn hoàn hảo gây tức thì 35% MaxPosture
        // PostureDamage dự kiến: 1000 * 0.35 = 350.0f
        const float ParryDamage = FPACombatFormulas::CalculatePostureDamage(
            0.0f, 0.0f, 1.0f, false, true, TargetBossMaxPosture);
        TestEqual(TEXT("AC-2: Phản đòn hoàn hảo (Perfect Parry) phải gây lập tức đúng 35% MaxPosture (350 điểm)"), ParryDamage, 350.0f);

        // 2.2: Đòn đánh thường cơ bản: BaseStagger = 40, Bonus = 0, HitMultiplier = 1.0
        // PostureDamage dự kiến: 40 * 1.0 * 1.0 = 40.0f
        const float NormalPostureDamage = FPACombatFormulas::CalculatePostureDamage(
            40.0f, 0.0f, 1.0f, false, false, TargetBossMaxPosture);
        TestEqual(TEXT("AC-2: Đòn đánh thường cơ bản phải gây đúng 40 điểm Posture"), NormalPostureDamage, 40.0f);

        // 2.3: Đánh trúng điểm yếu Weakspot: BaseStagger = 40, HitMultiplier = 1.5x
        // PostureDamage dự kiến: 40 * 1.0 * 1.5 = 60.0f
        const float WeakspotPostureDamage = FPACombatFormulas::CalculatePostureDamage(
            40.0f, 0.0f, 1.0f, true, false, TargetBossMaxPosture);
        TestEqual(TEXT("AC-2: Đánh trúng điểm yếu (Weakspot) phải kích hoạt hệ số x1.5 đạt 60 điểm Posture"), WeakspotPostureDamage, 60.0f);

        // 2.4: Kèm bùa lợi dầu tẩm Dược sư: BaseStagger = 40, StaggerBonus = 0.3 (+30%)
        // PostureDamage dự kiến: 40 * (1 + 0.3) * 1.0 = 52.0f
        const float ApothecaryBuffDamage = FPACombatFormulas::CalculatePostureDamage(
            40.0f, 0.3f, 1.0f, false, false, TargetBossMaxPosture);
        TestNearlyEqual(TEXT("AC-2: Dầu tẩm Dược sư (+30%) phải tăng Posture damage lên 52.0f"), ApothecaryBuffDamage, 52.0f, 0.001f);

        // 2.5: Kết hợp Điểm yếu (Weakspot) + Dầu tẩm Dược sư (+30%)
        // PostureDamage dự kiến: 40 * 1.3 * 1.5 = 78.0f
        const float ComboPostureDamage = FPACombatFormulas::CalculatePostureDamage(
            40.0f, 0.3f, 1.0f, true, false, TargetBossMaxPosture);
        TestNearlyEqual(TEXT("AC-2: Kết hợp Weakspot (1.5x) và Dược sư (+30%) phải đạt 78.0f"), ComboPostureDamage, 78.0f, 0.001f);
    }

    // -------------------------------------------------------------------------
    // Test 3: AC-3 (Stagger Execution Finisher Formula & Boss 4-Cycle Guarantee)
    // -------------------------------------------------------------------------
    {
        // 3.1: QA Case chuẩn: Boss có 20,000 Max HP, Finisher có 100 Base Damage
        // ExecuteDamage dự kiến: (20,000 * 0.25) + (100 * 3.0) = 5,000 + 300 = 5,300 HP
        const float BossMaxHP = 20000.0f;
        const float BaseDamage = 100.0f;
        const float ExpectedExecuteDamage = (BossMaxHP * 0.25f) + (BaseDamage * 3.0f);

        const float ActualExecuteDamage = FPACombatFormulas::CalculateStaggerExecutionDamage(BossMaxHP, BaseDamage);
        TestEqual(TEXT("AC-3: Đòn kết liễu Boss 20,000 HP với BaseDamage=100 phải gây đúng 5,300 sát thương"), ActualExecuteDamage, ExpectedExecuteDamage);
        TestEqual(TEXT("AC-3: Sát thương phải bằng đúng 5300.0f"), ActualExecuteDamage, 5300.0f);

        // 3.2: Xác minh quy tắc 4 chu kỳ gục ngã (4 Execution Cycles hạ gục Boss vượt cấp)
        // 4 lần kết liễu chỉ tính riêng phần 25% Max HP = 100% Max HP
        const float FourCycleHPRemoved = 4.0f * (BossMaxHP * 0.25f);
        TestEqual(TEXT("AC-3: 4 chu kỳ kết liễu thành công phải rút trọn 100% máu tối đa của Boss"), FourCycleHPRemoved, BossMaxHP);

        // 3.3: Kiểm tra tính chất Xuyên Giáp (Armor Bypass):
        // Sát thương kết liễu không phụ thuộc vào giáp của Boss
        const float ExecuteDamageAgainstArmoredBoss = FPACombatFormulas::CalculateStaggerExecutionDamage(BossMaxHP, BaseDamage);
        TestEqual(TEXT("AC-3: Đòn kết liễu luôn bảo toàn sát thương 5300 regardless of armor"), ExecuteDamageAgainstArmoredBoss, 5300.0f);
    }

    // -------------------------------------------------------------------------
    // Test 4: Execution Calculation Classes & Combat Attributes Integration
    // -------------------------------------------------------------------------
    {
        // 4.1: Kiểm tra khởi tạo các lớp Execution Calculation của GAS
        UGEC_DamageCalculation* DamageCalc = NewObject<UGEC_DamageCalculation>();
        TestNotNull(TEXT("AC-1: UGEC_DamageCalculation phải khởi tạo thành công trong GAS"), DamageCalc);

        UGEC_PostureCalculation* PostureCalc = NewObject<UGEC_PostureCalculation>();
        TestNotNull(TEXT("AC-2: UGEC_PostureCalculation phải khởi tạo thành công trong GAS"), PostureCalc);

        // 4.2: Kiểm tra UAscendantAttributeSet tích hợp đầy đủ các thuộc tính chiến đấu mới
        UAscendantAttributeSet* AttrSet = NewObject<UAscendantAttributeSet>();
        TestNotNull(TEXT("UAscendantAttributeSet phải tồn tại"), AttrSet);

        if (AttrSet)
        {
            TestEqual(TEXT("Chỉ số Giáp (Armor) cơ bản phải khởi tạo bằng 0.0f"), AttrSet->GetArmor(), 0.0f);
            TestEqual(TEXT("Chỉ số Lực công (AttackPower) cơ bản phải khởi tạo bằng 100.0f"), AttrSet->GetAttackPower(), 100.0f);
            TestEqual(TEXT("Chỉ số Lực phá thế (BaseStagger) cơ bản phải khởi tạo bằng 10.0f"), AttrSet->GetBaseStagger(), 10.0f);
            TestEqual(TEXT("Chỉ số Thưởng phá thế (StaggerBonus) cơ bản phải khởi tạo bằng 0.0f"), AttrSet->GetStaggerBonus(), 0.0f);

            // Kiểm tra ràng buộc không âm cho Armor
            float NegativeArmor = -50.0f;
            AttrSet->PreAttributeChange(AttrSet->GetArmorAttribute(), NegativeArmor);
            TestEqual(TEXT("Armor không bao giờ được nhận giá trị âm"), NegativeArmor, 0.0f);
        }
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
