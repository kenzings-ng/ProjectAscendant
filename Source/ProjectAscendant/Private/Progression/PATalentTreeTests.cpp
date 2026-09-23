// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Progression/PATalentTreeComponent.h"
#include "Progression/PATalentTreeTypes.h"
#include "Progression/PAProgressionComponent.h"
#include "Combat/AscendantAttributeSet.h"
#include "AbilitySystemComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace TalentTreeTestHelper
{
	struct FTestContext
	{
		TObjectPtr<AActor> TestActor;
		TObjectPtr<UPATalentTreeComponent> TalentTreeComp;
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

	static FTestContext CreateTestContext(UWorld* World, EPACharacterClass CharacterClass = EPACharacterClass::Vanguard)
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

		// Initialize base attributes
		Ctx.AttrSet->InitHealth(500.0f);
		Ctx.AttrSet->InitMaxHealth(500.0f);
		Ctx.AttrSet->InitMana(100.0f);
		Ctx.AttrSet->InitMaxMana(100.0f);
		Ctx.AttrSet->InitStamina(100.0f);
		Ctx.AttrSet->InitMaxStamina(100.0f);
		Ctx.AttrSet->InitPosture(0.0f);
		Ctx.AttrSet->InitMaxPosture(100.0f);
		Ctx.AttrSet->InitAttackPower(10.0f);
		Ctx.AttrSet->InitArmor(5.0f);

		// ProgressionComponent
		Ctx.ProgressionComp = NewObject<UPAProgressionComponent>(Ctx.TestActor.Get());
		Ctx.ProgressionComp->RegisterComponent();

		// TalentTreeComponent
		Ctx.TalentTreeComp = NewObject<UPATalentTreeComponent>(Ctx.TestActor.Get());
		Ctx.TalentTreeComp->SetCharacterClass(CharacterClass);
		Ctx.TalentTreeComp->RegisterComponent();

		return Ctx;
	}
}

