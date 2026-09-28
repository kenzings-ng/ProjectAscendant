// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Animation/PASpineBossAnimationComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPASpineBossAnimationTests
 *
 * Kiểm thử tự động cho hệ thống Spine 4.3 Runtime Component của Boss Stone Golem:
 * - AC-1: Component khởi tạo an toàn ở chế độ headless.
 * - AC-2: Cấu hình mặc định animation mapping (Idle, Walk, Slam, Stagger, Death).
 * - AC-3: Quản lý trạng thái và chuyển đổi Spine State (EPASpineBossState).
 * - AC-4: Tên Event Slam Impact đồng bộ chuẩn ("slam_impact").
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPASpineBossAnimationTests,
	"ProjectAscendant.AI.SpineBossAnimation",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPASpineBossAnimationTests::RunTest(const FString& Parameters)
{
	// =========================================================================
	// AC-1: Component Instantiation & Default State
	// =========================================================================
	{
		UPASpineBossAnimationComponent* SpineComp = NewObject<UPASpineBossAnimationComponent>();
		TestNotNull(TEXT("AC-1: UPASpineBossAnimationComponent instantiated"), SpineComp);
		TestFalse(TEXT("AC-1: Spine mode inactive by default"), SpineComp->IsSpineModeActive());
		TestEqual(TEXT("AC-1: Initial state is Idle"), SpineComp->GetCurrentState(), EPASpineBossState::Idle);

		// =========================================================================
		// AC-2: Animation Name Mapping Configuration
		// =========================================================================
		TestEqual(TEXT("AC-2: IdleAnimName is 'idle'"), SpineComp->IdleAnimName, FString(TEXT("idle")));
		TestEqual(TEXT("AC-2: WalkAnimName is 'walk'"), SpineComp->WalkAnimName, FString(TEXT("walk")));
		TestEqual(TEXT("AC-2: SlamAnimName is 'slam'"), SpineComp->SlamAnimName, FString(TEXT("slam")));
		TestEqual(TEXT("AC-2: StaggerAnimName is 'stagger'"), SpineComp->StaggerAnimName, FString(TEXT("stagger")));
		TestEqual(TEXT("AC-2: DeathAnimName is 'death'"), SpineComp->DeathAnimName, FString(TEXT("death")));
		TestEqual(TEXT("AC-2: SlamImpactEventName is 'slam_impact'"), SpineComp->SlamImpactEventName, FString(TEXT("slam_impact")));

		// =========================================================================
		// AC-3: Mode Activation & Safety
		// =========================================================================
		SpineComp->SetSpineModeActive(true);
		TestTrue(TEXT("AC-3: Spine mode activated"), SpineComp->IsSpineModeActive());

		SpineComp->SetSpineModeActive(false);
		TestFalse(TEXT("AC-3: Spine mode deactivated"), SpineComp->IsSpineModeActive());
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
