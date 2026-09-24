// Copyright Project Ascendant. All Rights Reserved.

#include "Crafting/PABlacksmithSubsystem.h"
#include "Combat/AscendantAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"

UPABlacksmithSubsystem::UPABlacksmithSubsystem()
{
}

void UPABlacksmithSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPABlacksmithSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UPABlacksmithSubsystem::CanAddSocket(
	const FPASavedItemInstance& Item,
	EPAForgeTier ForgeTier,
	int32& OutSocketIndexToUnlock,
	EPACraftingError& OutError) const
{
	OutSocketIndexToUnlock = INDEX_NONE;

	// Quy tắc 1: Outpost Forge (Tier 1): CanAddSocket luôn trả về false, mọi socket giữ nguyên bIsUnlocked = false.
	if (ForgeTier <= EPAForgeTier::Tier1_Outpost || ForgeTier == EPAForgeTier::None)
	{
		OutError = EPACraftingError::MaxTierLevelReached;
		return false;
	}

	// Kiểm tra xem item có cấu hình ô ngọc nào không (Common / Uncommon có 0 socket)
	if (Item.SocketSlots.Num() == 0)
	{
		OutError = EPACraftingError::ItemCannotBeSocketed;
		return false;
	}

	// Tìm ô ngọc đầu tiên còn ở trạng thái khóa (bIsUnlocked == false)
	int32 FirstLockedIndex = INDEX_NONE;
	for (int32 i = 0; i < Item.SocketSlots.Num(); ++i)
	{
		if (!Item.SocketSlots[i].bIsUnlocked)
		{
			FirstLockedIndex = i;
			break;
		}
	}

	// Không còn ô ngọc nào chưa mở khóa -> Đạt trần số lượng socket
	if (FirstLockedIndex == INDEX_NONE)
	{
		OutError = EPACraftingError::MaxSocketsReached;
		return false;
	}

	if (CanAddSocketAtIndex(Item, ForgeTier, FirstLockedIndex, OutError))
	{
		OutSocketIndexToUnlock = FirstLockedIndex;
		return true;
	}

	return false;
}

bool UPABlacksmithSubsystem::CanAddSocket(
	const FPASavedItemInstance& Item,
	EPAForgeTier ForgeTier,
	EPACraftingError& OutError) const
{
	int32 IgnoredIndex = INDEX_NONE;
	return CanAddSocket(Item, ForgeTier, IgnoredIndex, OutError);
}

bool UPABlacksmithSubsystem::CanAddSocketAtIndex(
	const FPASavedItemInstance& Item,
	EPAForgeTier ForgeTier,
	int32 SocketIndex,
	EPACraftingError& OutError) const
{
	// Quy tắc 1: Outpost Forge (Tier 1) luôn từ chối mọi yêu cầu đục lỗ
	if (ForgeTier <= EPAForgeTier::Tier1_Outpost || ForgeTier == EPAForgeTier::None)
	{
		OutError = EPACraftingError::MaxTierLevelReached;
		return false;
	}

	if (!Item.SocketSlots.IsValidIndex(SocketIndex))
	{
		OutError = EPACraftingError::InvalidSocketIndex;
		return false;
	}

	const FPASocketSlot& TargetSlot = Item.SocketSlots[SocketIndex];

	// Đã mở khóa rồi thì không thể mở khóa tiếp
	if (TargetSlot.bIsUnlocked)
	{
		OutError = EPACraftingError::MaxSocketsReached;
		return false;
	}

	// Đếm số lượng ô Regular đã mở khóa hiện tại trên item
	const int32 UnlockedRegularCount = GetUnlockedRegularSocketCount(Item);

	if (TargetSlot.SocketType == EPASocketType::Regular)
	{
		// Quy tắc 2: Field Forge (Tier 2): Cho phép mở khóa (bIsUnlocked = true) tối đa 2 socket Regular trên đồ Rare/Epic.
		// Từ chối nếu vượt quá 2 hoặc item không đủ Regular socket slot.
		if (UnlockedRegularCount >= 2)
		{
			OutError = EPACraftingError::MaxSocketsReached;
			return false;
		}

		if (ForgeTier >= EPAForgeTier::Tier2_Field)
		{
			OutError = EPACraftingError::None;
			return true;
		}

		OutError = EPACraftingError::MaxTierLevelReached;
		return false;
	}
	else if (TargetSlot.SocketType == EPASocketType::Prismatic)
	{
		// Quy tắc 3: Forbidden Forge (Tier 3): Cho phép mở khóa socket Prismatic (độc quyền Legendary).
		if (ForgeTier < EPAForgeTier::Tier3_Forbidden)
		{
			// Field Forge (Tier 2) không được phép mở ô Prismatic
			OutError = EPACraftingError::MaxTierLevelReached;
			return false;
		}

		// Xác thực độc quyền Legendary (bảo đảm kiểm tra chính xác kể cả khi tag chưa nạp từ config)
		const FName TagName = Item.RarityTag.GetTagName();
		if (TagName != FName("Item.Rarity.Legendary") && TagName != FName("Rarity.Legendary"))
		{
			OutError = EPACraftingError::ItemCannotBeSocketed;
			return false;
		}

		OutError = EPACraftingError::None;
		return true;
	}

	OutError = EPACraftingError::ItemCannotBeSocketed;
	return false;
}

