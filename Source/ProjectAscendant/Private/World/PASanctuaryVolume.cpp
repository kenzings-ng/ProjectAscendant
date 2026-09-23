// Copyright Project Ascendant. All Rights Reserved.

#include "World/PASanctuaryVolume.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Engine/World.h"

// -----------------------------------------------------------------------------
// 1. Pure Mathematical & Logic Pipeline (FPASanctuaryLeashPipeline)
// -----------------------------------------------------------------------------

bool FPASanctuaryLeashPipeline::IsInsideSanctuary(
	const FVector& ActorLocation,
	const FVector& SanctuaryLocation,
	float Radius)
{
	// AC-1: Kiểm tra khoảng cách 2D trên mặt phẳng Isometric
	const float Distance2D = FVector::Dist2D(ActorLocation, SanctuaryLocation);
	return Distance2D <= FMath::Max(0.0f, Radius);
}

bool FPASanctuaryLeashPipeline::IsBeyondLeashDistance(
	const FVector& CurrentLocation,
	const FVector& SpawnOrigin,
	float MaxDistance)
{
	// AC-2: Quái vật bị kéo vượt quá khoảng cách MaxDistance (2500cm)
	const float CurrentDist2D = FVector::Dist2D(CurrentLocation, SpawnOrigin);
	return CurrentDist2D > FMath::Max(0.0f, MaxDistance);
}

float FPASanctuaryLeashPipeline::CalculateLeashReturnSpeed(float BaseWalkSpeed, float Multiplier)
{
	// AC-2: Tốc độ hồi vị trí gấp 1.5x tốc độ di chuyển cơ bản
	return FMath::Max(0.0f, BaseWalkSpeed) * FMath::Max(1.0f, Multiplier);
}

bool FPASanctuaryLeashPipeline::HasArrivedAtOrigin(
	const FVector& CurrentLocation,
	const FVector& SpawnOrigin,
	float Tolerance)
{
	// AC-2: Đã về đến điểm xuất phát trong bán kính sai số cho phép (50cm)
	const float CurrentDist2D = FVector::Dist2D(CurrentLocation, SpawnOrigin);
	return CurrentDist2D <= FMath::Max(0.0f, Tolerance);
}

FVector FPASanctuaryLeashPipeline::CalculateReturnDirection(
	const FVector& CurrentLocation,
	const FVector& SpawnOrigin)
{
	// AC-2: Vector hướng quay về điểm xuất phát trên mặt phẳng XY
	FVector Direction = SpawnOrigin - CurrentLocation;
	Direction.Z = 0.0f;
	return Direction.GetSafeNormal2D();
}

bool FPASanctuaryLeashPipeline::CanActivateOffensiveAbilityInSanctuary(bool bIsInSanctuary)
{
	// AC-1: Đang ở trong Sanctuary thì tuyệt đối không được kích hoạt kỹ năng tấn công
	return !bIsInSanctuary;
}

// -----------------------------------------------------------------------------
// 2. Sanctuary Volume Safezone Actor (APASanctuaryVolume)
// -----------------------------------------------------------------------------

APASanctuaryVolume::APASanctuaryVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(false);

	// Tạo SphereComponent làm RootComponent với bán kính mặc định 1000cm (AC-1)
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SanctuarySphereComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(FPASanctuaryLeashPipeline::kSanctuaryDefaultRadius);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetGenerateOverlapEvents(true);

	// Khởi tạo các Gameplay Tags
	TagStateInSanctuary = FGameplayTag::RequestGameplayTag(TEXT("State.InSanctuary"), false);
	TagStateInvulnerable = FGameplayTag::RequestGameplayTag(TEXT("State.Invulnerable"), false);
	TagZoneSanctuary = FGameplayTag::RequestGameplayTag(TEXT("Zone.Sanctuary"), false);
	TagZoneWilderness = FGameplayTag::RequestGameplayTag(TEXT("Zone.Wilderness"), false);
}

