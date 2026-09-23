// Copyright Project Ascendant. All Rights Reserved.

#include "Combat/PAPostureSyncComponent.h"
#include "Character/PABaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"

UPAPostureSyncComponent::UPAPostureSyncComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	bIsStaggered = false;
	FinisherPlayerUID.Invalidate();
	FinisherPlayerId = TEXT("");
	BreakTimestamp = 0.0f;
	CachedTargetMaxHP = 1000.0f;
}

void UPAPostureSyncComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UPAPostureSyncComponent::IsInFinisherExclusiveWindow(float CurrentTime) const
{
	if (!bIsStaggered)
	{
		return false;
	}

	const float Time = GetEffectiveTime(CurrentTime);
	const float Elapsed = Time - BreakTimestamp;
	return Elapsed >= 0.0f && Elapsed < kFinisherExclusiveWindow;
}

bool UPAPostureSyncComponent::CanExecuteByUID(const FGuid& InPlayerUID, float CurrentTime) const
{
	if (!bIsStaggered || !InPlayerUID.IsValid())
	{
		return false;
	}

	const float Time = GetEffectiveTime(CurrentTime);
	const float Elapsed = Time - BreakTimestamp;

	// AC-7: Trong 1.5s đầu tiên, chỉ duy nhất Finisher mới được phép thực thi
	if (Elapsed < kFinisherExclusiveWindow)
	{
		return InPlayerUID == FinisherPlayerUID;
	}

	// Sau 1.5s, mở tự do cho mọi người chơi tham chiến
	return true;
}

bool UPAPostureSyncComponent::CanExecuteById(const FString& InPlayerId, float CurrentTime) const
{
	if (!bIsStaggered || InPlayerId.IsEmpty())
	{
		return false;
	}

	const float Time = GetEffectiveTime(CurrentTime);
	const float Elapsed = Time - BreakTimestamp;

	if (Elapsed < kFinisherExclusiveWindow)
	{
		return InPlayerId == FinisherPlayerId;
	}

	return true;
}

bool UPAPostureSyncComponent::CanExecute(APABaseCharacter* InCharacter, float CurrentTime) const
{
	if (!bIsStaggered || !InCharacter)
	{
		return false;
	}

	// Kiểm tra cự ly không gian (<= 250 cm tương tác) nếu cả 2 actor đều có vị trí thế giới
	if (AActor* OwnerActor = GetOwner())
	{
		const float Dist = FVector::Dist2D(InCharacter->GetActorLocation(), OwnerActor->GetActorLocation());
		if (Dist > kMaxExecutionInteractionDistance)
		{
			return false;
		}
	}

	const float Time = GetEffectiveTime(CurrentTime);
	const float Elapsed = Time - BreakTimestamp;

	// Trong 1.5s đầu, chỉ Finisher được kết liễu
	if (Elapsed < kFinisherExclusiveWindow)
	{
		return InCharacter == FinisherCharacter.Get() || (FinisherPlayerUID.IsValid() && CanExecuteById(GetActorIdentifier(InCharacter), CurrentTime));
	}

	// Sau 1.5s, bất kỳ người chơi nào đứng trong cự ly 250 cm đều được phép
	return true;
}

void UPAPostureSyncComponent::NotifyPostureBroken(APABaseCharacter* InFinisher, const FGuid& InFinisherUID, float TargetMaxHP)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return;
	}

	FinisherCharacter = InFinisher;
	FinisherPlayerUID = InFinisherUID;
	const FString FinisherId = InFinisher ? GetActorIdentifier(InFinisher) : InFinisherUID.ToString();
	NotifyPostureBrokenWithId(FinisherId, TargetMaxHP, -1.0f);
}

void UPAPostureSyncComponent::NotifyPostureBrokenWithId(const FString& InFinisherId, float TargetMaxHP, float SimulatedTime)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return;
	}

	bIsStaggered = true;
	FinisherPlayerId = InFinisherId;
	if (!FinisherPlayerUID.IsValid())
	{
		FGuid::Parse(InFinisherId, FinisherPlayerUID);
	}
	BreakTimestamp = GetEffectiveTime(SimulatedTime);
	CachedTargetMaxHP = FMath::Max(1.0f, TargetMaxHP);

	// Gán tag State.Staggered cho mục tiêu nếu có GAS
	if (APABaseCharacter* OwnerChar = Cast<APABaseCharacter>(GetOwner()))
	{
		ApplyGameplayTagToCharacter(OwnerChar, FName("State.Staggered"), true);
	}

	// Gán tag State.FinisherPriority cho người ra đòn kết liễu nếu có GAS
	if (FinisherCharacter.IsValid())
	{
		ApplyGameplayTagToCharacter(FinisherCharacter.Get(), FName("State.FinisherPriority"), true);
	}

	// Thiết lập hẹn giờ nếu có World hợp lệ
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_FinisherExclusiveWindow);
		World->GetTimerManager().ClearTimer(TimerHandle_StaggerDuration);

		World->GetTimerManager().SetTimer(
			TimerHandle_FinisherExclusiveWindow,
			this,
			&UPAPostureSyncComponent::HandleFinisherWindowExpired,
			kFinisherExclusiveWindow,
			false
		);

		World->GetTimerManager().SetTimer(
			TimerHandle_StaggerDuration,
			this,
			&UPAPostureSyncComponent::RecoverFromStagger,
			kStaggerDuration,
			false
		);
	}

	OnPostureStaggered.Broadcast(FinisherPlayerId, kStaggerDuration);
}

void UPAPostureSyncComponent::HandleFinisherWindowExpired()
{
	if (FinisherCharacter.IsValid())
	{
		ApplyGameplayTagToCharacter(FinisherCharacter.Get(), FName("State.FinisherPriority"), false);
	}

	OnFinisherWindowExpired.Broadcast(FinisherPlayerId);
}

