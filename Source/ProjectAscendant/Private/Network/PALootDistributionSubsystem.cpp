// Copyright Project Ascendant. All Rights Reserved.

#include "Network/PALootDistributionSubsystem.h"
#include "Network/PALootDropletActor.h"
#include "Character/PABaseCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

void UPALootDistributionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	EncounterContributions.Empty();
}

void UPALootDistributionSubsystem::Deinitialize()
{
	EncounterContributions.Empty();
	Super::Deinitialize();
}

FGuid UPALootDistributionSubsystem::ResolveUID(const FString& PlayerId) const
{
	FGuid ParsedGuid;
	if (FGuid::Parse(PlayerId, ParsedGuid))
	{
		return ParsedGuid;
	}

	// Tạo GUID xác định (deterministic) từ chuỗi định danh nếu không phải chuỗi GUID
	const uint32 HashA = GetTypeHash(PlayerId);
	const uint32 HashB = GetTypeHash(PlayerId + TEXT("_B"));
	const uint32 HashC = GetTypeHash(PlayerId + TEXT("_C"));
	const uint32 HashD = GetTypeHash(PlayerId + TEXT("_D"));
	return FGuid(HashA, HashB, HashC, HashD);
}

void UPALootDistributionSubsystem::RecordCombatContributionWithUID(
	const FString& EncounterId,
	const FGuid& PlayerUID,
	const FString& PlayerId,
	APABaseCharacter* PlayerChar,
	float HPDamage,
	float PostureDamage)
{
	if (EncounterId.IsEmpty() || !PlayerUID.IsValid())
	{
		return;
	}

	TMap<FGuid, FPAContributionRecord>& EncounterMap = EncounterContributions.FindOrAdd(EncounterId);
	FPAContributionRecord& Record = EncounterMap.FindOrAdd(PlayerUID);

	Record.PlayerUID = PlayerUID;
	Record.PlayerId = PlayerId.IsEmpty() ? PlayerUID.ToString() : PlayerId;
	Record.DamageDealt += FMath::Max(0.0f, HPDamage);
	Record.PostureDamageDealt += FMath::Max(0.0f, PostureDamage);

	if (PlayerChar)
	{
		Record.PlayerCharacter = PlayerChar;
	}
}

void UPALootDistributionSubsystem::RecordCombatContribution(
	const FString& EncounterId,
	const FString& PlayerId,
	APABaseCharacter* PlayerChar,
	float HPDamage,
	float PostureDamage)
{
	if (PlayerId.IsEmpty())
	{
		return;
	}

	const FGuid ResolvedUID = ResolveUID(PlayerId);
	RecordCombatContributionWithUID(EncounterId, ResolvedUID, PlayerId, PlayerChar, HPDamage, PostureDamage);
}

bool UPALootDistributionSubsystem::GetContributionRecordByUID(
	const FString& EncounterId,
	const FGuid& PlayerUID,
	FPAContributionRecord& OutRecord) const
{
	if (const TMap<FGuid, FPAContributionRecord>* EncounterMap = EncounterContributions.Find(EncounterId))
	{
		if (const FPAContributionRecord* FoundRecord = EncounterMap->Find(PlayerUID))
		{
			OutRecord = *FoundRecord;
			return true;
		}
	}
	return false;
}

bool UPALootDistributionSubsystem::GetContributionRecord(
	const FString& EncounterId,
	const FString& PlayerId,
	FPAContributionRecord& OutRecord) const
{
	return GetContributionRecordByUID(EncounterId, ResolveUID(PlayerId), OutRecord);
}

bool UPALootDistributionSubsystem::IsPlayerEligibleForLootByUID(
	const FString& EncounterId,
	const FGuid& PlayerUID,
	float BossMaxHP,
	float BossMaxPosture) const
{
	FPAContributionRecord Record;
	if (!GetContributionRecordByUID(EncounterId, PlayerUID, Record))
	{
		return false;
	}

	const float ClampedMaxHP = FMath::Max(1.0f, BossMaxHP);
	const float ClampedMaxPosture = FMath::Max(1.0f, BossMaxPosture);

	const float HPRatio = Record.DamageDealt / ClampedMaxHP;
	const float PostureRatio = Record.PostureDamageDealt / ClampedMaxPosture;

	// AC-6: Đóng góp >= 5% tổng lượng sát thương HP HOẶC >= 10% sát thương Posture
	return (HPRatio >= kBossLootContributionHPRatio) || (PostureRatio >= kBossLootContributionPostureRatio);
}

bool UPALootDistributionSubsystem::IsPlayerEligibleForLoot(
	const FString& EncounterId,
	const FString& PlayerId,
	float BossMaxHP,
	float BossMaxPosture) const
{
	return IsPlayerEligibleForLootByUID(EncounterId, ResolveUID(PlayerId), BossMaxHP, BossMaxPosture);
}

TArray<FGuid> UPALootDistributionSubsystem::GetEligiblePlayerUIDsForLoot(
	const FString& EncounterId,
	float BossMaxHP,
	float BossMaxPosture) const
{
	TArray<FGuid> EligibleUIDs;

	if (const TMap<FGuid, FPAContributionRecord>* EncounterMap = EncounterContributions.Find(EncounterId))
	{
		for (const auto& Pair : *EncounterMap)
		{
			if (IsPlayerEligibleForLootByUID(EncounterId, Pair.Key, BossMaxHP, BossMaxPosture))
			{
				EligibleUIDs.Add(Pair.Key);
			}
		}
	}

	return EligibleUIDs;
}

