// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Controller/PAIsometricMovementMath.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPACursorDeprojectionAimTest
 *
 * Kiểm thử tự động đơn vị cho cơ chế chiếu tia con trỏ chuột và ngắm bắn độc lập (Story 002).
 * Kiểm tra các tiêu chí nghiệm thu:
 *  - AC-1: Giao cắt tia raycast với mặt phẳng nằm ngang tại cao độ Z của nhân vật (Ground Plane De-projection).
 *  - AC-2: Ngắm bắn và xoay mặt độc lập với hướng di chuyển (Decoupled Backpedaling).
 *  - AC-3: Lọc vùng chết Gamepad Deadzone (0.2) và bảo lưu góc xoay khi buông cần analog.
 *  - Phân chia chính xác 8 cung góc độ trực giao cho hoạt ảnh PaperZD.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPACursorDeprojectionAimTest,
    "ProjectAscendant.Foundation.Controller.CursorDeprojectionAndAiming",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPACursorDeprojectionAimTest::RunTest(const FString& Parameters)
{
    const float Sqrt2Over2 = FMath::Sqrt(2.0f) / 2.0f; // ~0.7071f

    // -------------------------------------------------------------------------
    // Test 1: AC-1 (Ground Plane De-projection Math)
    // -------------------------------------------------------------------------
    {
        // Giả lập Camera Isometric đặt tại (0, -1200, 1200) nhìn xuống nghiêng 45 độ
        const FVector RayOrigin(0.0f, -1200.0f, 1200.0f);
        // Hướng tia nhìn thẳng xuống giao cắt mặt phẳng đất tại (0, 0, 0)
        const FVector RayDirection = FVector(0.0f, 1.0f, -1.0f).GetSafeNormal();
        const float GroundZ = 0.0f;

        FVector IntersectionPoint;
        const bool bHit = UPAIsometricMovementMath::DeprojectRayToGroundPlane(RayOrigin, RayDirection, GroundZ, IntersectionPoint);

        TestTrue(TEXT("AC-1: Tia raycast phải cắt mặt phẳng ngang thành công"), bHit);
        TestEqual(TEXT("AC-1: Cao độ Z của giao điểm phải bằng đúng GroundZ (0.0f)"), IntersectionPoint.Z, 0.0f);
        TestNearlyEqual(TEXT("AC-1: Tọa độ X giao điểm phải là 0.0f"), IntersectionPoint.X, 0.0f, 0.1f);
        TestNearlyEqual(TEXT("AC-1: Tọa độ Y giao điểm phải là 0.0f (tâm màn hình)"), IntersectionPoint.Y, 0.0f, 0.1f);

        // Trường hợp tia gần như song song với mặt đất (RayDirection.Z ~ 0)
        const FVector ParallelRayOrigin(100.0f, 100.0f, 50.0f);
        const FVector ParallelRayDirection(1.0f, 0.0f, 0.0f); // Hoàn toàn nằm ngang
        FVector ParallelIntersection;
        const bool bParallelHit = UPAIsometricMovementMath::DeprojectRayToGroundPlane(ParallelRayOrigin, ParallelRayDirection, GroundZ, ParallelIntersection);
        TestFalse(TEXT("AC-1: Tia song song với mặt đất phải trả về false và không gây crash chia 0"), bParallelHit);
    }

    // -------------------------------------------------------------------------
    // Test 2: AC-2 (Decoupled Aiming & Backpedaling)
    // -------------------------------------------------------------------------
    {
        const FVector CharacterLocation(0.0f, 0.0f, 0.0f);
        // Mục tiêu nằm ở góc Đông Bắc (+500, +500, 0)
        const FVector TargetLocation(500.0f, 500.0f, 0.0f);

        // Tính toán vector hướng ngắm
        const FVector AimDirection = UPAIsometricMovementMath::CalculateAimDirection2D(CharacterLocation, TargetLocation);
        TestNearlyEqual(TEXT("AC-2: AimDirection.X phải là +sqrt(2)/2"), AimDirection.X, Sqrt2Over2, 0.001f);
        TestNearlyEqual(TEXT("AC-2: AimDirection.Y phải là +sqrt(2)/2"), AimDirection.Y, Sqrt2Over2, 0.001f);
        TestEqual(TEXT("AC-2: AimDirection.Z phải bằng 0.0f"), AimDirection.Z, 0.0f);

        // Tính góc ngắm theo độ
        const float AimAngle = UPAIsometricMovementMath::CalculateAimAngleDegrees(AimDirection);
        TestNearlyEqual(TEXT("AC-2: Góc ngắm hướng Đông Bắc phải là 45.0 độ"), AimAngle, 45.0f, 0.1f);

        // Lấy hướng 8 chiều
        const EPAAimDirection8Way Orientation = UPAIsometricMovementMath::Get8WayDirectionFromAngle(AimAngle);
        TestEqual(TEXT("AC-2: Hướng 8 chiều phải là NorthEast"), Orientation, EPAAimDirection8Way::NorthEast);

        // Kiểm tra độc lập: Người chơi đang lùi về phía Tây Nam (-500 cm/s)
        const FVector2D BackwardMoveInput(-1.0f, -1.0f); // Phím lùi
        const FVector VelocityDirection = UPAIsometricMovementMath::CalculateWorldDirection(BackwardMoveInput, 45.0f);

        // Vector di chuyển ngược 180 độ so với vector ngắm bắn
        const float DotProduct = FVector::DotProduct(AimDirection, VelocityDirection);
        TestNearlyEqual(TEXT("AC-2: Dot product giữa hướng ngắm và hướng di chuyển khi lùi phải xấp xỉ -1.0 (Ngược hướng 180 độ)"), DotProduct, -1.0f, 0.01f);
        TestEqual(TEXT("AC-2: Nhân vật vẫn duy trì góc ngắm NorthEast mặc dù đang di chuyển lùi về SouthWest"), Orientation, EPAAimDirection8Way::NorthEast);
    }

    // -------------------------------------------------------------------------
    // Test 3: Phân Vùng 8 Hướng Trực Giao PaperZD (Octant Mapping)
    // -------------------------------------------------------------------------
    {
        // Đông (0 độ): [-22.5, +22.5]
        TestEqual(TEXT("0 độ phải là East"), UPAIsometricMovementMath::Get8WayDirectionFromAngle(0.0f), EPAAimDirection8Way::East);
        TestEqual(TEXT("20 độ phải là East"), UPAIsometricMovementMath::Get8WayDirectionFromAngle(20.0f), EPAAimDirection8Way::East);
        TestEqual(TEXT("-20 độ phải là East"), UPAIsometricMovementMath::Get8WayDirectionFromAngle(-20.0f), EPAAimDirection8Way::East);

        // Đông Bắc (45 độ): [+22.5, +67.5]
        TestEqual(TEXT("45 độ phải là NorthEast"), UPAIsometricMovementMath::Get8WayDirectionFromAngle(45.0f), EPAAimDirection8Way::NorthEast);

        // Bắc (90 độ): [+67.5, +112.5]
        TestEqual(TEXT("90 độ phải là North"), UPAIsometricMovementMath::Get8WayDirectionFromAngle(90.0f), EPAAimDirection8Way::North);

        // Tây Bắc (135 độ): [+112.5, +157.5]
        TestEqual(TEXT("135 độ phải là NorthWest"), UPAIsometricMovementMath::Get8WayDirectionFromAngle(135.0f), EPAAimDirection8Way::NorthWest);

        // Tây (180 độ / -180 độ): [157.5, 180] hoặc [-180, -157.5]
        TestEqual(TEXT("180 độ phải là West"), UPAIsometricMovementMath::Get8WayDirectionFromAngle(180.0f), EPAAimDirection8Way::West);
        TestEqual(TEXT("-180 độ phải là West"), UPAIsometricMovementMath::Get8WayDirectionFromAngle(-180.0f), EPAAimDirection8Way::West);

        // Tây Nam (-135 độ): [-157.5, -112.5]
        TestEqual(TEXT("-135 độ phải là SouthWest"), UPAIsometricMovementMath::Get8WayDirectionFromAngle(-135.0f), EPAAimDirection8Way::SouthWest);

        // Nam (-90 độ): [-112.5, -67.5]
        TestEqual(TEXT("-90 độ phải là South"), UPAIsometricMovementMath::Get8WayDirectionFromAngle(-90.0f), EPAAimDirection8Way::South);

        // Đông Nam (-45 độ): [-67.5, -22.5]
        TestEqual(TEXT("-45 độ phải là SouthEast"), UPAIsometricMovementMath::Get8WayDirectionFromAngle(-45.0f), EPAAimDirection8Way::SouthEast);
    }

    // -------------------------------------------------------------------------
    // Test 4: AC-3 (Gamepad Twin-Stick Deadzone Filtering)
    // -------------------------------------------------------------------------
    {
        const float Deadzone = 0.20f;

        // 4.1: Cần analog bị lệch nhẹ trong vùng chết (độ dài vector = 0.1414 < 0.20)
        const FVector2D SubDeadzoneInput(0.1f, 0.1f);
        const FVector2D FilteredSub = UPAIsometricMovementMath::FilterGamepadAimInput(SubDeadzoneInput, Deadzone);
        TestTrue(TEXT("AC-3: Input trong vùng chết (< 0.20) phải bị lọc thành ZeroVector (bảo lưu hướng cũ)"), FilteredSub.IsNearlyZero());

        // 4.2: Cần analog đẩy mạnh vượt qua deadzone (X = 0.6, Y = 0.8, Size = 1.0)
        const FVector2D ActiveInput(0.6f, 0.8f);
        const FVector2D FilteredActive = UPAIsometricMovementMath::FilterGamepadAimInput(ActiveInput, Deadzone);
        TestFalse(TEXT("AC-3: Input vượt vùng chết phải được xử lý và khác ZeroVector"), FilteredActive.IsNearlyZero());
        TestNearlyEqual(TEXT("AC-3: Hướng của vector sau khi lọc phải trùng khớp với hướng ban đầu"), (FilteredActive.GetSafeNormal() - ActiveInput.GetSafeNormal()).Size(), 0.0f, 0.001f);
        TestEqual(TEXT("AC-3: Độ lớn sau khi tái chuẩn hóa ở cực đại (Size 1.0) phải là 1.0"), FilteredActive.Size(), 1.0f);
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
