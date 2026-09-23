// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UI/PAPlayerVitalsTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAPlayerVitalsTests
 *
 * Kiểm thử tự động cho Story hud-001 (Player Vitals & Status HUD Engine):
 *  - AC-1: GAS Attribute Binding — ánh xạ chỉ số sang phần trăm tức thì.
 *  - AC-2: Catch-Up Ghost Bar — trễ 0.40s, nội suy tốc độ 3.5, hồi máu bám sát.
 *  - AC-3: Golden Flash + Exhaustion — lóe vàng 0.20s, trạng thái kiệt sức.
 *  - AC-4: Low Health Vignette + Heartbeat — viền đỏ < 20% HP, BPM 60→100.
 *
 * Tất cả test chạy trên FPAVitalsModel thuần túy (không UMG).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAPlayerVitalsTests,
	"ProjectAscendant.UI.PlayerVitals",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
	/** Sai số tương đối cho phép ở phép so sánh dấu phẩy động */
	constexpr float kTolerance = 0.01f;

	/** Giả lập nhiều tick liên tục cho Model.Update() */
	void SimulateTicks(FPAVitalsModel& Model, float TotalTime, float StepSize = 0.016f)
	{
		float Elapsed = 0.0f;
		while (Elapsed < TotalTime)
		{
			const float Dt = FMath::Min(StepSize, TotalTime - Elapsed);
			Model.Update(Dt);
			Elapsed += Dt;
		}
	}
}

// ---------------------------------------------------------------------------
// RunTest
// ---------------------------------------------------------------------------

