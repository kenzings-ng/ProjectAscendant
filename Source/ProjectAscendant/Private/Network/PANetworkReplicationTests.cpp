// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Network/PAIrisSpatialFilter.h"
#include "Character/PABaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperZDAnimationComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPANetworkReplicationTests
 *
 * Kiểm thử tự động cho Giai đoạn 1.5 (Network & Iris Replication Verification):
 * - AC-1: Iris Replication Configuration & Spatial Filtering Tiers (High/Mid/Dormant).
 * - AC-2: Iris Frequency and Culling calculations (60Hz / 30Hz / 0Hz Cull).
 * - AC-3: Player ASC Replication Mode is Mixed (EGameplayEffectReplicationMode::Mixed).
 * - AC-4: Character Network Replication flags (bReplicates, ReplicateMovement, TickOnDedicatedServer).
 * - AC-5: Server-Authoritative GAS Tag State Transitions (State.Hurt, State.Stunned, State.Dead).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPANetworkReplicationTests,
	"ProjectAscendant.Network.IrisReplication",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPANetworkReplicationTests::RunTest(const FString& Parameters)
{
	// =========================================================================
	// AC-1 & AC-2: Iris Spatial Filter Tiers & Bandwidth Culling
	// =========================================================================
	{
		FPAIrisSpatialConfig Config;
		Config.Tier1Radius = 1500.0f;
		Config.Tier2Radius = 3500.0f;
		Config.Tier1Frequency = 60.0f;
		Config.Tier2Frequency = 30.0f;
		Config.Tier3Frequency = 0.0f;

		// 1.1 Kiểm tra cự ly gần (Tier 1: High Frequency)
		const FVector Observer(0.0f, 0.0f, 0.0f);
		const FVector NearTarget(1000.0f, 0.0f, 0.0f); // 10m <= 15m
		const EPAIrisSpatialTier Tier1 = UPAIrisSpatialFilter::EvaluateSpatialTier(Observer, NearTarget, Config);
		TestEqual(TEXT("AC-1: Distance 1000cm is Tier1_HighFrequency"), Tier1, EPAIrisSpatialTier::Tier1_HighFrequency);
		TestEqual(TEXT("AC-2: Tier 1 frequency is 60Hz"), UPAIrisSpatialFilter::GetReplicationFrequencyForDistance(1000.0f, Config), 60.0f);
		TestFalse(TEXT("AC-2: Distance 1000cm is not culled"), UPAIrisSpatialFilter::ShouldCullReplication(1000.0f, Config));
		TestNearlyEqual(TEXT("AC-2: Tier 1 interval is ~0.0167s"), UPAIrisSpatialFilter::CalculateReplicationInterval(60.0f), 1.0f / 60.0f, 0.001f);

		// 1.2 Kiểm tra cự ly trung bình (Tier 2: Mid Frequency)
		const FVector MidTarget(2500.0f, 0.0f, 0.0f); // 25m <= 35m
		const EPAIrisSpatialTier Tier2 = UPAIrisSpatialFilter::EvaluateSpatialTier(Observer, MidTarget, Config);
		TestEqual(TEXT("AC-1: Distance 2500cm is Tier2_MidFrequency"), Tier2, EPAIrisSpatialTier::Tier2_MidFrequency);
		TestEqual(TEXT("AC-2: Tier 2 frequency is 30Hz"), UPAIrisSpatialFilter::GetReplicationFrequencyForDistance(2500.0f, Config), 30.0f);
		TestFalse(TEXT("AC-2: Distance 2500cm is not culled"), UPAIrisSpatialFilter::ShouldCullReplication(2500.0f, Config));
		TestNearlyEqual(TEXT("AC-2: Tier 2 interval is ~0.0333s"), UPAIrisSpatialFilter::CalculateReplicationInterval(30.0f), 1.0f / 30.0f, 0.001f);

		// 1.3 Kiểm tra cự ly xa / ngoài tầm nhìn (Tier 3: Dormant / Culled)
		const FVector FarTarget(5000.0f, 0.0f, 0.0f); // 50m > 35m
		const EPAIrisSpatialTier Tier3 = UPAIrisSpatialFilter::EvaluateSpatialTier(Observer, FarTarget, Config);
		TestEqual(TEXT("AC-1: Distance 5000cm is Tier3_Dormant"), Tier3, EPAIrisSpatialTier::Tier3_Dormant);
		TestEqual(TEXT("AC-2: Tier 3 frequency is 0Hz"), UPAIrisSpatialFilter::GetReplicationFrequencyForDistance(5000.0f, Config), 0.0f);
		TestTrue(TEXT("AC-2: Distance 5000cm is culled from high-frequency replication"), UPAIrisSpatialFilter::ShouldCullReplication(5000.0f, Config));
		TestTrue(TEXT("AC-2: Tier 3 interval is infinite (> 99999s)"), UPAIrisSpatialFilter::CalculateReplicationInterval(0.0f) >= 999999.0f);
	}

	// =========================================================================
	// AC-3 & AC-4: Player ASC Replication Mode & Network Flags
	// =========================================================================
	{
		APABaseCharacter* Char = NewObject<APABaseCharacter>();
		TestNotNull(TEXT("AC-3: APABaseCharacter created"), Char);

		if (Char)
		{
			// AC-3: Kiểm tra ASC Replication Mode là Mixed (khuyến nghị cho nhân vật người chơi trong GAS)
			UAbilitySystemComponent* ASC = Char->GetAbilitySystemComponent();
			TestNotNull(TEXT("AC-3: Character has valid AbilitySystemComponent"), ASC);
			if (ASC)
			{
				TestTrue(TEXT("AC-3: ASC is replicated"), ASC->GetIsReplicated());
				TestEqual(TEXT("AC-3: ASC Replication Mode is Mixed"),
					ASC->ReplicationMode,
					EGameplayEffectReplicationMode::Mixed);
			}

			// AC-4: Kiểm tra Network Replication Flags
			TestTrue(TEXT("AC-4: Character bReplicates is true"), Char->GetIsReplicated());
			TestTrue(TEXT("AC-4: Character IsReplicatingMovement is true"), Char->IsReplicatingMovement());

			// PaperZD Dedicated Server execution flag
			UPaperZDAnimationComponent* AnimComp = Char->GetPaperZDAnimComponent();
			TestNotNull(TEXT("AC-4: Character has PaperZDAnimationComponent"), AnimComp);
			if (AnimComp)
			{
				TestTrue(TEXT("AC-4: AnimComp ticks on Dedicated Server"),
					AnimComp->PrimaryComponentTick.bAllowTickOnDedicatedServer);
			}
		}
	}

	// =========================================================================
	// AC-5: Server-Authoritative GAS Tag State Transitions
	// =========================================================================
	{
		UGameplayTagsManager& TagMgr = UGameplayTagsManager::Get();
		TagMgr.AddNativeGameplayTag(FName(TEXT("State.Hurt")));
		TagMgr.AddNativeGameplayTag(FName(TEXT("State.Stunned")));
		TagMgr.AddNativeGameplayTag(FName(TEXT("State.Broken")));
		TagMgr.AddNativeGameplayTag(FName(TEXT("State.Dead")));

		APABaseCharacter* Char = NewObject<APABaseCharacter>();
		if (Char)
		{
			UAbilitySystemComponent* ASC = Char->GetAbilitySystemComponent();
			if (ASC)
			{
				const FGameplayTag TagHurt = FGameplayTag::RequestGameplayTag(FName("State.Hurt"));
				const FGameplayTag TagStunned = FGameplayTag::RequestGameplayTag(FName("State.Stunned"));
				const FGameplayTag TagDead = FGameplayTag::RequestGameplayTag(FName("State.Dead"));

				TestTrue(TEXT("AC-5: State.Hurt gameplay tag is registered"), TagHurt.IsValid());
				TestTrue(TEXT("AC-5: State.Stunned gameplay tag is registered"), TagStunned.IsValid());
				TestTrue(TEXT("AC-5: State.Dead gameplay tag is registered"), TagDead.IsValid());

				// Test posture break state transition (Broken + Stunned)
				Char->HandlePostureBroken(nullptr);
				TestTrue(TEXT("AC-5: Character has State.Stunned upon posture break"), ASC->HasMatchingGameplayTag(TagStunned));

				// Test out of health state transition (Dead)
				Char->HandleOutOfHealth(nullptr);
				TestTrue(TEXT("AC-5: Character has State.Dead upon out of health"), ASC->HasMatchingGameplayTag(TagDead));

				// Verify movement disabled on death
				if (UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement())
				{
					TestEqual(TEXT("AC-5: MovementMode is MOVE_None on death"), MoveComp->MovementMode.GetValue(), (uint8)MOVE_None);
				}
			}
		}
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
