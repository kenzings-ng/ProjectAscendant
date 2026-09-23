// Copyright Project Ascendant. All Rights Reserved.

#include "Combat/PAGameplayAbility_Dash.h"
#include "Combat/PAStaminaComponent.h"
#include "Combat/AscendantAttributeSet.h"
#include "Character/PABaseCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Curves/CurveFloat.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

// -----------------------------------------------------------------------------
// 1. Pure Mathematical & State Logic (FPADashPipeline)
// -----------------------------------------------------------------------------

bool FPADashPipeline::CanActivateDash(float CurrentStamina, bool bIsExhausted, float Cost)
{
	// AC-1: Nếu nhân vật mang trạng thái Kiệt Sức (State.Exhausted) -> Chặn tuyệt đối
	if (bIsExhausted)
	{
		return false;
	}

	// AC-1: Dash tốn 25 Stamina. Nếu Stamina hiện tại < 25 -> Chặn tuyệt đối
	if (CurrentStamina < Cost)
	{
		return false;
	}

	return true;
}

bool FPADashPipeline::IsWithinIFrameWindow(float ElapsedTime, float StartTime, float EndTime)
{
	// AC-2: Cửa sổ I-Frame bất tử tuyệt đối từ StartTime đến EndTime (0.05s - 0.25s)
	return (ElapsedTime >= StartTime && ElapsedTime <= EndTime);
}

float FPADashPipeline::CalculatePeakVelocity(float Distance, float Duration)
{
	if (Duration <= 0.0f)
	{
		return 0.0f;
	}

	// AC-3: Với linear decay V(t) = V_peak * (1 - t / T), tích phân quãng đường:
	// D = (1 / 2) * V_peak * T  =>  V_peak = (2 * D) / T
	return (2.0f * Distance) / Duration;
}

float FPADashPipeline::CalculateDecayingVelocity(float ElapsedTime, float Distance, float Duration)
{
	if (Duration <= 0.0f || ElapsedTime >= Duration)
	{
		return 0.0f;
	}

	const float PeakVel = CalculatePeakVelocity(Distance, Duration);
	const float NormalizedTime = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);
	return PeakVel * (1.0f - NormalizedTime);
}

float FPADashPipeline::CalculateDisplacementAtTime(float ElapsedTime, float Distance, float Duration)
{
	if (Duration <= 0.0f || ElapsedTime <= 0.0f)
	{
		return 0.0f;
	}

	const float ClampedTime = FMath::Min(ElapsedTime, Duration);
	const float PeakVel = CalculatePeakVelocity(Distance, Duration);

	// S(t) = V_peak * (t - (t^2) / (2 * Duration))
	return PeakVel * (ClampedTime - (ClampedTime * ClampedTime) / (2.0f * Duration));
}

FVector FPADashPipeline::ResolveDashDirection(const FVector& MovementVector, const FVector& AimDirection, const FVector& ForwardVector)
{
	// Ưu tiên 1: MovementVector (khi đang di chuyển)
	FVector Move2D = FVector(MovementVector.X, MovementVector.Y, 0.0f);
	if (!Move2D.IsNearlyZero(0.01f))
	{
		return Move2D.GetSafeNormal();
	}

	// Ưu tiên 2: AimDirection (khi đứng yên nhưng có ngắm hướng)
	FVector Aim2D = FVector(AimDirection.X, AimDirection.Y, 0.0f);
	if (!Aim2D.IsNearlyZero(0.01f))
	{
		return Aim2D.GetSafeNormal();
	}

	// Ưu tiên 3: ForwardVector (hướng mặt nhân vật)
	FVector Fwd2D = FVector(ForwardVector.X, ForwardVector.Y, 0.0f);
	if (!Fwd2D.IsNearlyZero(0.01f))
	{
		return Fwd2D.GetSafeNormal();
	}

	return FVector::ForwardVector;
}

// -----------------------------------------------------------------------------
// 2. Gameplay Ability Implementation (UPAGameplayAbility_Dash)
// -----------------------------------------------------------------------------

UPAGameplayAbility_Dash::UPAGameplayAbility_Dash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Khởi tạo các GameplayTags chuẩn của hệ thống
	TagAbilityDash = FGameplayTag::RequestGameplayTag(FName("Ability.Skill.Dash"), false);
	TagStateDashing = FGameplayTag::RequestGameplayTag(FName("State.Dashing"), false);
	TagStateInvulnerable = FGameplayTag::RequestGameplayTag(FName("State.Invulnerable"), false);
	TagCooldownDash = FGameplayTag::RequestGameplayTag(FName("Cooldown.Dash"), false);
	TagStateExhausted = FGameplayTag::RequestGameplayTag(FName("State.Exhausted"), false);

	if (TagAbilityDash.IsValid())
	{
		FGameplayTagContainer AbilityTagContainer;
		AbilityTagContainer.AddTag(TagAbilityDash);
		SetAssetTags(AbilityTagContainer);
	}

	// Khi kích hoạt Dash, tự động gắn thẻ State.Dashing vào Owner
	if (TagStateDashing.IsValid())
	{
		ActivationOwnedTags.AddTag(TagStateDashing);
	}
}

