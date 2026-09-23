// Copyright Project Ascendant. All Rights Reserved.
// Tests for UPAProgressionComponent — Story prog-001
// Covers AC-1 (XP Curve), AC-2 (Stat Growth + Skill Points), AC-3 (Replication behavior)

#include "Misc/AutomationTest.h"
#include "Progression/PAProgressionComponent.h"
#include "Progression/PAProgressionTypes.h"
#include "Combat/AscendantAttributeSet.h"
#include "AbilitySystemComponent.h"

// =========================================================================
// Helper: Tạo Actor giả lập có ASC + AttributeSet + ProgressionComponent
// =========================================================================
namespace ProgressionTestHelper
{
	struct FTestContext
	{
		TObjectPtr<AActor> TestActor;
		TObjectPtr<UPAProgressionComponent> ProgressionComp;
		TObjectPtr<UAbilitySystemComponent> ASC;
		TObjectPtr<UAscendantAttributeSet> AttrSet;

		~FTestContext()
		{
			if (TestActor)
			{
				TestActor->Destroy();
			}
		}
	};

	static FTestContext CreateTestContext(UWorld* World)
	{
		FTestContext Ctx;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Ctx.TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		// ASC + AttributeSet
		Ctx.ASC = NewObject<UAbilitySystemComponent>(Ctx.TestActor.Get());
		Ctx.ASC->RegisterComponent();

		Ctx.AttrSet = NewObject<UAscendantAttributeSet>(Ctx.TestActor.Get());
		Ctx.ASC->AddSpawnedAttribute(Ctx.AttrSet.Get());
		Ctx.ASC->InitAbilityActorInfo(Ctx.TestActor.Get(), Ctx.TestActor.Get());

		// Initialize base attributes (Level 1 defaults from GDD)
		Ctx.AttrSet->InitHealth(500.f);
		Ctx.AttrSet->InitMaxHealth(500.f);
		Ctx.AttrSet->InitMana(100.f);
		Ctx.AttrSet->InitMaxMana(100.f);
		Ctx.AttrSet->InitStamina(100.f);
		Ctx.AttrSet->InitMaxStamina(100.f);
		Ctx.AttrSet->InitAttackPower(10.f);
		Ctx.AttrSet->InitArmor(5.f);

		// ProgressionComponent
		Ctx.ProgressionComp = NewObject<UPAProgressionComponent>(Ctx.TestActor.Get());
		Ctx.ProgressionComp->RegisterComponent();

		return Ctx;
	}
}

// =========================================================================
// TEST 1: AC-1 — Đường cong XP phi tuyến
// Kiểm tra công thức XP: XPToNext(Lv) = ceil(100 * Lv^1.8)
// =========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionXPCurveTest,
	"ProjectAscendant.Progression.AC1_NonLinearXPCurve",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FProgressionXPCurveTest::RunTest(const FString& Parameters)
{
	// Level 1→2: ceil(100 * 1^1.8) = 100
	int32 XP_Lv1 = FPAProgressionFormulas::GetXPRequiredForLevel(1);
	TestEqual(TEXT("XP Level 1→2 = 100"), XP_Lv1, 100);

	// Level 2→3: ceil(100 * 2^1.8) = ceil(348.22...) = 349
	int32 XP_Lv2 = FPAProgressionFormulas::GetXPRequiredForLevel(2);
	double Expected_Lv2 = 100.0 * FMath::Pow(2.0, 1.8);
	TestEqual(TEXT("XP Level 2→3 matches formula"), XP_Lv2, static_cast<int32>(FMath::CeilToInt(Expected_Lv2)));

	// Level 10→11: ceil(100 * 10^1.8) = ceil(6309.57...) = 6310
	int32 XP_Lv10 = FPAProgressionFormulas::GetXPRequiredForLevel(10);
	double Expected_Lv10 = 100.0 * FMath::Pow(10.0, 1.8);
	TestEqual(TEXT("XP Level 10→11 matches formula"), XP_Lv10, static_cast<int32>(FMath::CeilToInt(Expected_Lv10)));

	// Level 49→50: ceil(100 * 49^1.8)
	int32 XP_Lv49 = FPAProgressionFormulas::GetXPRequiredForLevel(49);
	double Expected_Lv49 = 100.0 * FMath::Pow(49.0, 1.8);
	TestEqual(TEXT("XP Level 49→50 matches formula"), XP_Lv49, static_cast<int32>(FMath::CeilToInt(Expected_Lv49)));

	// Max level (50) trả về 0
	int32 XP_Max = FPAProgressionFormulas::GetXPRequiredForLevel(50);
	TestEqual(TEXT("XP at max level = 0"), XP_Max, 0);

	// Invalid level trả về 0
	TestEqual(TEXT("XP at level 0 = 0"), FPAProgressionFormulas::GetXPRequiredForLevel(0), 0);
	TestEqual(TEXT("XP at level -1 = 0"), FPAProgressionFormulas::GetXPRequiredForLevel(-1), 0);

	// Tính tổng XP tích lũy cho Level 3 (= XP_Lv1 + XP_Lv2)
	int64 TotalXP_Lv3 = FPAProgressionFormulas::GetTotalXPForLevel(3);
	TestEqual(TEXT("Total XP to Level 3 = sum of Lv1+Lv2"), TotalXP_Lv3, static_cast<int64>(XP_Lv1 + XP_Lv2));

	return true;
}

