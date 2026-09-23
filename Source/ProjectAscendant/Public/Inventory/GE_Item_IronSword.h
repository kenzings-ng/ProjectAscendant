// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_Item_IronSword.generated.h"

/**
 * UGE_Item_IronSword
 *
 * GameplayEffect cấp chỉ số Sát thương vật lý (+15 AttackPower) khi trang bị Kiếm Sắt (AC-1).
 */
UCLASS()
class PROJECTASCENDANT_API UGE_Item_IronSword : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_Item_IronSword();
};
