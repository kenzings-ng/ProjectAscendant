// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Network/PANetLagCompensation.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPANetLagCompensationTest
 *
 * Kiểm thử tự động cho hệ thống Dedicated Server Locomotion Prediction,
 * Lag Compensation Rewind và Soft Reconciliation (Story 001 / net-001).
 *
 * Tiêu chí nghiệm thu:
 *  - AC-1: Server lưu trữ circular buffer 200ms (20 mẫu @ 100Hz); tua lại vị trí mục tiêu theo timestamp để quét va chạm đòn đánh;
 *          từ chối timestamp tương lai; kẹp timestamp quá 200ms về mốc 200ms.
 *  - AC-12: Ngưỡng sai lệch 15cm; nếu <= 15cm thì bỏ qua (chống jitter); nếu > 15cm thì làm mịn hàm mũ qua 0.15s thay vì giật cục.
 *  - AC-9: Cấu hình va chạm giữa đồng minh trong khu vực PvE cho phép xuyên qua nhau (Zero Body Blocking);
 *          quái vật và chế độ PvP giữ va chạm cản trở (Blocking).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPANetLagCompensationTest,
	"ProjectAscendant.Foundation.Netcode.LocomotionLagCompensation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPANetLagCompensationTest::RunTest(const FString& Parameters)
{
	// -------------------------------------------------------------------------
	// Test 1: AC-1 (Circular Buffer Storage & 20-Sample / 200ms Capacity)
	// -------------------------------------------------------------------------
	{
		FPAPositionHistoryBuffer HistoryBuffer;
		TestEqual(TEXT("AC-1: Buffer khởi tạo ban đầu phải rỗng"), HistoryBuffer.GetSnapshotCount(), 0);

		// Ghi 25 mẫu với bước nhảy 10ms (100Hz), bắt đầu từ T = 1.00s đến T = 1.24s
		for (int32 i = 0; i < 25; ++i)
		{
			const float Timestamp = 1.00f + (i * 0.01f);
			const FVector Location(i * 10.0f, 0.0f, 0.0f);
			HistoryBuffer.RecordSnapshot(Timestamp, Location, FRotator::ZeroRotator);
		}

		// Buffer chỉ chứa tối đa 20 mẫu (200ms)
		TestEqual(TEXT("AC-1: Dung lượng buffer phải được giới hạn tối đa ở 20 mẫu"), HistoryBuffer.GetSnapshotCount(), 20);

		// 5 mẫu đầu tiên (1.00s - 1.04s) đã bị ghi đè, mẫu cũ nhất còn lại phải là T = 1.05s
		TestNearlyEqual(TEXT("AC-1: Mẫu cũ nhất sau khi ghi đè vòng tròn phải là T = 1.05s"), HistoryBuffer.GetOldestTimestamp(), 1.05f, 0.0001f);
		TestNearlyEqual(TEXT("AC-1: Mẫu mới nhất phải là T = 1.24s"), HistoryBuffer.GetNewestTimestamp(), 1.24f, 0.0001f);
	}

	// -------------------------------------------------------------------------
	// Test 2: AC-1 (Linear Interpolation Accuracy Between Snapshots)
	// -------------------------------------------------------------------------
	{
		FPAPositionHistoryBuffer HistoryBuffer;
		// Ghi 2 snapshot tại T = 10.00s và T = 10.10s
		HistoryBuffer.RecordSnapshot(10.00f, FVector(0.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
		HistoryBuffer.RecordSnapshot(10.10f, FVector(100.0f, 200.0f, 0.0f), FRotator(0.0f, 90.0f, 0.0f));

		FPAPastTransform InterpSnap;
		// Truy vấn tại T = 10.05s (chính xác điểm giữa 50%)
		const bool bSuccess = HistoryBuffer.GetInterpolatedSnapshot(10.05f, InterpSnap);

		TestTrue(TEXT("AC-1: Truy vấn snapshot nội suy trong khoảng thời gian hợp lệ phải thành công"), bSuccess);
		TestNearlyEqual(TEXT("AC-1: Tọa độ X nội suy tại 50% phải là 50.0 cm"), InterpSnap.Location.X, 50.0f, 0.01f);
		TestNearlyEqual(TEXT("AC-1: Tọa độ Y nội suy tại 50% phải là 100.0 cm"), InterpSnap.Location.Y, 100.0f, 0.01f);
		TestNearlyEqual(TEXT("AC-1: Góc Yaw nội suy tại 50% phải là 45.0 độ"), InterpSnap.Rotation.Yaw, 45.0f, 0.01f);
	}

	// -------------------------------------------------------------------------
	// Test 3: AC-1 (Timestamp Guardrails - Future Rejection & 200ms Clamping)
	// -------------------------------------------------------------------------
	{
		const float ServerTime = 5.00f;
		const float MaxLag = 0.20f;
		float ClampedTime = 0.0f;

		// 3.1: Timestamp trong tương lai (T = 5.05s > ServerTime 5.00s)
		const bool bFutureValid = UPANetcodeMath::ValidateClientTimestamp(5.05f, ServerTime, MaxLag, ClampedTime);
		TestFalse(TEXT("AC-1: Timestamp trong tương lai phải bị từ chối tuyệt đối"), bFutureValid);

		// 3.2: Timestamp trễ quá 200ms (T = 4.70s, trễ 300ms so với ServerTime 5.00s)
		const bool bOldValid = UPANetcodeMath::ValidateClientTimestamp(4.70f, ServerTime, MaxLag, ClampedTime);
		TestTrue(TEXT("AC-1: Timestamp trễ quá 200ms vẫn được chấp nhận nhưng bị kẹp về giới hạn trần"), bOldValid);
		TestNearlyEqual(TEXT("AC-1: Thời gian tua lại phải bị kẹp cứng tại mốc 200ms (5.00 - 0.20 = 4.80s)"), ClampedTime, 4.80f, 0.001f);

		// 3.3: Timestamp hợp lệ trong khoảng 200ms (T = 4.85s, trễ 150ms)
		const bool bNormalValid = UPANetcodeMath::ValidateClientTimestamp(4.85f, ServerTime, MaxLag, ClampedTime);
		TestTrue(TEXT("AC-1: Timestamp hợp lệ (150ms) phải được chấp thuận"), bNormalValid);
		TestNearlyEqual(TEXT("AC-1: Thời gian không bị thay đổi và giữ nguyên 4.85s"), ClampedTime, 4.85f, 0.001f);
	}

	// -------------------------------------------------------------------------
	// Test 4: AC-1 (Melee Sweep Validation Under 150ms Simulated Latency)
	// -------------------------------------------------------------------------
	{
		// Kịch bản QA Test AC-1:
		// Target di chuyển ngang với vận tốc 600 cm/s dọc theo trục X.
		// Tại ServerTime = 2.00s: Target đang ở tọa độ X = 1200 cm.
		// Attacker ở tọa độ X = 1000 cm với tầm đánh AttackRange = 100 cm, bán kính Target = 34 cm (Tầm đánh hiệu dụng = 134 cm).
		// Attacker có ping 150ms (T_client = 1.85s).
		// Tại T = 1.85s, Target từng ở vị trí: 1200 - (600 * 0.15) = 1110 cm.

		const FVector AttackerPos(1000.0f, 0.0f, 0.0f);
		const FVector ServerCurrentTargetPos(1200.0f, 0.0f, 0.0f);
		const FVector RewoundTargetPos(1110.0f, 0.0f, 0.0f);
		const float AttackRange = 100.0f;
		const float TargetRadius = 34.0f;

		// 4.1: Nếu server không tua lại vị trí (kiểm tra theo vị trí hiện tại X = 1200):
		// Khoảng cách = 200 cm > 134 cm -> Đánh trượt
		const bool bHitWithoutRewind = UPANetcodeMath::ValidateMeleeSweep(AttackerPos, ServerCurrentTargetPos, AttackRange, TargetRadius);
		TestFalse(TEXT("AC-1: Nếu không tua lại vị trí, đòn đánh sẽ bị trượt do target đã chạy xa khỏi tầm"), bHitWithoutRewind);

		// 4.2: Khi server kích hoạt Lag Compensation Rewind về 150ms trước (X = 1110):
		// Khoảng cách = 110 cm <= 134 cm -> Xác nhận trúng đòn!
		const bool bHitWithRewind = UPANetcodeMath::ValidateMeleeSweep(AttackerPos, RewoundTargetPos, AttackRange, TargetRadius);
		TestTrue(TEXT("AC-1: Khi server tua lại 150ms theo timestamp của client, đòn đánh được xác nhận trúng đích"), bHitWithRewind);
	}

	// -------------------------------------------------------------------------
	// Test 5: AC-12 (Server Soft Reconciliation Threshold & Exponential Smoothing)
	// -------------------------------------------------------------------------
	{
		const FVector ServerPos(0.0f, 0.0f, 0.0f);

		// 5.1: Sai lệch nhỏ <= 15cm (Client ở vị trí 10cm, Server ở 0cm)
		const FVector ClientPosMinor(10.0f, 0.0f, 0.0f);
		const FPASoftReconciliationResult MinorResult = UPANetcodeMath::CalculateSoftReconciliation(ClientPosMinor, ServerPos, 0.0f);

		TestFalse(TEXT("AC-12: Sai lệch nhỏ <= 15cm không được kích hoạt hiệu chỉnh để chống camera giật"), MinorResult.bNeedsCorrection);
		TestEqual(TEXT("AC-12: Vị trí giữ nguyên theo dự đoán client"), MinorResult.SmoothedPosition, ClientPosMinor);

		// 5.2: Sai lệch lớn > 15cm (Client ở vị trí 25cm, Server ở 0cm -> lệch 25cm)
		const FVector ClientPosMajor(25.0f, 0.0f, 0.0f);

		// Tại thời điểm bắt đầu hiệu chỉnh (Elapsed = 0.0s)
		const FPASoftReconciliationResult MajorStart = UPANetcodeMath::CalculateSoftReconciliation(ClientPosMajor, ServerPos, 0.0f, 0.15f);
		TestTrue(TEXT("AC-12: Sai lệch > 15cm phải kích hoạt gói tin và cờ hiệu chỉnh"), MajorStart.bNeedsCorrection);
		TestEqual(TEXT("AC-12: Vị trí bắt đầu hiệu chỉnh tại mốc ClientPos"), MajorStart.SmoothedPosition, ClientPosMajor);

		// Tại nửa thời gian làm mịn (Elapsed = 0.075s / 0.15s)
		const FPASoftReconciliationResult MajorMid = UPANetcodeMath::CalculateSoftReconciliation(ClientPosMajor, ServerPos, 0.075f, 0.15f);
		TestNearlyEqual(TEXT("AC-12: Tại 50% thời gian blend, vị trí phải được nội suy mượt về 12.5 cm"), MajorMid.SmoothedPosition.X, 12.5f, 0.01f);

		// Khi kết thúc làm mịn (Elapsed = 0.15s)
		const FPASoftReconciliationResult MajorEnd = UPANetcodeMath::CalculateSoftReconciliation(ClientPosMajor, ServerPos, 0.15f, 0.15f);
		TestNearlyEqual(TEXT("AC-12: Khi kết thúc thời gian blend (0.15s), sai lệch vị trí phải triệt tiêu về 0 cm"), MajorEnd.SmoothedPosition.X, 0.0f, 0.01f);
	}

	// -------------------------------------------------------------------------
	// Test 6: AC-9 (PvE Ally Pass-Through vs Hostile Blocking Collision)
	// -------------------------------------------------------------------------
	{
		// 6.1: Đồng minh trong khu vực PvE
		const bool bAllyInPvE = UPANetcodeMath::EvaluateAllyCollisionPassThrough(true, true);
		TestTrue(TEXT("AC-9: Người chơi đồng minh trong khu vực PvE được phép đi xuyên thân thể (Zero Body Blocking)"), bAllyInPvE);

		// 6.2: Quái vật địch trong khu vực PvE
		const bool bEnemyInPvE = UPANetcodeMath::EvaluateAllyCollisionPassThrough(false, true);
		TestFalse(TEXT("AC-9: Quái vật/Boss trong PvE phải duy trì va chạm cản trở (Blocking Collision)"), bEnemyInPvE);

		// 6.3: Người chơi khác phe trong khu vực PvP
		const bool bPvPArena = UPANetcodeMath::EvaluateAllyCollisionPassThrough(false, false);
		TestFalse(TEXT("AC-9: Trong chế độ PvP, va chạm cản trở không được bỏ qua"), bPvPArena);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
