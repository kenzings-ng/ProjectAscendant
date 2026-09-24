// Copyright Project Ascendant. All Rights Reserved.

#include "Itemization/PASavedItemInstance.h"
#include "Inventory/PAItemStaticDataAsset.h"

FPASavedItemInstance::FPASavedItemInstance()
	: ItemInstanceUID(FGuid())
	, ItemDefId(NAME_None)
	, ItemStaticData(nullptr)
	, SlotIndex(INDEX_NONE)
	, ItemLevel(1)
	, CurrentDurability(100.0f)
	, MaxDurability(100.0f)
	, RarityTag(FGameplayTag())
	, EnhancementLevel(0)
	, StackCount(1)
{
}

FPASavedItemInstance::FPASavedItemInstance(
	const FGuid& InUID,
	FName InDefId,
	UItemStaticDataAsset* InStaticData,
	int32 InSlotIndex,
	int32 InLevel)
	: ItemInstanceUID(InUID.IsValid() ? InUID : FGuid::NewGuid())
	, ItemDefId(InDefId)
	, ItemStaticData(InStaticData)
	, SlotIndex(InSlotIndex)
	, ItemLevel(InLevel)
	, CurrentDurability(100.0f)
	, MaxDurability(100.0f)
	, RarityTag(FGameplayTag())
	, EnhancementLevel(0)
	, StackCount(1)
{
}

void FPASavedItemInstance::PreReplicatedRemove(const FPAInventoryFastArray& InArraySerializer)
{
}

void FPASavedItemInstance::PostReplicatedAdd(const FPAInventoryFastArray& InArraySerializer)
{
}

void FPASavedItemInstance::PostReplicatedChange(const FPAInventoryFastArray& InArraySerializer)
{
}

bool FPASavedItemInstance::Serialize(FArchive& Ar)
{
	Ar << *this;
	return true;
}

FArchive& operator<<(FArchive& Ar, FPASavedItemInstance& Item)
{
	Ar << Item.ItemInstanceUID;
	Ar << Item.ItemDefId;
	Ar << Item.SlotIndex;
	Ar << Item.ItemLevel;
	Ar << Item.CurrentDurability;
	Ar << Item.MaxDurability;
	
	// GameplayTag serialization
	if (Ar.IsLoading())
	{
		FString TagString;
		Ar << TagString;
		Item.RarityTag = TagString.IsEmpty() ? FGameplayTag() : FGameplayTag::RequestGameplayTag(FName(*TagString), false);
	}
	else
	{
		FString TagString = Item.RarityTag.IsValid() ? Item.RarityTag.ToString() : FString();
		Ar << TagString;
	}

	Ar << Item.EnhancementLevel;
	Ar << Item.StackCount;

	// Active Affixes Array
	Ar << Item.ActiveAffixes;

	// Socket Slots Array
	Ar << Item.SocketSlots;

	return Ar;
}

FPASavedItemInstance* FPAInventoryFastArray::FindItemByUID(const FGuid& InUID)
{
	if (!InUID.IsValid())
	{
		return nullptr;
	}

	for (FPASavedItemInstance& Item : Items)
	{
		if (Item.ItemInstanceUID == InUID)
		{
			return &Item;
		}
	}
	return nullptr;
}

const FPASavedItemInstance* FPAInventoryFastArray::FindItemByUID(const FGuid& InUID) const
{
	if (!InUID.IsValid())
	{
		return nullptr;
	}

	for (const FPASavedItemInstance& Item : Items)
	{
		if (Item.ItemInstanceUID == InUID)
		{
			return &Item;
		}
	}
	return nullptr;
}

FPASavedItemInstance* FPAInventoryFastArray::GetItemAtSlot(int32 InSlotIndex)
{
	if (InSlotIndex == INDEX_NONE)
	{
		return nullptr;
	}

	for (FPASavedItemInstance& Item : Items)
	{
		if (Item.SlotIndex == InSlotIndex)
		{
			return &Item;
		}
	}
	return nullptr;
}

const FPASavedItemInstance* FPAInventoryFastArray::GetItemAtSlot(int32 InSlotIndex) const
{
	if (InSlotIndex == INDEX_NONE)
	{
		return nullptr;
	}

	for (const FPASavedItemInstance& Item : Items)
	{
		if (Item.SlotIndex == InSlotIndex)
		{
			return &Item;
		}
	}
	return nullptr;
}

void FPAInventoryFastArray::AddItem(const FPASavedItemInstance& NewItem)
{
	FPASavedItemInstance& AddedItem = Items.Add_GetRef(NewItem);
	MarkItemDirty(AddedItem);
}

bool FPAInventoryFastArray::RemoveItem(const FGuid& InUID)
{
	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		if (Items[Index].ItemInstanceUID == InUID)
		{
			Items.RemoveAt(Index);
			MarkArrayDirty();
			return true;
		}
	}
	return false;
}

void FPAInventoryFastArray::Clear()
{
	Items.Empty();
	MarkArrayDirty();
}

bool FPAInventoryFastArray::Serialize(FArchive& Ar)
{
	Ar << *this;
	return true;
}

FArchive& operator<<(FArchive& Ar, FPAInventoryFastArray& FastArray)
{
	Ar << FastArray.Items;
	return Ar;
}
