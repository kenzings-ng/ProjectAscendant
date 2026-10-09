// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Character/PABaseCharacter.h"
#include "Character/PAStoneGolemBoss.h"
#include "AbilitySystemComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAGasReplicationModeTest
 *
 * DECISIONS.md §11 (X8 / review M7):
 *  - Nhân vật người chơi (APABaseCharacter — pawn mặc định của APAGameModeBase, dùng cho Vanguard/Ranger/Arcanist)
 *    dùng EGameplayEffectReplicationMode::Mixed.
 *  - Boss / quái / NPC có ASC (hiện tại: APAStoneGolemBoss) dùng EGameplayEffectReplicationMode::Minimal.
 * Kiểm tra cả CDO lẫn instance mới tạo để đảm bảo giá trị được đặt trong constructor.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAGasReplicationModeTest,
	"ProjectAscendant.Core.Character.GasReplicationMode",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAGasReplicationModeTest::RunTest(const FString& Parameters)
{
	// --- Player character: Mixed ---
	const APABaseCharacter* PlayerCDO = GetDefault<APABaseCharacter>();
	const UAbilitySystemComponent* PlayerCDOASC = PlayerCDO ? PlayerCDO->GetAbilitySystemComponent() : nullptr;
	if (!TestNotNull(TEXT("Player CDO phải có AbilitySystemComponent"), PlayerCDOASC))
	{
		return false;
	}
	TestEqual(TEXT("Player CDO ASC phải dùng Mixed"),
		static_cast<int32>(PlayerCDOASC->ReplicationMode), static_cast<int32>(EGameplayEffectReplicationMode::Mixed));

	APABaseCharacter* Player = NewObject<APABaseCharacter>();
	const UAbilitySystemComponent* PlayerASC = Player ? Player->GetAbilitySystemComponent() : nullptr;
	if (!TestNotNull(TEXT("Player instance phải có AbilitySystemComponent"), PlayerASC))
	{
		return false;
	}
	TestEqual(TEXT("Player instance ASC phải dùng Mixed"),
		static_cast<int32>(PlayerASC->ReplicationMode), static_cast<int32>(EGameplayEffectReplicationMode::Mixed));

	// --- Boss (non-player): Minimal ---
	const APAStoneGolemBoss* BossCDO = GetDefault<APAStoneGolemBoss>();
	const UAbilitySystemComponent* BossCDOASC = BossCDO ? BossCDO->GetAbilitySystemComponent() : nullptr;
	if (!TestNotNull(TEXT("Boss CDO phải có AbilitySystemComponent"), BossCDOASC))
	{
		return false;
	}
	TestEqual(TEXT("Boss CDO ASC phải dùng Minimal"),
		static_cast<int32>(BossCDOASC->ReplicationMode), static_cast<int32>(EGameplayEffectReplicationMode::Minimal));

	APAStoneGolemBoss* Boss = NewObject<APAStoneGolemBoss>();
	const UAbilitySystemComponent* BossASC = Boss ? Boss->GetAbilitySystemComponent() : nullptr;
	if (!TestNotNull(TEXT("Boss instance phải có AbilitySystemComponent"), BossASC))
	{
		return false;
	}
	TestEqual(TEXT("Boss instance ASC phải dùng Minimal"),
		static_cast<int32>(BossASC->ReplicationMode), static_cast<int32>(EGameplayEffectReplicationMode::Minimal));
	TestTrue(TEXT("Boss ASC vẫn replicate (Minimal: Attributes + Tags + Cues)"), BossASC->GetIsReplicated());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
