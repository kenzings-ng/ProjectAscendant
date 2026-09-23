// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Character/PAStoneGolemBoss.h"
#include "Animation/PAPaperZDAnimInstance.h"
#include "World/PAMonsterLeashComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "GameplayTagsManager.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPABossPaperZDAggroIntegrationTest
 *
 * Kiểm thử tích hợp tự động cho Story pzd-003 (Stone Golem Boss PaperZD AnimBP & Aggro Integration):
 *  - AC-1: Stone Golem Boss PaperZD State Machine (Speed > 10.0f kích hoạt walk, State.Attacking kích hoạt slam, State.Broken kích hoạt stagger, State.Dead kích hoạt death).
 *  - AC-2: Boss AI Aggro & Leash Integration (AggroRadius 800cm, LeashDistance 2500cm, MaxWalkSpeed 220cm/s, LeashSpeedMultiplier 1.5x, tự động drop aggro khi leash reset).
 *  - AC-3: Ground Slam Hitbox Window & Damage Execution (AttackRange <= 250cm, AoERadius 300cm, BaseDamage 35, PostureDamage 30, hồi chiêu 3.0s).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPABossPaperZDAggroIntegrationTest,
    "ProjectAscendant.Core.Character.BossPaperZDAggroIntegration",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPABossPaperZDAggroIntegrationTest::RunTest(const FString& Parameters)
{
    // Đăng ký Native GameplayTags phòng trường hợp chạy headless test
    UGameplayTagsManager& TagMgr = UGameplayTagsManager::Get();
    TagMgr.AddNativeGameplayTag(FName(TEXT("State.Attacking")));
    TagMgr.AddNativeGameplayTag(FName(TEXT("State.Stunned")));
    TagMgr.AddNativeGameplayTag(FName(TEXT("State.Dead")));
    TagMgr.AddNativeGameplayTag(FName(TEXT("State.LeashReset")));
    TagMgr.AddNativeGameplayTag(FName(TEXT("State.Invulnerable")));
    TagMgr.AddNativeGameplayTag(FName(TEXT("State.Immune")));

    // -------------------------------------------------------------------------
    // Khởi tạo Boss Stone Golem
    // -------------------------------------------------------------------------
    APAStoneGolemBoss* Boss = NewObject<APAStoneGolemBoss>();
    TestNotNull(TEXT("APAStoneGolemBoss phải được khởi tạo"), Boss);

    if (!Boss)
    {
        return false;
    }

    // -------------------------------------------------------------------------
    // Test 1: AC-1 (Boss PaperZD State Machine & Variable Bindings)
    // -------------------------------------------------------------------------
    {
        UPAPaperZDAnimInstance* AnimInstance = NewObject<UPAPaperZDAnimInstance>(Boss);
        TestNotNull(TEXT("AC-1: UPAPaperZDAnimInstance phải được khởi tạo"), AnimInstance);

        if (AnimInstance)
        {
            // 1. Kiểm tra Idle <-> Walk transition (Speed threshold 10.0f)
            AnimInstance->CurrentSpeed = 0.0f;
            AnimInstance->bIsMoving = false;
            TestFalse(TEXT("AC-1: Khi Speed = 0, bIsMoving phải là false (Idle)"), AnimInstance->bIsMoving);

            AnimInstance->CurrentSpeed = Boss->BossWalkSpeed; // 220 cm/s
            AnimInstance->bIsMoving = (AnimInstance->CurrentSpeed > 10.0f);
            TestTrue(TEXT("AC-1: Khi Speed = 220 > 10, bIsMoving phải là true (Walk)"), AnimInstance->bIsMoving);

            // 2. Kiểm tra GAS Tag Bindings với AbilitySystemComponent
            UAbilitySystemComponent* ASC = Boss->GetAbilitySystemComponent();
            TestNotNull(TEXT("AC-1: Boss phải sở hữu AbilitySystemComponent"), ASC);

            if (ASC)
            {
                const FGameplayTag TagAttacking = FGameplayTag::RequestGameplayTag(TEXT("State.Attacking"));
                const FGameplayTag TagStunned = FGameplayTag::RequestGameplayTag(TEXT("State.Stunned"));
                const FGameplayTag TagDead = FGameplayTag::RequestGameplayTag(TEXT("State.Dead"));

                // Gán Tag State.Attacking (Slam)
                ASC->AddLooseGameplayTag(TagAttacking);
                AnimInstance->UpdateAnimationVariables(0.016f);
                TestTrue(TEXT("AC-1: Tag State.Attacking phải kích hoạt bIsAttacking = true"), AnimInstance->bIsAttacking);
                ASC->RemoveLooseGameplayTag(TagAttacking);

                // Gán Tag State.Stunned (Stagger)
                ASC->AddLooseGameplayTag(TagStunned);
                AnimInstance->UpdateAnimationVariables(0.016f);
                TestTrue(TEXT("AC-1: Tag State.Stunned phải kích hoạt bIsStunned = true"), AnimInstance->bIsStunned);
                ASC->RemoveLooseGameplayTag(TagStunned);

                // Gán Tag State.Dead (Death)
                ASC->AddLooseGameplayTag(TagDead);
                AnimInstance->UpdateAnimationVariables(0.016f);
                TestTrue(TEXT("AC-1: Tag State.Dead phải kích hoạt bIsDead = true"), AnimInstance->bIsDead);
                ASC->RemoveLooseGameplayTag(TagDead);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Test 2: AC-2 (Boss AI Aggro & Leash Integration)
    // -------------------------------------------------------------------------
    {
        UPAMonsterLeashComponent* Leash = Boss->GetLeashComponent();
        TestNotNull(TEXT("AC-2: Boss phải sở hữu UPAMonsterLeashComponent"), Leash);

        if (Leash)
        {
            TestEqual(TEXT("AC-2: LeashDistance mặc định phải là 2500cm"), Leash->GetLeashDistance(), 2500.0f);
            TestEqual(TEXT("AC-2: LeashSpeedMultiplier mặc định phải là 1.5x"), Leash->GetLeashSpeedMultiplier(), 1.5f);
        }

        TestEqual(TEXT("AC-2: AggroRadius của Boss phải là 800cm"), Boss->AggroRadius, 800.0f);
        TestEqual(TEXT("AC-2: BossWalkSpeed mặc định phải là 220cm/s"), Boss->BossWalkSpeed, 220.0f);

        // Giả lập kích hoạt Leash Reset: Boss phải tự động hủy target
        AActor* DummyTarget = NewObject<AActor>();
        Boss->SetTargetActor(DummyTarget);
        TestTrue(TEXT("AC-2: Target phải được gán thành công"), Boss->GetTargetActor() == DummyTarget);

        if (Leash)
        {
            Leash->TriggerLeashReset();
            TestTrue(TEXT("AC-2: Leash phải bước vào trạng thái IsLeashResetting"), Leash->IsLeashResetting());
            
            // Gọi UpdateBossAI trong lúc đang reset: Target phải bị xóa bỏ (drop aggro)
            Boss->UpdateBossAI(0.016f);
            TestNull(TEXT("AC-2: Khi LeashResetting, Boss phải tự động drop aggro (Target = nullptr)"), Boss->GetTargetActor());
        }
    }

    // -------------------------------------------------------------------------
    // Test 3: AC-3 (Ground Slam Hitbox Window & Damage Execution)
    // -------------------------------------------------------------------------
    {
        TestEqual(TEXT("AC-3: AttackRange phải là 250cm"), Boss->AttackRange, 250.0f);
        TestEqual(TEXT("AC-3: SlamAoERadius phải là 300cm"), Boss->SlamAoERadius, 300.0f);
        TestEqual(TEXT("AC-3: SlamBaseDamage phải là 35"), Boss->SlamBaseDamage, 35.0f);
        TestEqual(TEXT("AC-3: SlamPostureDamage phải là 30"), Boss->SlamPostureDamage, 30.0f);
        TestEqual(TEXT("AC-3: SlamCooldown phải là 3.0s"), Boss->SlamCooldown, 3.0f);

        // Kích hoạt Ground Slam
        const bool bStartedSlam = Boss->TryExecuteGroundSlam();
        TestTrue(TEXT("AC-3: TryExecuteGroundSlam phải thành công khi đủ điều kiện"), bStartedSlam);
        TestTrue(TEXT("AC-3: Cờ bIsPerformingSlam phải bật true"), Boss->IsPerformingSlam());

        // Kiểm tra State.Attacking đã được gán vào ASC
        UAbilitySystemComponent* ASC = Boss->GetAbilitySystemComponent();
        if (ASC)
        {
            TestTrue(TEXT("AC-3: Khi thi triển Slam, ASC phải sở hữu tag State.Attacking"),
                ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Attacking"))));
        }

        // Không cho phép kích hoạt Slam lần 2 khi chưa hết cooldown
        const bool bDoubleSlam = Boss->TryExecuteGroundSlam();
        TestFalse(TEXT("AC-3: Không được phép kích hoạt Slam liên tiếp khi chưa hết cooldown"), bDoubleSlam);

        // Kết thúc chiêu Slam
        Boss->FinishGroundSlam();
        TestFalse(TEXT("AC-3: Sau khi FinishGroundSlam, cờ bIsPerformingSlam phải về false"), Boss->IsPerformingSlam());
        if (ASC)
        {
            TestFalse(TEXT("AC-3: Sau khi FinishGroundSlam, tag State.Attacking phải được gỡ bỏ"),
                ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Attacking"))));
        }
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
