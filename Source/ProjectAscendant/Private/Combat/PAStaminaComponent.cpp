// Copyright Project Ascendant. All Rights Reserved.

#include "Combat/PAStaminaComponent.h"
#include "Combat/AscendantAttributeSet.h"
#include "Character/PABaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

// -----------------------------------------------------------------------------
// 1. Pure Mathematical & State Logic (FPAStaminaPipeline)
// -----------------------------------------------------------------------------

bool FPAStaminaPipeline::ConsumeStamina(float CurrentStamina, float Cost, float& OutRemainingStamina, bool& bOutIsDesperationRoll)
{
	// Nếu thể lực đã chạm 0 hoặc âm -> Khóa hành động hoàn toàn
	if (CurrentStamina <= 0.0f)
	{
		OutRemainingStamina = 0.0f;
		bOutIsDesperationRoll = false;
		return false;
	}

	// Đủ thể lực cho cú lướt tiêu chuẩn (AC-1)
	if (CurrentStamina >= Cost)
	{
		OutRemainingStamina = CurrentStamina - Cost;
		bOutIsDesperationRoll = false;
		return true;
	}

	// AC-1: Cú Lướt Tuyệt Vọng (Desperation Roll): Stamina > 0 nhưng < 25
	// Vẫn cho phép né thoát thân nhưng xả sạch bình thể lực về 0
	OutRemainingStamina = 0.0f;
	bOutIsDesperationRoll = true;
	return true;
}

float FPAStaminaPipeline::CalculateStaminaAfterTime(float StartStamina, float MaxStamina, float RegenRate, float ElapsedTime, float RegenDelay)
{
	const float ClampedMax = FMath::Max(1.0f, MaxStamina);
	const float ClampedStart = FMath::Clamp(StartStamina, 0.0f, ClampedMax);

	// Trong suốt thời lượng trễ 0.6s (AC-2): thể lực đứng yên không hồi
	if (ElapsedTime <= RegenDelay)
	{
		return ClampedStart;
	}

	// Sau khi hết trễ: hồi phục theo tốc độ 45 điểm/giây
	const float ActiveRegenTime = ElapsedTime - RegenDelay;
	const float Regenerated = ClampedStart + (RegenRate * ActiveRegenTime);
	return FMath::Clamp(Regenerated, 0.0f, ClampedMax);
}

float FPAStaminaPipeline::GetExhaustionDuration(bool bIsDesperationRoll)
{
	// AC-3: 2.2 giây cho Desperation Roll; 1.5 giây cho tiêu hao cạn thông thường
	return bIsDesperationRoll ? kDesperationExhaustionDuration : kStandardExhaustionDuration;
}

bool FPAStaminaPipeline::CanRecoverFromExhaustion(float CurrentStamina, float MaxStamina, float ElapsedExhaustionTime, float RequiredDuration, float ThresholdPct)
{
	// AC-3: Điều kiện kép — Hết thời gian phạt VÀ Thể lực hồi phục vượt mốc 30%
	const bool bTimeExpired = ElapsedExhaustionTime >= RequiredDuration;
	const bool bStaminaSufficient = CurrentStamina >= (MaxStamina * ThresholdPct);
	return bTimeExpired && bStaminaSufficient;
}

float FPAStaminaPipeline::CalculateExhaustedMoveSpeed(float BaseMoveSpeed)
{
	// AC-3: Giảm 25% tốc độ chạy khi kiệt sức (550 -> 412.5 cm/s)
	return BaseMoveSpeed * (1.0f - kMoveSpeedPenaltyPct);
}

// -----------------------------------------------------------------------------
// 2. Component Implementation (UPAStaminaComponent)
// -----------------------------------------------------------------------------

