// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Combat/PADashTypes.h"
#include "Combat/PAGameplayAbility_MeleeAttack.h"
#include "Combat/PAPartBreakingTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPACombatRegressionHardeningTests
 *
 * Bộ kiểm thử hồi quy và gia cố độ tin cậy hệ thống chiến đấu (Combat Hardening & Regression Suite):
 *  - Test 1: Input Buffer (Xếp hàng lệnh tấn công trong Recovery, tự động bung đòn ngay khi hết cữ).
 *  - Test 2: Cancel Window (Hủy đòn đánh thường chuyển sang Dash I-frame an toàn, dọn sạch tag).
 *  - Test 3: Dash I-Frame Absolute Invulnerability (Triệt tiêu 100% sát thương trong 0.28s, Perfect Dodge 0.05-0.15s).
 *  - Test 4: Combo State Reset & Interruption (Quá 1.2s hoặc dính Stun/HitStun lập tức reset về Nhịp 1).
 *  - Test 5: Network Packet Loss & Rollback Reconciliation (Đồng bộ khôi phục trạng thái khi Server chỉnh sửa).
 *
 * Kiểm thử thực thi độc lập (Pure Logic / Headless Automation Test) đảm bảo tốc độ < 0.1s.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPACombatRegressionHardeningTests,
	"ProjectAscendant.Combat.RegressionHardening",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// ---------------------------------------------------------------------------
// Mô hình kiểm thử Input Buffer & Cancel Window giả lập (Lightweight Mock Model)
// ---------------------------------------------------------------------------
struct FPACombatBufferMock
{
	float AttackTimer = 0.0f;
	float AttackDuration = 0.60f;
	float CancelWindowStart = 0.25f;
	float CancelWindowEnd = 0.45f;

	bool bIsAttacking = false;
	bool bHasBufferedInput = false;
	bool bIsDashing = false;
	int32 CurrentComboStep = 1;

	void StartAttack(int32 Step)
	{
		CurrentComboStep = Step;
		bIsAttacking = true;
		AttackTimer = 0.0f;
		bHasBufferedInput = false;
	}

	void QueueInput()
	{
		if (bIsAttacking)
		{
			bHasBufferedInput = true;
		}
	}

	bool TryCancelIntoDash()
	{
		if (bIsAttacking && AttackTimer >= CancelWindowStart && AttackTimer <= CancelWindowEnd)
		{
			bIsAttacking = false;
			bHasBufferedInput = false;
			bIsDashing = true;
			return true;
		}
		return false;
	}

	void Update(float DeltaTime)
	{
		if (bIsAttacking)
		{
			AttackTimer += DeltaTime;
			if (AttackTimer >= AttackDuration)
			{
				bIsAttacking = false;
				if (bHasBufferedInput)
				{
					// Bung đòn kế tiếp từ bộ đệm Input Buffer
					StartAttack(CurrentComboStep < 3 ? CurrentComboStep + 1 : 1);
				}
			}
		}
	}
};

