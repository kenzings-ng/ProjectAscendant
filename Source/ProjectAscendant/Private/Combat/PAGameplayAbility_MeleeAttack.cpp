// Copyright Project Ascendant. All Rights Reserved.

#include "Combat/PAGameplayAbility_MeleeAttack.h"
#include "Combat/AscendantAttributeSet.h"
#include "Combat/PAPostureSyncComponent.h"
#include "Combat/PADamageExecutionCalculation.h"
#include "Network/PANetLagCompensation.h"
#include "Character/PABaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "TimerManager.h"

// -----------------------------------------------------------------------------
// 1. Pure Mathematical & State Logic (FPAComboFinisherPipeline)
// -----------------------------------------------------------------------------

FPAComboStepConfig FPAComboFinisherPipeline::GetComboStepConfig(int32 ComboStep)
{
	static const FPAComboStepConfig kConfigs[3] = {
		{ 1.0f, 10.0f }, // Nhịp 1: 1.0x sát thương, 10 Posture damage (AC-1)
		{ 1.2f, 15.0f }, // Nhịp 2: 1.2x sát thương, 15 Posture damage (AC-1)
		{ 1.6f, 25.0f }  // Nhịp 3: 1.6x sát thương, 25 Posture damage (AC-1)
	};

	if (ComboStep >= 1 && ComboStep <= 3)
	{
		return kConfigs[ComboStep - 1];
	}

	return kConfigs[0];
}

int32 FPAComboFinisherPipeline::AdvanceComboStep(int32 CurrentStep, float TimeSinceLastAttack, float ResetDelay)
{
	// AC-1: Nếu khoảng thời gian giữa 2 đòn liên tiếp vượt quá 1.2s -> Tự động reset về Nhịp 1
	if (TimeSinceLastAttack > ResetDelay || CurrentStep <= 0)
	{
		return 1;
	}

	// AC-1: Tăng tiến chuỗi 3 nhịp: 1 -> 2 -> 3 -> 1
	if (CurrentStep == 1)
	{
		return 2;
	}
	else if (CurrentStep == 2)
	{
		return 3;
	}

	return 1;
}

float FPAComboFinisherPipeline::GetComboDamageMultiplier(int32 ComboStep)
{
	return GetComboStepConfig(ComboStep).DamageMultiplier;
}

float FPAComboFinisherPipeline::GetComboPostureDamage(int32 ComboStep)
{
	return GetComboStepConfig(ComboStep).PostureDamage;
}

bool FPAComboFinisherPipeline::IsTargetInAttackSector(
	const FVector& AttackerPos,
	const FVector& AttackerFacingDirection,
	const FVector& TargetPos,
	float Radius,
	float HalfAngleDeg)
{
	FVector Diff = TargetPos - AttackerPos;
	Diff.Z = 0.0f; // Bỏ qua trục cao trong không gian 2.5D

	const float Distance = Diff.Size();
	if (Distance > Radius || Distance < KINDA_SMALL_NUMBER)
	{
		return false;
	}

	FVector Facing2D = FVector(AttackerFacingDirection.X, AttackerFacingDirection.Y, 0.0f).GetSafeNormal();
	if (Facing2D.IsNearlyZero())
	{
		Facing2D = FVector::ForwardVector;
	}

	const FVector DirToTarget = Diff.GetSafeNormal();
	const float DotProduct = FMath::Clamp(FVector::DotProduct(Facing2D, DirToTarget), -1.0f, 1.0f);
	const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	return (AngleDeg <= HalfAngleDeg);
}

bool FPAComboFinisherPipeline::CanExecuteFinisher(float DistanceToTarget, bool bTargetHasBrokenTag, float MaxDistance)
{
	// AC-3: Cự ly <= 250cm VÀ Mục tiêu mang thẻ State.Broken
	return (DistanceToTarget <= MaxDistance && bTargetHasBrokenTag);
}

float FPAComboFinisherPipeline::CalculateFinisherDamage(float TargetMaxHP, float Ratio)
{
	// AC-3: 25% Max HP sát thương chuẩn
	return FMath::Max(0.0f, TargetMaxHP) * Ratio;
}

// -----------------------------------------------------------------------------
// 2. Gameplay Ability Implementation (UPAGameplayAbility_MeleeAttack)
// -----------------------------------------------------------------------------

