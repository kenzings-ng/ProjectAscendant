// Copyright Project Ascendant. All Rights Reserved.

#include "Combat/PAGameplayAbility_Finisher.h"
#include "Combat/PAGameplayAbility_MeleeAttack.h"
#include "Combat/AscendantAttributeSet.h"
#include "Combat/PAPostureSyncComponent.h"
#include "Character/PABaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"

UPAGameplayAbility_Finisher::UPAGameplayAbility_Finisher()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	TagAbilityFinisher = FGameplayTag::RequestGameplayTag(FName("Ability.Skill.Finisher"), false);
	TagStateInvulnerable = FGameplayTag::RequestGameplayTag(FName("State.Invulnerable"), false);
	TagStateStunned = FGameplayTag::RequestGameplayTag(FName("State.Stunned"), false);
	TagStateBroken = FGameplayTag::RequestGameplayTag(FName("State.Broken"), false);
	TagStateStaggered = FGameplayTag::RequestGameplayTag(FName("State.Staggered"), false);

	if (TagAbilityFinisher.IsValid())
	{
		FGameplayTagContainer AbilityTagContainer;
		AbilityTagContainer.AddTag(TagAbilityFinisher);
		SetAssetTags(AbilityTagContainer);
	}

	// AC-1: Chặn kích hoạt kỹ năng kết liễu trong Sanctuary bằng ActivationBlockedTags chuẩn của GAS
	static const FGameplayTag TagInSanctuary = FGameplayTag::RequestGameplayTag(FName("State.InSanctuary"), false);
	if (TagInSanctuary.IsValid())
	{
		ActivationBlockedTags.AddTag(TagInSanctuary);
	}
}

bool UPAGameplayAbility_Finisher::ValidateExecutionTarget(const AActor* Attacker, const AActor* Target) const
{
	if (!Attacker || !Target || Target == Attacker)
	{
		return false;
	}

	// AC-3: Kiểm tra cự ly không gian (cự ly 2D <= 250cm)
	FVector Diff = Target->GetActorLocation() - Attacker->GetActorLocation();
	Diff.Z = 0.0f;
	const float Distance = Diff.Size();

	// AC-3: Kiểm tra mục tiêu phải mang thẻ State.Broken hoặc State.Staggered
	bool bTargetBroken = false;
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(Target)))
	{
		bTargetBroken = TargetASC->HasMatchingGameplayTag(TagStateBroken) ||
			(TagStateStaggered.IsValid() && TargetASC->HasMatchingGameplayTag(TagStateStaggered));
	}

	return FPAComboFinisherPipeline::CanExecuteFinisher(Distance, bTargetBroken, MaxExecutionDistance);
}

bool UPAGameplayAbility_Finisher::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid() || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	UAbilitySystemComponent* AttackerASC = ActorInfo->AbilitySystemComponent.Get();
	static const FGameplayTag TagDead = FGameplayTag::RequestGameplayTag(FName("State.Dead"), false);
	static const FGameplayTag TagInSanctuary = FGameplayTag::RequestGameplayTag(FName("State.InSanctuary"), false);

	// Người tung chiêu không được đang chết hoặc bị choáng
	if (AttackerASC->HasMatchingGameplayTag(TagStateStunned) || AttackerASC->HasMatchingGameplayTag(TagDead))
	{
		return false;
	}

	// AC-1 (Story map-002): Chặn tuyệt đối kích hoạt kỹ năng kết liễu khi đang trong Sanctuary
	if (TagInSanctuary.IsValid() && AttackerASC->HasMatchingGameplayTag(TagInSanctuary))
	{
		return false;
	}

	AActor* Attacker = ActorInfo->AvatarActor.Get();
	AActor* Target = ExecutionTarget.Get();
	if (!Target)
	{
		// Tự động quét tìm mục tiêu vỡ thế gần nhất trong tầm kết liễu
		Target = FindBestExecutionTarget(Attacker);
	}

	if (!Target)
	{
		return false;
	}

	return ValidateExecutionTarget(Attacker, Target);
}

