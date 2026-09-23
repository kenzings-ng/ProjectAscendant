// Copyright Project Ascendant. All Rights Reserved.

#include "Character/PAStoneGolemBoss.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Combat/AscendantAttributeSet.h"
#include "Combat/PAPostureSyncComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "PaperZDAnimationComponent.h"
#include "PaperZDAnimInstance.h"
#include "Engine/Blueprint.h"

APAStoneGolemBoss::APAStoneGolemBoss(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	// Tạo Leash Component và cấu hình thông số theo AC-2
	LeashComponent = CreateDefaultSubobject<UPAMonsterLeashComponent>(TEXT("LeashComponent"));
	if (LeashComponent)
	{
		LeashComponent->SetLeashDistance(2500.0f);
		LeashComponent->SetLeashSpeedMultiplier(1.5f);
	}

	// Cấu hình thông số mặc định cho Stone Golem Boss
	AggroRadius = 800.0f;
	AttackRange = 250.0f;
	SlamCooldown = 3.0f;
	SlamAoERadius = 300.0f;
	SlamBaseDamage = 35.0f;
	SlamPostureDamage = 30.0f;
	BossWalkSpeed = 220.0f;

	bIsPerformingSlam = false;
	bCanSlam = true;

	SetBaseMoveSpeed(BossWalkSpeed);
}

void APAStoneGolemBoss::BeginPlay()
{
	Super::BeginPlay();

	SetBaseMoveSpeed(BossWalkSpeed);

	// Nạp Sprite Flipbook mặc định cho Boss Stone Golem
	if (SpriteComponent)
	{
		if (UPaperFlipbook* GolemIdleFb = Cast<UPaperFlipbook>(StaticLoadObject(UPaperFlipbook::StaticClass(), nullptr, TEXT("/Game/art/characters/boss/flipbooks/FB_Golem_Idle.FB_Golem_Idle"))))
		{
			SpriteComponent->SetFlipbook(GolemIdleFb);
		}
	}

	// Nạp PaperZD Animation Blueprint cho Boss Stone Golem
	if (PaperZDAnimComponent)
	{
		UClass* BossAnimClass = StaticLoadClass(UPaperZDAnimInstance::StaticClass(), nullptr, TEXT("/Game/art/characters/boss/anim/ABP_Boss.ABP_Boss_C"));
		if (!BossAnimClass)
		{
			if (UBlueprint* BossBP = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, TEXT("/Game/art/characters/boss/anim/ABP_Boss.ABP_Boss"))))
			{
				BossAnimClass = BossBP->GeneratedClass;
			}
		}
		if (BossAnimClass)
		{
			PaperZDAnimComponent->SetAnimInstanceClass(BossAnimClass);
		}
	}

	if (LeashComponent)
	{
		LeashComponent->SetSpawnOrigin(GetActorLocation());
		LeashComponent->OnLeashStateChanged.AddDynamic(this, &APAStoneGolemBoss::HandleLeashStateChanged);
		LeashComponent->OnLeashCompleted.AddDynamic(this, &APAStoneGolemBoss::HandleLeashCompleted);
	}
}

void APAStoneGolemBoss::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		UpdateBossAI(DeltaSeconds);
	}
}

void APAStoneGolemBoss::UpdateBossAI(float DeltaSeconds)
{
	// 1. Nếu đang trong trạng thái hồi vị trí (Leash Resetting), bỏ qua truy đuổi
	if (LeashComponent && LeashComponent->IsLeashResetting())
	{
		CurrentTarget = nullptr;
		return;
	}

	// 2. Nếu đang tung đòn Ground Slam, đứng yên dập đất
	if (bIsPerformingSlam)
	{
		return;
	}

	// 3. Nếu chưa có mục tiêu, quét tìm người chơi gần nhất trong bán kính AggroRadius (hỗ trợ Dedicated Server)
	if (!CurrentTarget.IsValid())
	{
		float ClosestDistSq = FMath::Square(AggroRadius);
		AActor* BestTarget = nullptr;

		if (UWorld* World = GetWorld())
		{
			for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
			{
				if (APlayerController* PC = Iterator->Get())
				{
					if (APawn* PlayerPawn = PC->GetPawn())
					{
						if (PlayerPawn != this)
						{
							const float DistSq = FVector::DistSquared2D(GetActorLocation(), PlayerPawn->GetActorLocation());
							if (DistSq <= ClosestDistSq)
							{
								ClosestDistSq = DistSq;
								BestTarget = PlayerPawn;
							}
						}
					}
				}
			}
		}

		if (BestTarget)
		{
			CurrentTarget = BestTarget;
		}
	}

	// 4. Nếu có mục tiêu hợp lệ
	if (CurrentTarget.IsValid())
	{
		// Kiểm tra nếu mục tiêu đã chết, vào vùng an toàn Sanctuary, hoặc ra quá xa khỏi tầm Aggro
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CurrentTarget.Get()))
		{
			static const FGameplayTag TagDead = FGameplayTag::RequestGameplayTag(TEXT("State.Dead"), false);
			static const FGameplayTag TagInSanctuary = FGameplayTag::RequestGameplayTag(TEXT("State.InSanctuary"), false);
			if ((TagDead.IsValid() && TargetASC->HasMatchingGameplayTag(TagDead)) ||
				(TagInSanctuary.IsValid() && TargetASC->HasMatchingGameplayTag(TagInSanctuary)))
			{
				CurrentTarget = nullptr;
				return;
			}
		}

		const float Dist = FVector::Dist2D(GetActorLocation(), CurrentTarget->GetActorLocation());
		if (Dist > AggroRadius * 1.5f)
		{
			CurrentTarget = nullptr;
			return;
		}

		// Hướng quay mặt ngắm về phía mục tiêu
		const FVector ToTarget = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		UpdateAimDirection(ToTarget);

		// Nếu đã áp sát trong tầm đánh (AttackRange <= 250cm)
		if (Dist <= AttackRange)
		{
			TryExecuteGroundSlam();
		}
		else
		{
			// Di chuyển áp sát mục tiêu
			AddMovementInput(ToTarget, 1.0f);
		}
	}
}

