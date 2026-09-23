// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PAGameModeBase.generated.h"

class UPALoginWidget;

/**
 * APAGameModeBase
 *
 * GameMode cơ sở cho Project Ascendant (Dedicated Server Authority).
 * Tự động gán APABaseCharacter (Pawn) và APABasePlayerController (Controller).
 */
UCLASS()
class PROJECTASCENDANT_API APAGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	APAGameModeBase();
};
