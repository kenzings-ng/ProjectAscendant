// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Controller/PAIsometricMovementMath.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAScreenRelativeMovementTest
 *
 * Kiểm thử tự động đơn vị cho hệ thống điều khiển di chuyển 2.5D Isometric (Story 001).
 * Kiểm tra các tiêu chí nghiệm thu:
 *  - AC-1: Ánh xạ di chuyển trực giao theo màn hình isometric qua ma trận xoay Yaw 45 độ.
 *  - AC-2: Chuẩn hóa vector đường chéo về 1.0, kẹp vận tốc tối đa đúng 550 cm/s (ngăn chặn bug chạy chéo 1.414x).
 *  - AC-3: Triệt tiêu phím đối nghịch (W+S hoặc A+D) đưa vector về (0,0) và chuyển trạng thái Idle ngay trong 1 frame.
 *  - Kiểm tra hệ số suy giảm vận tốc khi dính trạng thái Kiệt Sức State.Exhausted (-25%).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPAScreenRelativeMovementTest,
    "ProjectAscendant.Foundation.Controller.ScreenRelativeMovement",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAScreenRelativeMovementTest::RunTest(const FString& Parameters)
{
    const float Sqrt2Over2 = FMath::Sqrt(2.0f) / 2.0f; // ~0.70710678f

    // -------------------------------------------------------------------------
    // Test 1: AC-1 (Camera-Relative Orthogonal 8-Way Mapping)
    // -------------------------------------------------------------------------
    {
        // 1.1: Nhấn phím W (Input hướng thẳng lên màn hình: X = 0, Y = +1)
        const FVector2D InputUp(0.0f, 1.0f);
        const FVector WorldDirUp = UPAIsometricMovementMath::CalculateWorldDirection(InputUp, 45.0f);
        
        // Với Yaw = 45 độ:
        // Forward vector = (cos 45, sin 45, 0) = (+0.7071, +0.7071, 0)
        // Vector thế giới kết quả phải chạy chéo theo trục tọa độ thế giới (tiến thẳng lên góc nhìn màn hình)
        TestNearlyEqual(TEXT("AC-1 [W]: WorldDirection.X phải xấp xỉ +sqrt(2)/2"), WorldDirUp.X, Sqrt2Over2, 0.001f);
        TestNearlyEqual(TEXT("AC-1 [W]: WorldDirection.Y phải xấp xỉ +sqrt(2)/2"), WorldDirUp.Y, Sqrt2Over2, 0.001f);
        TestEqual(TEXT("AC-1 [W]: WorldDirection.Z phải bằng 0.0 (chặt chẽ trên mặt phẳng ngang)"), WorldDirUp.Z, 0.0f);
        TestNearlyEqual(TEXT("AC-1 [W]: Độ dài vector phải bằng đúng 1.0"), WorldDirUp.Size(), 1.0f, 0.001f);

        // 1.2: Nhấn phím S (Input lùi thẳng xuống màn hình: X = 0, Y = -1)
        const FVector2D InputDown(0.0f, -1.0f);
        const FVector WorldDirDown = UPAIsometricMovementMath::CalculateWorldDirection(InputDown, 45.0f);
        TestNearlyEqual(TEXT("AC-1 [S]: WorldDirection.X phải xấp xỉ -sqrt(2)/2"), WorldDirDown.X, -Sqrt2Over2, 0.001f);
        TestNearlyEqual(TEXT("AC-1 [S]: WorldDirection.Y phải xấp xỉ -sqrt(2)/2"), WorldDirDown.Y, -Sqrt2Over2, 0.001f);
        TestNearlyEqual(TEXT("AC-1 [S]: Độ dài vector phải bằng đúng 1.0"), WorldDirDown.Size(), 1.0f, 0.001f);

        // 1.3: Nhấn phím D (Input sang phải màn hình: X = +1, Y = 0)
        const FVector2D InputRight(1.0f, 0.0f);
        const FVector WorldDirRight = UPAIsometricMovementMath::CalculateWorldDirection(InputRight, 45.0f);
        // Right vector với Yaw = 45 độ: (-sin 45, cos 45, 0) = (-0.7071, +0.7071, 0)
        TestNearlyEqual(TEXT("AC-1 [D]: WorldDirection.X phải xấp xỉ -sqrt(2)/2"), WorldDirRight.X, -Sqrt2Over2, 0.001f);
        TestNearlyEqual(TEXT("AC-1 [D]: WorldDirection.Y phải xấp xỉ +sqrt(2)/2"), WorldDirRight.Y, Sqrt2Over2, 0.001f);
        TestNearlyEqual(TEXT("AC-1 [D]: Độ dài vector phải bằng đúng 1.0"), WorldDirRight.Size(), 1.0f, 0.001f);

        // 1.4: Nhấn phím A (Input sang trái màn hình: X = -1, Y = 0)
        const FVector2D InputLeft(-1.0f, 0.0f);
        const FVector WorldDirLeft = UPAIsometricMovementMath::CalculateWorldDirection(InputLeft, 45.0f);
        TestNearlyEqual(TEXT("AC-1 [A]: WorldDirection.X phải xấp xỉ +sqrt(2)/2"), WorldDirLeft.X, Sqrt2Over2, 0.001f);
        TestNearlyEqual(TEXT("AC-1 [A]: WorldDirection.Y phải xấp xỉ -sqrt(2)/2"), WorldDirLeft.Y, -Sqrt2Over2, 0.001f);
        TestNearlyEqual(TEXT("AC-1 [A]: Độ dài vector phải bằng đúng 1.0"), WorldDirLeft.Size(), 1.0f, 0.001f);
    }

    // -------------------------------------------------------------------------
    // Test 2: AC-2 (Diagonal Velocity Normalization & Anti-Exploit Clamp)
    // -------------------------------------------------------------------------
    {
        // Khi người chơi nhấn W + D cùng lúc: Input 2D thô là (X = 1.0, Y = 1.0)
        const FVector2D RawDiagonalInput(1.0f, 1.0f);
        const float RawMagnitude = RawDiagonalInput.Size(); // sqrt(1^2 + 1^2) ~ 1.41421f
        TestNearlyEqual(TEXT("AC-2: Độ dài vector thô khi bấm W+D phải là ~1.414"), RawMagnitude, 1.41421f, 0.001f);

        // Chuẩn hóa vector input
        const FVector2D NormalizedInput = UPAIsometricMovementMath::NormalizeInputVector(RawDiagonalInput);
        TestNearlyEqual(TEXT("AC-2: Vector sau khi chuẩn hóa phải có độ dài chính xác 1.000"), NormalizedInput.Size(), 1.0f, 0.001f);

        // Tính hướng thế giới 3D từ vector đường chéo
        const FVector WorldDiagonalDir = UPAIsometricMovementMath::CalculateWorldDirection(RawDiagonalInput, 45.0f);
        TestNearlyEqual(TEXT("AC-2: Vector hướng thế giới đường chéo phải được kẹp độ dài đúng 1.0"), WorldDiagonalDir.Size(), 1.0f, 0.001f);

        // Kiểm tra vận tốc di chuyển đường chéo
        const float BaseMoveSpeed = 550.0f;
        const float DiagonalSpeed = WorldDiagonalDir.Size() * BaseMoveSpeed;
        TestEqual(TEXT("AC-2: Vận tốc di chuyển đường chéo phải bằng đúng vận tốc cơ sở (550 cm/s), không bị nhân 1.414x"), DiagonalSpeed, 550.0f);
    }

    // -------------------------------------------------------------------------
    // Test 3: AC-3 (Opposing Input Cancellation & Instant Idle Transition)
    // -------------------------------------------------------------------------
    {
        // 3.1: Người chơi nhấn W và S cùng lúc (Tiến + Lùi)
        const FVector2D OpposingVertical = UPAIsometricMovementMath::ResolveOpposingInputs(true, true, false, false);
        TestEqual(TEXT("AC-3: Nhấn đồng thời W + S phải triệt tiêu trục dọc về 0.0"), OpposingVertical.Y, 0.0f);
        TestTrue(TEXT("AC-3: Vector sau khi triệt tiêu W+S phải là ZeroVector (chuyển Idle ngay)"), OpposingVertical.IsNearlyZero());

        // 3.2: Người chơi nhấn A và D cùng lúc (Trái + Phải)
        const FVector2D OpposingHorizontal = UPAIsometricMovementMath::ResolveOpposingInputs(false, false, true, true);
        TestEqual(TEXT("AC-3: Nhấn đồng thời A + D phải triệt tiêu trục ngang về 0.0"), OpposingHorizontal.X, 0.0f);
        TestTrue(TEXT("AC-3: Vector sau khi triệt tiêu A+D phải là ZeroVector (chuyển Idle ngay)"), OpposingHorizontal.IsNearlyZero());

        // 3.3: Người chơi giữ W nhưng nhấn cả A lẫn D (W + A + D)
        const FVector2D PartialOpposing = UPAIsometricMovementMath::ResolveOpposingInputs(true, false, true, true);
        TestEqual(TEXT("AC-3 [W+A+D]: Trục ngang A+D bị triệt tiêu về 0"), PartialOpposing.X, 0.0f);
        TestEqual(TEXT("AC-3 [W+A+D]: Trục dọc W vẫn bảo lưu 1.0"), PartialOpposing.Y, 1.0f);

        // Vector hướng thế giới khi triệt tiêu hoàn toàn
        const FVector ResultingWorldDir = UPAIsometricMovementMath::CalculateWorldDirection(OpposingVertical, 45.0f);
        TestTrue(TEXT("AC-3: Gia tốc di chuyển thế giới phải bằng 0 khi phím bị triệt tiêu"), ResultingWorldDir.IsNearlyZero());
    }

    // -------------------------------------------------------------------------
    // Test 4: Trạng Thái Kiệt Sức (State.Exhausted Movement Penalty)
    // -------------------------------------------------------------------------
    {
        const float BaseSpeed = 550.0f;

        // Bình thường: 550 cm/s
        const float NormalSpeed = UPAIsometricMovementMath::CalculateEffectiveMoveSpeed(BaseSpeed, false);
        TestEqual(TEXT("Vận tốc bình thường phải là 550 cm/s"), NormalSpeed, 550.0f);

        // Khi Kiệt Sức: giảm 25% => 550 * 0.75 = 412.5 cm/s
        const float ExhaustedSpeed = UPAIsometricMovementMath::CalculateEffectiveMoveSpeed(BaseSpeed, true);
        TestEqual(TEXT("Vận tốc khi Kiệt Sức (State.Exhausted) phải bị giảm 25% xuống 412.5 cm/s"), ExhaustedSpeed, 412.5f);
    }

    // -------------------------------------------------------------------------
    // Test 5: Biên Ngoại Lệ (Zero Input & Sub-Deadzone)
    // -------------------------------------------------------------------------
    {
        const FVector2D ZeroInput(0.0f, 0.0f);
        const FVector ZeroWorldDir = UPAIsometricMovementMath::CalculateWorldDirection(ZeroInput, 45.0f);
        TestTrue(TEXT("Zero input phải trả về ZeroVector mà không gây crash chia 0"), ZeroWorldDir.IsNearlyZero());

        const FVector2D SubDeadzoneInput(0.00001f, 0.00001f);
        const FVector2D NormalizedSub = UPAIsometricMovementMath::NormalizeInputVector(SubDeadzoneInput);
        TestTrue(TEXT("Input siêu nhỏ dưới epsilon phải được xử lý an toàn"), NormalizedSub.X >= 0.0f);
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