void APASanctuaryVolume::BeginPlay()
{
	Super::BeginPlay();

	if (SphereComponent)
	{
		SphereComponent->SetSphereRadius(SanctuaryRadius);
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &APASanctuaryVolume::OnOverlapBegin);
		SphereComponent->OnComponentEndOverlap.AddDynamic(this, &APASanctuaryVolume::OnOverlapEnd);
	}

	InitCachedGameplayEffects();
}

void APASanctuaryVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		// Dọn dẹp toàn bộ effect khi volume bị hủy
		for (auto& Pair : ActiveSanctuaryEffectHandles)
		{
			if (AActor* Actor = Pair.Key.Get())
			{
				if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
				{
					if (Pair.Value.IsValid())
					{
						ASC->RemoveActiveGameplayEffect(Pair.Value);
					}
				}
			}
		}
		ActiveSanctuaryEffectHandles.Empty();

		for (auto& Pair : ActiveWildernessEffectHandles)
		{
			if (AActor* Actor = Pair.Key.Get())
			{
				if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
				{
					if (Pair.Value.IsValid())
					{
						ASC->RemoveActiveGameplayEffect(Pair.Value);
					}
				}
			}
		}
		ActiveWildernessEffectHandles.Empty();
	}

	Super::EndPlay(EndPlayReason);
}

void APASanctuaryVolume::InitCachedGameplayEffects()
{
	// Khởi tạo và lưu cache GameplayEffect cho Sanctuary (Zero Runtime Allocations)
	if (!CachedSanctuaryEffect)
	{
		CachedSanctuaryEffect = NewObject<UGameplayEffect>(this, FName(TEXT("GE_CachedSanctuarySafezone")));
		CachedSanctuaryEffect->DurationPolicy = EGameplayEffectDurationType::Infinite;

		UTargetTagsGameplayEffectComponent& TargetTagsComp = CachedSanctuaryEffect->AddComponent<UTargetTagsGameplayEffectComponent>();
		FInheritedTagContainer TagContainer;
		if (TagStateInSanctuary.IsValid()) TagContainer.Added.AddTag(TagStateInSanctuary);
		if (TagStateInvulnerable.IsValid()) TagContainer.Added.AddTag(TagStateInvulnerable);
		if (TagZoneSanctuary.IsValid()) TagContainer.Added.AddTag(TagZoneSanctuary);
		TargetTagsComp.SetAndApplyTargetTagChanges(TagContainer);
	}

	// Khởi tạo và lưu cache GameplayEffect cho Wilderness
	if (!CachedWildernessEffect)
	{
		CachedWildernessEffect = NewObject<UGameplayEffect>(this, FName(TEXT("GE_CachedWildernessZone")));
		CachedWildernessEffect->DurationPolicy = EGameplayEffectDurationType::Infinite;

		UTargetTagsGameplayEffectComponent& TargetTagsComp = CachedWildernessEffect->AddComponent<UTargetTagsGameplayEffectComponent>();
		FInheritedTagContainer TagContainer;
		if (TagZoneWilderness.IsValid()) TagContainer.Added.AddTag(TagZoneWilderness);
		TargetTagsComp.SetAndApplyTargetTagChanges(TagContainer);
	}
}

bool APASanctuaryVolume::IsPlayerActor(const AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	// 1. Kiểm tra pawn có đang được người chơi điều khiển hay không
	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		if (Pawn->IsPlayerControlled())
		{
			return true;
		}
	}

	// 2. Kiểm tra ActorTag "Player"
	if (Actor->ActorHasTag(FName(TEXT("Player"))))
	{
		return true;
	}

	// 3. Kiểm tra Gameplay Tag Role.Player trên AbilitySystemComponent
	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor))
	{
		if (const UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			static const FGameplayTag TagRolePlayer = FGameplayTag::RequestGameplayTag(FName(TEXT("Role.Player")), false);
			if (TagRolePlayer.IsValid() && ASC->HasMatchingGameplayTag(TagRolePlayer))
			{
				return true;
			}
		}
	}
	else if (const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(Actor)))
	{
		static const FGameplayTag TagRolePlayer = FGameplayTag::RequestGameplayTag(FName(TEXT("Role.Player")), false);
		if (TagRolePlayer.IsValid() && ASC->HasMatchingGameplayTag(TagRolePlayer))
		{
			return true;
		}
	}

	return false;
}