bool UPAPostureSyncComponent::TryExecuteStagger(APABaseCharacter* InCharacter, float TargetMaxHP, float CurrentTime)
{
	if (!InCharacter)
	{
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!CanExecute(InCharacter, CurrentTime))
	{
		return false;
	}

	const float EffectiveMaxHP = TargetMaxHP > 0.0f ? TargetMaxHP : CachedTargetMaxHP;
	const float ExecutionDamage = EffectiveMaxHP * kStaggerExecutionHPRatio;

	InternalClearStagger(true, InCharacter, ExecutionDamage);
	return true;
}

bool UPAPostureSyncComponent::TryExecuteStaggerById(const FString& InPlayerId, float TargetMaxHP, float CurrentTime)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!CanExecuteById(InPlayerId, CurrentTime))
	{
		return false;
	}

	const float EffectiveMaxHP = TargetMaxHP > 0.0f ? TargetMaxHP : CachedTargetMaxHP;
	const float ExecutionDamage = EffectiveMaxHP * kStaggerExecutionHPRatio;

	InternalClearStagger(true, nullptr, ExecutionDamage);
	return true;
}

bool UPAPostureSyncComponent::TryExecuteStaggerByUID(const FGuid& InPlayerUID, float TargetMaxHP, float CurrentTime)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!CanExecuteByUID(InPlayerUID, CurrentTime))
	{
		return false;
	}

	const float EffectiveMaxHP = TargetMaxHP > 0.0f ? TargetMaxHP : CachedTargetMaxHP;
	const float ExecutionDamage = EffectiveMaxHP * kStaggerExecutionHPRatio;

	InternalClearStagger(true, nullptr, ExecutionDamage);
	return true;
}

void UPAPostureSyncComponent::RecoverFromStagger()
{
	if (!bIsStaggered)
	{
		return;
	}

	InternalClearStagger(false, nullptr, 0.0f);
}

void UPAPostureSyncComponent::InternalClearStagger(bool bWasExecuted, APABaseCharacter* ActualExecutor, float ExecutionDamage)
{
	// Dọn dẹp bộ hẹn giờ
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_FinisherExclusiveWindow);
		World->GetTimerManager().ClearTimer(TimerHandle_StaggerDuration);
	}

	// Gỡ bỏ gameplay tags
	if (APABaseCharacter* OwnerChar = Cast<APABaseCharacter>(GetOwner()))
	{
		ApplyGameplayTagToCharacter(OwnerChar, FName("State.Staggered"), false);
	}

	if (FinisherCharacter.IsValid())
	{
		ApplyGameplayTagToCharacter(FinisherCharacter.Get(), FName("State.FinisherPriority"), false);
	}

	bIsStaggered = false;

	if (bWasExecuted)
	{
		// Vinh danh chính xác người bấm nút kết liễu
		APABaseCharacter* FinalExecutor = ActualExecutor ? ActualExecutor : FinisherCharacter.Get();
		OnStaggerExecutionAccepted.Broadcast(FinalExecutor, ExecutionDamage);
	}

	OnStaggerRecovered.Broadcast(bWasExecuted);
}

void UPAPostureSyncComponent::ResetPostureSync()
{
	RecoverFromStagger();
	FinisherPlayerUID.Invalidate();
	FinisherPlayerId = TEXT("");
	FinisherCharacter = nullptr;
	BreakTimestamp = 0.0f;
	CachedTargetMaxHP = 1000.0f;
}

void UPAPostureSyncComponent::ApplyGameplayTagToCharacter(APABaseCharacter* Character, const FName& TagName, bool bAdd)
{
	if (!Character)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
	{
		const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
		if (Tag.IsValid())
		{
			if (bAdd)
			{
				ASC->AddLooseGameplayTag(Tag);
			}
			else
			{
				ASC->RemoveLooseGameplayTag(Tag);
			}
		}
	}
}

FString UPAPostureSyncComponent::GetActorIdentifier(AActor* Actor) const
{
	if (!Actor)
	{
		return TEXT("Unknown");
	}
	return FString::Printf(TEXT("Player_%s"), *Actor->GetName());
}

float UPAPostureSyncComponent::GetEffectiveTime(float SimulatedTime) const
{
	if (SimulatedTime >= 0.0f)
	{
		return SimulatedTime;
	}

	if (const UWorld* World = GetWorld())
	{
		return World->GetTimeSeconds();
	}

	return 0.0f;
}

void UPAPostureSyncComponent::OnRep_IsStaggered()
{
	if (bIsStaggered)
	{
		OnPostureStaggered.Broadcast(FinisherPlayerId, kStaggerDuration);
	}
	else
	{
		OnStaggerRecovered.Broadcast(false);
	}
}

void UPAPostureSyncComponent::OnRep_FinisherPlayerId()
{
}

void UPAPostureSyncComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Tuân thủ Control Manifest §5.4: Dùng DOREPLIFETIME_CONDITION có điều kiện rõ ràng
	DOREPLIFETIME_CONDITION(UPAPostureSyncComponent, bIsStaggered, COND_None);
	DOREPLIFETIME_CONDITION(UPAPostureSyncComponent, FinisherPlayerUID, COND_None);
	DOREPLIFETIME_CONDITION(UPAPostureSyncComponent, FinisherPlayerId, COND_None);
	DOREPLIFETIME_CONDITION(UPAPostureSyncComponent, BreakTimestamp, COND_None);
	DOREPLIFETIME_CONDITION(UPAPostureSyncComponent, CachedTargetMaxHP, COND_None);
}
