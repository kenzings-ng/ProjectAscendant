// Copyright Project Ascendant. All Rights Reserved.

#include "World/PAMonsterLeashComponent.h"
#include "Combat/PAThreatComponent.h"
#include "Combat/PAPostureSyncComponent.h"
#include "Combat/AscendantAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UPAMonsterLeashComponent::UPAMonsterLeashComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	TagStateLeashReset = FGameplayTag::RequestGameplayTag(TEXT("State.LeashReset"), false);
	TagStateInvulnerable = FGameplayTag::RequestGameplayTag(TEXT("State.Invulnerable"), false);
	TagStateImmune = FGameplayTag::RequestGameplayTag(TEXT("State.Immune"), false);
}

void UPAMonsterLeashComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner)
	{
		if (SpawnOrigin.IsZero())
		{
			SpawnOrigin = Owner->GetActorLocation();
		}

		if (Owner->HasAuthority())
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					LeashCheckTimerHandle,
					this,
					&UPAMonsterLeashComponent::CheckLeashStatus,
					CheckInterval,
					true
				);
			}
		}
	}

	InitCachedGameplayEffects();
}

void UPAMonsterLeashComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LeashCheckTimerHandle);
	}

	if (bIsLeashResetting)
	{
		RemoveImmunityAndTags();
	}

	Super::EndPlay(EndPlayReason);
}

void UPAMonsterLeashComponent::InitCachedGameplayEffects()
{
	// Cache GameplayEffect bất tử (Zero Runtime Allocations)
	if (!CachedImmunityEffect)
	{
		CachedImmunityEffect = NewObject<UGameplayEffect>(this, FName(TEXT("GE_CachedLeashImmunity")));
		CachedImmunityEffect->DurationPolicy = EGameplayEffectDurationType::Infinite;

		UTargetTagsGameplayEffectComponent& TargetTagsComp = CachedImmunityEffect->AddComponent<UTargetTagsGameplayEffectComponent>();
		FInheritedTagContainer TagContainer;
		if (TagStateLeashReset.IsValid()) TagContainer.Added.AddTag(TagStateLeashReset);
		if (TagStateInvulnerable.IsValid()) TagContainer.Added.AddTag(TagStateInvulnerable);
		if (TagStateImmune.IsValid()) TagContainer.Added.AddTag(TagStateImmune);
		TargetTagsComp.SetAndApplyTargetTagChanges(TagContainer);
	}
}

void UPAMonsterLeashComponent::SetSpawnOrigin(const FVector& InOrigin)
{
	SpawnOrigin = InOrigin;
}

void UPAMonsterLeashComponent::CheckLeashStatus()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FVector CurrentLocation = Owner->GetActorLocation();

	if (!bIsLeashResetting)
	{
		// AC-2: Kiểm tra cự ly nếu vượt quá 2500cm khỏi spawn origin
		if (FPASanctuaryLeashPipeline::IsBeyondLeashDistance(CurrentLocation, SpawnOrigin, LeashDistance))
		{
			TriggerLeashReset();
		}
	}
	else
	{
		// AC-2: Đang trong quá trình chạy về, tiếp tục đẩy lệnh di chuyển về gốc
		ExecuteReturnMovement();

		// AC-2: Kiểm tra đã về đến nơi chưa (tolerance 50cm)
		if (FPASanctuaryLeashPipeline::HasArrivedAtOrigin(CurrentLocation, SpawnOrigin, ArrivalTolerance))
		{
			CompleteLeashReset();
		}
	}
}

void UPAMonsterLeashComponent::ExecuteReturnMovement()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// AC-2: Thúc đẩy Pawn di chuyển theo hướng quay về SpawnOrigin
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		const FVector ReturnDir = FPASanctuaryLeashPipeline::CalculateReturnDirection(Pawn->GetActorLocation(), SpawnOrigin);
		Pawn->AddMovementInput(ReturnDir, 1.0f);
	}
}

void UPAMonsterLeashComponent::TriggerLeashReset()
{
	if (bIsLeashResetting)
	{
		return;
	}

	bIsLeashResetting = true;
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// 1. AC-2: Xóa toàn bộ nộ khí và hủy mục tiêu (drop aggro)
	if (UPAThreatComponent* ThreatComp = Owner->FindComponentByClass<UPAThreatComponent>())
	{
		ThreatComp->TriggerLeashReset();
	}

	// 2. AC-2: Tăng tốc độ chạy về gấp 1.5x walk speed
	if (UCharacterMovementComponent* MoveComp = Owner->FindComponentByClass<UCharacterMovementComponent>())
	{
		CachedOriginalWalkSpeed = MoveComp->MaxWalkSpeed;
		MoveComp->MaxWalkSpeed = FPASanctuaryLeashPipeline::CalculateLeashReturnSpeed(CachedOriginalWalkSpeed, LeashSpeedMultiplier);
	}

	// 3. AC-2: Nhận trạng thái bất tử (State.Invulnerable / State.Immune) và State.LeashReset
	ApplyImmunityAndTags();

	// 4. AC-2: Hồi phục 100% Health và Posture qua Server-Authoritative GameplayEffect
	RegenerateHealthAndPosture();

	// 5. AC-2: Khởi động di chuyển quay về gốc
	ExecuteReturnMovement();

	// 6. Phát delegate thông báo thay đổi trạng thái
	OnLeashStateChanged.Broadcast(true);
}