void APASanctuaryVolume::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// AC-1: Chỉ áp dụng vùng an toàn cho Người chơi (Lọc bỏ Quái vật / Boss)
	if (!HasAuthority() || !OtherActor || !IsPlayerActor(OtherActor))
	{
		return;
	}

	ApplySanctuaryEffects(OtherActor);
}

void APASanctuaryVolume::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!HasAuthority() || !OtherActor || !IsPlayerActor(OtherActor))
	{
		return;
	}

	RemoveSanctuaryEffects(OtherActor);
}

void APASanctuaryVolume::ApplySanctuaryEffects(AActor* TargetActor)
{
	if (!TargetActor || ActiveSanctuaryEffectHandles.Contains(TargetActor))
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (ASC && CachedSanctuaryEffect)
	{
		// Gỡ bỏ effect Wilderness nếu có
		if (FActiveGameplayEffectHandle* WildHandle = ActiveWildernessEffectHandles.Find(TargetActor))
		{
			if (WildHandle->IsValid())
			{
				ASC->RemoveActiveGameplayEffect(*WildHandle);
			}
			ActiveWildernessEffectHandles.Remove(TargetActor);
		}

		// Áp dụng CachedSanctuaryEffect (Iris đồng bộ các thẻ State.InSanctuary, State.Invulnerable, Zone.Sanctuary)
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);
		FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectToSelf(CachedSanctuaryEffect, 1.0f, ContextHandle);
		ActiveSanctuaryEffectHandles.Add(TargetActor, Handle);
	}

	// AC-3: Bắn delegate thông báo chuyển sang Zone.Sanctuary
	OnZoneTransitionChanged.Broadcast(TargetActor, TagZoneSanctuary);
}

void APASanctuaryVolume::RemoveSanctuaryEffects(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (ASC)
	{
		if (FActiveGameplayEffectHandle* HandlePtr = ActiveSanctuaryEffectHandles.Find(TargetActor))
		{
			if (HandlePtr->IsValid())
			{
				ASC->RemoveActiveGameplayEffect(*HandlePtr);
			}
			ActiveSanctuaryEffectHandles.Remove(TargetActor);
		}

		// Rời Sanctuary thì áp dụng CachedWildernessEffect (đồng bộ qua Iris)
		if (CachedWildernessEffect)
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);
			FActiveGameplayEffectHandle WildHandle = ASC->ApplyGameplayEffectToSelf(CachedWildernessEffect, 1.0f, ContextHandle);
			ActiveWildernessEffectHandles.Add(TargetActor, WildHandle);
		}
	}

	// AC-3: Bắn delegate thông báo chuyển sang Zone.Wilderness
	OnZoneTransitionChanged.Broadcast(TargetActor, TagZoneWilderness);
}

bool APASanctuaryVolume::IsActorInsideSanctuary(const AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return false;
	}

	if (ActiveSanctuaryEffectHandles.Contains(TargetActor))
	{
		return true;
	}

	return FPASanctuaryLeashPipeline::IsInsideSanctuary(TargetActor->GetActorLocation(), GetActorLocation(), SanctuaryRadius);
}

float APASanctuaryVolume::GetSanctuaryRadius() const
{
	return SanctuaryRadius;
}

void APASanctuaryVolume::SetSanctuaryRadius(float NewRadius)
{
	SanctuaryRadius = FMath::Max(100.0f, NewRadius);
	if (SphereComponent)
	{
		SphereComponent->SetSphereRadius(SanctuaryRadius);
	}
}