bool UPABlacksmithSubsystem::UnlockNextSocket(
	FPASavedItemInstance& Item,
	EPAForgeTier ForgeTier,
	EPACraftingError& OutError)
{
	int32 TargetIndex = INDEX_NONE;
	if (!CanAddSocket(Item, ForgeTier, TargetIndex, OutError))
	{
		return false;
	}

	Item.SocketSlots[TargetIndex].bIsUnlocked = true;
	OutError = EPACraftingError::None;
	return true;
}

bool UPABlacksmithSubsystem::UnlockSocketAtIndex(
	FPASavedItemInstance& Item,
	EPAForgeTier ForgeTier,
	int32 SocketIndex,
	EPACraftingError& OutError)
{
	if (!CanAddSocketAtIndex(Item, ForgeTier, SocketIndex, OutError))
	{
		return false;
	}

	Item.SocketSlots[SocketIndex].bIsUnlocked = true;
	OutError = EPACraftingError::None;
	return true;
}

int32 UPABlacksmithSubsystem::GetUnlockedRegularSocketCount(const FPASavedItemInstance& Item)
{
	int32 Count = 0;
	for (const FPASocketSlot& Slot : Item.SocketSlots)
	{
		if (Slot.SocketType == EPASocketType::Regular && Slot.bIsUnlocked)
		{
			Count++;
		}
	}
	return Count;
}

int32 UPABlacksmithSubsystem::GetUnlockedPrismaticSocketCount(const FPASavedItemInstance& Item)
{
	int32 Count = 0;
	for (const FPASocketSlot& Slot : Item.SocketSlots)
	{
		if (Slot.SocketType == EPASocketType::Prismatic && Slot.bIsUnlocked)
		{
			Count++;
		}
	}
	return Count;
}

int32 UPABlacksmithSubsystem::GetTotalUnlockedSocketCount(const FPASavedItemInstance& Item)
{
	int32 Count = 0;
	for (const FPASocketSlot& Slot : Item.SocketSlots)
	{
		if (Slot.bIsUnlocked)
		{
			Count++;
		}
	}
	return Count;
}

bool UPABlacksmithSubsystem::SocketGem(
	FPASavedItemInstance& Item,
	int32 SocketIndex,
	FName GemId,
	UAscendantAttributeSet* TargetAttributeSet,
	EPACraftingError& OutError)
{
	if (!Item.SocketSlots.IsValidIndex(SocketIndex))
	{
		OutError = EPACraftingError::InvalidSocketIndex;
		return false;
	}

	FPASocketSlot& TargetSlot = Item.SocketSlots[SocketIndex];

	// 1. Chỉ khảm được vào ô socket đã mở khóa (bIsUnlocked == true)
	if (!TargetSlot.bIsUnlocked)
	{
		OutError = EPACraftingError::ItemCannotBeSocketed;
		return false;
	}

	// 2. Không được khảm đè lên slot đã có ngọc
	if (TargetSlot.HasGem())
	{
		OutError = EPACraftingError::SocketNotEmpty;
		return false;
	}

	// 3. Kiểm tra tính hợp lệ của GemId
	FName TargetAttribute;
	float BonusValue = 0.0f;
	if (!GetGemStatBonus(GemId, TargetAttribute, BonusValue))
	{
		OutError = EPACraftingError::InvalidGemItem;
		return false;
	}

	// 4. Nếu là socket Regular, không cho phép khảm Prismatic Gem
	const bool bIsPrismaticGem = GemId.ToString().ToLower().Contains(TEXT("prismatic"));
	if (TargetSlot.SocketType == EPASocketType::Regular && bIsPrismaticGem)
	{
		OutError = EPACraftingError::InvalidGemItem;
		return false;
	}

	// Gán ngọc vào slot
	TargetSlot.SocketedGemId = GemId;

	// Áp dụng chỉ số cộng thêm vào AttributeSet nếu được truyền vào
	if (TargetAttributeSet)
	{
		ApplyGemBonusToAttributeSet(GemId, TargetAttributeSet, true);
	}

	OutError = EPACraftingError::None;
	return true;
}

bool UPABlacksmithSubsystem::SocketGemByUID(
	TArray<FPASavedItemInstance>& InventoryItems,
	const FGuid& ItemUID,
	int32 SocketIndex,
	FName GemId,
	UAscendantAttributeSet* TargetAttributeSet,
	EPACraftingError& OutError)
{
	for (FPASavedItemInstance& Item : InventoryItems)
	{
		if (Item.ItemInstanceUID == ItemUID)
		{
			return SocketGem(Item, SocketIndex, GemId, TargetAttributeSet, OutError);
		}
	}

	OutError = EPACraftingError::ItemNotFound;
	return false;
}

