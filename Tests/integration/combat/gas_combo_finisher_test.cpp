// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Combat/PAGameplayAbility_MeleeAttack.h"
#include "Combat/PAGameplayAbility_Finisher.h"
#include "Combat/PAPostureSyncComponent.h"
#include "Combat/AscendantAttributeSet.h"
#include "Character/PABaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAGASComboFinisherIntegrationTest
 *
 * Kiểm thử tích hợp tự động cho Story cmbt-001 (GAS 3-Hit Combo & Posture Finisher Execution):
 *  - AC-1 (3-Hit Combo Sequence): Successive inputs within 1.2s chain Attack 1 (1.0x, 10 Posture) -> Attack 2 (1.2x, 15 Posture) -> Attack 3 (1.6x, 25 Posture). Delay > 1.2s resets to Attack 1.
 *  - AC-2 (AnimNotify Hitbox Raycast): Sector trace (radius 180cm, angle 90° forward cone / +/- 45°) validated on server.
 *  - AC-3 (Posture Finisher Execution): Targeted enemy with State.Broken tag and distance <= 250cm triggers execution sequence: 1.5s stun, attacker 1.5s invulnerable, 25% max HP pure damage, posture reset.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAGASComboFinisherIntegrationTest,
	"ProjectAscendant.Core.Combat.GASComboFinisherIntegration",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAGASComboFinisherIntegrationTest::RunTest(const FString& Parameters)
{
	// =========================================================================
	// 1. Kiểm thử Pure Pipeline Math & Logic (FPAComboFinisherPipeline)
	// =========================================================================

	// -------------------------------------------------------------------------
	// 1.1: AC-1 (3-Hit Combo Chain & Reset Delay)
	// -------------------------------------------------------------------------
	{
		// Khởi đầu chuỗi (step <= 0) luôn trả về Nhịp 1
		TestEqual(TEXT("AC-1 Pipeline: Khởi đầu chuỗi đòn đánh -> Nhịp 1"),
			FPAComboFinisherPipeline::AdvanceComboStep(0, 0.0f, 1.2f), 1);

		// Nhịp 1 -> Nhịp 2 trong vòng 1.2s (ví dụ sau 0.5s)
		TestEqual(TEXT("AC-1 Pipeline: Nhịp 1 nối đòn sau 0.5s (<= 1.2s) -> Nhịp 2"),
			FPAComboFinisherPipeline::AdvanceComboStep(1, 0.5f, 1.2f), 2);

		// Nhịp 2 -> Nhịp 3 trong vòng 1.2s (ví dụ sau 0.8s)
		TestEqual(TEXT("AC-1 Pipeline: Nhịp 2 nối đòn sau 0.8s (<= 1.2s) -> Nhịp 3"),
			FPAComboFinisherPipeline::AdvanceComboStep(2, 0.8f, 1.2f), 3);

		// Nhịp 3 -> Quay lại Nhịp 1 nếu tiếp tục chém trong 1.2s
		TestEqual(TEXT("AC-1 Pipeline: Nhịp 3 nối đòn sau 0.4s (<= 1.2s) -> Vòng lặp về Nhịp 1"),
			FPAComboFinisherPipeline::AdvanceComboStep(3, 0.4f, 1.2f), 1);

		// Reset về Nhịp 1 nếu quá thời gian chờ (> 1.2s)
		TestEqual(TEXT("AC-1 Pipeline: Đang ở Nhịp 1 nhưng chờ 1.21s (> 1.2s) -> Reset về Nhịp 1"),
			FPAComboFinisherPipeline::AdvanceComboStep(1, 1.21f, 1.2f), 1);

		TestEqual(TEXT("AC-1 Pipeline: Đang ở Nhịp 2 nhưng chờ 2.0s (> 1.2s) -> Reset về Nhịp 1"),
			FPAComboFinisherPipeline::AdvanceComboStep(2, 2.0f, 1.2f), 1);

		TestEqual(TEXT("AC-1 Pipeline: Đang ở Nhịp 3 nhưng chờ 1.5s (> 1.2s) -> Reset về Nhịp 1"),
			FPAComboFinisherPipeline::AdvanceComboStep(3, 1.5f, 1.2f), 1);

		// Hệ số sát thương AC-1: Nhịp 1 = 1.0x, Nhịp 2 = 1.2x, Nhịp 3 = 1.6x
		TestEqual(TEXT("AC-1 Pipeline: Sát thương Nhịp 1 = 1.0x"),
			FPAComboFinisherPipeline::GetComboDamageMultiplier(1), 1.0f);
		TestEqual(TEXT("AC-1 Pipeline: Sát thương Nhịp 2 = 1.2x"),
			FPAComboFinisherPipeline::GetComboDamageMultiplier(2), 1.2f);
		TestEqual(TEXT("AC-1 Pipeline: Sát thương Nhịp 3 = 1.6x"),
			FPAComboFinisherPipeline::GetComboDamageMultiplier(3), 1.6f);

		// Sát thương Posture AC-1: Nhịp 1 = 10, Nhịp 2 = 15, Nhịp 3 = 25
		TestEqual(TEXT("AC-1 Pipeline: Posture Damage Nhịp 1 = 10"),
			FPAComboFinisherPipeline::GetComboPostureDamage(1), 10.0f);
		TestEqual(TEXT("AC-1 Pipeline: Posture Damage Nhịp 2 = 15"),
			FPAComboFinisherPipeline::GetComboPostureDamage(2), 15.0f);
		TestEqual(TEXT("AC-1 Pipeline: Posture Damage Nhịp 3 = 25"),
			FPAComboFinisherPipeline::GetComboPostureDamage(3), 25.0f);

		// Kiểm tra cấu trúc FPAComboStepConfig (tránh repeated switches)
		const FPAComboStepConfig Step1Config = FPAComboFinisherPipeline::GetComboStepConfig(1);
		TestEqual(TEXT("AC-1 Pipeline: Step 1 Config Multiplier = 1.0f"), Step1Config.DamageMultiplier, 1.0f);
		TestEqual(TEXT("AC-1 Pipeline: Step 1 Config Posture = 10.0f"), Step1Config.PostureDamage, 10.0f);

		const FPAComboStepConfig Step2Config = FPAComboFinisherPipeline::GetComboStepConfig(2);
		TestEqual(TEXT("AC-1 Pipeline: Step 2 Config Multiplier = 1.2f"), Step2Config.DamageMultiplier, 1.2f);
		TestEqual(TEXT("AC-1 Pipeline: Step 2 Config Posture = 15.0f"), Step2Config.PostureDamage, 15.0f);

		const FPAComboStepConfig Step3Config = FPAComboFinisherPipeline::GetComboStepConfig(3);
		TestEqual(TEXT("AC-1 Pipeline: Step 3 Config Multiplier = 1.6f"), Step3Config.DamageMultiplier, 1.6f);
		TestEqual(TEXT("AC-1 Pipeline: Step 3 Config Posture = 25.0f"), Step3Config.PostureDamage, 25.0f);
	}

	// -------------------------------------------------------------------------
	// 1.2: AC-2 (Sector Trace Geometry: R = 180cm, Angle = 90° / +/- 45°)
	// -------------------------------------------------------------------------
	{
		const FVector AttackerPos = FVector(0.0f, 0.0f, 0.0f);
		const FVector Facing = FVector(1.0f, 0.0f, 0.0f); // Hướng mặt theo trục X

		// Mục tiêu thẳng phía trước ở cự ly 100cm (<= 180cm, góc 0°)
		TestTrue(TEXT("AC-2 Pipeline: Mục tiêu thẳng phía trước 100cm -> Nằm trong Sector"),
			FPAComboFinisherPipeline::IsTargetInAttackSector(AttackerPos, Facing, FVector(100.0f, 0.0f, 0.0f), 180.0f, 45.0f));

		// Mục tiêu ở mép biên cự ly 180cm (góc 0°)
		TestTrue(TEXT("AC-2 Pipeline: Mục tiêu ở đúng biên bán kính 180cm -> Nằm trong Sector"),
			FPAComboFinisherPipeline::IsTargetInAttackSector(AttackerPos, Facing, FVector(180.0f, 0.0f, 0.0f), 180.0f, 45.0f));

		// Mục tiêu vượt quá bán kính 180cm (181cm)
		TestFalse(TEXT("AC-2 Pipeline: Mục tiêu ở cự ly 181cm (> 180cm) -> Nằm ngoài Sector"),
			FPAComboFinisherPipeline::IsTargetInAttackSector(AttackerPos, Facing, FVector(181.0f, 0.0f, 0.0f), 180.0f, 45.0f));

		// Mục tiêu chếch 45 độ ở cự ly 100cm: X = 100*cos(45°), Y = 100*sin(45°)
		const float Cos45 = FMath::Cos(FMath::DegreesToRadians(45.0f));
		const float Sin45 = FMath::Sin(FMath::DegreesToRadians(45.0f));
		TestTrue(TEXT("AC-2 Pipeline: Mục tiêu ở góc chính xác 45 độ (mép nón) -> Nằm trong Sector"),
			FPAComboFinisherPipeline::IsTargetInAttackSector(AttackerPos, Facing, FVector(100.0f * Cos45, 100.0f * Sin45, 0.0f), 180.0f, 45.0f));

		// Mục tiêu chếch 46 độ (> 45 độ biên): X = 100*cos(46°), Y = 100*sin(46°)
		const float Cos46 = FMath::Cos(FMath::DegreesToRadians(46.0f));
		const float Sin46 = FMath::Sin(FMath::DegreesToRadians(46.0f));
		TestFalse(TEXT("AC-2 Pipeline: Mục tiêu ở góc 46 độ (> 45 độ) -> Nằm ngoài Sector"),
			FPAComboFinisherPipeline::IsTargetInAttackSector(AttackerPos, Facing, FVector(100.0f * Cos46, 100.0f * Sin46, 0.0f), 180.0f, 45.0f));

		// Mục tiêu phía sau lưng (-100, 0, 0)
		TestFalse(TEXT("AC-2 Pipeline: Mục tiêu sau lưng (180 độ) -> Nằm ngoài Sector"),
			FPAComboFinisherPipeline::IsTargetInAttackSector(AttackerPos, Facing, FVector(-100.0f, 0.0f, 0.0f), 180.0f, 45.0f));
	}

	// -------------------------------------------------------------------------
	// 1.3: AC-3 (Posture Finisher Validation & 25% Pure Damage)
	// -------------------------------------------------------------------------
	{
		// Cự ly <= 250cm và có tag State.Broken -> Đạt điều kiện kết liễu
		TestTrue(TEXT("AC-3 Pipeline: Cự ly 200cm (<= 250cm) và mục tiêu bị Broken -> Cho phép Finisher"),
			FPAComboFinisherPipeline::CanExecuteFinisher(200.0f, true, 250.0f));

		TestTrue(TEXT("AC-3 Pipeline: Cự ly 250cm vừa đủ và mục tiêu bị Broken -> Cho phép Finisher"),
			FPAComboFinisherPipeline::CanExecuteFinisher(250.0f, true, 250.0f));

		// Cự ly > 250cm dù mục tiêu vỡ thế -> Chặn tuyệt đối
		TestFalse(TEXT("AC-3 Pipeline: Cự ly 251cm (> 250cm) dù bị Broken -> Chặn kết liễu"),
			FPAComboFinisherPipeline::CanExecuteFinisher(251.0f, true, 250.0f));

		// Cự ly gần (100cm) nhưng mục tiêu KHÔNG bị vỡ thế -> Chặn tuyệt đối
		TestFalse(TEXT("AC-3 Pipeline: Cự ly 100cm nhưng mục tiêu chưa bị Broken -> Chặn kết liễu"),
			FPAComboFinisherPipeline::CanExecuteFinisher(100.0f, false, 250.0f));

		// Sát thương chuẩn AC-3: Đúng 25% Max HP của mục tiêu
		TestEqual(TEXT("AC-3 Pipeline: Sát thương Finisher với MaxHP = 1000 là 250 (25%)"),
			FPAComboFinisherPipeline::CalculateFinisherDamage(1000.0f, 0.25f), 250.0f);

		TestEqual(TEXT("AC-3 Pipeline: Sát thương Finisher với Boss MaxHP = 4000 là 1000 (25%)"),
			FPAComboFinisherPipeline::CalculateFinisherDamage(4000.0f, 0.25f), 1000.0f);
	}

	// =========================================================================
	// 2. Kiểm thử GAS Ability: UPAGameplayAbility_MeleeAttack
	// =========================================================================
	{
		UPAGameplayAbility_MeleeAttack* MeleeAbility = NewObject<UPAGameplayAbility_MeleeAttack>();
		TestNotNull(TEXT("AC-1 GAS: Khởi tạo UPAGameplayAbility_MeleeAttack thành công"), MeleeAbility);

		if (MeleeAbility)
		{
			TestEqual(TEXT("AC-1 GAS: Ban đầu CurrentComboStep = 0"), MeleeAbility->GetCurrentComboStep(), 0);

			// Kiểm tra Asset Tags của Ability
			const FGameplayTagContainer& AbilityTags = MeleeAbility->GetAssetTags();
			static const FGameplayTag TagAttack = FGameplayTag::RequestGameplayTag(FName("Ability.Skill.Attack"), false);
			if (TagAttack.IsValid())
			{
				TestTrue(TEXT("AC-1 GAS: Ability sở hữu thẻ Ability.Skill.Attack"), AbilityTags.HasTag(TagAttack));
			}

			// Kiểm tra ResetCombo
			MeleeAbility->ResetCombo();
			TestEqual(TEXT("AC-1 GAS: Sau ResetCombo(), CurrentComboStep phải bằng 0"), MeleeAbility->GetCurrentComboStep(), 0);
		}
	}

	// =========================================================================
	// 3. Kiểm thử GAS Ability: UPAGameplayAbility_Finisher
	// =========================================================================
	{
		UPAGameplayAbility_Finisher* FinisherAbility = NewObject<UPAGameplayAbility_Finisher>();
		TestNotNull(TEXT("AC-3 GAS: Khởi tạo UPAGameplayAbility_Finisher thành công"), FinisherAbility);

		if (FinisherAbility)
		{
			TestEqual(TEXT("AC-3 GAS: MaxExecutionDistance mặc định phải là 250.0f"), FinisherAbility->GetMaxExecutionDistance(), 250.0f);
			TestEqual(TEXT("AC-3 GAS: FinisherDuration mặc định phải là 1.5f"), FinisherAbility->GetFinisherDuration(), 1.5f);
			TestEqual(TEXT("AC-3 GAS: FinisherDamageRatio mặc định phải là 0.25f (25%)"), FinisherAbility->GetFinisherDamageRatio(), 0.25f);

			const FGameplayTagContainer& AbilityTags = FinisherAbility->GetAssetTags();
			static const FGameplayTag TagFinisher = FGameplayTag::RequestGameplayTag(FName("Ability.Skill.Finisher"), false);
			if (TagFinisher.IsValid())
			{
				TestTrue(TEXT("AC-3 GAS: Ability sở hữu thẻ Ability.Skill.Finisher"), AbilityTags.HasTag(TagFinisher));
			}
		}
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