UPAGameplayAbility_MeleeAttack::UPAGameplayAbility_MeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	TagAbilityAttack = FGameplayTag::RequestGameplayTag(FName("Ability.Skill.Attack"), false);
	TagStateAttacking = FGameplayTag::RequestGameplayTag(FName("State.Attacking"), false);
	TagStateBroken = FGameplayTag::RequestGameplayTag(FName("State.Broken"), false);
	TagStateStaggered = FGameplayTag::RequestGameplayTag(FName("State.Staggered"), false);

	if (TagAbilityAttack.IsValid())
	{
		FGameplayTagContainer AbilityTagContainer;
		AbilityTagContainer.AddTag(TagAbilityAttack);
		SetAssetTags(AbilityTagContainer);
	}

	if (TagStateAttacking.IsValid())
	{
		ActivationOwnedTags.AddTag(TagStateAttacking);
	}

	// AC-1: Chặn kích hoạt kỹ năng trong Sanctuary bằng ActivationBlockedTags chuẩn của GAS
	static const FGameplayTag TagInSanctuary = FGameplayTag::RequestGameplayTag(FName("State.InSanctuary"), false);
	if (TagInSanctuary.IsValid())
	{
		ActivationBlockedTags.AddTag(TagInSanctuary);
	}
}

bool UPAGameplayAbility_MeleeAttack::CanActivateAbility(
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

	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	static const FGameplayTag TagStunned = FGameplayTag::RequestGameplayTag(FName("State.Stunned"), false);
	static const FGameplayTag TagDead = FGameplayTag::RequestGameplayTag(FName("State.Dead"), false);
	static const FGameplayTag TagInSanctuary = FGameplayTag::RequestGameplayTag(FName("State.InSanctuary"), false);

	if (ASC->HasMatchingGameplayTag(TagStunned) || ASC->HasMatchingGameplayTag(TagDead))
	{
		return false;
	}

	// AC-1 (Story map-002): Chặn tuyệt đối kích hoạt kỹ năng tấn công khi đang trong vùng an toàn Sanctuary
	if (TagInSanctuary.IsValid() && ASC->HasMatchingGameplayTag(TagInSanctuary))
	{
		return false;
	}

	return true;
}

void UPAGameplayAbility_MeleeAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float TimeSinceLastAttack = CurrentTime - LastAttackTimestamp;

	// AC-1: Tính toán nhịp combo kế tiếp với giới hạn 1.2s
	CurrentComboStep = FPAComboFinisherPipeline::AdvanceComboStep(CurrentComboStep, TimeSinceLastAttack, ComboResetDelay);
	LastAttackTimestamp = CurrentTime;

	const float DamageMultiplier = FPAComboFinisherPipeline::GetComboDamageMultiplier(CurrentComboStep);
	const float PostureDamage = FPAComboFinisherPipeline::GetComboPostureDamage(CurrentComboStep);

	// Phát delegate thông báo nhịp combo
	OnComboAttackExecuted.Broadcast(CurrentComboStep, DamageMultiplier, PostureDamage);

	// Đồng bộ hóa Client Activation Timestamp qua TargetData phục vụ Lag Compensation
	const bool bHasAuthority = K2_HasAuthority() || (ActorInfo && ActorInfo->IsNetAuthority());
	if (!bHasAuthority && IsLocallyControlled())
	{
		ReplicatedClientTimestamp = CurrentTime;

		if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
		{
			FGameplayAbilityTargetDataHandle TargetDataHandle;
			FGameplayAbilityTargetData_LocationInfo* TimestampData = new FGameplayAbilityTargetData_LocationInfo();
			TimestampData->TargetLocation.LiteralTransform.SetLocation(FVector(CurrentTime, 0.0f, 0.0f));
			TargetDataHandle.Add(TimestampData);

			FGameplayTag ApplicationTag;
			ActorInfo->AbilitySystemComponent->CallServerSetReplicatedTargetData(
				Handle,
				ActivationInfo.GetActivationPredictionKey(),
				TargetDataHandle,
				ApplicationTag,
				ActorInfo->AbilitySystemComponent->ScopedPredictionKey
			);
		}
	}
	else if (bHasAuthority && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		// Đăng ký nhận TargetData từ client (hoặc gọi ngay nếu TargetData đã đến trước)
		TargetDataDelegateHandle = ActorInfo->AbilitySystemComponent->AbilityTargetDataSetDelegate(
			Handle,
			ActivationInfo.GetActivationPredictionKey()
		).AddUObject(this, &UPAGameplayAbility_MeleeAttack::OnTargetDataReceived);

		ActorInfo->AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(
			Handle,
			ActivationInfo.GetActivationPredictionKey()
		);
	}

	// Lắng nghe sự kiện Event.Combat.Hitbox.Active từ PaperZD AnimNotify
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		static const FGameplayTag TagHitboxActive = FGameplayTag::RequestGameplayTag(FName("Event.Combat.Hitbox.Active"), false);
		HitboxEventDelegateHandle = ActorInfo->AbilitySystemComponent->AddGameplayEventTagContainerDelegate(
			FGameplayTagContainer(TagHitboxActive),
			FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UPAGameplayAbility_MeleeAttack::OnHitboxEventReceived)
		);
	}

	// Thiết lập thời lượng kết thúc đòn chém (AC-1 input chaining)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackDurationTimerHandle);
		World->GetTimerManager().SetTimer(
			AttackDurationTimerHandle,
			this,
			&UPAGameplayAbility_MeleeAttack::OnAttackCompleted,
			AttackDuration,
			false
		);
	}
}