TArray<FString> UPALootDistributionSubsystem::GetEligiblePlayersForLoot(
	const FString& EncounterId,
	float BossMaxHP,
	float BossMaxPosture) const
{
	TArray<FString> EligiblePlayers;

	if (const TMap<FGuid, FPAContributionRecord>* EncounterMap = EncounterContributions.Find(EncounterId))
	{
		for (const auto& Pair : *EncounterMap)
		{
			if (IsPlayerEligibleForLootByUID(EncounterId, Pair.Key, BossMaxHP, BossMaxPosture))
			{
				EligiblePlayers.Add(Pair.Value.PlayerId);
			}
		}
	}

	return EligiblePlayers;
}

TArray<APALootDropletActor*> UPALootDistributionSubsystem::DistributeInstancedLoot(
	const FString& EncounterId,
	float BossMaxHP,
	float BossMaxPosture,
	const FVector& DropOrigin,
	int32 BaseGoldReward,
	const TArray<FName>& DropItemPool)
{
	TArray<APALootDropletActor*> SpawnedDroplets;
	const TArray<FGuid> EligibleUIDs = GetEligiblePlayerUIDsForLoot(EncounterId, BossMaxHP, BossMaxPosture);

	UWorld* World = GetWorld();
	if (!World || EligibleUIDs.Num() == 0)
	{
		return SpawnedDroplets;
	}

	const float AngleStep = 2.0f * PI / FMath::Max(1, EligibleUIDs.Num());
	int32 Index = 0;

	for (const FGuid& PlayerUID : EligibleUIDs)
	{
		const float Angle = Index * AngleStep;
		const FVector DropLocation = DropOrigin + FVector(FMath::Cos(Angle) * 60.0f, FMath::Sin(Angle) * 60.0f, 0.0f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APALootDropletActor* Droplet = World->SpawnActor<APALootDropletActor>(
			APALootDropletActor::StaticClass(),
			DropLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Droplet)
		{
			APlayerController* TargetPC = nullptr;
			FPAContributionRecord Record;
			if (GetContributionRecordByUID(EncounterId, PlayerUID, Record) && Record.PlayerCharacter.IsValid())
			{
				TargetPC = Cast<APlayerController>(Record.PlayerCharacter->GetController());
			}

			Droplet->InitializeDropletWithUID(PlayerUID, Record.PlayerId, TargetPC, EncounterId, BaseGoldReward, DropItemPool);
			SpawnedDroplets.Add(Droplet);

			OnInstancedLootSpawned.Broadcast(EncounterId, Record.PlayerId, Droplet);
		}

		Index++;
	}

	return SpawnedDroplets;
}

void UPALootDistributionSubsystem::ClearEncounterData(const FString& EncounterId)
{
	EncounterContributions.Remove(EncounterId);
}

float UPALootDistributionSubsystem::GetMoraleBonus(int32 PartySize)
{
	if (PartySize <= 1)
	{
		return 0.0f;
	}
	else if (PartySize == 2)
	{
		return kMoraleBonus2P; // +10%
	}
	else if (PartySize == 3)
	{
		return kMoraleBonus3P; // +20%
	}
	else
	{
		return kMoraleBonus4P; // +35% (4 người trở lên)
	}
}

int32 UPALootDistributionSubsystem::CalculatePartyMemberExp(int32 BaseMonsterExp, int32 TotalPartySize, int32 ValidMemberCount)
{
	if (BaseMonsterExp <= 0 || ValidMemberCount <= 0)
	{
		return 0;
	}

	const float MoraleBonus = GetMoraleBonus(TotalPartySize);
	const float TotalExp = BaseMonsterExp * (1.0f + MoraleBonus);
	return FMath::RoundToInt(TotalExp / ValidMemberCount);
}

bool UPALootDistributionSubsystem::IsPartyMemberInRange(const FVector& MemberLocation, const FVector& MobDeathLocation, float Radius)
{
	return FVector::Dist(MemberLocation, MobDeathLocation) <= Radius;
}

TMap<FString, int32> UPALootDistributionSubsystem::DistributePartyExp(
	int32 BaseMonsterExp,
	const TArray<FPAPartyMemberInfo>& PartyMembers,
	const FVector& MonsterDeathLocation)
{
	TMap<FString, int32> AwardedExp;
	const int32 TotalPartySize = PartyMembers.Num();

	if (TotalPartySize == 0 || BaseMonsterExp <= 0)
	{
		return AwardedExp;
	}

	// Đếm số lượng thành viên trong phạm vi 3000 cm (AC-8)
	int32 ValidMemberCount = 0;
	for (const FPAPartyMemberInfo& Member : PartyMembers)
	{
		if (IsPartyMemberInRange(Member.Location, MonsterDeathLocation, kPartyExpRadius))
		{
			ValidMemberCount++;
		}
	}

	const int32 ExpPerValidMember = CalculatePartyMemberExp(BaseMonsterExp, TotalPartySize, ValidMemberCount);

	for (const FPAPartyMemberInfo& Member : PartyMembers)
	{
		// Thành viên trong bán kính nhận điểm EXP chia sẻ; thành viên ngoài bán kính nhận đúng 0 EXP
		if (IsPartyMemberInRange(Member.Location, MonsterDeathLocation, kPartyExpRadius))
		{
			AwardedExp.Add(Member.PlayerId, ExpPerValidMember);
		}
		else
		{
			AwardedExp.Add(Member.PlayerId, 0);
		}
	}

	return AwardedExp;
}
