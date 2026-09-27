// Copyright Project Ascendant. All Rights Reserved.

#include "Animation/PASpineBossAnimationComponent.h"
#include "Character/PAStoneGolemBoss.h"
#include "PaperFlipbookComponent.h"
#include "PaperZDAnimationComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"

UPASpineBossAnimationComponent::UPASpineBossAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bIsSpineActive = false;
	bIsLockedInAction = false;
}

void UPASpineBossAnimationComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerBoss = Cast<APAStoneGolemBoss>(GetOwner());
	if (AActor* OwnerActor = GetOwner())
	{
		SpineAnimComp = OwnerActor->FindComponentByClass<USpineSkeletonAnimationComponent>();
		SpineRendererComp = OwnerActor->FindComponentByClass<USpineSkeletonRendererComponent>();

		if (SpineAnimComp.IsValid())
		{
			SpineAnimComp->AnimationEvent.AddDynamic(this, &UPASpineBossAnimationComponent::HandleSpineAnimationEvent);
			SpineAnimComp->AnimationComplete.AddDynamic(this, &UPASpineBossAnimationComponent::HandleSpineAnimationComplete);
		}
	}

	if (bIsSpineActive)
	{
		SetSpineModeActive(true);
	}
}

void UPASpineBossAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsSpineActive || !SpineAnimComp.IsValid() || bIsLockedInAction)
	{
		return;
	}

	UpdateLocomotionState();
}

void UPASpineBossAnimationComponent::SetSpineModeActive(bool bActive)
{
	bIsSpineActive = bActive;

	if (!OwnerBoss.IsValid())
	{
		return;
	}

	UPaperFlipbookComponent* SpriteComp = OwnerBoss->GetSpriteComponent();
	UPaperZDAnimationComponent* PaperZDComp = OwnerBoss->GetPaperZDAnimComponent();

	if (bActive)
	{
		// Ẩn Flipbook gốc để nhường chỗ cho Spine Renderer
		if (SpriteComp)
		{
			SpriteComp->SetVisibility(false);
		}
		if (PaperZDComp)
		{
			PaperZDComp->SetActive(false);
		}
		if (SpineRendererComp.IsValid())
		{
			SpineRendererComp->SetVisibility(true);
		}

		// Kích hoạt animation Idle mặc định ban đầu
		if (SpineAnimComp.IsValid())
		{
			SpineAnimComp->SetAnimation(0, IdleAnimName, true);
			CurrentState = EPASpineBossState::Idle;
		}
	}
	else
	{
		// Khôi phục Flipbook gốc và PaperZD
		if (SpriteComp)
		{
			SpriteComp->SetVisibility(true);
		}
		if (PaperZDComp)
		{
			PaperZDComp->SetActive(true);
		}
		if (SpineRendererComp.IsValid())
		{
			SpineRendererComp->SetVisibility(false);
		}
	}
}

void UPASpineBossAnimationComponent::UpdateLocomotionState()
{
	if (!OwnerBoss.IsValid() || !SpineAnimComp.IsValid())
	{
		return;
	}

	const float SpeedSq = OwnerBoss->GetVelocity().SizeSquared2D();
	const bool bIsMoving = SpeedSq > 100.0f; // > 10 cm/s

	if (bIsMoving && CurrentState != EPASpineBossState::Walk)
	{
		CurrentState = EPASpineBossState::Walk;
		UTrackEntry* Track = SpineAnimComp->SetAnimation(0, WalkAnimName, true);
		if (Track)
		{
			Track->SetMixDuration(DefaultMixDuration);
		}
	}
	else if (!bIsMoving && CurrentState != EPASpineBossState::Idle)
	{
		CurrentState = EPASpineBossState::Idle;
		UTrackEntry* Track = SpineAnimComp->SetAnimation(0, IdleAnimName, true);
		if (Track)
		{
			Track->SetMixDuration(DefaultMixDuration);
		}
	}
}

bool UPASpineBossAnimationComponent::PlaySpineSlamAnimation()
{
	if (!SpineAnimComp.IsValid())
	{
		return false;
	}

	bIsLockedInAction = true;
	CurrentState = EPASpineBossState::Slam;

	UTrackEntry* Track = SpineAnimComp->SetAnimation(0, SlamAnimName, false);
	if (Track)
	{
		Track->SetMixDuration(0.1f);
		return true;
	}

	bIsLockedInAction = false;
	return false;
}

bool UPASpineBossAnimationComponent::PlaySpineStaggerAnimation()
{
	if (!SpineAnimComp.IsValid())
	{
		return false;
	}

	bIsLockedInAction = true;
	CurrentState = EPASpineBossState::Stagger;

	UTrackEntry* Track = SpineAnimComp->SetAnimation(0, StaggerAnimName, false);
	if (Track)
	{
		Track->SetMixDuration(0.05f);
		return true;
	}

	bIsLockedInAction = false;
	return false;
}

bool UPASpineBossAnimationComponent::PlaySpineDeathAnimation()
{
	if (!SpineAnimComp.IsValid())
	{
		return false;
	}

	bIsLockedInAction = true;
	CurrentState = EPASpineBossState::Death;

	UTrackEntry* Track = SpineAnimComp->SetAnimation(0, DeathAnimName, false);
	if (Track)
	{
		Track->SetMixDuration(0.1f);
		return true;
	}

	return false;
}

void UPASpineBossAnimationComponent::HandleSpineAnimationEvent(UTrackEntry* TrackEntry, FSpineEvent Event)
{
	OnSpineBossEvent.Broadcast(Event.Name);

	// Nếu sự kiện trùng với điểm chạm đất của chiêu Slam
	if (Event.Name.Equals(SlamImpactEventName, ESearchCase::IgnoreCase))
	{
		if (OwnerBoss.IsValid())
		{
			OwnerBoss->PerformSlamAoEDamage();
		}
	}
}

void UPASpineBossAnimationComponent::HandleSpineAnimationComplete(UTrackEntry* TrackEntry)
{
	if (!TrackEntry)
	{
		return;
	}

	const FString CompletedAnim = TrackEntry->getAnimationName();

	if (CompletedAnim.Equals(SlamAnimName, ESearchCase::IgnoreCase))
	{
		bIsLockedInAction = false;
		CurrentState = EPASpineBossState::Idle;

		if (OwnerBoss.IsValid())
		{
			OwnerBoss->FinishGroundSlam();
		}

		if (SpineAnimComp.IsValid())
		{
			SpineAnimComp->SetAnimation(0, IdleAnimName, true);
		}
	}
	else if (CompletedAnim.Equals(StaggerAnimName, ESearchCase::IgnoreCase))
	{
		bIsLockedInAction = false;
		CurrentState = EPASpineBossState::Idle;

		if (SpineAnimComp.IsValid())
		{
			SpineAnimComp->SetAnimation(0, IdleAnimName, true);
		}
	}
}