void UPAGameplayAbility_MeleeAttack::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ApplicationTag)
{
	if (Data.Num() > 0)
	{
		const FVector Loc = Data.Get(0)->GetEndPoint();
		if (Loc.X > 0.0f)
		{
			ReplicatedClientTimestamp = Loc.X;
		}
	}
}

void UPAGameplayAbility_MeleeAttack::OnHitboxEventReceived(FGameplayTag MatchingTag, const FGameplayEventData* Payload)
{
	// AC-2: Khi nhận AnimNotify mở hitbox, ưu tiên ReplicatedClientTimestamp từ client để thực hiện Lag Compensation Rewind
	const float Timestamp = (ReplicatedClientTimestamp > 0.0f)
		? ReplicatedClientTimestamp
		: ((Payload && Payload->EventMagnitude > 0.0f) ? Payload->EventMagnitude : (GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0f));
	PerformSectorHitboxTrace(Timestamp);
}

TArray<AActor*> UPAGameplayAbility_MeleeAttack::PerformSectorHitboxTrace(float ClientTimestamp)
{
	TArray<AActor*> HitActors;

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return HitActors;
	}

	// Tiêu chuẩn docs/architecture/control-manifest.md: Dedicated Server Authority
	// Không thực thi va chạm và áp dụng sát thương xác thực trên client
	const bool bHasAuthority = K2_HasAuthority() || (ActorInfo && ActorInfo->IsNetAuthority());
	if (!bHasAuthority)
	{
		return HitActors;
	}

	AActor* Attacker = ActorInfo->AvatarActor.Get();
	const FVector AttackerPos = Attacker->GetActorLocation();

	// Xác định hướng vung kiếm từ aim direction hoặc actor forward
	FVector AttackerFacing = Attacker->GetActorForwardVector();
	if (APABaseCharacter* BaseChar = Cast<APABaseCharacter>(Attacker))
	{
		AttackerFacing = BaseChar->GetCurrentAimDirection();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return HitActors;
	}

	// AC-2 & docs/architecture/control-manifest.md: 3D Chaos Sweep Volumes
	TArray<FHitResult> HitResults;
	const FVector SweepStart = AttackerPos;
	const FVector SweepEnd = AttackerPos + (AttackerFacing * (SectorTraceRadius * 0.5f));
	FCollisionShape SweepShape = FCollisionShape::MakeSphere(SectorTraceRadius);
	FCollisionQueryParams QueryParams(TEXT("MeleeAttackSectorSweep"), false, Attacker);

	World->SweepMultiByChannel(
		HitResults,
		SweepStart,
		SweepEnd,
		FQuat::Identity,
		ECC_Pawn,
		SweepShape,
		QueryParams
	);

	const float DamageMultiplier = FPAComboFinisherPipeline::GetComboDamageMultiplier(CurrentComboStep);
	const float BasePosture = FPAComboFinisherPipeline::GetComboPostureDamage(CurrentComboStep);
	const float FinalDamage = BaseAttackDamage * DamageMultiplier;

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == Attacker || HitActors.Contains(HitActor))
		{
			continue;
		}

		FVector TargetPos = HitActor->GetActorLocation();

		// Thẩm định vị trí qua Lag Compensation nếu có timestamp hợp lệ (AC-2 & ADR-0001)
		if (ClientTimestamp > 0.0f)
		{
			float ClampedTime = 0.0f;
			const float ServerTime = World->GetTimeSeconds();
			if (UPANetcodeMath::ValidateClientTimestamp(ClientTimestamp, ServerTime, UPANetcodeMath::kDefaultMaxLagCompensation, ClampedTime))
			{
				// Tua lại vị trí mục tiêu (Rewind) về thời điểm ClampedTime trong quá khứ
				if (APABaseCharacter* TargetBaseChar = Cast<APABaseCharacter>(HitActor))
				{
					FPAPastTransform RewoundTransform;
					if (TargetBaseChar->GetRewoundTransform(ClampedTime, RewoundTransform))
					{
						TargetPos = RewoundTransform.Location;
					}
				}

				// Thẩm định va chạm tầm quét với vị trí đã được rewind
				if (!UPANetcodeMath::ValidateMeleeSweep(AttackerPos, TargetPos, SectorTraceRadius))
				{
					continue;
				}
			}
		}

		// AC-2: Kiểm tra hình nón quét 90 độ (+/- 45 độ) dựa trên vị trí mục tiêu (đã rewind nếu có lag comp)
		if (FPAComboFinisherPipeline::IsTargetInAttackSector(AttackerPos, AttackerFacing, TargetPos, SectorTraceRadius, SectorTraceHalfAngleDeg))
		{
			HitActors.Add(HitActor);

			// Gây sát thương và posture damage thông qua GameplayEffect chuẩn (Server-Authoritative)
			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
			{
				static const FGameplayTag TagInvuln = FGameplayTag::RequestGameplayTag(FName("State.Invulnerable"), false);
				static const FGameplayTag TagSanctuary = FGameplayTag::RequestGameplayTag(FName("State.InSanctuary"), false);
				if ((TagInvuln.IsValid() && TargetASC->HasMatchingGameplayTag(TagInvuln)) ||
					(TagSanctuary.IsValid() && TargetASC->HasMatchingGameplayTag(TagSanctuary)))
				{
					// Mục tiêu đang mang I-frame né lướt hoặc ở trong vùng an toàn Sanctuary
					continue;
				}

				// 1. Áp dụng sát thương HP qua GameplayEffect chuẩn (không const_cast AttributeSet)
				UGameplayEffect* DamageEffect = NewObject<UGameplayEffect>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UGameplayEffect::StaticClass(), TEXT("GE_MeleeDamage")));
				if (DamageEffect)
				{
					DamageEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
					FGameplayModifierInfo ModInfo;
					ModInfo.Attribute = UAscendantAttributeSet::GetHealthAttribute();
					ModInfo.ModifierOp = EGameplayModOp::Additive;
					ModInfo.ModifierMagnitude = FScalableFloat(-FinalDamage);
					DamageEffect->Modifiers.Add(ModInfo);

					FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
					ContextHandle.AddInstigator(Attacker, Attacker);
					TargetASC->ApplyGameplayEffectToSelf(DamageEffect, 1.0f, ContextHandle);
				}

				// 2. Áp dụng sát thương Posture qua GameplayEffect chuẩn
				UGameplayEffect* PostureEffect = NewObject<UGameplayEffect>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UGameplayEffect::StaticClass(), TEXT("GE_MeleePostureDamage")));
				if (PostureEffect)
				{
					PostureEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
					FGameplayModifierInfo ModInfo;
					ModInfo.Attribute = UAscendantAttributeSet::GetPostureAttribute();
					ModInfo.ModifierOp = EGameplayModOp::Additive;
					ModInfo.ModifierMagnitude = FScalableFloat(BasePosture);
					PostureEffect->Modifiers.Add(ModInfo);

					FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
					ContextHandle.AddInstigator(Attacker, Attacker);
					TargetASC->ApplyGameplayEffectToSelf(PostureEffect, 1.0f, ContextHandle);
				}
			}

			OnSectorHitboxHit.Broadcast(HitActor, FinalDamage, BasePosture);
		}
	}

	return HitActors;
}

void UPAGameplayAbility_MeleeAttack::ResetCombo()
{
	CurrentComboStep = 0;
	LastAttackTimestamp = -100.0f;
}

void UPAGameplayAbility_MeleeAttack::OnAttackCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPAGameplayAbility_MeleeAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackDurationTimerHandle);
	}

	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveLooseGameplayTag(TagStateAttacking);

		if (HitboxEventDelegateHandle.IsValid())
		{
			static const FGameplayTag TagHitboxActive = FGameplayTag::RequestGameplayTag(FName("Event.Combat.Hitbox.Active"), false);
			ActorInfo->AbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(FGameplayTagContainer(TagHitboxActive), HitboxEventDelegateHandle);
			HitboxEventDelegateHandle.Reset();
		}

		if (TargetDataDelegateHandle.IsValid())
		{
			ActorInfo->AbilitySystemComponent->AbilityTargetDataSetDelegate(
				Handle,
				ActivationInfo.GetActivationPredictionKey()
			).Remove(TargetDataDelegateHandle);
			TargetDataDelegateHandle.Reset();
		}
	}

	ReplicatedClientTimestamp = -1.0f;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