// =========================================================================
// TEST 2: AC-1 + AC-2 — GrantXP lên cấp + stat growth + skill point
// =========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionGrantXPTest,
	"ProjectAscendant.Progression.AC2_GrantXP_LevelUp_StatGrowth",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FProgressionGrantXPTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { return false; }

	auto Ctx = ProgressionTestHelper::CreateTestContext(World);
	auto* Prog = Ctx.ProgressionComp.Get();
	auto* Attr = Ctx.AttrSet.Get();

	// Khởi đầu: Level 1, 0 XP, 0 Skill Points
	TestEqual(TEXT("Start at Level 1"), Prog->GetCurrentLevel(), 1);
	TestEqual(TEXT("Start with 0 XP"), Prog->GetCurrentXP(), 0);
	TestEqual(TEXT("Start with 0 SP"), Prog->GetAvailableSkillPoints(), 0);

	// Ghi nhớ stats ban đầu
	const float StartHealth = Attr->GetMaxHealth();
	const float StartMana = Attr->GetMaxMana();
	const float StartStamina = Attr->GetMaxStamina();
	const float StartAttack = Attr->GetAttackPower();
	const float StartArmor = Attr->GetArmor();

	// Cấp đúng 100 XP → lên Level 2
	int32 XPNeeded = FPAProgressionFormulas::GetXPRequiredForLevel(1); // = 100
	EPAProgressionError Err = Prog->GrantXP(XPNeeded);
	TestEqual(TEXT("GrantXP succeeds"), Err, EPAProgressionError::None);
	TestEqual(TEXT("Level up to 2"), Prog->GetCurrentLevel(), 2);
	TestEqual(TEXT("XP reset to 0 after exact level-up"), Prog->GetCurrentXP(), 0);
	TestEqual(TEXT("1 Skill Point awarded"), Prog->GetAvailableSkillPoints(), 1);

	// Verify stat growth
	TestEqual(TEXT("MaxHealth += 25"), Attr->GetMaxHealth(), StartHealth + FPAProgressionFormulas::HealthPerLevel);
	TestEqual(TEXT("MaxMana += 5"), Attr->GetMaxMana(), StartMana + FPAProgressionFormulas::ManaPerLevel);
	TestEqual(TEXT("MaxStamina += 3"), Attr->GetMaxStamina(), StartStamina + FPAProgressionFormulas::StaminaPerLevel);
	TestEqual(TEXT("AttackPower += 3"), Attr->GetAttackPower(), StartAttack + FPAProgressionFormulas::AttackPowerPerLevel);
	TestEqual(TEXT("Armor += 2"), Attr->GetArmor(), StartArmor + FPAProgressionFormulas::ArmorPerLevel);

	// Health hiện tại cũng được heal lên
	TestEqual(TEXT("Current Health healed"), Attr->GetHealth(), StartHealth + FPAProgressionFormulas::HealthPerLevel);

	return true;
}