bool APAStoneGolemBoss::TryExecuteGroundSlam()
{
	if (!bCanSlam || bIsPerformingSlam)
	{
		return false;
	}

	bIsPerformingSlam = true;
	bCanSlam = false;

	// Gắn tag State.Attacking để PaperZD AnimBP kích hoạt chuyển sang state Slam
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Attacking")));
	}

	// Dừng quán tính di chuyển khi chuẩn bị dập
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}

	// Hẹn giờ kích hoạt gây sát thương AoE tại thời điểm dập búa (0.4s)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SlamDurationTimerHandle,
			this,
			&APAStoneGolemBoss::PerformSlamAoEDamage,
			0.4f,
			false
		);
	}

	return true;
}

void APAStoneGolemBoss::PerformSlamAoEDamage()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		FinishGroundSlam();
		return;
	}

	// Quét vòng tròn AoE bán kính 300cm quanh vị trí của Boss
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape AoESphere = FCollisionShape::MakeSphere(SlamAoERadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const bool bHit = World->OverlapMultiByChannel(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		AoESphere,
		QueryParams
	);

	if (bHit)
	{
		for (const FOverlapResult& Overlap : OverlapResults)
		{
			AActor* HitActor = Overlap.GetActor();
			if (HitActor && HitActor != this)
			{
				if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
				{
					static const FGameplayTag TagInvuln = FGameplayTag::RequestGameplayTag(TEXT("State.Invulnerable"), false);
					static const FGameplayTag TagSanctuary = FGameplayTag::RequestGameplayTag(TEXT("State.InSanctuary"), false);
					if ((TagInvuln.IsValid() && TargetASC->HasMatchingGameplayTag(TagInvuln)) ||
						(TagSanctuary.IsValid() && TargetASC->HasMatchingGameplayTag(TagSanctuary)))
					{
						continue; // Bỏ qua nếu mục tiêu đang có I-frame né lướt hoặc ở trong vùng an toàn
					}

					// 1. Áp dụng sát thương máu lên mục tiêu qua GameplayEffect chuẩn (Server-Authoritative)
					UGameplayEffect* DamageEffect = NewObject<UGameplayEffect>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UGameplayEffect::StaticClass(), TEXT("GE_BossSlamDamage")));
					if (DamageEffect)
					{
						DamageEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
						FGameplayModifierInfo ModInfo;
						ModInfo.Attribute = UAscendantAttributeSet::GetHealthAttribute();
						ModInfo.ModifierOp = EGameplayModOp::Additive;
						ModInfo.ModifierMagnitude = FScalableFloat(-SlamBaseDamage);
						DamageEffect->Modifiers.Add(ModInfo);

						FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
						ContextHandle.AddSourceObject(this);
						TargetASC->ApplyGameplayEffectToSelf(DamageEffect, 1.0f, ContextHandle);
					}

					// 2. Áp dụng sát thương Posture (AC-3: 30 Posture damage)
					UGameplayEffect* PostureEffect = NewObject<UGameplayEffect>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UGameplayEffect::StaticClass(), TEXT("GE_BossSlamPostureDamage")));
					if (PostureEffect)
					{
						PostureEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
						FGameplayModifierInfo ModInfo;
						ModInfo.Attribute = UAscendantAttributeSet::GetPostureAttribute();
						ModInfo.ModifierOp = EGameplayModOp::Additive;
						ModInfo.ModifierMagnitude = FScalableFloat(SlamPostureDamage);
						PostureEffect->Modifiers.Add(ModInfo);

						FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
						ContextHandle.AddSourceObject(this);
						TargetASC->ApplyGameplayEffectToSelf(PostureEffect, 1.0f, ContextHandle);
					}
				}
				else
				{
					// Fallback cho Actor không có GAS
					UGameplayStatics::ApplyDamage(
						HitActor,
						SlamBaseDamage,
						GetController(),
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}
	}

	// Kết thúc chiêu sau 0.2s nữa
	World->GetTimerManager().SetTimer(
		SlamDurationTimerHandle,
		this,
		&APAStoneGolemBoss::FinishGroundSlam,
		0.2f,
		false
	);
}

void APAStoneGolemBoss::FinishGroundSlam()
{
	bIsPerformingSlam = false;

	// Gỡ tag State.Attacking
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Attacking")));
	}

	// Đặt hẹn giờ hồi chiêu SlamCooldown (3.0s)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SlamCooldownTimerHandle,
			this,
			&APAStoneGolemBoss::ResetSlamCooldown,
			SlamCooldown,
			false
		);
	}
}

void APAStoneGolemBoss::ResetSlamCooldown()
{
	bCanSlam = true;
}

void APAStoneGolemBoss::SetTargetActor(AActor* NewTarget)
{
	CurrentTarget = NewTarget;
}

void APAStoneGolemBoss::HandleLeashStateChanged(bool bIsLeashing)
{
	if (bIsLeashing)
	{
		CurrentTarget = nullptr;
		if (bIsPerformingSlam)
		{
			FinishGroundSlam();
		}
	}
}

void APAStoneGolemBoss::HandleLeashCompleted()
{
	CurrentTarget = nullptr;
}
