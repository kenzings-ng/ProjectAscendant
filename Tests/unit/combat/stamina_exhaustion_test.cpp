// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Combat/PAStaminaComponent.h"
#include "Combat/AscendantAttributeSet.h"
#include "Character/PABaseCharacter.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAStaminaExhaustionTest
 *
 * Kiểm thử tự động đơn vị cho chu trình Thể lực (Stamina), Cú lướt tuyệt vọng (Desperation Roll),
 * Độ trễ hồi phục (Regen Delay) và Trạng thái Kiệt Sức (Exhaustion State Pipeline) - Story 003 / attr-003.
 *
 * Tiêu chí nghiệm thu:
 *  - AC-1: Tiêu hao 25 Stamina cho Dash. Kích hoạt Desperation Roll khi Stamina > 0 nhưng < 25 (xả về 0 và phạt 2.2s).
 *  - AC-2: Trễ 0.6s sau khi tiêu hao mới bắt đầu hồi thể lực; Tốc độ hồi 45 điểm/s tới MaxStamina.
 *  - AC-3: Kiệt Sức State.Exhausted giảm 25% tốc chạy (550 -> 412.5 cm/s), khóa lướt Ability.Block.Dash,
 *          thời lượng 1.5s (hoặc 2.2s với Desperation Roll), chỉ giải trừ khi hết thời gian VÀ Stamina >= 30%.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPAStaminaExhaustionTest,
    "ProjectAscendant.Foundation.Combat.StaminaExhaustionPipeline",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAStaminaExhaustionTest::RunTest(const FString& Parameters)
{
    // -------------------------------------------------------------------------
    // Test 1: AC-1 (Stamina Consumption & Desperation Roll Logic)
    // -------------------------------------------------------------------------
    {
        // 1.1: Dash tiêu chuẩn với đầy bình thể lực (100 Stamina)
        const float StartingStamina100 = 100.0f;
        const float DashCost = 25.0f;
        float RemainingStamina = 0.0f;
        bool bIsDesperationRoll = false;

        const bool bCanDash1 = FPAStaminaPipeline::ConsumeStamina(StartingStamina100, DashCost, RemainingStamina, bIsDesperationRoll);
        TestTrue(TEXT("AC-1: Nhân vật có 100 Stamina phải thực hiện được cú Dash chuẩn"), bCanDash1);
        TestEqual(TEXT("AC-1: Thể lực còn lại sau khi Dash chuẩn phải là 75.0f (100 - 25)"), RemainingStamina, 75.0f);
        TestFalse(TEXT("AC-1: Dash chuẩn không phải là Desperation Roll"), bIsDesperationRoll);

        // 1.2: Cú Lướt Tuyệt Vọng (Desperation Roll): Stamina = 10 (0 < Stamina < 25)
        const float StartingStamina10 = 10.0f;
        const bool bCanDashDesperation = FPAStaminaPipeline::ConsumeStamina(StartingStamina10, DashCost, RemainingStamina, bIsDesperationRoll);
        TestTrue(TEXT("AC-1: Nhân vật có 10 Stamina vẫn được phép lướt thoát hiểm (Desperation Roll)"), bCanDashDesperation);
        TestEqual(TEXT("AC-1: Desperation Roll xả sạch bình thể lực về đúng 0.0f"), RemainingStamina, 0.0f);
        TestTrue(TEXT("AC-1: Cờ bIsDesperationRoll phải được bật true"), bIsDesperationRoll);

        // 1.3: Thể lực cạn kiệt (Stamina = 0)
        const float StartingStamina0 = 0.0f;
        const bool bCanDashZero = FPAStaminaPipeline::ConsumeStamina(StartingStamina0, DashCost, RemainingStamina, bIsDesperationRoll);
        TestFalse(TEXT("AC-1: Khi Stamina = 0, hành động Dash phải bị từ chối tuyệt đối"), bCanDashZero);
        TestEqual(TEXT("AC-1: Thể lực vẫn giữ nguyên ở mức 0.0f"), RemainingStamina, 0.0f);
        TestFalse(TEXT("AC-1: Không kích hoạt Desperation Roll khi thể lực đã ở mức 0"), bIsDesperationRoll);
    }

    // -------------------------------------------------------------------------
    // Test 2: AC-2 (Regeneration Delay & 45.0/s Recovery Curve)
    // -------------------------------------------------------------------------
    {
        const float StartStamina = 50.0f;
        const float MaxStamina = 100.0f;
        const float RegenRate = 45.0f;  // 45 điểm/s
        const float RegenDelay = 0.6f;  // 0.6s trễ

        // 2.1: Tại T = 0.5s (< 0.6s): Đang trong thời gian trễ, thể lực không được hồi
        const float StaminaAt05s = FPAStaminaPipeline::CalculateStaminaAfterTime(StartStamina, MaxStamina, RegenRate, 0.5f, RegenDelay);
        TestEqual(TEXT("AC-2: Tại T=0.5s (<0.6s delay), thể lực phải đứng yên ở mức 50.0f"), StaminaAt05s, 50.0f);

        // 2.2: Tại T = 0.6s (Đúng mốc trễ): Thể lực bắt đầu mở khóa nhưng chưa tăng
        const float StaminaAt06s = FPAStaminaPipeline::CalculateStaminaAfterTime(StartStamina, MaxStamina, RegenRate, 0.6f, RegenDelay);
        TestEqual(TEXT("AC-2: Tại T=0.6s, thể lực vẫn ở mức 50.0f"), StaminaAt06s, 50.0f);

        // 2.3: Tại T = 1.6s (1.0s hồi phục chủ động sau 0.6s delay):
        // Thể lực = 50 + (45 * 1.0) = 95.0f
        const float StaminaAt16s = FPAStaminaPipeline::CalculateStaminaAfterTime(StartStamina, MaxStamina, RegenRate, 1.6f, RegenDelay);
        TestEqual(TEXT("AC-2: Tại T=1.6s (1s sau delay), thể lực phải đạt đúng 95.0f (50 + 45*1)"), StaminaAt16s, 95.0f);

        // 2.4: Tại T = 3.0s: Thể lực hồi vượt trần MaxStamina -> kẹp cứng ở mức 100.0f
        const float StaminaAt30s = FPAStaminaPipeline::CalculateStaminaAfterTime(StartStamina, MaxStamina, RegenRate, 3.0f, RegenDelay);
        TestEqual(TEXT("AC-2: Tại T=3.0s, thể lực hồi đầy phải bị kẹp ở mức MaxStamina (100.0f)"), StaminaAt30s, 100.0f);
    }

    // -------------------------------------------------------------------------
    // Test 3: AC-3 (Exhaustion State, Penalties & Recovery Conditions)
    // -------------------------------------------------------------------------
    {
        const float BaseSpeed = 550.0f;
        const float MaxStamina = 100.0f;

        // 3.1: Kiểm tra giảm 25% tốc độ chạy khi Kiệt Sức (550 * 0.75 = 412.5 cm/s)
        const float ExhaustedSpeed = FPAStaminaPipeline::CalculateExhaustedMoveSpeed(BaseSpeed);
        TestEqual(TEXT("AC-3: Vận tốc khi kiệt sức phải giảm 25% còn 412.5 cm/s"), ExhaustedSpeed, 412.5f);

        // 3.2: Thời lượng phạt kiệt sức
        TestEqual(TEXT("AC-3: Thời lượng kiệt sức tiêu chuẩn phải là 1.5 giây"), FPAStaminaPipeline::GetExhaustionDuration(false), 1.5f);
        TestEqual(TEXT("AC-3: Thời lượng kiệt sức do Desperation Roll phải là 2.2 giây"), FPAStaminaPipeline::GetExhaustionDuration(true), 2.2f);

        // 3.3: Kiểm tra điều kiện giải trừ kiệt sức tiêu chuẩn (1.5s phạt):
        // Trường hợp A: Hết 1.5s nhưng Stamina chỉ mới hồi 20 điểm (< 30% MaxStamina) -> Chưa được thoát
        const bool bCanRecoverA = FPAStaminaPipeline::CanRecoverFromExhaustion(20.0f, MaxStamina, 1.6f, 1.5f);
        TestFalse(TEXT("AC-3: Chưa được thoát kiệt sức khi Stamina < 30% dù đã hết 1.5s phạt"), bCanRecoverA);

        // Trường hợp B: Chưa hết 1.5s (ví dụ mới 1.0s) dù Stamina đã nạp 50 điểm (> 30%) -> Chưa được thoát
        const bool bCanRecoverB = FPAStaminaPipeline::CanRecoverFromExhaustion(50.0f, MaxStamina, 1.0f, 1.5f);
        TestFalse(TEXT("AC-3: Chưa được thoát kiệt sức khi chưa hết 1.5s phạt dù Stamina > 30%"), bCanRecoverB);

        // Trường hợp C: Đã hết 1.5s VÀ Stamina đạt 30 điểm (>= 30%) -> Thoát kiệt sức thành công
        const bool bCanRecoverC = FPAStaminaPipeline::CanRecoverFromExhaustion(30.0f, MaxStamina, 1.5f, 1.5f);
        TestTrue(TEXT("AC-3: Đủ điều kiện thoát kiệt sức khi hết 1.5s VÀ Stamina >= 30%"), bCanRecoverC);

        // 3.4: Kiểm tra điều kiện giải trừ kiệt sức Desperation Roll (2.2s phạt):
        // Tại T = 1.8s (đã qua 1.5s thường nhưng chưa đủ 2.2s của Desperation Roll) -> Chưa được thoát
        const bool bCanRecoverDesperationEarly = FPAStaminaPipeline::CanRecoverFromExhaustion(50.0f, MaxStamina, 1.8f, 2.2f);
        TestFalse(TEXT("AC-3: Desperation Roll bắt buộc phải chờ đủ 2.2s mới được giải trừ kiệt sức"), bCanRecoverDesperationEarly);

        // Tại T = 2.2s và Stamina = 35 -> Thoát thành công
        const bool bCanRecoverDesperationDone = FPAStaminaPipeline::CanRecoverFromExhaustion(35.0f, MaxStamina, 2.2f, 2.2f);
        TestTrue(TEXT("AC-3: Desperation Roll thoát kiệt sức thành công sau 2.2s và Stamina >= 30%"), bCanRecoverDesperationDone);
    }

    // -------------------------------------------------------------------------
    // Test 4: Component Architecture & PABaseCharacter Integration
    // -------------------------------------------------------------------------
    {
        // 4.1: Khởi tạo component kiểm tra cờ không dùng Tick (Control Manifest)
        UPAStaminaComponent* StaminaComp = NewObject<UPAStaminaComponent>();
        TestNotNull(TEXT("UPAStaminaComponent phải được khởi tạo thành công"), StaminaComp);

        if (StaminaComp)
        {
            TestFalse(TEXT("UPAStaminaComponent không được phép kích hoạt Tick (Control Manifest)"), StaminaComp->PrimaryComponentTick.bCanEverTick);
            TestFalse(TEXT("UPAStaminaComponent mặc định không ở trạng thái kiệt sức"), StaminaComp->IsExhausted());
        }
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
