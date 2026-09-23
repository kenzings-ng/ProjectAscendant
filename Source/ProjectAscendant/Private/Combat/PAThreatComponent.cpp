// Copyright Project Ascendant. All Rights Reserved.

#include "Combat/PAThreatComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

UPAThreatComponent::UPAThreatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	ThreatRetargetRatio = 1.30f;       // AC-2: 130%
	ThreatDecayDelay = 3.0f;           // AC-3: 3.0s
	ThreatDecayRate = 0.10f;           // AC-3: 10%/giây
	TauntMultiplier = 5.0f;            // AC-11: 5.0x
	TauntBonusFlatThreat = 100.0f;     // AC-11: Top + 100
	LeashMaxDistance = 2500.0f;        // AC-5: 2500 cm
	RetargetEvaluationInterval = 1.0f; // AC-2: Quét định kỳ 1.0s

	SpawnOrigin = FVector::ZeroVector;
	bIsLeashResetting = false;
	RetargetEvaluationTimer = 0.0f;
}

void UPAThreatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		if (SpawnOrigin.IsNearlyZero())
		{
			SpawnOrigin = Owner->GetActorLocation();
		}
	}
}

void UPAThreatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Dedicated Server là nơi duy nhất tính toán Threat
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return;
	}

	// Nếu đang trong trạng thái hồi vị trí Leash Reset -> kiểm tra đã về gốc chưa
	if (bIsLeashResetting)
	{
		if (AActor* Owner = GetOwner())
		{
			const float DistToSpawn = FVector::Dist2D(Owner->GetActorLocation(), SpawnOrigin);
			if (DistToSpawn <= 50.0f)
			{
				CompleteLeashReset();
			}
		}
		return;
	}

	// 1. Kiểm tra giới hạn kéo quái (AC-5: Leash Boundary Reset 2500 cm)
	if (AActor* Owner = GetOwner())
	{
		CheckLeashDistance(Owner->GetActorLocation());
		if (bIsLeashResetting)
		{
			return;
		}
	}

	// 2. Xử lý suy giảm nộ khí khi không hành động (AC-3: Threat Decay)
	TickThreatDecay(DeltaTime);

	// 3. Đánh giá chu kỳ chuyển đổi mục tiêu (AC-2: 130% Threat Retargeting Rule mỗi 1.0s)
	RetargetEvaluationTimer += DeltaTime;
	if (RetargetEvaluationTimer >= RetargetEvaluationInterval)
	{
		RetargetEvaluationTimer = 0.0f;
		EvaluateThreatRetargeting();
	}
}

float UPAThreatComponent::GetMultiplierForThreatSource(EPAThreatSource Source)
{
	switch (Source)
	{
	case EPAThreatSource::Damage:
		return 1.0f;
	case EPAThreatSource::PostureDamage:
		return 2.5f;
	case EPAThreatSource::Healing:
		return 0.5f;
	case EPAThreatSource::Taunt:
		return 5.0f;
	default:
		return 1.0f;
	}
}

FString UPAThreatComponent::GetKeyForActor(AActor* Actor) const
{
	if (!Actor)
	{
		return TEXT("Unknown");
	}
	return FString::Printf(TEXT("Actor_%s"), *Actor->GetName());
}

float UPAThreatComponent::FindCurrentTopThreat(FString& OutTopId, AActor*& OutTopActor) const
{
	float HighestThreat = 0.0f;
	OutTopId.Empty();
	OutTopActor = nullptr;

	for (const auto& Pair : ThreatTable)
	{
		if (Pair.Value.ThreatScore > HighestThreat)
		{
			HighestThreat = Pair.Value.ThreatScore;
			OutTopId = Pair.Key;
			OutTopActor = Pair.Value.CombatantActor.Get();
		}
	}

	return HighestThreat;
}

void UPAThreatComponent::AddThreat(AActor* InstigatorActor, float RawAmount, EPAThreatSource Source)
{
	if (!InstigatorActor || bIsLeashResetting || RawAmount <= 0.0f)
	{
		return;
	}

	const FString Key = GetKeyForActor(InstigatorActor);
	const float Multiplier = GetMultiplierForThreatSource(Source);
	const float AddedThreat = RawAmount * Multiplier;

	FPAThreatEntry& Entry = ThreatTable.FindOrAdd(Key);
	Entry.CombatantActor = InstigatorActor;
	Entry.CombatantId = Key;
	Entry.ThreatScore += AddedThreat;
	Entry.TimeSinceLastActivity = 0.0f; // Reset bộ đếm không hành động (AC-3)

	// Nếu chưa có mục tiêu hoặc mục tiêu trước đó đã chết/hủy, gán ngay lập tức
	if (CurrentAggroTargetId.IsEmpty() || !CurrentAggroTarget.IsValid())
	{
		EvaluateThreatRetargeting();
	}
}

