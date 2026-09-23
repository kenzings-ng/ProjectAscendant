// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Notifies/PaperZDAnimNotifyState.h"
#include "PAPaperZDNotifyState_Hitbox.generated.h"

class APABaseCharacter;

/**
 * UPAPaperZDNotifyState_Hitbox
 *
 * PaperZD NotifyState dùng trong các animation tấn công (Attack Combo):
 *  - OnNotifyBegin: Mở cửa sổ va chạm Hitbox trên Server Authority, bắt đầu quét trace kiểm tra trúng đòn.
 *  - OnNotifyEnd: Đóng cửa sổ va chạm Hitbox trên Server Authority, kết thúc quét va chạm.
 */
UCLASS(Blueprintable, meta = (DisplayName = "PA Combat Hitbox Window"))
class PROJECTASCENDANT_API UPAPaperZDNotifyState_Hitbox : public UPaperZDAnimNotifyState
{
	GENERATED_BODY()

public:
	UPAPaperZDNotifyState_Hitbox();

	virtual void OnNotifyBegin_Implementation(UPaperZDAnimInstance* OwningInstance) const override;
	virtual void OnNotifyEnd_Implementation(UPaperZDAnimInstance* OwningInstance) const override;

private:
	APABaseCharacter* GetOwningCharacter(UPaperZDAnimInstance* OwningInstance) const;
};
