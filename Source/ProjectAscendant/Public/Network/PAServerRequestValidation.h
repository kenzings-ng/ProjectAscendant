// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AActor;
class APawn;
class APlayerController;
class UActorComponent;

/**
 * PAServerRequestValidation (X11a, DECISIONS §11 / ADR-0003 / control-manifest "Dedicated Server Authority")
 *
 * Server-side helpers used by the Blacksmith and Merchant Server RPCs to identify the requesting player and
 * to verify that client-supplied component pointers (Inventory / Wallet) actually belong to that player.
 *
 * Requesting player: a Server RPC on an actor (or one of its components) can only be sent by the owning
 * connection of that actor, so the requesting player is the APlayerController found on the actor's owner chain.
 * For actors without a player owner (NPC forges / merchants placed in the world) this resolves to nullptr and
 * the request is rejected. Routing those RPCs through a player-owned actor is X11b.
 */
namespace PAServerRequestValidation
{
	/** Walks the owner chain of Actor (Pawn -> Controller first) and returns the first APlayerController, or nullptr. */
	PROJECTASCENDANT_API APlayerController* ResolveOwningPlayerController(const AActor* Actor);

	/** True only if Component is non-null and its owning actor resolves to RequestingController. */
	PROJECTASCENDANT_API bool IsComponentOwnedBy(const UActorComponent* Component, const APlayerController* RequestingController);

	/** True if Actor's ability system component carries the State.InCombat tag (server-side combat state). */
	PROJECTASCENDANT_API bool IsActorInCombat(const AActor* Actor);
}
