// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_Item_IronPlate.generated.h"

/**
 * UGE_Item_IronPlate
 *
 * GameplayEffect cấp chỉ số Phòng ngự (+40 Armor) khi trang bị Áo Giáp Sắt (AC-1).
 */
UCLASS()
class PROJECTASCENDANT_API UGE_Item_IronPlate : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_Item_IronPlate();
};