void UPAThreatComponent::AddThreatById(const FString& InstigatorId, float RawAmount, EPAThreatSource Source)
{
	if (InstigatorId.IsEmpty() || bIsLeashResetting || RawAmount <= 0.0f)
	{
		return;
	}

	const float Multiplier = GetMultiplierForThreatSource(Source);
	const float AddedThreat = RawAmount * Multiplier;

	FPAThreatEntry& Entry = ThreatTable.FindOrAdd(InstigatorId);
	Entry.CombatantId = InstigatorId;
	Entry.ThreatScore += AddedThreat;
	Entry.TimeSinceLastActivity = 0.0f;

	if (CurrentAggroTargetId.IsEmpty())
	{
		EvaluateThreatRetargeting();
	}
}

void UPAThreatComponent::ApplyTaunt(AActor* TaunterActor, float BaseTauntThreat)
{
	if (!TaunterActor || bIsLeashResetting)
	{
		return;
	}

	const FString Key = GetKeyForActor(TaunterActor);
	ApplyTauntById(Key, BaseTauntThreat);

	if (FPAThreatEntry* Entry = ThreatTable.Find(Key))
	{
		Entry->CombatantActor = TaunterActor;
	}
}

void UPAThreatComponent::ApplyTauntById(const FString& TaunterId, float BaseTauntThreat)
{
	if (TaunterId.IsEmpty() || bIsLeashResetting)
	{
		return;
	}

	// 1. Tìm Top Threat hiện tại
	FString TopId;
	AActor* TopActor = nullptr;
	const float CurrentTopThreat = FindCurrentTopThreat(TopId, TopActor);

	// 2. Tính nộ khí tăng cường từ Taunt (AC-11: hệ số 5.0x)
	const float BonusThreatFromAbility = BaseTauntThreat * TauntMultiplier;

	// 3. Snap nộ khí của Tanker lên: Max(TopThreat + 100, CasterThreat + BonusThreatFromAbility)
	FPAThreatEntry& Entry = ThreatTable.FindOrAdd(TaunterId);
	Entry.CombatantId = TaunterId;
	const float TargetSnapThreat = FMath::Max(CurrentTopThreat + TauntBonusFlatThreat, Entry.ThreatScore + BonusThreatFromAbility);
	Entry.ThreatScore = TargetSnapThreat;
	Entry.TimeSinceLastActivity = 0.0f;

	// 4. Taunt ép buộc chuyển mục tiêu tức thì trong 1 tick (AC-11: Instant Target Snap)
	AActor* OldTarget = CurrentAggroTarget.Get();
	CurrentAggroTarget = Entry.CombatantActor;
	CurrentAggroTargetId = TaunterId;

	OnAggroTargetChanged.Broadcast(CurrentAggroTarget.Get(), OldTarget);
}