void UPAGameplayAbility_Finisher::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bFinisherExecuted = false;

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* Attacker = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!Attacker)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Tiêu chuẩn docs/architecture/control-manifest.md: Server Authority
	const bool bHasAuthority = K2_HasAuthority() || (ActorInfo && ActorInfo->IsNetAuthority());

	// AC-3: Người tung chiêu nhận State.Invulnerable trong 1.5s
	ApplyAttackerInvulnerabilityEffect();

	if (!bHasAuthority && IsLocallyControlled())
	{
		// Phía Client (Local Predicted):
		// 1. Xác định target kết liễu từ EventData hoặc quét tìm target hợp lệ
		if (!ExecutionTarget.IsValid())
		{
			if (TriggerEventData && TriggerEventData->Target)
			{
				ExecutionTarget = const_cast<AActor*>(TriggerEventData->Target.Get());
			}
			else
			{
				ExecutionTarget = FindBestExecutionTarget(Attacker);
			}
		}

		// 2. Replicate Target sang Server qua TargetData
		if (ExecutionTarget.IsValid() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
		{
			FGameplayAbilityTargetDataHandle TargetDataHandle;
			FGameplayAbilityTargetData_ActorArray* ActorData = new FGameplayAbilityTargetData_ActorArray();
			ActorData->TargetActorArray.Add(ExecutionTarget);
			TargetDataHandle.Add(ActorData);

			FGameplayTag ApplicationTag;
			ActorInfo->AbilitySystemComponent->CallServerSetReplicatedTargetData(
				Handle,
				ActivationInfo.GetActivationPredictionKey(),
				TargetDataHandle,
				ApplicationTag,
				ActorInfo->AbilitySystemComponent->ScopedPredictionKey
			);

			// Dự đoán broadcast UI/feedback cục bộ (KHÔNG áp dụng stun/damage lên đối thủ từ client)
			float TargetMaxHP = 1000.0f;
			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ExecutionTarget.Get()))
			{
				if (const UAscendantAttributeSet* TargetAttrSet = Cast<UAscendantAttributeSet>(TargetASC->GetAttributeSet(UAscendantAttributeSet::StaticClass())))
				{
					TargetMaxHP = TargetAttrSet->GetMaxHealth();
				}
			}
			const float PredictedDamage = FPAComboFinisherPipeline::CalculateFinisherDamage(TargetMaxHP, FinisherDamageRatio);
			OnFinisherExecuted.Broadcast(ExecutionTarget.Get(), PredictedDamage);
		}
	}
	else if (bHasAuthority)
	{
		// Phía Server Authority:
		AActor* TargetCandidate = (TriggerEventData && TriggerEventData->Target)
			? const_cast<AActor*>(TriggerEventData->Target.Get())
			: (ExecutionTarget.IsValid() ? ExecutionTarget.Get() : nullptr);

		// Thẩm định mục tiêu nếu đã được truyền trực tiếp
		if (TargetCandidate && ValidateExecutionTarget(Attacker, TargetCandidate))
		{
			ExecutionTarget = TargetCandidate;
			ExecuteFinisherOnTarget(TargetCandidate);
		}
		else
		{
			// Đăng ký nhận TargetData khi Client gửi đến sau (hoặc gọi ngay nếu TargetData đã đến trước)
			if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
			{
				TargetDataDelegateHandle = ActorInfo->AbilitySystemComponent->AbilityTargetDataSetDelegate(
					Handle,
					ActivationInfo.GetActivationPredictionKey()
				).AddUObject(this, &UPAGameplayAbility_Finisher::OnTargetDataReceived);

				ActorInfo->AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(
					Handle,
					ActivationInfo.GetActivationPredictionKey()
				);
			}

			// Dự phòng (cho AI bot hoặc unit test headless không qua client TargetData):
			if (!bFinisherExecuted)
			{
				AActor* FallbackTarget = FindBestExecutionTarget(Attacker);
				if (FallbackTarget && ValidateExecutionTarget(Attacker, FallbackTarget))
				{
					ExecutionTarget = FallbackTarget;
					ExecuteFinisherOnTarget(FallbackTarget);
				}
			}
		}
	}

	// Lên lịch hoàn thành chuỗi kết liễu sau 1.5s
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FinisherTimerHandle);
		World->GetTimerManager().SetTimer(
			FinisherTimerHandle,
			this,
			&UPAGameplayAbility_Finisher::OnFinisherSequenceCompleted,
			FinisherDuration,
			false
		);
	}
	else
	{
		// Trong môi trường test headless không có World timer, kết thúc trực tiếp
		OnFinisherSequenceCompleted();
	}
}

void UPAGameplayAbility_Finisher::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ApplicationTag)
{
	if (bFinisherExecuted)
	{
		return;
	}

	AActor* Attacker = GetAvatarActorFromActorInfo();
	AActor* TargetCandidate = nullptr;
	if (Data.Num() > 0 && Data.Get(0)->GetActors().Num() > 0)
	{
		TargetCandidate = Data.Get(0)->GetActors()[0].Get();
	}

	if (TargetCandidate && ValidateExecutionTarget(Attacker, TargetCandidate))
	{
		ExecutionTarget = TargetCandidate;
		ExecuteFinisherOnTarget(TargetCandidate);
	}

	if (CurrentActorInfo && CurrentActorInfo->AbilitySystemComponent.IsValid())
	{
		CurrentActorInfo->AbilitySystemComponent->ConsumeClientReplicatedTargetData(
			CurrentSpecHandle,
			CurrentActivationInfo.GetActivationPredictionKey()
		);
	}
}