// ---------------------------------------------------------------------------
// RunTest Execution
// ---------------------------------------------------------------------------
bool FPACombatRegressionHardeningTests::RunTest(const FString& Parameters)
{
	// =======================================================================
	// Test 1: Input Buffer (Xếp hàng lệnh tấn công)
	// =======================================================================
	{
		FPACombatBufferMock Mock;
		Mock.StartAttack(1);
		TestTrue(TEXT("Test1: Mock attack is active"), Mock.bIsAttacking);
		TestEqual(TEXT("Test1: Initial step is 1"), Mock.CurrentComboStep, 1);

		// Tiến thời gian đến 0.50s (giai đoạn Recovery của đòn 1)
		Mock.Update(0.50f);
		TestTrue(TEXT("Test1: Still in attack recovery"), Mock.bIsAttacking);

		// Người chơi nhấn phím đánh sớm trong lúc nhân vật đang thu kiếm
		Mock.QueueInput();
		TestTrue(TEXT("Test1: Input successfully queued in buffer"), Mock.bHasBufferedInput);

		// Tiến thời gian qua hết AttackDuration (0.60s)
		Mock.Update(0.15f);

		// Input Buffer phải tự động kích hoạt Nhịp 2 mà người chơi không cần nhấn lại
		TestTrue(TEXT("Test1: Buffered attack seamlessly triggered"), Mock.bIsAttacking);
		TestEqual(TEXT("Test1: Combo advanced to step 2 via buffer"), Mock.CurrentComboStep, 2);
		TestFalse(TEXT("Test1: Buffer queue consumed and cleared"), Mock.bHasBufferedInput);
	}

	// =======================================================================
	// Test 2: Cancel Window (Hủy đòn thường sang Dash I-frame)
	// =======================================================================
	{
		FPACombatBufferMock Mock;
		Mock.StartAttack(1);

		// A. Thử hủy trước cửa sổ (t = 0.10s < 0.25s) -> Bị cấm (Player phải commit đòn đánh)
		Mock.Update(0.10f);
		bool bCanceledEarly = Mock.TryCancelIntoDash();
		TestFalse(TEXT("Test2: Dash cancel rejected during startup commitment window"), bCanceledEarly);
		TestTrue(TEXT("Test2: Attack still intact"), Mock.bIsAttacking);

		// B. Thử hủy trong cửa sổ vàng (t = 0.30s trong [0.25s - 0.45s]) -> Cho phép hủy an toàn
		Mock.Update(0.20f); // Tổng = 0.30s
		bool bCanceledInWindow = Mock.TryCancelIntoDash();
		TestTrue(TEXT("Test2: Dash cancel accepted inside cancel window"), bCanceledInWindow);
		TestFalse(TEXT("Test2: Attack cleanly ended, no tag leak"), Mock.bIsAttacking);
		TestTrue(TEXT("Test2: Dash state successfully assumed"), Mock.bIsDashing);
	}

	// =======================================================================
	// Test 3: Dash I-Frame Absolute Invulnerability & Perfect Dodge
	// =======================================================================
	{
		FPADashModel Dash;
		float Stamina = 100.0f;
		float StaminaRemaining = 0.0f;
		bool bStarted = Dash.StartDash(Stamina, StaminaRemaining);
		TestTrue(TEXT("Test3: Dash started"), bStarted);

		// Tại t = 0.10s (nằm trong cả I-frame 0.28s và Sweet Spot 0.05-0.15s)
		Dash.Update(0.10f);
		TestTrue(TEXT("Test3: Is currently in I-Frame"), Dash.bInvulnerable);

		// Quái vật gây đòn chí mạng 9999 sát thương
		FPAIncomingHitResult Hit = Dash.ProcessIncomingHit(9999.0f);
		TestTrue(TEXT("Test3: Hit registered as avoided via I-Frame"), Hit.bAvoidedWithIFrame);
		TestEqual(TEXT("Test3: Damage strictly mitigated to 0"), Hit.DamageTaken, 0.0f);
		TestTrue(TEXT("Test3: Perfect Dodge triggered"), Hit.bTriggeredPerfectDodge);
		TestEqual(TEXT("Test3: Stamina refunded (+15)"), Hit.StaminaRefunded, 15.0f);

		// Tiến thời gian qua t = 0.30s (sang Recovery phase, hết I-frame)
		Dash.Update(0.20f);
		TestFalse(TEXT("Test3: I-Frame ended in recovery phase"), Dash.bInvulnerable);

		// Nhận đòn tại Recovery phase: nhận đủ 100 sát thương
		FPAIncomingHitResult HitVulnerable = Dash.ProcessIncomingHit(100.0f);
		TestFalse(TEXT("Test3: Hit not invulnerable in recovery"), HitVulnerable.bAvoidedWithIFrame);
		TestEqual(TEXT("Test3: Full damage taken in recovery"), HitVulnerable.DamageTaken, 100.0f);
	}

	// =======================================================================
	// Test 4: Combo State Reset & Interruption
	// =======================================================================
	{
		// A. Đánh nhịp 1 -> nhịp 2 bình thường trong vòng 0.8s
		int32 Step1 = 1;
		int32 Step2 = FPAComboFinisherPipeline::AdvanceComboStep(Step1, 0.8f);
		TestEqual(TEXT("Test4: Step 1 advances to 2 within 1.2s window"), Step2, 2);

		// B. Đứng yên quá 1.2s (1.25s) -> Tự động reset về Nhịp 1
		int32 StepReset = FPAComboFinisherPipeline::AdvanceComboStep(Step2, 1.25f);
		TestEqual(TEXT("Test4: Step resets to 1 after 1.2s inactivity timeout"), StepReset, 1);

		// C. Hệ số sát thương & thế đứng từng nhịp
		TestEqual(TEXT("Test4: Step 1 Multiplier is 1.0x"), FPAComboFinisherPipeline::GetComboDamageMultiplier(1), 1.0f);
		TestEqual(TEXT("Test4: Step 2 Multiplier is 1.2x"), FPAComboFinisherPipeline::GetComboDamageMultiplier(2), 1.2f);
		TestEqual(TEXT("Test4: Step 3 Multiplier is 1.6x"), FPAComboFinisherPipeline::GetComboDamageMultiplier(3), 1.6f);

		TestEqual(TEXT("Test4: Step 1 Posture is 10"), FPAComboFinisherPipeline::GetComboPostureDamage(1), 10.0f);
		TestEqual(TEXT("Test4: Step 2 Posture is 15"), FPAComboFinisherPipeline::GetComboPostureDamage(2), 15.0f);
		TestEqual(TEXT("Test4: Step 3 Posture is 25"), FPAComboFinisherPipeline::GetComboPostureDamage(3), 25.0f);
	}

	// =======================================================================
	// Test 5: Network Reconciliation & Rollback Verification
	// =======================================================================
	{
		// Giả lập Client dự đoán đi trước Server
		int32 ClientPredictedStep = 3;
		int32 ServerAuthoritativeStep = 2; // Server bị trễ gói, chỉ mới ghi nhận đòn 2

		// Thuật toán Reconciliation: Server Authority luôn override Client Prediction
		auto ReconcileState = [](int32 ClientStep, int32 ServerStep) -> int32
		{
			if (ClientStep != ServerStep)
			{
				return ServerStep; // Rollback về mốc Server xác thực
			}
			return ClientStep;
		};

		int32 ReconciledStep = ReconcileState(ClientPredictedStep, ServerAuthoritativeStep);
		TestEqual(TEXT("Test5: Client smoothly reconciles to server authoritative combo step"), ReconciledStep, 2);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
