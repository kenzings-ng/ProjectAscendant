// Copyright Project Ascendant. All Rights Reserved.

#include "Game/PAGameModeBase.h"
#include "Character/PABaseCharacter.h"
#include "Controller/PABasePlayerController.h"

APAGameModeBase::APAGameModeBase()
{
	// Thiết lập nhân vật mặc định và bộ điều khiển theo chuẩn 2.5D Isometric của dự án
	DefaultPawnClass = APABaseCharacter::StaticClass();
	PlayerControllerClass = APABasePlayerController::StaticClass();
}
