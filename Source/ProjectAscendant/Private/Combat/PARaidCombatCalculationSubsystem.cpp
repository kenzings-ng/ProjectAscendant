// Copyright Project Ascendant. All Rights Reserved.

#include "Combat/PARaidCombatCalculationSubsystem.h"

UPARaidCombatCalculationSubsystem::UPARaidCombatCalculationSubsystem()
{
}

// -----------------------------------------------------------------------------
// Rule 1: Max Rule (Stagger Duration Hard Cap)
// -----------------------------------------------------------------------------

float UPARaidCombatCalculationSubsystem::CalculateStaggerDuration(float BaseDuration, float CandidateBonus) const
{
	const float DesiredDuration = BaseDuration + FMath::Max(0.0f, CandidateBonus);
	return FMath::Min(HARD_CAP_STAGGER_DURATION, DesiredDuration);
}

float UPARaidCombatCalculationSubsystem::CalculateMaxStaggerFromMultiple(float BaseDuration, const TArray<float>& CandidateBonuses) const
{
	float MaxBonus = 0.0f;
	for (float Bonus : CandidateBonuses)
	{
		MaxBonus = FMath::Max(MaxBonus, Bonus);
	}

	return CalculateStaggerDuration(BaseDuration, MaxBonus);
}

// -----------------------------------------------------------------------------
// Rule 2: Instigator Only (Execution & Stagger Leech)
// -----------------------------------------------------------------------------

bool UPARaidCombatCalculationSubsystem::TryBeginExecution(const FGuid& PlayerId, float CurrentTime, float Duration)
{
	if (!PlayerId.IsValid())
	{
		return false;
	}

	if (IsExecutionActive(CurrentTime))
	{
		return false; // Đã có người khác chiếm quyền kết liễu
	}

	ActiveExecutorId = PlayerId;
	ExecutionEndTime = CurrentTime + FMath::Max(0.1f, Duration);
	return true;
}

bool UPARaidCombatCalculationSubsystem::IsExecutionActive(float CurrentTime) const
{
	return ActiveExecutorId.IsValid() && (ExecutionEndTime > CurrentTime);
}

float UPARaidCombatCalculationSubsystem::CalculateExecutionDamage(float BossMaxHealth, float ExecutorExecutionDmgPct, const FGuid& PlayerId) const
{
	if (!PlayerId.IsValid() || PlayerId != ActiveExecutorId)
	{
		return 0.0f; // Chỉ Executor mới được tính sát thương
	}

	const float BaseExecutionDamage = BossMaxHealth * 0.25f; // 25% Boss Max HP
	const float BonusMultiplier = 1.0f + (FMath::Max(0.0f, ExecutorExecutionDmgPct) / 100.0f);
	return BaseExecutionDamage * BonusMultiplier;
}

float UPARaidCombatCalculationSubsystem::CalculateStaggerLeech(float DamageDealt, float ExecutorLeechPct, const FGuid& PlayerId) const
{
	if (!PlayerId.IsValid() || PlayerId != ActiveExecutorId)
	{
		return 0.0f; // Chỉ Executor mới được hồi máu
	}

	return DamageDealt * (FMath::Max(0.0f, ExecutorLeechPct) / 100.0f);
}

void UPARaidCombatCalculationSubsystem::EndExecution()
{
	ActiveExecutorId.Invalidate();
	ExecutionEndTime = -1.0f;
}

// -----------------------------------------------------------------------------
// Rule 3: Personal Outgoing & Bleed Manager
// -----------------------------------------------------------------------------