void UPAGameplayAbility_Finisher::ExecuteFinisherOnTarget(AActor* ValidatedTarget)
{
	if (!ValidatedTarget || bFinisherExecuted)
	{
		return;
	}

	const bool bHasAuthority = K2_HasAuthority() || (CurrentActorInfo && CurrentActorInfo->IsNetAuthority());
	if (!bHasAuthority)
	{
		return;
	}

	bFinisherExecuted = true;
	AActor* Attacker = GetAvatarActorFromActorInfo();

	// AC-3: Nạn nhân bị khóa choáng State.Stunned trong 1.5s (Chỉ thực thi trên Server Authority)
	ApplyTargetStunEffect(ValidatedTarget);

	// Áp dụng sát thương và hóa giải trên Server Authority
	float TargetMaxHP = 1000.0f;
	float DamageDealt = 0.0f;

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ValidatedTarget))
	{
		// Đọc Max HP một cách an toàn qua API công khai của AttributeSet
		if (const UAscendantAttributeSet* TargetAttrSet = Cast<UAscendantAttributeSet>(TargetASC->GetAttributeSet(UAscendantAttributeSet::StaticClass())))
		{
			TargetMaxHP = TargetAttrSet->GetMaxHealth();
		}

		DamageDealt = FPAComboFinisherPipeline::CalculateFinisherDamage(TargetMaxHP, FinisherDamageRatio);

		// 1. Áp dụng sát thương chuẩn (25% Max HP pure damage) qua GameplayEffect
		UGameplayEffect* FinisherDamageEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GE_FinisherDamage")));
		if (FinisherDamageEffect)
		{
			FinisherDamageEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
			FGameplayModifierInfo ModInfo;
			ModInfo.Attribute = UAscendantAttributeSet::GetHealthAttribute();
			ModInfo.ModifierOp = EGameplayModOp::Additive;
			ModInfo.ModifierMagnitude = FScalableFloat(-DamageDealt);
			FinisherDamageEffect->Modifiers.Add(ModInfo);

			FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
			ContextHandle.AddInstigator(Attacker, Attacker);
			TargetASC->ApplyGameplayEffectToSelf(FinisherDamageEffect, 1.0f, ContextHandle);
		}

		// 2. Reset Posture về 0 qua GameplayEffect (Override 0.0)
		UGameplayEffect* PostureResetEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GE_FinisherPostureReset")));
		if (PostureResetEffect)
		{
			PostureResetEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
			FGameplayModifierInfo ModInfo;
			ModInfo.Attribute = UAscendantAttributeSet::GetPostureAttribute();
			ModInfo.ModifierOp = EGameplayModOp::Override;
			ModInfo.ModifierMagnitude = FScalableFloat(0.0f);
			PostureResetEffect->Modifiers.Add(ModInfo);

			FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
			ContextHandle.AddInstigator(Attacker, Attacker);
			TargetASC->ApplyGameplayEffectToSelf(PostureResetEffect, 1.0f, ContextHandle);
		}

		// Hóa giải thẻ State.Broken và State.Staggered
		TargetASC->RemoveLooseGameplayTag(TagStateBroken);
		TargetASC->RemoveLooseGameplayTag(TagStateStaggered);
	}
	else
	{
		DamageDealt = FPAComboFinisherPipeline::CalculateFinisherDamage(TargetMaxHP, FinisherDamageRatio);
	}

	// Đồng bộ thông báo cho PostureSyncComponent nếu có
	if (UPAPostureSyncComponent* PostureSync = ValidatedTarget->FindComponentByClass<UPAPostureSyncComponent>())
	{
		APABaseCharacter* AttackerChar = Cast<APABaseCharacter>(Attacker);
		PostureSync->TryExecuteStagger(AttackerChar, TargetMaxHP);
	}

	// AC-3: Bắn delegate OnFinisherExecuted thông báo sát thương
	OnFinisherExecuted.Broadcast(ValidatedTarget, DamageDealt);
}