bool FPAPlayerVitalsTests::RunTest(const FString& Parameters)
{
	// ===========================================================
	// AC-1: GAS Attribute Binding — Phần trăm tính chính xác
	// ===========================================================
	{
		FPAVitalsModel Model;

		// QA Test 1: MaxHealth 100, Health 80 → 0.80
		Model.SetHealth(80.0f, 100.0f);
		TestNearlyEqual(TEXT("AC1: HealthPercent 80/100 = 0.80"), Model.GetHealthPercent(), 0.80f, kTolerance);

		// Stamina 60/200 → 0.30
		Model.SetStamina(60.0f, 200.0f);
		TestNearlyEqual(TEXT("AC1: StaminaPercent 60/200 = 0.30"), Model.GetStaminaPercent(), 0.30f, kTolerance);

		// Mana 0/100 → 0.0
		Model.SetMana(0.0f, 100.0f);
		TestNearlyEqual(TEXT("AC1: ManaPercent 0/100 = 0.0"), Model.GetManaPercent(), 0.0f, kTolerance);

		// Edge: MaxHealth = 0 → phòng chia 0 → 0.0
		Model.SetHealth(50.0f, 0.0f);
		// MaxHealth clamped to 1.0f, so 50/1 → clamped to 1.0
		// Actually: SetHealth clamps MaxHealth = Max(1, 0) = 1, CurrentHealth = Clamp(50, 0, 1) = 1.0
		TestNearlyEqual(TEXT("AC1: MaxHealth=0 guard → percent 1.0"), Model.GetHealthPercent(), 1.0f, kTolerance);

		// Full health → 1.0
		Model.SetHealth(100.0f, 100.0f);
		TestNearlyEqual(TEXT("AC1: Full health → 1.0"), Model.GetHealthPercent(), 1.0f, kTolerance);
	}

	// ===========================================================
	// AC-2: Catch-Up Ghost Bar
	// ===========================================================
	{
		FPAVitalsModel Model;
		Model.SetHealth(100.0f, 100.0f);

		// --- Mất máu: ghost giữ vị trí cũ trong 0.40s ---
		Model.SetHealth(50.0f, 100.0f);
		TestNearlyEqual(TEXT("AC2: Health drops immediately to 0.50"), Model.GetHealthPercent(), 0.50f, kTolerance);
		TestNearlyEqual(TEXT("AC2: Ghost retains old position 1.0"), Model.GetGhostHealthPercent(), 1.0f, kTolerance);

		// QA Test 2: Tại t = 0.20s ghost vẫn giữ nguyên (delay 0.40s chưa hết)
		SimulateTicks(Model, 0.20f);
		TestNearlyEqual(TEXT("AC2: Ghost at t=0.20s still ~1.0 (delay)"), Model.GetGhostHealthPercent(), 1.0f, kTolerance);

		// Tại t = 0.40s delay vừa hết, ghost bắt đầu di chuyển nhưng chưa xa
		SimulateTicks(Model, 0.20f);
		// GhostDelayTimer should be 0 now, interpolation just started
		TestTrue(TEXT("AC2: Ghost at t=0.40s > 0.50 (starting interp)"), Model.GetGhostHealthPercent() > 0.50f);

		// QA Test 2 (continued): Tại t = 0.50s+ ghost đang nội suy
		// After delay expires, ghost should start approaching 0.50
		float GhostBefore = Model.GetGhostHealthPercent();
		SimulateTicks(Model, 0.10f);
		float GhostAfter = Model.GetGhostHealthPercent();
		TestTrue(TEXT("AC2: Ghost moving toward target (decreasing)"), GhostAfter < GhostBefore);

		// Sau đủ lâu (2 giây thêm) ghost gần bằng health thực tế
		SimulateTicks(Model, 2.0f);
		TestNearlyEqual(TEXT("AC2: Ghost converges to HealthPercent"), Model.GetGhostHealthPercent(), Model.GetHealthPercent(), 0.05f);

		// --- Hồi máu: ghost bám sát ngay lập tức ---
		Model.SetHealth(80.0f, 100.0f);
		TestNearlyEqual(TEXT("AC2: Healing → ghost matches immediately"), Model.GetGhostHealthPercent(), 0.80f, kTolerance);
	}

	// ===========================================================
	// AC-3: Stamina State Feedback & Golden Flash
	// ===========================================================
	{
		// --- QA Test 3: Perfect Dodge Golden Flash ---
		FPAVitalsModel Model;
		Model.SetStamina(50.0f, 100.0f);

		Model.TriggerPerfectDodge();
		TestEqual(TEXT("AC3: PerfectDodge → GoldenFlash state"), Model.StaminaState, EPAStaminaBarState::GoldenFlash);

		// Stamina refund +15 → 65/100
		TestNearlyEqual(TEXT("AC3: Stamina refunded to 65"), Model.CurrentStamina, 65.0f, kTolerance);

		// Sau 0.10s flash vẫn đang chạy
		SimulateTicks(Model, 0.10f);
		TestEqual(TEXT("AC3: Flash still active at t=0.10s"), Model.StaminaState, EPAStaminaBarState::GoldenFlash);

		// Sau tổng 0.20s+ flash kết thúc, trở về Normal
		SimulateTicks(Model, 0.11f);
		TestEqual(TEXT("AC3: Flash ends → Normal"), Model.StaminaState, EPAStaminaBarState::Normal);

		// --- Stamina refund ở mốc gần max: clamp 100 ---
		FPAVitalsModel Model2;
		Model2.SetStamina(95.0f, 100.0f);
		Model2.TriggerPerfectDodge();
		TestNearlyEqual(TEXT("AC3: Stamina refund clamped at max"), Model2.CurrentStamina, 100.0f, kTolerance);

		// --- QA Test 5: Exhaustion Tag ---
		FPAVitalsModel ModelExh;
		ModelExh.SetExhausted(true);
		TestEqual(TEXT("AC3: Exhaustion → Exhausted state"), ModelExh.StaminaState, EPAStaminaBarState::Exhausted);
		TestTrue(TEXT("AC3: ExhaustionVignette active"), ModelExh.bExhaustionVignetteActive);

		// Perfect Dodge bị chặn khi kiệt sức
		ModelExh.SetStamina(50.0f, 100.0f);
		ModelExh.TriggerPerfectDodge();
		TestEqual(TEXT("AC3: PerfectDodge blocked during Exhausted"), ModelExh.StaminaState, EPAStaminaBarState::Exhausted);
		TestNearlyEqual(TEXT("AC3: No stamina refund during Exhausted"), ModelExh.CurrentStamina, 50.0f, kTolerance);

		// Hết kiệt sức → Normal
		ModelExh.SetExhausted(false);
		TestEqual(TEXT("AC3: Exhaustion cleared → Normal"), ModelExh.StaminaState, EPAStaminaBarState::Normal);
		TestFalse(TEXT("AC3: ExhaustionVignette cleared"), ModelExh.bExhaustionVignetteActive);

		// --- Golden Flash hết trong khi Exhausted → về Exhausted (không Normal) ---
		FPAVitalsModel ModelOverlap;
		ModelOverlap.TriggerPerfectDodge();
		TestEqual(TEXT("AC3: Flash started"), ModelOverlap.StaminaState, EPAStaminaBarState::GoldenFlash);
		ModelOverlap.SetExhausted(true); // áp dụng Exhausted giữa flash
		TestEqual(TEXT("AC3: Exhausted overrides flash"), ModelOverlap.StaminaState, EPAStaminaBarState::Exhausted);
	}

	// ===========================================================
	// AC-4: Low Health Vignette & Heartbeat BPM
	// ===========================================================
	{
		FPAVitalsModel Model;

		// Máu đầy → không vignette
		Model.SetHealth(100.0f, 100.0f);
		TestFalse(TEXT("AC4: Full health → no vignette"), Model.bLowHealthVignetteActive);
		TestNearlyEqual(TEXT("AC4: Full health → BPM 0"), Model.CurrentHeartbeatBPM, 0.0f, kTolerance);

		// Tại ngưỡng 20% → vừa trên ngưỡng → không kích hoạt (>=20%)
		Model.SetHealth(20.0f, 100.0f);
		TestFalse(TEXT("AC4: At exactly 20% → no vignette (not <)"), Model.bLowHealthVignetteActive);

		// QA Test 4: Máu 10/100 = 10% < 20% → kích hoạt
		// BPM = 60 + 40 * (1 - 10 / (0.20 * 100)) = 60 + 40 * (1 - 0.50) = 60 + 20 = 80
		Model.SetHealth(10.0f, 100.0f);
		TestTrue(TEXT("AC4: 10% health → vignette active"), Model.bLowHealthVignetteActive);
		TestNearlyEqual(TEXT("AC4: BPM at 10% health = 80"), Model.CurrentHeartbeatBPM, 80.0f, kTolerance);

		// Máu 1/100 = 1% → BPM gần max
		// BPM = 60 + 40 * (1 - 1/20) = 60 + 40 * 0.95 = 98
		Model.SetHealth(1.0f, 100.0f);
		TestTrue(TEXT("AC4: 1% health → vignette active"), Model.bLowHealthVignetteActive);
		TestNearlyEqual(TEXT("AC4: BPM at 1% health = 98"), Model.CurrentHeartbeatBPM, 98.0f, kTolerance);

		// Máu 0/100 = 0% → vignette tắt (chết, không cần nhịp tim)
		Model.SetHealth(0.0f, 100.0f);
		TestFalse(TEXT("AC4: 0% health → vignette off (dead)"), Model.bLowHealthVignetteActive);
		TestNearlyEqual(TEXT("AC4: BPM at 0% = 0 (dead)"), Model.CurrentHeartbeatBPM, 0.0f, kTolerance);

		// Hồi máu vượt ngưỡng → vignette tắt
		Model.SetHealth(5.0f, 100.0f);
		TestTrue(TEXT("AC4: Low health → vignette on"), Model.bLowHealthVignetteActive);
		Model.SetHealth(30.0f, 100.0f);
		TestFalse(TEXT("AC4: Healed above 20% → vignette off"), Model.bLowHealthVignetteActive);
		TestNearlyEqual(TEXT("AC4: Healed → BPM 0"), Model.CurrentHeartbeatBPM, 0.0f, kTolerance);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
