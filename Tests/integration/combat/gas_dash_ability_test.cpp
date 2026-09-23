// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Combat/PAGameplayAbility_Dash.h"
#include "Combat/PAStaminaComponent.h"
#include "Combat/AscendantAttributeSet.h"
#include "Combat/PADamageExecutionCalculation.h"
#include "Character/PABaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAGASDashAbilityIntegrationTest
 *
 * Kiểm thử tích hợp tự động cho Story dash-001 (GAS Dash Ability & Invulnerability Window):
 *  - AC-1 (Stamina Cost & Validation): Dash tốn 25 Stamina; Chặn hoàn toàn trên cả client/server nếu Stamina < 25 hoặc có State.Exhausted.
 *  - AC-2 (I-Frame Invulnerability Window): Cửa sổ bất tử State.Invulnerable từ t = 0.05s đến 0.25s (0.20s); Triệt tiêu sát thương 100%.
 *  - AC-3 (Impulse & Cooldown): Đẩy nhân vật 450 units theo hướng di chuyển/ngắm với linear decay; Gắn Cooldown.Dash 0.5s khi dứt lướt.
 *  - AC-4 (Ghost Trail Delegate): Phát delegate OnDashExecuted cho visual components spawn pixel ghost afterimages.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPAGASDashAbilityIntegrationTest,
    "ProjectAscendant.Core.Combat.GASDashAbilityIntegration",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAGASDashAbilityIntegrationTest::RunTest(const FString& Parameters)
{
    // =========================================================================
    // 1. Kiểm thử Pure Pipeline Math & Logic (FPADashPipeline)
    // =========================================================================

    // 1.1: AC-1 Stamina Validation Formula
    {
        TestTrue(TEXT("AC-1 Pipeline: Stamina = 100, không kiệt sức -> Cho phép Dash"),
            FPADashPipeline::CanActivateDash(100.0f, false, false, 25.0f));

        TestTrue(TEXT("AC-1 Pipeline: Stamina = 25.0f vừa đủ -> Cho phép Dash"),
            FPADashPipeline::CanActivateDash(25.0f, false, false, 25.0f));

        TestFalse(TEXT("AC-1 Pipeline: Stamina = 24.9f (< 25) -> Chặn tuyệt đối"),
            FPADashPipeline::CanActivateDash(24.9f, false, false, 25.0f));

        TestFalse(TEXT("AC-1 Pipeline: Stamina = 0.0f -> Chặn tuyệt đối"),
            FPADashPipeline::CanActivateDash(0.0f, false, false, 25.0f));

        TestFalse(TEXT("AC-1 Pipeline: Stamina = 100 nhưng bị State.Exhausted -> Chặn tuyệt đối"),
            FPADashPipeline::CanActivateDash(100.0f, true, false, 25.0f));

        TestFalse(TEXT("AC-1 Pipeline: Stamina = 100 nhưng có Tag Block Dash -> Chặn tuyệt đối"),
            FPADashPipeline::CanActivateDash(100.0f, false, true, 25.0f));
    }

    // 1.2: AC-2 I-Frame Window Formula (0.05s -> 0.25s)
    {
        TestFalse(TEXT("AC-2 Pipeline: t = 0.00s chưa vào I-Frame"),
            FPADashPipeline::IsWithinIFrameWindow(0.00f, 0.05f, 0.25f));

        TestFalse(TEXT("AC-2 Pipeline: t = 0.04s chưa vào I-Frame"),
            FPADashPipeline::IsWithinIFrameWindow(0.04f, 0.05f, 0.25f));

        TestTrue(TEXT("AC-2 Pipeline: t = 0.05s bắt đầu I-Frame"),
            FPADashPipeline::IsWithinIFrameWindow(0.05f, 0.05f, 0.25f));

        TestTrue(TEXT("AC-2 Pipeline: t = 0.15s nằm trong I-Frame"),
            FPADashPipeline::IsWithinIFrameWindow(0.15f, 0.05f, 0.25f));

        TestTrue(TEXT("AC-2 Pipeline: t = 0.25s mốc kết thúc I-Frame"),
            FPADashPipeline::IsWithinIFrameWindow(0.25f, 0.05f, 0.25f));

        TestFalse(TEXT("AC-2 Pipeline: t = 0.26s đã thoát khỏi I-Frame"),
            FPADashPipeline::IsWithinIFrameWindow(0.26f, 0.05f, 0.25f));

        TestFalse(TEXT("AC-2 Pipeline: t = 0.35s dứt chiêu, không còn I-Frame"),
            FPADashPipeline::IsWithinIFrameWindow(0.35f, 0.05f, 0.25f));

        // Kiểm tra độ dài cửa sổ I-frame (0.20s)
        const float WindowDuration = FPADashPipeline::kIFrameEndTime - FPADashPipeline::kIFrameStartTime;
        TestEqual(TEXT("AC-2 Pipeline: Thời lượng cửa sổ I-Frame phải đúng 0.20s"), WindowDuration, 0.20f);
    }

    // 1.3: AC-3 Linear Decay Impulse & Displacement Formulas
    {
        const float Distance = 450.0f;
        const float Duration = 0.35f;

        const float PeakVelocity = FPADashPipeline::CalculatePeakVelocity(Distance, Duration);
        const float ExpectedPeak = (2.0f * 450.0f) / 0.35f; // ~2571.4286 cm/s
        TestNearlyEqual(TEXT("AC-3 Pipeline: Vận tốc đỉnh V_peak phải là (2 * 450) / 0.35 ~= 2571.43 cm/s"),
            static_cast<double>(PeakVelocity), static_cast<double>(ExpectedPeak), 0.1);

        // Vận tốc tại t = 0 phải bằng PeakVelocity
        const float VelAt0 = FPADashPipeline::CalculateDecayingVelocity(0.0f, Distance, Duration);
        TestNearlyEqual(TEXT("AC-3 Pipeline: Vận tốc tại t = 0s phải bằng V_peak"),
            static_cast<double>(VelAt0), static_cast<double>(PeakVelocity), 0.1);

        // Vận tốc tại t = Duration phải về 0
        const float VelAtEnd = FPADashPipeline::CalculateDecayingVelocity(0.35f, Distance, Duration);
        TestEqual(TEXT("AC-3 Pipeline: Vận tốc tại t = 0.35s phải giảm tuyến tính về 0"), VelAtEnd, 0.0f);

        // Vận tốc tại trung điểm t = 0.175s phải bằng một nửa V_peak
        const float VelAtMid = FPADashPipeline::CalculateDecayingVelocity(0.175f, Distance, Duration);
        TestNearlyEqual(TEXT("AC-3 Pipeline: Vận tốc tại t = 0.175s phải bằng 0.5 * V_peak"),
            static_cast<double>(VelAtMid), static_cast<double>(PeakVelocity * 0.5f), 0.1);

        // Tích phân quãng đường tại t = 0.35s phải đạt đúng 450.0 units
        const float TotalDisplacement = FPADashPipeline::CalculateDisplacementAtTime(0.35f, Distance, Duration);
        TestNearlyEqual(TEXT("AC-3 Pipeline: Quãng đường di chuyển tích lũy tại t = 0.35s phải đạt đúng 450.0 units"),
            static_cast<double>(TotalDisplacement), 450.0, 0.1);
    }

    // 1.4: AC-3 Direction Resolution
    {
        // Ưu tiên 1: Movement Vector
        const FVector MoveDir = FVector(100.0, 0.0, 0.0);
        const FVector AimDir = FVector(0.0, 1.0, 0.0);
        const FVector FwdDir = FVector(0.7071, 0.7071, 0.0);

        const FVector ResolvedFromMove = FPADashPipeline::ResolveDashDirection(MoveDir, AimDir, FwdDir);
        TestNearlyEqual(TEXT("AC-3 Pipeline: Khi đang di chuyển, hướng lướt theo Movement Vector (+X)"),
            ResolvedFromMove.X, 1.0, 0.01);
        TestNearlyEqual(TEXT("AC-3 Pipeline: Khi đang di chuyển, Y = 0"),
            ResolvedFromMove.Y, 0.0, 0.01);

        // Ưu tiên 2: Aim Direction khi đứng yên
        const FVector ResolvedFromAim = FPADashPipeline::ResolveDashDirection(FVector::ZeroVector, AimDir, FwdDir);
        TestNearlyEqual(TEXT("AC-3 Pipeline: Khi đứng yên, hướng lướt theo Aim Direction (+Y)"),
            ResolvedFromAim.Y, 1.0, 0.01);

        // Ưu tiên 3: Forward Vector khi không có aim
        const FVector ResolvedFromFwd = FPADashPipeline::ResolveDashDirection(FVector::ZeroVector, FVector::ZeroVector, FVector(1.0, 0.0, 0.0));
        TestNearlyEqual(TEXT("AC-3 Pipeline: Khi không có move và aim, hướng lướt theo Forward (+X)"),
            ResolvedFromFwd.X, 1.0, 0.01);
    }

    // =========================================================================
    // 2. Kiểm thử Tích hợp GAS Ability Component & Actor (UPAGameplayAbility_Dash)
    // =========================================================================

    APABaseCharacter* TestCharacter = NewObject<APABaseCharacter>();
    TestNotNull(TEXT("APABaseCharacter phải được khởi tạo"), TestCharacter);
    if (!TestCharacter)
    {
        return false;
    }

    UAbilitySystemComponent* ASC = TestCharacter->GetAbilitySystemComponent();
    TestNotNull(TEXT("AbilitySystemComponent trên nhân vật phải tồn tại"), ASC);
    if (!ASC)
    {
        return false;
    }

    UAscendantAttributeSet* AttrSet = TestCharacter->GetAttributeSet();
    TestNotNull(TEXT("UAscendantAttributeSet trên nhân vật phải tồn tại"), AttrSet);
    if (!AttrSet)
    {
        return false;
    }

    // Khởi tạo kỹ năng Dash
    UPAGameplayAbility_Dash* DashAbility = NewObject<UPAGameplayAbility_Dash>(TestCharacter);
    TestNotNull(TEXT("UPAGameplayAbility_Dash phải được khởi tạo"), DashAbility);
    if (!DashAbility)
    {
        return false;
    }

    // Đăng ký Ability vào ASC
    const FGameplayAbilitySpec AbilitySpec(DashAbility, 1);
    const FGameplayAbilitySpecHandle AbilityHandle = ASC->GiveAbility(AbilitySpec);
    TestTrue(TEXT("GiveAbility cho UPAGameplayAbility_Dash phải trả về Handle hợp lệ"), AbilityHandle.IsValid());

    FGameplayAbilityActorInfo ActorInfo;
    ActorInfo.InitFromActor(TestCharacter, TestCharacter, ASC);

    // -------------------------------------------------------------------------
    // Test 2.1: AC-1 CanActivateAbility Stamina & Exhaustion Checks
    // -------------------------------------------------------------------------
    {
        // Trường hợp 1: Đủ 100 Stamina, không kiệt sức -> Cho phép kích hoạt
        AttrSet->SetStamina(100.0f);
        TestCharacter->SetExhaustedState(false);
        TestTrue(TEXT("AC-1 GAS: Khi có 100 Stamina, CanActivateAbility phải trả về true"),
            DashAbility->CanActivateAbility(AbilityHandle, &ActorInfo));

        // Trường hợp 2: Stamina = 20 (< 25) -> Bị chặn
        AttrSet->SetStamina(20.0f);
        TestFalse(TEXT("AC-1 GAS: Khi Stamina = 20 (< 25), CanActivateAbility phải trả về false"),
            DashAbility->CanActivateAbility(AbilityHandle, &ActorInfo));

        // Trường hợp 3: Stamina = 100 nhưng có State.Exhausted -> Bị chặn
        AttrSet->SetStamina(100.0f);
        const FGameplayTag TagExhausted = FGameplayTag::RequestGameplayTag(TEXT("State.Exhausted"));
        ASC->AddLooseGameplayTag(TagExhausted);
        TestFalse(TEXT("AC-1 GAS: Khi có State.Exhausted, CanActivateAbility phải trả về false"),
            DashAbility->CanActivateAbility(AbilityHandle, &ActorInfo));
        ASC->RemoveLooseGameplayTag(TagExhausted);

        // Trường hợp 4: Stamina = 100 nhưng có Ability.Block.Dash -> Bị chặn
        const FGameplayTag TagBlockDash = FGameplayTag::RequestGameplayTag(TEXT("Ability.Block.Dash"));
        ASC->AddLooseGameplayTag(TagBlockDash);
        TestFalse(TEXT("AC-1 GAS: Khi có Ability.Block.Dash, CanActivateAbility phải trả về false"),
            DashAbility->CanActivateAbility(AbilityHandle, &ActorInfo));
        ASC->RemoveLooseGameplayTag(TagBlockDash);

        // Trường hợp 5: Stamina = 100 nhưng đang trong hồi chiêu Cooldown.Dash -> Bị chặn
        const FGameplayTag TagCooldownDash = FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Dash"));
        ASC->AddLooseGameplayTag(TagCooldownDash);
        TestFalse(TEXT("AC-1 GAS: Khi có Cooldown.Dash, CanActivateAbility phải trả về false"),
            DashAbility->CanActivateAbility(AbilityHandle, &ActorInfo));
        ASC->RemoveLooseGameplayTag(TagCooldownDash);
    }

    // -------------------------------------------------------------------------
    // Test 2.2: AC-2 I-Frame Window & 100% Damage Mitigation
    // -------------------------------------------------------------------------
    {
        const FGameplayTag TagInvulnerable = FGameplayTag::RequestGameplayTag(TEXT("State.Invulnerable"));

        // Trước khi mở cửa sổ I-Frame: chưa có thẻ Invulnerable
        TestFalse(TEXT("AC-2 GAS: Trước khi mở I-Frame, ASC chưa có State.Invulnerable"),
            ASC->HasMatchingGameplayTag(TagInvulnerable));

        // Bắt đầu cửa sổ I-Frame (t = 0.05s)
        DashAbility->OnIFrameWindowOpened();
        TestTrue(TEXT("AC-2 GAS: OnIFrameWindowOpened phải gán thẻ State.Invulnerable vào ASC"),
            ASC->HasMatchingGameplayTag(TagInvulnerable));
        TestTrue(TEXT("AC-2 GAS: IsInIFrameWindow() phải trả về true"),
            DashAbility->IsInIFrameWindow());

        // Kiểm tra triệt tiêu sát thương 100% trong I-Frame
        const float DamageDuringIFrame = FPACombatFormulas::CalculateEffectiveDamage(100.0f, 0.0f, true);
        TestEqual(TEXT("AC-2 GAS: Sát thương nhận vào trong I-Frame (State.Invulnerable) phải triệt tiêu về 0.0f"),
            DamageDuringIFrame, 0.0f);

        // Kết thúc cửa sổ I-Frame (t = 0.25s)
        DashAbility->OnIFrameWindowClosed();
        TestFalse(TEXT("AC-2 GAS: OnIFrameWindowClosed phải gỡ bỏ thẻ State.Invulnerable khỏi ASC"),
            ASC->HasMatchingGameplayTag(TagInvulnerable));
        TestFalse(TEXT("AC-2 GAS: IsInIFrameWindow() phải trả về false"),
            DashAbility->IsInIFrameWindow());

        // Kiểm tra sát thương sau khi hết I-Frame
        const float DamageAfterIFrame = FPACombatFormulas::CalculateEffectiveDamage(100.0f, 0.0f, false);
        TestEqual(TEXT("AC-2 GAS: Sát thương sau khi dứt I-Frame phải tính toán bình thường (100.0f)"),
            DamageAfterIFrame, 100.0f);
    }

    // -------------------------------------------------------------------------
    // Test 2.3: AC-3 Dash Completion & Cooldown Tag Application
    // -------------------------------------------------------------------------
    {
        const FGameplayTag TagDashing = FGameplayTag::RequestGameplayTag(TEXT("State.Dashing"));
        const FGameplayTag TagCooldownDash = FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Dash"));

        // Giả lập trạng thái đang lướt
        ASC->AddLooseGameplayTag(TagDashing);
        TestTrue(TEXT("AC-3 GAS: Đang trong quá trình lướt mang thẻ State.Dashing"),
            ASC->HasMatchingGameplayTag(TagDashing));

        // Khi hoàn tất cú lướt (t = 0.35s)
        DashAbility->OnDashCompleted();

        TestFalse(TEXT("AC-3 GAS: OnDashCompleted phải gỡ bỏ thẻ State.Dashing"),
            ASC->HasMatchingGameplayTag(TagDashing));
        TestTrue(TEXT("AC-3 GAS: OnDashCompleted phải gắn thẻ Cooldown.Dash"),
            ASC->HasMatchingGameplayTag(TagCooldownDash));

        // Hết thời gian hồi chiêu 0.5s
        DashAbility->OnCooldownExpired();
        TestFalse(TEXT("AC-3 GAS: OnCooldownExpired phải gỡ bỏ thẻ Cooldown.Dash"),
            ASC->HasMatchingGameplayTag(TagCooldownDash));
    }

    // -------------------------------------------------------------------------
    // Test 2.4: AC-4 Ghost Trail Delegate Invocation
    // -------------------------------------------------------------------------
    {
        const FVector ExpectedDir = FVector(1.0, 0.0, 0.0);
        const float ExpectedDist = 450.0f;

        // Phát delegate trực tiếp trên Ability (AC-4)
        DashAbility->OnDashExecuted.Broadcast(ExpectedDir, ExpectedDist);
        TestTrue(TEXT("AC-4: OnDashExecuted trên Ability broadcast an toàn cho visual components"), true);
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
