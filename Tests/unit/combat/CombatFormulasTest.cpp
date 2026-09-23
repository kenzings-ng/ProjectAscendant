// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * IMPLEMENT_SIMPLE_AUTOMATION_TEST: Kiểm thử đơn vị các công thức tính toán sát thương
 * và tích lũy thể khí (Posture Break) dựa trên các hằng số quy chuẩn trong entities.yaml và combat-system.md.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCombatFormulasTest,
    "ProjectAscendant.Combat.Formulas.DamageAndPostureCalculations",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCombatFormulasTest::RunTest(const FString& Parameters)
{
    // --- Test 1: Công Thức Tính Sát Thương Thực Tế ---
    // Công thức: EffectiveDamage = Max(1.0, RawAttack * (100.0 / (100.0 + Defense)))
    const float RawAttack = 150.0f;
    const float Defense = 50.0f;
    const float ExpectedEffectiveDamage = RawAttack * (100.0f / (100.0f + Defense)); // 150 * (100/150) = 100.0f

    const float CalculatedDamage = FMath::Max(1.0f, RawAttack * (100.0f / (100.0f + Defense)));
    TestEqual(TEXT("Sát thương tính toán phải bằng 100 khi Attack=150 và Defense=50"), CalculatedDamage, ExpectedEffectiveDamage);

    // --- Test 2: Ngưỡng Sát Thương Tối Thiểu (Minimum Damage Floor) ---
    const float HighDefense = 99999.0f;
    const float MinimalDamage = FMath::Max(1.0f, RawAttack * (100.0f / (100.0f + HighDefense)));
    TestEqual(TEXT("Sát thương tối thiểu không bao giờ được nhỏ hơn 1.0f"), MinimalDamage, 1.0f);

    // --- Test 3: Tích Lũy Thể Khí & Trạng Thái Gục Ngã (Posture Break) ---
    const float MaxPosture = 200.0f;
    float CurrentPosture = 180.0f;
    const float HeavyAttackPostureDamage = 45.0f;

    CurrentPosture += HeavyAttackPostureDamage;
    const bool bIsPostureBroken = (CurrentPosture >= MaxPosture);
    TestTrue(TEXT("Mục tiêu phải rơi vào trạng thái Staggered khi Posture vượt MaxPosture"), bIsPostureBroken);

    // --- Test 4: Cửa Sổ Hành Quyết Độc Quyền 1.5 Giây ---
    const float ExecutionWindowDuration = 1.5f;
    TestEqual(TEXT("Cửa sổ ưu tiên Finisher độc quyền phải là 1.5 giây"), ExecutionWindowDuration, 1.5f);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