bool UPARaidCombatCalculationSubsystem::ApplyBleed(const FGuid& PlayerId, float BleedDps, float CurrentTime)
{
	if (!PlayerId.IsValid())
	{
		return false;
	}

	// 1. Kiểm tra xem người chơi đã có nguồn Bleed đang hoạt động hay chưa
	for (FPABleedInstance& Instance : ActiveBleeds)
	{
		if (Instance.PlayerId == PlayerId)
		{
			// Tăng stack (tối đa 3), làm mới thời lượng 3.0s
			Instance.Stacks = FMath::Min(MAX_BLEED_STACKS_PER_PLAYER, Instance.Stacks + 1);
			Instance.DamagePerSecond = FMath::Max(Instance.DamagePerSecond, BleedDps);
			Instance.RemainingDuration = BLEED_REFRESH_DURATION;
			return true;
		}
	}

	// 2. Nếu chưa đủ 10 nguồn: Thêm mới trực tiếp với Grace Period = 2.0s
	if (ActiveBleeds.Num() < MAX_ACTIVE_BLEED_SOURCES)
	{
		ActiveBleeds.Add(FPABleedInstance(PlayerId, BleedDps));
		return true;
	}

	// 3. Nếu đã đủ 10 nguồn: Tìm nguồn có Grace Period đã hết (<= 0.0s) và DPS thấp hơn để thay thế
	int32 EvictCandidateIndex = INDEX_NONE;
	float LowestDpsFound = BleedDps;

	for (int32 i = 0; i < ActiveBleeds.Num(); ++i)
	{
		const FPABleedInstance& Existing = ActiveBleeds[i];
		// Nguồn còn trong Grace Period (> 0s) được bảo hộ 100%, TUYỆT ĐỐI KHÔNG BỊ EVICT
		if (Existing.GracePeriodRemaining <= 0.0f)
		{
			const float CurrentSourceDps = Existing.GetCurrentDps();
			if (CurrentSourceDps < LowestDpsFound)
			{
				LowestDpsFound = CurrentSourceDps;
				EvictCandidateIndex = i;
			}
		}
	}

	if (EvictCandidateIndex != INDEX_NONE)
	{
		ActiveBleeds[EvictCandidateIndex] = FPABleedInstance(PlayerId, BleedDps);
		return true;
	}

	// Cả 10 nguồn đều đang trong Grace Period hoặc có DPS cao hơn -> Từ chối
	return false;
}

void UPARaidCombatCalculationSubsystem::TickBleed(float DeltaTime, float& OutTotalDamageDealt)
{
	OutTotalDamageDealt = 0.0f;

	if (DeltaTime <= 0.0f)
	{
		return;
	}

	for (int32 i = ActiveBleeds.Num() - 1; i >= 0; --i)
	{
		FPABleedInstance& Instance = ActiveBleeds[i];

		// Đếm ngược Grace Period
		Instance.GracePeriodRemaining = FMath::Max(0.0f, Instance.GracePeriodRemaining - DeltaTime);

		// Tính sát thương tick
		const float DmgThisTick = Instance.GetCurrentDps() * DeltaTime;
		OutTotalDamageDealt += DmgThisTick;

		// Lưu trữ vĩnh viễn vào Atomic Damage Ledger
		AtomicDamageLedger.FindOrAdd(Instance.PlayerId) += DmgThisTick;

		// Đếm ngược thời lượng
		Instance.RemainingDuration -= DeltaTime;
		if (Instance.RemainingDuration <= 0.0f)
		{
			ActiveBleeds.RemoveAt(i);
		}
	}
}

float UPARaidCombatCalculationSubsystem::GetPlayerRecordedDamage(const FGuid& PlayerId) const
{
	const float* Found = AtomicDamageLedger.Find(PlayerId);
	return Found ? *Found : 0.0f;
}

float UPARaidCombatCalculationSubsystem::GetTotalBleedDPS() const
{
	float TotalDps = 0.0f;
	for (const FPABleedInstance& Instance : ActiveBleeds)
	{
		TotalDps += Instance.GetCurrentDps();
	}

	return FMath::Min(GLOBAL_BLEED_MAX_DPS, TotalDps);
}

void UPARaidCombatCalculationSubsystem::ResetBleedState()
{
	ActiveBleeds.Empty();
	AtomicDamageLedger.Empty();
}

// -----------------------------------------------------------------------------
// Rule 4: Event DR (Diminishing Returns for Simultaneous Parries)
// -----------------------------------------------------------------------------

float UPARaidCombatCalculationSubsystem::CalculateParryPostureReflect(const FGuid& PlayerId, float BasePostureReflect, float CurrentTime)
{
	if (BasePostureReflect <= 0.0f)
	{
		return 0.0f;
	}

	// Kiểm tra cửa sổ 1.0 giây
	if (CurrentParryWindowStartTime < 0.0f || (CurrentTime - CurrentParryWindowStartTime) >= PARRY_WINDOW_DURATION)
	{
		CurrentParryWindowStartTime = CurrentTime;
		ParryCountInCurrentWindow = 0;
	}

	ParryCountInCurrentWindow++;

	float Multiplier = 1.0f;
	if (ParryCountInCurrentWindow == 1)
	{
		Multiplier = 1.0f; // 100% cho cú parry đầu tiên
	}
	else if (ParryCountInCurrentWindow == 2)
	{
		Multiplier = 0.5f; // 50% cho cú parry thứ hai
	}
	else
	{
		Multiplier = 0.25f; // 25% cho các cú parry thứ ba trở đi
	}

	return BasePostureReflect * Multiplier;
}

void UPARaidCombatCalculationSubsystem::ResetParryWindow()
{
	ParryCountInCurrentWindow = 0;
	CurrentParryWindowStartTime = -1.0f;
}
