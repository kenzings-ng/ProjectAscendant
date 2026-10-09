// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Combat/PAThreatComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAThreatTableTest
 *
 * Kiểm thử tự động đơn vị cho Bảng Nộ Khí và Đồng Bộ Aggro Boss (Story 003 / net-003).
 *
 * Tiêu chí nghiệm thu:
 *  - AC-2: Quy tắc chuyển mục tiêu 130% (Threat Retargeting Rule). Quét mỗi 1.0s, chỉ đổi mục tiêu khi nộ >= 130%.
 *  - AC-3: Suy giảm nộ khí khi không hành động (Threat Decay). Đứng yên 3.0s, sau đó suy giảm 10% mỗi giây.
 *  - AC-11: Kỹ năng Khiêu Khích Tanker (Taunt Snap & Multiplier). Nhân 5.0x và snap lên Max(Top + 100, Caster).
 *  - AC-5: Giới hạn kéo quái (Leash Boundary Reset 2500 cm). Vượt 2500 cm chuyển sang trạng thái bất tử, xóa nộ khí.
 *  - Nguồn nộ khí: Damage 1.0x, Posture 2.5x, Healing 0.5x.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAThreatTableTest,
	"ProjectAscendant.Foundation.Combat.ContestedThreatTable",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAThreatTableTest::RunTest(const FString& Parameters)
{
	// -------------------------------------------------------------------------
	// Test 1: Hệ số nguồn nộ khí (Damage 1.0x, Posture 2.5x, Healing 0.5x)
	// -------------------------------------------------------------------------
	{
		UPAThreatComponent* ThreatComp = NewObject<UPAThreatComponent>();

		// 100 sát thương máu -> 100 Threat (1.0x)
		ThreatComp->AddThreatById(TEXT("DPS_Player"), 100.0f, EPAThreatSource::Damage);
		TestNearlyEqual(TEXT("Nguồn Damage phải nhân hệ số 1.0x"), ThreatComp->GetThreatById(TEXT("DPS_Player")), 100.0f, 0.01f);

		// 100 sát thương phá thế đứng -> 250 Threat (2.5x)
		ThreatComp->AddThreatById(TEXT("Bruiser_Player"), 100.0f, EPAThreatSource::PostureDamage);
		TestNearlyEqual(TEXT("Nguồn Posture Damage phải nhân hệ số 2.5x"), ThreatComp->GetThreatById(TEXT("Bruiser_Player")), 250.0f, 0.01f);

		// 100 điểm hồi phục máu -> 50 Threat (0.5x)
		ThreatComp->AddThreatById(TEXT("Healer_Player"), 100.0f, EPAThreatSource::Healing);
		TestNearlyEqual(TEXT("Nguồn Healing phải nhân hệ số 0.5x"), ThreatComp->GetThreatById(TEXT("Healer_Player")), 50.0f, 0.01f);

		TestEqual(TEXT("Theo dõi chính xác 3 thực thể tham chiến"), ThreatComp->GetTrackedCombatantCount(), 3);
	}

	// -------------------------------------------------------------------------
	// Test 2: AC-2 (130% Threat Retargeting Rule)
	// -------------------------------------------------------------------------
	{
		UPAThreatComponent* ThreatComp = NewObject<UPAThreatComponent>();

		// Khởi tạo mục tiêu ban đầu: Player A có 1000 Threat
		ThreatComp->AddThreatById(TEXT("Player_A"), 1000.0f, EPAThreatSource::Damage);
		TestEqual(TEXT("AC-2: Mục tiêu khởi điểm phải là Player A"), ThreatComp->GetCurrentAggroTargetId(), TEXT("Player_A"));

		// Player B gây 1200 Threat (đạt 120%, < 130%)
		ThreatComp->AddThreatById(TEXT("Player_B"), 1200.0f, EPAThreatSource::Damage);
		ThreatComp->EvaluateThreatRetargeting();

		TestEqual(TEXT("AC-2: Boss vẫn duy trì target Player A vì 1200 chỉ đạt 120% (< 130%)"),
			ThreatComp->GetCurrentAggroTargetId(), TEXT("Player_A"));

		// Player B gây thêm sát thương để đạt 1310 Threat (đạt 131% >= 130%)
		ThreatComp->AddThreatById(TEXT("Player_B"), 110.0f, EPAThreatSource::Damage); // Tổng 1310
		ThreatComp->EvaluateThreatRetargeting();

		TestEqual(TEXT("AC-2: Boss ngay lập tức chuyển mục tiêu sang Player B khi đạt 131% (> 130%)"),
			ThreatComp->GetCurrentAggroTargetId(), TEXT("Player_B"));
	}

	// -------------------------------------------------------------------------
	// Test 3: AC-3 (Threat Decay - Suy giảm nộ khí khi không hành động)
	// -------------------------------------------------------------------------
	{
		UPAThreatComponent* ThreatComp = NewObject<UPAThreatComponent>();

		// Player A tích lũy 1000 Threat tại T=0
		ThreatComp->AddThreatById(TEXT("Player_A"), 1000.0f, EPAThreatSource::Damage);

		// Trôi qua 3.0s không có hành động (đúng mốc trễ 3.0s)
		ThreatComp->TickThreatDecay(3.0f);
		TestNearlyEqual(TEXT("AC-3: Tại mốc T=3.0s nộ khí vẫn duy trì trọn vẹn 1000 điểm"),
			ThreatComp->GetThreatById(TEXT("Player_A")), 1000.0f, 0.01f);

		// Trôi qua thêm 1.0s (T=4.0s) -> suy giảm 10% mỗi giây
		ThreatComp->TickThreatDecay(1.0f);
		TestNearlyEqual(TEXT("AC-3: Tại mốc T=4.0s nộ khí suy giảm 10% còn lại 900 điểm"),
			ThreatComp->GetThreatById(TEXT("Player_A")), 900.0f, 0.5f);
	}

	// -------------------------------------------------------------------------
	// Test 4: AC-11 (Tank Taunt Mechanic & Instant Target Snap)
	// -------------------------------------------------------------------------
	{
		UPAThreatComponent* ThreatComp = NewObject<UPAThreatComponent>();

		// Mage gây 5000 Threat, trở thành mục tiêu chính
		ThreatComp->AddThreatById(TEXT("Mage_DPS"), 5000.0f, EPAThreatSource::Damage);
		TestEqual(TEXT("AC-11: Mục tiêu ban đầu là Mage"), ThreatComp->GetCurrentAggroTargetId(), TEXT("Mage_DPS"));

		// Vanguard chỉ có 500 Threat ban đầu
		ThreatComp->AddThreatById(TEXT("Tank_Vanguard"), 500.0f, EPAThreatSource::Damage);

		// Vanguard kích hoạt kỹ năng Taunt (Khiêu Khích)
		ThreatComp->ApplyTauntById(TEXT("Tank_Vanguard"), 0.0f);

		// Snap lên Max(Top + 100, Caster) = Max(5000 + 100, 500) = 5100 Threat
		TestNearlyEqual(TEXT("AC-11: Nộ khí Tanker phải snap tức thì lên 5100 điểm (Top + 100)"),
			ThreatComp->GetThreatById(TEXT("Tank_Vanguard")), 5100.0f, 0.01f);

		// Ép buộc boss chuyển mục tiêu sang Vanguard ngay lập tức
		TestEqual(TEXT("AC-11: Boss lập tức chuyển mục tiêu công kích sang Tanker Vanguard"),
			ThreatComp->GetCurrentAggroTargetId(), TEXT("Tank_Vanguard"));
	}

	// -------------------------------------------------------------------------
	// Test 5: AC-5 (Leash Boundary Reset & Invulnerability - Cự ly 2500 cm)
	// -------------------------------------------------------------------------
	{
		UPAThreatComponent* ThreatComp = NewObject<UPAThreatComponent>();
		ThreatComp->SetSpawnOrigin(FVector(0.0f, 0.0f, 0.0f));

		ThreatComp->AddThreatById(TEXT("Player_Runner"), 2000.0f, EPAThreatSource::Damage);
		TestEqual(TEXT("AC-5: Player_Runner đang là mục tiêu"), ThreatComp->GetCurrentAggroTargetId(), TEXT("Player_Runner"));
		TestFalse(TEXT("AC-5: Trạng thái bình thường không bất tử"), ThreatComp->IsInvulnerable());

		// Boss bị kéo đến tọa độ 2400 cm (< 2500 cm) -> Chưa reset
		const bool bLeashTriggeredNear = ThreatComp->CheckLeashDistance(FVector(2400.0f, 0.0f, 0.0f));
		TestFalse(TEXT("AC-5: Khoảng cách 2400 cm chưa chạm ngưỡng 2500 cm"), bLeashTriggeredNear);
		TestFalse(TEXT("AC-5: Boss vẫn giao tranh bình thường"), ThreatComp->IsInvulnerable());

		// Boss bị kéo vượt quá 2500 cm (2550 cm) -> Kích hoạt Leash Reset
		const bool bLeashTriggeredFar = ThreatComp->CheckLeashDistance(FVector(2550.0f, 0.0f, 0.0f));
		TestTrue(TEXT("AC-5: Khoảng cách 2550 cm kích hoạt Leash Boundary Reset"), bLeashTriggeredFar);
		TestTrue(TEXT("AC-5: Boss chuyển sang trạng thái Bất Tử (Invulnerable) khi hồi vị trí"), ThreatComp->IsInvulnerable());
		TestEqual(TEXT("AC-5: Bảng nộ khí bị xóa sạch về 0 đối thủ"), ThreatComp->GetTrackedCombatantCount(), 0);
		TestTrue(TEXT("AC-5: Mục tiêu bị hủy bỏ"), ThreatComp->GetCurrentAggroTargetId().IsEmpty());

		// Thử tấn công Boss trong lúc đang chạy về gốc -> Không nhận thêm nộ khí
		ThreatComp->AddThreatById(TEXT("Player_Runner"), 500.0f, EPAThreatSource::Damage);
		TestEqual(TEXT("AC-5: Không thể gây nộ khí hay sát thương khi boss đang reset"), ThreatComp->GetTrackedCombatantCount(), 0);

		// Khi quay trở về điểm Spawn (< 50cm) -> Kết thúc Reset
		ThreatComp->CompleteLeashReset();
		TestFalse(TEXT("AC-5: Khi về lại điểm Spawn, Boss thoát trạng thái bất tử"), ThreatComp->IsInvulnerable());
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