/**
 * FPATalentTreeStructureTest
 * AC-1: Cấu trúc cây kỹ năng 3 nhánh chuyên môn hóa cho Vanguard/Ranger/Arcanist.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPATalentTreeStructureTest,
	"ProjectAscendant.Progression.TalentTree.AC1_ClassTreeStructure",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPATalentTreeStructureTest::RunTest(const FString& Parameters)
{
	// 1. Vanguard Tree: 9 nodes, 3 branches (0 = Ironclad, 1 = Duelist, 2 = Juggernaut)
	const TArray<FPATalentNode> VanguardNodes = FPATalentTreeDatabase::GetTalentTreeForClass(EPACharacterClass::Vanguard);
	TestEqual(TEXT("AC-1: Vanguard has 9 talent nodes"), VanguardNodes.Num(), 9);

	int32 VanguardB0 = 0, VanguardB1 = 0, VanguardB2 = 0;
	for (const FPATalentNode& Node : VanguardNodes)
	{
		if (Node.BranchIndex == 0) VanguardB0++;
		else if (Node.BranchIndex == 1) VanguardB1++;
		else if (Node.BranchIndex == 2) VanguardB2++;
	}
	TestEqual(TEXT("AC-1: Vanguard Branch 0 (Ironclad) has 3 nodes"), VanguardB0, 3);
	TestEqual(TEXT("AC-1: Vanguard Branch 1 (Duelist) has 3 nodes"), VanguardB1, 3);
	TestEqual(TEXT("AC-1: Vanguard Branch 2 (Juggernaut) has 3 nodes"), VanguardB2, 3);

	// 2. Ranger Tree: 9 nodes, 3 branches (0 = Marksman, 1 = Windrunner, 2 = Trapper)
	const TArray<FPATalentNode> RangerNodes = FPATalentTreeDatabase::GetTalentTreeForClass(EPACharacterClass::Ranger);
	TestEqual(TEXT("AC-1: Ranger has 9 talent nodes"), RangerNodes.Num(), 9);

	// 3. Arcanist Tree: 9 nodes, 3 branches (0 = Pyromancer, 1 = Chronomancer, 2 = Leyline)
	const TArray<FPATalentNode> ArcanistNodes = FPATalentTreeDatabase::GetTalentTreeForClass(EPACharacterClass::Arcanist);
	TestEqual(TEXT("AC-1: Arcanist has 9 talent nodes"), ArcanistNodes.Num(), 9);

	// 4. Prerequisite validation check on Tier 2 & Tier 3
	const FPATalentNode* VanguardTier2 = FPATalentTreeDatabase::FindNode(EPACharacterClass::Vanguard, FName("Vanguard_Ironclad_2"));
	TestNotNull(TEXT("AC-1: Vanguard Ironclad Tier 2 exists"), VanguardTier2);
	if (VanguardTier2)
	{
		TestEqual(TEXT("AC-1: Tier 2 requires Tier 1 node as prerequisite"), VanguardTier2->PrerequisiteNodeId, FName("Vanguard_Ironclad_1"));
		TestEqual(TEXT("AC-1: Tier 2 requires Level 5"), VanguardTier2->RequiredLevel, 5);
	}

	return true;
}

/**
 * FPATalentTreeUnlockTest
 * AC-2: Mở khóa các node nội tại, kiểm tra điều kiện tiên quyết, cấp độ và cập nhật AttributeSet.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPATalentTreeUnlockTest,
	"ProjectAscendant.Progression.TalentTree.AC2_UnlockNode_AttributeBinding",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPATalentTreeUnlockTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { return false; }

	auto Ctx = TalentTreeTestHelper::CreateTestContext(World, EPACharacterClass::Vanguard);
	auto* Talents = Ctx.TalentTreeComp.Get();
	auto* Prog = Ctx.ProgressionComp.Get();
	auto* Attr = Ctx.AttrSet.Get();

	EPATalentTreeError ErrorCode = EPATalentTreeError::None;

	// Initial check: Level 1, 0 Skill Points
	TestEqual(TEXT("AC-2: Starts with 0 Skill Points"), Prog->GetAvailableSkillPoints(), 0);

	// 1. Attempt to unlock without skill points -> InsufficientSkillPoints
	bool bUnlocked = Talents->UnlockNode(FName("Vanguard_Ironclad_1"), ErrorCode);
	TestFalse(TEXT("AC-2: Unlock fails with 0 skill points"), bUnlocked);
	TestEqual(TEXT("AC-2: Error is InsufficientSkillPoints"), ErrorCode, EPATalentTreeError::InsufficientSkillPoints);

	// 2. Grant enough XP to level up to Level 6 (gives 5 Skill Points)
	for (int32 i = 1; i <= 5; ++i)
	{
		Prog->GrantXP(FPAProgressionFormulas::GetXPRequiredForLevel(i));
	}
	TestEqual(TEXT("AC-2: Character reached Level 6"), Prog->GetCurrentLevel(), 6);
	TestEqual(TEXT("AC-2: Character has 5 Skill Points"), Prog->GetAvailableSkillPoints(), 5);

	// Record pre-talent attributes
	const float BaseAttack = Attr->GetAttackPower();
	const float BasePosture = Attr->GetMaxPosture();
	const float BaseArmor = Attr->GetArmor();
	const float BaseHealth = Attr->GetMaxHealth();

	// 3. Attempt to unlock Tier 2 node without Tier 1 prerequisite -> PrerequisiteNotMet
	bUnlocked = Talents->UnlockNode(FName("Vanguard_Ironclad_2"), ErrorCode);
	TestFalse(TEXT("AC-2: Cannot unlock Tier 2 without Tier 1"), bUnlocked);
	TestEqual(TEXT("AC-2: Error is PrerequisiteNotMet"), ErrorCode, EPATalentTreeError::PrerequisiteNotMet);

	// 4. Unlock Tier 1 node: Vanguard_Ironclad_1 (+10 MaxPosture, +5 Armor)
	bUnlocked = Talents->UnlockNode(FName("Vanguard_Ironclad_1"), ErrorCode);
	TestTrue(TEXT("AC-2: Unlock Tier 1 node succeeds"), bUnlocked);
	TestEqual(TEXT("AC-2: Error is None"), ErrorCode, EPATalentTreeError::None);
	TestEqual(TEXT("AC-2: Skill Points deducted to 4"), Prog->GetAvailableSkillPoints(), 4);
	TestTrue(TEXT("AC-2: Node is marked unlocked"), Talents->IsNodeUnlocked(FName("Vanguard_Ironclad_1")));

	// Verify AttributeSet updated directly
	TestEqual(TEXT("AC-2: MaxPosture increased by 10"), Attr->GetMaxPosture(), BasePosture + 10.0f);
	TestEqual(TEXT("AC-2: Armor increased by 5"), Attr->GetArmor(), BaseArmor + 5.0f);

	// 5. Attempt unlock already unlocked node -> NodeAlreadyUnlocked
	bUnlocked = Talents->UnlockNode(FName("Vanguard_Ironclad_1"), ErrorCode);
	TestFalse(TEXT("AC-2: Cannot re-unlock node"), bUnlocked);
	TestEqual(TEXT("AC-2: Error is NodeAlreadyUnlocked"), ErrorCode, EPATalentTreeError::NodeAlreadyUnlocked);

	// 6. Unlock Tier 2 node: Vanguard_Ironclad_2 (+20 MaxPosture, +50 MaxHealth)
	bUnlocked = Talents->UnlockNode(FName("Vanguard_Ironclad_2"), ErrorCode);
	TestTrue(TEXT("AC-2: Unlock Tier 2 node succeeds now"), bUnlocked);
	TestEqual(TEXT("AC-2: MaxPosture now +30 total"), Attr->GetMaxPosture(), BasePosture + 30.0f);
	TestEqual(TEXT("AC-2: MaxHealth increased by 50"), Attr->GetMaxHealth(), BaseHealth + 50.0f);

	// 7. Unlock Dash Cooldown node: Vanguard_Juggernaut_1 (0.05s Dash CDR, +5 AttackPower)
	bUnlocked = Talents->UnlockNode(FName("Vanguard_Juggernaut_1"), ErrorCode);
	TestTrue(TEXT("AC-2: Unlock Juggernaut 1 succeeds"), bUnlocked);
	TestEqual(TEXT("AC-2: Dash CDR is 0.05s"), Talents->GetTotalDashCooldownReduction(), 0.05f);
	TestEqual(TEXT("AC-2: AttackPower increased by 5"), Attr->GetAttackPower(), BaseAttack + 5.0f);

	return true;
}

/**
 * FPATalentTreeResetTest
 * AC-3: Tẩy điểm kỹ năng, hoàn trả Skill Points và gỡ bỏ chỉ số khỏi AttributeSet.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPATalentTreeResetTest,
	"ProjectAscendant.Progression.TalentTree.AC3_ResetTalents_Refund",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPATalentTreeResetTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { return false; }

	auto Ctx = TalentTreeTestHelper::CreateTestContext(World, EPACharacterClass::Vanguard);
	auto* Talents = Ctx.TalentTreeComp.Get();
	auto* Prog = Ctx.ProgressionComp.Get();
	auto* Attr = Ctx.AttrSet.Get();

	EPATalentTreeError ErrorCode = EPATalentTreeError::None;

	// Reset when no nodes unlocked -> NoNodesToReset
	bool bReset = Talents->ResetTalents(ErrorCode);
	TestFalse(TEXT("AC-3: Reset fails when no nodes unlocked"), bReset);
	TestEqual(TEXT("AC-3: Error is NoNodesToReset"), ErrorCode, EPATalentTreeError::NoNodesToReset);

	// Level up to Level 6 (5 skill points)
	for (int32 i = 1; i <= 5; ++i)
	{
		Prog->GrantXP(FPAProgressionFormulas::GetXPRequiredForLevel(i));
	}

	const float PreTalentAttack = Attr->GetAttackPower();
	const float PreTalentPosture = Attr->GetMaxPosture();
	const float PreTalentArmor = Attr->GetArmor();
	const float PreTalentHealth = Attr->GetMaxHealth();

	// Unlock 3 nodes (spending 3 skill points)
	Talents->UnlockNode(FName("Vanguard_Ironclad_1"), ErrorCode);
	Talents->UnlockNode(FName("Vanguard_Ironclad_2"), ErrorCode);
	Talents->UnlockNode(FName("Vanguard_Juggernaut_1"), ErrorCode);

	TestEqual(TEXT("AC-3: 3 nodes unlocked"), Talents->GetUnlockedNodeCount(), 3);
	TestEqual(TEXT("AC-3: 2 Skill Points remaining (5 - 3)"), Prog->GetAvailableSkillPoints(), 2);
	TestTrue(TEXT("AC-3: Posture is higher than pre-talent"), Attr->GetMaxPosture() > PreTalentPosture);

	// Perform ResetTalents
	bReset = Talents->ResetTalents(ErrorCode);
	TestTrue(TEXT("AC-3: ResetTalents succeeds"), bReset);
	TestEqual(TEXT("AC-3: Error is None"), ErrorCode, EPATalentTreeError::None);

	// Verify all 3 Skill Points refunded back to ProgressionComponent (2 + 3 = 5)
	TestEqual(TEXT("AC-3: Skill Points refunded to 5"), Prog->GetAvailableSkillPoints(), 5);

	// Verify all nodes cleared
	TestEqual(TEXT("AC-3: Unlocked node count reset to 0"), Talents->GetUnlockedNodeCount(), 0);
	TestFalse(TEXT("AC-3: Ironclad 1 no longer unlocked"), Talents->IsNodeUnlocked(FName("Vanguard_Ironclad_1")));
	TestEqual(TEXT("AC-3: Dash CDR reset to 0"), Talents->GetTotalDashCooldownReduction(), 0.0f);

	// Verify AttributeSet restored to exact pre-talent values
	TestEqual(TEXT("AC-3: AttackPower restored to pre-talent"), Attr->GetAttackPower(), PreTalentAttack);
	TestEqual(TEXT("AC-3: MaxPosture restored to pre-talent"), Attr->GetMaxPosture(), PreTalentPosture);
	TestEqual(TEXT("AC-3: Armor restored to pre-talent"), Attr->GetArmor(), PreTalentArmor);
	TestEqual(TEXT("AC-3: MaxHealth restored to pre-talent"), Attr->GetMaxHealth(), PreTalentHealth);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