void UPAThreatComponent::EvaluateThreatRetargeting()
{
	if (bIsLeashResetting || ThreatTable.Num() == 0)
	{
		if (!CurrentAggroTargetId.IsEmpty())
		{
			AActor* OldTarget = CurrentAggroTarget.Get();
			CurrentAggroTarget = nullptr;
			CurrentAggroTargetId.Empty();
			OnAggroTargetChanged.Broadcast(nullptr, OldTarget);
		}
		return;
	}

	FString TopId;
	AActor* TopActor = nullptr;
	const float HighestThreat = FindCurrentTopThreat(TopId, TopActor);

	if (HighestThreat <= 0.0f)
	{
		return;
	}

	// Nếu chưa có mục tiêu hiện tại, chọn ngay người dẫn đầu
	if (CurrentAggroTargetId.IsEmpty())
	{
		AActor* OldTarget = CurrentAggroTarget.Get();
		CurrentAggroTarget = TopActor;
		CurrentAggroTargetId = TopId;
		OnAggroTargetChanged.Broadcast(CurrentAggroTarget.Get(), OldTarget);
		return;
	}

	// Kiểm tra nộ khí của mục tiêu hiện tại
	const float CurrentTargetThreat = GetThreatById(CurrentAggroTargetId);

	// Nếu mục tiêu hiện tại đã chết/bị xóa hoặc nộ khí = 0
	if (CurrentTargetThreat <= 0.0f || TopId == CurrentAggroTargetId)
	{
		if (TopId != CurrentAggroTargetId)
		{
			AActor* OldTarget = CurrentAggroTarget.Get();
			CurrentAggroTarget = TopActor;
			CurrentAggroTargetId = TopId;
			OnAggroTargetChanged.Broadcast(CurrentAggroTarget.Get(), OldTarget);
		}
		return;
	}

	// AC-2: 130% Threat Retargeting Rule
	// Chỉ chuyển target khi HighestThreat >= CurrentTargetThreat * 1.30f
	const float RequiredThreshold = CurrentTargetThreat * ThreatRetargetRatio;
	if (HighestThreat >= RequiredThreshold)
	{
		AActor* OldTarget = CurrentAggroTarget.Get();
		CurrentAggroTarget = TopActor;
		CurrentAggroTargetId = TopId;
		OnAggroTargetChanged.Broadcast(CurrentAggroTarget.Get(), OldTarget);
	}
}

void UPAThreatComponent::TickThreatDecay(float DeltaTime)
{
	if (bIsLeashResetting || ThreatTable.Num() == 0 || DeltaTime <= 0.0f)
	{
		return;
	}

	for (auto& Pair : ThreatTable)
	{
		FPAThreatEntry& Entry = Pair.Value;
		Entry.TimeSinceLastActivity += DeltaTime;

		// AC-3: Nếu không có hành động sau 3.0s, suy giảm 10% mỗi giây
		if (Entry.TimeSinceLastActivity > ThreatDecayDelay)
		{
			const float DecayFraction = ThreatDecayRate * DeltaTime;
			Entry.ThreatScore = FMath::Max(0.0f, Entry.ThreatScore * (1.0f - DecayFraction));
		}
	}
}

bool UPAThreatComponent::CheckLeashDistance(const FVector& CurrentLocation)
{
	if (bIsLeashResetting)
	{
		return true;
	}

	const float CurrentDistance2D = FVector::Dist2D(CurrentLocation, SpawnOrigin);
	if (CurrentDistance2D > LeashMaxDistance)
	{
		TriggerLeashReset();
		return true;
	}

	return false;
}

void UPAThreatComponent::TriggerLeashReset()
{
	if (bIsLeashResetting)
	{
		return;
	}

	bIsLeashResetting = true;

	// AC-5: Xóa bỏ toàn bộ nộ khí và hủy mục tiêu
	ClearThreatTable();

	AActor* OldTarget = CurrentAggroTarget.Get();
	CurrentAggroTarget = nullptr;
	CurrentAggroTargetId.Empty();

	if (OldTarget != nullptr)
	{
		OnAggroTargetChanged.Broadcast(nullptr, OldTarget);
	}

	OnLeashResetStateChanged.Broadcast(true);
}

void UPAThreatComponent::CompleteLeashReset()
{
	if (!bIsLeashResetting)
	{
		return;
	}

	bIsLeashResetting = false;
	OnLeashResetStateChanged.Broadcast(false);
}

float UPAThreatComponent::GetThreat(AActor* CombatantActor) const
{
	if (!CombatantActor)
	{
		return 0.0f;
	}
	return GetThreatById(GetKeyForActor(CombatantActor));
}

float UPAThreatComponent::GetThreatById(const FString& CombatantId) const
{
	if (const FPAThreatEntry* Entry = ThreatTable.Find(CombatantId))
	{
		return Entry->ThreatScore;
	}
	return 0.0f;
}

void UPAThreatComponent::ClearThreatTable()
{
	ThreatTable.Empty();
}

void UPAThreatComponent::OnRep_IsLeashResetting()
{
	OnLeashResetStateChanged.Broadcast(bIsLeashResetting);
}

void UPAThreatComponent::OnRep_AggroTarget()
{
	OnAggroTargetChanged.Broadcast(CurrentAggroTarget.Get(), nullptr);
}

void UPAThreatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPAThreatComponent, bIsLeashResetting);
	DOREPLIFETIME(UPAThreatComponent, CurrentAggroTarget);
}
