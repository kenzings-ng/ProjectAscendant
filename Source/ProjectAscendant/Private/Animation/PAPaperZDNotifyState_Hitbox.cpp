// Copyright Project Ascendant. All Rights Reserved.

#include "Animation/PAPaperZDNotifyState_Hitbox.h"
#include "Animation/PAPaperZDAnimInstance.h"
#include "Character/PABaseCharacter.h"

UPAPaperZDNotifyState_Hitbox::UPAPaperZDNotifyState_Hitbox()
{
}

APABaseCharacter* UPAPaperZDNotifyState_Hitbox::GetOwningCharacter(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance)
	{
		return nullptr;
	}

	AActor* OwnerActor = OwningInstance->GetOwningActor();
	return Cast<APABaseCharacter>(OwnerActor);
}

void UPAPaperZDNotifyState_Hitbox::OnNotifyBegin_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	Super::OnNotifyBegin_Implementation(OwningInstance);

	if (APABaseCharacter* Character = GetOwningCharacter(OwningInstance))
	{
		// ADR-0002 & Control Manifest: Hitbox state và combat trace chỉ chạy trên Server Authority
		if (Character->HasAuthority())
		{
			Character->NotifyHitboxOpened();
		}
	}
}

void UPAPaperZDNotifyState_Hitbox::OnNotifyEnd_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	Super::OnNotifyEnd_Implementation(OwningInstance);

	if (APABaseCharacter* Character = GetOwningCharacter(OwningInstance))
	{
		// ADR-0002 & Control Manifest: Hitbox state và combat trace chỉ chạy trên Server Authority
		if (Character->HasAuthority())
		{
			Character->NotifyHitboxClosed();
		}
	}
}