UPAStaminaComponent::UPAStaminaComponent()
{
	// Control Manifest Rule: Tuyệt đối không dùng Tick() để thăm dò hồi thể lực!
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPAStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPAStaminaComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Dọn dẹp toàn bộ Timer handles khi component bị hủy
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RegenDelayTimerHandle);
		World->GetTimerManager().ClearTimer(RegenTickTimerHandle);
		World->GetTimerManager().ClearTimer(ExhaustionTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

bool UPAStaminaComponent::TryConsumeStaminaForDash(bool& bOutDesperationRoll)
{
	bOutDesperationRoll = false;

	// Nếu đang bị khóa lướt hoặc thể lực cạn kiệt thì từ chối ngay
	if (IsDashBlocked())
	{
		return false;
	}

	UAscendantAttributeSet* AttrSet = GetOwnerAttributeSet();
	if (!AttrSet)
	{
		return false;
	}

	const float CurrentStamina = AttrSet->GetStamina();
	float RemainingStamina = 0.0f;
	const bool bCanConsume = FPAStaminaPipeline::ConsumeStamina(CurrentStamina, DashStaminaCost, RemainingStamina, bOutDesperationRoll);

	if (!bCanConsume)
	{
		return false;
	}

	// Cập nhật giá trị thể lực mới vào AttributeSet
	AttrSet->SetStamina(RemainingStamina);

	// Phát sự kiện broadcast
	const float ConsumedAmount = CurrentStamina - RemainingStamina;
	OnStaminaConsumed.Broadcast(ConsumedAmount, bOutDesperationRoll);

	// Kích hoạt trạng thái Kiệt Sức nếu Stamina về 0 hoặc là Desperation Roll
	if (bOutDesperationRoll || RemainingStamina <= 0.0f)
	{
		ApplyExhaustion(bOutDesperationRoll);
	}

	// Đặt lại bộ đếm trễ hồi phục 0.6s (AC-2)
	StartRegenDelayTimer();

	return true;
}

void UPAStaminaComponent::ConsumeStamina(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	UAscendantAttributeSet* AttrSet = GetOwnerAttributeSet();
	if (!AttrSet)
	{
		return;
	}

	const float CurrentStamina = AttrSet->GetStamina();
	const float NewStamina = FMath::Max(0.0f, CurrentStamina - Amount);
	AttrSet->SetStamina(NewStamina);

	OnStaminaConsumed.Broadcast(CurrentStamina - NewStamina, false);

	if (NewStamina <= 0.0f)
	{
		ApplyExhaustion(false);
	}

	StartRegenDelayTimer();
}

bool UPAStaminaComponent::IsDashBlocked() const
{
	if (bIsExhausted)
	{
		return true;
	}

	if (UAbilitySystemComponent* ASC = GetOwnerASC())
	{
		static const FGameplayTag TagDashBlocked = FGameplayTag::RequestGameplayTag(FName("Ability.Block.Dash"), false);
		if (ASC->HasMatchingGameplayTag(TagDashBlocked))
		{
			return true;
		}
	}

	return GetCurrentStamina() <= 0.0f;
}

float UPAStaminaComponent::GetCurrentStamina() const
{
	if (const UAscendantAttributeSet* AttrSet = GetOwnerAttributeSet())
	{
		return AttrSet->GetStamina();
	}
	return 0.0f;
}

float UPAStaminaComponent::GetMaxStamina() const
{
	if (const UAscendantAttributeSet* AttrSet = GetOwnerAttributeSet())
	{
		return AttrSet->GetMaxStamina();
	}
	return 100.0f;
}

void UPAStaminaComponent::StartRegenDelayTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Hủy bỏ chu trình hồi phục hiện thời
	World->GetTimerManager().ClearTimer(RegenDelayTimerHandle);
	World->GetTimerManager().ClearTimer(RegenTickTimerHandle);

	// Kích hoạt Timer trễ 0.6s
	World->GetTimerManager().SetTimer(
		RegenDelayTimerHandle,
		this,
		&UPAStaminaComponent::OnRegenDelayExpired,
		StaminaRegenDelay,
		false
	);
}

void UPAStaminaComponent::OnRegenDelayExpired()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Hết độ trễ 0.6s -> Kích hoạt chu trình hồi phục định kỳ 0.05s
	World->GetTimerManager().SetTimer(
		RegenTickTimerHandle,
		this,
		&UPAStaminaComponent::HandleActiveRegenTick,
		RegenTickInterval,
		true
	);
}

