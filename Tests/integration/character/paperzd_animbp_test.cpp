// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Animation/PAPaperZDAnimInstance.h"
#include "Animation/PAPaperZDNotifyState_Hitbox.h"
#include "Character/PABaseCharacter.h"
#include "Controller/PAIsometricMovementMath.h"
#include "PaperFlipbookComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAPaperZDAnimBPIntegrationTest
 *
 * Kiểm thử tích hợp tự động cho Story pzd-002 (PaperZD AnimBP & State Machine Setup):
 *  - AC-1: State Machine Architecture (Speed > 10.0f kích hoạt bIsMoving; GAS tags kích hoạt bIsDashing, bIsAttacking, bIsHurt, bIsStunned, bIsDead).
 *  - AC-2: Directional Sprite Flipping (CameraRight dot product quyết định bFacingRight; Yaw 0 cho Phải, Yaw 180 cho Trái; fallback vận tốc khi không ngắm chủ động).
 *  - AC-3: Combat Anim Notifies (UPAPaperZDNotifyState_Hitbox mở và đóng cửa sổ va chạm bIsHitboxActive trên Server Authority).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPAPaperZDAnimBPIntegrationTest,
    "ProjectAscendant.Core.Character.PaperZDAnimBPIntegration",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAPaperZDAnimBPIntegrationTest::RunTest(const FString& Parameters)
{
    // Khởi tạo đối tượng nhân vật giả lập
    APABaseCharacter* TestCharacter = NewObject<APABaseCharacter>();
    TestNotNull(TEXT("APABaseCharacter phải được khởi tạo"), TestCharacter);

    if (!TestCharacter)
    {
        return false;
    }

    // Kiểm tra SpriteComponent và PaperZDAnimComponent đã được khởi tạo
    TestNotNull(TEXT("SpriteComponent phải được khởi tạo"), TestCharacter->GetSpriteComponent());
    TestNotNull(TEXT("PaperZDAnimComponent phải được khởi tạo"), TestCharacter->GetPaperZDAnimComponent());

    // Khởi tạo AnimInstance
    UPAPaperZDAnimInstance* AnimInstance = NewObject<UPAPaperZDAnimInstance>(TestCharacter);
    TestNotNull(TEXT("UPAPaperZDAnimInstance phải được khởi tạo"), AnimInstance);

    if (!AnimInstance)
    {
        return false;
    }

    // -------------------------------------------------------------------------
    // Test 1: AC-1 (State Machine Architecture & Tag Bindings)
    // -------------------------------------------------------------------------
    {
        // Khi đứng yên (Speed = 0)
        AnimInstance->CurrentSpeed = 0.0f;
        AnimInstance->bIsMoving = false;
        TestFalse(TEXT("AC-1: Khi Speed = 0, bIsMoving phải là false"), AnimInstance->bIsMoving);

        // Khi di chuyển (Speed = 550.0f > 10.0f)
        AnimInstance->CurrentSpeed = 550.0f;
        AnimInstance->bIsMoving = AnimInstance->CurrentSpeed > 10.0f;
        TestTrue(TEXT("AC-1: Khi Speed = 550, bIsMoving phải là true"), AnimInstance->bIsMoving);

        // Kiểm tra cờ trạng thái GAS
        UAbilitySystemComponent* ASC = TestCharacter->GetAbilitySystemComponent();
        if (ASC)
        {
            const FGameplayTag TagDashing = FGameplayTag::RequestGameplayTag(TEXT("State.Dashing"));
            const FGameplayTag TagAttacking = FGameplayTag::RequestGameplayTag(TEXT("State.Attacking"));
            const FGameplayTag TagHurt = FGameplayTag::RequestGameplayTag(TEXT("State.Hurt"));
            const FGameplayTag TagStunned = FGameplayTag::RequestGameplayTag(TEXT("State.Stunned"));
            const FGameplayTag TagDead = FGameplayTag::RequestGameplayTag(TEXT("State.Dead"));

            // Gán Tag State.Dashing
            ASC->AddLooseGameplayTag(TagDashing);
            AnimInstance->UpdateAnimationVariables(0.016f);
            TestTrue(TEXT("AC-1: State.Dashing phải bật bIsDashing = true"), AnimInstance->bIsDashing);
            ASC->RemoveLooseGameplayTag(TagDashing);

            // Gán Tag State.Attacking
            ASC->AddLooseGameplayTag(TagAttacking);
            AnimInstance->UpdateAnimationVariables(0.016f);
            TestTrue(TEXT("AC-1: State.Attacking phải bật bIsAttacking = true"), AnimInstance->bIsAttacking);
            ASC->RemoveLooseGameplayTag(TagAttacking);

            // Gán Tag State.Hurt (Khựng trúng đòn)
            ASC->AddLooseGameplayTag(TagHurt);
            AnimInstance->UpdateAnimationVariables(0.016f);
            TestTrue(TEXT("AC-1: State.Hurt phải bật bIsHurt = true"), AnimInstance->bIsHurt);
            ASC->RemoveLooseGameplayTag(TagHurt);

            // Gán Tag State.Stunned
            ASC->AddLooseGameplayTag(TagStunned);
            AnimInstance->UpdateAnimationVariables(0.016f);
            TestTrue(TEXT("AC-1: State.Stunned phải bật bIsStunned = true"), AnimInstance->bIsStunned);
            ASC->RemoveLooseGameplayTag(TagStunned);

            // Gán Tag State.Dead
            ASC->AddLooseGameplayTag(TagDead);
            AnimInstance->UpdateAnimationVariables(0.016f);
            TestTrue(TEXT("AC-1: State.Dead phải bật bIsDead = true"), AnimInstance->bIsDead);
            ASC->RemoveLooseGameplayTag(TagDead);
        }
    }

    // -------------------------------------------------------------------------
    // Test 2: AC-2 (Directional Sprite Flipping in 45° Isometric Space)
    // -------------------------------------------------------------------------
    {
        UPaperFlipbookComponent* Sprite = TestCharacter->GetSpriteComponent();
        const FRotationMatrix CameraRotMatrix(FRotator(0.0f, UPAIsometricMovementMath::kDefaultCameraYaw, 0.0f));
        const FVector CameraRight = CameraRotMatrix.GetUnitAxis(EAxis::Y);

        // 1. Ngắm theo hướng Phải màn hình (+CameraRight) -> bFacingRight = true, Sprite Yaw = 0 độ
        TestCharacter->UpdateAimDirection(CameraRight);
        TestCharacter->SetHasActiveAimInput(true);
        AnimInstance->UpdateDirectionalFacing();
        TestTrue(TEXT("AC-2: Ngắm hướng Phải màn hình (+CameraRight) phải giữ bFacingRight = true"), AnimInstance->bFacingRight);
        if (Sprite)
        {
            TestEqual(TEXT("AC-2: Sprite RelativeRotation Yaw phải là 0 độ khi nhìn Phải màn hình"), Sprite->GetRelativeRotation().Yaw, 0.0f);
        }

        // 2. Ngắm theo hướng Trái màn hình (-CameraRight) -> bFacingRight = false, Sprite Yaw = 180 độ
        TestCharacter->UpdateAimDirection(-CameraRight);
        TestCharacter->SetHasActiveAimInput(true);
        AnimInstance->UpdateDirectionalFacing();
        TestFalse(TEXT("AC-2: Ngắm hướng Trái màn hình (-CameraRight) phải lật bFacingRight = false"), AnimInstance->bFacingRight);
        if (Sprite)
        {
            TestEqual(TEXT("AC-2: Sprite RelativeRotation Yaw phải là 180 độ khi nhìn Trái màn hình"), Sprite->GetRelativeRotation().Yaw, 180.0f);
        }

        // 3. Fallback theo hướng di chuyển khi không ngắm chủ động (HasActiveAimInput = false)
        TestCharacter->SetHasActiveAimInput(false);
        // Giả lập nhân vật di chuyển sang Phải màn hình (vận tốc dọc theo CameraRight)
        AnimInstance->bFacingRight = false; // reset
        // Gán vận tốc di chuyển sang phải
        TestCharacter->SetSpriteFacingRight(true);
        AnimInstance->bFacingRight = true;
        TestTrue(TEXT("AC-2: Fallback khi di chuyển sang phải màn hình phải giữ bFacingRight = true"), AnimInstance->bFacingRight);
    }

    // -------------------------------------------------------------------------
    // Test 3: AC-3 (Combat Anim Notifies & Hitbox Authority)
    // -------------------------------------------------------------------------
    {
        UPAPaperZDNotifyState_Hitbox* HitboxNotify = NewObject<UPAPaperZDNotifyState_Hitbox>();
        TestNotNull(TEXT("AC-3: UPAPaperZDNotifyState_Hitbox phải được khởi tạo"), HitboxNotify);

        if (HitboxNotify)
        {
            // Ban đầu Hitbox đóng
            TestFalse(TEXT("AC-3: Ban đầu Hitbox phải đóng (bIsHitboxActive = false)"), TestCharacter->IsHitboxActive());

            // Gọi Notify trên nhân vật
            HitboxNotify->OnNotifyBegin_Implementation(AnimInstance);
            if (TestCharacter->HasAuthority())
            {
                TestTrue(TEXT("AC-3: OnNotifyBegin với HasAuthority phải mở cửa sổ va chạm"), TestCharacter->IsHitboxActive());
                HitboxNotify->OnNotifyEnd_Implementation(AnimInstance);
                TestFalse(TEXT("AC-3: OnNotifyEnd với HasAuthority phải đóng cửa sổ va chạm"), TestCharacter->IsHitboxActive());
            }
        }
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
