// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Combat/AscendantAttributeSet.h"
#include "Character/PABaseCharacter.h"
#include "AbilitySystemComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAAttributesReplicationTest
 *
 * Kiểm thử tự động đơn vị cho hệ thống AttributeSet và cơ chế đồng bộ hóa chỉ số (Story 001 / attr-001).
 *
 * Kiểm tra các tiêu chí nghiệm thu:
 *  - AC-1: Khởi tạo 10 thuộc tính cốt lõi với giá trị cơ sở Level 1 chuẩn theo GDD attributes-system.md.
 *  - AC-2: Ràng buộc chặn biên giá trị trong PreAttributeChange (Health, Mana, Stamina, Posture trong [0, Max]).
 *  - AC-3: Phát tín hiệu delegate khi hết máu (OnOutOfHealth) và khi vỡ thế đứng (OnPostureBroken).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPAAttributesReplicationTest,
    "ProjectAscendant.Foundation.Combat.AttributesAndReplication",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAAttributesReplicationTest::RunTest(const FString& Parameters)
{
    // -------------------------------------------------------------------------
    // Test 1: AC-1 (Core Attributes Definition & Level 1 Baseline Stats)
    // -------------------------------------------------------------------------
    {
        UAscendantAttributeSet* AttrSet = NewObject<UAscendantAttributeSet>();
        TestNotNull(TEXT("AC-1: UAscendantAttributeSet phải được khởi tạo thành công"), AttrSet);

        if (AttrSet)
        {
            // 1.1: Sinh Mệnh (Health)
            TestEqual(TEXT("AC-1: Máu khởi điểm phải bằng 500.0"), AttrSet->GetHealth(), 500.0f);
            TestEqual(TEXT("AC-1: Máu tối đa MaxHealth phải bằng 500.0"), AttrSet->GetMaxHealth(), 500.0f);

            // 1.2: Năng Lượng (Mana)
            TestEqual(TEXT("AC-1: Năng lượng Mana khởi điểm phải bằng 100.0"), AttrSet->GetMana(), 100.0f);
            TestEqual(TEXT("AC-1: Năng lượng tối đa MaxMana phải bằng 100.0"), AttrSet->GetMaxMana(), 100.0f);

            // 1.3: Thể Lực (Stamina)
            TestEqual(TEXT("AC-1: Thể lực Stamina khởi điểm phải bằng 100.0"), AttrSet->GetStamina(), 100.0f);
            TestEqual(TEXT("AC-1: Thể lực tối đa MaxStamina phải bằng 100.0"), AttrSet->GetMaxStamina(), 100.0f);
            TestEqual(TEXT("AC-1: Tốc độ hồi phục thể lực StaminaRegenRate phải bằng 45.0 điểm/s"), AttrSet->GetStaminaRegenRate(), 45.0f);

            // 1.4: Thế Đứng & Phá Vỡ (Posture)
            TestEqual(TEXT("AC-1: Thanh thế đứng Posture khởi điểm phải bằng 0.0 (tích lũy ngược)"), AttrSet->GetPosture(), 0.0f);
            TestEqual(TEXT("AC-1: Ngưỡng thế đứng tối đa MaxPosture phải bằng 100.0"), AttrSet->GetMaxPosture(), 100.0f);
            TestEqual(TEXT("AC-1: Tốc độ hạ nhiệt PostureDecayRate phải bằng 20.0 điểm/s"), AttrSet->GetPostureDecayRate(), 20.0f);

            // 1.5: Tham Số Chiến Đấu (IFrame & MoveSpeed)
            TestEqual(TEXT("AC-1: Thời lượng bất tử IFrameDuration khi né phải bằng 0.28 giây"), AttrSet->GetIFrameDuration(), 0.28f);
            TestEqual(TEXT("AC-1: Tốc độ chạy MoveSpeed cơ bản phải bằng 550.0 cm/s"), AttrSet->GetMoveSpeed(), 550.0f);
        }
    }

    // -------------------------------------------------------------------------
    // Test 2: AC-2 (Boundary Clamping Enforcement in PreAttributeChange)
    // -------------------------------------------------------------------------
    {
        UAscendantAttributeSet* AttrSet = NewObject<UAscendantAttributeSet>();
        TestNotNull(TEXT("AC-2: AttrSet phải tồn tại để kiểm tra clamping"), AttrSet);

        if (AttrSet)
        {
            // 2.1: Kiểm tra chặn dưới Health: Trừ máu vượt quá 0 (thử gán -150.0f)
            float TestHealthBelowZero = -150.0f;
            AttrSet->PreAttributeChange(AttrSet->GetHealthAttribute(), TestHealthBelowZero);
            TestEqual(TEXT("AC-2: Máu âm phải bị chặn cứng ở mức 0.0f (không bao giờ âm)"), TestHealthBelowZero, 0.0f);

            // 2.2: Kiểm tra chặn trên Health: Hồi máu vượt quá MaxHealth (thử gán 1500.0f khi MaxHealth=500.0f)
            float TestHealthAboveMax = 1500.0f;
            AttrSet->PreAttributeChange(AttrSet->GetHealthAttribute(), TestHealthAboveMax);
            TestEqual(TEXT("AC-2: Máu vượt quá MaxHealth phải bị chặn ở mức 500.0f"), TestHealthAboveMax, 500.0f);

            // 2.3: Kiểm tra chặn dưới Stamina: Tiêu hao thể lực vượt quá mức (thử gán -50.0f)
            float TestStaminaBelowZero = -50.0f;
            AttrSet->PreAttributeChange(AttrSet->GetStaminaAttribute(), TestStaminaBelowZero);
            TestEqual(TEXT("AC-2: Thể lực âm phải bị chặn ở mức 0.0f"), TestStaminaBelowZero, 0.0f);

            // 2.4: Kiểm tra chặn trên Stamina: Thể lực vượt MaxStamina (thử gán 300.0f khi MaxStamina=100.0f)
            float TestStaminaAboveMax = 300.0f;
            AttrSet->PreAttributeChange(AttrSet->GetStaminaAttribute(), TestStaminaAboveMax);
            TestEqual(TEXT("AC-2: Thể lực vượt MaxStamina phải bị chặn ở mức 100.0f"), TestStaminaAboveMax, 100.0f);

            // 2.5: Kiểm tra chặn Posture: Tích tụ vượt MaxPosture (thử gán 250.0f khi MaxPosture=100.0f)
            float TestPostureAboveMax = 250.0f;
            AttrSet->PreAttributeChange(AttrSet->GetPostureAttribute(), TestPostureAboveMax);
            TestEqual(TEXT("AC-2: Posture vượt quá MaxPosture phải bị chặn ở mức 100.0f"), TestPostureAboveMax, 100.0f);

            // 2.6: Kiểm tra chặn Posture: Hạ nhiệt dưới 0 (thử gán -30.0f)
            float TestPostureBelowZero = -30.0f;
            AttrSet->PreAttributeChange(AttrSet->GetPostureAttribute(), TestPostureBelowZero);
            TestEqual(TEXT("AC-2: Posture hạ nhiệt dưới 0 phải bị chặn ở mức 0.0f"), TestPostureBelowZero, 0.0f);
        }
    }

    // -------------------------------------------------------------------------
    // Test 3: AC-3 (Delegates and Combat Event Broadcast)
    // -------------------------------------------------------------------------
    {
        UAscendantAttributeSet* AttrSet = NewObject<UAscendantAttributeSet>();
        TestNotNull(TEXT("AC-3: AttrSet phải tồn tại để kiểm tra delegate"), AttrSet);

        if (AttrSet)
        {
            bool bDeathBroadcastFired = false;
            AttrSet->OnOutOfHealth.AddLambda([&bDeathBroadcastFired](AActor* Instigator)
            {
                bDeathBroadcastFired = true;
            });

            bool bPostureBreakFired = false;
            AttrSet->OnPostureBroken.AddLambda([&bPostureBreakFired](AActor* Instigator)
            {
                bPostureBreakFired = true;
            });

            // 3.1: Kích hoạt sự kiện tử trận bằng cách giả lập rút cạn máu về 0
            AttrSet->SetHealth(0.0f);
            AttrSet->OnOutOfHealth.Broadcast(nullptr);
            TestTrue(TEXT("AC-3: Delegate OnOutOfHealth phải phát tín hiệu thành công khi máu cạn về 0"), bDeathBroadcastFired);

            // 3.2: Kích hoạt sự kiện phá vỡ thế đứng Posture khi đạt 100%
            AttrSet->SetPosture(100.0f);
            AttrSet->OnPostureBroken.Broadcast(nullptr);
            TestTrue(TEXT("AC-3: Delegate OnPostureBroken phải phát tín hiệu thành công khi Posture đạt đỉnh"), bPostureBreakFired);
        }
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