bool UPAGameplayAbility_Dash::CanActivateAbility(
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

	// AC-3: Kiểm tra hồi chiêu Cooldown.Dash
	if (ASC->HasMatchingGameplayTag(TagCooldownDash))
	{
		return false;
	}

	// AC-1: Kiểm tra trạng thái Kiệt Sức
	bool bIsExhausted = ASC->HasMatchingGameplayTag(TagStateExhausted);

	float CurrentStamina = 0.0f;
	if (const APABaseCharacter* BaseChar = Cast<APABaseCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (BaseChar->IsExhausted())
		{
			bIsExhausted = true;
		}

		if (BaseChar->GetStaminaComponent())
		{
			CurrentStamina = BaseChar->GetStaminaComponent()->GetCurrentStamina();
		}
		else if (const UAscendantAttributeSet* AttrSet = BaseChar->GetAttributeSet())
		{
			CurrentStamina = AttrSet->GetStamina();
		}
	}
	else if (const UAscendantAttributeSet* AttrSet = Cast<UAscendantAttributeSet>(ASC->GetAttributeSet(UAscendantAttributeSet::StaticClass())))
	{
		CurrentStamina = AttrSet->GetStamina();
	}

	// AC-1: Dash costs 25 Stamina. If current Stamina < 25 or character possesses State.Exhausted, activation is strictly blocked on both client and server.
	return FPADashPipeline::CanActivateDash(CurrentStamina, bIsExhausted, DashStaminaCost);
}

void UPAGameplayAbility_Dash::ActivateAbility(
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

	APABaseCharacter* BaseChar = Cast<APABaseCharacter>(ActorInfo->AvatarActor.Get());
	ACharacter* Character = BaseChar ? BaseChar : Cast<ACharacter>(ActorInfo->AvatarActor.Get());

	// AC-1: Tiêu hao 25 Stamina thông qua GameplayEffect chuẩn
	ApplyCostEffect();

	// AC-3: Xác định hướng lướt chuẩn hóa
	FVector MoveInputOrVel = FVector::ZeroVector;
	FVector AimDir = FVector::ZeroVector;
	FVector Fwd = FVector::ForwardVector;

	if (Character)
	{
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			if (!MoveComp->GetCurrentAcceleration().IsNearlyZero())
			{
				MoveInputOrVel = MoveComp->GetCurrentAcceleration();
			}
			else if (!MoveComp->Velocity.IsNearlyZero())
			{
				MoveInputOrVel = MoveComp->Velocity;
			}
		}

		if (BaseChar)
		{
			AimDir = BaseChar->GetCurrentAimDirection();
		}
		Fwd = Character->GetActorForwardVector();
	}

	ResolvedDashDirection = FPADashPipeline::ResolveDashDirection(MoveInputOrVel, AimDir, Fwd);

	// Gán thẻ State.Dashing vào ASC
	if (ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->AddLooseGameplayTag(TagStateDashing);
	}

	// AC-4: Bắn delegate OnDashExecuted cho visual components spawn pixel ghost afterimages
	OnDashExecuted.Broadcast(ResolvedDashDirection, DashDistance);

	// AC-3: Thực hiện lực đẩy 450 units trong 0.35s với Linear Decay thông qua Root Motion Task
	const float PeakVelocity = FPADashPipeline::CalculatePeakVelocity(DashDistance, DashDuration);

	bool bRootMotionTaskStarted = false;
	if (Character && Character->GetCharacterMovement())
	{
		// Tạo đường cong linear decay: tại t = 0 -> 1.0 (PeakVelocity), tại t = 1.0 (hết 0.35s) -> 0.0
		UCurveFloat* LinearDecayCurve = NewObject<UCurveFloat>(GetTransientPackage(), FName(TEXT("DashLinearDecayCurve")));
		if (LinearDecayCurve)
		{
			LinearDecayCurve->FloatCurve.AddKey(0.0f, 1.0f);
			LinearDecayCurve->FloatCurve.AddKey(1.0f, 0.0f);

			UAbilityTask_ApplyRootMotionConstantForce* RootMotionTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
				this,
				FName(TEXT("DashRootMotion")),
				ResolvedDashDirection,
				PeakVelocity,
				DashDuration,
				false,
				LinearDecayCurve,
				ERootMotionFinishVelocityMode::SetVelocity,
				FVector::ZeroVector,
				0.0f,
				false
			);

			if (RootMotionTask)
			{
				RootMotionTask->OnFinish.AddDynamic(this, &UPAGameplayAbility_Dash::OnDashCompleted);
				RootMotionTask->ReadyForActivation();
				bRootMotionTaskStarted = true;
			}
		}
	}

	// Thiết lập Timer điều phối I-Frame và dự phòng thời lượng Dash
	if (UWorld* World = GetWorld())
	{
		ClearActiveTimers();

		// AC-2: t = 0.05s bắt đầu cửa sổ I-Frame
		World->GetTimerManager().SetTimer(
			IFrameStartTimerHandle,
			this,
			&UPAGameplayAbility_Dash::OnIFrameWindowOpened,
			IFrameStartTime,
			false
		);

		// Nếu RootMotionTask không khởi chạy được (ví dụ môi trường Unit Test), dùng timer để hoàn thành cú lướt
		if (!bRootMotionTaskStarted)
		{
			World->GetTimerManager().SetTimer(
				DashDurationTimerHandle,
				this,
				&UPAGameplayAbility_Dash::OnDashCompleted,
				DashDuration,
				false
			);
		}
	}
	else
	{
		// Nếu không có World (chạy headless / isolated unit test), hoàn thành trực tiếp
		OnDashCompleted();
	}
}

