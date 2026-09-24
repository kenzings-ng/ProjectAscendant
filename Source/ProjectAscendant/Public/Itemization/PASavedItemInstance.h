// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GameplayTagContainer.h"
#include "Itemization/PAAffixTypes.h"
#include "PASavedItemInstance.generated.h"

class UItemStaticDataAsset;
struct FPAInventoryFastArray;

/**
 * FPASavedItemInstance
 *
 * Cấu trúc đại diện cho phiên bản trang bị sống động (Living Equipment) và có thể lưu trữ,
 * kế thừa FFastArraySerializerItem để đồng bộ mạng hiệu năng cao qua NetDeltaSerialize (Story item-001).
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPASavedItemInstance : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Itemization|Instance")
	FGuid ItemInstanceUID;

	UPROPERTY(BlueprintReadOnly, Category = "Itemization|Instance")
	FName ItemDefId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Itemization|Instance")
	TObjectPtr<UItemStaticDataAsset> ItemStaticData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Itemization|Instance")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Itemization|Instance")
	int32 ItemLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Itemization|Instance")
	float CurrentDurability = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Itemization|Instance")
	float MaxDurability = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Itemization|Instance")
	FGameplayTag RarityTag;

	UPROPERTY(BlueprintReadOnly, Category = "Itemization|Instance")
	int32 EnhancementLevel = 0; // +0 đến +10

	UPROPERTY(BlueprintReadOnly, Category = "Itemization|Instance")
	TArray<FPAAffixInstance> ActiveAffixes;

	UPROPERTY(BlueprintReadOnly, Category = "Itemization|Instance")
	TArray<FPASocketSlot> SocketSlots;

	UPROPERTY(BlueprintReadOnly, Category = "Itemization|Instance")
	int32 StackCount = 1;

	FPASavedItemInstance();

	FPASavedItemInstance(
		const FGuid& InUID,
		FName InDefId,
		UItemStaticDataAsset* InStaticData,
		int32 InSlotIndex = INDEX_NONE,
		int32 InLevel = 1);

	// FastArraySerializer lifecycle callbacks
	void PreReplicatedRemove(const FPAInventoryFastArray& InArraySerializer);
	void PostReplicatedAdd(const FPAInventoryFastArray& InArraySerializer);
	void PostReplicatedChange(const FPAInventoryFastArray& InArraySerializer);

	// Binary serialization cho SaveGame / Dedicated Server database
	bool Serialize(FArchive& Ar);
	friend FArchive& operator<<(FArchive& Ar, FPASavedItemInstance& Item);
};

template<>
struct TStructOpsTypeTraits<FPASavedItemInstance> : public TStructOpsTypeTraitsBase2<FPASavedItemInstance>
{
	enum
	{
		WithSerializer = true,
	};
};

/**
 * FPAInventoryFastArray
 *
 * Mảng quản lý túi đồ / trang bị nén mạng theo chuẩn Unreal Engine FFastArraySerializer.
 * Đảm bảo chỉ gửi đúng delta overhead (<= 0.5ms) khi có 1 item thay đổi (Story item-001 AC-1).
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAInventoryFastArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPASavedItemInstance> Items;

	FPAInventoryFastArray() = default;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FPASavedItemInstance, FPAInventoryFastArray>(Items, DeltaParms, *this);
	}

	void MarkItemDirty(FPASavedItemInstance& Item)
	{
		FFastArraySerializer::MarkItemDirty(Item);
	}

	void MarkArrayDirty()
	{
		FFastArraySerializer::MarkArrayDirty();
	}

	// Helper operations
	FPASavedItemInstance* FindItemByUID(const FGuid& InUID);
	const FPASavedItemInstance* FindItemByUID(const FGuid& InUID) const;

	FPASavedItemInstance* GetItemAtSlot(int32 InSlotIndex);
	const FPASavedItemInstance* GetItemAtSlot(int32 InSlotIndex) const;

	void AddItem(const FPASavedItemInstance& NewItem);
	bool RemoveItem(const FGuid& InUID);
	void Clear();

	int32 Num() const { return Items.Num(); }

	// Binary serialization cho toàn bộ mảng túi đồ
	bool Serialize(FArchive& Ar);
	friend FArchive& operator<<(FArchive& Ar, FPAInventoryFastArray& FastArray);
};

template<>
struct TStructOpsTypeTraits<FPAInventoryFastArray> : public TStructOpsTypeTraitsBase2<FPAInventoryFastArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithSerializer = true,
	};
};