void UPAMonsterLeashComponent::CompleteLeashReset()
{
	if (!bIsLeashResetting)
	{
		return;
	}

	bIsLeashResetting = false;
	AActor* Owner = GetOwner();

	// 1. Tháo gỡ trạng thái bất tử và tag LeashReset
	RemoveImmunityAndTags();

	// 2. Khôi phục tốc độ di chuyển gốc
	if (Owner)
	{
		if (UCharacterMovementComponent* MoveComp = Owner->FindComponentByClass<UCharacterMovementComponent>())
		{
			if (CachedOriginalWalkSpeed > 0.0f)
			{
				MoveComp->MaxWalkSpeed = CachedOriginalWalkSpeed;
			}
		}

		if (UPAThreatComponent* ThreatComp = Owner->FindComponentByClass<UPAThreatComponent>())
		{
			ThreatComp->CompleteLeashReset();
		}
	}

	// 3. Đảm bảo máu và posture hồi đầy đủ khi kết thúc leash
	RegenerateHealthAndPosture();

	// 4. Phát delegate thông báo hoàn tất hồi vị trí
	OnLeashStateChanged.Broadcast(false);
	OnLeashCompleted.Broadcast();
}

void UPAMonsterLeashComponent::ApplyImmunityAndTags()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (ASC && ASC->AbilityActorInfo.IsValid() && CachedImmunityEffect)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);
		LeashImmunityEffectHandle = ASC->ApplyGameplayEffectToSelf(CachedImmunityEffect, 1.0f, ContextHandle);
	}
}

void UPAMonsterLeashComponent::RemoveImmunityAndTags()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (ASC && LeashImmunityEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(LeashImmunityEffectHandle);
		LeashImmunityEffectHandle.Invalidate();
	}
}

void UPAMonsterLeashComponent::RegenerateHealthAndPosture()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (ASC && ASC->AbilityActorInfo.IsValid())
	{
		float MaxHP = 1000.0f;
		if (const UAscendantAttributeSet* AttrSet = Cast<UAscendantAttributeSet>(ASC->GetAttributeSet(UAscendantAttributeSet::StaticClass())))
		{
			MaxHP = AttrSet->GetMaxHealth();
		}

		// AC-2: Áp dụng Instant GameplayEffect để khôi phục 100% Health và Posture = 0
		UGameplayEffect* RegenEffect = NewObject<UGameplayEffect>(this, FName(TEXT("GE_LeashRegenFull")));
		if (RegenEffect)
		{
			RegenEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

			// Gán Health = MaxHealth (Override)
			FGameplayModifierInfo HealthMod;
			HealthMod.Attribute = UAscendantAttributeSet::GetHealthAttribute();
			HealthMod.ModifierOp = EGameplayModOp::Override;
			HealthMod.ModifierMagnitude = FScalableFloat(MaxHP);
			RegenEffect->Modifiers.Add(HealthMod);

			// Gán Posture = 0.0f (Override - 100% phục hồi thế đứng)
			FGameplayModifierInfo PostureMod;
			PostureMod.Attribute = UAscendantAttributeSet::GetPostureAttribute();
			PostureMod.ModifierOp = EGameplayModOp::Override;
			PostureMod.ModifierMagnitude = FScalableFloat(0.0f);
			RegenEffect->Modifiers.Add(PostureMod);

			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);
			ASC->ApplyGameplayEffectToSelf(RegenEffect, 1.0f, ContextHandle);
		}
	}

	// Đồng bộ thông báo cho PostureSyncComponent nếu có
	if (UPAPostureSyncComponent* PostureSync = Owner->FindComponentByClass<UPAPostureSyncComponent>())
	{
		PostureSync->ResetPostureSync();
	}
}

FVector UPAMonsterLeashComponent::GetReturnDirection() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ZeroVector;
	}

	return FPASanctuaryLeashPipeline::CalculateReturnDirection(Owner->GetActorLocation(), SpawnOrigin);
}
