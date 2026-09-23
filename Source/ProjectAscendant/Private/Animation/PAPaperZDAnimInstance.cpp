// Copyright Project Ascendant. All Rights Reserved.

#include "Animation/PAPaperZDAnimInstance.h"
#include "Character/PABaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

UPAPaperZDAnimInstance::UPAPaperZDAnimInstance()
	: CurrentSpeed(0.0f)
	, bIsMoving(false)
	, bIsDashing(false)
	, bIsAttacking(false)
	, bIsHurt(false)
	, bIsStunned(false)
	, bIsDead(false)
	, bFacingRight(true)
	, CurrentAimDirection8Way(EPAAimDirection8Way::East)
{
}

void UPAPaperZDAnimInstance::CacheOwnerReferencesIfNeeded()
{
	if (!OwningPABaseCharacter.IsValid())
	{
		AActor* Owner = GetOwningActor();
		if (!Owner)
		{
			Owner = Cast<AActor>(GetOuter());
		}
		if (Owner)
		{
			OwningPABaseCharacter = Cast<APABaseCharacter>(Owner);
			if (OwningPABaseCharacter.IsValid())
			{
				CachedAbilitySystemComponent = OwningPABaseCharacter->GetAbilitySystemComponent();
			}
		}
	}
}

void UPAPaperZDAnimInstance::OnInit_Implementation()
{
	Super::OnInit_Implementation();
	CacheOwnerReferencesIfNeeded();
}

void UPAPaperZDAnimInstance::OnTick_Implementation(float DeltaTime)
{
	Super::OnTick_Implementation(DeltaTime);

	UpdateAnimationVariables(DeltaTime);
	UpdateDirectionalFacing();
}

void UPAPaperZDAnimInstance::UpdateAnimationVariables(float DeltaTime)
{
	CacheOwnerReferencesIfNeeded();

	if (OwningPABaseCharacter.IsValid())
	{
		// Cập nhật vận tốc di chuyển 2D
		CurrentSpeed = OwningPABaseCharacter->GetVelocity().Size2D();
		bIsMoving = CurrentSpeed > 10.0f;

		// Cập nhật hướng ngắm 8 chiều
		CurrentAimDirection8Way = OwningPABaseCharacter->GetCurrent8WayOrientation();

		// Cập nhật trạng thái từ Gameplay Tags trong GAS
		if (CachedAbilitySystemComponent.IsValid())
		{
			static const FGameplayTag TagDashing = FGameplayTag::RequestGameplayTag(TEXT("State.Dashing"), false);
			static const FGameplayTag TagAttacking = FGameplayTag::RequestGameplayTag(TEXT("State.Attacking"), false);
			static const FGameplayTag TagHurt = FGameplayTag::RequestGameplayTag(TEXT("State.Hurt"), false);
			static const FGameplayTag TagStunned = FGameplayTag::RequestGameplayTag(TEXT("State.Stunned"), false);
			static const FGameplayTag TagBroken = FGameplayTag::RequestGameplayTag(TEXT("State.Broken"), false);
			static const FGameplayTag TagDead = FGameplayTag::RequestGameplayTag(TEXT("State.Dead"), false);

			bIsDashing = TagDashing.IsValid() && CachedAbilitySystemComponent->HasMatchingGameplayTag(TagDashing);
			bIsAttacking = TagAttacking.IsValid() && CachedAbilitySystemComponent->HasMatchingGameplayTag(TagAttacking);
			bIsHurt = TagHurt.IsValid() && CachedAbilitySystemComponent->HasMatchingGameplayTag(TagHurt);
			bIsStunned = (TagStunned.IsValid() && CachedAbilitySystemComponent->HasMatchingGameplayTag(TagStunned)) ||
			             (TagBroken.IsValid() && CachedAbilitySystemComponent->HasMatchingGameplayTag(TagBroken));
			bIsDead = TagDead.IsValid() && CachedAbilitySystemComponent->HasMatchingGameplayTag(TagDead);
		}
	}
}

void UPAPaperZDAnimInstance::UpdateDirectionalFacing()
{
	CacheOwnerReferencesIfNeeded();

	if (OwningPABaseCharacter.IsValid())
	{
		// AC-2: Lấy vector trục Phải màn hình trong không gian 2.5D Isometric (CameraYaw = 45 độ)
		const FRotationMatrix CameraRotMatrix(FRotator(0.0f, UPAIsometricMovementMath::kDefaultCameraYaw, 0.0f));
		const FVector CameraRight = CameraRotMatrix.GetUnitAxis(EAxis::Y);

		// Nếu có input ngắm chủ động từ chuột hoặc analog stick
		if (OwningPABaseCharacter->HasActiveAimInput())
		{
			const FVector AimDir = OwningPABaseCharacter->GetCurrentAimDirection();
			const float DotRight = FVector::DotProduct(AimDir.GetSafeNormal2D(), CameraRight);
			bFacingRight = DotRight >= 0.0f;
		}
		else
		{
			// Fallback theo hướng di chuyển (velocity) khi không ngắm chủ động
			const FVector MoveVelocity = OwningPABaseCharacter->GetVelocity();
			if (MoveVelocity.SizeSquared2D() > 100.0f)
			{
				const float DotRight = FVector::DotProduct(MoveVelocity.GetSafeNormal2D(), CameraRight);
				bFacingRight = DotRight >= 0.0f;
			}
		}

		// Xoay thực tế Sprite Component trên APABaseCharacter (Yaw 0 độ = Phải, Yaw 180 độ = Trái)
		OwningPABaseCharacter->SetSpriteFacingRight(bFacingRight);
	}
}