void UPAStaminaComponent::HandleActiveRegenTick()
{
	UAscendantAttributeSet* AttrSet = GetOwnerAttributeSet();
	if (!AttrSet)
	{
		return;
	}

	const float CurrentStamina = AttrSet->GetStamina();
	const float MaxStamina = AttrSet->GetMaxStamina();
	const float RegenRate = AttrSet->GetStaminaRegenRate();

	// Tăng thể lực theo nhịp RegenTickInterval (45.0 * 0.05 = 2.25 điểm mỗi nhịp)
	const float StepStamina = RegenRate * RegenTickInterval;
	const float NewStamina = FMath::Min(MaxStamina, CurrentStamina + StepStamina);
	AttrSet->SetStamina(NewStamina);

	// Nếu đang kiệt sức, kiểm tra xem đã đủ điều kiện hồi phục chưa
	if (bIsExhausted)
	{
		CheckExhaustionRecovery();
	}

	// Đạt tối đa -> dừng timer định kỳ để tiết kiệm CPU
	if (NewStamina >= MaxStamina)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(RegenTickTimerHandle);
		}
	}
}

void UPAStaminaComponent::ApplyExhaustion(bool bFromDesperationRoll)
{
	bIsExhausted = true;
	bIsDesperationRoll = bFromDesperationRoll;
	bExhaustionTimerFinished = false;

	// Cập nhật tốc độ di chuyển trên nhân vật (-25%)
	if (APABaseCharacter* BaseChar = Cast<APABaseCharacter>(GetOwner()))
	{
		BaseChar->SetExhaustedState(true);
	}

	// Gắn các thẻ GameplayTags vào ASC
	UpdateExhaustionTags(true);

	// Đặt timer thời lượng phạt (1.5s hoặc 2.2s)
	const float Duration = FPAStaminaPipeline::GetExhaustionDuration(bFromDesperationRoll);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExhaustionTimerHandle);
		World->GetTimerManager().SetTimer(
			ExhaustionTimerHandle,
			this,
			&UPAStaminaComponent::OnExhaustionDurationExpired,
			Duration,
			false
		);
	}

	OnExhaustionStateChanged.Broadcast(true);
}

void UPAStaminaComponent::OnExhaustionDurationExpired()
{
	bExhaustionTimerFinished = true;
	CheckExhaustionRecovery();
}

void UPAStaminaComponent::CheckExhaustionRecovery()
{
	if (!bIsExhausted || !bExhaustionTimerFinished)
	{
		return;
	}

	const float CurrentStamina = GetCurrentStamina();
	const float MaxStamina = GetMaxStamina();

	// Kiểm tra điều kiện thể lực đã vượt 30% MaxStamina (30.0 điểm)
	if (CurrentStamina >= (MaxStamina * FPAStaminaPipeline::kRecoveryThresholdPct))
	{
		ClearExhaustion();
	}
}

void UPAStaminaComponent::ClearExhaustion()
{
	bIsExhausted = false;
	bIsDesperationRoll = false;
	bExhaustionTimerFinished = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExhaustionTimerHandle);
	}

	// Khôi phục vận tốc di chuyển bình thường
	if (APABaseCharacter* BaseChar = Cast<APABaseCharacter>(GetOwner()))
	{
		BaseChar->SetExhaustedState(false);
	}

	// Tháo bỏ các thẻ State.Exhausted và Ability.Block.Dash
	UpdateExhaustionTags(false);

	OnExhaustionStateChanged.Broadcast(false);
}

void UPAStaminaComponent::UpdateExhaustionTags(bool bEnable)
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return;
	}

	static const FGameplayTag TagExhausted = FGameplayTag::RequestGameplayTag(FName("State.Exhausted"), false);
	static const FGameplayTag TagBlockDash = FGameplayTag::RequestGameplayTag(FName("Ability.Block.Dash"), false);

	if (bEnable)
	{
		ASC->AddLooseGameplayTag(TagExhausted);
		ASC->AddLooseGameplayTag(TagBlockDash);
	}
	else
	{
		ASC->RemoveLooseGameplayTag(TagExhausted);
		ASC->RemoveLooseGameplayTag(TagBlockDash);
	}
}

UAbilitySystemComponent* UPAStaminaComponent::GetOwnerASC() const
{
	if (const APABaseCharacter* BaseChar = Cast<APABaseCharacter>(GetOwner()))
	{
		return BaseChar->GetAbilitySystemComponent();
	}
	return nullptr;
}

UAscendantAttributeSet* UPAStaminaComponent::GetOwnerAttributeSet() const
{
	if (const APABaseCharacter* BaseChar = Cast<APABaseCharacter>(GetOwner()))
	{
		return BaseChar->GetAttributeSet();
	}
	return nullptr;
}
