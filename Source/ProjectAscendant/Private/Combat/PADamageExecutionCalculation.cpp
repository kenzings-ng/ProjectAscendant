// Copyright Project Ascendant. All Rights Reserved.

#include "Combat/PADamageExecutionCalculation.h"
#include "Combat/AscendantAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"

// -----------------------------------------------------------------------------
// 1. Pure Mathematical Formulas (FPACombatFormulas)
// -----------------------------------------------------------------------------

float FPACombatFormulas::CalculateEffectiveDamage(float RawDamage, float Armor, bool bIsInvulnerable, bool bIsInSanctuary)
{
	// AC-1: Nếu mục tiêu đang có I-Frame (State.Invulnerable) hoặc trong Sanctuary (State.InSanctuary), sát thương triệt tiêu về 0
	if (bIsInvulnerable || bIsInSanctuary)
	{
		return 0.0f;
	}

	const float ClampedRaw = FMath::Max(0.0f, RawDamage);
	const float ClampedArmor = FMath::Max(0.0f, Armor);

	// AC-1: DamageTaken = RawDamage * (100 / (100 + Armor))
	const float MitigationFactor = 100.0f / (100.0f + ClampedArmor);
	return ClampedRaw * MitigationFactor;
}

float FPACombatFormulas::CalculatePostureDamage(
	float BaseStagger,
	float StaggerBonus,
	float HitMultiplier,
	bool bIsWeakspot,
	bool bIsPerfectParry,
	float TargetMaxPosture)
{
	// AC-2: Phản đòn hoàn hảo (Perfect Parry) lập tức áp dụng 35% MaxPosture của mục tiêu
	if (bIsPerfectParry)
	{
		const float ClampedMaxPosture = FMath::Max(1.0f, TargetMaxPosture);
		return ClampedMaxPosture * 0.35f;
	}

	const float ClampedBaseStagger = FMath::Max(0.0f, BaseStagger);
	const float ClampedBonus = FMath::Max(0.0f, StaggerBonus);

	// AC-2: Đánh trúng điểm yếu (Weakspot) áp dụng hệ số HitMultiplier = 1.5x
	float EffectiveMultiplier = FMath::Max(0.0f, HitMultiplier);
	if (bIsWeakspot && EffectiveMultiplier < 1.5f)
	{
		EffectiveMultiplier = 1.5f;
	}

	// AC-2: PostureDamage = BaseStagger * (1 + StaggerBonus) * HitMultiplier
	return ClampedBaseStagger * (1.0f + ClampedBonus) * EffectiveMultiplier;
}

float FPACombatFormulas::CalculateStaggerExecutionDamage(float TargetMaxHP, float BaseDamage)
{
	const float ClampedMaxHP = FMath::Max(0.0f, TargetMaxHP);
	const float ClampedBaseDamage = FMath::Max(0.0f, BaseDamage);

	// AC-3: ExecuteDamage = (TargetMaxHP * 0.25) + (BaseDamage * 3.0)
	// Bỏ qua giáp hoàn toàn để hạ gục boss trong 4 chu kỳ kết liễu
	return (ClampedMaxHP * 0.25f) + (ClampedBaseDamage * 3.0f);
}

// -----------------------------------------------------------------------------
// 2. Damage Execution Calculation (UPADamageExecutionCalculation)
// -----------------------------------------------------------------------------

struct FPACombatDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MaxHealth);
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);

	FPACombatDamageStatics()
	{
		// Target captures (Chỉ số của nạn nhân bị tấn công)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAscendantAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAscendantAttributeSet, Health, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAscendantAttributeSet, MaxHealth, Target, false);

		// Source captures (Chỉ số của người ra đòn)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAscendantAttributeSet, AttackPower, Source, true);
	}
};

static const FPACombatDamageStatics& DamageStatics()
{
	static FPACombatDamageStatics DStatics;
	return DStatics;
}

UPADamageExecutionCalculation::UPADamageExecutionCalculation()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().HealthDef);
	RelevantAttributesToCapture.Add(DamageStatics().MaxHealthDef);
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
}

void UPADamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// AC-1: Kiểm tra thẻ Bất Tử State.Invulnerable trên mục tiêu (I-Frame), State.InSanctuary, hoặc State.Immune
	static const FGameplayTag TagInvulnerable = FGameplayTag::RequestGameplayTag(FName("State.Invulnerable"), false);
	static const FGameplayTag TagInSanctuary = FGameplayTag::RequestGameplayTag(FName("State.InSanctuary"), false);
	static const FGameplayTag TagImmune = FGameplayTag::RequestGameplayTag(FName("State.Immune"), false);

	if (TargetTags && (TargetTags->HasTag(TagInvulnerable) || TargetTags->HasTag(TagInSanctuary) || TargetTags->HasTag(TagImmune)))
	{
		// Triệt tiêu sát thương hoàn toàn khi né đòn trúng cửa sổ I-frame, ở trong Sanctuary, hoặc đang LeashReset
		return;
	}

	// Đọc AttackPower của Source
	float AttackPower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluationParameters, AttackPower);
	AttackPower = FMath::Max(0.0f, AttackPower);

	// Đọc MaxHealth của Target
	float TargetMaxHealth = 500.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().MaxHealthDef, EvaluationParameters, TargetMaxHealth);
	TargetMaxHealth = FMath::Max(1.0f, TargetMaxHealth);

	// Kiểm tra trạng thái Choáng vỡ thế State.Staggered trên mục tiêu (AC-3)
	static const FGameplayTag TagStaggered = FGameplayTag::RequestGameplayTag(FName("State.Staggered"), false);
	const bool bIsStaggered = TargetTags && TargetTags->HasTag(TagStaggered);

	float FinalDamage = 0.0f;

	if (bIsStaggered)
	{
		// AC-3: Đòn kết liễu Stagger Execution rút 25% Max HP + 3x Base Damage, bỏ qua giáp
		FinalDamage = FPACombatFormulas::CalculateStaggerExecutionDamage(TargetMaxHealth, AttackPower);
	}
	else
	{
		// AC-1: Tính sát thương chuẩn có trừ giáp
		float TargetArmor = 0.0f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
		TargetArmor = FMath::Max(0.0f, TargetArmor);

		const bool bIsInSanctuary = TargetTags && TargetTags->HasTag(TagInSanctuary);
		FinalDamage = FPACombatFormulas::CalculateEffectiveDamage(AttackPower, TargetArmor, false, bIsInSanctuary);
	}

	if (FinalDamage > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			DamageStatics().HealthProperty,
			EGameplayModOp::Additive,
			-FinalDamage
		));
	}
}

// -----------------------------------------------------------------------------
// 3. Posture Execution Calculation (UPAPostureExecutionCalculation)
// -----------------------------------------------------------------------------

struct FPACombatPostureStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Posture);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MaxPosture);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BaseStagger);
	DECLARE_ATTRIBUTE_CAPTUREDEF(StaggerBonus);

	FPACombatPostureStatics()
	{
		// Target captures (Thế đứng của mục tiêu chịu đòn)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAscendantAttributeSet, Posture, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAscendantAttributeSet, MaxPosture, Target, false);

		// Source captures (Lực phá thế của người ra đòn)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAscendantAttributeSet, BaseStagger, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAscendantAttributeSet, StaggerBonus, Source, true);
	}
};

static const FPACombatPostureStatics& PostureStatics()
{
	static FPACombatPostureStatics PStatics;
	return PStatics;
}

UPAPostureExecutionCalculation::UPAPostureExecutionCalculation()
{
	RelevantAttributesToCapture.Add(PostureStatics().PostureDef);
	RelevantAttributesToCapture.Add(PostureStatics().MaxPostureDef);
	RelevantAttributesToCapture.Add(PostureStatics().BaseStaggerDef);
	RelevantAttributesToCapture.Add(PostureStatics().StaggerBonusDef);
}

void UPAPostureExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Kiểm tra I-Frame: Nếu mục tiêu bất tử thì không nhận Posture damage
	static const FGameplayTag TagInvulnerable = FGameplayTag::RequestGameplayTag(FName("State.Invulnerable"), false);
	if (TargetTags && TargetTags->HasTag(TagInvulnerable))
	{
		return;
	}

	// Đọc MaxPosture của mục tiêu
	float TargetMaxPosture = 100.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PostureStatics().MaxPostureDef, EvaluationParameters, TargetMaxPosture);
	TargetMaxPosture = FMath::Max(1.0f, TargetMaxPosture);

	// Đọc BaseStagger và StaggerBonus từ Source
	float BaseStagger = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PostureStatics().BaseStaggerDef, EvaluationParameters, BaseStagger);
	BaseStagger = FMath::Max(0.0f, BaseStagger);

	float StaggerBonus = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PostureStatics().StaggerBonusDef, EvaluationParameters, StaggerBonus);
	StaggerBonus = FMath::Max(0.0f, StaggerBonus);

	// AC-2: Kiểm tra thẻ Phản đòn hoàn hảo và Đánh điểm yếu
	static const FGameplayTag TagParryCounter = FGameplayTag::RequestGameplayTag(FName("Combat.ParryCounter"), false);
	static const FGameplayTag TagWeakspot = FGameplayTag::RequestGameplayTag(FName("Combat.WeakspotHit"), false);

	const bool bIsPerfectParry = SourceTags && SourceTags->HasTag(TagParryCounter);
	const bool bIsWeakspot = SourceTags && SourceTags->HasTag(TagWeakspot);
	const float HitMultiplier = bIsWeakspot ? 1.5f : 1.0f;

	const float PostureDamage = FPACombatFormulas::CalculatePostureDamage(
		BaseStagger,
		StaggerBonus,
		HitMultiplier,
		bIsWeakspot,
		bIsPerfectParry,
		TargetMaxPosture
	);

	if (PostureDamage > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			PostureStatics().PostureProperty,
			EGameplayModOp::Additive,
			PostureDamage
		));
	}
}
