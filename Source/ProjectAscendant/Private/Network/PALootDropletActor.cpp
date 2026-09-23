// Copyright Project Ascendant. All Rights Reserved.

#include "Network/PALootDropletActor.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

APALootDropletActor::APALootDropletActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	OwningPlayerUID.Invalidate();
	OwningPlayerId = TEXT("");
	DropSourceEncounterId = TEXT("");
	GoldReward = 0;
	bIsClaimed = false;
	DropletLifespanSeconds = 1800.0f; // 30 phút theo GDD
}

void APALootDropletActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && DropletLifespanSeconds > 0.0f)
	{
		SetLifeSpan(DropletLifespanSeconds);
	}
}

bool APALootDropletActor::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	if (bIsClaimed)
	{
		return false;
	}

	const APlayerController* OwningPC = OwningPlayerController.Get();
	if (!OwningPC)
	{
		OwningPC = Cast<APlayerController>(GetOwner());
	}

	// AC-6: Chỉ duy nhất máy khách có PlayerController sở hữu mới nhận được Actor replication
	if (OwningPC)
	{
		return RealViewer == OwningPC || ViewTarget == OwningPC || ViewTarget == OwningPC->GetPawn();
	}

	return false;
}

void APALootDropletActor::InitializeDropletWithUID(
	const FGuid& InPlayerUID,
	const FString& InPlayerId,
	APlayerController* InPC,
	const FString& InEncounterId,
	int32 InGoldReward,
	const TArray<FName>& InItemIds)
{
	if (!HasAuthority())
	{
		return;
	}

	OwningPlayerUID = InPlayerUID;
	OwningPlayerId = InPlayerId;
	OwningPlayerController = InPC;
	DropSourceEncounterId = InEncounterId;
	GoldReward = FMath::Max(0, InGoldReward);
	DroppedItemIds = InItemIds;
	bIsClaimed = false;

	if (InPC)
	{
		SetOwner(InPC);
	}
}

void APALootDropletActor::InitializeDroplet(
	const FString& InPlayerId,
	APlayerController* InPC,
	const FString& InEncounterId,
	int32 InGoldReward,
	const TArray<FName>& InItemIds)
{
	FGuid ParsedUID;
	FGuid::Parse(InPlayerId, ParsedUID);
	InitializeDropletWithUID(ParsedUID, InPlayerId, InPC, InEncounterId, InGoldReward, InItemIds);
}

bool APALootDropletActor::CanPlayerSeeOrInteractByUID(const FGuid& InPlayerUID) const
{
	if (bIsClaimed || !InPlayerUID.IsValid())
	{
		return false;
	}

	return InPlayerUID == OwningPlayerUID;
}

bool APALootDropletActor::CanPlayerSeeOrInteract(const FString& InPlayerId) const
{
	if (bIsClaimed || InPlayerId.IsEmpty())
	{
		return false;
	}

	if (OwningPlayerUID.IsValid())
	{
		FGuid ParsedUID;
		if (FGuid::Parse(InPlayerId, ParsedUID))
		{
			return ParsedUID == OwningPlayerUID;
		}
	}

	return InPlayerId == OwningPlayerId;
}

bool APALootDropletActor::IsVisibleToLocalController(const APlayerController* LocalPC) const
{
	if (bIsClaimed || !LocalPC)
	{
		return false;
	}

	return LocalPC == OwningPlayerController.Get() || GetOwner() == LocalPC;
}

bool APALootDropletActor::TryClaimLoot(const FString& InPlayerId, int32& OutGold, TArray<FName>& OutItems)
{
	OutGold = 0;
	OutItems.Empty();

	// Guard bắt buộc trên Dedicated Server
	if (!HasAuthority())
	{
		return false;
	}

	if (!CanPlayerSeeOrInteract(InPlayerId))
	{
		return false;
	}

	bIsClaimed = true;
	OutGold = GoldReward;
	OutItems = DroppedItemIds;

	OnLootClaimed.Broadcast(OwningPlayerId, GoldReward);

	Destroy();
	return true;
}

bool APALootDropletActor::TryClaimLootByUID(const FGuid& InPlayerUID, int32& OutGold, TArray<FName>& OutItems)
{
	OutGold = 0;
	OutItems.Empty();

	if (!HasAuthority())
	{
		return false;
	}

	if (!CanPlayerSeeOrInteractByUID(InPlayerUID))
	{
		return false;
	}

	bIsClaimed = true;
	OutGold = GoldReward;
	OutItems = DroppedItemIds;

	OnLootClaimed.Broadcast(OwningPlayerId, GoldReward);

	Destroy();
	return true;
}

void APALootDropletActor::OnRep_IsClaimed()
{
	if (bIsClaimed)
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}

void APALootDropletActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APALootDropletActor, OwningPlayerUID, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(APALootDropletActor, OwningPlayerId, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(APALootDropletActor, DropSourceEncounterId, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(APALootDropletActor, GoldReward, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(APALootDropletActor, DroppedItemIds, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(APALootDropletActor, bIsClaimed, COND_None);
}
