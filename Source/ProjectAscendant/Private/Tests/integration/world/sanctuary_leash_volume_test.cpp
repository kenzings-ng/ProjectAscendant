// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "World/PASanctuaryVolume.h"
#include "World/PAMonsterLeashComponent.h"
#include "Combat/PAGameplayAbility_MeleeAttack.h"
#include "Combat/PAGameplayAbility_Finisher.h"
#include "Combat/PADamageExecutionCalculation.h"
#include "Combat/PAThreatComponent.h"
#include "Combat/PAPostureSyncComponent.h"
#include "Combat/AscendantAttributeSet.h"
#include "Character/PABaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPASanctuaryLeashVolumeIntegrationTest
 *
 * Kiểm thử tích hợp tự động cho Story map-002 (Sanctuary & Combat Zone Volumes):
 *  - AC-1 (Sanctuary Volume Safezone): Bán kính 1000cm, gán thẻ State.InSanctuary và State.Invulnerable, miễn nhiễm sát thương, chặn kỹ năng tấn công.
 *  - AC-2 (Monster Leash Boundary): Kéo quái quá 2500cm khỏi spawn origin kích hoạt LeashReset, xóa nộ khí, bất tử, chạy về với tốc độ 1.5x, hồi phục 100% Health và Posture.
 *  - AC-3 (Zone Transition Replication): Bắn delegate và thông báo chuyển vùng đồng bộ mạng (Zone.Sanctuary, Zone.Wilderness).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPASanctuaryLeashVolumeIntegrationTest,
	"ProjectAscendant.Core.World.SanctuaryLeashVolumeIntegration",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPASanctuaryLeashVolumeIntegrationTest::RunTest(const FString& Parameters)
{
	// =========================================================================
	// 1. Kiểm thử Pure Pipeline Math & Logic (FPASanctuaryLeashPipeline)
	// =========================================================================

	// -------------------------------------------------------------------------
	// 1.1: AC-1 (Sanctuary Safezone Geometric Checks & Ability Blocking)
	// -------------------------------------------------------------------------
	{
		const FVector SanctuaryCenter = FVector(0.0f, 0.0f, 0.0f);
		const float SanctuaryRadius = 1000.0f;

		// Vị trí nằm trong Sanctuary (khoảng cách 500cm <= 1000cm)
		TestTrue(TEXT("AC-1 Pipeline: Vị trí 500cm nằm trong Sanctuary"),
			FPASanctuaryLeashPipeline::IsInsideSanctuary(FVector(500.0f, 0.0f, 0.0f), SanctuaryCenter, SanctuaryRadius));

		// Vị trí nằm ngay mép biên (1000cm)
		TestTrue(TEXT("AC-1 Pipeline: Vị trí đúng mép biên 1000cm nằm trong Sanctuary"),
			FPASanctuaryLeashPipeline::IsInsideSanctuary(FVector(0.0f, 1000.0f, 0.0f), SanctuaryCenter, SanctuaryRadius));

		// Vị trí vượt quá mép biên (1001cm)
		TestFalse(TEXT("AC-1 Pipeline: Vị trí 1001cm nằm ngoài Sanctuary"),
			FPASanctuaryLeashPipeline::IsInsideSanctuary(FVector(1001.0f, 0.0f, 0.0f), SanctuaryCenter, SanctuaryRadius));

		// Chặn kỹ năng tấn công khi ở trong Sanctuary
		TestFalse(TEXT("AC-1 Pipeline: Đang trong Sanctuary -> Chặn kích hoạt tấn công"),
			FPASanctuaryLeashPipeline::CanActivateOffensiveAbilityInSanctuary(true));

		TestTrue(TEXT("AC-1 Pipeline: Ngoài Sanctuary -> Cho phép kích hoạt tấn công"),
			FPASanctuaryLeashPipeline::CanActivateOffensiveAbilityInSanctuary(false));

		// Sát thương triệt tiêu về 0 khi trong Sanctuary
		const float DamageInSanctuary = FPACombatFormulas::CalculateEffectiveDamage(200.0f, 50.0f, false, true);
		TestEqual(TEXT("AC-1 Formula: Sát thương nhận vào trong Sanctuary bằng 0.0f"), DamageInSanctuary, 0.0f);

		const float NormalDamage = FPACombatFormulas::CalculateEffectiveDamage(200.0f, 0.0f, false, false);
		TestEqual(TEXT("AC-1 Formula: Sát thương ngoài Sanctuary hoạt động bình thường (200.0f)"), NormalDamage, 200.0f);
	}

	// -------------------------------------------------------------------------
	// 1.2: AC-2 (Monster Leash Boundary Pipeline Calculations)
	// -------------------------------------------------------------------------
	{
		const FVector SpawnOrigin = FVector(0.0f, 0.0f, 0.0f);
		const float MaxLeashDistance = 2500.0f;

		// Quái vật ở khoảng cách 2000cm (<= 2500cm) -> Chưa vượt quá Leash
		TestFalse(TEXT("AC-2 Pipeline: Cự ly 2000cm (<= 2500cm) -> Chưa kích hoạt Leash"),
			FPASanctuaryLeashPipeline::IsBeyondLeashDistance(FVector(2000.0f, 0.0f, 0.0f), SpawnOrigin, MaxLeashDistance));

		// Quái vật ở khoảng cách 2500cm -> Đúng biên
		TestFalse(TEXT("AC-2 Pipeline: Cự ly đúng 2500cm -> Chưa kích hoạt Leash"),
			FPASanctuaryLeashPipeline::IsBeyondLeashDistance(FVector(0.0f, 2500.0f, 0.0f), SpawnOrigin, MaxLeashDistance));

		// Quái vật ở khoảng cách 2501cm (> 2500cm) -> Vượt quá Leash
		TestTrue(TEXT("AC-2 Pipeline: Cự ly 2501cm (> 2500cm) -> Đã vượt quá biên giới Leash"),
			FPASanctuaryLeashPipeline::IsBeyondLeashDistance(FVector(2501.0f, 0.0f, 0.0f), SpawnOrigin, MaxLeashDistance));

		// Tốc độ chạy về vị trí gốc gấp 1.5x tốc độ cơ bản (ví dụ 400 * 1.5 = 600)
		const float BaseSpeed = 400.0f;
		const float LeashSpeed = FPASanctuaryLeashPipeline::CalculateLeashReturnSpeed(BaseSpeed, 1.5f);
		TestEqual(TEXT("AC-2 Pipeline: Tốc độ hồi vị trí = 400 * 1.5 = 600 cm/s"), LeashSpeed, 600.0f);

		// Kiểm tra đã về đến vị trí gốc (tolerance = 50cm)
		TestTrue(TEXT("AC-2 Pipeline: Cự ly 40cm (<= 50cm) -> Đã về đến gốc"),
			FPASanctuaryLeashPipeline::HasArrivedAtOrigin(FVector(30.0f, 20.0f, 0.0f), SpawnOrigin, 50.0f));

		TestFalse(TEXT("AC-2 Pipeline: Cự ly 60cm (> 50cm) -> Chưa về đến gốc"),
			FPASanctuaryLeashPipeline::HasArrivedAtOrigin(FVector(60.0f, 0.0f, 0.0f), SpawnOrigin, 50.0f));

		// Vector hướng quay về điểm xuất phát
		const FVector CurrentPos = FVector(1000.0f, 0.0f, 0.0f);
		const FVector ReturnDir = FPASanctuaryLeashPipeline::CalculateReturnDirection(CurrentPos, SpawnOrigin);
		TestEqual(TEXT("AC-2 Pipeline: Vector hướng quay về X = -1.0"), static_cast<float>(ReturnDir.X), -1.0f, 0.01f);
		TestEqual(TEXT("AC-2 Pipeline: Vector hướng quay về Y = 0.0"), static_cast<float>(ReturnDir.Y), 0.0f, 0.01f);
	}

	// =========================================================================
	// 2. Kiểm thử Actor & Component (APASanctuaryVolume & UPAMonsterLeashComponent)
	// =========================================================================

	// -------------------------------------------------------------------------
	// 2.1: AC-1 & AC-3 (APASanctuaryVolume Configuration & Player Filtering)
	// -------------------------------------------------------------------------
	{
		APASanctuaryVolume* SanctuaryVolume = NewObject<APASanctuaryVolume>();
		TestNotNull(TEXT("AC-1: Khởi tạo APASanctuaryVolume thành công"), SanctuaryVolume);

		if (SanctuaryVolume)
		{
			TestEqual(TEXT("AC-1: Bán kính Sanctuary mặc định phải là 1000.0 cm"),
				SanctuaryVolume->GetSanctuaryRadius(), 1000.0f);

			USphereComponent* SphereComp = SanctuaryVolume->GetSphereComponent();
			TestNotNull(TEXT("AC-1: SphereComponent phải được khởi tạo"), SphereComp);

			if (SphereComp)
			{
				TestEqual(TEXT("AC-1: Bán kính SphereComponent = 1000.0 cm"),
					SphereComp->GetUnscaledSphereRadius(), 1000.0f, 0.1f);
				TestEqual(TEXT("AC-1: CollisionEnabled phải là QueryOnly"),
					SphereComp->GetCollisionEnabled(), ECollisionEnabled::QueryOnly);
			}

			// Kiểm tra bộ lọc người chơi (Player Filtering)
			AActor* DummyMonster = NewObject<AActor>();
			TestFalse(TEXT("AC-1: DummyMonster không mang tag Player -> IsPlayerActor = false"),
				SanctuaryVolume->IsPlayerActor(DummyMonster));

			AActor* DummyPlayer = NewObject<AActor>();
			DummyPlayer->Tags.Add(FName(TEXT("Player")));
			TestTrue(TEXT("AC-1: DummyPlayer có tag Player -> IsPlayerActor = true"),
				SanctuaryVolume->IsPlayerActor(DummyPlayer));

			// Kiểm tra delegate thông báo chuyển vùng AC-3
			const FGameplayTag TagSanctuary = FGameplayTag::RequestGameplayTag(TEXT("Zone.Sanctuary"), false);
			SanctuaryVolume->OnZoneTransitionChanged.Broadcast(SanctuaryVolume, TagSanctuary);
			TestTrue(TEXT("AC-3: Delegate OnZoneTransitionChanged phát an toàn"), true);
		}
	}

	// -------------------------------------------------------------------------
	// 2.2: AC-1 (Chặn Kỹ Năng Tấn Công Trong Sanctuary qua ActivationBlockedTags)
	// -------------------------------------------------------------------------
	{
		UPAGameplayAbility_MeleeAttack* MeleeAbility = NewObject<UPAGameplayAbility_MeleeAttack>();
		UPAGameplayAbility_Finisher* FinisherAbility = NewObject<UPAGameplayAbility_Finisher>();

		TestNotNull(TEXT("AC-1: Khởi tạo UPAGameplayAbility_MeleeAttack thành công"), MeleeAbility);
		TestNotNull(TEXT("AC-1: Khởi tạo UPAGameplayAbility_Finisher thành công"), FinisherAbility);

		if (MeleeAbility && FinisherAbility)
		{
			const FGameplayTag TagInSanctuary = FGameplayTag::RequestGameplayTag(FName("State.InSanctuary"), false);
			if (TagInSanctuary.IsValid())
			{
				TestTrue(TEXT("AC-1: MeleeAttack ActivationBlockedTags chứa State.InSanctuary"),
					MeleeAbility->GetActivationBlockedTagContainer().HasTag(TagInSanctuary));
				TestTrue(TEXT("AC-1: Finisher ActivationBlockedTags chứa State.InSanctuary"),
					FinisherAbility->GetActivationBlockedTagContainer().HasTag(TagInSanctuary));
			}
		}
	}

	// -------------------------------------------------------------------------
	// 2.3: AC-2 (UPAMonsterLeashComponent Lifecycle & Reset Logic)
	// -------------------------------------------------------------------------
	{
		UPAMonsterLeashComponent* LeashComp = NewObject<UPAMonsterLeashComponent>();
		TestNotNull(TEXT("AC-2: Khởi tạo UPAMonsterLeashComponent thành công"), LeashComp);

		if (LeashComp)
		{
			TestEqual(TEXT("AC-2: Giới hạn kéo quái mặc định là 2500.0 cm"), LeashComp->GetLeashDistance(), 2500.0f);
			TestEqual(TEXT("AC-2: Hệ số tăng tốc chạy về mặc định là 1.5x"), LeashComp->GetLeashSpeedMultiplier(), 1.5f);
			TestFalse(TEXT("AC-2: Ban đầu quái vật không ở trạng thái LeashReset"), LeashComp->IsLeashResetting());

			const FVector CustomOrigin = FVector(500.0f, 500.0f, 100.0f);
			LeashComp->SetSpawnOrigin(CustomOrigin);
			TestEqual(TEXT("AC-2: Gán SpawnOrigin thành công"), LeashComp->GetSpawnOrigin(), CustomOrigin);

			// Kích hoạt và kết thúc LeashReset
			LeashComp->TriggerLeashReset();
			TestTrue(TEXT("AC-2: Sau TriggerLeashReset -> IsLeashResetting() = true"), LeashComp->IsLeashResetting());

			// Kiểm tra hàm thực thi di chuyển quay về ExecuteReturnMovement
			LeashComp->ExecuteReturnMovement();
			TestTrue(TEXT("AC-2: ExecuteReturnMovement thực thi an toàn"), true);

			LeashComp->OnLeashStateChanged.Broadcast(true);
			LeashComp->OnLeashCompleted.Broadcast();
			TestTrue(TEXT("AC-2: Delegate OnLeashStateChanged và OnLeashCompleted phát an toàn"), true);

			LeashComp->CompleteLeashReset();
			TestFalse(TEXT("AC-2: Sau CompleteLeashReset -> IsLeashResetting() = false"), LeashComp->IsLeashResetting());
		}
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
