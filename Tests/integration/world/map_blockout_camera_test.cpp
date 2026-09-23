// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Controller/PAIsometricMovementMath.h"
#include "Character/PABaseCharacter.h"
#include "Game/PAGameModeBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "Misc/ConfigCacheIni.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAMapBlockoutCameraIntegrationTest
 *
 * Kiểm thử tích hợp cho Story map-001:
 *  - AC-1: Cấu hình Default Map trong DefaultEngine.ini trỏ tới /Game/Maps/L_Ruins_Blockout.
 *  - AC-2: Góc camera Isometric cố định Pitch -45°, Yaw +45°, SpringArm không bị va chạm đâm vào người chơi.
 *  - AC-3: Thử nghiệm Line-of-Sight Occlusion Raycast xác định vật cản giữa Camera và Nhân vật.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPAMapBlockoutCameraIntegrationTest,
    "ProjectAscendant.Core.World.MapBlockoutCameraIntegration",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAMapBlockoutCameraIntegrationTest::RunTest(const FString& Parameters)
{
    // -------------------------------------------------------------------------
    // Test 1: AC-1 (Default Map Configuration Verification)
    // -------------------------------------------------------------------------
    {
        FString EditorStartupMap;
        FString GameDefaultMap;
        
        GConfig->GetString(
            TEXT("/Script/EngineSettings.GameMapsSettings"),
            TEXT("EditorStartupMap"),
            EditorStartupMap,
            GEngineIni
        );

        GConfig->GetString(
            TEXT("/Script/EngineSettings.GameMapsSettings"),
            TEXT("GameDefaultMap"),
            GameDefaultMap,
            GEngineIni
        );

        const bool bValidEditorMap = EditorStartupMap.Contains(TEXT("L_Ruins_Blockout")) || EditorStartupMap.Contains(TEXT("L_VerdantFrontier_Outpost"));
        const bool bValidGameMap = GameDefaultMap.Contains(TEXT("L_Ruins_Blockout")) || GameDefaultMap.Contains(TEXT("L_VerdantFrontier_Outpost"));
        TestTrue(TEXT("AC-1: EditorStartupMap phải chứa L_Ruins_Blockout hoặc L_VerdantFrontier_Outpost"), bValidEditorMap);
        TestTrue(TEXT("AC-1: GameDefaultMap phải chứa L_Ruins_Blockout hoặc L_VerdantFrontier_Outpost"), bValidGameMap);
    }

    // -------------------------------------------------------------------------
    // Test 2: AC-2 (Isometric Camera Rig & SpringArm Verification)
    // -------------------------------------------------------------------------
    {
        APABaseCharacter* DummyCharacter = NewObject<APABaseCharacter>();
        TestNotNull(TEXT("AC-2: DummyCharacter phải được khởi tạo thành công"), DummyCharacter);

        if (DummyCharacter)
        {
            USpringArmComponent* Boom = DummyCharacter->GetCameraBoom();
            TestNotNull(TEXT("AC-2: CameraBoom phải tồn tại trên APABaseCharacter"), Boom);

            if (Boom)
            {
                TestNearlyEqual(TEXT("AC-2: TargetArmLength phải bằng 1400.0 cm"), Boom->TargetArmLength, 1400.0f, 0.1f);
                TestFalse(TEXT("AC-2: bDoCollisionTest PHẢI bằng false"), Boom->bDoCollisionTest);
                TestTrue(TEXT("AC-2: bEnableCameraLag phải bật"), Boom->bEnableCameraLag);

                const FRotator BoomRot = Boom->GetRelativeRotation();
                TestNearlyEqual(TEXT("AC-2: Boom Relative Pitch phải là -45 độ"), static_cast<float>(BoomRot.Pitch), -45.0f, 0.1f);
                TestNearlyEqual(TEXT("AC-2: Boom Relative Yaw phải là 45 độ"), static_cast<float>(BoomRot.Yaw), 45.0f, 0.1f);
            }

            UCameraComponent* Cam = DummyCharacter->GetFollowCamera();
            TestNotNull(TEXT("AC-2: FollowCamera phải tồn tại"), Cam);
            if (Cam)
            {
                TestEqual(TEXT("AC-2: ProjectionMode của FollowCamera phải là Perspective"), Cam->ProjectionMode, ECameraProjectionMode::Perspective);
                TestNearlyEqual(TEXT("AC-2: FieldOfView phải chuẩn 2.5D Isometric (50 độ)"), Cam->FieldOfView, 50.0f, 5.0f);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Test 3: AC-3 (Line-of-Sight Occlusion Raycast Simulation)
    // -------------------------------------------------------------------------
    {
        // Vị trí nhân vật tại gốc tọa độ (0, 0, 100)
        const FVector PlayerLocation(0.0f, 0.0f, 100.0f);

        // Vector hướng nhìn camera Isometric (-45 pitch, +45 yaw)
        const FRotator CameraRot(-45.0f, 45.0f, 0.0f);
        const FVector CameraBackVector = -CameraRot.Vector();
        const FVector CameraLocation = PlayerLocation + (CameraBackVector * 1400.0f);

        // Cột đá đặt tại (-300, -300, 300) nằm chính giữa đường nhìn từ Camera đến Player
        const FVector PillarLocation(-300.0f, -300.0f, 300.0f);
        const FBox PillarBox(PillarLocation - FVector(100.0f, 100.0f, 300.0f), PillarLocation + FVector(100.0f, 100.0f, 300.0f));

        // Kiểm tra xem tia nhìn từ Camera -> Player có giao cắt với Hộp giới hạn của cột đá không
        FVector HitLocation;
        FVector HitNormal;
        float HitTime;
        const bool bIntersect = FMath::LineExtentBoxIntersection(
            PillarBox,
            CameraLocation,
            PlayerLocation,
            FVector(10.0f, 10.0f, 10.0f), // Bán kính tia quét capsule nhỏ
            HitLocation,
            HitNormal,
            HitTime
        );

        TestTrue(TEXT("AC-3: Đường nhìn từ Camera đến Player phải bị cột đá cắt ngang để kích hoạt Occlusion Dither"), bIntersect);
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
