// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Network/PAIrisSpatialFilter.h"
#include "Network/PAGhostBodySubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAIrisGhostBodyTest
 *
 * Kiểm thử tự động đơn vị cho Hệ thống Iris Dynamic Spatial Prioritization
 * và Cơ chế Xác Ma (Ghost Body) chống rút dây mạng khi đang chiến đấu (Story 002 / net-002).
 *
 * Tiêu chí nghiệm thu:
 *  - AC-10: Đồng bộ mạng Iris 3 cấp độ: Cự ly <= 15m đạt 60Hz; 15m - 35m giảm tải còn 30Hz; > 35m chuyển sang ngủ đông (Dormant / Cull 0Hz).
 *  - AC-4: Rớt mạng khi ngoài giao tranh được logout ngay (0s); Rớt mạng khi đang có tag InCombat biến thành Xác Ma tồn tại đúng 15.0s,
 *          vẫn nhận sát thương và cho phép Reconnect liền mạch trong 15s.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAIrisGhostBodyTest,
	"ProjectAscendant.Foundation.Netcode.IrisSpatialAndGhostBody",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAIrisGhostBodyTest::RunTest(const FString& Parameters)
{
	// -------------------------------------------------------------------------
	// Test 1: AC-10 (Iris Tier 1 - Cận Chiến Cao Tần 60Hz, <= 1500 cm)
	// -------------------------------------------------------------------------
	{
		const FVector ObserverPos(0.0f, 0.0f, 0.0f);
		const FVector TargetNear(1000.0f, 0.0f, 0.0f); // 10m = 1000 cm <= 1500 cm

		const EPAIrisSpatialTier Tier = UPAIrisSpatialFilter::EvaluateSpatialTier(ObserverPos, TargetNear);
		TestEqual(TEXT("AC-10: Khoảng cách 10m phải thuộc Cấp 1 (Tier 1 High Frequency)"), Tier, EPAIrisSpatialTier::Tier1_HighFrequency);

		const float Freq = UPAIrisSpatialFilter::GetReplicationFrequencyForDistance(1000.0f);
		TestEqual(TEXT("AC-10: Tần số đồng bộ tại 10m phải là 60Hz"), Freq, 60.0f);

		const bool bCull = UPAIrisSpatialFilter::ShouldCullReplication(1000.0f);
		TestFalse(TEXT("AC-10: Cự ly 10m không được phép cull"), bCull);

		const float Interval = UPAIrisSpatialFilter::CalculateReplicationInterval(Freq);
		TestNearlyEqual(TEXT("AC-10: Chu kỳ gửi gói tin 60Hz là ~0.0166s"), Interval, 1.0f / 60.0f, 0.001f);
	}

	// -------------------------------------------------------------------------
	// Test 2: AC-10 (Iris Tier 2 - Tầm Trung Giảm Tải 30Hz, 1500 - 3500 cm)
	// -------------------------------------------------------------------------
	{
		const FVector ObserverPos(0.0f, 0.0f, 0.0f);
		const FVector TargetMid(2500.0f, 0.0f, 0.0f); // 25m = 2500 cm (trong khoảng 15m - 35m)

		const EPAIrisSpatialTier Tier = UPAIrisSpatialFilter::EvaluateSpatialTier(ObserverPos, TargetMid);
		TestEqual(TEXT("AC-10: Khoảng cách 25m phải thuộc Cấp 2 (Tier 2 Mid Frequency)"), Tier, EPAIrisSpatialTier::Tier2_MidFrequency);

		const float Freq = UPAIrisSpatialFilter::GetReplicationFrequencyForDistance(2500.0f);
		TestEqual(TEXT("AC-10: Tần số đồng bộ tại 25m phải giảm tải còn 30Hz"), Freq, 30.0f);

		const bool bCull = UPAIrisSpatialFilter::ShouldCullReplication(2500.0f);
		TestFalse(TEXT("AC-10: Cự ly 25m vẫn được đồng bộ mượt mà, không cull"), bCull);
	}

	// -------------------------------------------------------------------------
	// Test 3: AC-10 (Iris Tier 3 - Ngủ Đông / Culling, > 3500 cm)
	// -------------------------------------------------------------------------
	{
		const FVector ObserverPos(0.0f, 0.0f, 0.0f);
		const FVector TargetFar(4500.0f, 0.0f, 0.0f); // 45m = 4500 cm > 3500 cm

		const EPAIrisSpatialTier Tier = UPAIrisSpatialFilter::EvaluateSpatialTier(ObserverPos, TargetFar);
		TestEqual(TEXT("AC-10: Khoảng cách > 35m phải chuyển sang Cấp 3 (Tier 3 Dormant)"), Tier, EPAIrisSpatialTier::Tier3_Dormant);

		const float Freq = UPAIrisSpatialFilter::GetReplicationFrequencyForDistance(4500.0f);
		TestEqual(TEXT("AC-10: Tần số đồng bộ tại 45m phải đưa về 0Hz"), Freq, 0.0f);

		const bool bCull = UPAIrisSpatialFilter::ShouldCullReplication(4500.0f);
		TestTrue(TEXT("AC-10: Cự ly ngoài 35m phải được kích hoạt culling chống nghẽn mạng"), bCull);
	}

	// -------------------------------------------------------------------------
	// Test 4: AC-4 (Rút Mạng Ngoài Giao Tranh - Cho Phép Thoát Game Tức Thì)
	// -------------------------------------------------------------------------
	{
		UPAGhostBodySubsystem* GhostSubsystem = NewObject<UPAGhostBodySubsystem>();
		GhostSubsystem->Initialize(*static_cast<FSubsystemCollectionBase*>(nullptr));

		const FString SafePlayerId = TEXT("Player_Peaceful_01");
		const bool bInCombat = false;

		// Ngắt kết nối khi không trong giao tranh
		const bool bGhostCreated = GhostSubsystem->RegisterDisconnect(SafePlayerId, bInCombat);
		TestFalse(TEXT("AC-4: Ngắt kết nối ngoài giao tranh không tạo Xác Ma"), bGhostCreated);
		TestFalse(TEXT("AC-4: Không có Xác Ma nào hoạt động cho người chơi này"), GhostSubsystem->IsGhostActive(SafePlayerId));
		TestEqual(TEXT("AC-4: Tổng số Xác Ma trên Server phải là 0"), GhostSubsystem->GetActiveGhostCount(), 0);
	}

	// -------------------------------------------------------------------------
	// Test 5: AC-4 (Rút Mạng Trong Giao Tranh - Duy Trì Xác Ma Đúng 15.0 Giây)
	// -------------------------------------------------------------------------
	{
		UPAGhostBodySubsystem* GhostSubsystem = NewObject<UPAGhostBodySubsystem>();
		GhostSubsystem->Initialize(*static_cast<FSubsystemCollectionBase*>(nullptr));

		const FString CombatPlayerId = TEXT("Player_Combat_99");
		const bool bInCombat = true;

		// Ngắt kết nối khi đang combat
		const bool bGhostCreated = GhostSubsystem->RegisterDisconnect(CombatPlayerId, bInCombat, 15.0f);
		TestTrue(TEXT("AC-4: Ngắt kết nối khi đang trong giao tranh phải kích hoạt Xác Ma"), bGhostCreated);
		TestTrue(TEXT("AC-4: Xác Ma phải ở trạng thái hoạt động"), GhostSubsystem->IsGhostActive(CombatPlayerId));
		TestEqual(TEXT("AC-4: Thời gian duy trì Xác Ma ban đầu phải là 15.0 giây"), GhostSubsystem->GetRemainingGhostTime(CombatPlayerId), 15.0f);

		// Trôi qua 5.0 giây
		const bool bExpiredEarly = GhostSubsystem->TickGhostLifespan(CombatPlayerId, 5.0f);
		TestFalse(TEXT("AC-4: Sau 5s Xác Ma chưa được hết hạn"), bExpiredEarly);
		TestNearlyEqual(TEXT("AC-4: Thời gian còn lại phải là 10.0s (15 - 5)"), GhostSubsystem->GetRemainingGhostTime(CombatPlayerId), 10.0f, 0.001f);
	}

	// -------------------------------------------------------------------------
	// Test 6: AC-4 (Kết Nối Lại Thành Công Trong Vòng 15 Giây - Reconnect)
	// -------------------------------------------------------------------------
	{
		UPAGhostBodySubsystem* GhostSubsystem = NewObject<UPAGhostBodySubsystem>();
		GhostSubsystem->Initialize(*static_cast<FSubsystemCollectionBase*>(nullptr));

		const FString ReconnectingPlayerId = TEXT("Player_Reconnect_42");
		GhostSubsystem->RegisterDisconnect(ReconnectingPlayerId, true, 15.0f);

		// Sau 7 giây người chơi mở lại game và kết nối lại
		GhostSubsystem->TickGhostLifespan(ReconnectingPlayerId, 7.0f);

		const bool bReconnected = GhostSubsystem->HandleReconnect(ReconnectingPlayerId);
		TestTrue(TEXT("AC-4: Reconnect trong 15s phải thành công liền mạch"), bReconnected);
		TestFalse(TEXT("AC-4: Sau khi Reconnect, Xác Ma được hủy bỏ để người chơi điều khiển lại"), GhostSubsystem->IsGhostActive(ReconnectingPlayerId));
		TestEqual(TEXT("AC-4: Danh sách Xác Ma trở về 0"), GhostSubsystem->GetActiveGhostCount(), 0);
	}

	// -------------------------------------------------------------------------
	// Test 7: AC-4 (Hết Hạn 15 Giây Hoặc Bị Tiêu Diệt Trong Lúc Rớt Mạng)
	// -------------------------------------------------------------------------
	{
		UPAGhostBodySubsystem* GhostSubsystem = NewObject<UPAGhostBodySubsystem>();
		GhostSubsystem->Initialize(*static_cast<FSubsystemCollectionBase*>(nullptr));

		// 7.1: Hết hạn 15s -> Giải phóng an toàn
		const FString TimeoutPlayerId = TEXT("Player_Timeout_01");
		GhostSubsystem->RegisterDisconnect(TimeoutPlayerId, true, 15.0f);

		const bool bExpired = GhostSubsystem->TickGhostLifespan(TimeoutPlayerId, 15.0f);
		TestTrue(TEXT("AC-4: Khi chạm mốc 15.0s, Xác Ma phải được giải phóng"), bExpired);
		TestFalse(TEXT("AC-4: Sau 15s Xác Ma không còn hoạt động"), GhostSubsystem->IsGhostActive(TimeoutPlayerId));

		// 7.2: Bị quái vật đánh chết trong 15s rớt mạng
		const FString SlainPlayerId = TEXT("Player_Slain_02");
		GhostSubsystem->RegisterDisconnect(SlainPlayerId, true, 15.0f);

		const bool bKilled = GhostSubsystem->ApplyLethalDamageToGhost(SlainPlayerId);
		TestTrue(TEXT("AC-4: Sát thương chí mạng áp dụng thành công lên Xác Ma"), bKilled);
		TestFalse(TEXT("AC-4: Xác Ma bị tiêu diệt không còn trong danh sách hoạt động"), GhostSubsystem->IsGhostActive(SlainPlayerId));

		// Thử Reconnect sau khi đã bị đánh chết -> Thất bại, phải hồi sinh
		const bool bReconnectAfterDead = GhostSubsystem->HandleReconnect(SlainPlayerId);
		TestFalse(TEXT("AC-4: Không thể Reconnect vào nhân vật đã tử vong"), bReconnectAfterDead);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
