// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Controller/PAIsometricMovementMath.h"
#include "Character/PABaseCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPACameraLookAheadOcclusionIntegrationTest
 *
 * Kiểm thử tích hợp cho hệ thống Camera Isometric, SpringArm, cơ chế trôi đón đầu (Dynamic Look-Ahead)
 * và hình trụ quét làm mờ dither vật thể che khuất tầm nhìn (Story 003).
 *
 * Kiểm tra các tiêu chí nghiệm thu:
 *  - AC-1: Cấu hình SpringArm chuẩn 2.5D Isometric (Pitch -45°, Yaw 45°, ArmLength 1200, bDoCollisionTest=false, CameraLag=12).
 *  - AC-2: Độ trôi đón đầu Look-Ahead tối đa 250cm theo hướng ngắm và hồi tâm mượt mà với tốc độ 8.0.
 *  - AC-3: Quét hình trụ Line-of-Sight bán kính 200cm, kích hoạt làm mờ DitherOpacity = 0.25 cho vật cản che khuất tầm nhìn.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPACameraLookAheadOcclusionIntegrationTest,
    "ProjectAscendant.Foundation.Controller.CameraLookAheadAndOcclusion",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPACameraLookAheadOcclusionIntegrationTest::RunTest(const FString& Parameters)
{
    // -------------------------------------------------------------------------
    // Test 1: AC-1 (Isometric SpringArm Specifications & Invariance)
    // -------------------------------------------------------------------------
    {
        // Kiểm tra các hằng số quy chuẩn hệ thống Camera 2.5D Isometric
        TestNearlyEqual(TEXT("AC-1: Góc nghiêng Pitch mặc định phải là -45.0 độ"), UPAIsometricMovementMath::kDefaultCameraPitch, -45.0f, 0.01f);
        TestNearlyEqual(TEXT("AC-1: Góc xoay Yaw mặc định phải là +45.0 độ"), UPAIsometricMovementMath::kDefaultCameraYaw, 45.0f, 0.01f);
        TestNearlyEqual(TEXT("AC-1: Chiều dài tay vươn SpringArm TargetArmLength phải là 1200.0 cm"), UPAIsometricMovementMath::kDefaultTargetArmLength, 1200.0f, 0.01f);
        TestNearlyEqual(TEXT("AC-1: Tốc độ trễ camera CameraLagSpeed phải là 12.0"), UPAIsometricMovementMath::kDefaultCameraLagSpeed, 12.0f, 0.01f);

        // Khởi tạo đối tượng nhân vật giả lập để kiểm tra cấu hình component
        APABaseCharacter* DummyCharacter = NewObject<APABaseCharacter>();
        TestNotNull(TEXT("AC-1: DummyCharacter phải được khởi tạo thành công"), DummyCharacter);

        if (DummyCharacter)
        {
            USpringArmComponent* Boom = DummyCharacter->GetCameraBoom();
            TestNotNull(TEXT("AC-1: CameraBoom (USpringArmComponent) phải tồn tại trên APABaseCharacter"), Boom);

            if (Boom)
            {
                TestEqual(TEXT("AC-1: TargetArmLength của CameraBoom phải là 1200.0 cm"), Boom->TargetArmLength, 1200.0f);
                TestFalse(TEXT("AC-1: bDoCollisionTest PHẢI bằng false để ngăn camera tự thu phóng đâm vào nhân vật"), Boom->bDoCollisionTest);
                TestTrue(TEXT("AC-1: bEnableCameraLag phải bật để làm mượt chuyển động lướt né"), Boom->bEnableCameraLag);
                TestNearlyEqual(TEXT("AC-1: CameraLagSpeed phải bằng 12.0"), Boom->CameraLagSpeed, 12.0f, 0.01f);

                // Khóa góc xoay không cho xoay theo nhân vật
                TestFalse(TEXT("AC-1: bInheritPitch phải là false"), Boom->bInheritPitch);
                TestFalse(TEXT("AC-1: bInheritYaw phải là false"), Boom->bInheritYaw);
                TestFalse(TEXT("AC-1: bInheritRoll phải là false"), Boom->bInheritRoll);

                // Kiểm tra góc xoay tương đối ban đầu
                const FRotator BoomRot = Boom->GetRelativeRotation();
                TestNearlyEqual(TEXT("AC-1: Boom Relative Pitch phải là -45 độ"), BoomRot.Pitch, -45.0f, 0.1f);
                TestNearlyEqual(TEXT("AC-1: Boom Relative Yaw phải là 45 độ"), BoomRot.Yaw, 45.0f, 0.1f);
                TestNearlyEqual(TEXT("AC-1: Boom Relative Roll phải là 0 độ"), BoomRot.Roll, 0.0f, 0.1f);
            }

            UCameraComponent* Cam = DummyCharacter->GetFollowCamera();
            TestNotNull(TEXT("AC-1: FollowCamera (UCameraComponent) phải tồn tại trên APABaseCharacter"), Cam);
            if (Cam)
            {
                TestFalse(TEXT("AC-1: FollowCamera bUsePawnControlRotation phải là false"), Cam->bUsePawnControlRotation);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Test 2: AC-2 (Dynamic Look-Ahead Offset Bounds & Interpolation)
    // -------------------------------------------------------------------------
    {
        const FVector CharacterLocation(0.0f, 0.0f, 0.0f);

        // Trường hợp 1: Chuột rê cực xa (+1500 cm dọc trục X) -> Khoảng cách vượt quá ngưỡng 250 cm
        const FVector FarCursorLocation(1500.0f, 0.0f, 0.0f);
        const FVector TargetOffsetFar = UPAIsometricMovementMath::CalculateLookAheadTargetOffset(CharacterLocation, FarCursorLocation, 250.0f, 0.35f);

        TestNearlyEqual(TEXT("AC-2: TargetOffset.X khi chuột ở xa phải bị chặn trên ở mức tối đa 250.0 cm"), TargetOffsetFar.X, 250.0f, 0.01f);
        TestNearlyEqual(TEXT("AC-2: TargetOffset.Y phải là 0.0 khi chuột nằm thẳng trục X"), TargetOffsetFar.Y, 0.0f, 0.01f);
        TestEqual(TEXT("AC-2: TargetOffset.Z phải luôn bằng 0.0f (trên mặt phẳng ngang)"), TargetOffsetFar.Z, 0.0f);

        // Trường hợp 2: Chuột rê cự ly gần (+300 cm) -> TargetDist = 300 * 0.35 = 105 cm (< 250 cm)
        const FVector NearCursorLocation(300.0f, 0.0f, 0.0f);
        const FVector TargetOffsetNear = UPAIsometricMovementMath::CalculateLookAheadTargetOffset(CharacterLocation, NearCursorLocation, 250.0f, 0.35f);
        TestNearlyEqual(TEXT("AC-2: TargetOffset.X khi chuột ở gần phải bằng 300 * 0.35 = 105.0 cm"), TargetOffsetNear.X, 105.0f, 0.01f);

        // Trường hợp 3: Chuột di chuyển chéo góc Đông Bắc (+500, +500)
        const FVector DiagonalCursorLocation(500.0f, 500.0f, 0.0f);
        const FVector TargetOffsetDiag = UPAIsometricMovementMath::CalculateLookAheadTargetOffset(CharacterLocation, DiagonalCursorLocation, 250.0f, 0.35f);
        const float Sqrt2Over2 = FMath::Sqrt(2.0f) / 2.0f;
        // Dist2D = ~707.1 cm -> 707.1 * 0.35 = 247.48 cm (< 250 cm)
        const float ExpectedOffset = 707.10678f * 0.35f;
        TestNearlyEqual(TEXT("AC-2: Vector đón đầu chéo X phải đúng hướng sqrt(2)/2"), TargetOffsetDiag.X, ExpectedOffset * Sqrt2Over2, 0.5f);
        TestNearlyEqual(TEXT("AC-2: Vector đón đầu chéo Y phải đúng hướng sqrt(2)/2"), TargetOffsetDiag.Y, ExpectedOffset * Sqrt2Over2, 0.5f);

        // Trường hợp 4: Kiểm tra nội suy làm mượt VInterpTo và hồi tâm khi chuột về (0, 0)
        FVector CurrentOffset(250.0f, 0.0f, 0.0f);
        const FVector ZeroTarget(0.0f, 0.0f, 0.0f);
        const float DeltaTime = 0.016f; // Giả lập 60 FPS
        const float InterpSpeed = 8.0f; // LookAheadReturnSpeed = 8.0

        FVector InterpStep1 = UPAIsometricMovementMath::InterpolateLookAheadOffset(CurrentOffset, ZeroTarget, DeltaTime, InterpSpeed);
        TestTrue(TEXT("AC-2: Độ lệch camera phải giảm dần về 0 khi hồi tâm"), InterpStep1.X < 250.0f && InterpStep1.X > 0.0f);

        // Chạy tiếp 60 bước (~1 giây) -> offset phải tiệm cận hoàn toàn về 0
        for (int32 Step = 0; Step < 60; ++Step)
        {
            CurrentOffset = UPAIsometricMovementMath::InterpolateLookAheadOffset(CurrentOffset, ZeroTarget, DeltaTime, InterpSpeed);
        }
        TestNearlyEqual(TEXT("AC-2: Sau 1 giây hồi tâm, SocketOffset phải trở về xấp xỉ 0.0 cm"), CurrentOffset.X, 0.0f, 0.1f);
    }

    // -------------------------------------------------------------------------
    // Test 3: AC-3 (Line-of-Sight Occlusion Cylinder & Dither Transparency)
    // -------------------------------------------------------------------------
    {
        // Camera đặt tại (0, -1200, 1200) hướng nhìn về nhân vật tại (0, 0, 0)
        const FVector CameraLocation(0.0f, -1200.0f, 1200.0f);
        const FVector CharacterLocation(0.0f, 0.0f, 0.0f);
        const float OcclusionRadius = 200.0f; // Bán kính hình trụ 200 cm

        // 3.1: Điểm nằm ngay chính giữa đường ngắm Camera -> Nhân vật tại vị trí (0, -600, 600)
        const FVector DirectHitPoint(0.0f, -600.0f, 600.0f);
        const bool bDirectInCylinder = UPAIsometricMovementMath::IsPointWithinOcclusionCylinder(DirectHitPoint, CameraLocation, CharacterLocation, OcclusionRadius);
        TestTrue(TEXT("AC-3: Điểm nằm trực tiếp trên đường nối camera và nhân vật phải nằm trong hình trụ che khuất"), bDirectInCylinder);

        // 3.2: Điểm lệch sang bên 100 cm (bán kính lệch 100cm < 200cm)
        const FVector LateralClosePoint(100.0f, -600.0f, 600.0f);
        const bool bLateralCloseIn = UPAIsometricMovementMath::IsPointWithinOcclusionCylinder(LateralClosePoint, CameraLocation, CharacterLocation, OcclusionRadius);
        TestTrue(TEXT("AC-3: Điểm lệch ngang 100cm (nhỏ hơn bán kính 200cm) phải được tính là che khuất"), bLateralCloseIn);

        // 3.3: Điểm lệch sang bên 350 cm (vượt quá bán kính 200cm)
        const FVector LateralFarPoint(350.0f, -600.0f, 600.0f);
        const bool bLateralFarIn = UPAIsometricMovementMath::IsPointWithinOcclusionCylinder(LateralFarPoint, CameraLocation, CharacterLocation, OcclusionRadius);
        TestFalse(TEXT("AC-3: Điểm lệch ngang 350cm (vượt quá bán kính 200cm) KHÔNG được tính là che khuất"), bLateralFarIn);

        // 3.4: Điểm nằm ở phía sau lưng Camera (ngoài đoạn thẳng giữa camera và nhân vật)
        const FVector BehindCameraPoint(0.0f, -1600.0f, 1600.0f);
        const bool bBehindCamIn = UPAIsometricMovementMath::IsPointWithinOcclusionCylinder(BehindCameraPoint, CameraLocation, CharacterLocation, OcclusionRadius);
        TestFalse(TEXT("AC-3: Điểm nằm sau lưng Camera không được tính vào hình trụ che khuất tầm nhìn"), bBehindCamIn);

        // 3.5: Kiểm tra tham số Dither Opacity quy chuẩn theo GDD
        TestEqual(TEXT("AC-3: Độ mờ dither tiêu chuẩn khi che khuất phải là 0.25f"), UPAIsometricMovementMath::kDefaultOcclusionDitherOpacity, 0.25f);
        TestEqual(TEXT("AC-3: Bán kính làm mờ tiêu chuẩn phải là 200.0 cm"), UPAIsometricMovementMath::kDefaultOcclusionFadeRadius, 200.0f);
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
