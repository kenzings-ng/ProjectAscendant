// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Crafting/PABlacksmithTypes.h"
#include "Economy/PAMerchantTypes.h"
#include "PAServiceRequestTestProbe.generated.h"

/**
 * X11b automation-test helper: records the dynamic UI events of UPAMerchantShopWidget / UPABlacksmithForgeWidget
 * so tests can assert that completion fires only after the server confirmation. Not used by game code.
 */
UCLASS(Transient)
class UPAServiceRequestTestProbe : public UObject
{
	GENERATED_BODY()

public:
	int32 ShopCompletedCount = 0;
	int32 ShopRejectedCount = 0;
	EPATransactionError LastShopError = EPATransactionError::None;

	int32 ForgeCompletedCount = 0;
	bool bLastForgeSuccess = false;
	EPACraftingError LastForgeError = EPACraftingError::None;

	UFUNCTION()
	void HandleShopCompleted() { ++ShopCompletedCount; }

	UFUNCTION()
	void HandleShopRejected(EPATransactionError ErrorCode) { ++ShopRejectedCount; LastShopError = ErrorCode; }

	UFUNCTION()
	void HandleForgeCompleted(bool bSuccess, EPACraftingError ErrorCode) { ++ForgeCompletedCount; bLastForgeSuccess = bSuccess; LastForgeError = ErrorCode; }
};
