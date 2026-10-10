// Copyright Project Ascendant. All Rights Reserved.

#include "Network/PAServerRequestValidation.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"

namespace PAServerRequestValidation
{
	APlayerController* ResolveOwningPlayerController(const AActor* Actor)
	{
		// Bounded walk: owner chains are shallow (Component actor -> Pawn -> PlayerController).
		constexpr int32 MaxDepth = 8;
		const AActor* Current = Actor;
		for (int32 Depth = 0; Current && Depth < MaxDepth; ++Depth)
		{
			if (const APlayerController* AsPC = Cast<APlayerController>(Current))
			{
				return const_cast<APlayerController*>(AsPC);
			}

			if (const APawn* AsPawn = Cast<APawn>(Current))
			{
				if (APlayerController* PawnPC = Cast<APlayerController>(AsPawn->GetController()))
				{
					return PawnPC;
				}
			}

			Current = Current->GetOwner();
		}
		return nullptr;
	}

	bool IsComponentOwnedBy(const UActorComponent* Component, const APlayerController* RequestingController)
	{
		if (!Component || !RequestingController)
		{
			return false;
		}
		return ResolveOwningPlayerController(Component->GetOwner()) == RequestingController;
	}

	bool IsActorInCombat(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
		if (!ASC)
		{
			return false;
		}

		static const FGameplayTag TagInCombat = FGameplayTag::RequestGameplayTag(FName("State.InCombat"), false);
		return TagInCombat.IsValid() && ASC->HasMatchingGameplayTag(TagInCombat);
	}
}