void UPAGameplayAbility_Dash::ApplyCostEffect()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	// Áp dụng trừ thể lực thông qua UGameplayEffect chuẩn
	UGameplayEffect* CostEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GE_DashCost")));
	if (CostEffect)
	{
		CostEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

		FGameplayModifierInfo ModInfo;
		ModInfo.Attribute = UAscendantAttributeSet::GetStaminaAttribute();
		ModInfo.ModifierOp = EGameplayModOp::Additive;
		ModInfo.ModifierMagnitude = FScalableFloat(-DashStaminaCost);
		CostEffect->Modifiers.Add(ModInfo);

		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		ASC->ApplyGameplayEffectToSelf(CostEffect, 1.0f, EffectContext);
	}
}

void UPAGameplayAbility_Dash::ApplyInvulnerabilityEffect()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	// AC-2: Áp dụng UGameplayEffect có thời lượng 0.20s gán thẻ State.Invulnerable
	UGameplayEffect* InvulnEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GE_DashInvulnerability")));
	if (InvulnEffect)
	{
		InvulnEffect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
		InvulnEffect->DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(IFrameDuration));

		UTargetTagsGameplayEffectComponent& TargetTagsComp = InvulnEffect->AddComponent<UTargetTagsGameplayEffectComponent>();
		FInheritedTagContainer TagContainer;
		TagContainer.Added.AddTag(TagStateInvulnerable);
		TargetTagsComp.SetAndApplyTargetTagChanges(TagContainer);

		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		ActiveInvulnerabilityHandle = ASC->ApplyGameplayEffectToSelf(InvulnEffect, 1.0f, EffectContext);
		bAppliedInvulnerability = true;
	}
}

void UPAGameplayAbility_Dash::ApplyCooldownEffect()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	// AC-3: Áp dụng UGameplayEffect hồi chiêu 0.5s gán thẻ Cooldown.Dash
	UGameplayEffect* CooldownEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GE_DashCooldown")));
	if (CooldownEffect)
	{
		CooldownEffect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
		CooldownEffect->DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(CooldownDuration));

		UTargetTagsGameplayEffectComponent& TargetTagsComp = CooldownEffect->AddComponent<UTargetTagsGameplayEffectComponent>();
		FInheritedTagContainer TagContainer;
		TagContainer.Added.AddTag(TagCooldownDash);
		TargetTagsComp.SetAndApplyTargetTagChanges(TagContainer);

		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		ASC->ApplyGameplayEffectToSelf(CooldownEffect, 1.0f, EffectContext);
	}
}

void UPAGameplayAbility_Dash::OnIFrameWindowOpened()
{
	// AC-2: Mở cửa sổ I-Frame tại t = 0.05s
	ApplyInvulnerabilityEffect();

	if (UWorld* World = GetWorld())
	{
		// Đặt timer đóng cửa sổ I-frame sau 0.20s
		World->GetTimerManager().SetTimer(
			IFrameEndTimerHandle,
			this,
			&UPAGameplayAbility_Dash::OnIFrameWindowClosed,
			IFrameDuration,
			false
		);
	}
}

void UPAGameplayAbility_Dash::OnIFrameWindowClosed()
{
	// AC-2: Đóng cửa sổ I-Frame tại t = 0.25s
	if (ActiveInvulnerabilityHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(ActiveInvulnerabilityHandle);
		}
		ActiveInvulnerabilityHandle.Invalidate();
	}
	bAppliedInvulnerability = false;
}

void UPAGameplayAbility_Dash::OnDashCompleted()
{
	OnIFrameWindowClosed();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPAGameplayAbility_Dash::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	OnIFrameWindowClosed();

	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveLooseGameplayTag(TagStateDashing);
	}

	// AC-3: Bất kể hoàn thành bình thường hay bị ngắt quãng, luôn áp dụng hồi chiêu Cooldown.Dash 0.5s
	ApplyCooldownEffect();

	ClearActiveTimers();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPAGameplayAbility_Dash::ClearActiveTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IFrameStartTimerHandle);
		World->GetTimerManager().ClearTimer(IFrameEndTimerHandle);
		World->GetTimerManager().ClearTimer(DashDurationTimerHandle);
	}
}