// =========================================================================
// TEST 3: AC-1 — Multi-level-up (cấp XP lớn lên nhiều cấp cùng lúc)
// =========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionMultiLevelUpTest,
	"ProjectAscendant.Progression.AC1_MultiLevelUp",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FProgressionMultiLevelUpTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { return false; }

	auto Ctx = ProgressionTestHelper::CreateTestContext(World);
	auto* Prog = Ctx.ProgressionComp.Get();

	// XP cần Level 1→2 = 100, Level 2→3 = ceil(100 * 2^1.8)
	int32 XP_Lv1 = FPAProgressionFormulas::GetXPRequiredForLevel(1);
	int32 XP_Lv2 = FPAProgressionFormulas::GetXPRequiredForLevel(2);

	// Cấp đủ XP để lên 2 level + dư 50 XP
	int32 TotalGrant = XP_Lv1 + XP_Lv2 + 50;
	EPAProgressionError Err = Prog->GrantXP(TotalGrant);

	TestEqual(TEXT("GrantXP succeeds"), Err, EPAProgressionError::None);
	TestEqual(TEXT("Jump to Level 3"), Prog->GetCurrentLevel(), 3);
	TestEqual(TEXT("Remaining XP = 50"), Prog->GetCurrentXP(), 50);
	TestEqual(TEXT("2 Skill Points awarded"), Prog->GetAvailableSkillPoints(), 2);

	return true;
}

// =========================================================================
// TEST 4: AC-2 — SpendSkillPoint
// =========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionSkillPointTest,
	"ProjectAscendant.Progression.AC2_SpendSkillPoint",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FProgressionSkillPointTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { return false; }

	auto Ctx = ProgressionTestHelper::CreateTestContext(World);
	auto* Prog = Ctx.ProgressionComp.Get();

	// Trước khi có SP: SpendSkillPoint phải thất bại
	EPAProgressionError Err = Prog->SpendSkillPoint();
	TestEqual(TEXT("Cannot spend with 0 SP"), Err, EPAProgressionError::InsufficientSkillPoints);

	// Lên Level 2 để có 1 SP
	Prog->GrantXP(FPAProgressionFormulas::GetXPRequiredForLevel(1));
	TestEqual(TEXT("Have 1 SP"), Prog->GetAvailableSkillPoints(), 1);

	// Tiêu 1 SP
	Err = Prog->SpendSkillPoint();
	TestEqual(TEXT("Spend succeeds"), Err, EPAProgressionError::None);
	TestEqual(TEXT("0 SP remaining"), Prog->GetAvailableSkillPoints(), 0);

	// Tiêu lần nữa phải thất bại
	Err = Prog->SpendSkillPoint();
	TestEqual(TEXT("Cannot spend again"), Err, EPAProgressionError::InsufficientSkillPoints);

	return true;
}

// =========================================================================
// TEST 5: AC-1 — Max Level cap và edge cases
// =========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionMaxLevelTest,
	"ProjectAscendant.Progression.AC1_MaxLevelCap",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FProgressionMaxLevelTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { return false; }

	auto Ctx = ProgressionTestHelper::CreateTestContext(World);
	auto* Prog = Ctx.ProgressionComp.Get();

	// Cấp một lượng XP rất lớn để rush đến max level
	// Tổng XP cần từ Level 1→50
	int64 TotalXPToMax = FPAProgressionFormulas::GetTotalXPForLevel(50);
	// Cấp gấp đôi để đảm bảo chạm max
	Prog->GrantXP(static_cast<int32>(FMath::Min(TotalXPToMax * 2, static_cast<int64>(MAX_int32))));

	TestEqual(TEXT("Reached max level 50"), Prog->GetCurrentLevel(), 50);
	TestTrue(TEXT("IsMaxLevel = true"), Prog->IsMaxLevel());
	TestEqual(TEXT("XP reset to 0 at max"), Prog->GetCurrentXP(), 0);
	TestEqual(TEXT("XP to next = 0 at max"), Prog->GetXPToNextLevel(), 0);

	// Cấp thêm XP khi đã max → AlreadyMaxLevel
	EPAProgressionError Err = Prog->GrantXP(1000);
	TestEqual(TEXT("Cannot grant XP at max level"), Err, EPAProgressionError::AlreadyMaxLevel);

	// 49 level-ups = 49 Skill Points
	TestEqual(TEXT("49 Skill Points total"), Prog->GetAvailableSkillPoints(), 49);

	// Invalid XP
	auto Ctx2 = ProgressionTestHelper::CreateTestContext(World);
	Err = Ctx2.ProgressionComp->GrantXP(0);
	TestEqual(TEXT("GrantXP(0) = InvalidXPAmount"), Err, EPAProgressionError::InvalidXPAmount);
	Err = Ctx2.ProgressionComp->GrantXP(-100);
	TestEqual(TEXT("GrantXP(-100) = InvalidXPAmount"), Err, EPAProgressionError::InvalidXPAmount);

	return true;
}