bool UPABlacksmithSubsystem::UnsocketGem(
	FPASavedItemInstance& Item,
	int32 SocketIndex,
	UAscendantAttributeSet* TargetAttributeSet,
	EPACraftingError& OutError)
{
	if (!Item.SocketSlots.IsValidIndex(SocketIndex))
	{
		OutError = EPACraftingError::InvalidSocketIndex;
		return false;
	}

	FPASocketSlot& TargetSlot = Item.SocketSlots[SocketIndex];

	if (!TargetSlot.bIsUnlocked)
	{
		OutError = EPACraftingError::InvalidSocketIndex;
		return false;
	}

	if (!TargetSlot.HasGem())
	{
		OutError = EPACraftingError::SocketEmpty;
		return false;
	}

	const FName OldGemId = TargetSlot.SocketedGemId;
	TargetSlot.SocketedGemId = NAME_None;

	if (TargetAttributeSet)
	{
		ApplyGemBonusToAttributeSet(OldGemId, TargetAttributeSet, false);
	}

	OutError = EPACraftingError::None;
	return true;
}

bool UPABlacksmithSubsystem::GetGemStatBonus(FName GemId, FName& OutAttributeName, float& OutValue)
{
	const FString Str = GemId.ToString().ToLower();

	// Ruby: +20.0f MaxPosture
	if (Str.Contains(TEXT("ruby")))
	{
		OutAttributeName = FName("MaxPosture");
		OutValue = 20.0f;
		return true;
	}

	// Sapphire: +25.0f MaxMana
	if (Str.Contains(TEXT("sapphire")))
	{
		OutAttributeName = FName("MaxMana");
		OutValue = 25.0f;
		return true;
	}

	// Topaz: +15.0f MaxStamina
	if (Str.Contains(TEXT("topaz")))
	{
		OutAttributeName = FName("MaxStamina");
		OutValue = 15.0f;
		return true;
	}

	// Prismatic: +10.0f AttackPower
	if (Str.Contains(TEXT("prismatic")))
	{
		OutAttributeName = FName("AttackPower");
		OutValue = 10.0f;
		return true;
	}

	OutAttributeName = NAME_None;
	OutValue = 0.0f;
	return false;
}

bool UPABlacksmithSubsystem::ApplyGemBonusToAttributeSet(
	FName GemId,
	UAscendantAttributeSet* AttributeSet,
	bool bApply)
{
	if (!AttributeSet)
	{
		return false;
	}

	FName AttributeName;
	float Value = 0.0f;
	if (!GetGemStatBonus(GemId, AttributeName, Value))
	{
		return false;
	}

	const float Delta = bApply ? Value : -Value;

	// Kiểm tra nếu AttributeSet gắn với một Actor có ASC
	AActor* OwningActor = Cast<AActor>(AttributeSet->GetOuter());
	UAbilitySystemComponent* ASC = OwningActor ? AttributeSet->GetOwningAbilitySystemComponent() : nullptr;

	if (AttributeName == FName("MaxPosture"))
	{
		const float NewVal = FMath::Max(0.0f, AttributeSet->GetMaxPosture() + Delta);
		if (ASC)
		{
			AttributeSet->SetMaxPosture(NewVal);
		}
		else
		{
			AttributeSet->InitMaxPosture(NewVal);
		}
		return true;
	}
	else if (AttributeName == FName("MaxMana"))
	{
		const float NewMax = FMath::Max(0.0f, AttributeSet->GetMaxMana() + Delta);
		const float NewCur = FMath::Clamp(AttributeSet->GetMana() + Delta, 0.0f, NewMax);
		if (ASC)
		{
			AttributeSet->SetMaxMana(NewMax);
			AttributeSet->SetMana(NewCur);
		}
		else
		{
			AttributeSet->InitMaxMana(NewMax);
			AttributeSet->InitMana(NewCur);
		}
		return true;
	}
	else if (AttributeName == FName("MaxStamina"))
	{
		const float NewMax = FMath::Max(0.0f, AttributeSet->GetMaxStamina() + Delta);
		const float NewCur = FMath::Clamp(AttributeSet->GetStamina() + Delta, 0.0f, NewMax);
		if (ASC)
		{
			AttributeSet->SetMaxStamina(NewMax);
			AttributeSet->SetStamina(NewCur);
		}
		else
		{
			AttributeSet->InitMaxStamina(NewMax);
			AttributeSet->InitStamina(NewCur);
		}
		return true;
	}
	else if (AttributeName == FName("AttackPower"))
	{
		const float NewVal = FMath::Max(0.0f, AttributeSet->GetAttackPower() + Delta);
		if (ASC)
		{
			AttributeSet->SetAttackPower(NewVal);
		}
		else
		{
			AttributeSet->InitAttackPower(NewVal);
		}
		return true;
	}

	return false;
}