void UPAGameplayAbility_Finisher::ApplyAttackerInvulnerabilityEffect()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	// AC-3: Áp dụng GameplayEffect 1.5s gán thẻ State.Invulnerable
	UGameplayEffect* InvulnEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GE_FinisherAttackerInvuln")));
	if (InvulnEffect)
	{
		InvulnEffect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
		InvulnEffect->DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(FinisherDuration));

		UTargetTagsGameplayEffectComponent& TargetTagsComp = InvulnEffect->AddComponent<UTargetTagsGameplayEffectComponent>();
		FInheritedTagContainer TagContainer;
		TagContainer.Added.AddTag(TagStateInvulnerable);
		TargetTagsComp.SetAndApplyTargetTagChanges(TagContainer);

		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		ActiveInvulnerabilityHandle = ASC->ApplyGameplayEffectToSelf(InvulnEffect, 1.0f, EffectContext);
	}
	else
	{
		ASC->AddLooseGameplayTag(TagStateInvulnerable);
	}
}

void UPAGameplayAbility_Finisher::ApplyTargetStunEffect(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return;
	}

	// AC-3: Áp dụng GameplayEffect 1.5s gán thẻ State.Stunned
	UGameplayEffect* StunEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GE_FinisherTargetStun")));
	if (StunEffect)
	{
		StunEffect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
		StunEffect->DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(FinisherDuration));

		UTargetTagsGameplayEffectComponent& TargetTagsComp = StunEffect->AddComponent<UTargetTagsGameplayEffectComponent>();
		FInheritedTagContainer TagContainer;
		TagContainer.Added.AddTag(TagStateStunned);
		TargetTagsComp.SetAndApplyTargetTagChanges(TagContainer);

		FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		ActiveTargetStunHandle = TargetASC->ApplyGameplayEffectToSelf(StunEffect, 1.0f, EffectContext);
	}
	else
	{
		TargetASC->AddLooseGameplayTag(TagStateStunned);
	}
}

void UPAGameplayAbility_Finisher::OnFinisherSequenceCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPAGameplayAbility_Finisher::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FinisherTimerHandle);
	}

	// Hủy đăng ký delegate TargetData nếu đang chờ
	if (TargetDataDelegateHandle.IsValid() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->AbilityTargetDataSetDelegate(
			Handle,
			ActivationInfo.GetActivationPredictionKey()
		).Remove(TargetDataDelegateHandle);
		TargetDataDelegateHandle.Reset();
	}

	// Thu hồi hiệu ứng bất tử trên người tung chiêu
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		UAbilitySystemComponent* AttackerASC = ActorInfo->AbilitySystemComponent.Get();
		if (ActiveInvulnerabilityHandle.IsValid())
		{
			AttackerASC->RemoveActiveGameplayEffect(ActiveInvulnerabilityHandle);
			ActiveInvulnerabilityHandle.Invalidate();
		}
		AttackerASC->RemoveLooseGameplayTag(TagStateInvulnerable);
	}

	// Thu hồi hiệu ứng choáng trên nạn nhân
	if (ExecutionTarget.IsValid())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ExecutionTarget.Get()))
		{
			if (ActiveTargetStunHandle.IsValid())
			{
				TargetASC->RemoveActiveGameplayEffect(ActiveTargetStunHandle);
				ActiveTargetStunHandle.Invalidate();
			}
			TargetASC->RemoveLooseGameplayTag(TagStateStunned);
		}
	}

	ExecutionTarget = nullptr;
	bFinisherExecuted = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

AActor* UPAGameplayAbility_Finisher::FindBestExecutionTarget(const AActor* Attacker) const
{
	if (!Attacker || !Attacker->GetWorld())
	{
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	float ClosestDistSq = FMath::Square(MaxExecutionDistance);

	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(MaxExecutionDistance);
	FCollisionQueryParams Params(TEXT("FinisherTargetOverlap"), false, Attacker);

	if (Attacker->GetWorld()->OverlapMultiByChannel(Overlaps, Attacker->GetActorLocation(), FQuat::Identity, ECC_Pawn, Sphere, Params))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* Candidate = Overlap.GetActor();
			if (!Candidate || Candidate == Attacker)
			{
				continue;
			}

			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate))
			{
				const bool bHasBrokenTag = TargetASC->HasMatchingGameplayTag(TagStateBroken) ||
					(TagStateStaggered.IsValid() && TargetASC->HasMatchingGameplayTag(TagStateStaggered));
				if (bHasBrokenTag)
				{
					FVector Diff = Candidate->GetActorLocation() - Attacker->GetActorLocation();
					Diff.Z = 0.0f;
					const float DistSq = Diff.SizeSquared();
					if (DistSq <= ClosestDistSq)
					{
						ClosestDistSq = DistSq;
						BestTarget = Candidate;
					}
				}
			}
		}
	}

	return BestTarget;
}
